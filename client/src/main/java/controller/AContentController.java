package main.java.controller;

/**
 * Created by Frixs on 16.10.2018.
 */
public abstract class AContentController {
    /* Reference to MainWindowController instance. */
    private AWindowController windowController = null;

    public void setWindowController(AWindowController controller) {
        windowController = controller;
    }

    public AWindowController getWindowController() {
        return windowController;
    }

    /**
     * This methods runs after whole process of changing window content.
     */
    abstract public void afterInitialize();
}
