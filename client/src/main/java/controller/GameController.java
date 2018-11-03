package main.java.controller;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.scene.image.ImageView;
import javafx.scene.layout.StackPane;
import javafx.scene.paint.Color;
import javafx.scene.text.Text;
import main.java.model.Client;
import main.java.model.Game;
import main.java.model.Message;
import main.java.model.Player;

import java.util.ArrayList;

/**
 * Created by Frixs on 16.10.2018.
 */
public class GameController extends AContentController {

    @FXML
    private Text gameNameT;

    @FXML
    private Text scoreT;

    @FXML
    private Text goalT;

    @FXML
    private Text p1NicknameT;

    @FXML
    private Text p2NicknameT;

    @FXML
    private ImageView p2ChoiceIV;

    @FXML
    private ImageView p1ChoiceIV;

    @FXML
    private StackPane yourTurnPane;

    @FXML
    private StackPane anotherPlayerTurnPane;

    private ArrayList<Text> pNicknameTList = null;
    private ArrayList<ImageView> pChoiceIVList = null;

    /**
     * no-args constructor
     */
    public GameController() {
    }

    /**
     * Initialization
     */
    @FXML
    private void initialize() {
        pNicknameTList = new ArrayList<>();
        pNicknameTList.add(p1NicknameT);
        pNicknameTList.add(p2NicknameT);

        pChoiceIVList = new ArrayList<>();
        pChoiceIVList.add(p1ChoiceIV);
        pChoiceIVList.add(p2ChoiceIV);
    }

    @Override
    public void afterInitialize() {
    }

    /**
     * Switch player control panels.
     * @param bShow     Check if you should show choice panel or hide it.
     */
    public void setChoicePanel(boolean bShow) {
        if (bShow) {
            yourTurnPane.setVisible(true);
            anotherPlayerTurnPane.setVisible(false);
        } else {
            yourTurnPane.setVisible(false);
            anotherPlayerTurnPane.setVisible(true);
        }
    }

    /**
     * Set player to GUI.
     *
     * @param g The game.
     */
    public void setWindow(Game g) {
        p1NicknameT.setText("- - -");
        p1ChoiceIV.setImage(null);
        p2NicknameT.setText("- - -");
        p2ChoiceIV.setImage(null);
        gameNameT.setText(g.getName());
        goalT.setText("Goal: " + g.getGoal());
        scoreT.setText("0 : 0");

        setChoicePanel(false);
    }

    /**
     * Update window information about players.
     *
     * @param list Player list.
     */
    public void updatePlayers(ArrayList<Player> list) {
        int i;
        String score;

        // Set default first.
        p1NicknameT.setText("- - -");
        p1NicknameT.setFill(Color.valueOf("707070"));
        p2NicknameT.setText("- - -");
        p2NicknameT.setFill(Color.valueOf("707070"));

        // Set UI by player values.
        for (i = 0; i < pNicknameTList.size(); i++) {
            if (list.size() <= i)
                break;

            pNicknameTList.get(i).setText(list.get(i).getNickname());
            pNicknameTList.get(i).setFill(list.get(i).getColor());
        }

        // Set score to GUI.
        if (list.size() > 0)
            score =  "" + list.get(0).getScore();
        else
            score = "0";

        score += " : ";

        if (list.size() > 1)
            score +=  "" + list.get(1).getScore();
        else
            score += "0";

        scoreT.setText(score);

        // Check limit.
        if (list.size() > pNicknameTList.size()) {
            System.out.println("ERROR occurred!");
            System.out.println("There are more players in the game than expected!");
        }
    }

    @FXML
    void onActionLeaveBtn(ActionEvent event) {
        Client.SELF.sendMessage(new Message("disconnect_player_from_game")); // Token message.
    }

    @FXML
    void onActionChoosePaperBtn(ActionEvent event) {
        sendChoice(2);
    }

    @FXML
    void onActionChooseRockBtn(ActionEvent event) {
        sendChoice(1);
    }

    @FXML
    void onActionChooseScissorsBtn(ActionEvent event) {
        sendChoice(3);
    }

    private void sendChoice(int choice) {
        if (choice <= 0)
            return;

        setChoicePanel(false);

        Client.SELF.sendMessage(new Message("game_choice_selected;" + choice)); // Token message.
    }
}
