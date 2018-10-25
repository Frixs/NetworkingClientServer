package main.java.controller;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.image.ImageView;
import javafx.scene.layout.StackPane;
import javafx.scene.paint.Color;
import javafx.scene.text.Text;
import main.java.model.Game;
import main.java.model.Player;

import java.util.ArrayList;

/**
 * Created by Frixs on 16.10.2018.
 */
public class GameController extends AContentController {

    @FXML
    private Text gameNameT;

    @FXML
    private Text scoreT;

    @FXML
    private Text goalT;

    @FXML
    private Text p1NicknameT;

    @FXML
    private Text p2NicknameT;

    @FXML
    private ImageView p2ChoiceIV;

    @FXML
    private ImageView p1ChoiceIV;

    @FXML
    private StackPane yourTurnPane;

    @FXML
    private StackPane anotherPlayerTurnPane;

    private ArrayList<Text> pNicknameTList = null;
    private ArrayList<ImageView> pChoiceIVList = null;

    /**
     * no-args constructor
     */
    public GameController() {
    }

    /**
     * Initialization
     */
    @FXML
    private void initialize() {
        pNicknameTList = new ArrayList<>();
        pNicknameTList.add(p1NicknameT);
        pNicknameTList.add(p2NicknameT);

        pChoiceIVList = new ArrayList<>();
        pChoiceIVList.add(p1ChoiceIV);
        pChoiceIVList.add(p2ChoiceIV);
    }

    @Override
    public void afterInitialize() {

    }

    /**
     * Set player to GUI.
     *
     * @param g The game.
     */
    public void setWindow(Game g) {
        p1NicknameT.setText("- - -");
        p1ChoiceIV.setImage(null);
        p2NicknameT.setText("- - -");
        p2ChoiceIV.setImage(null);
        gameNameT.setText(g.getName());
        goalT.setText("Goal: " + g.getGoal());
        scoreT.setText("0 : 0");

        yourTurnPane.setVisible(false);
        anotherPlayerTurnPane.setVisible(true);
    }

    /**
     * Update window information about players.
     *
     * @param list Player list.
     */
    public void updatePlayers(ArrayList<Player> list) {
        int i;

        // Set default first.
        p1NicknameT.setText("- - -");
        p1NicknameT.setFill(Color.valueOf("707070"));
        p2NicknameT.setText("- - -");
        p2NicknameT.setFill(Color.valueOf("707070"));

        // Set UI by player values.
        for (i = 0; i < pNicknameTList.size(); i++) {
            if (list.size() <= i)
                break;

            pNicknameTList.get(i).setText(list.get(i).getNickname());
            pNicknameTList.get(i).setFill(list.get(i).getColor());
        }

        if (list.size() > pNicknameTList.size()) {
            System.out.println("ERROR occurred!");
            System.out.println("There are more players in the game than expected!");
        }
    }

    @FXML
    void onActionChoosePaperBtn(ActionEvent event) {

    }

    @FXML
    void onActionChooseRockBtn(ActionEvent event) {

    }

    @FXML
    void onActionChooseScissorsBtn(ActionEvent event) {

    }

    @FXML
    void onActionLeaveBtn(ActionEvent event) {

    }

    @FXML
    void onActionQuitBtn(ActionEvent event) {

    }
}
