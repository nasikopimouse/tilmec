#ifndef MULTI_FILE_H
#define MULTI_FILE_H

#include "att_word_extractor.h"
#include "att_kks_analog_trend_extractor.h"
#include "att_kks_binary_event_extractor.h"

struct binary_event_extractor* binary_event_extractor_multi_new (void);
int binary_event_extractor_join
	(struct binary_event_extractor*, struct binary_event_extractor*);


	
struct kks_analog_trend_list* kks_analog_trend_list_multi_new(void);
int kks_analog_trend_list_add
	(struct kks_analog_trend_list*, struct kks_analog_trend_list*);
	
int content_word_join (struct content_word*, struct content_word*);
int kks_analog_trend_data_join
	(struct kks_analog_trend*, struct kks_analog_trend_data*);

int analog_binary_joint_print_file
	(struct kks_analog_trend_list*,
		struct binary_event_extractor*);
int analog_binary_joint_print_file_gold
	(struct kks_analog_trend_list*,
		struct binary_event_extractor*);
int analog_binary_joint_print_file_gold_mobile
	(struct kks_analog_trend_list*,
		struct binary_event_extractor*);
int susd(void);
int susd_mobile(void);
int excel_merge(void);
int tmline_urutkan_waktu (const void*, const void*);
int tmline_urutkan_waktu_binary (const void*, const void*);
int analog_merge_print_file(struct kks_analog_trend_list*);
int binary_merge_print_file(struct binary_event_extractor*);
char* ctime_hilangkan_newline(char*);

#endif
