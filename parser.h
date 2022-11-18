/**********************************************************************
 * Copyright (c) 2020
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#ifndef __PARSER_H__
#define __PARSER_H__

#define MAX_NR_TOKENS	32	/* Maximum length of tokens in a command */
#define MAX_TOKEN_LEN	128	/* Maximum length of single token */
#define MAX_COMMAND_LEN	4096 /* Maximum length of assembly string */


/***********************************************************************
 * parse_command()
 *
 * DESCRIPTION
 *  Parse @command, and put each command token into @tokens[] and the number of
 *  tokes into @nr_tokens. You may use this implemention or your own from PA0.
 *
 *  A command token is defined as a string without any whitespace (i.e., *space*
 *  and *tab* in this programming assignment). For exmaple,
 *   command = "  cp  -pr /home/sslab   /path/to/dest  "
 *
 *  then, nr_tokens = 4, and tokens is
 *    tokens[0] = "cp"
 *    tokens[1] = "-pr"
 *    tokens[2] = "/home/sslab"
 *    tokens[3] = "/path/to/dest"
 *    tokens[>=4] = NULL
 *
 *
 * RETURN VALUE
 *  Return 1 if @nr_tokens > 0
 *  Return 0 otherwise
 *
 */
int parse_command(char *command, int *nr_tokens, char *tokens[]);

#endif
