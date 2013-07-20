/*
 * @brief op header
 *
 * @author Matthias Beyer
 *
 * @license Have a look at the LICENSE file, shipped with the code repository
 */

#ifndef __OP_H__
#define __OP_H__

#define OP_LONG_OPTION_PREF "--"
#define OP_SHORT_OPTION_PREF "-"

void            op_init         (unsigned int opt_count);
void            op_add_option   (const char **keys, const unsigned int key_count);
int             op_is_set       (const char *key);
unsigned int    op_arg_cnt      (const char *key);
const char **   op_args         (const char *key);
void            op_shutdown     (void);

void            op_parse        (const char **argv, const int argc);
#endif //__OP_H__
