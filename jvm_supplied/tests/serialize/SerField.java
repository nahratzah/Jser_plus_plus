
import java.io.FileOutputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;

public class SerField implements Serializable {
    private static final long serialVersionUID = 0x12345L;

    private byte byteField = 0;
    private short shortField = 1;
    private int intField = 2;
    private long longField = 3;
    private float floatField = 4;
    private double doubleField = 5;
    private String stringField = "six";
    private Object nullField = null;

    public static void main(String[] args) throws Exception {
        if (args.length != 1) {
            System.err.println("Invocation: require output file name");
            System.exit(1);
        }

        try (ObjectOutputStream oos = new ObjectOutputStream(new FileOutputStream(args[0]))) {
            oos.writeObject(new SerField());
        }
    }
}
