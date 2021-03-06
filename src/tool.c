/* Copyright (c) 2010-2018 the corto developers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <corto>

static int runCommand(char *cmd, int argc, char *argv[]) {
    /* If a command is found, run the command with remaining args */
    corto_id package;
    sprintf(package, "driver/tool/%s", cmd);
    return ut_run(package, argc, argv);
}

static int loadArguments(int argc, char* argv[]) {
    int i;
    for (i = 0; i < argc; i++) {
        if (ut_use(argv[i], 0, NULL)) {
            return i + 1;
        }
    }
    return 0;
}

static void printLogo(void) {
    char *color1 = UT_GREEN, *color2 = UT_CYAN;
    printf("\n");
    printf("            %s##########%s\n", color1, UT_NORMAL);
    printf("          %s##########%s ####%s\n", color1, color2, UT_NORMAL);
    printf("        %s##########%s ########%s\n", color1, color2, UT_NORMAL);
    printf("      %s##########%s   ##########%s\n", color1, color2, UT_NORMAL);
    printf("    %s##########%s       ##########%s\n", color1, color2, UT_NORMAL);
    printf("  %s##########%s           ##########%s\n", color1, color2, UT_NORMAL);
    printf("   %s##########%s         ##########%s\n", color1, color2, UT_NORMAL);
    printf("     %s##########%s     ##########%s\n", color1, color2, UT_NORMAL);
    printf("       %s##########%s ##########%s\n", color1, color2, UT_NORMAL);
    printf("         %s######%s ##########%s\n", color1, color2, UT_NORMAL);
    printf("           %s##%s ##########%s\n", color1, color2, UT_NORMAL);
    printf("\n");
}

static void printUsage(void) {
    printf("Usage: corto [options] <command> [arguments]\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h,--help                  Display this usage information\n");
    printf("\n");
    printf("  version and logo:\n");
    printf("  --patch                    Display major.minor.patch version\n");
    printf("  --minor                    Display major.minor version\n");
    printf("  --major                    Display major version\n");
    printf("  -v                         Same as --patch\n");
    printf("  --version                  Display current version and build\n");
    printf("  --logo                     Display logo\n");
    printf("\n");
    printf("  loading packages & files:\n");
    printf("  -l,--load [file/package]   Pass all subsequent arguments to specified file/package\n");
    printf("  -a,--keep-alive            Keep corto running after command is executed\n");
    printf("\n");
    printf("  security:\n");
    printf("  --force-security           Fail if no secure/lock is configured.\n");
    printf("  --username                 Provide username for logging in.\n");
    printf("  --password                 Provide password for logging in.\n");
    printf("\n");
    printf("  configuration:\n");
    printf("  --name [id]                Assign a name to the process\n");
    printf("  --config [path]            A path or file that contains configuration\n");
    printf("  --cwd [path]               Specify the current working directory\n");
    printf("\n");
    printf("  tracing:\n");
    printf("  --debug                    Set verbosity to DEBUG\n");
    printf("  --trace                    Set verbosity to TRACE\n");
    printf("  --verbosity                Set verbosity level (DEBUG, TRACE, OK, INFO, WARNING, ERROR, CRITICAL)\n");
    printf("  --log-depth                Set verbosity depth\n");
    printf("  --exit-on-exception        Exit program on exception\n");
    printf("  --abort-on-exception       Abort program on exception\n");
    printf("  --profile                  Enable profiling\n");
    printf("  --mono                     Disable colors\n");
    printf("  --show-lines               Show linenumbers of log messages\n");
    printf("  --show-time                Show time of log messages\n");
    printf("  --show-delta               Show time delta between log messages\n");
    printf("  --show-proc                Show process id\n");
    printf("  --mute                     Mute errors from loaded packages\n");
    printf("  --backtrace                Enable backtraces for tracing\n");
    printf("\n");
    printf("  memory management:\n");
    printf("  --trace-mem                Trace memory operations\n");
    printf("  --collect-cycles           Collect reference cycles\n");
    printf("  --collect-tls              Cleanup TLS in mainthread. Process always exits with 0\n");
}

#define PARSE_OPTION(short, long, action)\
    if (argv[i][0] == '-') {\
        if (argv[i][1] == '-') {\
            if (long && !strcmp(&argv[i][2], long ? long : "")) {\
                action;\
                parsed = true;\
            }\
        } else {\
            if (short && argv[i][1] == short) {\
                action;\
                parsed = true;\
            }\
        }\
    }

/* Global options */
static bool load = false;
static bool keep_alive = false;
static bool mute = false;
static char *appname;
static char *cwd;
static bool showLines = false;
static bool showTime = false;
static bool showDelta = false;
static bool showProc = false;
static bool profile = false;
static bool force_security = false;
static char *username;
static char *password;

static void printVersion(bool minor, bool patch) {
    if (patch) {
        printf("%s.%s.%s\n", BAKE_VERSION_MAJOR, BAKE_VERSION_MINOR, BAKE_VERSION_PATCH);
    } else
    if (minor) {
        printf("%s.%s\n", BAKE_VERSION_MAJOR, BAKE_VERSION_MINOR);
    } else {
        printf("%s\n", BAKE_VERSION_MAJOR);
    }
}

static void printLongVersion(void) {
    printf("corto version %s (%s)\n  build %s\n",
        BAKE_VERSION,
        UT_PLATFORM_STRING,
        corto_get_build());
}

static void set_verbosity(const char *verbosity) {
    if (!verbosity) {
        return;
    }

    if (!stricmp(verbosity, "DEBUG")) {
        ut_log_verbositySet(UT_DEBUG);
    } else if (!stricmp(verbosity, "TRACE")) {
        ut_log_verbositySet(UT_TRACE);
    } else if (!stricmp(verbosity, "OK")) {
        ut_log_verbositySet(UT_OK);
    } else if (!stricmp(verbosity, "INFO")) {
        ut_log_verbositySet(UT_INFO);
    } else if (!stricmp(verbosity, "WARNING")) {
        ut_log_verbositySet(UT_WARNING);
    } else if (!stricmp(verbosity, "ERROR")) {
        ut_log_verbositySet(UT_ERROR);
    } else if (!stricmp(verbosity, "CRITICAL")) {
        ut_log_verbositySet(UT_CRITICAL);
    } else {
        printf("not a valid verbosity level '%s'\n", verbosity);
    }
}

static int parseGenericArgs(int argc, char *argv[]) {
    int i;
    bool parsed;

    for(i = 0; i < argc; i++) {
        parsed = false;
        if (argv[i][0] == '-') {
            PARSE_OPTION('l', "load", load = true; i ++; break);
            PARSE_OPTION('a', "keep-alive", keep_alive = true);
            PARSE_OPTION(0, "patch", printVersion(true, true));
            PARSE_OPTION(0, "minor", printVersion(true, false));
            PARSE_OPTION(0, "major", printVersion(false, false));
            PARSE_OPTION(0, "version", printLongVersion());
            PARSE_OPTION('h', "help", printUsage());
            PARSE_OPTION(0, "logo", printLogo());
            PARSE_OPTION(0, "name", appname = argv[i + 1]; i ++);
            PARSE_OPTION(0, "config", ut_setenv("CORTO_CONFIG", argv[i + 1]); i ++);
            PARSE_OPTION(0, "force-security", force_security = true);
            PARSE_OPTION(0, "username", username = argv[i + 1]; i ++);
            PARSE_OPTION(0, "password", password = argv[i + 1]; i ++);
            PARSE_OPTION(0, "cwd", cwd = argv[i + 1]; i ++);
            PARSE_OPTION(0, "debug", ut_log_verbositySet(UT_DEBUG));
            PARSE_OPTION(0, "trace", ut_log_verbositySet(UT_TRACE));
            PARSE_OPTION('v', "verbosity", set_verbosity(argv[i + 1]); i ++);
            PARSE_OPTION(0, "log-depth", ut_log_verbositySetDepth(atoi(argv[i + 1])); i ++);
            PARSE_OPTION(0, "exit-on-exception", ut_log_setExceptionAction(UT_LOG_ON_EXCEPTION_EXIT));
            PARSE_OPTION(0, "abort-on-exception", ut_log_setExceptionAction(UT_LOG_ON_EXCEPTION_ABORT));
            PARSE_OPTION(0, "profile", profile = true);
            PARSE_OPTION(0, "mono", ut_log_useColors(false));
            PARSE_OPTION(0, "show-lines", showLines = true);
            PARSE_OPTION(0, "show-time", showTime = true);
            PARSE_OPTION(0, "show-delta", showDelta = true);
            PARSE_OPTION(0, "show-proc", showProc = true);
            PARSE_OPTION(0, "mute", mute = true);
            PARSE_OPTION(0, "backtrace", UT_LOG_BACKTRACE = true);
            PARSE_OPTION(0, "trace-mem", CORTO_TRACE_MEM = true);
            PARSE_OPTION(0, "collect-cycles", CORTO_COLLECT_CYCLES = true);
            PARSE_OPTION(0, "collect-tls", CORTO_COLLECT_TLS = true);

            if (!parsed) {
                fprintf(stderr, "unknown option '%s' (use corto --help to see available options)\n", argv[i]);
                return -1;
            }
        } else {
            break;
        }
    }

    return i + 1;
}

int main(int argc, char *argv[]) {
    int result = 0;

    appname = argv[0];

    /* Parse arguments before first command. Any arguments after the first
     * command or file are passed to that file or command. */
    int last_parsed = parseGenericArgs(argc - 1, &argv[1]);

    if (showLines) {
        char *fmt = ut_asprintf("%s %s", "%f:%l", ut_log_fmtGet ());
        ut_log_fmt (fmt);
        free(fmt);
    }

    if (showTime) {
        char *fmt = ut_asprintf("%s %s", "%T", ut_log_fmtGet ());
        ut_log_fmt (fmt);
        free(fmt);
    }

    if (showDelta) {
        char *fmt = ut_asprintf("%s %s", "%d", ut_log_fmtGet ());
        ut_log_fmt (fmt);
        free(fmt);
    }

    if (showProc) {
        char *fmt = ut_asprintf("%s %s", "%A", ut_log_fmtGet ());
        ut_log_fmt (fmt);
        free(fmt);
    }

    if (profile) {
        ut_log_profile(true);
    }

    /* If arguments are invalid, don't bother starting corto */
    if (last_parsed != -1) {
        char *cmd = "unknown";

        if (keep_alive) {
            ut_setenv("CORTO_KEEP_ALIVE", "true");
        }

        /* Start corto */
        corto_start(appname);

        /* Load configuration (if CORTO_CONFIG is set) */
        if (corto_load_config()) {
            return -1;
        }

        /* After loading configuration, enable security */
        if (!corto_enable_security(true)) {
            if (force_security) {
                ut_throw("failed to enable security (is a lock configured?)");
                goto error;
            }
        } else {
            ut_info("security enabled");
        }

        if (username) {
            const char *session_id = corto_login(username, password);
            if (!session_id) {
                ut_throw("login failed");
                goto error;
            } else {
                /* Immediately activate session */
                corto_set_session(session_id);
            }
        }

        /* If there are more arguments than that have been parsed so far, there must
         * be a command or file to be loaded */
        if (argc > last_parsed) {
            char **cur_args = &argv[last_parsed];
            int cur_count = argc - last_parsed;

            /* If load is set, only load a single package/file, and pass all
             * remaining arguments to this file */
            if (load) {
                result = ut_use(cur_args[0], cur_count, cur_args);
            } else {
                /* Test if current argument is a command */
                corto_id package;
                sprintf(package, "driver.tool.%s", cur_args[0]);
                const char *lib = NULL;
                cmd = cur_args[0];
                if (!strchr(cmd, '.') && cmd[0] != '/' && (lib = ut_locate(package, NULL, UT_LOCATE_LIB))) {
                    result = runCommand(cur_args[0], cur_count, cur_args);
                } else {
                    /* If not a command, load subsequent arguments */
                    result = loadArguments(cur_count, cur_args);
                    if (result) {
                        cmd = cur_args[result - 1];
                    }
                }
            }
        } else if (argc == 1) {
            /* Run default command */
            if ((result = runCommand("default", 0, NULL))) {
                ut_catch(); /* Clear previous error */
                ut_throw("no default command configured");
            }
        }

        if (result) {
            if (!ut_raised()) {
                ut_throw("errors occurred while loading '%s'", cmd);
            }
            ut_raise();
        }

        if (keep_alive) {
            ut_info("Keeping process alive, press CTRL-C to exit");
            while (true) {
                ut_sleep(1, 0);
            }
        }

        /* Stop corto */
        corto_stop();
    } else {
        result = -1;
    }

    return result;
error:
    corto_stop();
    return -1;
}
