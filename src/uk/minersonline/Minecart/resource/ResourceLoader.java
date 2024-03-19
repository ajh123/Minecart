package uk.minersonline.Minecart.resource;

import me.lofienjoyer.valkyrie.engine.graphics.texture.ImageData;

import java.awt.image.BufferedImage;
import java.io.File;
import java.nio.ByteBuffer;

public interface ResourceLoader {
	ImageData loadImage(ResourceIdentifier path) throws ResourceLoadingException;
	BufferedImage loadBufferedImage(ResourceIdentifier path) throws ResourceLoadingException;
	ByteBuffer fileToBuffer(ResourceIdentifier path) throws ResourceLoadingException;
	File loadFile(ResourceIdentifier path) throws ResourceLoadingException;
}
