#ifndef DCS_TIME_H
#define DCS_TIME_H
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "att_kks_analog_trend_extractor.h"

/*
is_str1_bef_str2 adalah variabel untuk mengetahui status
input waktu. Jika -1 maka input belum dihitung.
Jika 1 maka str1 sebelum str2. Jika 0, maka
str1 sesudah str2. Jika selain millisecond
str1 = str2, maka hasilnya adalah 2.
Setiap memasukkan input angka baru, maka akan reset ke -1.
diff_time_str ditulis sama seperti format DCS
*/

struct waktu_detail
{
	long tahun, bulan, tanggal, jam, menit, detik, mili;
};

struct dcs_time
{
	char* time_str_1;
	char* time_str_2;
	char* diff_time_str;
	long jam_result, menit_result,
		detik_result, mili_result;
	long total_mili_result;
	int is_str1_bef_str2;
	int is_input_changed;
	struct waktu_detail* waktu_detail_1;
	struct waktu_detail* waktu_detail_2;
};

int dcs_time_delete (struct dcs_time*);
struct dcs_time* dcs_time_new(void);
long dcs_time_get_diff_time (struct dcs_time*);
int dcs_time_get_diff_time_millisec(struct dcs_time*);
int dcs_time_get_diff_time_sec(struct dcs_time*);
int dcs_time_set_timestr_1(struct dcs_time*, char*);
int dcs_time_set_timestr_2(struct dcs_time*, char*);
int get_millisecond (char*);
int dcs_time_print (struct dcs_time*);
long dcs_time_get_jam_result (struct dcs_time*);
long dcs_time_get_menit_result (struct dcs_time*);
long dcs_time_get_detik_result (struct dcs_time*);
long dcs_time_get_mili_result (struct dcs_time*);
int dcs_time_parse_waktu (struct waktu_detail*, char*);
char* dcs_time_waktu_detail_to_str (char*, struct waktu_detail*);
char* add_minutes (char*, int);
int dcs_time_get_status (struct dcs_time*);
int apakah_waktu_urut (char*, char*);

#endif
