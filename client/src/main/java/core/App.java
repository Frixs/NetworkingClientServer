package main.java.core;

import javafx.animation.PauseTransition;
import javafx.application.Application;
import javafx.application.Platform;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;
import javafx.util.Duration;
import main.java.model.Client;
import main.java.model.WindowContent;

import static java.lang.Thread.sleep;

public class App extends Application {

    public static final double REPAINT_DELAY_IN_SEC = 0.1;

    /**
     * What to do if application was closed
     */
    public static void shutdown() {
        Client.SELF.mainWindowController.loadContent(WindowContent.END);

        PauseTransition delayOverlay = new PauseTransition(Duration.seconds(REPAINT_DELAY_IN_SEC));
        delayOverlay.setOnFinished(event -> {
            Client.SELF.disconnect();
            Platform.exit();
        });
        delayOverlay.play();
    }

    @Override
    public void start(Stage primaryStage) throws Exception {
        Parent root = FXMLLoader.load(getClass().getResource("/main/resource/view/MainWindow.fxml"));

        Scene scene = new Scene(root, 640, 480);
        scene.getStylesheets().add("/main/resource/css/global.css");
        scene.getStylesheets().add("/main/resource/css/MainWindow.css");
        scene.getStylesheets().add("/main/resource/css/ConnectionForm.css");
        scene.getStylesheets().add("/main/resource/css/Lobby.css");
        scene.getStylesheets().add("/main/resource/css/Game.css");
        scene.getStylesheets().add("/main/resource/css/End.css");

        primaryStage.setTitle("Rock Paper Scissors");
        primaryStage.setScene(scene);

        primaryStage.setResizable(false);
        primaryStage.setMinWidth(640);
        primaryStage.setMinHeight(480);
        primaryStage.setMaxWidth(640);
        primaryStage.setMaxHeight(480);
        Platform.setImplicitExit(false);
        primaryStage.setOnCloseRequest(e -> {
            App.shutdown();
            e.consume();
        });

        primaryStage.show();
    }

    public static void main(String[] args) {
        launch(args);
    }
}
