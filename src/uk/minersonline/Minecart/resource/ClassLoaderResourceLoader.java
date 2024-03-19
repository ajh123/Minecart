package uk.minersonline.Minecart.resource;

import me.lofienjoyer.valkyrie.engine.graphics.texture.ImageData;
import org.lwjgl.BufferUtils;
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
import java.nio.file.Path;

import static org.lwjgl.stb.STBImage.*;

class ClassLoaderResourceLoader implements ResourceLoader {
	@Override
	public ImageData loadImage(ResourceIdentifier path) throws ResourceLoadingException {
		try {
			Path pathO = path.toPath();
			InputStream file = this.getClass().getClassLoader().getResourceAsStream(pathO.toString());

			if (file == null) {
				throw new IOException("Could not load file, the file is not found");
			}

			byte[] data = file.readAllBytes();
			ByteBuffer img = BufferUtils.createByteBuffer(data.length);
			img.put(data);
			img.flip();

			ByteBuffer buf;
			int width;
			int height;

			try (MemoryStack stack = MemoryStack.stackPush()) {
				IntBuffer w = stack.mallocInt(1);
				IntBuffer h = stack.mallocInt(1);
				IntBuffer channels = stack.mallocInt(1);

				buf = stbi_load_from_memory(img, w, h, channels, 4);
				if (buf == null) {
					throw new ResourceLoadingException("Image not loaded: " + stbi_failure_reason());
				}

				width = w.get();
				height = h.get();
			}

			return new ImageData(width, height, buf);
		} catch (IOException e) {
			throw new ResourceLoadingException();
		}
	}

	@Override
	public BufferedImage loadBufferedImage(ResourceIdentifier path) throws ResourceLoadingException {
		try {
			Path pathO = path.toPath();
			InputStream file = this.getClass().getClassLoader().getResourceAsStream(pathO.toString());
			if (file == null) {
				throw new IOException("Could not load file, the file is not found");
			}
			return ImageIO.read(file);
		} catch (IOException e) {
			throw new ResourceLoadingException();
		}
	}

	@Override
	public ByteBuffer fileToBuffer(ResourceIdentifier path) throws ResourceLoadingException {
		try {
			Path pathO = path.toPath();
			InputStream file = this.getClass().getClassLoader().getResourceAsStream(pathO.toString());
			if (file == null) {
				throw new IOException("Could not load file, the file is not found");
			}

			byte[] data = file.readAllBytes();
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
		try {
			Path pathO = path.toPath();
			URL file = this.getClass().getClassLoader().getResource(pathO.toString());
			if (file == null) {
				throw new IOException("Could not load file, the file is not found");
			}
			return new File(file.toURI());
		} catch (IOException | URISyntaxException e) {
			throw new ResourceLoadingException();
		}
	}
}
