package main.java.controller;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.image.ImageView;
import javafx.scene.layout.StackPane;
import javafx.scene.text.Text;

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
     * no-args constructor
     */
    public GameController() {
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
