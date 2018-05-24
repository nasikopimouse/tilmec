#include <math.h>
#include "dcs_time.h"
#include "droptest.h"
#include "att_kks_analog_trend_extractor.h"

/*
Ini adalah referensi copas dari file excel:
=(((0.5*((3900/2)^2)*(((2*ACOS((1981-J116)/1981)*180/PI())*PI()/180)-(SIN((2*ACOS
	((1981-J116)/1981)*180/PI())*PI()/180)))))*23165)/1000000000+(((0.3738*3.14)*
	(1850^2)*J116)/10^9)
*/

char* kks_droptest[] = 
{
	"5 0MKA01 CE901||XQ01", // unit load gross
	"5 0HAD10 FL901||XQ01", // drum level
	"5 0LAA10 CT001||XQ01", // fwt temperature
	"5 0LAA10 FL901||XQ01", // fwt level
	"5 0MAG20 CL901||XQ02"  // hotwell level
};


int kks_drop_test_change_unit (char** kks_droptest_list, char unit)
{
	char ch;
	int i;
	if (kks_droptest_list == NULL || unit == NULL)
		return -1;
	ch = unit;
	for (i=0; i< 5; i++)
		memset (kks_droptest_list[i], ch, 1);
	return 0;
}

double fwt_drop_test_calculation (double fwt_level_start, double fwt_level_finish)
{
	double fwt_start, fwt_finish;
	fwt_start = (((0.5*(pow((3900/2), 2))*
				(((2*acos((1981-fwt_level_start)/1981)*180/M_PI)*M_PI/180)
				-(sin((2*acos((1981-fwt_level_start)/1981)*180/M_PI)*M_PI/180)))))*23165)
				/1000000000+(((0.3738*3.14)*(pow(1850, 2))*fwt_level_start)/pow(10, 9));
	fwt_finish = (((0.5*(pow((3900/2), 2))*
				(((2*acos((1981-fwt_level_finish)/1981)*180/M_PI)*M_PI/180)
				-(sin((2*acos((1981-fwt_level_finish)/1981)*180/M_PI)*M_PI/180)))))*23165)
				/1000000000+(((0.3738*3.14)*(pow(1850, 2))*fwt_level_finish)/pow(10, 9));
	return fwt_start-fwt_finish;
}

double fwt_droptest_calculation_round_one_hour (double fwt_level_start, double fwt_level_finish, int minute_duration)
{
	double drop_level, drop_rate, remain_drop, remain_minute;
	int one_hour = 60;	
	drop_level = fwt_drop_test_calculation (fwt_level_start, fwt_level_finish);
	drop_rate = drop_level / minute_duration;
	remain_minute = one_hour - minute_duration;
	remain_drop = drop_rate * remain_minute;
	return drop_level + remain_drop;
}

double fwt_drop_test_calculate_tank_volume (double fwt_level_start)
{
		return (((0.5*(pow((3900/2), 2))*
				(((2*acos((1981-fwt_level_start)/1981)*180/M_PI)*M_PI/180)
				-(sin((2*acos((1981-fwt_level_start)/1981)*180/M_PI)*M_PI/180)))))*23165)
				/1000000000+(((0.3738*3.14)*(pow(1850, 2))*fwt_level_start)/pow(10, 9));
}

/* lebih baik duration dicatat di depan, agar kode bisa
dibuat untuk semua durasi waktu  */
struct droptest* droptest_new(char* fnam, int duration, int chk)
{
	int i;
	struct droptest* temp;
	struct droptest_data* temp_data;
	struct droptest_data* temp_data_head = NULL;
	/* hasil parse file   */
	struct word_extractor* word_data;
	/* data list kks   */
	struct kks_analog_trend_list* analog_list;
	/* data per kks  */
	struct kks_analog_trend* data_kks;
	struct kks_analog_trend_data* time_start;
	double fwt_start, fwt_end;
	struct kks_analog_trend_data* time_end;
	struct kks_analog_trend_data_wrapper* data_wrapper;
	FILE* file_data;
	char unit;
	if (fnam == NULL)
		return NULL;
	/* word extractor ada di sini.   */
	word_data = word_extractor_new();
	if (word_data == NULL)
		return NULL;
	word_extractor_read_file (word_data, fnam);
	/* analog trend list ada di sini    */
	analog_list = kks_analog_trend_list_new (word_data);
	
	
	
	if(analog_list == NULL)
		return NULL;
	/* Mencari paket trend dengan kks tertentu (cari kks untuk fwt level) */
	/* failed di sini!!!! */
	unit = analog_list->plant_unit;
	kks_drop_test_change_unit (kks_droptest, unit);
	for (i = 0; i< 5; i++)
	{
		data_kks = kks_analog_trend_find_kks (analog_list, kks_droptest[i]);
		if (data_kks == NULL)
			break;
	}
	if (data_kks == NULL)
	{
		if (data_kks == NULL)
		printf ("Cek file analog, apakah lengkap KKS untuk droptest?\n");
		// return NULL;
		exit (-1);
	}
	data_kks = kks_analog_trend_find_kks (analog_list, kks_droptest[3]);
	/* mencari time start dari paket trend yang sudah ditemukan */
	time_start = kks_analog_trend_find_time_start_chunk (data_kks);
	/* mencari time end berdasarkan time start dan duration (duration dalam menit, 30-60)  */
		/* menentukan apakah trend yang ada eligible
			(grafiknya konsisten menurun dan durasi waktu cukup) pada trend fwt
			pada data_kks. Time end bisa langsung mencari di trend dengan
			membandingkan string time end dengan string time pada data analog.
			Jika tidak eligible data jangan dimasukkan ke train
		*/		
	while (time_start->prev != NULL)
		time_start = time_start->prev;
	while (time_start != NULL)
	{
		if (chk == CHECK_LEVEL_FLUCTUATION)
		{
			time_end = kks_analog_trend_consistent_decrease_chunk
				(time_start, kks_analog_trend_get_time_ptr (time_start),
				duration);
			if (time_end == NULL)
			{
				// printf ("debug2\n");
				time_start = time_start->next;
				continue;
			}
		}
		else
		{
			time_end =
				kks_analog_trend_data_get_data_at_next
					(time_start, duration);
			if (time_end == NULL)
				break;
		}

		/* Jika ada data, maka buat chunk baru, dan masukkan data  */
		temp_data = droptest_data_new();
		if (temp_data != NULL)
		{
			temp_data->source_data_start =
				kks_analog_trend_get_data_at (analog_list,
				kks_analog_trend_get_time_ptr(time_start));
			temp_data->time_start =
				kks_analog_trend_get_time_ptr(time_start);
			temp_data->source_data_end = 
				kks_analog_trend_get_data_at (analog_list,
				kks_analog_trend_get_time_ptr(time_end));
			temp_data->time_end =
				kks_analog_trend_get_time_ptr(time_end);
			temp_data->duration = duration;
			data_wrapper =
				kks_analog_trend_data_wrapper_find_from_kks
					(
					/* unit load gross start  */
						temp_data->source_data_start, kks_droptest[0]
					);
			temp_data->load_start =
				kks_analog_trend_data_wrapper_get_value (data_wrapper);
			data_wrapper =
				kks_analog_trend_data_wrapper_find_from_kks
					(
					/* unit load gross end */
						temp_data->source_data_end, kks_droptest[0]
					);
			temp_data->load_end =
				kks_analog_trend_data_wrapper_get_value (data_wrapper);

			data_wrapper =
				kks_analog_trend_data_wrapper_find_from_kks
					(
					/* drum level start  */
						temp_data->source_data_start, kks_droptest[1]
					);
			temp_data->drum_start =
				kks_analog_trend_data_wrapper_get_value (data_wrapper);

				data_wrapper =
				kks_analog_trend_data_wrapper_find_from_kks
					(
					/* drum level end  */
						temp_data->source_data_end, kks_droptest[1]
					);
			temp_data->drum_end =
				kks_analog_trend_data_wrapper_get_value (data_wrapper);

				data_wrapper =
				kks_analog_trend_data_wrapper_find_from_kks
					(
					/* fwt temp start  */
						temp_data->source_data_start, kks_droptest[2]
					);
			fwt_start =
				kks_analog_trend_data_wrapper_get_value (data_wrapper);
			data_wrapper =
				kks_analog_trend_data_wrapper_find_from_kks
					(
					/* fwt temp end  */
						temp_data->source_data_end, kks_droptest[2]
					);
			fwt_end =
				kks_analog_trend_data_wrapper_get_value (data_wrapper);
			temp_data->fwtmp_avg = (fwt_start+fwt_end)/2;

			data_wrapper =
				kks_analog_trend_data_wrapper_find_from_kks
					(
					/* fwt level start  */
						temp_data->source_data_start, kks_droptest[3]
					);
			temp_data->fwt_start =
				kks_analog_trend_data_wrapper_get_value (data_wrapper);
			temp_data->fwt_vol_start =
				fwt_drop_test_calculate_tank_volume (temp_data->fwt_start);
			data_wrapper =
				kks_analog_trend_data_wrapper_find_from_kks
					(
					/* fwt level end  */
						temp_data->source_data_end, kks_droptest[3]
					);
			temp_data->fwt_end =
				kks_analog_trend_data_wrapper_get_value (data_wrapper);
			temp_data->fwt_vol_end =
				fwt_drop_test_calculate_tank_volume (temp_data->fwt_end);
			data_wrapper =
				kks_analog_trend_data_wrapper_find_from_kks
					(
					/* hotwell level start  */
						temp_data->source_data_start, kks_droptest[4]
					);
			temp_data->hotw_start =
				kks_analog_trend_data_wrapper_get_value (data_wrapper);
			data_wrapper =
				kks_analog_trend_data_wrapper_find_from_kks
					(
					/* hotwell level end  */
						temp_data->source_data_end, kks_droptest[4]
					);
			temp_data->hotw_end =
				kks_analog_trend_data_wrapper_get_value (data_wrapper);
				/*					
				char* kks_droptest[] = 
				{
					"5 0MKA01 CE901||XQ01", // unit load gross
					"5 0HAD10 FL901||XQ01", // drum level
					"5 0LAA10 CT001||XQ01", // fwt temperature
					"5 0LAA10 FL901||XQ01", // fwt level
					"5 0MAG20 CL901||XQ02"  // hotwell level
				};
				*/			
			/*  gunakan list kks untuk mencari data   */
			/* Jika operasi memori sukses, mulai masukkan data, dan tambahkan data pada list  */
			/*
			Contoh logic ada pada kode ...main.c untuk mengambil data
			dari keseluruhan kks dengan basis dari array list kks
			*/
			temp_data->droptest_result = fwt_drop_test_calculation
				(temp_data->fwt_start,
					temp_data->fwt_end);
			if (temp_data_head == NULL)
				temp_data_head = temp_data;
			else
				droptest_add (temp_data_head, temp_data);
		}
		/* tambahkan satu menit pada time start */
		time_start = time_start -> next;
	}
	temp_data = temp_data_head;
	while (temp_data != NULL)
		temp_data = temp_data->next;
	if (temp_data_head == NULL)
	{
		printf ("No data.\n");
		return NULL;
	}
	temp = (struct droptest*) malloc (sizeof (struct droptest));
	if (temp != NULL)
	{
		temp->raw_data = word_data;
		temp->data = analog_list;
		temp->list_content = temp_data_head;
	}	
	return temp;
}


int droptest_add (struct droptest_data* chain,
	struct droptest_data* chunk)
{
	if (chunk == NULL)
		return -1;
	if (chain != NULL)
	{
		while (chain->next != NULL)
			chain = chain->next;
		chunk->index = chain->index;
		chunk->index++;
		chain->next = chunk;
		chunk->prev = chain;
	}
	return 0;
}

struct droptest_data* droptest_data_new()
{
	struct droptest_data* temp;
	temp = (struct droptest_data*) malloc
		(sizeof (struct droptest_data));
	if (temp == NULL)
		return NULL;
	temp->prev = NULL;
	temp->next = NULL;
	temp->upr = NULL;
	temp->time_start = NULL;
	temp->time_end = NULL;
	temp->duration =
	temp->droptest_result = 
	temp->fwt_start = 
	temp->fwt_end = 
	temp->fwt_vol_start = 
	temp->fwt_vol_end = 
	temp->hotw_start =
	temp->hotw_end = 
	temp->load_start =
	temp->load_end = 
	temp->drum_start =
	temp->drum_end =
	temp->fwtmp_avg = -1;
	temp->source_data_start = NULL;
	temp->source_data_end = NULL;
	return temp;
}

int droptest_print(struct droptest* arg, char* fnam)
{
	/*
	Ini harus disesuaikan dengan format excel
	yang ada di kantor. Tetapi bisa dibuat belakangan.
	*/
	char strtemp[1024];
	FILE* fdat;
	struct droptest_data* temp;
	if (arg == NULL)
		return -1;
	if (arg->list_content == NULL)
		return -1;
	if (fnam == NULL)
		return -1;
	memset (strtemp, 0, 1024);
	if ((fdat = fopen (fnam, "w")) == NULL)
	{
		printf ("Open file %s error. Program dihentikan\n", fnam);
		exit (-1);
	}
	temp = arg->list_content;
	while (temp->prev != NULL)
		temp = temp->prev;
	/*
	Date	Starting time	Ending time	Test period	Load	FWT temp.	Initial Hotwell level	Final Hotwell level	Initial FWT level	Final FWT level	Initial Drum level	Final Drum level
			hour	MW	deg C	mm	mm	mm	mm	mm	mm	
	*/
	fprintf (fdat, "date;time_start;time_end;period;load;FWT_temp;hotwell_init;hotwell_final;FWT_init;FWT_Final;drum_Init;drum_final;FWT_vol_init;FWT_vol_final;FWT_level_drop_rate\n");

	while (temp != NULL)
	{
		fprintf
		(fdat, "%s;%s;%s;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f\n",
			ambil_tanggal_saja (temp->time_start),
			ambil_waktu_tanpa_mili (temp->time_start),
			ambil_waktu_tanpa_mili (temp->time_end),
			temp->duration/60,
			temp->load_start,
			temp->fwtmp_avg,
			temp->hotw_start,
			temp->hotw_end,
			temp->fwt_start,
			temp->fwt_end,
			temp->drum_start,
			temp->drum_end,
			temp->fwt_vol_start,
			temp->fwt_vol_end,	
			temp->droptest_result
		);
		temp = temp->next;
	}
	if (fclose (fdat) != 0)
	{
		printf ("Error close file. Program dihentikan.\n");
		exit (-1);
	}
	return 0;
}

int droptest_main(char* fnam_in, double dur)
{
	char* fnam_out = "OUTPUT_FW_DROPTEST.csv";
	struct droptest* temp;
	if (fnam_in == NULL || fnam_out == NULL)
	{
		return -1;
	}
	temp = droptest_new (fnam_in, dur, NO_CHECK_LEVEL_FLUCTUATION);
	if (temp == NULL)
	{
		return -1;
	}
	droptest_print (temp, fnam_out);
	return 0;
}