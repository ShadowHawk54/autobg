/* Copyright (c) 2013 Ryan Porterfield
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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <iso646.h>
#include <op.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#define DAEMON_NAME     "autobgd"
#define PROGRAM         "autobg"
#define VERSION         "0.1.5"
#define DATE            "2013-07-17"
#define WALLPAPER       "Pictures/Wallpapers"

#define HELP_BIT        (1 << 0) // 0b00000001
#define VERSION_BIT     (1 << 1) // 0b00000010
#define DAEMON_BIT      (1 << 2) // 0b00000100
#define DIRECTORY_BIT   (1 << 3) // 0b00001000
#define INTERVAL_BIT    (1 << 4) // 0b00010000

struct program_options {
    int         delay;
    const char  **path;
    //const char * (*sort)(const char **);
};

/*********************** Command line arguments ***********************/
const char *D[] = { "-D", "--daemon"    };
const char *d[] = { "-d", "--directory" };
const char *h[] = { "-h", "--help"      };
const char *i[] = { "-i", "--interval"  };
const char *v[] = { "-v", "--version"   };

/************************ Function Prototypes *************************/
// Setup functions
static char *   get_directory       (const int);
static char *   get_relpath         (const char*);
static void     init_args           ();
static int      parse_ops           ();

// Daeamonize functions
static void     close_io            ();
static int      daemonize           ();
static void     open_log            ();
static void     process             ();
static pid_t    spawn_child         ();

// Program functions
static void     change_bg           (const char *);
static char *   get_next_bg         (DIR*, FILE*);
static int      next_bg             (const char *);
static char *   parse_current_bg    (FILE*);

// Print functions
static void     print_help          (const int);
static void     print_opt           (const char *, const char *, const char *);
static void     print_version       ();

/******************************** MAIN ********************************/
int main (int argc, const char **argv)
{
    init_args();

    const int ops = parse_ops(argc, argv);
    if (ops & HELP_BIT) {
        print_help(ops);
        return EXIT_SUCCESS;
    }
    if (ops & VERSION_BIT) {
        print_version();
        return EXIT_SUCCESS;
    }

    char *path = get_directory(ops);

    if (not (ops & DAEMON_BIT)) {
        next_bg(path);
        free(path);
        return EXIT_SUCCESS;
    }

    int d = daemonize();
    if (d < 0)
        return EXIT_FAILURE;
    if (d > 0)
        return EXIT_SUCCESS;

    process();

    return EXIT_SUCCESS;
}

/************************** Setup Functions ***************************/
static char *get_directory (const int ops)
{
    if (not (ops & DIRECTORY_BIT)) {
        return get_relpath(WALLPAPER);
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

static char *get_relpath (const char *relpath)
{
    char *home = getenv("HOME");
    int i = 0;
    for (char c = 33; c not_eq 0; c = home[i]) {
        ++i;
    }
    int j = 0;
    for (char c = 33; c not_eq 0; c = relpath[j]) {
        ++j;
    }
    char *path = malloc(i + j + 1);
    strcpy(path, home);
    strcat(path, "/");
    strcat(path, relpath);
    return path;
}

static void init_args ()
{
    op_init(5);

    op_add_option(d, 2);
    op_add_option(D, 2);
    op_add_option(h, 2);
    op_add_option(i, 2);
    op_add_option(v, 2);
}

static int parse_ops (int argc, const char **argv)
{
    op_parse(argv, argc);
    int flags = 0;

    if (op_is_set(h[0]))
        flags = flags | HELP_BIT;
    if (op_is_set(v[0]))
        flags = flags | VERSION_BIT;
    if (op_is_set(D[0]))
        flags = flags | DAEMON_BIT;
    if (op_is_set(d[0]))
        flags = flags | DIRECTORY_BIT;
    if (op_is_set(i[0]))
        flags = flags | INTERVAL_BIT;

    return flags;
}


/************************ Daemonize Functions *************************/
/**
 * Closes the connection to stdin, stdout, and stderr so we don't dump
 * anything to the console.
 */
static void close_io ()
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
static int daemonize ()
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
static void open_log ()
{
    setlogmask(LOG_UPTO(LOG_NOTICE));
    openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR, LOG_USER);
    syslog(LOG_INFO, "Starting Daemon");
}

static void process (const char *dir)
{

}

/**
 * Forks the parent process.
 *
 * @return 0 If this is the child process, >0 if successfully
 *              daemonized, or <0 if unsuccessfullly daemonized.
 */
static pid_t spawn_child ()
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
static void change_bg (const char *path)
{
}

static char *get_next_bg (DIR *dir, FILE *feh)
{
    char *current = parse_current_bg(feh);
    struct dirent *ent;
    while ((ent = readdir(dir)) not_eq NULL) {
        char *f = ent->d_name;
        if (f[0] == '.')
            continue;
        printf("%s\n", f);
    }
    free(current);
    return "NULL";
}

/**
 * Opens the directory specified, gets the next wallpaper, and changes
 * the wallpaper.
 */
static int next_bg (const char *path)
{
    DIR *dir;
    FILE *feh;
    dir = opendir(path);
    const char *fehpath = get_relpath(".fehbg");
    feh = fopen(fehpath, "r");
    if (dir == NULL) { 
        fprintf(stderr, "ERROR: Cannot open wallpaper directory.\n");
        return EXIT_FAILURE;
    } if (feh == NULL) {
        fprintf(stderr, "ERROR: Cannot open %s\n", fehpath);
        return EXIT_FAILURE;
    }
 
    const char *bg = get_next_bg(dir, feh);
    closedir(dir);
    fclose(feh);

    change_bg(bg);
    return EXIT_SUCCESS;
}

static char * parse_current_bg (FILE *feh)
{
    char *in = malloc(128);
    while ((fscanf(feh, "%s", in) not_eq EOF)) {
        if (in[0] == '/')
            break;
    }
    return in;
}

/************************** Print Functions ***************************/
static void print_help (const int flags)
{
    print_version();
    printf("Usage:\n%s [-Dhv] [-d <directory>] [-i <interval>]\n\n", PROGRAM);
    printf("OPTIONS\n");
    print_opt("-h", "--help", "Print this message");
    print_opt("-v", "--version", "Print the current version");
    print_opt("-D", "--daemon", "Run as a daemon");
    print_opt("-d", "--directory", "Specify the directory to search in");
    print_opt("-i", "--interval",
            "Value in minutes to wait between each wallpaper. Only works\
                \twith the -D option");
}

static void print_opt(const char *s, const char *l, const char *m)
{
    printf("%s, %s\n", s, l);
    printf("\t\t%s\n\n", m);
}

static void print_version ()
{
    printf("%s version %s - Compiled on %s\n\n", PROGRAM, VERSION, DATE);
}

// EOF
