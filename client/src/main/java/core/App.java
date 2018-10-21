package main.java.core;

import javafx.application.Application;
import javafx.application.Platform;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;

public class App extends Application {

    /**
     * What to do if application was closed
     */
    public static void shutdown() {
        Platform.exit();
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

        primaryStage.setTitle("Rock Paper Scissors");
        primaryStage.setScene(scene);

        primaryStage.setResizable(false);
        primaryStage.setMinWidth(640);
        primaryStage.setMinHeight(480);
        primaryStage.setMaxWidth(640);
        primaryStage.setMaxHeight(480);

        primaryStage.setOnCloseRequest(e -> App.shutdown());

        primaryStage.show();
    }

    public static void main(String[] args) {
        launch(args);
    }
}
