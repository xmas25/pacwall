#include "generate.h"
#include "opts.h"
#include "util.h"

int main(int argc, char **argv) {
    if (!geteuid()) {
        panic("Running as %s is not supported", "root");
    }

    const struct opts opts = parse_opts(argc, argv);
    chdir_xdg("XDG_CACHE_HOME", ".cache/", "pacwall");

    pid_t fetch_pid = -1;
    if (!opts._skip_fetch) {
        fetch_pid =
            subprocess_begin("/usr/lib/pacwall/fetchupdates.sh", "updates.db", opts.db);
    }

    if (!opts._skip_generate) {
        generate_graph(fetch_pid, &opts);
    } else if (fetch_pid > 0) {
        subprocess_wait(fetch_pid, "/usr/lib/pacwall/fetchupdates.sh");
    }

    if (!opts._skip_hook && opts.hook != NULL) {
        char *cwd = getcwd(NULL, 0);
        char *w = malloc(strlen(cwd) + strlen("/pacwall.png") + 1);
        stpcpy(stpcpy(w, cwd), "/pacwall.png");
        free(cwd);
        setenv("W", w, 1);

        execlp(opts.shell, opts.shell, "-c", opts.hook, (char *)NULL);
        panic("Could not execute shell %s: %s", opts.shell, strerror(errno));
    }
}
