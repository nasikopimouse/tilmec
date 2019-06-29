#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "att.h"
#include "dcs_time.h"
#include "att_kks_analog_trend_extractor.h"
#include "att_kks_binary_event_extractor.h"
#include "att_word_extractor_main.h"
#include "att_word_extractor.h"
#include "att.h"

char* kks_analog1 [] =
{
	"5 0MKA01 CE901||XQ01",
	"5 0HJA00 EY001||XQ09",
	"5 0HJA00 EY001||XV54",
	"5 0LBA30 CT001||XQ01",
	"5 0LBA30 CP901||XQ01",
	"5 0LBA30 CF901||XQ01",
	"5 0LBB11 CT903||XQ01",
	"5 0LBB12 CT903||XQ01",
	"5 0MAA12 FG151||XQ01",
	"5 0MAA22 FG151||XQ01",
	"5 0MAB12 FG151||XQ01",
	"5 0MAB22 FG151||XQ01",
	"5 0MKA01 CE005||XQ01",
	"5 0CJA04 DU001||XQ03"
};

/*
reset_millisec()
Format tanggal dcs pada parameter milidetik akan reset ke 0.
Jika berhasil, akan mengembalikan pointer ke data.
Jika parameter adalah NULL atau data yang dimasukkan
bukan tanggal dcs, maka akan mengembalikan nilai NULL.
*/
char* reset_millisec (char* data)
{
	char* data1 = data;
	int i;
	if (data == NULL || apakah_datetime (data) != 1)
		return NULL;
	for (i = 20; i< 23; i++)
	{
		memset (data1+i, '0', 1);
	}
	return data;
}

/*
att_sgc_ketemu()
Akan mengecek keberadaan signal SGC ATT
pada data binary. Jika signal ada,
maka akan mengembalikan nilai 1,
jika tidak ada, akan mengembalikan
nilai 0, jika parameter yang dimasukkan
bermasalah, maka akan mengembalikan nilai
-1;
*/
int att_sgc_ketemu(struct binary_event* binary_d)
{
	char* data_kks;
	char* data_val;
	if (binary_d == NULL)
		return -1;
	data_kks = binary_d->kks;
	data_val = binary_d->val;
	if (data_kks == NULL || data_val == NULL)
		return -1;
	if (!strcmp (data_kks+2, ATT_RTPM_ATT_SIGNAL)
		&& !strcmp (data_val, ATT_RTPM_ATT_SIGNAL_STATUS))
		return 1;
	return 0;
}

/*
att_valve_stroke_detector_find()
Mencari di trend analog apakah
terjadi stroke pada valve dan
mengecek apakah strokenya karena ATT
atau karena proses lain.
return 0, operasi sukses
return -1, hanya ditemukan nilai NULL
return -2, data yang diharapkan tidak tersedia
return -3, bukan ATT
*/
int att_valve_stroke_detector_find
	(struct att_valve_stroke_detector* arg)
{
	struct kks_analog_trend_data* data;
	// struct kks_analog_trend_data* data_head;
	struct kks_analog_trend_data* data1;
	struct kks_analog_trend* trend_temp;
	struct binary_event_extractor* binary_file;
	struct binary_event* binary_data;
	struct binary_event* binary_data_head;
	struct dcs_time* waktu_dcs;
	char* init_time;
	int count = 0;
	int tunggu_valve_stabil;
	int i;
	int valve_stroke = 0;
	int adalah_att = 0;
	double val, prev_val, init_val;
	if (arg == NULL)
		return -1;
	waktu_dcs = dcs_time_new();
	if (waktu_dcs == NULL)
		return -1;
	binary_file = arg->data_binary;
	binary_data = binary_file->contents;
	if (binary_file == NULL || binary_data == NULL)
		return -1;
	while (binary_data->prev != NULL)
		binary_data = binary_data->prev;
	trend_temp = arg->data->list_content;
	if (trend_temp == NULL)
		return -1;
	while (trend_temp->prev != NULL)
		trend_temp = trend_temp->prev;
	/* Mencari apakah KKS yang dimaksud ada di list  */
	while (trend_temp != NULL)
	{
		if (!strcmp(trend_temp->kks, arg->kks))
		{
			data = trend_temp->data;
			break;
		}
		trend_temp = trend_temp->next;
	}
	if (data == NULL)
		return -2; /* KKS tidak ada di list  */
	while (data->prev != NULL)
		data = data->prev;
	// data_head = data;
	while (binary_data->prev != NULL)
		binary_data = binary_data->prev;
	// binary_data_head = binary_data;
	if (data != NULL)
	{
		while (binary_data!= NULL)
		{
			/* mengkonfirmasi apakah sedang running sgc ATT */
			if (att_sgc_ketemu(binary_data))
			{
				adalah_att = 1;
				init_time = binary_event_get_time(binary_data);
				init_time = datetime_binary_correcting (init_time);
				init_time = reset_millisec (init_time);
				arg->time_init = (char*) malloc (1024);
				memset (arg->time_init, '\0', 1024);
				strcpy (arg->time_init, init_time);
				dcs_time_set_timestr_1(waktu_dcs, init_time);
				break;
			}
			binary_data = binary_data->next;
		}
		if (adalah_att == 0)
		{
			// return -3; /* bukan ATT  */
			printf ("\n\nSelection KKS ATT ada di petunjuk. Ketik <nama_program> -h.\nProgram dihentikan. Resume tidak berhasil dibuat.\n");
			exit(-1);
		}
		tunggu_valve_stabil = 0;
		while (data!= NULL)
		{
			/* buat space sementara untuk pelacakan rate  */
			data1 = data;
			val = data1->val_f;
			init_val = val;
			/*  rate detector
				mulai bekerja jika ada perbedaan
				nilai antar slot waktu */
			prev_val = val;
			for (i = 0; i< RATE_K; i++)
			{
				if (data1->next!= NULL)
					data1 = data1->next;
			}
			val = data1->val_f;
			/* valve stroking close */
			if ((((val-prev_val)/RATE_K)<-0.8)&&
				(((val-prev_val)/RATE_K)>-1.3)&&
				valve_stroke == 0 &&
				val < (init_val - RATE_K))
			{
				valve_stroke = 1;
				if (arg->time_start == NULL)
					arg->time_start = (char*) malloc (1024);
				memset (arg->time_start, '\0', 1024);
				strcpy (arg->time_start,data->time_str);
				data = data1;
			}
			/* valve stroking open  */
			if ((((val-prev_val)/RATE_K)>0.8)&&
				(((val-prev_val)/RATE_K)<1.3)&&
				valve_stroke == 1 &&
				val >= init_val
				)
			{
				for (i = 0; i< RATE_K; i++)
				{
					if (data->next!= NULL)
						data = data->next;
				}
				tunggu_valve_stabil = 1;
			}
			/* ATT selesai, valve bergerak open.
				Tunggu valve stabil  */

			if (tunggu_valve_stabil == 1 &&
				(((val-prev_val)/RATE_K)<0.1) &&
				 val > init_val - 1 &&
				 val < init_val + 1)
			{
				if (count == 3)
				{
					if (arg->time_end == NULL)
						arg->time_end = (char*) malloc (1024);
					memset (arg->time_end, '\0', 1024);
					strcpy (arg->time_end, data->time_str);
					/* Jika stroke bukan karena ATT, ulangi  */

					if (timings_find (arg) < 0)
					{

						count = 0;
						valve_stroke = 0;
						tunggu_valve_stabil = 0;
						if (data->next==NULL)
							return -3; /* Bukan ATT */
						continue;
					}
					else

						break;
				}
				count++;
			}
			data = data->next;
		}
	}

	return 0;
}

/*

return -1 jika data NULL
*/
int timings_find (struct att_valve_stroke_detector* arg)
{
	char kks_main_tag[1024];
	char buf_kks[1024];
	char buf_signal[1024];
	char buf_time[8][1024];
	char kks_temp[8][1024];
	char kks_trip_valve[4][1024];
	char kks_valve_ok[2][1024];
	int kemunculan[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int kemunculan_valve_ok[2] = {0, 0};
	struct dcs_time* waktu_dcs;
	int indeks_begin = -1, indeks_end = -1;
	int k, l = 0;
	unsigned int spasi, spasi_total = 35;
	struct binary_event* data;
	struct binary_event* data_main;
	int i, j, trip, selisih_waktu,
		att_cuk = 0;
	int binary_sesuai;
	int start_berhitung_waktu = 0;
	/* kalkulasi waktu dengan engine dcs_time */
	waktu_dcs = dcs_time_new();
	if (waktu_dcs == NULL)
		return -1;

	data = arg->data_binary->contents;
	if (data == NULL)
		return -1;
	if (data->prev != NULL)
		data = data->prev;
	/* ini diperlukan untuk mencegah membaca chunk NULL */
	data_main = data;
	memset (buf_kks, 0, 1024);
	memset (buf_signal, 0, 1024);
	memset (kks_main_tag, 0, 1024);
	for (i=0; i<8; i++)
		memset (buf_time[i], 0, 1024);
	strcpy(kks_main_tag, arg->kks);
	/*
	ekstrak bagian main tag dari kks (5 0MAA1).
	Bagian selanjutnya akan ditambahkan kemudian.
	*/
	memset (kks_main_tag+7, 0, 1);
	for (i=0; i<8; i++)
	{
		memset (kks_temp[i], 0, 1024);
	}
	for (j = 0; j<8; j+=2)
		strcpy (kks_temp[j], kks_main_tag);
	for (i=0; i<2; i++)
	{
		memset (kks_valve_ok[i], 0, 1024);
		strcpy (kks_valve_ok[i], kks_main_tag);
	}

	for (i=0; i<4; i++)
	{
		memset (kks_trip_valve[i], 0, 1024);
		strcpy (kks_trip_valve[i], kks_main_tag);
	}
	strcat(kks_temp[0], STOP_VALVE_OPEN);
	strcpy(kks_temp[1], NOT_OPEN);
	strcat(kks_temp[2], STOP_VALVE_CLOSE);
	strcpy(kks_temp[3], CLOSE);
	strcat(kks_temp[4], CONTROL_VALVE_OPEN);
	strcpy(kks_temp[5], NOT_OPEN);
	strcat(kks_temp[6], CONTROL_VALVE_CLOSE);
	strcpy(kks_temp[7], CV_CLOSE);
	strcat(kks_trip_valve[0], ESV_TRIP_VALVE_1);
	strcat(kks_trip_valve[1], ESV_TRIP_VALVE_2);
	strcat(kks_trip_valve[2], CV_TRIP_VALVE_1);
	strcat(kks_trip_valve[3], CV_TRIP_VALVE_2);
	strcat(kks_valve_ok[0], STOP_VALVE_TIME_OK);
	strcat(kks_valve_ok[1], CONTROL_VALVE_TIME_OK);
	/*
	Mencari indeks pada data binary yang
	sesuai dengan waktu start dan waktu
	stop yang sudah ditemukan oleh
	stroke valve detector
	*/
	data = data_main;
	binary_sesuai = 0;
	while (data->prev != NULL)
		data = data->prev;
	/* Mencari titik start */
	dcs_time_set_timestr_1 (waktu_dcs, arg->time_start);
	while (data!= NULL)
	{
		dcs_time_set_timestr_2 (waktu_dcs, data->datetime_str);
		if (dcs_time_get_diff_time (waktu_dcs)> 0)
		{
			data = data->prev;
			indeks_begin = data->index;
			binary_sesuai = 1;
			break;
		}
		data = data->next;
	}
	if (binary_sesuai != 1)
	{
		printf
			("Waktu data analog dan binary tidak sama. Program dihentikan.\n");
		exit (-1);
	}
	/* mencari titik end  */
	data = data_main;
	while (data->prev != NULL)
		data = data->prev;
	dcs_time_set_timestr_1 (waktu_dcs, arg->time_end);
	while (data!=NULL)
	{
		dcs_time_set_timestr_2 (waktu_dcs, data->datetime_str);
		if (dcs_time_get_diff_time (waktu_dcs)> 0)
		{
			indeks_end = data->index;
			break;
		}
		data = data->next;
	}
	data = data_main;
	while (data->prev != NULL)
		data = data->prev;
	/*
	memastikan apakah att dengan membaca
	apakah timing trip valve adalah att
	dicek dari event binary
	*/
	trip = 0;
	selisih_waktu = 0;
	data = data_main;
	while (data->prev != NULL)
		data = data->prev;
	while (data != NULL)
	{
		if (data->index < indeks_begin)
		{
			data = data->next;
			continue;
		}
		if (data->index > indeks_end)
			break;
		strcpy(buf_kks, data->kks);
		strcpy(buf_signal, data->val);
		if (att_cuk == 0)
		{
			for (i=0; i<4; i++)
			{
				if (!strcmp(kks_trip_valve[i], data->kks)
					&& !strcmp (data->val, CLOSE))
					trip++;
				if (start_berhitung_waktu == 0)
				{
					start_berhitung_waktu = 1;
					dcs_time_set_timestr_1 (waktu_dcs, data->datetime_str);
				}
				dcs_time_set_timestr_2 (waktu_dcs, data->datetime_str);
				selisih_waktu += dcs_time_get_diff_time(waktu_dcs);
				dcs_time_set_timestr_1 (waktu_dcs, data->datetime_str);
			}
		}
		data = data-> next;
	}
	if (trip == 4 && selisih_waktu/4 > 500)
		att_cuk = 1;
	else
		return -3; /* Bukan ATT  */
	data = data_main;
	while (data->prev != NULL)
		data = data->prev;
	while (data != NULL)
	{
		/* Batasi pencarian hanya pada range
		yang diinginkan saja   */
		if (data->index < indeks_begin)
		{
			data = data->next;
			continue;
		}
		if (data->index > indeks_end)
			break;
		strcpy(buf_kks, data->kks);
		strcpy(buf_signal, data->val);
		if (att_cuk == 1)
		{
			/* sol 1, stop valve not open    */
			if (!strcmp(buf_kks, kks_temp[0])
				&&!strcmp (buf_signal, kks_temp[1])
				&& kemunculan[0] == 0)
			{
				kemunculan[0] = 1;
				strcpy (buf_time[0], data->datetime_str);
				arg->valve_timing.es_1_open_signal = (char*) malloc (1024);
				memset (arg->valve_timing.es_1_open_signal, 0, 1024);
				strcpy (arg->valve_timing.es_1_open_signal, data->datetime_str);
				strcat (arg->valve_timing.es_1_open_signal, " ");
				strcat (arg->valve_timing.es_1_open_signal, data->val);
				strcat (arg->valve_timing.es_1_open_signal, " ");
				strncat (arg->valve_timing.es_1_open_signal, data->description, 20);
				for (spasi = 0; spasi < (spasi_total-( strlen ( data->description) + strlen (data->val ))); spasi++)
					strcat (arg->valve_timing.es_1_open_signal, "-");
				strcat (arg->valve_timing.es_1_open_signal, data->kks);
			}
			/* sol 2, stop valve not open    */
			else if (!strcmp(buf_kks, kks_temp[0])
				&&!strcmp (buf_signal, kks_temp[1])
				&& kemunculan[0] == 1
				&& kemunculan[1] == 0)
			{
				kemunculan[1] = 1;
				strcpy (buf_time[1], data->datetime_str);
				arg->valve_timing.es_2_open_signal = (char*) malloc (1024);
				memset (arg->valve_timing.es_2_open_signal, 0, 1024);
				strcpy (arg->valve_timing.es_2_open_signal, data->datetime_str);
				strcat (arg->valve_timing.es_2_open_signal, " ");
				strcat (arg->valve_timing.es_2_open_signal, data->val);
				strcat (arg->valve_timing.es_2_open_signal, " ");
				strncat (arg->valve_timing.es_2_open_signal, data->description, 20);
				for (spasi = 0; spasi < (spasi_total-( strlen ( data->description) + strlen (data->val ))); spasi++)
					strcat (arg->valve_timing.es_2_open_signal, "-");
				strcat (arg->valve_timing.es_2_open_signal, data->kks);
			}
			/* sol 1, stop valve close  */
			if (!strcmp(buf_kks, kks_temp[2])
				&&!strcmp (buf_signal, kks_temp[3])
				&& kemunculan[2] == 0)
			{
				kemunculan[2] = 1;
				strcpy (buf_time[2], data->datetime_str);
				dcs_time_set_timestr_1(waktu_dcs, buf_time[0]);
				dcs_time_set_timestr_2(waktu_dcs, buf_time[2]);
				arg->valve_timing.esv_trip_sol_1 =
					dcs_time_get_diff_time (waktu_dcs);
				arg->valve_timing.es_1_close_signal = (char*) malloc (1024);
				memset (arg->valve_timing.es_1_close_signal, 0, 1024);
				strcpy (arg->valve_timing.es_1_close_signal, data->datetime_str);
				strcat (arg->valve_timing.es_1_close_signal, " ");
				strcat (arg->valve_timing.es_1_close_signal, data->val);
				strcat (arg->valve_timing.es_1_close_signal, " ");
				strncat (arg->valve_timing.es_1_close_signal, data->description, 20);
				for (spasi = 0; spasi < (spasi_total-( strlen ( data->description) + strlen (data->val ))); spasi++)
					strcat (arg->valve_timing.es_1_close_signal, "-");
				strcat (arg->valve_timing.es_1_close_signal, data->kks);
			}
			/* sol 2, stop valve close    */
			else if (!strcmp(buf_kks, kks_temp[2])
				&&!strcmp (buf_signal, kks_temp[3])
				&& kemunculan[2] == 1
				&& kemunculan[3] == 0)
			{
				kemunculan[3] = 1;
				strcpy (buf_time[3], data->datetime_str);
				dcs_time_set_timestr_1(waktu_dcs, buf_time[1]);
				dcs_time_set_timestr_2(waktu_dcs, buf_time[3]);
				arg->valve_timing.esv_trip_sol_2 =
					dcs_time_get_diff_time (waktu_dcs);
				arg->valve_timing.es_2_close_signal = (char*) malloc (1024);
				memset (arg->valve_timing.es_2_close_signal, 0, 1024);
				strcpy (arg->valve_timing.es_2_close_signal, data->datetime_str);
				strcat (arg->valve_timing.es_2_close_signal, " ");
				strcat (arg->valve_timing.es_2_close_signal, data->val);
				strcat (arg->valve_timing.es_2_close_signal, " ");
				strncat (arg->valve_timing.es_2_close_signal, data->description, 20);
				for (spasi = 0; spasi < (spasi_total-( strlen ( data->description) + strlen (data->val ))); spasi++)
					strcat (arg->valve_timing.es_2_close_signal, "-");
				strcat (arg->valve_timing.es_2_close_signal, data->kks);
			}
			/* sol 1, control valve not open  */
			if (!strcmp(buf_kks, kks_temp[4])
				&&!strcmp (buf_signal, kks_temp[5])
				&& kemunculan[4] == 0&& kemunculan[3] == 1)
			{
				kemunculan[4] = 1;
				strcpy (buf_time[4], data->datetime_str);
				arg->valve_timing.cs_1_open_signal = (char*) malloc (1024);
				memset (arg->valve_timing.cs_1_open_signal, 0, 1024);
				strcpy (arg->valve_timing.cs_1_open_signal, data->datetime_str);
				strcat (arg->valve_timing.cs_1_open_signal, " ");
				strcat (arg->valve_timing.cs_1_open_signal, data->val);
				strcat (arg->valve_timing.cs_1_open_signal, " ");
				strncat (arg->valve_timing.cs_1_open_signal, data->description, 20);
				for (spasi = 0; spasi < (spasi_total-( strlen ( data->description) + strlen (data->val ))); spasi++)
					strcat (arg->valve_timing.cs_1_open_signal, "-");
				strcat (arg->valve_timing.cs_1_open_signal, data->kks);
			}
			/* sol 2, control valve not open    */
			else if (!strcmp(buf_kks, kks_temp[4])
				&&!strcmp (buf_signal, kks_temp[5])
				&& kemunculan[4] == 1&& kemunculan[3] == 1
				&& kemunculan[5] == 0)
			{
				kemunculan[5] = 1;
				strcpy (buf_time[5], data->datetime_str);
				arg->valve_timing.cs_2_open_signal = (char*) malloc (1024);
				memset (arg->valve_timing.cs_2_open_signal, 0, 1024);
				strcpy (arg->valve_timing.cs_2_open_signal, data->datetime_str);
				strcat (arg->valve_timing.cs_2_open_signal, " ");
				strcat (arg->valve_timing.cs_2_open_signal, data->val);
				strcat (arg->valve_timing.cs_2_open_signal, " ");
				strncat (arg->valve_timing.cs_2_open_signal, data->description, 20);
				for (spasi = 0; spasi < (spasi_total-( strlen ( data->description) + strlen (data->val ))); spasi++)
					strcat (arg->valve_timing.cs_2_open_signal, "-");
				strcat (arg->valve_timing.cs_2_open_signal, data->kks);
			}
			/* sol 1, control valve close  */
			if (!strcmp(buf_kks, kks_temp[6])
				&&!strcmp (buf_signal, kks_temp[7])
				&& kemunculan[6] == 0&& kemunculan[3] == 1)
			{
				kemunculan[6] = 1;
				strcpy (buf_time[6], data->datetime_str);
				dcs_time_set_timestr_1(waktu_dcs, buf_time[4]);
				dcs_time_set_timestr_2(waktu_dcs, buf_time[6]);
				arg->valve_timing.cv_trip_sol_1 =
					dcs_time_get_diff_time (waktu_dcs);
					arg->valve_timing.cs_1_close_signal = (char*) malloc (1024);
				memset (arg->valve_timing.cs_1_close_signal, 0, 1024);
				strcpy (arg->valve_timing.cs_1_close_signal, data->datetime_str);
				strcat (arg->valve_timing.cs_1_close_signal, " ");
				strcat (arg->valve_timing.cs_1_close_signal, data->val);
				strcat (arg->valve_timing.cs_1_close_signal, " ");
				strncat (arg->valve_timing.cs_1_close_signal, data->description, 20);
				for (spasi = 0; spasi < (spasi_total-( strlen ( data->description) + strlen (data->val ))); spasi++)
					strcat (arg->valve_timing.cs_1_close_signal, "-");
				strcat (arg->valve_timing.cs_1_close_signal, data->kks);
			}
			/* sol 2, control valve close   */
			else if (!strcmp(buf_kks, kks_temp[6])
				&&!strcmp (buf_signal, kks_temp[7])
				&& kemunculan[6] == 1&& kemunculan[3] == 1
				&& kemunculan[7] == 0)
			{
				kemunculan[7] = 1;
				strcpy (buf_time[7], data->datetime_str);
				dcs_time_set_timestr_1(waktu_dcs, buf_time[5]);
				dcs_time_set_timestr_2(waktu_dcs, buf_time[7]);
				arg->valve_timing.cv_trip_sol_2 =
					dcs_time_get_diff_time (waktu_dcs);
				arg->valve_timing.cs_2_close_signal = (char*) malloc (1024);
				memset (arg->valve_timing.cs_2_close_signal, 0, 1024);
				strcpy (arg->valve_timing.cs_2_close_signal, data->datetime_str);
				strcat (arg->valve_timing.cs_2_close_signal, " ");
				strcat (arg->valve_timing.cs_2_close_signal, data->val);
				strcat (arg->valve_timing.cs_2_close_signal, " ");
				strncat (arg->valve_timing.cs_2_close_signal, data->description, 20);
				for (spasi = 0; spasi < (spasi_total-( strlen ( data->description) + strlen (data->val ))); spasi++)
					strcat (arg->valve_timing.cs_2_close_signal, "-");
				strcat (arg->valve_timing.cs_2_close_signal, data->kks);
			}
			if (!strcmp(buf_kks, kks_valve_ok[0])
				&&!strcmp (buf_signal, CLOSE_TIME_OK)
				&& kemunculan_valve_ok[0] == 0)
			{
				kemunculan_valve_ok[0] = 1;
				arg->valve_timing.es_1_status = 1;
				arg->valve_timing.es_1_time_str = (char*) malloc (1024);
				memset (arg->valve_timing.es_1_time_str, 0, 1024);
				strcpy (arg->valve_timing.es_1_time_str, data->datetime_str);
			}
			if (!strcmp(buf_kks, kks_valve_ok[0])
				&&!strcmp (buf_signal, CLOSE_TIME_OK)
				&& kemunculan_valve_ok[0] == 1)
			{
				arg->valve_timing.es_2_status = 1;
				arg->valve_timing.es_2_time_str = (char*) malloc (1024);
				memset (arg->valve_timing.es_2_time_str, 0, 1024);
				strcpy (arg->valve_timing.es_2_time_str, data->datetime_str);
				if ((arg->valve_timing.es_1_status +
					arg->valve_timing.es_2_status) == 2)
					arg->valve_timing.att_sv_sukses = 1;
				else
					arg->valve_timing.att_sv_sukses = 0;
			}
			if (!strcmp(buf_kks, kks_valve_ok[1])
				&&!strcmp (buf_signal, CLOSE_TIME_OK)
				&& kemunculan_valve_ok[1] == 0)
			{
				kemunculan_valve_ok[1] = 1;
				arg->valve_timing.cs_1_status = 1;
				arg->valve_timing.cs_1_time_str = (char*) malloc (1024);
				memset (arg->valve_timing.cs_1_time_str, 0, 1024);
				strcpy (arg->valve_timing.cs_1_time_str, data->datetime_str);
			}
			if (!strcmp(buf_kks, kks_valve_ok[1])
				&&!strcmp (buf_signal, CLOSE_TIME_OK)
				&& kemunculan_valve_ok[1] == 1)
			{
				arg->valve_timing.cs_2_status = 1;
				arg->valve_timing.cs_2_time_str = (char*) malloc (1024);
				memset (arg->valve_timing.cs_2_time_str, 0, 1024);
				strcpy (arg->valve_timing.cs_2_time_str, data->datetime_str);
				if ((arg->valve_timing.cs_1_status +
					arg->valve_timing.cs_2_status) == 2)
					arg->valve_timing.att_cv_sukses = 1;
				else
					arg->valve_timing.att_cv_sukses = 0;
			}
			if (kemunculan_valve_ok[0] == 1 && kemunculan_valve_ok[1] == 1)
			{
				if ((arg->valve_timing.es_1_status +
					arg->valve_timing.es_2_status +
					arg->valve_timing.cs_1_status +
					arg->valve_timing.cs_2_status ) == 4)
					arg->valve_timing.att_sukses = 1;
				else
					arg->valve_timing.att_sukses = 0;
			}
		}
		else
			return -3;
		data = data->next;
	}
	for (k = 0; k< 8; k++)
	{
		l += kemunculan[k];
	}
	if (l < 8)
		return -3;
	return 0;
}

struct att_valve_stroke_detector* att_valve_stroke_detector_new
	(struct kks_analog_trend_list* data,
		struct binary_event_extractor* data_binary,
			char* kks)
{
	struct att_valve_stroke_detector* temp;
	char* kks_temp;
	char kks_plant_unit;
	char analog_plant_unit;
	char binary_plant_unit;
	if (kks == NULL || data == NULL || data_binary == NULL)
		return NULL;
	if (apakah_kks(kks) != 1)
		return NULL;
	kks_plant_unit = unit_manakah (kks);
	analog_plant_unit = kks_analog_trend_list_get_plant_unit (data);
	binary_plant_unit = binary_event_extractor_get_plant_unit (data_binary);
	if (kks_plant_unit != analog_plant_unit ||
		analog_plant_unit != binary_plant_unit ||
		kks_plant_unit != binary_plant_unit)
		return NULL;
	kks_temp = (char*) malloc (1024);
	memset (kks_temp, '\0', 1024);
	temp = (struct att_valve_stroke_detector*) malloc
		(sizeof(struct att_valve_stroke_detector));
	if (temp == NULL)
		return NULL;
	temp->time_start = NULL;
	temp->time_end = NULL;
	strcpy(kks_temp, kks);
	temp->kks = kks_temp;
	temp->data = data;
	temp->valve_timing.esv_trip_sol_1 = temp->valve_timing.esv_trip_sol_2 =
	temp->valve_timing.cv_trip_sol_1 = temp->valve_timing.cv_trip_sol_2 = -1;
	temp->valve_timing.es_1_status = temp->valve_timing.es_2_status =
	temp->valve_timing.cs_1_status = temp->valve_timing.cs_2_status = 0;
	temp->valve_timing.es_1_time_str = NULL;
	temp->valve_timing.es_2_time_str = NULL;
	temp->valve_timing.cs_1_time_str = NULL;
	temp->valve_timing.cs_2_time_str = NULL;
	temp->valve_timing.es_1_open_signal = NULL;
	temp->valve_timing.es_1_close_signal = NULL;
	temp->valve_timing.es_2_open_signal = NULL;
	temp->valve_timing.es_2_close_signal = NULL;
	temp->valve_timing.cs_1_open_signal = NULL;
	temp->valve_timing.cs_1_close_signal = NULL;
	temp->valve_timing.cs_2_open_signal = NULL;
	temp->valve_timing.cs_2_close_signal = NULL;
	temp->valve_timing.att_sv_sukses = 0;
	temp->valve_timing.att_cv_sukses = 0;
	temp->valve_timing.att_sukses = 0;
	temp->data_binary = data_binary;
	return temp;
}

struct att_values* att_values_new
	(struct kks_analog_trend_list* data,
		char* kks, char* time_init, char* time_start, char* time_end)
{
	struct att_values* temp;
	char* kks_temp;
	char* start;
	char* end;
	char* init;
	char* init_time_filter;
	if (kks == NULL || data == NULL)
		return NULL;
	if (apakah_kks(kks) != 1)
		return NULL;
	kks_temp = (char*) malloc (1024);
	memset (kks_temp, '\0', 1024);
	start = (char*) malloc (1024);
	memset (start, '\0', 1024);
	end = (char*) malloc (1024);
	memset (end, '\0', 1024);
	init= (char*) malloc (1024);
	memset (init, '\0', 1024);
	temp = (struct att_values*) malloc
		(sizeof(struct att_values));
	if (temp == NULL)
		return NULL;
	strcpy (init, time_init);
	temp->time_init = init;
	strcpy (start, time_start);
	temp->time_start = start;
	strcpy (end, time_end);
	temp->time_end = end;
	strcpy(kks_temp, kks);
	temp->kks = kks_temp;
	temp->data = data;
	if ((init_time_filter = datetime_binary_correcting
				(temp->time_init))!=NULL)
		temp->time_init = init_time_filter;
	temp->time_at_init_str = (char*) malloc (1024);
	memset (temp->time_at_init_str, '\0', 1024);
	strcpy (temp->time_at_init_str, temp->time_init);
	temp->time_at_max_str = (char*) malloc (1024);
	memset (temp->time_at_max_str, '\0', 1024);
	temp->time_at_min_str = (char*) malloc (1024);
	memset (temp->time_at_min_str, '\0', 1024);
	temp->description = (char*) malloc (1024);
	memset (temp->description, '\0', 1024);
	return temp;
}

int att_values_find (struct att_values* arg)
{
	struct kks_analog_trend_data* data = NULL;
	struct kks_analog_trend* trend_temp;
	double min_val = 0, avg_val = 0, max_val = 0, val;
	char* time_temp;
	char* init_time_filter;
	int count =  1;
	int time_mulai = 1;
	int waktu_ketemu = 0;
	int cari_init = 1;
	if (arg == NULL)
		return -1;
	trend_temp = arg->data->list_content;
	if (trend_temp == NULL)
		return -1;
	while (trend_temp->prev != NULL)
		trend_temp = trend_temp->prev;
	while (trend_temp!= NULL)
	{
		if (!strcmp(trend_temp->kks, arg->kks))
		{
			data = trend_temp->data;
			strcpy (arg->description, trend_temp->description);
			break;
		}
		trend_temp = trend_temp->next;
	}
	if ((init_time_filter =
				datetime_binary_correcting(arg->time_init))
						!=NULL)
	{
		init_time_filter = reset_millisec(init_time_filter);
		arg->time_init = init_time_filter;
	}
	if (data != NULL)
	{
		while (data->prev != NULL)
			data = data->prev;
		while (data != NULL)
		{
			if (cari_init == 1)
			{
				if (!strcmp (data->time_str, arg->time_init))
				{
					cari_init = 0;
					arg->init_val = data->val_f;
				}
			}
			if (!strcmp (data->time_str, arg->time_start))
				waktu_ketemu = 1;
			if (waktu_ketemu == 1)
			{
				val = data->val_f;
				time_temp = data->time_str;
				if (time_mulai == 1)
				{
					min_val = data->val_f;
					max_val = data->val_f;
					strcpy (arg->time_at_min_str, time_temp);
					strcpy (arg->time_at_max_str, time_temp);
					time_mulai = 0;
				}
				if (val < min_val)
				{
					min_val = val;
					strcpy (arg->time_at_min_str, time_temp);
				}
				if (val > max_val)
				{
					max_val = val;
					strcpy (arg->time_at_max_str, time_temp);
				}
				avg_val += val;
				count++;
				if (!strcmp (data->time_str, arg->time_end))
					break;
			}
			data = data->next;
		}
		avg_val /= count;
		arg->avg_val = avg_val;
		arg->min_val = min_val;
		arg->max_val = max_val;
	}
	return 0;
}

int cetak_error (struct att_valve_stroke_detector* arg)
{
	if (arg == NULL)
	{
		printf ("Serious Error Detected! ...Maybe KKS is not same?\n");
		return -1;
	}
	return 0;
}

int file_open_error (char* fnam)
{
	printf ("File %s error. Program dihentikan.\n", fnam);
	exit (-1);
	return 0;
}

/*
Kita tidak lagi menggunakan argc dan argv.
Kita perlu menghilangkan kode yang memakai
kode ini.
*/
int att_main(char* analog_fnam, char* binary_fnam)
{
#if defined VERBOSE_OUTPUT
	char* att_result_fnam = "ATT_REPORT.txt";
#endif
	char* att_form= "ATT_REPORT_STD_FORM.txt";
	char* status_yes[] = {"   ", "Yes"};
	char* status_no[] = {"No", "  "};
	char* status_ok[] = {"No", "OK"};
	char plant_unit;
	int i, j;
	FILE* file_att_result;
	FILE* file_att_form;
	struct word_extractor* word_extractor_obj;
	struct kks_analog_trend_list* kks_list;
	struct kks_analog_trend* kks_analog;
	struct att_valve_stroke_detector* hp_turbine_1;
	struct att_valve_stroke_detector* hp_turbine_2;
	struct att_valve_stroke_detector* ip_turbine_1;
	struct att_valve_stroke_detector* ip_turbine_2;
	struct binary_event_extractor* binevt;
	struct att_values* att_val[14][4];
	

	if ((file_att_result = fopen (analog_fnam, "r"))!=NULL)
		fclose(file_att_result);
	else
		file_open_error (analog_fnam);
	if ((file_att_result = fopen (binary_fnam, "r"))!=NULL)
		fclose(file_att_result);
	else
		file_open_error (binary_fnam);
	
	/*  step 1  */
	word_extractor_obj = word_extractor_new ();
	/*  step 2  */
	word_extractor_read_file(word_extractor_obj, analog_fnam);
	/*  step 3  */
	kks_list = kks_analog_trend_list_new (word_extractor_obj);
	/*  step 4  */
	plant_unit = kks_list->plant_unit;
	/* auto plant unit selection    */
	for (i=0; i<14; i++)
		kks_analog1[i][0] = plant_unit;
	kks_analog = kks_list->list_content;
	while (kks_analog->prev!=NULL)
		kks_analog = kks_analog->prev;

#if defined VERBOSE_OUTPUT
	while (kks_analog!=NULL)
	{
		kks_analog_trend_print_file (kks_analog);
		kks_analog = kks_analog->next;
	}
#endif
	/* step 1  */
	binevt = binary_event_extractor_new(binary_fnam);
	/* step 2  */
#if defined VERBOSE_OUTPUT
	binary_event_extractor_print_file_formatted(binevt);
#endif

#if defined VERBOSE_OUTPUT
	file_att_result = fopen (att_result_fnam, "w");
	if (file_att_result == NULL)
	{
		printf ("Error operasi file. Program dihentikan.\n");
		exit (-1);
	}
	fprintf (file_att_result, "-----------------------------\n");
	fprintf (file_att_result, "U%c0 Automatic Turbine Testing\n", plant_unit);
	fprintf (file_att_result, "-----------------------------\n");
#endif



	hp_turbine_1 = att_valve_stroke_detector_new(kks_list, binevt, kks_analog1[8]);
	/* salah satu error yang dapat dideteksi adalah apakah kks sama   */
	if (cetak_error (hp_turbine_1) == -1)
		exit (-1);
	att_valve_stroke_detector_find(hp_turbine_1);

#if defined VERBOSE_OUTPUT
	fprintf (file_att_result, "\n***HP turbine 1 stroke*** %s\nStart: %s\nEnd: %s\n",
		hp_turbine_1->kks, hp_turbine_1->time_start, hp_turbine_1->time_end);
	fprintf (file_att_result, "ESV trip 1 close time: %ldms, OK stated @: %s\n",
		hp_turbine_1->valve_timing.esv_trip_sol_1, hp_turbine_1->valve_timing.es_1_time_str);
	fprintf (file_att_result, "Timing signal: %s\n", hp_turbine_1->valve_timing.es_1_open_signal);
	fprintf (file_att_result, "Timing signal: %s\n", hp_turbine_1->valve_timing.es_1_close_signal);
	fprintf (file_att_result, "ESV trip 2 close time: %ldms, OK stated @: %s\n",
		hp_turbine_1->valve_timing.esv_trip_sol_2, hp_turbine_1->valve_timing.es_2_time_str);
	fprintf (file_att_result, "Timing signal: %s\n", hp_turbine_1->valve_timing.es_2_open_signal);
	fprintf (file_att_result, "Timing signal: %s\n", hp_turbine_1->valve_timing.es_2_close_signal);
	fprintf (file_att_result, "CV trip 1 close time: %ldms, OK stated @: %s\n",
		hp_turbine_1->valve_timing.cv_trip_sol_1, hp_turbine_1->valve_timing.cs_1_time_str);
	fprintf (file_att_result, "Timing signal: %s\n", hp_turbine_1->valve_timing.cs_1_open_signal);
	fprintf (file_att_result, "Timing signal: %s\n", hp_turbine_1->valve_timing.cs_1_close_signal);
	fprintf (file_att_result, "CV trip 2 close time: %ldms, OK stated @: %s\n",
		hp_turbine_1->valve_timing.cv_trip_sol_2, hp_turbine_1->valve_timing.cs_2_time_str);
	fprintf (file_att_result, "Timing signal: %s\n", hp_turbine_1->valve_timing.cs_2_open_signal);
	fprintf (file_att_result, "Timing signal: %s\n", hp_turbine_1->valve_timing.cs_2_close_signal);
#endif

	j = 0;

	for (i=0; i<14; i++)
	{
		att_val[i][j] = att_values_new
			(kks_list, kks_analog1[i], hp_turbine_1->time_init, hp_turbine_1->time_start,
				hp_turbine_1->time_end);
		att_values_find(att_val[i][j]);
#if defined VERBOSE_OUTPUT
		fprintf (file_att_result, "KKS: %s, Description: %s\n",
			att_val[i][j]->kks, att_val[i][j]->description);
		fprintf (file_att_result, "Time: %s, Init: %f\nTime: %s, Min: %f\nTime: %s, Max: %f\nSwing: %f\n",
			att_val[i][j]->time_at_init_str, att_val[i][j]->init_val,
			att_val[i][j]->time_at_min_str, att_val[i][j]->min_val,
			att_val[i][j]->time_at_max_str, att_val[i][j]->max_val,
			att_val[i][j]->max_val-att_val[i][j]->min_val
			);
#endif
	}
	hp_turbine_2 = att_valve_stroke_detector_new(kks_list, binevt, kks_analog1[9]);
	if (cetak_error (hp_turbine_2) == -1)
		exit (-1);
	att_valve_stroke_detector_find(hp_turbine_2);
#if defined VERBOSE_OUTPUT
	fprintf (file_att_result, "\n***HP turbine 2 stroke*** %s\nStart: %s\nEnd: %s\n",
		hp_turbine_2->kks, hp_turbine_2->time_start, hp_turbine_2->time_end);

		fprintf (file_att_result, "ESV trip 1 close time: %ldms, OK stated @: %s\n",
		hp_turbine_2->valve_timing.esv_trip_sol_1, hp_turbine_2->valve_timing.es_1_time_str);
	fprintf (file_att_result, "Timing signal: %s\n", hp_turbine_2->valve_timing.es_1_open_signal);
	fprintf (file_att_result, "Timing signal: %s\n", hp_turbine_2->valve_timing.es_1_close_signal);
	fprintf (file_att_result, "ESV trip 2 close time: %ldms, OK stated @: %s\n",
		hp_turbine_2->valve_timing.esv_trip_sol_2, hp_turbine_2->valve_timing.es_2_time_str);
	fprintf (file_att_result, "Timing signal: %s\n", hp_turbine_2->valve_timing.es_2_open_signal);
	fprintf (file_att_result, "Timing signal: %s\n", hp_turbine_2->valve_timing.es_2_close_signal);
	fprintf (file_att_result, "CV trip 1 close time: %ldms, OK stated @: %s\n",
		hp_turbine_2->valve_timing.cv_trip_sol_1, hp_turbine_2->valve_timing.cs_1_time_str);
	fprintf (file_att_result, "Timing signal: %s\n", hp_turbine_2->valve_timing.cs_1_open_signal);
	fprintf (file_att_result, "Timing signal: %s\n", hp_turbine_2->valve_timing.cs_1_close_signal);
	fprintf (file_att_result, "CV trip 2 close time: %ldms, OK stated @: %s\n",
		hp_turbine_2->valve_timing.cv_trip_sol_2, hp_turbine_2->valve_timing.cs_2_time_str);
	fprintf (file_att_result, "Timing signal: %s\n", hp_turbine_2->valve_timing.cs_2_open_signal);
	fprintf (file_att_result, "Timing signal: %s\n", hp_turbine_2->valve_timing.cs_2_close_signal);
#endif
	j = 1;

	for (i=0; i<14; i++)
	{
		att_val[i][j] = att_values_new
			(kks_list, kks_analog1[i], hp_turbine_2->time_init, hp_turbine_2->time_start,
				hp_turbine_2->time_end);
		att_values_find(att_val[i][j]);
#if defined VERBOSE_OUTPUT
		fprintf (file_att_result, "KKS: %s, Description: %s\n",
			att_val[i][j]->kks, att_val[i][j]->description);
		fprintf (file_att_result, "Time: %s, Init: %f\nTime: %s, Min: %f\nTime: %s, Max: %f\nSwing: %f\n",
			att_val[i][j]->time_at_init_str, att_val[i][j]->init_val,
			att_val[i][j]->time_at_min_str, att_val[i][j]->min_val,
			att_val[i][j]->time_at_max_str, att_val[i][j]->max_val,
			att_val[i][j]->max_val-att_val[i][j]->min_val
			);
#endif
	}
	ip_turbine_1 = att_valve_stroke_detector_new(kks_list, binevt, kks_analog1[10]);
	if (cetak_error (ip_turbine_1) == -1)
		exit (-1);
	att_valve_stroke_detector_find(ip_turbine_1);
#if defined VERBOSE_OUTPUT
	fprintf (file_att_result, "\n***IP turbine 1 stroke*** %s\nStart: %s\nEnd: %s\n",
		ip_turbine_1->kks, ip_turbine_1->time_start, ip_turbine_1->time_end);
	fprintf (file_att_result, "ESV trip 1 close time: %ldms, OK stated @: %s\n",
		ip_turbine_1->valve_timing.esv_trip_sol_1, ip_turbine_1->valve_timing.es_1_time_str);
	fprintf (file_att_result, "Timing signal: %s\n", ip_turbine_1->valve_timing.es_1_open_signal);
	fprintf (file_att_result, "Timing signal: %s\n", ip_turbine_1->valve_timing.es_1_close_signal);
	fprintf (file_att_result, "ESV trip 2 close time: %ldms, OK stated @: %s\n",
		ip_turbine_1->valve_timing.esv_trip_sol_2, ip_turbine_1->valve_timing.es_2_time_str);
	fprintf (file_att_result, "Timing signal: %s\n", ip_turbine_1->valve_timing.es_2_open_signal);
	fprintf (file_att_result, "Timing signal: %s\n", ip_turbine_1->valve_timing.es_2_close_signal);
	fprintf (file_att_result, "CV trip 1 close time: %ldms, OK stated @: %s\n",
		ip_turbine_1->valve_timing.cv_trip_sol_1, ip_turbine_1->valve_timing.cs_1_time_str);
	fprintf (file_att_result, "Timing signal: %s\n", ip_turbine_1->valve_timing.cs_1_open_signal);
	fprintf (file_att_result, "Timing signal: %s\n", ip_turbine_1->valve_timing.cs_1_close_signal);
	fprintf (file_att_result, "CV trip 2 close time: %ldms, OK stated @: %s\n",
		ip_turbine_1->valve_timing.cv_trip_sol_2, ip_turbine_1->valve_timing.cs_2_time_str);
	fprintf (file_att_result, "Timing signal: %s\n", ip_turbine_1->valve_timing.cs_2_open_signal);
	fprintf (file_att_result, "Timing signal: %s\n", ip_turbine_1->valve_timing.cs_2_close_signal);
#endif

	j = 2;

	for (i=0; i<14; i++)
	{
		att_val[i][j] = att_values_new
			(kks_list, kks_analog1[i], ip_turbine_1->time_init, ip_turbine_1->time_start,
				ip_turbine_1->time_end);
		att_values_find(att_val[i][j]);
#if defined VERBOSE_OUTPUT
		fprintf (file_att_result, "KKS: %s, Description: %s\n",
			att_val[i][j]->kks, att_val[i][j]->description);
		fprintf (file_att_result, "Time: %s, Init: %f\nTime: %s, Min: %f\nTime: %s, Max: %f\nSwing: %f\n",
			att_val[i][j]->time_at_init_str, att_val[i][j]->init_val,
			att_val[i][j]->time_at_min_str, att_val[i][j]->min_val,
			att_val[i][j]->time_at_max_str, att_val[i][j]->max_val,
			att_val[i][j]->max_val-att_val[i][j]->min_val
			);
#endif
	}
	ip_turbine_2 = att_valve_stroke_detector_new(kks_list, binevt, kks_analog1[11]);
	if (cetak_error (ip_turbine_2) == -1)
		exit (-1);
	att_valve_stroke_detector_find(ip_turbine_2);
#if defined VERBOSE_OUTPUT
	fprintf (file_att_result, "\n***IP turbine 2 stroke*** %s\nStart: %s\nEnd: %s\n",
		ip_turbine_2->kks, ip_turbine_2->time_start, ip_turbine_2->time_end);
	fprintf (file_att_result, "ESV trip 1 close time: %ldms, OK stated @: %s\n",
		hp_turbine_1->valve_timing.esv_trip_sol_1, ip_turbine_2->valve_timing.es_1_time_str);
	fprintf (file_att_result, "Timing signal: %s\n", ip_turbine_2->valve_timing.es_1_open_signal);
	fprintf (file_att_result, "Timing signal: %s\n", ip_turbine_2->valve_timing.es_1_close_signal);
	fprintf (file_att_result, "ESV trip 2 close time: %ldms, OK stated @: %s\n",
		ip_turbine_2->valve_timing.esv_trip_sol_2, ip_turbine_2->valve_timing.es_2_time_str);
	fprintf (file_att_result, "Timing signal: %s\n", ip_turbine_2->valve_timing.es_2_open_signal);
	fprintf (file_att_result, "Timing signal: %s\n", ip_turbine_2->valve_timing.es_2_close_signal);
	fprintf (file_att_result, "CV trip 1 close time: %ldms, OK stated @: %s\n",
		ip_turbine_2->valve_timing.cv_trip_sol_1, ip_turbine_2->valve_timing.cs_1_time_str);
	fprintf (file_att_result, "Timing signal: %s\n", ip_turbine_2->valve_timing.cs_1_open_signal);
	fprintf (file_att_result, "Timing signal: %s\n", ip_turbine_2->valve_timing.cs_1_close_signal);
	fprintf (file_att_result, "CV trip 2 close time: %ldms, OK stated @: %s\n",
		ip_turbine_2->valve_timing.cv_trip_sol_2, ip_turbine_2->valve_timing.cs_2_time_str);
	fprintf (file_att_result, "Timing signal: %s\n", ip_turbine_2->valve_timing.cs_2_open_signal);
	fprintf (file_att_result, "Timing signal: %s\n", ip_turbine_2->valve_timing.cs_2_close_signal);
#endif
	j = 3;
	for (i=0; i<14; i++)
	{
		att_val[i][j] = att_values_new
			(kks_list, kks_analog1[i], ip_turbine_2->time_init, ip_turbine_2->time_start,
				ip_turbine_2->time_end);
		att_values_find(att_val[i][j]);
#if defined VERBOSE_OUTPUT
		fprintf (file_att_result, "KKS: %s, Description: %s\n",
			att_val[i][j]->kks, att_val[i][j]->description);
		fprintf (file_att_result, "Time: %s, Init: %f\nTime: %s, Min: %f\nTime: %s, Max: %f\nSwing: %f\n",
			att_val[i][j]->time_at_init_str, att_val[i][j]->init_val,
			att_val[i][j]->time_at_min_str, att_val[i][j]->min_val,
			att_val[i][j]->time_at_max_str, att_val[i][j]->max_val,
			att_val[i][j]->max_val-att_val[i][j]->min_val
			);
#endif
	}
#if defined VERBOSE_OUTPUT
	if (fclose (file_att_result)!= 0)
	{
		printf ("Error menutup file. Apakah disk sudah penuh?\n");
		exit (-1);
	}
#endif


	file_att_form = fopen (att_form, "w");
	if (file_att_form == NULL)
	{
		printf ("Error operasi file. Program dihentikan.\n");
		exit (-1);
	}

	fprintf (file_att_form, "ATT Data Sheet.\n");
	fprintf (file_att_form, "Unit: %c0. Date: %s.\n", plant_unit, bulan_angka_ke_huruf(hp_turbine_1->time_init));

	fprintf (file_att_form, "\nCondition Before Testing:\n");
	fprintf (file_att_form, "|No.|  Description  |  Value   |  Description  |  Value   |\n");
	fprintf (file_att_form, "| 1.| LOADACTV   MW | % 8.3f | HRHL2TMP   oC | % 8.3f |\n",
		att_val[0][0]->init_val, att_val[7][0]->init_val);
	fprintf (file_att_form, "| 2.| RSHRATIO    %% | % 8.3f | HPCV1POS    %% | % 8.3f |\n",
		att_val[1][0]->init_val, att_val[8][0]->init_val);
	fprintf (file_att_form, "| 3.| FUFLOWMS    %% | % 8.3f | HPCV2POS    %% | % 8.3f |\n",
		att_val[2][0]->init_val, att_val[9][0]->init_val);
	fprintf (file_att_form, "| 4.| MSTMTEMP   oC | % 8.3f | IPCV1POS    %% | % 8.3f |\n",
		att_val[3][0]->init_val, att_val[10][0]->init_val);
	fprintf (file_att_form, "| 5.| MSTMPRES  bar | % 8.3f | IPCV2POS    %% | % 8.3f |\n",
		att_val[4][0]->init_val, att_val[11][0]->init_val);
	fprintf (file_att_form, "| 6.| MSTMFLOW kg/s | % 8.3f | FREQUENC   Hz | % 8.3f |\n",
		att_val[5][0]->init_val, att_val[12][0]->init_val);
	fprintf (file_att_form, "| 7.| HRHL1TMP   oC | % 8.3f | THROTTLV    %% | % 8.3f |\n",
		att_val[6][0]->init_val, att_val[13][0]->init_val);

	fprintf (file_att_form, "\nCondition During ATT: (testing started via SGC ATT)\n");
	fprintf (file_att_form, "|   |               |   HP CV 1 Movement  |   HP CV 2 Movement  |\n");
	fprintf (file_att_form, "|No.|    Params     | %s - %s | %s - %s |\n",
		ambil_waktu_tanpa_mili (hp_turbine_1->time_start),
		ambil_waktu_tanpa_mili (hp_turbine_1->time_end),
		ambil_waktu_tanpa_mili (hp_turbine_2->time_start),
		ambil_waktu_tanpa_mili (hp_turbine_2->time_end));
	fprintf (file_att_form, "|   |               |    Min   |    Max   |    Min   |    Max   |\n");
	fprintf (file_att_form, "| 1.| LOADACTV   MW | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[0][0]->min_val,
		att_val[0][0]->max_val,
		att_val[0][1]->min_val,
		att_val[0][1]->max_val);
	fprintf (file_att_form, "| 2.| RSHRATIO    %% | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[1][0]->min_val,
		att_val[1][0]->max_val,
		att_val[1][1]->min_val,
		att_val[1][1]->max_val);
	fprintf (file_att_form, "| 3.| FUFLOWMS    %% | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[2][0]->min_val,
		att_val[2][0]->max_val,
		att_val[2][1]->min_val,
		att_val[2][1]->max_val);
	fprintf (file_att_form, "| 4.| MSTMTEMP   oC | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[3][0]->min_val,
		att_val[3][0]->max_val,
		att_val[3][1]->min_val,
		att_val[3][1]->max_val);
	fprintf (file_att_form, "| 5.| MSTMPRES  bar | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[4][0]->min_val,
		att_val[4][0]->max_val,
		att_val[4][1]->min_val,
		att_val[4][1]->max_val);
	fprintf (file_att_form, "| 6.| MSTMFLOW kg/s | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[5][0]->min_val,
		att_val[5][0]->max_val,
		att_val[5][1]->min_val,
		att_val[5][1]->max_val);
	fprintf (file_att_form, "| 7.| HRHL1TMP   oC | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[6][0]->min_val,
		att_val[6][0]->max_val,
		att_val[6][1]->min_val,
		att_val[6][1]->max_val);
	fprintf (file_att_form, "| 8.| HRHL2TMP   oC | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[7][0]->min_val,
		att_val[7][0]->max_val,
		att_val[7][1]->min_val,
		att_val[7][1]->max_val);
	fprintf (file_att_form, "| 9.| HPCV1POS    %% | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[8][0]->min_val,
		att_val[8][0]->max_val,
		att_val[8][1]->min_val,
		att_val[8][1]->max_val);
	fprintf (file_att_form, "|10.| HPCV2POS    %% | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[9][0]->min_val,
		att_val[9][0]->max_val,
		att_val[9][1]->min_val,
		att_val[9][1]->max_val);
	fprintf (file_att_form, "|11.| IPCV1POS    %% | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[10][0]->min_val,
		att_val[10][0]->max_val,
		att_val[10][1]->min_val,
		att_val[10][1]->max_val);
	fprintf (file_att_form, "|12.| IPCV2POS    %% | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[11][0]->min_val,
		att_val[11][0]->max_val,
		att_val[11][1]->min_val,
		att_val[11][1]->max_val);
	fprintf (file_att_form, "|13.| FREQUENC   Hz | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[12][0]->min_val,
		att_val[12][0]->max_val,
		att_val[12][1]->min_val,
		att_val[12][1]->max_val);
	fprintf (file_att_form, "|14.| THROTTLV    %% | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[13][0]->min_val,
		att_val[13][0]->max_val,
		att_val[13][1]->min_val,
		att_val[13][1]->max_val);

	fprintf (file_att_form, "\nCondition During ATT: (testing started via SGC ATT)\n");
	fprintf (file_att_form, "|   |               |   IP CV 1 Movement  |   IP CV 2 Movement  |\n");
	fprintf (file_att_form, "|No.|    Params     | %s - %s | %s - %s |\n",
		ambil_waktu_tanpa_mili (ip_turbine_1->time_start),
		ambil_waktu_tanpa_mili (ip_turbine_1->time_end),
		ambil_waktu_tanpa_mili (ip_turbine_2->time_start),
		ambil_waktu_tanpa_mili (ip_turbine_2->time_end));
	fprintf (file_att_form, "|   |               |    Min   |    Max   |    Min   |    Max   |\n");
	fprintf (file_att_form, "| 1.| LOADACTV   MW | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[0][2]->min_val,
		att_val[0][2]->max_val,
		att_val[0][3]->min_val,
		att_val[0][3]->max_val);
	fprintf (file_att_form, "| 2.| RSHRATIO    %% | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[1][2]->min_val,
		att_val[1][2]->max_val,
		att_val[1][3]->min_val,
		att_val[1][3]->max_val);
	fprintf (file_att_form, "| 3.| FUFLOWMS    %% | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[2][2]->min_val,
		att_val[2][2]->max_val,
		att_val[2][3]->min_val,
		att_val[2][3]->max_val);
	fprintf (file_att_form, "| 4.| MSTMTEMP   oC | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[3][2]->min_val,
		att_val[3][2]->max_val,
		att_val[3][3]->min_val,
		att_val[3][3]->max_val);
	fprintf (file_att_form, "| 5.| MSTMPRES  bar | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[4][2]->min_val,
		att_val[4][2]->max_val,
		att_val[4][3]->min_val,
		att_val[4][3]->max_val);
	fprintf (file_att_form, "| 6.| MSTMFLOW kg/s | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[5][2]->min_val,
		att_val[5][2]->max_val,
		att_val[5][3]->min_val,
		att_val[5][3]->max_val);
	fprintf (file_att_form, "| 7.| HRHL1TMP   oC | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[6][2]->min_val,
		att_val[6][2]->max_val,
		att_val[6][3]->min_val,
		att_val[6][3]->max_val);
	fprintf (file_att_form, "| 8.| HRHL2TMP   oC | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[7][2]->min_val,
		att_val[7][2]->max_val,
		att_val[7][3]->min_val,
		att_val[7][3]->max_val);
	fprintf (file_att_form, "| 9.| HPCV1POS    %% | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[8][2]->min_val,
		att_val[8][2]->max_val,
		att_val[8][3]->min_val,
		att_val[8][3]->max_val);
	fprintf (file_att_form, "|10.| HPCV2POS    %% | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[9][2]->min_val,
		att_val[9][2]->max_val,
		att_val[9][3]->min_val,
		att_val[9][3]->max_val);
	fprintf (file_att_form, "|11.| IPCV1POS    %% | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[10][2]->min_val,
		att_val[10][2]->max_val,
		att_val[10][3]->min_val,
		att_val[10][3]->max_val);
	fprintf (file_att_form, "|12.| IPCV2POS    %% | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[11][2]->min_val,
		att_val[11][2]->max_val,
		att_val[11][3]->min_val,
		att_val[11][3]->max_val);
	fprintf (file_att_form, "|13.| FREQUENC   Hz | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[12][2]->min_val,
		att_val[12][2]->max_val,
		att_val[12][3]->min_val,
		att_val[12][3]->max_val);
	fprintf (file_att_form, "|14.| THROTTLV    %% | % 8.3f | % 8.3f | % 8.3f | % 8.3f |\n",
		att_val[13][2]->min_val,
		att_val[13][2]->max_val,
		att_val[13][3]->min_val,
		att_val[13][3]->max_val);

	fprintf (file_att_form, "\nTurbine Valves Closing Times: (testing started via SGC ATT)\n");
	fprintf (file_att_form, "|No.| Params  | Success  | Load(MW) |    Closing Time (msec)    |\n");
	fprintf (file_att_form, "|   |         | Yes | No |  Swing   | Sol 1 | OK? | Sol 2 | OK? |\n");
	fprintf (file_att_form, "| 1.| HPESV1  | %s | %s | % 8.3f |  %3li  | %s  |  %3li  | %s  |\n",
		status_yes[hp_turbine_1->valve_timing.att_sv_sukses],
		status_no[hp_turbine_1->valve_timing.att_sv_sukses],
		att_val[0][0]->max_val-att_val[0][0]->min_val,
		hp_turbine_1->valve_timing.esv_trip_sol_1,
		status_ok[hp_turbine_1->valve_timing.es_1_status],
		hp_turbine_1->valve_timing.esv_trip_sol_2,
		status_ok[hp_turbine_1->valve_timing.es_2_status]);
	fprintf (file_att_form, "|   | HPCV1   | %s | %s |          |  %3li  | %s  |  %3li  | %s  |\n",
		status_yes[hp_turbine_1->valve_timing.att_cv_sukses],
		status_no[hp_turbine_1->valve_timing.att_cv_sukses],
		hp_turbine_1->valve_timing.cv_trip_sol_1,
		status_ok[hp_turbine_1->valve_timing.cs_1_status],
		hp_turbine_1->valve_timing.cv_trip_sol_2,
		status_ok[hp_turbine_1->valve_timing.cs_2_status]);
	fprintf (file_att_form, "| 2.| HPESV2  | %s | %s | % 8.3f |  %3li  | %s  |  %3li  | %s  |\n",
		status_yes[hp_turbine_2->valve_timing.att_sv_sukses],
		status_no[hp_turbine_2->valve_timing.att_sv_sukses],
		att_val[0][1]->max_val-att_val[0][1]->min_val,
		hp_turbine_2->valve_timing.esv_trip_sol_1,
		status_ok[hp_turbine_2->valve_timing.es_1_status],
		hp_turbine_2->valve_timing.esv_trip_sol_2,
		status_ok[hp_turbine_2->valve_timing.es_2_status]);
	fprintf (file_att_form, "|   | HPCV2   | %s | %s |          |  %3li  | %s  |  %3li  | %s  |\n",
		status_yes[hp_turbine_2->valve_timing.att_cv_sukses],
		status_no[hp_turbine_2->valve_timing.att_cv_sukses],
		hp_turbine_2->valve_timing.cv_trip_sol_1,
		status_ok[hp_turbine_2->valve_timing.cs_1_status],
		hp_turbine_2->valve_timing.cv_trip_sol_2,
		status_ok[hp_turbine_2->valve_timing.es_2_status]);
	fprintf (file_att_form, "| 3.| IPESV1  | %s | %s | % 8.3f |  %3li  | %s  |  %3li  | %s  |\n",
		status_yes[ip_turbine_1->valve_timing.att_sv_sukses],
		status_no[ip_turbine_1->valve_timing.att_sv_sukses],
		att_val[0][2]->max_val-att_val[0][2]->min_val,
		ip_turbine_1->valve_timing.esv_trip_sol_1,
		status_ok[ip_turbine_1->valve_timing.es_1_status],
		ip_turbine_1->valve_timing.esv_trip_sol_2,
		status_ok[ip_turbine_1->valve_timing.es_2_status]);
	fprintf (file_att_form, "|   | IPCV1   | %s | %s |          |  %3li  | %s  |  %3li  | %s  |\n",
		status_yes[ip_turbine_1->valve_timing.att_cv_sukses],
		status_no[ip_turbine_1->valve_timing.att_cv_sukses],
		ip_turbine_1->valve_timing.cv_trip_sol_1,
		status_ok[ip_turbine_1->valve_timing.cs_1_status],
		ip_turbine_1->valve_timing.cv_trip_sol_2,
		status_ok[ip_turbine_1->valve_timing.es_2_status]);
	fprintf (file_att_form, "| 4.| IPESV2  | %s | %s | % 8.3f |  %3li  | %s  |  %3li  | %s  |\n",
		status_yes[ip_turbine_2->valve_timing.att_sv_sukses],
		status_no[ip_turbine_2->valve_timing.att_sv_sukses],
		att_val[0][3]->max_val-att_val[0][3]->min_val,
		ip_turbine_2->valve_timing.esv_trip_sol_1,
		status_ok[ip_turbine_2->valve_timing.es_1_status],
		ip_turbine_2->valve_timing.esv_trip_sol_2,
		status_ok[ip_turbine_2->valve_timing.es_2_status]);
	fprintf (file_att_form, "|   | IPCV2   | %s | %s |          |  %3li  | %s  |  %3li  | %s  |\n",
		status_yes[ip_turbine_2->valve_timing.att_cv_sukses],
		status_no[ip_turbine_2->valve_timing.att_cv_sukses],
		ip_turbine_2->valve_timing.cv_trip_sol_1,
		status_ok[ip_turbine_2->valve_timing.cs_1_status],
		ip_turbine_2->valve_timing.cv_trip_sol_2,
		status_ok[ip_turbine_2->valve_timing.es_2_status]);

	fprintf (file_att_form, "\nNote: (abnormalities, defects, testing cannot be done, etc)\n");
	fprintf (file_att_form, "_________________________________________________________________\n");
	fprintf (file_att_form, "_________________________________________________________________\n");
	fprintf (file_att_form, "_________________________________________________________________\n");
	fprintf (file_att_form, "\n");
/*
	fprintf (file_att_form, "Prepared by:                          Approved by:\n\n");
	fprintf (file_att_form, "Name:________________                 Name:_________________\n");
	fprintf (file_att_form, "Sign:                                 Sign:\n");
	fprintf (file_att_form, "     ________________                      _________________\n");
*/
	fprintf (file_att_form, "\nEvent Sequence Reading Guidance: \n");
	fprintf (file_att_form, "%s\n", hp_turbine_1->valve_timing.es_1_open_signal);
	fprintf (file_att_form, "%s\n", hp_turbine_1->valve_timing.es_1_close_signal);
	fprintf (file_att_form, "%s\n", hp_turbine_1->valve_timing.es_2_open_signal);
	fprintf (file_att_form, "%s\n", hp_turbine_1->valve_timing.es_2_close_signal);
	fprintf (file_att_form, "%s\n", hp_turbine_1->valve_timing.cs_1_open_signal);
	fprintf (file_att_form, "%s\n", hp_turbine_1->valve_timing.cs_1_close_signal);
	fprintf (file_att_form, "%s\n", hp_turbine_1->valve_timing.cs_2_open_signal);
	fprintf (file_att_form, "%s\n", hp_turbine_1->valve_timing.cs_2_close_signal);
	fprintf (file_att_form, "%s\n", hp_turbine_2->valve_timing.es_1_open_signal);
	fprintf (file_att_form, "%s\n", hp_turbine_2->valve_timing.es_1_close_signal);
	fprintf (file_att_form, "%s\n", hp_turbine_2->valve_timing.es_2_open_signal);
	fprintf (file_att_form, "%s\n", hp_turbine_2->valve_timing.es_2_close_signal);
	fprintf (file_att_form, "%s\n", hp_turbine_2->valve_timing.cs_1_open_signal);
	fprintf (file_att_form, "%s\n", hp_turbine_2->valve_timing.cs_1_close_signal);
	fprintf (file_att_form, "%s\n", hp_turbine_2->valve_timing.cs_2_open_signal);
	fprintf (file_att_form, "%s\n", hp_turbine_2->valve_timing.cs_2_close_signal);
	fprintf (file_att_form, "%s\n", ip_turbine_1->valve_timing.es_1_open_signal);
	fprintf (file_att_form, "%s\n", ip_turbine_1->valve_timing.es_1_close_signal);
	fprintf (file_att_form, "%s\n", ip_turbine_1->valve_timing.es_2_open_signal);
	fprintf (file_att_form, "%s\n", ip_turbine_1->valve_timing.es_2_close_signal);
	fprintf (file_att_form, "%s\n", ip_turbine_1->valve_timing.cs_1_open_signal);
	fprintf (file_att_form, "%s\n", ip_turbine_1->valve_timing.cs_1_close_signal);
	fprintf (file_att_form, "%s\n", ip_turbine_1->valve_timing.cs_2_open_signal);
	fprintf (file_att_form, "%s\n", ip_turbine_1->valve_timing.cs_2_close_signal);
	fprintf (file_att_form, "%s\n", ip_turbine_2->valve_timing.es_1_open_signal);
	fprintf (file_att_form, "%s\n", ip_turbine_2->valve_timing.es_1_close_signal);
	fprintf (file_att_form, "%s\n", ip_turbine_2->valve_timing.es_2_open_signal);
	fprintf (file_att_form, "%s\n", ip_turbine_2->valve_timing.es_2_close_signal);
	fprintf (file_att_form, "%s\n", ip_turbine_2->valve_timing.cs_1_open_signal);
	fprintf (file_att_form, "%s\n", ip_turbine_2->valve_timing.cs_1_close_signal);
	fprintf (file_att_form, "%s\n", ip_turbine_2->valve_timing.cs_2_open_signal);
	fprintf (file_att_form, "%s\n", ip_turbine_2->valve_timing.cs_2_close_signal);

	if (fclose (file_att_form)!= 0)
	{
		printf ("Error operasi file. Disk sudah penuh?\n");
		exit (-1);
	}
	return 0;
}
