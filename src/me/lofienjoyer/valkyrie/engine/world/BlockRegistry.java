package me.lofienjoyer.valkyrie.engine.world;

import me.lofienjoyer.valkyrie.Valkyrie;
import me.lofienjoyer.valkyrie.engine.graphics.mesh.BlockMeshType;
import me.lofienjoyer.valkyrie.engine.utils.YamlLoader;
import me.lofienjoyer.valkyrie.engine.world.registry.BlockBuilder;
import org.yaml.snakeyaml.Yaml;
import uk.minersonline.Minecart.resource.ResourceIdentifier;
import uk.minersonline.Minecart.resource.ResourceLoadingException;
import uk.minersonline.Minecart.resource.ResourceManager;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.util.*;
import java.util.stream.Collectors;

public class BlockRegistry {

    private static Block[] BLOCKS;
    public static int TILESET_TEXTURE_ID;
    private static List<ResourceIdentifier> texturesList;

    public static void setup(List<String> namespaces) {
        Valkyrie.LOG.info("Setting up block registry...");
        long timer = System.nanoTime();

        texturesList = new ArrayList<>();

        loadTextures(namespaces);

        loadBlocks(namespaces);

        Valkyrie.LOG.info("Block registry has been setup (" + ((System.nanoTime() - timer) / 1000000f) + "ms)");
    }

    public static Block getBlock(int id) {
        return BLOCKS[id];
    }

    public static Block getBlock(String name) {
        for (Block block : BLOCKS) {
            if (block != null && block.getName().equals(name))
                return block;
        }
        return null;
    }

    private static void loadBlocks(List<String> namespaces) {
        var blocksToLoad = new ArrayList<BlockBuilder>();

        for (String namespace : namespaces) {
            File blocksFolder = null;
            try {
                blocksFolder = ResourceManager.loadFile(
                        new ResourceIdentifier(namespace, "blocks")
                );
            } catch (ResourceLoadingException e) {
                throw new RuntimeException(e);
            }
            if (!blocksFolder.exists()) {
                throw new RuntimeException("res/blocks folder not found!");
            }
            var loader = new YamlLoader();

            var blockFiles = Arrays.stream(blocksFolder.listFiles()).collect(Collectors.toList());

            for (int i = 0; i < blockFiles.size(); i++) {
                var blockFile = blockFiles.get(i);

                if (!blockFile.getName().endsWith(".yml"))
                    continue;

                loader.loadFile(blockFile);

                var builder = new BlockBuilder();

                String name = loader.get("name", String.class);
                ResourceIdentifier identifier = new ResourceIdentifier(loader.get("texture", String.class));
                int texture = texturesList.indexOf(identifier);

                builder.setName(name);
                builder.setTexture(texture);

                loader.ifDataPresent("northTexture", String.class, (value) -> builder.setNorthTexture(texturesList.indexOf(new ResourceIdentifier(value))));
                loader.ifDataPresent("southTexture", String.class, (value) -> builder.setSouthTexture(texturesList.indexOf(new ResourceIdentifier(value))));
                loader.ifDataPresent("westTexture", String.class, (value) -> builder.setWestTexture(texturesList.indexOf(new ResourceIdentifier(value))));
                loader.ifDataPresent("eastTexture", String.class, (value) -> builder.setEastTexture(texturesList.indexOf(new ResourceIdentifier(value))));
                loader.ifDataPresent("topTexture", String.class, (value) -> builder.setTopTexture(texturesList.indexOf(new ResourceIdentifier(value))));
                loader.ifDataPresent("bottomTexture", String.class, (value) -> builder.setBottomTexture(texturesList.indexOf(new ResourceIdentifier(value))));

                builder.setTransparent(loader.get("transparent", Boolean.class, false));
                builder.setShouldDrawBetween(loader.get("drawBetween", Boolean.class, false));
                builder.setHasCollision(loader.get("collision", Boolean.class, true));

                loader.ifDataPresent("movementResistance", Double.class, (value) -> {
                    builder.setMovementResistance((float) (double) value);
                });

                loader.ifDataPresent("model", String.class, (value) -> {
                    switch (value) {
                        case "x":
                            builder.setMeshType(BlockMeshType.X);
                            builder.setCustomModel(true);
                            break;
                        case "block":
                        default:
                            builder.setMeshType(BlockMeshType.FULL);
                    }
                });

                blocksToLoad.add(builder);
            }
        }

        BLOCKS = new Block[blocksToLoad.size() + 1];
        for (int i = 0; i < blocksToLoad.size(); i++) {
            BLOCKS[i + 1] = blocksToLoad.get(i).toBlock(i + 1);
        }
    }

    private static void loadTextures(List<String> namespaces) {
        for (String namespace : namespaces) {
            File texturesFolder = null;
            try {
                texturesFolder = ResourceManager.loadFile(
                        new ResourceIdentifier(namespace, "textures/blocks")
                );
            } catch (ResourceLoadingException e) {
                throw new RuntimeException(e);
            }
            if (!texturesFolder.exists()) {
                throw new RuntimeException("res/textures folder not found!");
            }

            for (var textureFile : texturesFolder.listFiles()) {
                if (!textureFile.getName().endsWith(".png")) {
                    Valkyrie.LOG.info("Non-texture file found while loading textures: " + textureFile.getName());
                    continue;
                }

                ResourceIdentifier texture = new ResourceIdentifier(textureFile.getName().replace(".png", ""));
                texturesList.add(texture);
            }
        }

        TILESET_TEXTURE_ID = Valkyrie.LOADER.loadTileset(texturesList);
    }

    private static int getTextureId(String textureName) {
        int textureId = texturesList.indexOf(new ResourceIdentifier("textures/blocks/" + textureName + ".png"));

        if (textureId == -1) {
            texturesList.add(new ResourceIdentifier("textures/blocks/" + textureName + ".png"));
            textureId = texturesList.size() - 1;
        }

        return textureId;
    }

    public static int getBlockCount() {
        return BLOCKS.length;
    }

}
