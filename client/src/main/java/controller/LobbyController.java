package main.java.controller;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.ListView;
import main.java.model.Client;
import main.java.model.Message;

/**
 * Created by Frixs on 16.10.2018.
 */
public class LobbyController extends AContentController {

    @FXML
    private ListView<String> gameListLV;

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
    }

    @Override
    public void afterInitialize() {

    }

    @FXML
    void onActionNewGameBtn(ActionEvent event) {
        Client.SELF.sendMessage(new Message("create_new_game")); // Token message.
    }

    @FXML
    void onActionDisconnectBtn(ActionEvent event) {

    }
}
