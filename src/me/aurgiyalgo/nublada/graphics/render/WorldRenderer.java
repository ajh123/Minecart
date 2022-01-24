package me.aurgiyalgo.nublada.graphics.render;

import me.aurgiyalgo.nublada.graphics.camera.Camera;
import me.aurgiyalgo.nublada.graphics.shaders.StaticShader;
import me.aurgiyalgo.nublada.utils.Maths;
import me.aurgiyalgo.nublada.world.World;
import org.joml.Matrix4f;
import org.lwjgl.opengl.GL30;

import java.util.concurrent.atomic.AtomicInteger;

import static me.aurgiyalgo.nublada.world.World.CHUNK_WIDTH;

public class WorldRenderer {

    private static final int VIEW_DISTANCE = 16;

    private Matrix4f projectionMatrix;
    private final FrustumCullingTester tester;

    public WorldRenderer() {
        this.projectionMatrix = new Matrix4f();

        this.tester = new FrustumCullingTester();
    }

    public void render(World world, StaticShader shader, Camera camera) {
        GL30.glEnable(GL30.GL_DEPTH_TEST);
        GL30.glEnable(GL30.GL_CULL_FACE);
        GL30.glCullFace(GL30.GL_BACK);

        shader.loadProjectionMatrix(projectionMatrix);
        GL30.glBindTexture(GL30.GL_TEXTURE_2D_ARRAY, world.getTextureId());

        long timer = System.nanoTime();
        AtomicInteger counter = new AtomicInteger();

        int playerX = (int) Math.floor(camera.getPosition().x / (float) CHUNK_WIDTH);
        int playerZ = (int) Math.floor(camera.getPosition().z / (float) CHUNK_WIDTH);

        for(int x = -VIEW_DISTANCE; x <= VIEW_DISTANCE; x++){
            for(int z = -VIEW_DISTANCE; z <= VIEW_DISTANCE; z++){
                int chunkX = playerX + x;
                int chunkZ = playerZ + z;

                int distance = x * x + z * z;

                if(distance < VIEW_DISTANCE * VIEW_DISTANCE){
                    if (world.getChunk(chunkX, chunkZ) == null) {
                        world.addChunk(chunkX, chunkZ);
                    }
                }

            }
        }

        world.getChunks().forEach((position, chunk) -> {
            chunk.prepare();
            if (chunk.getModel() == null) return;
            if (!tester.isChunkInside(chunk, camera.getPosition().y)) return;
            counter.incrementAndGet();
            GL30.glBindVertexArray(chunk.getModel().getVao());
            GL30.glEnableVertexAttribArray(0);
            GL30.glEnableVertexAttribArray(1);
            GL30.glEnableVertexAttribArray(2);
            shader.loadTranslation(position);
            GL30.glDrawElements(GL30.GL_TRIANGLES, chunk.getModel().getVertexCount(), GL30.GL_UNSIGNED_INT, 0);
        });
        System.out.println("World render: " + ((System.nanoTime() - timer) / 1000000f) + "ms (" + counter.get() + " chunks)");

        GL30.glBindTexture(GL30.GL_TEXTURE_2D_ARRAY, 0);
        GL30.glDisableVertexAttribArray(0);
        GL30.glDisableVertexAttribArray(1);
        GL30.glEnableVertexAttribArray(2);
        GL30.glBindVertexArray(0);
    }

    public void updateFrustum(Camera camera) {
        tester.updateFrustum(projectionMatrix, Maths.createViewMatrix(camera));
    }

    public void setupProjectionMatrix(int width, int height) {
        System.out.println(width / (float)height);
        this.projectionMatrix = new Matrix4f();
        this.projectionMatrix.perspective(70, width / (float)height, 0.01f, 5000f);
    }

}
