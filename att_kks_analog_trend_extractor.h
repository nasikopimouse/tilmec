#ifndef ATT_KKS_ANALOG_TREND_EXTRACTOR_H
#define ATT_KKS_ANALOG_TREND_EXTRACTOR_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "att_word_extractor.h"
#include "att_kks_analog_trend_extractor.h"

#define END_WORD							"Page"
#define KKS_SIGNATURE						"||X"
#define TAG_SIGNATURE						"Tag"
#define TIME_WORD							"Time"
#define FILE_SIGNATURE_T3000				"SPPA-T3000"
#define FILE_SIGNATURE_ANALOG				"Analog Grid Report"
#define FILE_SIGNATURE_BINARY				"Event Sequence Report"
#define FILE_SIGNATURE_BINARY_OPERATION_SEQUENCE "Operation Sequence Report"
#define BINARY_EVENT_SEQUENCE		100
#define BINARY_OPERATION_SEQUENCE	200

#define TANGGALWAKTU_PANJANG_STRING				23
#define TANGGALWAKTU_AMPM_PANJANG_STRING		26


struct tmline
{
	struct tmline* prev;
	struct tmline* next;
	int index;
	char* time_str;
};

struct kks_analog_trend_list
{
	struct word_extractor* source_data;
	struct kks_analog_trend* list_content;
	struct content_word* all_content_data;
	struct tmline* timeline;
	char plant_unit;
};

struct kks_analog_trend
{
	int index;
	struct kks_analog_trend* next;
	struct kks_analog_trend* prev;
	char* kks;
	char* description;
	char* eng_unit;
	char* time_begin;
	char* time_end;
	double max_val;
	char* max_time;
	double min_val;
	char* min_time;
	double avg_val;
	int tag_pos;
	struct kks_analog_trend_data* data;
	struct word_extractor* source_data;
	struct kks_analog_trend_list* kks_list;
};

struct kks_analog_trend_data
{
	struct kks_analog_trend_data* prev;
	struct kks_analog_trend_data* next;
	struct kks_analog_trend* upr;
	struct kks_analog_trend* lwr;
	int index;
	char* kks; /* ambil pointer dari struct kks_analog_trend  */
	char* time_str;
	char* description;
	char* eng_unit;
	char* val;
	double val_f;
};

struct kks_analog_trend_data_wrapper
{
	struct kks_analog_trend_data_wrapper* prev;
	struct kks_analog_trend_data_wrapper* next;
	struct kks_analog_trend_data_wrapper* upr;
	struct kks_analog_trend_data_wrapper* lwr;
	int index;
	struct kks_analog_trend_data* data;
};


char unit_manakah (char*);
int apakah_ada (char*, char*);
int apakah_kks (char*);
int apakah_tag (char*);
int apakah_datetime(char*);
 // int apakah_datetime_binary(char*);
char* kks_to_fnam (char*);
char* datetime_binary_correcting(char*);
int konversi_ampm_ke_biasa(char*);
char* ambil_tanggal_saja (char*);
char* ambil_waktu_tanpa_mili (char*);
char* bulan_angka_ke_huruf(char*);
char* tanggal_buat_fnam(char*);
int kks_analog_trend_apakah_analog_grid(struct content_word*);
struct kks_analog_trend_list* kks_analog_trend_list_new
	(struct word_extractor*);
char kks_analog_trend_list_get_plant_unit (struct kks_analog_trend_list*);
struct kks_analog_trend* kks_analog_trend_new
	(struct word_extractor*, char*, struct kks_analog_trend_list*);
int kks_analog_trend_add (struct kks_analog_trend_list*,
	struct kks_analog_trend*);
struct kks_analog_trend_data* kks_analog_trend_data_new(void);
int kks_analog_trend_data_add(struct kks_analog_trend*,
	struct kks_analog_trend_data*);
int kks_analog_trend_search_data (struct kks_analog_trend*);
int kks_analog_trend_print(struct kks_analog_trend*);
int kks_analog_trend_print_file (struct kks_analog_trend*);
int kks_analog_trend_print_one_file (struct kks_analog_trend_list*);
int kks_analog_trend_set_kks(struct kks_analog_trend*, char*);
int kks_analog_trend_set_description(struct kks_analog_trend*, char*);
struct kks_analog_trend* kks_analog_trend_find_kks
	(struct kks_analog_trend_list*, char*);
struct kks_analog_trend* kks_analog_trend_find_time
	(struct kks_analog_trend_list*, char*, char*);
struct kks_analog_trend_data_wrapper* 
kks_analog_trend_wrapper_get_data_at
	(struct kks_analog_trend_list, char* datetime);
struct kks_analog_trend_data_wrapper*
kks_analog_trend_data_wrapper_new(void);
/* add = menambah kolom, prev-next  */
int kks_analog_trend_data_wrapper_add
	(struct kks_analog_trend_data_wrapper*,
	struct kks_analog_trend_data_wrapper*);
/* join = menambah anggota masing-masing kolom, upr-lwr */
int kks_analog_trend_data_wrapper_join
	(struct kks_analog_trend_data_wrapper*,
	struct kks_analog_trend_data_wrapper*);
struct kks_analog_trend_data_wrapper*
	kks_analog_trend_data_wrapper_find_from_kks
	(struct kks_analog_trend_data_wrapper* data,
	char* kks);
int kks_analog_trend_data_wrapper_delete
	(struct kks_analog_trend_data_wrapper*);
char* kks_analog_trend_find_time_start (struct kks_analog_trend*);
struct kks_analog_trend_data*
	kks_analog_trend_find_time_start_chunk (struct kks_analog_trend*);
char* kks_analog_trend_get_time_ptr (struct kks_analog_trend_data*);
char* kks_analog_trend_find_time_pair
	(struct kks_analog_trend*, char*,
	double, char*);	
struct kks_analog_trend_data*
kks_analog_trend_find_time_pair_chunk
	(struct kks_analog_trend*, char*,
	double);	
char* kks_analog_trend_consistent_decrease
	(struct kks_analog_trend_data*, char*,
	double, char*);
struct kks_analog_trend_data* kks_analog_trend_consistent_decrease_chunk
	(struct kks_analog_trend_data*, char*,
	double);
struct kks_analog_trend_data_wrapper* kks_analog_trend_get_data_at
	(struct kks_analog_trend_list*, char*);
double kks_analog_trend_data_wrapper_get_value
	(struct kks_analog_trend_data_wrapper*);
struct kks_analog_trend_data*
kks_analog_trend_data_get_data_at_next
	(struct kks_analog_trend_data*, double);
struct tmline* tmline_new(void);

#endif
