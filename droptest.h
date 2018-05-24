#ifndef DROPTEST_H
#define DROPTEST_H
#include <math.h>
#include "att_word_extractor.h"
#include "att_kks_analog_trend_extractor.h"


#define DROPTEST_DURATION_MINIMUM		30
#define DROPTEST_DURATION_MAXIMUM		60
#define CHECK_LEVEL_FLUCTUATION     1
#define NO_CHECK_LEVEL_FLUCTUATION   0

/*
Ekstrak data
ambil satu titik waktu.
cari apakah ada titik waktu untuk 1 jam ke depan.
jika ada, lacak apakah level fwt konsisten turun di antara dua tanggal itu.
Jika level sempat naik atau turun melebihi nilai pada kedua waktu, ulangi pencarian.
Jika ketemu, ambil nilai pada waktu awal dan waktu akhir.
hitung droptest.
buat struktur baru, masukkan semua data ke struktur baru itu.
Ulangi pencarian hingga tidak ada lagi rentang waktu 1jam.
buat format csv yang langsung bisa dipaste di perhitungan droptest cre.
Buat juga format csv yang langsung bisa dipaste di laporan rtpm.
cari apakah ada tanggal untuk setengah jam ke depan. Pakai command line parameter untuk membedakan.
Ke depan, pilihan antara unit 5 dan unit 6 juga dibedakan hanya berdasarkan parameter saja,
jadi satu program bisa berbagai macam file cmd, untuk "layanan" yang berbeda-beda.
*/

struct droptest
{
	struct word_extractor* raw_data;
	struct kks_analog_trend_list* data;
	struct droptest_data* list_content;
};

struct droptest_data
{
	struct droptest_data* prev;
	struct droptest_data* next;
	struct droptest* upr;
	int index;
	char* time_start;
	char* time_end;
	double duration;
	double droptest_result;
	double fwt_start;
	double fwt_end;
	double fwt_vol_start;
	double fwt_vol_end;
	double hotw_start;
	double hotw_end;
	double load_start;
	double load_end;
	double drum_start;
	double drum_end;
	double fwtmp_avg;
	struct kks_analog_trend_data_wrapper* source_data_start;
	struct kks_analog_trend_data_wrapper* source_data_end;
};

/* Membuat objek baru untuk droptest    */
struct droptest*
droptest_new(char*, int, int);
/* Menghitung pengurangan level air dalam m3 berdasarkan
pengurangan level fwt. */
double
fwt_drop_test_calculation (double, double);
/* Mengganti unit pada list konstanta kks yang akan dicari
datanya dari unit 50 menjadi unit 60  */
double
fwt_drop_test_calculation_round_one_hour (double, double, double);
int
kks_drop_test_change_unit (char**, char);
/* Mencari waktu trend pertama kali pada data trend   */
char*
kks_analog_trend_find_time_start (struct kks_analog_trend*);
/* Mencari dan mengecek apakah ada waktu trend akhir
dan waktu trend awal berdasarkan waktu awal dan durasi  */
char*
kks_analog_trend_get_time_pair
	(struct kks_analog_trend_data*, char*, char*);

struct droptest_data* droptest_data_new (void);

int droptest_add (struct droptest_data*,
	struct droptest_data*);

int droptest_main(char*, double);
int droptest_print (struct droptest*, char*);

double fwt_drop_test_calculate_tank_volume (double);

#endif
