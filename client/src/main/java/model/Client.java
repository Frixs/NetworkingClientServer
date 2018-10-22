package main.java.model;

import javafx.application.Platform;
import javafx.scene.control.Alert;
import javafx.scene.control.ButtonType;
import main.java.controller.AWindowController;
import main.java.controller.GameController;
import main.java.controller.LobbyController;

import java.awt.*;
import java.io.*;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.util.ArrayList;

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

    private Client() {
        reader = new Thread(this);
    }

    @Override
    public int connect(String ihost, int iport, String nickname) {
        if (ihost.trim().length() == 0 || nickname.trim().length() == 0)
            return 2;

        if (!Sanitize.SELF.checkString(nickname.trim(), true, true, true, new ArrayList<>()))
            return 2;

        this.port = iport;
        this.hostAddress = ihost;
        if (ihost.trim().length() > 15)
            this.hostAddress = ihost.trim().substring(0, 15);
        this.nickname = nickname;
        if (nickname.trim().length() > 20)
            this.nickname = nickname.trim().substring(0, 20);

        // Create a new socket.
        try {
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
        sendMessage(new Message("1;_player_nickname;" + this.nickname)); // Token message.

        try {
            String message;

            if ((message = in.readLine()) != null) {
                String[] tokens = message.split(";");

                if (tokens.length > 1 && tokens[1].compareTo("_player_id") == 0)
                    this.id = tokens[0].trim();
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
        } catch (IOException e) {
            System.out.println("ERROR: Unable to close streams.");
        } finally {
            try {
                if (this.socket != null)
                    this.socket.close();
            } catch (IOException e) {
                System.out.println("ERROR: Unable to close socket.");
            }
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
                    }
                    timeout++;

                } catch (SocketTimeoutException e) {
                    sendMessage(new Message("get_games")); // Token message.
                }
            } while (timeout < 2);

            mainWindowController.loadContent(WindowContent.CONNECTION_FORM);
            Alert alert = new Alert(Alert.AlertType.ERROR, "Disconnected from the server!", ButtonType.OK);
            alert.showAndWait();
//                    .filter(response -> response == ButtonType.OK)
//                    .ifPresent(response -> System.out.println("hey!"));

        } catch (IOException e) {
            mainWindowController.loadContent(WindowContent.CONNECTION_FORM);
            Alert alert = new Alert(Alert.AlertType.ERROR, "Unspecified ERROR occurred on the server!", ButtonType.OK);
            alert.showAndWait();
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
                case "update_players":
                    // TODO;
                    break;
                case "update_games":
                    Platform.runLater(() -> reqUpdateGames(tokens));
                    break;
                case "prepare_player_to_game":
                    Platform.runLater(() -> reqPreparePlayerToGame(tokens[0], nickname, tokens[2], tokens[3]));
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
     * REQ: Set player to be able to play the game.
     * @param id            Player ID.
     * @param nickname      Player nickname.
     * @param color         Player color
     * @param gameId        Game which player joined.
     */
    private void reqPreparePlayerToGame(String id, String nickname, String color, String gameId) {
        Color c;

        try {
            c = Color.decode("#" + color);
        } catch (NumberFormatException e) {
            System.out.println("ERROR occurred!");
            e.printStackTrace();
            return;
        }

        this.gameId = gameId;

        Player p = new Player(id, nickname, c);

        ((GameController) mainWindowController.loadContent(WindowContent.GAME)).setPlayer(p);
    }

    /**
     * REQ: Update game list view in the lobby menu.
     * @param tokens    The tokens.
     */
    private void reqUpdateGames(String[] tokens) {
        if (!(mainWindowController.getCurrentContentController() instanceof LobbyController))
            return;

        ((LobbyController) mainWindowController.getCurrentContentController()).getGameListLV().getItems().clear();

        for (int i = 2; (i + 1) < tokens.length; i+= 2) {
            tokens[i] = tokens[i].trim();
            tokens[i + 1] = tokens[i + 1].trim();
            ((LobbyController) mainWindowController.getCurrentContentController()).getGameListLV().getItems().add(new Game(tokens[i + 1], tokens[i]).toString());
        }

        timeout = 0;
    }
}

