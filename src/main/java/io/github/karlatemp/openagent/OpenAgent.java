package io.github.karlatemp.openagent;

import com.typesafe.config.Config;
import com.typesafe.config.ConfigFactory;

import java.io.*;
import java.lang.instrument.Instrumentation;
import java.lang.reflect.Method;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.jar.JarFile;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipOutputStream;

@SuppressWarnings("unused")
public class OpenAgent {
    private static final OpenAgent INSTANCE;

    public static OpenAgent getInstance() {
        SecurityManager sm = System.getSecurityManager();
        if (sm != null) {
            RuntimePermission perm = new RuntimePermission("openagent.use");
            sm.checkPermission(perm);
        }
        return INSTANCE;
    }

    private OpenAgent() {
    }

    public native long getObjectSize(Object object);

    private static native void initialize(String path);

    public native Class<?>[] getAllLoadedClasses();

    static {
        String[] usr = parsePaths("java.library.path");
        String[] sys = parsePaths("sun.boot.library.path");
        File instrument = scan(usr, sys, "instrument");
        INSTANCE = new OpenAgent();
        String os = System.getProperty("os.name");
        String arch = System.getProperty("os.arch");
        String release = null;
        if (os.startsWith("Windows")) {
            if (arch.equals("amd64")) {
                release = "windows-amd64.dll";
            }
        } else if (os.equals("Linux")) {
            if (arch.equals("amd64")) {
                release = "linux-amd64.so";
            }
        }
        if (release == null) {
            throw new ExceptionInInitializerError("Sorry, OpenAgent not supported on " + os + " " + arch);
        }
        String path;
        try {
            File tmp = File.createTempFile("OpenAgent/", "-" + release);
            path = tmp.getAbsolutePath();
            try (InputStream from = OpenAgent.class.getResourceAsStream("/openagent/" + release);
                 OutputStream to = new FileOutputStream(tmp)) {
                byte[] buffer = new byte[1024];
                while (true) {
                    int length = from.read(buffer);
                    if (length == -1) break;
                    to.write(buffer, 0, length);
                }
            }
        } catch (IOException ioException) {
            throw new ExceptionInInitializerError(ioException);
        }
        System.load(path);
//        System.load("G:\\CLionProjects\\jagent\\cmake-build-release\\libjagent.dll");
        try {
            File location = new File("cache/OpenAgent.jar");
            location.getParentFile().mkdirs();
            //noinspection EmptyTryBlock
            try (ZipFile ignored = new ZipFile(location)) {
            } catch (IOException ignored) {
                try (ZipOutputStream outputStream = new ZipOutputStream(
                        new FileOutputStream(location)
                )) {
                    String manifest = "Agent-Class: io.github.karlatemp.openagent.AgentRestore\n" +
                            "Manifest-Version: 1.0\n" +
                            "Can-Redefine-Classes: true\n" +
                            "Can-Set-Native-Method-Prefix: true\n" +
                            "Can-Retransform-Classes: true\n";
                    outputStream.putNextEntry(new ZipEntry("META-INF/"));
                    outputStream.putNextEntry(new ZipEntry("META-INF/MANIFEST.MF"));
                    outputStream.write(manifest.getBytes(StandardCharsets.UTF_8));
                }
            }
        } catch (Throwable any) {
            throw new ExceptionInInitializerError(any);
        }
        initialize(instrument.getPath());
    }

    private static String[] parsePaths(String propname) {
        String ldpath = System.getProperty(propname, "");
        String ps = File.pathSeparator;
        int ldlen = ldpath.length();
        int i, j, n;
        // Count the separators in the path
        i = ldpath.indexOf(ps);
        n = 0;
        while (i >= 0) {
            n++;
            i = ldpath.indexOf(ps, i + 1);
        }

        // allocate the array of paths - n :'s = n + 1 path elements
        String[] paths = new String[n + 1];

        // Fill the array with paths from the ldpath
        n = i = 0;
        j = ldpath.indexOf(ps);
        while (j >= 0) {
            if (j - i > 0) {
                paths[n++] = ldpath.substring(i, j);
            } else if (j - i == 0) {
                paths[n++] = ".";
            }
            i = j + 1;
            j = ldpath.indexOf(ps, i);
        }
        paths[n] = ldpath.substring(i, ldlen);
        return paths;
    }

    private static File scan(String[] usr, String[] sys, String name) {
        name = System.mapLibraryName(name);
        File sys0 = scan(sys, name);
        if (sys0 == null) return scan(usr, name);
        return sys0;
    }

    private static File scan(String[] paths, String name) {
        for (String pt : paths) {
            File file = new File(pt, name);
            if (file.isFile()) {
                return file;
            }
        }
        return null;
    }

    public static void main(String[] args) throws Throwable {
        Instrumentation instrumentation = AgentRestore.instrumentation;
        System.out.println(instrumentation);
        File config = new File("OpenAgent.conf");
        if (!config.isFile()) {
            try (FileOutputStream fos = new FileOutputStream(config);
                 InputStream from = OpenAgent.class.getResourceAsStream("/openagent/config.conf")) {
                byte[] buffer = new byte[2048];
                while (true) {
                    int length = from.read(buffer);
                    if (length == -1) {
                        break;
                    }
                    fos.write(buffer, 0, length);
                }
            }
        }
        Config rootConfig = ConfigFactory.parseFile(config);
        Config bootstrap = rootConfig.getConfig("bootstrap");
        List<? extends Config> agents = rootConfig.getConfigList("agents");
        for (Config sub : agents) {
            String path = sub.getString("path");
            String opt = sub.getString("opt");
            JarFile jar = new JarFile(path);
            instrumentation.appendToSystemClassLoaderSearch(jar);
            String classpath = jar.getManifest().getMainAttributes().getValue("Premain-Class");
            Class<?> startup = Class.forName(classpath);
            boolean a;
            Method met;
            try {
                met = startup.getMethod("premain", String.class, Instrumentation.class);
                a = true;
            } catch (Throwable ignored) {
                met = startup.getMethod("premain", String.class);
                a = false;
            }
            if (a) {
                met.invoke(null, opt, instrumentation);
            } else met.invoke(null, opt);
            jar.close();
        }
        JarFile root = new JarFile(bootstrap.getString("jar"));
        instrumentation.appendToSystemClassLoaderSearch(root);
        String cp;
        switch (bootstrap.getString("mode")) {
            case "jar": {
                String main = root.getManifest().getMainAttributes().getValue("Main-Class");
                String launcherAgent =
                        root.getManifest().getMainAttributes().getValue("Launcher-Agent-Class");
                if (launcherAgent != null) {
                    Class<?> startup = Class.forName(launcherAgent);
                    boolean a;
                    boolean iw = true;
                    Method met;
                    try {
                        met = startup.getMethod("agentmain", String.class, Instrumentation.class);
                        a = true;
                    } catch (Throwable ignored) {
                        try {
                            met = startup.getMethod("agentmain", String.class);
                            a = false;
                        } catch (Throwable ignored0) {
                            iw = false;
                            a = false;
                            met = null;
                        }
                    }
                    if (iw) {
                        if (a)
                            met.invoke(null, null, instrumentation);
                        else
                            met.invoke(null, (Object) null);
                    }
                }
                cp = main;
                break;
            }
            case "class": {
                cp = bootstrap.getString("class");
                break;
            }
            default:
                throw new AssertionError();
        }
        root.close();
        Class.forName(cp).getMethod("main", String[].class)
                .invoke(null, (Object) args);
    }
}
