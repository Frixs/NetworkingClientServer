package main.java.controller;

import javafx.animation.PauseTransition;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.control.TextField;
import javafx.scene.layout.StackPane;
import javafx.scene.text.Text;
import javafx.util.Duration;
import main.java.core.App;
import main.java.model.Client;
import main.java.listener.change.TextFieldMaxLength;
import main.java.model.Sanitize;
import main.java.model.WindowContent;

import java.util.ArrayList;

/**
 * Created by Frixs on 16.10.2018.
 */
public class ConnectionFormController extends AContentController {

    @FXML
    private StackPane connectionLoaderPane;

    @FXML
    private StackPane formPane;

    @FXML
    private Text inputErrorMessageT;

    @FXML
    private TextField nicknameTF;

    @FXML
    private TextField serverAddressTF;

    @FXML
    private TextField portTF;

    /**
     * no-args constructor
     */
    public ConnectionFormController() {
    }

    /**
     * Initialization
     */
    @FXML
    private void initialize() {
        nicknameTF.textProperty().addListener(new TextFieldMaxLength(nicknameTF, 19));
        serverAddressTF.textProperty().addListener(new TextFieldMaxLength(serverAddressTF, 15));
        portTF.textProperty().addListener(new TextFieldMaxLength(portTF, 5));

        if (Client.SELF.nickname != null)
            nicknameTF.setText(Client.SELF.nickname);
        if (Client.SELF.hostAddress != null)
            serverAddressTF.setText(Client.SELF.hostAddress);
        if (Client.SELF.port > 0)
            portTF.setText("" + Client.SELF.port);
    }

    @Override
    public void afterInitialize() {
    }

    @FXML
    void OnActionExitBtn(ActionEvent event) {
        App.shutdown();
    }

    @FXML
    void OnActionSubmitBtn(ActionEvent event) {
        inputErrorMessageT.setVisible(false);
        startConnecting();
    }

    /**
     * Process before calling client connection.
     */
    private void startConnecting() {
        int port;
        int overlayTime = 3;

        // Basic input validation for immediate feedback.
        if (nicknameTF.getText().trim().length() == 0 || serverAddressTF.getText().trim().length() == 0 || portTF.getText().trim().length() == 0) {
            inputErrorMessageT.setText("Wrong inputs!");
            inputErrorMessageT.setVisible(true);
            return;
        }

        try {
            port = Integer.parseInt(portTF.getText().trim());
        } catch (NumberFormatException e) {
            inputErrorMessageT.setText("Entered port is not a number!");
            inputErrorMessageT.setVisible(true);
            return;
        }

        if (!Sanitize.SELF.checkString(nicknameTF.getText().trim(), true, true, true, new ArrayList<>())) {
            inputErrorMessageT.setText("Only letters and numbers are allowed in your nickname!");
            inputErrorMessageT.setVisible(true);
            return;
        }

        // Start connection animation.
        formPane.setVisible(false);
        connectionLoaderPane.setVisible(true);

        // Start connection after animation time.
        PauseTransition delayOverlay = new PauseTransition(Duration.seconds(overlayTime));
        delayOverlay.setOnFinished(
                // Client connection call.
                event -> {
                    int connStatus = Client.SELF.connect(serverAddressTF.getText().trim(), port, nicknameTF.getText().trim());

                    if (connStatus == 0) { // Successful connection.
                        getWindowController().loadContent(WindowContent.LOBBY);

                    } else { // Handle error.
                        switch (connStatus) {
                            case 2:
                                inputErrorMessageT.setText("Wrong inputs!");
                            case 3:
                                inputErrorMessageT.setText("An unspecified ERROR occurred while connecting!");
                            default:
                                inputErrorMessageT.setText("Cannot connect to the server!");
                        }

                        inputErrorMessageT.setVisible(true);

                        connectionLoaderPane.setVisible(false);
                        formPane.setVisible(true);
                    }
                }
        );
        delayOverlay.play();
    }
}
