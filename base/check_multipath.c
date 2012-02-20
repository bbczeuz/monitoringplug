/***
 * Monitoring Plugin - check_multipath.c
 **
 *
 * check_multipath - Check RedHat Cluster status with clustat.
 *
 * Copyright (C) 2012 Marius Rieder <marius.rieder@durchmesser.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id$
 */

const char *progname  = "check_multipath";
const char *progdesc  = "Check multipath status";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "";

/* MP Includes */
#include "mp_common.h"
/* Default Includes */
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>

/* Global Vars */
int nonroot = 0;

int main (int argc, char **argv) {
    /* Local Vars */
    FILE        *fp;
    char        line[128];
    int         failed = 0;
    int         lines = 0;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    // Need to be root
    if (nonroot == 0)
        mp_noneroot_die();

    alarm(mp_timeout);

    // Parse clustat
    if (nonroot == 0)
        fp = mp_popen((char *[]) {"/sbin/multipath","-l", NULL});
    else
        fp = fopen("multipath","r");
    if (fp == NULL)
       unknown("Can't exec multipath");

    while ( fgets ( line, sizeof line, fp) != NULL ) {
        if (mp_verbose > 1) {
            printf(">> %s", line);
        }

        lines++;
        if (strstr(line, "failed") != NULL) {
            failed++;
        }

    }

    if (nonroot == 0) {
        if (mp_pclose(fp) != 0) {
            critical("Executing multipath faild!");
        }
    } else {
        fclose(fp);
    }

    if (lines == 0)
        warning("No paths defined");
    else if (failed > 0)
        critical("%d paths failed", failed);

    ok("Multipath");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        {"noroot", no_argument, NULL, (int)'n'},
        MP_LONGOPTS_END
    };

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"nt:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch(c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            case 'n':
                nonroot = 1;
                break;
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
                break;
        }

    }

    return(OK);
}

void print_help (void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();
}

/* vim: set ts=4 sw=4 et syn=c : */