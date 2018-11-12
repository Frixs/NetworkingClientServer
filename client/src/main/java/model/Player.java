package main.java.model;


import javafx.scene.paint.Color;

public class Player {
    private String id;
    private String nickname;
    private Color color;
    private int score;
    private int choice;

    public Player(String id, String nickname, Color color, int score, int choice) {
        this.id = id;
        this.nickname = nickname;
        this.color = color;
        this.score = score;
        this.choice = choice;
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

    public int getScore() {
        return score;
    }

    public int getChoice() {
        return choice;
    }
}
