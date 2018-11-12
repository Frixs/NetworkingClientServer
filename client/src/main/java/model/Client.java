package main.java.model;

import javafx.animation.PauseTransition;
import javafx.application.Platform;
import javafx.scene.control.Alert;
import javafx.scene.control.ButtonType;
import javafx.scene.paint.Color;
import javafx.util.Duration;
import main.java.controller.AWindowController;
import main.java.controller.GameController;
import main.java.controller.LobbyController;
import main.java.core.App;

import java.io.*;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.util.ArrayList;

import static java.lang.Thread.sleep;

/**
 * Created by Frixs on 17.10.2018.
 */
public class Client implements INetwork, Runnable {
    /**
     * Static variable referring to this singleton class.
     */
    public static final Client SELF = new Client();

    /** Reference to the main window controller. */
    public AWindowController mainWindowController = null;

    /* Client info. variables. */
    public String id;
    public String nickname;
    public String gameId;

    /**
     * Socket on which communication runs.
     */
    public Socket socket;
    /**
     * Host address.
     */
    public String hostAddress;
    /**
     * Host port number.
     */
    public int port;
    /**
     * Outgoing connection stream.
     */
    public PrintWriter out;
    /**
     * Incoming connection stream.
     */
    public BufferedReader in;
    /**
     * Reading thread.
     */
    private Thread reader = null;
    /**
     * NUmber of missed timeouts.
     */
    public int timeout = 0;

    /**
     * Says, if reader thread should stop the receiving cycle - method Run().
     * ----------
     * Manual use:
     * ---
     * We want to terminate the thread before the thread will go to wait to another message from the server again.
     * If we did not do that, the client would try to read some message until timeout and then alert us to an unexpected server error.
     * Because the disconnect method nulls "in" and "out" parameter while the thread is still trying to receive message.
     */
    private boolean stopReceiving = false;

    public Thread getReader() {
        return reader;
    }

    /**
     * Constructor.
     */
    private Client() {
    }

    @Override
    public int connect(String ihost, int iport, String nickname) {
        if (ihost.trim().length() == 0 || nickname.trim().length() == 0)
            return 2;

        if (!Sanitize.SELF.checkString(nickname.trim(), true, true, true, new ArrayList<>()))
            return 2;

        stopReceiving = false;
        reader = new Thread(this);

        this.port = iport;
        this.hostAddress = ihost;
        if (ihost.trim().length() > 15)
            this.hostAddress = ihost.trim().substring(0, 15);
        this.nickname = nickname;
        if (nickname.trim().length() > 20)
            this.nickname = nickname.trim().substring(0, 20);

        // Create a new socket.
        try {
            if (this.socket == null)
                this.socket = new Socket(this.hostAddress, this.port);
            this.socket.setSoTimeout(15000); // 15 sec.

            System.out.println("Connecting to server (" + this.socket.getInetAddress().getHostAddress() + ") as " + nickname + " ...");

            out = new PrintWriter(new OutputStreamWriter(this.socket.getOutputStream()));
            in = new BufferedReader(new InputStreamReader(this.socket.getInputStream()));

        } catch (Exception e) {
            this.socket = null;
        }

        if (this.socket == null)
            return 1;

        // Send client nickname to server to be able to identify the client.
        if (this.id == null) {
            sendMessage(new Message("1;_player_nickname;" + this.nickname)); // Token message.
        } else { // If the user is trying to reconnect.
            sendMessage(new Message("_player_reconnect;" + this.nickname)); // Token message.
        }

        // Wait for server response.
        try {
            String message;

            if ((message = in.readLine()) != null) {
                String[] tokens = message.split(";");

                if (tokens.length > 1 && tokens[1].compareTo("_player_id") == 0)
                    this.id = tokens[0].trim();
                else if (tokens.length > 1 && tokens[1].compareTo("_player_id_reconnected") == 0)
                    return -1; // Reconnected.
                else
                    return 3;
            }

            sendMessage(new Message("get_games")); // Token message.

        } catch (Exception e) {
            return 3;
        }

        // Start reading messages from the server.
        reader.start();

        System.out.println("Connected to server (" + this.socket.getInetAddress().getHostAddress() + ") as " + nickname + "!");
        return 0;
    }

    @Override
    public void disconnect() {
        try {
            in.close();
            out.close();
        } catch (IOException | NullPointerException e) {
            System.out.println("ERROR: Unable to close streams.");
        } finally {
            try {
                if (this.socket != null)
                    this.socket.close();
            } catch (IOException e) {
                System.out.println("ERROR: Unable to close socket.");
            }
        }

        nullParameters();

        if (reader != null) {
            //reader.interrupt();
            stopReceiving = true;
        }
    }

    @Override
    public void sendMessage(Message msg) {
        try {
            out.println(msg.getMessage());
            out.flush();
        } catch (Exception e) {
            System.out.println("ERROR: Message not sent! (" + msg.getMessage() + ")");
        }
    }

    /**
     * Null conection parameters.
     */
    private void nullParameters() {
        this.id = null;
        this.nickname = null;
        this.hostAddress = null;
        this.port = 0;
    }

    /**
     * Receive messages from the server.
     */
    @Override
    public void run() {
        String message;

        try {
            do {
                try {
                    while ((message = in.readLine()) != null) {
                        processMessage(message);
                        message = "";

                        if (stopReceiving)
                            return;
                    }
                    timeout++;
                    sleep(5000);

                } catch (SocketTimeoutException e) {
                    sendMessage(new Message("get_games")); // Token message.
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            } while (timeout < 2);

            Platform.runLater(() -> {
                mainWindowController.loadContent(WindowContent.CONNECTION_FORM);
                Alert alert = new Alert(Alert.AlertType.ERROR, "Lost connection to the server!", ButtonType.OK);
                alert.showAndWait();
//                    .filter(response -> response == ButtonType.OK)
//                    .ifPresent(response -> System.out.println("hey!"));
            });

        } catch (IOException e) {
            e.printStackTrace();
            Platform.runLater(() -> {
                mainWindowController.loadContent(WindowContent.CONNECTION_FORM);
                Alert alert = new Alert(Alert.AlertType.ERROR, "An unexpected ERROR occurred on the server!", ButtonType.OK);
                try {
                    alert.showAndWait();

                } catch (Exception ignored) {
                    ;
                }
            });
        }
    }

    /**
     * Process a message and do specific action according to the message.
     * @param message       Processed message.
     */
    private void processMessage(String message) {
        String[] tokens = message.split(";");

        if (tokens.length > 0) {
            // Token list which is acceptable from server side.
            // List of events which client accepts from server side.
            switch (tokens[1]) {
                case "player_crash":
                    if (!tokens[0].equals(this.id))
                        break;
                    Platform.runLater(() -> reqPlayerCrash());
                    break;

                case "kick_player":
                    Platform.runLater(() -> reqKickPlayer());
                    break;

                case "game_state":
                    break;

                case "set_player_win":
                    Platform.runLater(() -> reqSetPlayerWin(tokens));
                    break;

                case "update_players":
                    Platform.runLater(() -> reqUpdatePlayers(tokens));
                    break;

                case "update_games":
                    Platform.runLater(() -> reqUpdateGames(tokens));
                    break;

                case "prepare_window_for_game":
                    Platform.runLater(() -> reqPrepareWindowForGame(tokens[2], tokens[3], tokens[4]));
                    break;

                case "disconnect_player":
                    if (!tokens[0].equals(this.id))
                        break;
                    stopReceiving = true;
                    Platform.runLater(() -> reqDisconnectPlayer());
                    break;

                case "on_turn":
                    Platform.runLater(() -> reqOnTurn());
                    break;

                case "do_after_turn":
                    Platform.runLater(() -> reqDoAfterTurn());
                    break;

                case "leave_game":
                    if (!tokens[0].equals(this.id))
                        break;
                    Platform.runLater(() -> reqLeaveGame());
                    break;

                case "cannot_join_game":
                    Platform.runLater(() -> reqCannotJoinGame());
                    break;

                default:
                    System.out.println("ERROR occurred!");
                    System.out.println("\t- Received message does not fit the format!");
                    System.out.println("\t- Message: " + message);
            }
        } else {
            System.out.println("ERROR occurred!");
            System.out.println("Received message does not fit the format!");
        }
    }

    /**
     * Kick player from the game.
     */
    private void reqKickPlayer() {
        mainWindowController.loadContent(WindowContent.LOBBY);
    }

    /**
     * Evaluate winner of the game.
     * @param tokens    The tokens.
     */
    private void reqSetPlayerWin(String[] tokens) {
        Alert alert = null;
        if (tokens[0].equals(this.id)) {
            alert = new Alert(Alert.AlertType.INFORMATION, "Congratulation! You won the game!", ButtonType.OK);
        } else {
            alert = new Alert(Alert.AlertType.INFORMATION, "Player " + tokens[2] + " won the game!", ButtonType.OK);
        }
        alert.showAndWait();
    }

    /**
     * Do after turn.
     */
    private void reqDoAfterTurn() {
    }

    /**
     * Cannot join the game. Solve it.
     */
    private void reqCannotJoinGame() {
        if ((mainWindowController.getCurrentContentController() instanceof LobbyController))
            return;

        mainWindowController.loadContent(WindowContent.LOBBY);
    }

    /**
     * Leave the game window.
     */
    private void reqLeaveGame() {
        mainWindowController.loadContent(WindowContent.LOBBY);
        this.gameId = null;
    }

    /**
     * REQ: On turn update.
     */
    private void reqOnTurn() {
        if (!(mainWindowController.getCurrentContentController() instanceof GameController))
            return;

        ((GameController) mainWindowController.getCurrentContentController()).setChoicePanel(true);
    }

    /**
     * REQ: Unexpected problem occurs on server during player serve.
     */
    private void reqPlayerCrash() {
        reqDisconnectPlayer();
    }

    /**
     * REQ: Update game GUI according to players.
     * @param tokens        The tokens.
     */
    private void reqUpdatePlayers(String[] tokens) {
        if (!(mainWindowController.getCurrentContentController() instanceof GameController))
            return;

        ArrayList<Player> list = new ArrayList<Player>();
        Color c;
        int score;
        int choice;

        for (int i = 2; (i + 1) < tokens.length; i+= 5) {
            tokens[i] = tokens[i].trim();           // id;
            tokens[i + 1] = tokens[i + 1].trim();   // nickname;
            tokens[i + 2] = tokens[i + 2].trim();   // color;
            tokens[i + 3] = tokens[i + 3].trim();   // score;
            tokens[i + 4] = tokens[i + 4].trim();   // choice;

            try {
                c = Color.valueOf(tokens[i + 2]);
                score = Integer.parseInt(tokens[i + 3]);
                choice = Integer.parseInt(tokens[i + 4]);
            } catch (Exception e) {
                System.out.println("ERROR occurred!");
                System.out.println("Cannot decode player information!");
                e.printStackTrace();
                return;
            }

            list.add(new Player(tokens[i], tokens[i + 1], c, score, choice));
        }

        ((GameController) mainWindowController.getCurrentContentController()).updatePlayers(list);
    }

    /**
     * REQ: Disconnect player on client side.
     */
    private void reqDisconnectPlayer() {
        nullParameters();

        mainWindowController.loadContent(WindowContent.CONNECTION_FORM);

        PauseTransition delayOverlay = new PauseTransition(Duration.seconds(App.REPAINT_DELAY_IN_SEC));
        delayOverlay.setOnFinished(event -> {
            disconnect();
        });
        delayOverlay.play();
    }

    /**
     * REQ: Set player to be able to play the game.
     * @param gameId        Game which player joined.
     * @param gameName      Game name.
     * @param gameGoal      Goal of the game.
     */
    private void reqPrepareWindowForGame(String gameId, String gameName, String gameGoal) {
        Game g = null;

        this.gameId = gameId;

        try {
            g = new Game(gameId, gameName, Integer.parseInt(gameGoal));
        } catch (NumberFormatException e) {
            System.out.println("ERROR occurred!");
            System.out.println("Cannot decode game information!");
            e.printStackTrace();
            return;
        }

        ((GameController) mainWindowController.loadContent(WindowContent.GAME)).setWindow(g);
    }

    /**
     * REQ: Update game list view in the lobby menu.
     * @param tokens    The tokens.
     */
    private void reqUpdateGames(String[] tokens) {
        if (!(mainWindowController.getCurrentContentController() instanceof LobbyController))
            return;

        Game g = null;

        ((LobbyController) mainWindowController.getCurrentContentController()).getGameListLV().getItems().clear();

        for (int i = 2; (i + 1) < tokens.length; i+= 3) {
            tokens[i] = tokens[i].trim();
            tokens[i + 1] = tokens[i + 1].trim();
            tokens[i + 2] = tokens[i + 2].trim();

            try {
                g = new Game(tokens[i + 1], tokens[i], Integer.parseInt(tokens[i + 2]));

            } catch (NumberFormatException e) {
                System.out.println("ERROR occurred!");
                System.out.println("Cannot decode game information!");
                e.printStackTrace();
                return;
            }

            ((LobbyController) mainWindowController.getCurrentContentController()).getGameListLV().getItems().add(g.toString());
        }

        // Reset timeout.
        timeout = 0;
    }
}

