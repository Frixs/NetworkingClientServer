package main.java.model;

import javafx.scene.control.Alert;
import javafx.scene.control.ButtonType;
import main.java.controller.AWindowController;

import java.io.*;
import java.net.InetAddress;
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
     * NUmber of missed timeouts.
     */
    public int timeout = 0;

    private Client() {
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
            this.socket.setSoTimeout(60000);

            InetAddress address = this.socket.getInetAddress();
            System.out.println("Connecting to server (" + address.getHostAddress() + ") as " + nickname + ".");

            out = new PrintWriter(new OutputStreamWriter(this.socket.getOutputStream()));
            in = new BufferedReader(new InputStreamReader(this.socket.getInputStream()));

        } catch (Exception e) {
            this.socket = null;
        }

        if (this.socket == null)
            return 1;

        // Send client nickname to server to be able to identify the client.
        sendMessage(new Message("1;nickname;" + this.nickname));

        try {
            String message;

            if ((message = in.readLine()) != null) {
                String[] tokens = message.split(";");

                if (tokens.length > 1 && tokens[1].compareTo("id") == 0)
                    this.id = tokens[0].trim();
                else
                    return 3;
            }

            sendMessage(new Message("show_games"));

        } catch (Exception e) {
            return 3;
        }

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

    @Override
    public Message receiveMessage() {
        String line;

        // Read the response (a line) from the server.
        try {
            line = in.readLine();
            return new Message(line);
        } catch (IOException e) {
            System.err.println("ERROR: Read!");
        }

        return null;
    }

    /**
     * Receive messages from the server.
     */
    @Override
    public void run() {
        String message = new String();

        try {
            do {
                try {
                    while ((message = in.readLine()) != null) {
                        processMessage(message);
                        message = new String();
                    }
                    timeout++;

                } catch (SocketTimeoutException e) {
                    sendMessage(new Message("games"));
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
            // List of events which client accepts from server side.
            switch (tokens[1]) {
                case "games":
                    reqListOfGames(tokens);
                    break;
                default:
                    System.out.println("ERROR occurred!");
                    System.out.println("Received message does not fit the format!");
            }
        } else {
            System.out.println("ERROR occurred!");
            System.out.println("Received message does not fit the format!");
        }
    }

    /**
     * REQ: TODO
     * @param tokens
     */
    private void reqListOfGames(String[] tokens) {
    }
}

