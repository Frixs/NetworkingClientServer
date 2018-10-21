package main.java.model;

/**
 * Created by Frixs on 17.10.2018.
 */
public class Message {
    private String message;
    private int bytes;

    public Message(String msg) {
        message = msg;

        // Add user ID at the start of the message if exists and if it is not system message (marked as ID: 1).
        if (Client.SELF.id != null && msg.split(";")[0] != "1")
            message = Client.SELF.id + ";" + message;
        // If client id does not exist and the message do not contain default (system) ID then add the default ID.
        else if (Client.SELF.id == null && !msg.split(";")[0].equals("1"))
            message = "1;" + message;

        // Add game ID to th end of the message if exists.
        if (Client.SELF.gameId != null)
            message += ";" + Client.SELF.gameId + ";";

        // Add a separator to the end of the message unless there is any.
        if (!message.substring(message.length() - 1).equals(";"))
            message += ';';

        bytes = message.length();
    }

    public String getMessage() {
        return message;
    }

    public int getBytes() {
        return bytes;
    }
}
