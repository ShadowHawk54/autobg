/*
Copyright (c) 2013 Ryan Porterfield
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

   	* Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.

	* Neither the name of the copyright owners nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <autobg.h>

struct program_options {
    int         delay;
    const char  *path;
    //const char * (*sort)(const char **);
};

/*********************** Command line arguments ***********************/
const char *D[] = { "-D", "--daemon"    };
const char *d[] = { "-d", "--directory" };
const char *h[] = { "-h", "--help"      };
const char *i[] = { "-i", "--interval"  };
const char *v[] = { "-v", "--version"   };

/************************** Setup Functions ***************************/
char *get_directory (const int ops)
{
    if (not (ops & ABG_DIRECTORY_BIT)) {
        return get_relpath(ABG_WALLPAPER);
    }
    int i = op_arg_cnt(d[0]);
    char **args;
    if (i) {
        args = (char **) op_args(d[0]);
    } else {
        fprintf(stderr, "ERROR: Must specify a directory\n");
        print_help(ops);
        exit(EXIT_FAILURE);
    }
    return args[0];
}

char *get_relpath (const char *relpath)
{
    char *home = getenv("HOME");
    char *path = join_path(home, relpath);
    return path;
}

void init_args ()
{
    op_init(5);

    op_add_option(d, 2);
    op_add_option(D, 2);
    op_add_option(h, 2);
    op_add_option(i, 2);
    op_add_option(v, 2);
}

char *join_path (const char *root, const char *rel)
{
    int root_len = strlen(root);
    int rel_len  = strlen(rel);
    char *full   = malloc(root_len + rel_len + 1);

    strcpy(full, root);
    strcat(full, "/");
    strcat(full, rel);

    return full;
}

int parse_ops (int argc, const char **argv)
{
    op_parse(argv, argc);
    int flags = 0;

    if (op_is_set(h[0]))
        flags = flags | ABG_HELP_BIT;
    if (op_is_set(v[0]))
        flags = flags | ABG_VERSION_BIT;
    if (op_is_set(D[0]))
        flags = flags | ABG_DAEMON_BIT;
    if (op_is_set(d[0]))
        flags = flags | ABG_DIRECTORY_BIT;
    if (op_is_set(i[0]))
        flags = flags | ABG_INTERVAL_BIT;

    return flags;
}

void free_bg_strs (char **bgs)
{
    for (int i = 0; bgs[i] not_eq NULL; i++)
        free(bgs[i]);
    free(bgs);
}

/************************ Daemonize Functions *************************/
/**
 * Closes the connection to stdin, stdout, and stderr so we don't dump
 * anything to the console.
 */
void close_io ()
{
    close(STDIN_FILENO);                // Close stdin
    close(STDOUT_FILENO);               // Close stdout
    close(STDERR_FILENO);               // Close stderr
}

/**
 * Daemonizes the program.
 *
 * Delegation method that handles forking, and other things necessary
 * to safely and properly run as a daemon.
 *
 * @return 0 If this is the child process, >0 if successfully
 *              daemonized, or <0 if unsuccessfully daemonized.
 */
int daemonize ()
{
    open_log();

    int p = spawn_child();
    if (p not_eq 0)
        return p;

    umask(0);                           // Change file mask

    close_io();

    return 0;
}

/**
 * Handles the opening of the logger and setting the log mask.
 *
 * @returns 0 If successful. (Always successful, can this fail?)
 */
void open_log ()
{
    setlogmask(LOG_UPTO(LOG_NOTICE));
    openlog(ABG_DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR, LOG_USER);
    syslog(LOG_INFO, "Starting Daemon");
}

void process (const char *dir)
{

}

/**
 * Forks the parent process.
 *
 * @return 0 If this is the child process, >0 if successfully
 *              daemonized, or <0 if unsuccessfullly daemonized.
 */
pid_t spawn_child ()
{
    pid_t pid;
    pid = fork();                       // Fork the parent process
    if (pid not_eq 0)
        return pid;

    pid_t sid;
    sid = setsid();                     // Create new Signature ID for child
    if (sid < 0)                        // Bad sid
        return -1;
    return 0;
}

/************************* Program Functions **************************/
/*
 * 
 */
int change_bg (const char *path)
{
    const int exec_len  = strlen(ABG_EXEC);
    const int opts_len  = strlen(ABG_OPTS);
    const int path_len  = strlen(path);
    char *command       = malloc(exec_len + opts_len + path_len + 2);

    strcpy(command, ABG_EXEC);
    strcat(command, " ");
    strcat(command, ABG_OPTS);
    strcat(command, " ");
    strcat(command, path);

    printf("command: %s\n", command);
    int status = system(command);
    free(command);
    return status;
}

/*
 * 
 */
int count_bgs (const char *path, int *count)
{
    DIR *dir = opendir(path);
    if (dir == NULL) { 
        fprintf(stderr, "ERROR: Cannot open wallpaper directory.\n");
        return EXIT_FAILURE;
    }
    struct dirent *ent;
    // Get dir count
    int lcount = 0;
    while ((ent = readdir(dir)) not_eq NULL) {
        char *f = ent->d_name;
        if (f[0] == '.' && (f[1] == 0 or f[1] == '.'))
            continue;
        ++lcount;
    }
    closedir(dir);
    *count = lcount;
    return EXIT_SUCCESS;
}

/*
 * 
 */
int count_current_len (const char *path, int *count, long *offset)
{
    FILE *feh = fopen(path, "r");
    if (feh == NULL) 
        return EXIT_FAILURE;

    int lcount = 0, do_count = 0;
    for (char c = 33; c not_eq EOF; c = fgetc(feh)) {
        if (do_count and c == '\'') {
            break;
        }
        if (do_count) {
            ++lcount;
            continue;
        }
        if (c == '\'') {
            do_count = 1;
            *offset = ftell(feh);
            ++lcount;
        }
    }
    fclose(feh);
    *count = lcount;

    return EXIT_SUCCESS;
}

/*
 * 
 */
char *get_next_bg (char **bg_list, char *current)
{
    assert(bg_list != NULL);
    assert(current != NULL);
    char *next = bg_list[0];

    for (int i = 0; bg_list[i] not_eq NULL; i++) {
        if (strcmp(current, bg_list[i]))
            continue;
        int try = i + 1;
        if (bg_list[try] not_eq NULL)
            next = bg_list[try];
        break;
    }

    return next;
}

/**
 * Opens the directory specified, gets the next wallpaper, and changes
 * the wallpaper.
 */
int next_bg (const char *path)
{
    char *fehpath = get_relpath(".fehbg");
 
    int count = 0;
    if (count_bgs(path, &count))
        return EXIT_FAILURE;

    char **bg_list = malloc((count + 1) * sizeof(char*));
    bg_list[count] = NULL;
    if (populate_bgs(path, bg_list)) {
        printf("Populating failed");
        free(fehpath);
        free_bg_strs(bg_list);
        return EXIT_FAILURE;
    }

    count = 0;
    long offset = 0;
    if (count_current_len(fehpath, &count, &offset)) {
        printf("Counting failed");
        return EXIT_FAILURE;
    }

    char *current = malloc(count);
    if (parse_current_bg(fehpath, current, offset)) {
        printf("Parsing failed");
        free(fehpath);
        free_bg_strs(bg_list);
        free(current);
        return EXIT_SUCCESS;
    }

    char *bg = get_next_bg(bg_list, current);
    free(fehpath);
    free(current);
    change_bg(bg);
    free_bg_strs(bg_list);

    return EXIT_SUCCESS;
}

/*
 * 
 */
int parse_current_bg (const char *path, char *current, long offset)
{
    FILE *feh = fopen(path, "r");
    if (feh == NULL) {
        fprintf(stderr, "ERROR: Cannot open %s\n", path);
        return EXIT_FAILURE;
    }

    fseek(feh, offset, SEEK_SET);
    fscanf(feh, "%s", current);
    fclose(feh);

    size_t len = strlen(current);
    current[len - 1] = 0; // trim off an excess ' that feh puts in

    return EXIT_SUCCESS;
}

/*
 * 
 */
int populate_bgs (const char *path, char **bg_list)
{
    DIR *dir = opendir(path);
    if (dir == NULL) { 
        fprintf(stderr, "ERROR: Cannot open wallpaper directory.\n");
        return EXIT_FAILURE;
    }

    struct dirent   *ent;

    int i = 0;
    while ((ent = readdir(dir)) not_eq NULL) {
        char *f = ent->d_name;
        if (f[0] == '.' && (f[1] == 0 or f[1] == '.'))
            continue;
        char *abs = join_path(path, f);
        bg_list[i] = abs;
        ++i;
    }
    closedir(dir);
    return EXIT_SUCCESS;
}

/************************** Print Functions ***************************/
void print_help (const int flags)
{
    print_version();
    printf("Usage:\n%s [-Dhv] [-d <directory>] [-i <interval>]",
            ABG_PROGRAM_NAME);
    printf("\n\nOPTIONS\n");
    print_opt("-h", "--help", "Print this message");
    print_opt("-v", "--version", "Print the current version");
    print_opt("-D", "--daemon", "Run as a daemon");
    print_opt("-d", "--directory", "Specify the directory to search in");
    print_opt("-i", "--interval",
            "Value in minutes to wait between each wallpaper. Only works\
                \twith the -D option");
}

void print_opt(const char *s, const char *l, const char *m)
{
    printf("%s, %s\n", s, l);
    printf("\t\t%s\n\n", m);
}

void print_version ()
{
    printf("%s version %s - Compiled on %s\n\n", ABG_PROGRAM_NAME, ABG_VERSION,
            ABG_DATE);
}

// EOF
