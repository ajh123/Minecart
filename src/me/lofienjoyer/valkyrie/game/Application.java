package me.lofienjoyer.valkyrie.game;

import me.lofienjoyer.valkyrie.Valkyrie;

import java.util.List;

import static uk.minersonline.Minecart.resource.ResourceIdentifier.DEFAULT_NAMESPACE;

public class Application {

    public static void main(String[] args) {
        Valkyrie instance = new Valkyrie();
        instance.init(List.of(DEFAULT_NAMESPACE));
        instance.setCurrentScene(new WorldScene());
        instance.loop();
        instance.dispose();
    }

}
