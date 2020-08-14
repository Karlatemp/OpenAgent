package io.github.karlatemp.openagent;

import java.lang.instrument.Instrumentation;

public class AgentRestore {
    static Instrumentation instrumentation;

    public static void agentmain(
            String opt,
            Instrumentation instrumentation
    ) {
        AgentRestore.instrumentation = instrumentation;
    }
}
