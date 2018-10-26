package main.java.model;

/**
 * Created by Frixs on 17.10.2018.
 */
public interface INetwork {
    /**
     * Connect a new client to the server.
     * @param ihost         Internet host address.
     * @param iport         Internet host port number.
     * @param nickname      Nickname with which you are joining in.
     * @return              Status code.
     *                          -1 = OK - Reconnected.
     *                           0 = OK.
     *                           1 = Cannot connect to server.
     *                           2 = Wrong inputs.
     *                           3 = Error during connection.
     */
    public int connect(String ihost, int iport, String nickname);

    /**
     * Close the connection.
     */
    public void disconnect();

    /**
     * Send the message to connected server.
     * @param msg       The message.
     */
    public void sendMessage(Message msg);
}
