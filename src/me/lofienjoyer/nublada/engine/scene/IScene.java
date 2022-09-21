package me.lofienjoyer.nublada.engine.scene;

public interface IScene {

    void init();

    void render(float delta);

    void dispose();

    void onResize(int width, int height );

    void onClose();

}