package me.lofienjoyer.valkyrie.engine.graphics.texture;

import java.io.File;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.nio.file.Files;

import org.lwjgl.BufferUtils;
import org.lwjgl.system.MemoryStack;
import uk.minersonline.Minecart.resource.ResourceIdentifier;
import uk.minersonline.Minecart.resource.ResourceManager;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL30.glGenerateMipmap;
import static org.lwjgl.stb.STBImage.*;

public class Texture {

    private final int id;

    public Texture(ResourceIdentifier fileName) throws Exception {
        this(loadTexture(fileName));
    }

    public Texture(int id) {
        this.id = id;
    }

    public void bind() {
        glBindTexture(GL_TEXTURE_2D, id);
    }

    public int getId() {
        return id;
    }

    private static int loadTexture(ResourceIdentifier fileName) throws Exception {
        int width;
        int height;
        ByteBuffer buf;
        File file = ResourceManager.loadFile(fileName);

        try (MemoryStack stack = MemoryStack.stackPush()) {
            IntBuffer w = stack.mallocInt(1);
            IntBuffer h = stack.mallocInt(1);
            IntBuffer channels = stack.mallocInt(1);

            byte[] data = Files.readAllBytes(file.toPath());
            ByteBuffer img = BufferUtils.createByteBuffer(data.length);
            img.put(data);
            img.flip();

            buf = stbi_load_from_memory(img, w, h, channels, 4);
            if (buf == null) {
                throw new Exception("Image file [" + fileName  + "] not loaded: " + stbi_failure_reason());
            }

            width = w.get();
            height = h.get();
        }

        int textureId = glGenTextures();
        glBindTexture(GL_TEXTURE_2D, textureId);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, buf);

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(buf);

        glBindTexture(GL_TEXTURE_2D, 0);

        return textureId;
    }

    public void cleanup() {
        glDeleteTextures(id);
    }
}

