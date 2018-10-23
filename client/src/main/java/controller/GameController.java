package main.java.controller;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.image.ImageView;
import javafx.scene.layout.StackPane;
import javafx.scene.text.Text;
import main.java.model.Game;
import main.java.model.Player;

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
    private ImageView p2ChoiceIW;

    @FXML
    private ImageView p1ChoiceIW;

    @FXML
    private StackPane yourTurnPane;

    @FXML
    private StackPane anotherPlayerTurnPane;

    /**
     * Players who are set in the game.
     */
    private Player[] playerList = null;

    /**
     * no-args constructor
     */
    public GameController() {
         playerList = new Player[2];
         playerList[0] = null;
         playerList[1] = null;
    }

    /**
     * Initialization
     */
    @FXML
    private void initialize() {
    }

    @Override
    public void afterInitialize() {

    }

    /**
     * Set player to GUI.
     * @param p     The player.
     * @param g     The game.
     */
    public void setWindow(Player p, Game g) {
        int pIndex;

        if (playerList[0] == null) {
            pIndex = 0;
        } else if (playerList[1] == null) {
            pIndex = 1;
        } else {
            return;
        }



        // Set GUI.
        if (pIndex == 0) {

        } else {

        }
    }

    /**
     * Unset player from GUI.
     * @param p     The player.
     */
    public void unsetWindow(Player p) {
        // TODO;
        System.out.println("TEST");
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
