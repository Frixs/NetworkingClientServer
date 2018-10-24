package main.java.controller;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.ListView;
import javafx.scene.control.TextField;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.input.MouseEvent;
import main.java.listener.change.TextFieldMaxLength;
import main.java.model.Client;
import main.java.model.Game;
import main.java.model.Message;

/**
 * Created by Frixs on 16.10.2018.
 */
public class LobbyController extends AContentController {

    @FXML
    private ListView<String> gameListLV;

    @FXML
    private TextField goalTF;

    public TextField getGoalTF() {
        return goalTF;
    }

    public ListView<String> getGameListLV() {
        return gameListLV;
    }

    /**
     * no-args constructor
     */
    public LobbyController() {
    }

    /**
     * Initialization
     */
    @FXML
    private void initialize() {
        goalTF.textProperty().addListener(new TextFieldMaxLength(goalTF, 2));
    }

    @Override
    public void afterInitialize() {
    }

    /**
     * Join the game.
     * @param game  The game.
     */
    private void joinTheGame(Game game) {
        // TODO;
        System.out.println("TODO");
    }

    @FXML
    void onActionNewGameBtn(ActionEvent event) {
        int goal = 0;

        try {
            goal = Integer.parseInt(goalTF.getText().trim());

        } catch (NumberFormatException ignored) {
            ;
        }

        Client.SELF.sendMessage(new Message("create_new_game;"+ goal)); // Token message.
    }

    @FXML
    void onActionDisconnectBtn(ActionEvent event) {
        Client.SELF.sendMessage(new Message("disconnect_player"));
    }

    @FXML
    void onKeyReleasedItem(KeyEvent event) {
        if (event.getCode() != KeyCode.SPACE)
            return;

        if (gameListLV.getSelectionModel().getSelectedItem() == null)
            return;

        handleItemActionLV();
    }

    @FXML
    void onMouseClickedItem(MouseEvent event) {
        if (gameListLV.getSelectionModel().getSelectedItem() == null)
            return;

        handleItemActionLV();
    }

    /**
     * Handle action event on item in game list view.
     */
    private void handleItemActionLV() {
        Game g = null;
        String gameId = gameListLV.getSelectionModel().getSelectedItem().split(" [(]")[0].split("[-]")[1];
        String gameName = gameListLV.getSelectionModel().getSelectedItem().split(" [(]")[0];
        String gameGoal = gameListLV.getSelectionModel().getSelectedItem().split("[:] ")[1].split("[)]")[0];

        try {
            g = new Game(gameId, gameName, Integer.parseInt(gameGoal));

        } catch (NumberFormatException e) {
            System.out.println("ERROR occurred!");
            System.out.println("Cannot decode game information!");
            e.printStackTrace();
            return;
        }

        joinTheGame(g);
    }
}
