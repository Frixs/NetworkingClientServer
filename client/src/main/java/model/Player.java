package main.java.model;

import java.awt.*;

public class Player {
    private String id;
    private String nickname;
    private Color color;

    public Player(String id, String nickname, Color color) {
        this.id = id;
        this.nickname = nickname;
        this.color = color;
    }

    public String getId() {
        return id;
    }

    public String getNickname() {
        return nickname;
    }

    public Color getColor() {
        return color;
    }
}
