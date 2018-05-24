#ifndef FNAM_HANDLING_H
#define FNAM_HANDLING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dir.h>

#define ANALOG 10
#define BINARY 11

struct file_list
{
	int index;
	struct file_list* prev;
	struct file_list* next;
	int file_mode;  /* analog atau binary  */
	char file_name[MAXPATH];
};

int sample_main(void);
struct file_list* file_list_new1(void);
struct file_list* file_list_new(int);
int file_list_add (struct file_list*, struct file_list*);
int apakah_file_analog(char* fnam);
int apakah_file_binary(char*);
struct file_list* file_list_auto_new (int);

#endif
