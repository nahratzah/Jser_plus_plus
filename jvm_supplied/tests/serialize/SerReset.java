
import java.io.FileOutputStream;
import java.io.ObjectOutputStream;

public class SerReset {
    public static void main(String[] args) throws Exception {
        if (args.length != 1) {
            System.err.println("Invocation: require output file name");
            System.exit(1);
        }

        try (ObjectOutputStream oos = new ObjectOutputStream(new FileOutputStream(args[0]))) {
            oos.writeObject("foo");
            oos.reset();
            oos.writeObject("bar");
            oos.writeObject("bar");
        }
    }
}
