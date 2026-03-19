#ifndef FS_H
#define FS_H

#include "types.h"

#define MAX_FILES 16
#define MAX_FILENAME 32
#define MAX_FILE_SIZE 512

void fs_init(void);
int fs_create(const char *name, const char *data);
int fs_read(const char *name, char *buf);
int fs_delete(const char *name);
void fs_list(void);

#endif