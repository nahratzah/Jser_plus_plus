
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.io.Serializable;

public class SerException implements Serializable {
    private static final long serialVersionUID = 0x12345L;

    private void writeObject(ObjectOutputStream out) throws IOException {
        out.defaultWriteObject();
        throw new IOException("exception test");
    }

    private void readObject(java.io.ObjectInputStream in) throws IOException, ClassNotFoundException {
        in.defaultReadObject();
    }

    public static void main(String[] args) throws Exception {
        if (args.length != 1) {
            System.err.println("Invocation: require output file name");
            System.exit(1);
        }

        try (ObjectOutputStream oos = new ObjectOutputStream(new FileOutputStream(args[0]))) {
            try {
                oos.writeObject(new SerException());
            } catch (IOException ex) {
                // ignore
            }
        }
    }
}
