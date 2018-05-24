#ifndef ATT_H
#define ATT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dcs_time.h"
#include "att_kks_analog_trend_extractor.h"
#include "att_kks_binary_event_extractor.h"

#define ATT_RTPM_ATT_SIGNAL					"5 0MAY01 EC200||XS01"
#define ATT_RTPM_ATT_SIGNAL_STATUS			"STEP 01"
#define STOP_VALVE_OPEN						"1 FG051||Q1"
#define STOP_VALVE_CLOSE					"1 FG051||Q2"
#define CONTROL_VALVE_OPEN					"2 FG151||Q1"
#define CONTROL_VALVE_CLOSE					"2 FG151||Q2"
#define ESV_TRIP_VALVE_1					"1 AA013||XB02"
#define ESV_TRIP_VALVE_2					"1 AA014||XB02"
#define CV_TRIP_VALVE_1						"2 AA013||XB02"
#define CV_TRIP_VALVE_2						"2 AA014||XB02"
#define STOP_VALVE_TIME_OK					"1 AA001||XT10"
#define CONTROL_VALVE_TIME_OK				"2 AA001||XT10"
#define NOT_OPEN							"[OPEN]"
#define CLOSE								"CLOSED"
#define CV_CLOSE							"[N. CLOSED]"
#define CLOSE_TIME_OK						"OK"
#define RATE_K								25

/* Tidak perlu membuat objek baru, karena
timing adalah bagian dari stroke_valve_detector */
struct timings
{
	long esv_trip_sol_1;
	int es_1_status;
	char* es_1_time_str;
	char* es_1_open_signal;
	char* es_1_close_signal;
	long esv_trip_sol_2;
	int es_2_status;
	char* es_2_time_str;
	char* es_2_open_signal;
	char* es_2_close_signal;
	long cv_trip_sol_1;
	int cs_1_status;
	char* cs_1_time_str;
	char* cs_1_open_signal;
	char* cs_1_close_signal;
	long cv_trip_sol_2;
	int cs_2_status;
	char* cs_2_time_str;
	char* cs_2_open_signal;
	char* cs_2_close_signal;
	int att_sukses, att_sv_sukses, att_cv_sukses;
};

/* input kks dan data terkait, output time start dan time end,
dan valve timing. Dari nama kks cv sudah bisa dipastikan
apa nama kks lainnya yang diperlukan untuk assessment
data output dijadikan dasar untuk mengambil nilai dari trend
--> dari input kks bisa ditentukan mana tag trip solenoid,
apakah OK, dsb
*/
struct att_valve_stroke_detector
{
	char* time_start;
	char* time_end;
	char* time_init;
	char* kks;
	struct timings valve_timing;

	struct kks_analog_trend_list* data;
	struct binary_event_extractor* data_binary;
		/* Untuk mengecek apakah stroke benar2 adalah ATT */
};

/* input adalah waktu start dan waktu end, output adalah
nilai maks, nilai min, dan nilai average */
struct att_values
{
	char* time_init;
	char* time_start;
	char* time_end;
	char* kks;
	char* description;
	double init_val;
	char* time_at_init_str;
	double max_val;
	char* time_at_max_str;
	double min_val;
	char* time_at_min_str;
	double avg_val;
	struct kks_analog_trend_list* data;	
};

struct att_valve_timing
{
	struct binary_event* sol_v_close;
	struct binary_event* valv_not_open;
	struct binary_event* valv_close;
	struct binary_event* time_ok;
	double timing;
	int status_ok;
	struct binary_event_extractor* data;
};

struct att_valve_stroke_detector* att_valve_stroke_detector_new
	(struct kks_analog_trend_list*, struct binary_event_extractor*, char*);
int att_valve_stroke_detector_find(struct att_valve_stroke_detector*);
int timings_find (struct att_valve_stroke_detector*);
/* apakah data adalah rtpm att  */
int att_apakah_rtpm_att (struct att_valve_stroke_detector*); 
/* apakah stroke valve adalah bagian dari att */
int att_apakah_stroke_valve_att (struct att_valve_stroke_detector*); 
struct att_values* att_values_new (struct kks_analog_trend_list*, char*, char*, char*, char*);
int att_values_find (struct att_values*);
int att_sgc_ketemu(struct binary_event*);
int att_main (char*, char*);
int file_open_error (char*);

#endif
