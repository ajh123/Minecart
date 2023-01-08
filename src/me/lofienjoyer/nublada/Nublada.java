package me.lofienjoyer.nublada;

import me.lofienjoyer.nublada.engine.graphics.mesh.QuadMesh;
import me.lofienjoyer.nublada.engine.graphics.shaders.FboShader;
import me.lofienjoyer.nublada.engine.scene.IScene;
import me.lofienjoyer.nublada.engine.graphics.display.Window;
import me.lofienjoyer.nublada.engine.graphics.loader.Loader;
import me.lofienjoyer.nublada.engine.log.NubladaLogHandler;
import me.lofienjoyer.nublada.engine.world.BlockRegistry;
import org.lwjgl.glfw.GLFW;
import org.lwjgl.opengl.GL;
import org.lwjgl.opengl.GL11;
import org.lwjgl.opengl.GL30;

import java.util.logging.Level;
import java.util.logging.Logger;

public class Nublada {

    public static final Logger LOG = NubladaLogHandler.initLogs();
    public static final Loader LOADER = new Loader();

    private final Window window;
    public static long WINDOW_ID;

    private IScene currentScene;

    public Nublada() {
        LOG.setLevel(Level.INFO);

        // FIXME: 09/01/2022 Make this customizable
        this.window = new Window(1280, 720, "Nublada");

        WINDOW_ID = window.getId();
    }

    public void init() {
        GL.createCapabilities();

        BlockRegistry.setup();

        window.setClearColor(0.45f, 0.71f, 1.00f, 1f);

        window.show();
    }

    public void loop() {
        long timer = System.nanoTime();
        float delta = 1f;

        int framebuffer = GL30.glGenFramebuffers();
        GL30.glBindFramebuffer(GL30.GL_FRAMEBUFFER, framebuffer);

        int textureColorBuffer = GL30.glGenTextures();
        GL30.glBindTexture(GL30.GL_TEXTURE_2D, textureColorBuffer);
        GL30.glTexImage2D(GL30.GL_TEXTURE_2D, 0, GL30.GL_RGB, 640, 360, 0, GL30.GL_RGB, GL30.GL_UNSIGNED_BYTE, 0);
        GL30.glTexParameteri(GL30.GL_TEXTURE_2D, GL30.GL_TEXTURE_MIN_FILTER, GL30.GL_NEAREST);
        GL30.glTexParameteri(GL30.GL_TEXTURE_2D, GL30.GL_TEXTURE_MAG_FILTER, GL30.GL_NEAREST);
        GL30.glFramebufferTexture2D(GL30.GL_FRAMEBUFFER, GL30.GL_COLOR_ATTACHMENT0, GL30.GL_TEXTURE_2D, textureColorBuffer, 0);
        GL30.glBindTexture(GL30.GL_TEXTURE_2D, 0);

        int rbo = GL30.glGenRenderbuffers();
        GL30.glBindRenderbuffer(GL30.GL_RENDERBUFFER, rbo);
        GL30.glRenderbufferStorage(GL30.GL_RENDERBUFFER, GL30.GL_DEPTH24_STENCIL8, 640, 360);
        GL30.glFramebufferRenderbuffer(GL30.GL_FRAMEBUFFER, GL30.GL_DEPTH_STENCIL_ATTACHMENT, GL30.GL_RENDERBUFFER, rbo);
        GL30.glBindRenderbuffer(GL30.GL_RENDERBUFFER, 0);

        if (GL30.glCheckFramebufferStatus(GL30.GL_FRAMEBUFFER) != GL30.GL_FRAMEBUFFER_COMPLETE) {
            Nublada.LOG.severe("Error while creating framebuffer!");
        }

        FboShader shader = new FboShader();
        QuadMesh quadMesh = new QuadMesh();

        while (window.keepOpen()) {
            GL30.glBindFramebuffer(GL30.GL_FRAMEBUFFER, framebuffer);
            GL30.glClearColor(0.125f, 0f, 1.0f, 0.5f);
            GL30.glClear(GL11.GL_COLOR_BUFFER_BIT | GL11.GL_DEPTH_BUFFER_BIT);
            GL30.glEnable(GL11.GL_DEPTH_TEST);
            GL30.glViewport(0, 0, 640, 360);

            if (currentScene != null)
                currentScene.render(delta);

            GL30.glBindFramebuffer(GL30.GL_FRAMEBUFFER, 0);
            GL30.glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            GL30.glClear(GL30.GL_COLOR_BUFFER_BIT);

            shader.start();

            GL30.glViewport(0, 0, window.getWidth(), window.getHeight());
            GL30.glBindVertexArray(quadMesh.getVaoId());
            GL30.glDisable(GL11.GL_DEPTH_TEST);
            GL30.glBindTexture(GL11.GL_TEXTURE_2D, textureColorBuffer);
            GL30.glDrawArrays(GL11.GL_TRIANGLES, 0, 6);

            window.update();

            delta = (System.nanoTime() - timer) / 1000000000f;
            timer = System.nanoTime();

            GLFW.glfwSetWindowTitle(window.getId(), "Nublada | FPS: " + (int) (1f / delta) + " (delta: " + delta + "s)");
        }

        currentScene.onClose();
    }

    public void setCurrentScene(IScene scene) {
        if (currentScene != null)
            currentScene.dispose();
        scene.init();
        this.currentScene = scene;
        window.setResizeCallback(scene::onResize);
    }

    public void dispose() {
        LOADER.dispose();
    }

}
