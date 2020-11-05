#pragma once
#include "io/files.h"
#include <syslib.h>

void diskInit();
int show_file(const char *path) ;
int openFile(FileDescriptor *fd, const char *path, int mode);
