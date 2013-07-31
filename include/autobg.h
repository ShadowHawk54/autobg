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

#ifndef AUTOBG_H
#define AUTOBG_H

#include <assert.h>
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

#include <sys/stat.h>
#include <sys/types.h>

/************************* User Configuration *************************/
// External dependency which handles background switching for us
#define ABG_EXEC        "feh"
// Command line arguments passed to ABG_EXEC
#define ABG_OPTS        "--bg-scale"

/***************************** Constants ******************************/
#define ABG_DAEMON_NAME     "autobgd"
#define ABG_PROGRAM_NAME    "autobg"
#define ABG_VERSION         "0.1.7"
#define ABG_DATE            "2013-07-26"
#define ABG_WALLPAPER       "Pictures/Wallpapers"

#define ABG_HELP_BIT        (1 << 0) // 0b00000001
#define ABG_VERSION_BIT     (1 << 1) // 0b00000010
#define ABG_DAEMON_BIT      (1 << 2) // 0b00000100
#define ABG_DIRECTORY_BIT   (1 << 3) // 0b00001000
#define ABG_INTERVAL_BIT    (1 << 4) // 0b00010000

/************************ Function-like Macros ************************/
#define OVERFLOW(a, b)\
    ({ __typeof__ (a) _a = (a);\
       __typeof__ (b) _b = (b);\
       _a > _b ? 0 : _a })

/************************ Function Prototypes *************************/
// Setup functions
char *  get_directory       (const int);
char *  get_relpath         (const char*);
void    init_args           ();
char *  join_path           (const char *, const char *);
int     parse_ops           ();
void    free_bg_strs        (char **);

// Daeamonize functions
void    close_io            ();
int     daemonize           ();
void    open_log            ();
void    process             ();
pid_t   spawn_child         ();

// Program functions
int     change_bg           (const char *);
int     count_bgs           (const char *, int *);
int     count_current_len   (const char *, int *, long *);
char *  get_next_bg         (char **, char *);
int     next_bg             (const char *);
int     parse_current_bg    (const char *, char *, long);
int     populate_bgs        (const char *, char **);

// Print functions
void    print_help          (const int);
void    print_opt           (const char *, const char *, const char *);
void    print_version       ();

#endif // AUTOBG_H
