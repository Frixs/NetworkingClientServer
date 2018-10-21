package main.java.controller;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.ListView;

/**
 * Created by Frixs on 16.10.2018.
 */
public class LobbyController extends AContentController {

    @FXML
    private ListView<?> gameListLW;

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

    }

    @FXML
    void onActionDisconnectBtn(ActionEvent event) {

    }
}
