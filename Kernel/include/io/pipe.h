#pragma once
#include <io/files.h>
#include <syslib.h>

bool openPipe(FileDescriptor *readfd, FileDescriptor *writefd);
