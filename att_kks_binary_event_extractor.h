#ifndef ATT_KKS_BINARY_EVENT_EXTRACTOR_H
#define ATT_KKS_BINARY_EVENT_EXTRACTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "att_word_extractor.h"
#include "att_kks_analog_trend_extractor.h"

#define LINE_NOT_VALID		"[OTHER]"

struct binary_event
{
	int index;
	struct binary_event* prev;
	struct binary_event* next;
	struct binary_event_extractor* upr;
	char* datetime_str;
	char* kks;
	char* description;
	char* evt_type;
	char* prio;
	char* val;
	char* val_qlty;
	char* usern;
	char* eng_unit;
	int seqmod;
};


struct binary_event_extractor
{
	struct word_extractor* source_data;
	struct content_word* all_content_data;
	struct binary_event* contents;
	char plant_unit;
};

struct kks_occurence
{
	struct kks_occurence* prev;
	struct kks_occurence* next;
	struct kks_event_search* upr;
	int index;
	char* time_occurence;
};

struct kks_event_search
{
	char* kks;
	char* signal;
	int total_occurence;
	struct kks_occurence* occurence;
	struct binary_event_extractor* binary_data;
};

struct kks_occurence* kks_occurence_new(void);
int kks_occurence_add (struct kks_event_search*,
	struct kks_occurence*);
struct kks_event_search*
	kks_event_search_new (struct binary_event_extractor*, char*, char*);
int kks_event_search_event (struct kks_event_search*);
char* kks_event_search_get_time_occurence
	(struct kks_event_search*, int);
int kks_event_search_get_total_event_occurence
	(struct kks_event_search*);
struct binary_event_extractor* binary_event_extractor_new(char*);
char binary_event_extractor_get_plant_unit (struct binary_event_extractor*);
int binary_event_extractor_read_data(struct binary_event_extractor*);
struct binary_event* binary_event_new(void);
int binary_event_add (struct binary_event_extractor*, struct binary_event*);
char* binary_event_get_time(struct binary_event*);
int binary_event_extractor_print(struct binary_event_extractor*);
int binary_event_extractor_print_file(struct binary_event_extractor*);
int binary_event_extractor_print_file_formatted
		(struct binary_event_extractor*);
int binary_event_extractor_read_data_operation_sequence
	(struct binary_event_extractor*);
int apakah_binary_chunk_sama(struct binary_event*, struct binary_event*);
int binary_event_extractor_apakah_operation_binary(struct content_word*);
int binary_event_extractor_apakah_event_binary(struct content_word*);

#endif

