package main.java.controller;

import javafx.fxml.FXML;
import main.java.model.Client;
import main.java.model.WindowContent;

public class MainWindowController extends AWindowController {

    /**
     * no-args constructor
     */
    public MainWindowController() {
    }

    /**
     * Initialization
     */
    @FXML
    private void initialize() {
        loadContent(WindowContent.CONNECTION_FORM);
        Client.SELF.mainWindowController = this;
    }
}
