
import java.io.FileOutputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;

public class SerArray implements Serializable {
    private static final long serialVersionUID = 0x12345L;

    private byte[] byteField = new byte[]{0, 1, 2};
    private short[] shortField = new short[]{3, 4, 5};
    private int[] intField = new int[]{6, 7, 8};
    private long[] longField = new long[]{9, 10, 11};
    private float[] floatField = new float[]{12, 13, 14};
    private double[] doubleField = new double[]{15, 16, 17};
    private String[] stringField = new String[]{"eighteen", "nineteen", "twenty"};
    private Object[] nullField = new Object[]{null, null, null};

    public static void main(String[] args) throws Exception {
        if (args.length != 1) {
            System.err.println("Invocation: require output file name");
            System.exit(1);
        }

        try (ObjectOutputStream oos = new ObjectOutputStream(new FileOutputStream(args[0]))) {
            oos.writeObject(new SerArray());
        }
    }
}
