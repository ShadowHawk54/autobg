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

#include <autobg.h>

int main (int argc, const char **argv)
{
    init_args();

    const int ops = parse_ops(argc, argv);
    if (ops & ABG_HELP_BIT) {
        print_help(ops);
        return EXIT_SUCCESS;
    }
    if (ops & ABG_VERSION_BIT) {
        print_version();
        return EXIT_SUCCESS;
    }

    char *path = get_directory(ops);

    if (not (ops & ABG_DAEMON_BIT)) {
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

