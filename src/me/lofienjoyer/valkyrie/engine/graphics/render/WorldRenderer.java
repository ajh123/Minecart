package me.lofienjoyer.valkyrie.engine.graphics.render;

import me.lofienjoyer.valkyrie.Valkyrie;
import me.lofienjoyer.valkyrie.engine.config.Config;
import me.lofienjoyer.valkyrie.engine.events.Event;
import me.lofienjoyer.valkyrie.engine.events.mesh.MeshGenerationEvent;
import me.lofienjoyer.valkyrie.engine.events.world.ChunkLoadEvent;
import me.lofienjoyer.valkyrie.engine.events.world.ChunkUnloadEvent;
import me.lofienjoyer.valkyrie.engine.events.world.ChunkUpdateEvent;
import me.lofienjoyer.valkyrie.engine.graphics.camera.Camera;
import me.lofienjoyer.valkyrie.engine.graphics.mesh.MeshBundle;
import me.lofienjoyer.valkyrie.engine.graphics.shaders.Shader;
import me.lofienjoyer.valkyrie.engine.resources.ResourceLoader;
import me.lofienjoyer.valkyrie.engine.utils.Maths;
import me.lofienjoyer.valkyrie.engine.world.BlockRegistry;
import me.lofienjoyer.valkyrie.engine.world.Chunk;
import me.lofienjoyer.valkyrie.engine.world.ChunkState;
import me.lofienjoyer.valkyrie.engine.world.World;
import org.joml.Matrix4f;
import org.joml.Vector2i;
import org.joml.Vector3i;
import org.lwjgl.glfw.GLFW;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Future;

import static org.lwjgl.opengl.GL45.*;

public class WorldRenderer {

    public static int VIEW_DISTANCE = 8;

    private final World world;

    private Matrix4f projectionMatrix;
    private final FrustumCullingTester tester;

    private final Shader solidsShader;
    private final Shader transparencyShader;

    private final Map<Vector2i, MeshBundle> chunkMeshes;
    private final Map<Vector3i, Future<MeshBundle>> meshFutures;
    private final Map<Vector3i, MeshBundle> meshesToUpload;
    private final Map<Vector3i, Chunk> chunksToUpdate;

    private final List<Event> eventsToProcess;

    public WorldRenderer(World world) {
        this.world = world;

        this.projectionMatrix = new Matrix4f();
        this.solidsShader = ResourceLoader.loadShader("Solid Mesh Shader", "res/shaders/world/solid_vert.glsl", "res/shaders/world/solid_frag.glsl");
        this.transparencyShader = ResourceLoader.loadShader("Transparent Mesh Shader", "res/shaders/world/transparent_vert.glsl", "res/shaders/world/transparent_frag.glsl");

        this.tester = new FrustumCullingTester();

        this.chunkMeshes = new HashMap<>();
        this.meshFutures = new HashMap<>();
        this.meshesToUpload = new HashMap<>();
        this.chunksToUpdate = new HashMap<>();
        this.eventsToProcess = new ArrayList<>();

        // TODO: 9/1/24 Dispose this when the instance is destroyed
        Valkyrie.EVENT_HANDLER.registerListener(ChunkLoadEvent.class, this::handleChunkLoading);
        Valkyrie.EVENT_HANDLER.registerListener(ChunkUpdateEvent.class, this::handleChunkUpdating);
        Valkyrie.EVENT_HANDLER.registerListener(ChunkUnloadEvent.class, this::handleChunkUnloading);
        Valkyrie.EVENT_HANDLER.registerListener(MeshGenerationEvent.class, this::handleMeshGeneration);

        var config = Config.getInstance();
        VIEW_DISTANCE = config.get("view_distance", Integer.class);

        solidsShader.bind();
        solidsShader.loadInt("leavesId", BlockRegistry.getBlock(6).getTopTexture());
        solidsShader.loadInt("waterId", BlockRegistry.getBlock(7).getTopTexture());

        transparencyShader.bind();
        transparencyShader.loadInt("leavesId", BlockRegistry.getBlock(6).getTopTexture());
        transparencyShader.loadInt("waterId", BlockRegistry.getBlock(7).getTopTexture());

        Valkyrie.LOG.info("World renderer has been setup");
    }

    /**
     * Renders the world from the point of view of the provided camera
     * @param camera Camera to render the world from
     */
    public void render(Camera camera) {
        updateFrustum(camera);
        // Get the block the camera is in (for in-water effects)
        int headBlock = world.getBlock(camera.getPosition());

        var chunksToRender = new HashMap<Vector2i, MeshBundle>();
        chunkMeshes.forEach((position, mesh) -> {
            if (!mesh.isLoaded() || !tester.isChunkInside(position, camera.getPosition().y))
                return;

            chunksToRender.put(position, mesh);
        });

        Renderer.bindTexture2D(BlockRegistry.TILESET_TEXTURE_ID);
        // TODO: Move to a single draw call
        renderBlockMeshes(camera, headBlock, chunksToRender, false);
        renderBlockMeshes(camera, headBlock, chunksToRender, true);
        renderCustomModelMeshes(camera, headBlock, chunksToRender);
    }

    /**
     * Processes events, uploads meshes in queue and
     * sends pending chunks to be meshed to the meshing service
     */
    public void update() {
        processEvents();
        uploadPendingMeshes();
        generatePendingMeshes();
    }

    /**
     * @param camera Point of view from which render the scene
     * @param headBlock Id of the block the camera is inside
     * @param chunksToRender Chunks to be rendered
     */
    private void renderBlockMeshes(Camera camera, int headBlock, Map<Vector2i, MeshBundle> chunksToRender, boolean transparency) {
        if (transparency) {
            Renderer.enableBlend();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            Renderer.disableCullFace();
        } else {
            Renderer.enableDepthTest();
            Renderer.enableCullFace();
            Renderer.setCullFace(false);
        }

        solidsShader.bind();
        solidsShader.loadMatrix("viewMatrix", Maths.createViewMatrix(camera));
        solidsShader.loadVector("cameraPosition", camera.getPosition());
        solidsShader.loadFloat("time", (float) GLFW.glfwGetTime());
        solidsShader.loadFloat("viewDistance", VIEW_DISTANCE * 32 - 32);
        solidsShader.loadBoolean("inWater", headBlock == 7);
        solidsShader.loadBoolean("transparent", transparency);

        chunksToRender.forEach((position, mesh) -> {
            for (int y = 0; y < World.CHUNK_HEIGHT / World.CHUNK_SECTION_HEIGHT; y++) {
                if (mesh.getSolidMeshes(y).getVertexCount() == 0)
                    continue;
                solidsShader.loadMatrix("transformationMatrix", Maths.createTransformationMatrix(new Vector3i(position.x, y, position.y)));

                glBindVertexArray(mesh.getSolidMeshes(y).getVaoId());
                glEnableVertexAttribArray(0);

                glDrawElements(GL_TRIANGLES, mesh.getSolidMeshes(y).getVertexCount(), GL_UNSIGNED_INT, 0);
            }
        });
    }

    /**
     * @param camera Point of view from which render the scene
     * @param headBlock Id of the block the camera is inside
     * @param chunksToRender Chunks to be rendered
     */
    private void renderCustomModelMeshes(Camera camera, int headBlock, Map<Vector2i, MeshBundle> chunksToRender) {
        Renderer.enableBlend();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        Renderer.disableCullFace();

        transparencyShader.bind();
        transparencyShader.loadMatrix("viewMatrix", Maths.createViewMatrix(camera));
        transparencyShader.loadVector("cameraPosition", camera.getPosition());
        transparencyShader.loadFloat("time", (float) GLFW.glfwGetTime());
        transparencyShader.loadFloat("viewDistance", VIEW_DISTANCE * 32 - 32);
        transparencyShader.loadBoolean("inWater", headBlock == 7);
        transparencyShader.loadBoolean("transparent", false);

        chunksToRender.forEach((position, mesh) -> {
            for (int y = 0; y < World.CHUNK_HEIGHT / World.CHUNK_SECTION_HEIGHT; y++) {
                if (mesh.getSolidMeshes(y).getVertexCount() == 0)
                    continue;
                transparencyShader.loadMatrix("transformationMatrix", Maths.createTransformationMatrix(new Vector3i(position.x, y, position.y)));

                glBindVertexArray(mesh.getTransparentMeshes(y).getVaoId());
                glEnableVertexAttribArray(0);

                glDrawElements(GL_TRIANGLES, mesh.getTransparentMeshes(y).getVertexCount(), GL_UNSIGNED_INT, 0);
            }
        });

        Renderer.disableBlend();
    }

    private void handleChunkLoading(ChunkLoadEvent event) {
        synchronized (eventsToProcess) {
            eventsToProcess.add(event);
        }
    }

    private void handleChunkUpdating(ChunkUpdateEvent event) {
        synchronized (eventsToProcess) {
            eventsToProcess.add(event);
        }
    }

    private void handleMeshGeneration(MeshGenerationEvent event) {
        synchronized (eventsToProcess) {
            eventsToProcess.add(event);
        }
    }

    private void handleChunkUnloading(ChunkUnloadEvent event) {
        synchronized (eventsToProcess) {
            eventsToProcess.add(event);
        }
    }

    private void generateMesh(Chunk chunk, MeshBundle meshBundle, int section) {
        if (chunk.getState() == ChunkState.UNLOADED)
            return;

        var position = chunk.getPosition();
        var meshPosition = new Vector3i(position.x, section, position.y);
        var meshFuture = meshFutures.get(meshPosition);

        if (meshFuture != null) {
            meshFuture.cancel(true);
            meshFutures.remove(meshPosition);
        }

        chunk.cacheNeighbors();
        meshFuture = Valkyrie.getMeshingService().submit(() -> {
            meshBundle.compute(section);
            Valkyrie.EVENT_HANDLER.process(new MeshGenerationEvent(meshPosition, meshBundle));
            return meshBundle;
        });

        meshFutures.put(meshPosition, meshFuture);
    }

    // Clean this mess
    private void processEvents() {
        synchronized (eventsToProcess) {
            eventsToProcess.forEach(event -> {
                if (event instanceof ChunkLoadEvent) {
                    var chunkLoadEvent = (ChunkLoadEvent) event;
                    var meshBundle = new MeshBundle(chunkLoadEvent.getChunk());
                    chunkMeshes.put(chunkLoadEvent.getChunk().getPosition(), meshBundle);
                    for (int i = 0; i < 8; i++) {
                        chunksToUpdate.put(new Vector3i(chunkLoadEvent.getChunk().getPosition().x, i, chunkLoadEvent.getChunk().getPosition().y), chunkLoadEvent.getChunk());
                    }
                } else if (event instanceof ChunkUpdateEvent) {
                    var chunkUpdateEvent = (ChunkUpdateEvent) event;
                    if (chunkUpdateEvent.getChunk() == null)
                        return;

                    chunksToUpdate.put(new Vector3i(chunkUpdateEvent.getChunk().getPosition().x, chunkUpdateEvent.getUpdatePosition().y / World.CHUNK_SECTION_HEIGHT, chunkUpdateEvent.getChunk().getPosition().y), chunkUpdateEvent.getChunk());
                } else if (event instanceof ChunkUnloadEvent) {
                    var chunkUnloadEvent = (ChunkUnloadEvent) event;
                    for (int i = 0; i < 8; i++) {
                        var meshPosition = new Vector3i(chunkUnloadEvent.getChunk().getPosition().x, i, chunkUnloadEvent.getChunk().getPosition().y);
                        var meshFuture = meshFutures.get(meshPosition);
                        if (meshFuture != null) {
                            meshFuture.cancel(true);
                            meshFutures.remove(meshPosition);
                        }
                    }

                    chunkMeshes.remove(chunkUnloadEvent.getChunk().getPosition());
                } else if (event instanceof MeshGenerationEvent) {
                    var meshGenerationEvent = (MeshGenerationEvent) event;
                    meshesToUpload.put(meshGenerationEvent.getPosition(), meshGenerationEvent.getMeshBundle());
                }
            });

            eventsToProcess.clear();
        }
    }

    private void uploadPendingMeshes() {
        var meshesToUploadIterator = meshesToUpload.entrySet().iterator();
        while (meshesToUploadIterator.hasNext()) {
            var currentMeshEntry = meshesToUploadIterator.next();
            meshesToUploadIterator.remove();

            if (currentMeshEntry.getValue().loadMeshToGpu(currentMeshEntry.getKey().y))
                break;
        }
    }

    private void generatePendingMeshes() {
        chunksToUpdate.forEach((position, chunk) -> {
            generateMesh(chunk, chunkMeshes.get(chunk.getPosition()), position.y);
        });
        chunksToUpdate.clear();
    }

    private void updateFrustum(Camera camera) {
        tester.updateFrustum(projectionMatrix, Maths.createViewMatrix(camera));
    }

    /**
     * Updates the projection matrix based on the screen resolution
     * @param width Screen width
     * @param height Screen height
     */
    public void setupProjectionMatrix(int width, int height) {
        this.projectionMatrix = new Matrix4f();
        this.projectionMatrix.perspective(Valkyrie.FOV, width / (float)height, 0.01f, 5000f);

        solidsShader.bind();
        solidsShader.loadMatrix("projectionMatrix", projectionMatrix);
        transparencyShader.bind();
        transparencyShader.loadMatrix("projectionMatrix", projectionMatrix);
    }

}
