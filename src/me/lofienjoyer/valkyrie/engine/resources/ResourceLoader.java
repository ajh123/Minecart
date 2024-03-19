package me.lofienjoyer.valkyrie.engine.resources;

import me.lofienjoyer.valkyrie.engine.graphics.shaders.Shader;
import uk.minersonline.Minecart.resource.ResourceIdentifier;
import uk.minersonline.Minecart.resource.ResourceManager;
import uk.minersonline.Minecart.resource.ResourceLoadingException;

import java.io.File;
import java.io.IOException;
import java.util.Scanner;

public class ResourceLoader {

    public static Shader loadShader(String name, ResourceIdentifier vertexFile, ResourceIdentifier fragmentFile) {
        var vertexSource = loadStringFromFile(vertexFile);
        var fragmentSource = loadStringFromFile(fragmentFile);

        return new Shader(name, vertexSource, fragmentSource);
    }

    public static String loadStringFromFile(ResourceIdentifier fileName) {
        try {
            File file = ResourceManager.loadFile(fileName);
			return new Scanner(file).useDelimiter("\\Z").next();
        } catch (IOException | ResourceLoadingException e) {
            throw new RuntimeException(e);
        }
    }

}
