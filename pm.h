#ifndef PM_H
#define PM_H

#include <stddef.h>  // for size_t

extern char proj_path[];
extern char folder_prefix[];

// Utilities
char *dynstrcat(char *str1, const char *str2);

// Operations
int  newproj(const char *path, const char *folder_name, const char *ext);
int  viewdir(const char *base_path, int indent_level, int recursive);
int  removedir(const char *base_path);
void listdir(const char *base_path);
void enterdir(const char *base_path, int t);

#endif
