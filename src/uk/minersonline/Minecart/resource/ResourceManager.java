package uk.minersonline.Minecart.resource;

import me.lofienjoyer.valkyrie.engine.graphics.texture.ImageData;

import java.awt.image.BufferedImage;
import java.io.File;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

public class ResourceManager {
	private static final List<ResourceLoader> resourceLoaders = new ArrayList<>(List.of(
			new FileSystemResourceLoader(),
			new ClassLoaderResourceLoader()
	));

	@SuppressWarnings({"ReassignedVariable", "DataFlowIssue"})
	public static ImageData loadImage(ResourceIdentifier path) throws ResourceLoadingException {
		ImageData image = null;
		boolean nextLoader = true;
		ResourceLoadingException cause = null;
		for (ResourceLoader loader : resourceLoaders) {
			if (nextLoader) {
				try {
					image = loader.loadImage(path);
					nextLoader = false;
				} catch (ResourceLoadingException e) {
					nextLoader = true;
					// ^^^ IMPORTANT: set to true if there was an exception, so the next loader in the queue is used.
					// this is the reason why `@SuppressWarnings({"ReassignedVariable", "DataFlowIssue"})` is used.
					cause = e;
				}
			}
		}
		if (image == null) {
			if (cause != null) {
				throw new ResourceLoadingException("Could not load the image [" + path + "] no ResourceLoaders have found it.", cause);
			} else {
				throw new ResourceLoadingException("Could not load the image [" + path + "] no ResourceLoaders have found it.");
			}
		}
		return image;
	}

	@SuppressWarnings({"ReassignedVariable", "DataFlowIssue"})
	public static BufferedImage loadBufferedImage(ResourceIdentifier path) throws ResourceLoadingException {
		BufferedImage image = null;
		boolean nextLoader = true;
		ResourceLoadingException cause = null;
		for (ResourceLoader loader : resourceLoaders) {
			if (nextLoader) {
				try {
					image = loader.loadBufferedImage(path);
					nextLoader = false;
				} catch (ResourceLoadingException e) {
					nextLoader = true;
					// ^^^ IMPORTANT: set to true if there was an exception, so the next loader in the queue is used.
					// this is the reason why `@SuppressWarnings({"ReassignedVariable", "DataFlowIssue"})` is used.
					cause = e;
				}
			}
		}
		if (image == null) {
			if (cause != null) {
				throw new ResourceLoadingException("Could not load the image [" + path + "] no ResourceLoaders have found it.", cause);
			} else {
				throw new ResourceLoadingException("Could not load the image [" + path + "] no ResourceLoaders have found it.");
			}
		}
		return image;
	}

	@SuppressWarnings({"ReassignedVariable", "DataFlowIssue"})
	public static ByteBuffer fileToBuffer(ResourceIdentifier path)  throws ResourceLoadingException {
		ByteBuffer file = null;
		boolean nextLoader = true;
		ResourceLoadingException cause = null;
		for (ResourceLoader loader : resourceLoaders) {
			if (nextLoader) {
				try {
					file = loader.fileToBuffer(path);
					nextLoader = false;
				} catch (ResourceLoadingException e) {
					nextLoader = true;
					// ^^^ IMPORTANT: set to true if there was an exception, so the next loader in the queue is used.
					// this is the reason why `@SuppressWarnings({"ReassignedVariable", "DataFlowIssue"})` is used.
					cause = e;
				}
			}
		}
		if (file == null) {
			if (cause != null) {
				throw new ResourceLoadingException("Could not load the file [" + path + "] no ResourceLoaders have found it.", cause);
			} else {
				throw new ResourceLoadingException("Could not load the file [" + path + "] no ResourceLoaders have found it.");
			}
		}
		return file;
	}

	@SuppressWarnings({"ReassignedVariable", "DataFlowIssue"})
	public static File loadFile(ResourceIdentifier path)  throws ResourceLoadingException {
		File file = null;
		boolean nextLoader = true;
		ResourceLoadingException cause = null;
		for (ResourceLoader loader : resourceLoaders) {
			if (nextLoader) {
				try {
					file = loader.loadFile(path);
					nextLoader = false;
				} catch (ResourceLoadingException e) {
					nextLoader = true;
					// ^^^ IMPORTANT: set to true if there was an exception, so the next loader in the queue is used.
					// this is the reason why `@SuppressWarnings({"ReassignedVariable", "DataFlowIssue"})` is used.
					cause = e;
				}
			}
		}
		if (file == null) {
			if (cause != null) {
				throw new ResourceLoadingException("Could not load the file [" + path + "] no ResourceLoaders have found it.", cause);
			} else {
				throw new ResourceLoadingException("Could not load the file [" + path + "] no ResourceLoaders have found it.");
			}
		}
		return file;
	}
}
