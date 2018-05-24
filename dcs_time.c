#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "dcs_time.h"
#include "att_word_extractor_main.h"

int dcs_time_delete (struct dcs_time* data)
{
	if (data == NULL)
		return -1;
	if (data->diff_time_str != NULL)
		free (data->diff_time_str);
	if (data->waktu_detail_1!= NULL)
		free (data->waktu_detail_1);
	if (data->waktu_detail_2!= NULL)
		free (data->waktu_detail_2);
	free (data);
	return 0;
}

/*
Mengambil nilai mili detik dari
string waktu format DCS
*/
int get_millisecond (char* time_str)
{
	char* temp;
	char millisec[10];
	temp = NULL;
	memset (millisec, '\0', 10);
	temp = datetime_binary_correcting(time_str);
	if (temp == NULL)
		return -1;
/*
	if (time_str == NULL)
		return -1;
	if (apakah_datetime_binary(time_str))
		time_str++;
*/
	strcpy(millisec, time_str+20);
	return strtod(millisec, NULL);
}

char* apakah_dcs_datetime (char* tanggal)
{
	return datetime_binary_correcting(tanggal);
}

struct dcs_time* dcs_time_new ()
{
	struct dcs_time* temp;
	temp = (struct dcs_time*) malloc (sizeof (struct dcs_time));
	if (temp!= NULL)
	{
		temp->time_str_1 = NULL;
		temp->time_str_2 = NULL;
		temp->diff_time_str = NULL;
		temp->jam_result = temp->menit_result =
			temp->detik_result = temp->mili_result = -1;
		temp->is_input_changed = 0;
		temp->total_mili_result = -1;
		temp->is_str1_bef_str2 = -1;
		temp->waktu_detail_1 = (struct waktu_detail*) malloc
			(sizeof (struct waktu_detail));
		if (temp->waktu_detail_1 != NULL)
		{
			temp->waktu_detail_1->tahun = -1;
			temp->waktu_detail_1->bulan = -1;
			temp->waktu_detail_1->tanggal = -1;
			temp->waktu_detail_1->jam = -1;
			temp->waktu_detail_1->menit = -1;
			temp->waktu_detail_1->detik = -1;
			temp->waktu_detail_1->mili = -1;
		}
		temp->waktu_detail_2 = (struct waktu_detail*) malloc
			(sizeof (struct waktu_detail));
		if (temp->waktu_detail_2 != NULL)
		{
			temp->waktu_detail_2->tahun = -1;
			temp->waktu_detail_2->bulan = -1;
			temp->waktu_detail_2->tanggal = -1;
			temp->waktu_detail_2->jam = -1;
			temp->waktu_detail_2->menit = -1;
			temp->waktu_detail_2->detik = -1;
			temp->waktu_detail_2->mili = -1;
		}
	}
	return temp;
}

int dcs_time_set_timestr_1 (struct dcs_time* chain, char* timestr)
{
	if (chain == NULL || timestr == NULL)
		return -1;
	if (timestr==NULL)
		return -1;
	chain->time_str_1 = timestr;
	chain->is_input_changed = 1;
	chain->is_str1_bef_str2 = -1;
	dcs_time_parse_waktu (chain->waktu_detail_1, chain->time_str_1);
	return 0;
}

/* dcs_time_parse_waktu() adalah private method. Akan menambahkan data waktu terpisah
bertipe long pada struktur data. Data ini akan diisikan setiap kali
mengisi input. Membutuhkan struktur data dcs_time.  */
int dcs_time_parse_waktu (struct waktu_detail* data_waktu, char* waktu)
{
	char datetime1[1024];
	
	if (data_waktu == NULL || waktu == NULL)
		return -1;
	if (apakah_dcs_datetime (waktu) == NULL)
		return -1;
	memset (datetime1, 0, 1024);
	strcpy (datetime1, waktu);
	data_waktu->mili = strtol(datetime1+20, NULL, 0);
	memset (datetime1+19, '\0', 1);
	data_waktu->detik = strtol(datetime1+17, NULL, 0);
	memset(datetime1+16, '\0', 1);
	data_waktu->menit = strtol(datetime1+14, NULL, 0);
	memset(datetime1+13, '\0', 1);
	data_waktu->jam = strtol(datetime1+11, NULL, 0);
	memset(datetime1+10, '\0', 1);
	data_waktu->tanggal = strtol(datetime1+8, NULL, 0);
	memset(datetime1+7, '\0', 1);
	data_waktu->bulan = strtol(datetime1+5, NULL, 0);
	memset(datetime1+4, '\0', 1);
	data_waktu->tahun = strtol(datetime1, NULL, 0);
	return 0;
}

int dcs_time_set_timestr_2 (struct dcs_time* chain, char* timestr)
{
	if (chain == NULL || timestr == NULL)
		return -1;
	if (timestr==NULL)
		return -1;
	chain->time_str_2 = timestr;
	chain->is_input_changed = 1;
	chain->is_str1_bef_str2 = -1;
	dcs_time_parse_waktu (chain->waktu_detail_2, chain->time_str_2);
	return 0;
}


/* maksimal 60menit   */
/* Mungkin lebih baik jika memakai fungsi datetime dari C.
Tidak memerlukan struktur dcs_time. Method internal */
char* add_minutes (char* result, int minutes)
{
	long calc_menit;
	struct tm waktu_c;
	struct tm* waktu_ptr;
	time_t waktu1;
	struct waktu_detail temp;
	if (result == NULL)
		return NULL;
	if (apakah_dcs_datetime(result) == NULL)
		return NULL;
	memset (&temp, 0, sizeof (temp));
	dcs_time_parse_waktu (&temp, result);
	/*
	printf ("debug dcstime0 %s\n", result); 
	printf ("\ndebug dcstime1 %d/%02d/%02d %02d:%02d:%02d.%03d\n",
		temp.tahun, temp.bulan, temp.tanggal,
		temp.jam, temp.menit, temp.detik, temp.mili);
	*/
	calc_menit = minutes * 60;
	waktu_c.tm_sec = temp.detik;
	waktu_c.tm_min = temp.menit;
	waktu_c.tm_hour = temp.jam+1;
	/*
	printf ("debug jam1: %d, %d\n", temp.jam, waktu_c.tm_hour);
	*/
	waktu_c.tm_mday = temp.tanggal;
	waktu_c.tm_mon = temp.bulan-1;
	waktu_c.tm_year = temp.tahun-1900;
	waktu1 = mktime (&waktu_c);
	/*
	printf ("debug waktu1: %d %s", waktu1, ctime (&waktu1));
	*/
	waktu1+=calc_menit;
	waktu_ptr = localtime (&waktu1);
	/*
	printf ("debug waktu2: %d %s", waktu1, ctime (&waktu1));
	*/
	temp.menit = waktu_ptr->tm_min;
	temp.jam = waktu_ptr->tm_hour;
	/*
	printf ("debug jam2: %d, %d\n", temp.jam, waktu_ptr->tm_hour);
	*/
	temp.tanggal = waktu_ptr->tm_mday;
	temp.bulan = waktu_ptr->tm_mon+1;
	temp.tahun = waktu_ptr->tm_year+1900;
	temp.detik = waktu_ptr->tm_sec;
	/*
	printf ("debug dcstime2 %d/%02d/%02d %02d:%02d:%02d.%03d\n",
		temp.tahun, temp.bulan, temp.tanggal,
		temp.jam, temp.menit, temp.detik, temp.mili);
	*/
	sprintf (result, "%d/%02d/%02d %02d:%02d:%02d.%03d",
		temp.tahun, temp.bulan, temp.tanggal,
		temp.jam, temp.menit, temp.detik, temp.mili);
	return result;
}

/* result adalah pointer string waktu dcs yang sudah siap
		digunakan. Jangan memasukkan pointer lainnya yang ga jelas! */
char* dcs_time_waktu_detail_to_str (char* result, struct waktu_detail* d)
{
	/*
	Waktu string format DCS:
	2016/05/22 07:06:49.428
	*/
	if (result == NULL || d == NULL)
		return NULL;
	sprintf (result, "%d/%02d/%02d %02d:%02d:%02d.%03d",
		d->tahun, d->bulan, d->tanggal,
		d->jam, d->menit, d->detik, d->mili);
	return result;
}


int dcs_time_1_add_minutes (struct dcs_time* data, int minutes)
{
	char temp[1024];
	char* tmp;
	if (data == NULL)
		return NULL;
	memset (temp, 0, 1024);
	tmp = add_minutes (data->time_str_1, minutes);
	if (tmp == NULL)
		return -1;
	strcpy (temp, tmp);
	dcs_time_set_timestr_1(data, temp);
	return 0;
}

int dcs_time_2_add_minutes (struct dcs_time* data, int minutes)
{
	char temp[1024];
	char* tmp;
	if (data == NULL)
		return NULL;
	memset (temp, 0, 1024);
	tmp = add_minutes (data->time_str_2, minutes);
	if (tmp == NULL)
		return -1;
	strcpy (temp, tmp);
	dcs_time_set_timestr_2(data, temp);
	return 0;
}


/*
Fungsi ini hanya berlaku jika time selain millisecond
adalah nol. Akan diambil hanya bilangan millisecondnya saja.
Ini digunakan untuk perhitungan selisih waktu pada
event valve stroking
Angka 0 dapat saja dioutputkan jika waktu yang dimasukkan
pada mesin dcs_time adalah sama.
*/
int dcs_time_get_diff_time_millisec (struct dcs_time* chain)
{
	if (chain != NULL)
	{	/* Jika perhitungan belum dilakukan maka hitung! */
		if (chain->is_input_changed != 0)
			dcs_time_get_diff_time(chain);
		if (chain->jam_result == 0
			&& chain->menit_result == 0
			&& chain->detik_result == 0)
				return chain->mili_result;
	}
	return -1;
}

/*
Jika ditemukan selisih hanya detik, maka
akan mengembalikan nilai sebesar detik result.
Untuk milisecond bisa diambil melalui
get indidual pada engine.
Jika selisih yang ditemukan lebih dari atau
sama dengan satu menit atau selisih
hanya millisecond, maka akan mengembalikan nilai -1
Ini digunakan untuk mencari apakah range
yang ada adalah range ATT
-1
*/
int dcs_time_get_diff_time_sec(struct dcs_time* chain)
{
	if (chain != NULL)
	{
		if (chain->is_input_changed != 0)
			dcs_time_get_diff_time(chain);
		if (chain->jam_result == 0
			&& chain->menit_result == 0
			&& chain->detik_result > 0 )
			return chain->detik_result;
	}
	return -1;
}
/*
Hasil yang diinginkan ada pada nilai return. Jika
nilainya 1 berarti waktu1 adalah sebelum waktu2.
Jika nilainya 0 berarti waktu1 setelah waktu2.
Jika nilainya berarti waktu1 sama dengan waktu2.
Berarti yang dicari adalah apakah bedawaktu
adalah 0 dan 2. Nilai awal mestinya waktu 1 pada
slot1 dan waktu2 pada slot2 dan waktu1 lebih kecil
daripada waktu2.
*/
long dcs_time_get_diff_time(struct dcs_time* chain)
{
	/*
	Waktu string format DCS:
	2016/05/22 07:06:49.428
	*/
	int tahun1, bulan1, tanggal1, jam1,
		menit1, detik1, mili1;
	int tahun2, bulan2, tanggal2, jam2, menit2,
		detik2, mili2;
	int i;
	long jam_result, menit_result,
		detik_result, mili_result;
	time_t waktuint1;
	time_t waktuint2;
	ldiv_t hasil;
	double bedawaktu;
	char datetime1 [30];
	char datetime2 [30];
	char waktu_data[4][10];
	struct tm waktu1;
	struct tm waktu2;
	if (chain == NULL)
		return -1;
	memset (&waktu1, 0, sizeof (struct tm));
	memset (&waktu2, 0, sizeof (struct tm));
	for (i = 0; i < 4; i++)
		memset (waktu_data[i], '\0', 10);
	memset(datetime1, '\0', 30);
	memset(datetime2, '\0', 30);
	strcpy (datetime1, chain->time_str_1);
	strcpy (datetime2, chain->time_str_2);
	mili1 = strtod(datetime1+20, NULL);
	memset (datetime1+19, '\0', 1);
	detik1 = strtod(datetime1+17, NULL);
	memset(datetime1+16, '\0', 1);
	menit1 = strtod(datetime1+14, NULL);
	memset(datetime1+13, '\0', 1);
	jam1 = strtod(datetime1+11, NULL);
	memset(datetime1+10, '\0', 1);
	tanggal1 = strtod(datetime1+8, NULL);
	memset(datetime1+7, '\0', 1);
	bulan1 = strtod(datetime1+5, NULL);
	memset(datetime1+4, '\0', 1);
	tahun1 = strtod(datetime1, NULL);	
	mili2 = strtod(datetime2+20, NULL);
	memset (datetime2+19, '\0', 1);
	detik2 = strtod(datetime2+17, NULL);
	memset(datetime2+16, '\0', 1);
	menit2 = strtod(datetime2+14, NULL);
	memset(datetime2+13, '\0', 1);
	jam2 = strtod(datetime2+11, NULL);
	memset(datetime2+10, '\0', 1);
	tanggal2 = strtod(datetime2+8, NULL);
	memset(datetime2+7, '\0', 1);
	bulan2 = strtod(datetime2+5, NULL);
	memset(datetime2+4, '\0', 1);
	tahun2 = strtod(datetime2, NULL);	
	waktu1.tm_mday = tanggal1;
	waktu1.tm_mon = bulan1-1;
	waktu1.tm_year = tahun1-1900;
	waktu1.tm_hour = jam1;
	waktu1.tm_min = menit1;
	waktu1.tm_sec = detik1;
	waktu2.tm_mday = tanggal2;
	waktu2.tm_mon = bulan2-1;
	waktu2.tm_year = tahun2-1900;
	waktu2.tm_hour = jam2;
	waktu2.tm_min = menit2;
	waktu2.tm_sec = detik2;
	waktuint1 = mktime(&waktu1);
	waktuint2 = mktime(&waktu2);
	bedawaktu = difftime(waktuint2, waktuint1);
	if (bedawaktu == 0)
	{
		if (mili1 < mili2)
		{
			chain->is_str1_bef_str2 = 1;
			mili_result = mili2 - mili1;
			chain->jam_result = 0;
			chain->menit_result = 0;
			chain->detik_result = 0;
			chain->mili_result = mili_result;
			chain->total_mili_result = mili_result;
			chain->is_input_changed = 0;
		}
		else if (mili1 > mili2)
		{
			chain->is_str1_bef_str2 = 0;
			chain->jam_result = -1;
			chain->menit_result = -1;
			chain->detik_result = -1;
			chain->mili_result = -1;
			chain->total_mili_result = -1;
			chain->is_input_changed = 0;			
		}
		
		else
		{
			chain->is_str1_bef_str2 = 2;
			chain->jam_result = 0;
			chain->menit_result = 0;
			chain->detik_result = 0;
			chain->mili_result = 0;
			chain->total_mili_result = 0;
			chain->is_input_changed = 0;						
		}
	}
	if (bedawaktu > 0)
	{
		chain->is_str1_bef_str2 = 1;
		if (mili1 > mili2)
		{
			bedawaktu--;
			mili_result = (1000 - mili1) + mili2;
		}
		else
			mili_result = mili2 - mili1;
		hasil = ldiv ((long)bedawaktu, 3600);
		jam_result = hasil.quot;
		bedawaktu = hasil.rem;
		hasil = ldiv((long)bedawaktu, 60);
		menit_result = hasil.quot;
		detik_result = hasil.rem;
		chain->jam_result = jam_result;
		chain->menit_result = menit_result;
		chain->detik_result = detik_result;
		chain->mili_result = mili_result;
		chain->total_mili_result =
			(chain->mili_result) +
			(chain->detik_result*100) +
			((chain->menit_result*60)*100) +
			((chain->jam_result*60*60)*100);
		chain->is_input_changed = 0;
	}
	if (bedawaktu < 0)
	{
		chain->is_str1_bef_str2 = 0;
		chain->is_str1_bef_str2 = -1;
		chain->jam_result = -1;
		chain->menit_result = -1;
		chain->detik_result = -1;
		chain->mili_result = -1;
		chain->total_mili_result = -1;
		chain->is_input_changed = 0;			
	}
	if (chain->diff_time_str == NULL)
		chain->diff_time_str = (char*) malloc (1024);
	memset (chain->diff_time_str, '\0', 1024);
	sprintf (waktu_data[0], "%ldh, ", jam_result);
	sprintf (waktu_data[1], "%ldm, ", menit_result);
	sprintf (waktu_data[2], "%lds, ", detik_result);
	sprintf (waktu_data[3], "%ldms\n", mili_result);
	sprintf (chain->diff_time_str, "%s%s%s%s", waktu_data[0],
		waktu_data[1], waktu_data[2], waktu_data[3]);
	
	return chain->total_mili_result;
}

int dcs_time_get_status (struct dcs_time* chain)
{
	if (chain == NULL)
		return -1;
	return chain->is_str1_bef_str2;
}


int dcs_time_print (struct dcs_time* arg)
{
	if (arg == NULL)
		return -1;
	printf ("Time 1: %s\nTime 2: %s\nDiff: %s\n", arg->time_str_1, 
				arg->time_str_2, arg->diff_time_str);
	return 0;
}

/* Fungsi di bawah ini mengembalikan nilai pointer member struktur.
Fungsi seperti ini tidak akan bisa memproteksi class dari modifikasi.
Ini karena data bisa dengan bebas diakses oleh sistem lain yang
sudah mempunyai pointernya. Bagaimana menyiasati hal ini?
Salah satu caranya adalah dengan meminta pointer string dari luar.
Dengan begitu data akan hanya dikopikan saja. */
long dcs_time_get_jam_result (struct dcs_time* arg)
{
	if (arg!=NULL)
		return arg->jam_result;
	return -1;
}

long dcs_time_get_menit_result (struct dcs_time* arg)
{
	if (arg!=NULL)
		return arg->menit_result;
	return -1;
}

long dcs_time_get_detik_result (struct dcs_time* arg)
{
	if (arg!=NULL)
		return arg->detik_result;
	return -1;
}

long dcs_time_get_mili_result (struct dcs_time* arg)
{
	if (arg!=NULL)
		return arg->mili_result;
	return -1;
}


int apakah_waktu_urut (char* time1, char* time2)
{
	int tahun1, bulan1, tanggal1, jam1,
		menit1, detik1, mili1;
	int tahun2, bulan2, tanggal2, jam2, menit2,
		detik2, mili2;
	int i;
	long jam_result, menit_result,
		detik_result, mili_result;
	time_t waktuint1;
	time_t waktuint2;
	ldiv_t hasil;
	double bedawaktu;
	char datetime1 [30];
	char datetime2 [30];
	char waktu_data[4][10];
	struct tm waktu1;
	struct tm waktu2;
	
	if (time1 == NULL || time2 == NULL)
		return -1;
	if (apakah_datetime(time1) != 1 ||
			apakah_datetime(time2) != 1)
		return -1;
	memset (&waktu1, 0, sizeof (struct tm));
	memset (&waktu2, 0, sizeof (struct tm));		
	memset(datetime1, '\0', 30);
	memset(datetime2, '\0', 30);
	strcpy (datetime1, time1);
	strcpy (datetime2, time2);
	mili1 = strtod(datetime1+20, NULL);
	memset (datetime1+19, '\0', 1);
	detik1 = strtod(datetime1+17, NULL);
	memset(datetime1+16, '\0', 1);
	menit1 = strtod(datetime1+14, NULL);
	memset(datetime1+13, '\0', 1);
	jam1 = strtod(datetime1+11, NULL);
	memset(datetime1+10, '\0', 1);
	tanggal1 = strtod(datetime1+8, NULL);
	memset(datetime1+7, '\0', 1);
	bulan1 = strtod(datetime1+5, NULL);
	memset(datetime1+4, '\0', 1);
	tahun1 = strtod(datetime1, NULL);	
	mili2 = strtod(datetime2+20, NULL);
	memset (datetime2+19, '\0', 1);
	detik2 = strtod(datetime2+17, NULL);
	memset(datetime2+16, '\0', 1);
	menit2 = strtod(datetime2+14, NULL);
	memset(datetime2+13, '\0', 1);
	jam2 = strtod(datetime2+11, NULL);
	memset(datetime2+10, '\0', 1);
	tanggal2 = strtod(datetime2+8, NULL);
	memset(datetime2+7, '\0', 1);
	bulan2 = strtod(datetime2+5, NULL);
	memset(datetime2+4, '\0', 1);
	tahun2 = strtod(datetime2, NULL);	
	waktu1.tm_mday = tanggal1;
	waktu1.tm_mon = bulan1-1;
	waktu1.tm_year = tahun1-1900;
	waktu1.tm_hour = jam1;
	waktu1.tm_min = menit1;
	waktu1.tm_sec = detik1;
	waktu2.tm_mday = tanggal2;
	waktu2.tm_mon = bulan2-1;
	waktu2.tm_year = tahun2-1900;
	waktu2.tm_hour = jam2;
	waktu2.tm_min = menit2;
	waktu2.tm_sec = detik2;
	waktuint1 = mktime(&waktu1);
	waktuint2 = mktime(&waktu2);
	bedawaktu = difftime(waktuint2, waktuint1);
	if (bedawaktu == 0)
	{
		if (mili1 < mili2)
			return 1;
		else if (mili1 > mili2)
			return 0;
		else
			return 2;
	}
	if (bedawaktu > 0)
		return 1;
	if (bedawaktu < 0)
		return 0;
	return -1;
}
