package main.java.model;

import java.util.ArrayList;

/**
 * Created by Frixs on 17.10.2018.
 */
public class Sanitize {
    /**
     * Static variable referring to this singleton class.
     */
    public static final Sanitize SELF = new Sanitize();

    private Sanitize() {
    }

    /**
     * Check if string meets conditions.
     *
     * @param string          String to check.
     * @param lowerLetterFlag Allow lowercase letters?
     * @param upperLetterFlag Allow uppercase letters?
     * @param numberFlag      Allow numbers?
     * @param separatorFlag   Array with separators to check. If you do not want to check separators, leave it with empty ArrayList.
     * @return Bool of success.
     */
    public boolean checkString(String string, boolean lowerLetterFlag, boolean upperLetterFlag, boolean numberFlag, ArrayList<Character> separatorFlag) {
        boolean hasLowerLetters = false;
        boolean hasUpperLetters = false;
        boolean hasNumbers = false;

        for (int i = 0; i < string.length(); i++) {
            final Character ch = string.charAt(i);

            if (Character.isLowerCase(ch)) {
                hasLowerLetters = true;
            } else if (Character.isUpperCase(ch)) {
                hasUpperLetters = true;
            } else if (Character.isDigit(ch)) {
                hasNumbers = true;
            } else if (!separatorFlag.stream().anyMatch(o -> o.equals(ch))) { // If it is not even separator than it is probably character we do not want.
                return false;
            }
        }

        if (!lowerLetterFlag && hasLowerLetters)
            return false;
        if (!upperLetterFlag && hasUpperLetters)
            return false;
        if (!numberFlag && hasNumbers)
            return false;

        return true;
    }
}
