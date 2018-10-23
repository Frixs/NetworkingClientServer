package main.java.model;

public class Game {
    private String id;
    private String name;
    private int goal;

    public Game(String id, String name, int goal) {
        this.id = id;
        this.name = name;
        this.goal = goal;
    }

    public String getId() {
        return id;
    }

    public String getName() {
        return name;
    }

    public int getGoal() {
        return goal;
    }

    @Override
    public String toString() {
        return name + " (Goal: " + goal + ")";
    }
}
