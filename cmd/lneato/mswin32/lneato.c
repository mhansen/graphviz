/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

/* Lefteris Koutsofios - AT&T Bell Laboratories */

#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <shellapi.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cgraph/exit.h>

static char *shellpath;

static char *buildpath (char *);

int WINAPI wWinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
        PWSTR pCmdLine, int nCmdShow) {
    HANDLE handle;
    char cmd[256];
    char *path;
    char *s;
    int argc;
    LPWSTR *argv;

    shellpath = getenv ("PATH");
    if (!shellpath || !(path = buildpath ("lefty"))) {
        if (!GetModuleFileName (hInstance, cmd, 256) ||
                !(s = strrchr (cmd, '\\')))
            graphviz_exit(1);
        *s = 0;
        shellpath = &cmd[0];
        if (!(path = buildpath ("lefty")))
            graphviz_exit(1);
    }
    argv = CommandLineToArgvW(pCmdLine, &argc);
    if (argc == 1 && wcscmp(argv[0], L"-?") == 0) {
        fprintf(stderr, "usage: lneato [-V] [-lm (sync|async)] [-el (0|1)] <filename>\n");
        graphviz_exit(0);
    }
    if (pCmdLine[0] == 0)
        snprintf(cmd, sizeof(cmd), "%s -e \"load('dotty.lefty');"
                 "dotty.protogt.lserver='neato';dotty.simple(null);\"", path);
    else
        snprintf(cmd, sizeof(cmd), "%s -e \"load('dotty.lefty');"
                 "dotty.protogt.lserver='neato';dotty.simple('%Ns');\"", path,
                 pCmdLine);

    handle = WinExec (cmd, SW_SHOW);
    graphviz_exit(0);
}

#define PATHDEL '\\'
#define PATHSEP ';'

static char pathbuf[1024];
static char commandbuf[1024];

static char *buildpath (char *file) {
    struct stat statbuf;
    char *s1, *s2;
    int mode, pathi;

    if (file && file[0] && (file[0] == '.' || file[0] == PATHDEL))
        return file;
    mode = ~0;
    s1 = shellpath;
    while (*s1) {
        pathi = 0;
        while (*s1 && *s1 != PATHSEP)
            pathbuf[pathi++] = *s1++;
        if (*s1)
            s1++;
        pathbuf[pathi++] = PATHDEL;
        for (s2 = file; *s2; s2++)
            pathbuf[pathi++] = *s2;
        pathbuf[pathi++] = '.';
        pathbuf[pathi++] = 'e';
        pathbuf[pathi++] = 'x';
        pathbuf[pathi++] = 'e';
        pathbuf[pathi] = '\000';
        if (stat (pathbuf, &statbuf) == 0 && (statbuf.st_mode & mode))
            return pathbuf;
    }
    return NULL;
}
