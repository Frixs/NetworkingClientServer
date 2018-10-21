package main.java.controller;

import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.layout.AnchorPane;
import javafx.scene.layout.Pane;
import javafx.scene.layout.StackPane;
import main.java.model.WindowContent;

import java.io.IOException;

/**
 * Created by Frixs on 16.10.2018.
 */
public abstract class AWindowController {

    @FXML
    protected AnchorPane rootPane;

    @FXML
    protected StackPane rootContentPane;

    public AnchorPane getRootPane() {
        return rootPane;
    }

    public StackPane getRootContentPane() {
        return rootContentPane;
    }

    /**
     * Load new content in the main window.
     *
     * @param contentType           Which content you want to add.
     */
    public void loadContent(WindowContent contentType) {
        Pane pane = null;
        FXMLLoader fxmlLoader = null;

        try {
            switch (contentType) {
                case CONNECTION_FORM:
                    fxmlLoader = new FXMLLoader(getClass().getResource("/main/resource/view/ConnectionForm.fxml"));
                    pane = fxmlLoader.load();
                    ((ConnectionFormController) fxmlLoader.getController()).setWindowController(this);
                    break;
                case LOBBY:
                    fxmlLoader = new FXMLLoader(getClass().getResource("/main/resource/view/Lobby.fxml"));
                    pane = fxmlLoader.load();
                    ((LobbyController) fxmlLoader.getController()).setWindowController(this);
                    break;
                case GAME:
                    fxmlLoader = new FXMLLoader(getClass().getResource("/main/resource/view/Game.fxml"));
                    pane = fxmlLoader.load();
                    ((GameController) fxmlLoader.getController()).setWindowController(this);
                    break;
                default:
                    ;
            }

            rootContentPane.getChildren().setAll(pane);
        } catch (IOException e) {
            e.printStackTrace();
        }

        // Run after initialization process.
        ((AContentController) fxmlLoader.getController()).afterInitialize();
    }
}
