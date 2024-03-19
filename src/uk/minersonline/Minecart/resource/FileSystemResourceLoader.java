package uk.minersonline.Minecart.resource;

import me.lofienjoyer.valkyrie.Valkyrie;
import me.lofienjoyer.valkyrie.engine.graphics.texture.ImageData;
import org.lwjgl.BufferUtils;
import org.lwjgl.opengl.GL30;
import org.lwjgl.system.MemoryStack;

import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.URISyntaxException;
import java.net.URL;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.nio.file.Files;
import java.nio.file.Path;

import static org.lwjgl.stb.STBImage.stbi_failure_reason;
import static org.lwjgl.stb.STBImage.stbi_load;

class FileSystemResourceLoader implements ResourceLoader {
	@Override
	public ImageData loadImage(ResourceIdentifier path) throws ResourceLoadingException {
		Path pathO = path.toPath();
		ByteBuffer buf;
		int width;
		int height;

		try (MemoryStack stack = MemoryStack.stackPush()) {
			IntBuffer w = stack.mallocInt(1);
			IntBuffer h = stack.mallocInt(1);
			IntBuffer channels = stack.mallocInt(1);

			buf = stbi_load(pathO.toString(), w, h, channels, 4);
			if (buf == null) {
				throw new ResourceLoadingException("Image not loaded: " + stbi_failure_reason());
			}

			width = w.get();
			height = h.get();
		}

		return new ImageData(width, height, buf);
	}

	@Override
	public BufferedImage loadBufferedImage(ResourceIdentifier path) throws ResourceLoadingException {
		try {
			Path pathO = path.toPath();
			return ImageIO.read(new File(pathO.toString()));
		} catch (IOException e) {
			throw new ResourceLoadingException("The file could not be found", e);
		}
	}

	@Override
	public ByteBuffer fileToBuffer(ResourceIdentifier path) throws ResourceLoadingException {
		try {
			Path pathO = path.toPath();

			byte[] data = Files.readAllBytes(pathO);
			ByteBuffer img = BufferUtils.createByteBuffer(data.length);
			img.put(data);
			img.flip();

			return img;
		} catch (IOException e) {
			throw new ResourceLoadingException();
		}
	}

	@Override
	public File loadFile(ResourceIdentifier path) throws ResourceLoadingException {
		Path pathO = path.toPath();
		File file = new File(pathO.toString());
		if (!file.exists()) {
			throw new ResourceLoadingException("File "+path+" not found!");
		}
		return new File(pathO.toString());
	}
}
