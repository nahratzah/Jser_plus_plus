
import java.io.FileOutputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;

public class SerEnum implements Serializable {
    public static enum Impl {
        ENUM_VALUE;
    }

    public static void main(String[] args) throws Exception {
        if (args.length != 1) {
            System.err.println("Invocation: require output file name");
            System.exit(1);
        }

        try (ObjectOutputStream oos = new ObjectOutputStream(new FileOutputStream(args[0]))) {
            oos.writeObject(Impl.ENUM_VALUE);
        }
    }
}
