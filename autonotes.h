#ifndef AUTONOTE_H
#define AUTONOTE_H
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "att_word_extractor.h"
#include "att_kks_analog_trend_extractor.h"


#define FNAM "autonotes.csv"

struct autonote
{
	int index;
	struct autonote* prev;
	struct autonote* next;
	char* kks;
	char* description;
	char* val;
	struct content_word* data;
};

struct autonote* autonote_new (void);
struct autonote* autonote_read_file (void);
int autonote_print_data(struct autonote*);
int autonote_print_example_file(void);
char* autonote_find_note(struct autonote*, char*, char*);

#endif

