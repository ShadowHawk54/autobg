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
#include <stdarg.h>

#define FAIL "[\e[01;31mFAIL\e[00m]"
#define PASS "[\e[01;32mPASS\e[00m]"

#define MIN(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a > _b ? _b : _a; })

/************************ Function Prototypes *************************/
// Setup functions
static int test_count_bgs           ();
static int test_get_next_bg         ();
static int test_get_relpath         ();
static int test_join_path           ();
static int test_populate_bgs        ();

// Print functions
void       print_test_result        (const char *, ...);
void       print_test_status        (int, const char *);

/******************************** Main ********************************/
int main (int argc, const char **argv)
{
    printf("[\e[01;32mPASS\e[00m/\e[01;31mFAIL\e[00m]\tTest Name\t\t");
    printf("Expected:\t\tGot:\n\n");

    test_count_bgs();
    test_get_relpath();
    test_join_path();
    test_get_next_bg();

    return EXIT_SUCCESS;
}

/******************************** Print ********************************/
void print_test_result (const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
}

void print_test_status(int status, const char *test)
{
    if (status)
        printf("%s\t\t", FAIL);
    else
        printf("%s\t\t", PASS);
    printf("%s:\t", test);
    if (strlen(test) < 16)
        printf("\t");
}

/******************************** Tests ********************************/
static int test_count_bgs ()
{
    const int expected = 2;
    int got = 0;
    count_bgs("/home/ryan/Pictures/Wallpapers", &got);

    int status = (got == expected ? 0 : 1);
    
    print_test_status(status, "test_count_bgs");
    print_test_result("%d\t\t\t%d\n", expected, got);
    return status;
}

static int test_get_next_bg ()
{
    char *bg0 = "/home/ryan/Picture00.jpg";
    char *bg1 = "/home/ryan/Picture01.jpg";
    char *bg2 = "/home/ryan/Picture02.jpg";

    char **bg_list = malloc(4 * sizeof(char*));
    bg_list[0] = bg0;
    bg_list[1] = bg1;
    bg_list[2] = bg2;
    bg_list[3] = NULL;

    char *result1 = get_next_bg(bg_list, bg0);
    char *result2 = get_next_bg(bg_list, bg2);

    print_test_status(strcmp(bg1, result1), "test_count_bgs");
    print_test_result("%s\t%s\n", bg1, result1);

    print_test_status(strcmp(bg0, result2), "test_count_bgs");
    print_test_result("%s\t%s\n", bg0, result2);

    free(bg_list);
    return EXIT_SUCCESS;
}

static int test_get_relpath ()
{
    const char *expected = "/home/ryan/colors";
    const char *got      = get_relpath("colors");

    int status = strcmp(expected, got);

    print_test_status(status, "test_get_relpath");
    print_test_result("%s\t%s\n", expected, got);
    return status;
}

static int test_join_path ()
{
    const char *expected = "/usr/bin/autobg";
    const char *got      = join_path("/usr/bin", "autobg");

    int status = strcmp(expected, got);

    print_test_status(status, "test_join_path");
    print_test_result("%s\t\t%s\n", expected, got);
    return status;
}

static int test_populate_bgs ()
{

}

