#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "att_kks_analog_trend_extractor.h"
#include "att_word_extractor.h"
#include "att_word_extractor_main.h"
#include "droptest.h"
#include "dcs_time.h"
#include "tilmec_version.h"

/*  format kks: 5 0MKA01 CE901||XQ01 */
/* format datetime: 2016/05/22 07:00:31.000  */

/*
Jika benar signature waktu, maka return 1,
jika salah 0, jika fault -1
*/

char* bln[] =
{
	"JAN",
	"FEB",
	"MAR",
	"APR",
	"MAY",
	"JUN",
	"JUL",
	"AUG",
	"SEP",
	"OCT",
	"NOV",
	"DEC",
	"CUK"
};

/* Yang diproses harus copynya. Jangan memakai data
asli dari word extractor.   */
char* kks_to_fnam (char* kks)
{
	if (kks == NULL)
		return NULL;
	if (apakah_kks(kks) == 1)
	{
		memset (kks+1, '_', 1);
		memset (kks+8, '_', 1);
		memset (kks+14, '_', 1);
		memset (kks+15, '_', 1);
	}
	return kks;
}

/*
Ingat, sebenarnya untuk mengubah format
datetime sudah dilakukan pada modul binary
extractor.

Fungsi ini dipakai di modul ini untuk membaca
data time dan sekaligus membuang spasi di awal
jika ada.

Mungkin kita harus memodifikasi fungsi
ini sehingga sesuai dengan kebutuhan unit 60

*/

/*
char* datetime_binary_correcting(char* dtm)
{
	if (dtm == NULL)
		return NULL;
	if (apakah_datetime_binary(dtm)==1)
		return dtm+1;
	else if (apakah_datetime(dtm) == 1)
		return dtm;
	return NULL;
}
*/


/*
DCS unit common sekarang memakai format waktu
AM-PM. Ini tidak kompatibel dengan tilmec.
Kode harus ditambahkan untuk mengkonversi waktu
AM-PM menjadi waktu dengan format 24jam.

Fungsi yang sudah ada harus diubah. Jadi, membuat
fungsi baru untuk konversi seharusnya dipakai di
dalam fungsi yang lama.
*/
/*
// Jika tanggal untuk binary terdeteksi (ada spasi),
// maka hapus spasi terlebih dahulu
// Fungsi konversi ini hanya mengubah apa yang ada
// di parameter. Tidak akan emngalokasikan memori baru.
char* konversi_ampm_ke_biasa(char* dtm1)
{
	char* dtm = NULL;
	char temp[5];
	int i;
	if (dtm == NULL)
		return -1;
	// ubah pointer sehingga tanggal yang
	// awalnya ada spasi bisa hilang spasinya,
	// untuk pengukuran pointer selanjutnya.
	dtm = dtm1;
	dtm = delspaceawal(dtm);
	// Jika terdeteksi AM, jika jam menunjukkan
	// angka 12 maka diganti dengan nolnol. Kata AM
	// dihapus. Jika ketemu angka lain tidak perlu
	// diubah.
	// Jika terdeteksi PM, maka jika angka kurang
	// dari 12, maka angka ditambahkan dengan 12.
	
	if (!strcmp((char*)dtm+24, "AM"))
	{
		if ((char)dtm1[11] == '1'&&(char)dtm1[12] == '2')
		{
			dtm[11] = '0';
			dtm[12] = '0';
		}
	}
	if (!strcmp((char*)dtm1+24, "PM"))
	{
		for (i=0; i< 5; i++)
			temp[i] = '\0';
		temp[0] = (char)dtm1[11];
		temp[1] = (char)dtm1[12];
		i = atoi(temp);
		if (i < 12)
			i+=12;
		sprintf (temp, "%d", i);
		dtm1[11] = temp[0];
		dtm1[12] = temp[1];
	}
	// Hapus ampm
	dtm1[24] = '\0';
	dtm1[25] = '\0';
	printf ("debug dtm %s\n", dtm1);
	return 0;
}
/*
char* delspaceawal(char* dtm)
{
	int a;
	if (dtm == NULL)
		return NULL;
	a = 0;
	while (dtm[0]!='\0')
	{
		a = 1;
		if (isspace(dtm[0])!= 0)
			dtm++;
		else
			break;
	}
	if(a == 0)
		return NULL;
	return dtm;
}

char* datetime_binary_correcting(char* dtm)
{
	if (dtm== NULL)
		return NULL;
	// Jika ada spasi di awal, majukan pointer
	dtm = delspaceawal(dtm);
	if(apakah_datetime(dtm)==0)
		return NULL;
	if (strrchr(dtm, 'M')!=NULL)
		// Jika ampm, ubah menjadi tanggal biasa
		konversi_ampm_ke_biasa(dtm);
	return dtm;
}

*/

char* datetime_binary_correcting (char* dtm)
{
	int a, i;
	char temp[5];
	// printf ("debug datetime_binary_correcting() \n");
	if (dtm == NULL)
		return NULL;
	a = 0;
	while (dtm[0]!='\0')
	{
		a = 1;
		if (isspace(dtm[0])!= 0)
			dtm++;
		else
			break;
	}
	if(a == 0)
		return NULL;
	if (apakah_datetime(dtm)<=0)
		return NULL;
	if (apakah_datetime(dtm)==1)
		return dtm;
	if (!strcmp((char*)dtm+24, "AM"))
	{
		if ((char)dtm[11] == '1'&&(char)dtm[12] == '2')
		{
			dtm[11] = '0';
			dtm[12] = '0';
		}
	}
	if (!strcmp((char*)dtm+24, "PM"))
	{
		for (i=0; i< 5; i++)
			temp[i] = '\0';
		temp[0] = (char)dtm[11];
		temp[1] = (char)dtm[12];
		i = atoi(temp);
		if (i < 12)
			i+=12;
		sprintf (temp, "%d", i);
		dtm[11] = temp[0];
		dtm[12] = temp[1];
	}
	// Hapus ampm
	dtm[24] = '\0';
	dtm[25] = '\0';
	// printf ("debug datetime_binary_correcting() dtm %s\n", dtm);
	return dtm;
}


char* tanggal_buat_fnam(char* tanggal)
{
	if (tanggal == NULL)
		return NULL;
	memset (tanggal+4, '_', 1);
	memset (tanggal+7, '_', 1);
	memset (tanggal+10, NULL, 1);
	return tanggal;
}

/* Tidak diperbolehkan mengubah isi string pada parameter */
char* bulan_angka_ke_huruf(char* arg)
{
	/* format datetime: 2016/05/22 07:00:31.000  */
	char* temp;
	char* temp1;
	char* tanggal;
	char* bulan;
	char* tahun;
	int indeks_bulan = 13;
	if (arg == NULL)
		return NULL;
	temp1 = (char*) malloc (1024);
	if (temp1 == NULL)
		return NULL;
	if ((temp = ambil_tanggal_saja(arg))!=NULL)
	{
		tanggal = temp+8;
		memset (temp+7, 0, 1);
		bulan = temp+5;
		memset (temp+4, 0, 1);
		tahun = temp;
		if (!strcmp(bulan, "01"))
			indeks_bulan = 0;
		else if (!strcmp(bulan, "02"))
			indeks_bulan = 1;
		else if (!strcmp(bulan, "03"))
			indeks_bulan = 2;
		else if (!strcmp(bulan, "04"))
			indeks_bulan = 3;
		else if (!strcmp(bulan, "05"))
			indeks_bulan = 4;
		else if (!strcmp(bulan, "06"))
			indeks_bulan = 5;
		else if (!strcmp(bulan, "07"))
			indeks_bulan = 6;
		else if (!strcmp(bulan, "08"))
			indeks_bulan = 7;
		else if (!strcmp(bulan, "09"))
			indeks_bulan = 8;
		else if (!strcmp(bulan, "10"))
			indeks_bulan = 9;
		else if (!strcmp(bulan, "11"))
			indeks_bulan = 10;
		else if (!strcmp(bulan, "12"))
			indeks_bulan = 11;
		sprintf (temp1, "%s %s %s", tanggal, bln[indeks_bulan], tahun);
	}
	else
		return NULL;
	return temp1;
}

/* Tidak diperbolehkan mengubah isi dari parameter */
char* ambil_tanggal_saja (char* dtm)
{
	char* temp = NULL;
	if (datetime_binary_correcting (dtm) != NULL )
	{
		temp = (char*) malloc (1024);
		if (temp!=NULL)
		{
			memset (temp, 0, 1024);
			strcpy (temp, dtm);
			memset (temp+10, 0, 1);
		}
	}
	return temp;
}

/* Tidak diperbolehkan mengubah isi dari parameter */
char* ambil_waktu_tanpa_mili (char* dtm)
{
	char* temp = NULL;
	if (datetime_binary_correcting (dtm) != NULL )
	{
		temp = (char*) malloc (1024);
		if (temp!=NULL)
		{
			memset (temp, 0, 1024);
			strcpy (temp, dtm+11);
			memset (temp+8, 0, 1);
		}
	}
	return temp;	
}

// Jika -1 berarti string error
// Jika 0 berarti string tidak jelas
// Jika 1 berarti datetime biasa
// Jika 2 berarti datetime ampm

int apakah_datetime (char* dtm1)
{
	// Jika panjang string bukan ciri waktu biasa
	// atau waktu ampm maka reject.
	// Jika tanggal terdeteksi ampm maka lakukan sequence
	// pengubahan tanggal ampm ke tanggal format standar
	
	char* dtm;
	int mode, a;
	
	if (dtm1 == NULL)
		return -1;
	dtm = dtm1;
	mode = 0;
	a = 0;
	// cara yang lebih sederhana untuk menghilangkan spasi	
	// hilangkan spasi di depan
	while (dtm[0]!='\0')
	{
		a = 1;
		if (isspace(dtm[0])!=0)
			dtm++;
		else
			break;
	}
	
	if (a == 0)
		return 0;

	// cek apakah panjang string sesuai
	// untuk menghindari error penunjukan
	// pointer di bawah
	
	// printf ("debug %s\n", dtm);
	if (strlen (dtm1) < TANGGALWAKTU_PANJANG_STRING)
		return -1;
	
	// analisa
	if (isdigit((int)dtm[0])
		&&isdigit((int)dtm[1])
		&&isdigit((int)dtm[2])
		&&isdigit((int)dtm[3])
		&&(dtm[4] == '/')
		&&isdigit((int)dtm[5])
		&&isdigit((int)dtm[6])
		&&(dtm[7] == '/')
		&&isdigit((int)dtm[8])
		&&isdigit((int)dtm[9])
		&&isspace((int)dtm[10])
		&&isdigit((int)dtm[11])
		&&isdigit((int)dtm[12])
		&&(dtm[13] == ':')
		&&isdigit((int)dtm[14])
		&&isdigit((int)dtm[15])
		&&(dtm[16] == ':')
		&&isdigit((int)dtm[17])
		&&isdigit((int)dtm[18])
		&&(dtm[19] == '.')
		&&isdigit((int)dtm[20])
		&&isdigit((int)dtm[21])
		&&isdigit((int)dtm[22]))
			mode = 1;
	if ((mode == 1) &&
			(strlen (dtm) >= TANGGALWAKTU_AMPM_PANJANG_STRING) &&
				strrchr(dtm, 'M') != NULL)
		mode = 2;
	return mode;
}



/*
Fungsi apakah_datetimt_binary() mirip dengan
fungsi apakah_datetime(),
Untuk digunakan pada file ekstraksi dcs event biner, kolom waktu
didahului dengan spasi. Selain itu, fungsi
ini sama dengan apakah_datetime()
*/

char unit_manakah (char* kks)
{
	if (apakah_kks(kks)==1)
		return kks[0];
	return -1;
}
/*
int apakah_datetime_binary (char* dtm)
{
	if (dtm == NULL && strlen (dtm)!=
			TANGGALWAKTU_BINARY_PANJANG_STRING)
		return -1;
	if (isspace((int)dtm[0])
		&&isdigit((int)dtm[1])
		&&isdigit((int)dtm[2])
		&&isdigit((int)dtm[3])
		&&isdigit((int)dtm[4])
		&&(dtm[5] == '/')
		&&isdigit((int)dtm[6])
		&&isdigit((int)dtm[7])
		&&(dtm[8] == '/')
		&&isdigit((int)dtm[9])
		&&isdigit((int)dtm[10])
		&&isspace((int)dtm[11])
		&&isdigit((int)dtm[12])
		&&isdigit((int)dtm[13])
		&&(dtm[14] == ':')
		&&isdigit((int)dtm[15])
		&&isdigit((int)dtm[16])
		&&(dtm[17] == ':')
		&&isdigit((int)dtm[18])
		&&isdigit((int)dtm[19])
		&&(dtm[20] == '.')
		&&isdigit((int)dtm[21])
		&&isdigit((int)dtm[22])
		&&isdigit((int)dtm[23]))
			return 1;
	return 0;
}
*/


/*
Fungsi apakah_kks() untuk mengecek apakah
string yang ada pada argumen adalah KKS
dengan melihat signature. Jika ada nilainya 1,
jika tidak ada nilainya 0, jika fault
nilainya -1
*/
int apakah_kks (char* kks)
{	
	if (kks ==NULL || strlen(kks)<6)
		return -1;
	if ((isdigit((int)kks[0])
		&&(kks[0] == '5'
		||kks[0] == '6'
		||kks[0] == '0'))
		&& isspace ((int)kks[1])
		&& (isdigit ((int)kks[2])
		&& kks[2] == '0')
		&& isalpha ((int)kks[3])
		&& isalpha ((int)kks[4])
		&& isalpha ((int)kks[5]))
			return 1;
	return 0;
}

/*
Fungsi untuk mengecek apakah string
pada argumen adalah tag. Jika benar, maka nilainya
bilangan positif berupa tag nomor berapa,
jika Jika salah nilainya 0, jika fault nilainya
adalah -1

int apakah_tag (char* str)
{
	char temp[128];
	if (str == NULL)
		return -1;	
	memset (temp, '\0', 10);
	strcpy(temp, str);
	if (temp[0] == 'T'
		&& temp[1] == 'a'
		&& temp[2] == 'g'
		&& isdigit ((int)temp[3]))
			return strtol (temp+3, NULL, 0);
	return 0;
}

*/

int apakah_tag (char* str)
{
	if (str == NULL)
		return -1;
	if (str[0] == 'T'
		&& str[1] == 'a'
		&& str[2] == 'g'
		&& isdigit ((int)str[3]))
			return strtol (str+3, NULL, 0);
	return 0;
}
/*
Ini adalah fungsi untuk mengecek apakah
string2 ada sebagai bagian di string1, Jika str2 ada
di str 1, jika ada maka return 1, jika tidak
return 0, jika fault return -1
*/
int apakah_ada (char* str1, char* str2)
{
	unsigned int i = 0;
	char temp[1024];

	if ((strlen (str2) > strlen (str1))||(strlen(str2) > 1024))
		return -1;
	while (i < strlen (str1)-strlen(str2)+1)
	{
		memset (temp, '\0', 1024);
		memcpy (temp, str1+i, strlen(str2));
		if (!strcmp(temp, str2))
			return 1;
		i++;
	}
	return 0;
}

/* Ini akan menyalin data untuk return.    */
char kks_analog_trend_list_get_plant_unit (struct kks_analog_trend_list* arg)
{
	if (arg == NULL)
		return '-';
	return arg->plant_unit;
}


struct kks_analog_trend_list* kks_analog_trend_list_new
	(struct word_extractor* word_extr)
{
	struct kks_analog_trend_list* temp;
	struct content_word* all_content_data = NULL;
	struct word_extractor* file_data;
	struct kks_analog_trend* kks_analog_trend_temp;
	int adalah_t_3000 = 0, adalah_file_analog = 0;
	int unit_found = 0;
	int cari = 0;
	if (word_extr == NULL)
		return NULL;
	if (word_extr->contents == NULL)
		return NULL;
	temp = (struct kks_analog_trend_list*) malloc
		(sizeof (struct kks_analog_trend_list));
	if (temp != NULL)
	{
		file_data = word_extr;
		all_content_data = file_data->contents;
		while (all_content_data->prev != NULL)
			all_content_data = all_content_data->prev;
		temp->source_data = file_data;
		temp->all_content_data = all_content_data;
		temp->list_content = NULL;
		temp->timeline = NULL;
	}
	// printf ("debug kks_analog_trend_list_new() begin\n");
	while (all_content_data->prev != NULL)
		all_content_data = all_content_data->prev;	
	while (all_content_data!=NULL)
	{
		/*
		engine ini hanya mencari data di halaman
		pertama dari report. Jadi, kata "Page"
		digunakan sebagai penanda akhir halaman.
		Jika kata ini ditemukan, maka proses
		scanning dihentikan.
		Untuk data yang memakai point group,
		maka batasan dengan kata "page" saja
		tidak cukup. Maka harus dicari kata
		yang membatasi antara daftar KKS dan
		data yang sebenarnya.
		*/
		if (!strcmp (all_content_data->the_word,
				FILE_SIGNATURE_T3000))
			adalah_t_3000 = 1;
		if (!strcmp (all_content_data->the_word,
				FILE_SIGNATURE_ANALOG))
			adalah_file_analog = 1;
		if (adalah_t_3000 != 1 && adalah_file_analog != 1)
		{
			if (cari > 200)
			{
				printf ("\nBukan analog file yang valid. Program dihentikan.\n");
				exit (-1);
			}
			cari++;
			continue;
		}
		/*  if (apakah_ada (all_content_data->the_word, END_WORD)==1) 
			Daftar KKS bisa lebih dari 1 halaman. Jadi, batasan
			dengan "page" diganti dengan tag. Karena "tagx"
			adalah satu-satunya pembeda antara daftar KKS dengan
			isi data analog */
		if (apakah_tag (all_content_data->the_word) > 0)
			break;
		/*
		Jika KKS ditemukan, maka dilanjutkan dengan
		mencari data trend analog.
		Jika data tidak ditemukan, maka isi struktur
		akan NULL.
		*/
		if (unit_found == 0)
		{
			if (apakah_kks(all_content_data->the_word)==1)
			{
				temp->plant_unit = unit_manakah(all_content_data->the_word);
				unit_found = 1;
			}
		}
		if (apakah_kks(all_content_data->the_word)==1)
		{
			/*
			Membuat storage trend analog
			berdasarkan KKS dari chunk saat ini
			*/
		// printf ("debuggggg\n");

			kks_analog_trend_temp = kks_analog_trend_new
				(temp->source_data, all_content_data->the_word, temp);
			if (kks_analog_trend_temp!=NULL)
				kks_analog_trend_search_data (kks_analog_trend_temp);
			/*
			Masukkan data tambahan (KKS, deskripsi)
			pada struktur
			*/
			if (kks_analog_trend_temp!=NULL)
				kks_analog_trend_add (temp, kks_analog_trend_temp);
		}
		all_content_data = all_content_data->next;
	}
	// printf ("debug kks_analog_trend_list_new() end\n");
	if (adalah_t_3000 != 1 && adalah_file_analog != 1)
	{
		printf ("Bukan file data analog T3000. Program dihentikan.\n");
		exit (-1);
	}
	return temp;
}

int kks_analog_trend_add (struct kks_analog_trend_list* chain,
	struct kks_analog_trend* chunk)
{
	if (chunk == NULL || chain == NULL)
		return -1;
	if (chain->list_content == NULL)
		chain->list_content = chunk;
	else
	{
		while (chain->list_content->next != NULL)
			chain->list_content = chain->list_content->next;
		chunk->index = chain->list_content->index;
		chunk->index++;
		chain->list_content->next = chunk;
		chunk->prev = chain->list_content;
		chunk->next = NULL;
	}
	return 0;
}


/*
Menginisialisasi struktur data untuk satu KKS analog trend
*/
struct kks_analog_trend*
	kks_analog_trend_new
		(struct word_extractor* wordext, char* kks,
			struct kks_analog_trend_list* list_kks)
{
	struct kks_analog_trend* temp;
	if (wordext == NULL|| kks == NULL || list_kks == NULL)
		return NULL;
	if (!apakah_kks(kks))
		return NULL;
	temp = (struct kks_analog_trend*) malloc
		(sizeof (struct kks_analog_trend));
	if (temp!=NULL)
	{
		/* 
		KKS digunakan untuk referensi kode yang akan
		dieksekusi selanjutnya. Jika hanya mengambil
		pointer dari parameter, bisa jadi string
		akan diubah isinya. Jadi, dibuat alokasi
		memori baru. Jika nanti datanya sudah ditemukan,
		alokasi memori ini bisa difree, dan diganti
		dengan pointer dari word extractor.
		*/
		temp->kks = NULL;
		if (kks!=NULL)
		{
			temp->kks = (char*) malloc (1024);
			memset (temp->kks, '\0', 1024);
			strcpy(temp->kks, kks);
		}
		temp->description = NULL;
		temp->eng_unit = NULL;
		temp->time_begin = 0;
		temp->time_end = 0;
		temp->max_val = 0;
		temp->max_time = 0;
		temp->min_val = 0;
		temp->min_time = 0;
		temp->avg_val = 0;
		temp->tag_pos = -1;
		temp->prev = NULL;
		temp->next = NULL;
		temp->data = NULL;
		temp->source_data = wordext;
		temp->kks_list = list_kks;
	}
	return temp;
}

/*
Menginisialisasi chunk data untuk isi nilai analog
pada setiap slot waktu
*/
struct kks_analog_trend_data*
	kks_analog_trend_data_new()
{
	struct kks_analog_trend_data* temp;
	temp = (struct kks_analog_trend_data*) malloc
		(sizeof(struct kks_analog_trend_data));
	if (temp!=NULL)
	{
		temp->prev = NULL;
		temp->next = NULL;
		temp->upr = NULL;
		temp->lwr = NULL;
		temp->index = 0;
		temp->kks = NULL;
		temp->description = NULL;
		temp->eng_unit = NULL;
		temp->time_str = NULL;
		temp->val = NULL;
		temp->val_f = -888;
	}
	return temp;
}

/*
Menambahkan chunk data nilai analog per slot waktu
pada struktur analog trend per KKS
*/
int kks_analog_trend_data_add
	(struct kks_analog_trend* chain,
		struct kks_analog_trend_data* chunk)
{
	if (chunk == NULL)
		return -1;
	if (chain->data == NULL)
		chain->data = chunk;
	else
	{
		while (chain->data->next != NULL)
			chain->data = chain->data->next;
		chunk->index = chain->data->index;
		chunk->index++;
		chain->data->next = chunk;
		chunk->prev = chain->data;
		chunk->upr = chain;
	}
	return 0;	
}

int kks_analog_trend_apakah_analog_grid(struct content_word* arg)
{
	int adalah_t_3000 = 0, adalah_file_event = 0;
	struct content_word* temp;
	if (arg == NULL)
		return -1;	
	temp = arg;
	while (temp->prev != NULL)
		temp = temp->prev;
	while (temp!=NULL)
	{
		if (!strcmp (temp->the_word,
					FILE_SIGNATURE_T3000))
				adalah_t_3000 = 1;
		if (!strcmp (temp->the_word,
					FILE_SIGNATURE_ANALOG))
				adalah_file_event = 1;
		if (adalah_t_3000 == 1 && adalah_file_event == 1)
			return 1;
	temp = temp->next;
	}
	return 0;
}


/*

Mencari nilai analog pada file trend analog,
dan memasukkan nilainya ke struktur data analog
per KKS.
modul ini ada bug untuk dua KKS data. Hasilnya berantakan.
KKS kedua melompat antara waktu ampm dengan tanda v (void).
KKS pertama isinya value kedua.
Ternyata ini karena pada data asli ada tag1 yang kosong.
Ini sangat mungkin karena pada waktu memasukkan kks, ada
satu baris yang dibiarkan kosong dan tidak dihapus. Jadi
ini akan menghasilkan tag baru. Eureka. ALhamdulillah.

So, jika ada baris kosong pada waktu kita akan setup
analog grid, maka pada header nomor eken melompat,
tidak urut. Begitu juga dengan baris nilai data.

Ada tiga pilihan untuk data yang kondisinya seperti ini,
agar ke depannya tidak lagi terjadi kesalahan penulisan data.


Pilihan pertama adalah memperingatkan user pada menu help,
bahwa tidak diperbolehkan untuk mengosongkan salah satu
baris item.

Pilihan kedua adalah menggenerate error dan menghentikan program
jika ditemukan nomor indeks yang tidak urut.

Pilihan ke tiga adalah menulis ulang keseluruhan kode. Pilihan
ini memerlukan riset lagi dengan mendownload banyak data untuk
satu dan lebih dari satu item kosong di depan, di tengah, dan
di belakang. Ini akan membutuhkan banyak data.

*/
int kks_analog_trend_search_data (struct kks_analog_trend* temp)
{
	struct content_word* content;
	struct content_word* cari_tag;
	struct content_word* cari_tag1;	
	struct kks_analog_trend_data* trend;
	struct tmline* timeline = NULL;
	struct tmline* timeline_head = NULL;
	int nilai_tag;
	int i, j;
	int tidak_pake_description = 0;
	div_t kalkulasi_tag;
	int header_scan_kumplit = 0;
	int kata_time_ketemu = 0;
	int data_per_waktu_permulaan = 1;
	int tag_ketemu;	
	char kks_temp[1024];
	if (temp ==NULL)
		return -1;
	if (temp->source_data == NULL)
		return -1;
	content = temp->source_data->contents;
	if (content == NULL)
		return -1;
	while (content->prev != NULL)
		content = content->prev;
	memset (kks_temp, 0, 1024);
	strcpy (kks_temp, temp->kks);
	free (temp->kks);
	while (content != NULL) 
	{
		if (apakah_kks (content->the_word)==1&&
				header_scan_kumplit == 0)
		{
			if (!strcmp(content->the_word, kks_temp))
			{
				/*
				Alokasi memori untuk KKS pada inisialisasi
				objek bisa difree di sini, dan diganti dengan pointer
				dari word extractor.
				*/
				//=========
				temp->kks = content->the_word;
				//=========
				cari_tag = content;
				cari_tag = cari_tag->prev;
				if ((nilai_tag = apakah_tag
						(cari_tag->the_word)) > 0)		
				{
					// ini adalah kode baru untuk menanggulangi
					// data hasil error karena ada item kosong
					// pada form event sequence.
					// Kita pakai solusi pilihan ke dua.
					// this code is a shit. Need a fix.
					//-------
					cari_tag1 = cari_tag;
					cari_tag1 = cari_tag1->prev;
					if ((apakah_tag
							(cari_tag1->the_word)) > 0)
							{
								printf ("\nBaris KKS kosong terdeteksi. Program dihentikan.\n");
								exit (-1);
							}
					cari_tag1 = cari_tag1->next;
					cari_tag1 = cari_tag1->next;							
					if ((apakah_tag
							(cari_tag1->the_word)) > 0)
							{
								printf ("\nBaris KKS kosong terdeteksi. Program dihentikan.\n");
								exit (-1);
							}
					//-------
					temp->tag_pos = nilai_tag;
					for (j = 0; j<2; j++)
					{
						cari_tag = cari_tag->next;
						if (apakah_tag(cari_tag->the_word)>0)
							tidak_pake_description = 1;
					}
					// cari_tag = cari_tag->next;
					// cari_tag = cari_tag->next;
					if (tidak_pake_description == 0)
						temp->description = cari_tag->the_word;
					else
						temp->description = NULL;
					while (cari_tag != NULL)
					{
						if (!strcmp (cari_tag->the_word, TIME_WORD))
						{
							kata_time_ketemu = 1;
							break;
						}
						cari_tag = cari_tag->next;
					}
					if (kata_time_ketemu == 1)
					{
						cari_tag = cari_tag->next;
						while (cari_tag != NULL &&
							!(apakah_tag (cari_tag->the_word)<= 0))
							cari_tag = cari_tag->next;
						if (cari_tag != NULL)
						{
							if (nilai_tag >= 10)
							{
								kalkulasi_tag = div(nilai_tag, 9);
								nilai_tag =
									(kalkulasi_tag.rem < 1) ? 9 : kalkulasi_tag.rem;
							}					
							for (i=1; i< nilai_tag; i++)
							{
								if (cari_tag->next != NULL)
									cari_tag = cari_tag->next;
							}
							/*
							temp->eng_unit = (char*) malloc (1024);
							memset (temp->eng_unit, '\0', 1024);
							if (temp->eng_unit!=NULL)
							{
								strcpy(temp->eng_unit, cari_tag->the_word);
								header_scan_kumplit = 1;
							}
							*/
							
							if (tidak_pake_description != 1)
								temp->eng_unit = cari_tag->the_word;
							header_scan_kumplit = 1;
						}
					}
				}
				else
				{
					content = content->next;
					continue;
				}
			}
			else
			{
				content = content->next;
				continue;
			}
		}
		if (header_scan_kumplit == 1)
		{
			if (data_per_waktu_permulaan == 1)
			{
				while (content->prev!=NULL)
					content = content->prev;
				data_per_waktu_permulaan = 0;
			}
			if (!strcmp(content->the_word, TIME_WORD))
			{
				content = content->next;
				tag_ketemu = 0;
				while (content->next!=NULL &&
						((nilai_tag = apakah_tag(content->the_word))>0))
				{
					if (apakah_tag(content->the_word)==temp->tag_pos)
						tag_ketemu = 1;
					content = content->next;
				}
				if (tag_ketemu == 1)
				{
					while (content->next!=NULL &&
						strcmp(content->the_word, END_WORD))
					{
						if (apakah_datetime(content->the_word) > 0)
						{
							trend = kks_analog_trend_data_new();
							if (trend != NULL)
							{
								trend->kks = temp->kks;
								/*
								trend->time_str = (char*) malloc (1024);
								if (trend->time_str != NULL)
								{
									memset (trend->time_str, '\0', 1024);
									strcpy(trend->time_str, content->the_word);
								}
								*/
								if (temp->kks_list->timeline == NULL)
								{
									timeline = tmline_new();
									if (timeline != NULL)
										content->the_word = datetime_binary_correcting(content->the_word);
										timeline->time_str = content->the_word;
								}
								content->the_word = datetime_binary_correcting(content->the_word);
								trend->time_str = content->the_word;
								if (tidak_pake_description == 0)
								{	
									if (temp->tag_pos <10)
										nilai_tag = temp->tag_pos;
									else
									{
										kalkulasi_tag = div(temp->tag_pos, 9);
										nilai_tag =
											(kalkulasi_tag.rem < 1) ? 9 : kalkulasi_tag.rem;
									}
									for (i = 0; i < nilai_tag; i++ )
									{
										if(content->next != NULL)
											content = content->next;
									}
									/*
									trend->val = (char*) malloc (1024);
									if (trend->val != NULL)
									{
										memset (trend->val, '\0', 1024);
										strcpy(trend->val, content->the_word);
										trend->val_f = strtod(trend->val, NULL);
									}
									*/
									trend->val = content->the_word;
									trend->val_f = strtod(trend->val, NULL);
									trend->description = temp->description;
									trend->eng_unit = temp->eng_unit;
								}
								else
								{
									trend->val = "v";
								}
							}
							if (temp->kks_list->timeline == NULL)
							{
								if (timeline_head == NULL)
									timeline_head = timeline;
								else
								{
									while (timeline_head->next != NULL)
										timeline_head = timeline_head->next;
									timeline->index = timeline_head->index;
									timeline->index++;
									timeline_head->next = timeline;
									timeline->prev = timeline_head;									
								}
							}
							kks_analog_trend_data_add(temp, trend);
						}
						content = content->next;
					}
				}
			}
		}
		content = content->next;
	}
	if (temp->kks_list->timeline == NULL)
		temp->kks_list->timeline = timeline_head;
	/* Kode di bawah ini adalah untuk menghindari program crash
		karena file yang diinput tidak sesuai */
	if (header_scan_kumplit == 0)
	{
		printf ("Tolong file analognya dicek lagi. Program dihentikan.\n");
		exit (-1);
	}
	return 0;
}

int kks_analog_trend_print(struct kks_analog_trend* trend)
{
	struct kks_analog_trend_data* content;
	
	if (trend ==NULL )
		return -1;
	if (trend->data == NULL)
		return -1;
	content = trend->data;
	while (content->prev != NULL)
		content = content->prev;
	while (content != NULL)
	{
		printf ("Index: %d, KKS:%s, Time: %s, Value: %s, Desc: %s\n",
			content->index, content->kks, content->time_str,
				content->val, trend->eng_unit, trend->description);
		content = content->next;
	}
	return 0;
}

int kks_analog_trend_print_file (struct kks_analog_trend* trend)
{
	struct kks_analog_trend_data* content;
	FILE* file_output;
	int i = 0;
	char fnam[1024];
	if (trend ==NULL )
		return -1;
	if (trend->data == NULL)
		return -1;
	memset (fnam, 0, 1024);	
	strcpy (fnam, trend->kks);
	strcpy (fnam+strlen(fnam), ".csv");
	while (fnam[i]!=0)
	{
		if (fnam[i] == '|')
			fnam[i] = '_';
		i++;
	}
	file_output = fopen (fnam, "w");
	if (file_output == NULL)
	{
		printf ("Error open file output %s. Program dihentikan!\n", fnam);
		exit(-1);
	}
	content = trend->data;
	while (content->prev != NULL)
		content = content->prev;
	while (content != NULL)
	{
		fprintf (file_output, "%d;%s;%s;*%s;%s;%s;%s\n",
			content->index, content->time_str, content->kks, content->time_str,
				content->val, trend->eng_unit, trend->description);
		content = content->next;
	}
	if (fclose (file_output) != 0)
	{
		printf
			("Error close file output %s. Program dihentikan.\n",
				fnam);
		exit (-1);
	}
	return 0;
}


/*
Jika KKS ditemukan, maka nilai return adalah pointer ke struktur.
Jika KKS tidak ditemukan, maka nilai return adalah NULL.
Pointer ke struktur hasil return adalah train, yang bisa diproses
oleh fungsi lainnya.
*/
struct kks_analog_trend* kks_analog_trend_find_kks
	(struct kks_analog_trend_list* arg, char* kks)
{
	struct kks_analog_trend* temp;
	if (arg == NULL || kks == NULL)
	{
		return NULL;
	}
	if (apakah_kks(kks) != 1)
	{
		return NULL;
	}
	temp = arg->list_content;
	while (temp->prev != NULL)
		temp = temp->prev;
	while (temp != NULL)
	{
		if (!strcmp(temp->kks, kks))
			break;
		temp = temp->next;
	}
	return temp;
}

/* fungsi ini adalah lanjutan dari fungsi di atas, akan mencari data berdasarkan
waktu. Nilai yang dikembalikan adalah satu chunk waktu yang dimaksud. */
/*  Hasil return adalah hasil satu chunk waktu untuk satu struktur kks.  */
struct kks_analog_trend* kks_analog_trend_find_time
	(struct kks_analog_trend_list* arg, char* kks, char* time_arg)
{
	struct kks_analog_trend* temp;
	struct kks_analog_trend_data* temp_data;
	char* time_temp;
	if (arg == NULL || kks == NULL || time_arg == NULL)
		return NULL;
	time_temp = time_arg;
	/* apakah waktu sudah sesuai  */
	if ((time_temp = datetime_binary_correcting(time_temp))== NULL)
		return NULL;
	/* temukan satu kks terlebih dahulu
	Jika ketemu baru cari timenya. (biasanya trend dari dcs timelinenya
	sama untuk semua kks */
	temp = kks_analog_trend_find_kks(arg, kks);
	if (temp == NULL || temp->data == NULL)
		return NULL;
	/* Ini bukan trend_data, jadi mengapa harus direset posisinya? */
	temp_data = temp->data;
	while (temp_data->prev != NULL)
		temp_data = temp_data->prev;
	while (temp!=NULL)
	{
		/* apakah ada waktu di struktur? */
		if (!strcmp(temp_data->time_str, time_temp))
			return temp;
		temp_data = temp_data->next;
	}
	/* data tidak ditemukan */
	if (temp_data == NULL)
		return NULL;
	return temp;
}

/*
Kita akan membuat deretan data available untuk
satu waktu.
Fungsi ini digunakan untuk mengekstraksi semua
data KKS dalam satu waktu. Jika dalam trend analog
ada 5 kks, maka kelima data akan diambil
dan disimpan dalam linked list khusus wrapper.
*/
struct kks_analog_trend_data_wrapper* kks_analog_trend_get_data_at
	(struct kks_analog_trend_list* data, char* datetime)
{
	struct kks_analog_trend* kks_list;
	struct kks_analog_trend_data* temp;
	struct kks_analog_trend_data_wrapper* wrapper;
	struct kks_analog_trend_data_wrapper* wrapper_head = NULL;
	char* tanggal;
	if (datetime == NULL)
		return NULL;
	if ((tanggal = datetime_binary_correcting(datetime)) == NULL)
		return NULL;
	if (data == NULL)
		return NULL;
	if (data->list_content == NULL)
		return NULL;
	kks_list = data->list_content;
	while (kks_list->prev != NULL)
		kks_list = kks_list->prev;
	while (kks_list != NULL)
	{
		/* mengabil train data di setiap kks   */
		temp = kks_list->data;
		if (temp != NULL)
		{
			/* reset posisi pointer train   */
			while (temp->prev != NULL)
				temp = temp->prev;
			/* scanning apakah tanggal ada    */
			while (temp != NULL)
			{
				/* Jika ada maka buat wrapper baru   */
				if (!strcmp (temp->time_str, tanggal))
				{
					/* bikin chunk baru dan tambahkan data   */
					/* Menambahkan data ada sedikit perbedaan
						dengan sebelumnya, di mana tidak ada
						upr */
					wrapper = kks_analog_trend_data_wrapper_new();
					if (wrapper != NULL)
					{
						wrapper->data = temp;
						if (wrapper_head == NULL)
							wrapper_head = wrapper;
						else
							kks_analog_trend_data_wrapper_add
								(wrapper_head, wrapper);
					}

					/* Data sudah ditemukan, tidak perlu mencari lagi,
						pasti hanya satu data */
				break;
				}
				/* terus scanning jika data belum ketemu   */
				temp = temp->next;
			}
		}
		kks_list = kks_list->next;
	}
	return wrapper_head;
}

struct kks_analog_trend_data_wrapper*
kks_analog_trend_data_wrapper_new()
{
	struct kks_analog_trend_data_wrapper* temp;
	temp = (struct kks_analog_trend_data_wrapper*)
			malloc (sizeof (struct kks_analog_trend_data_wrapper));
	if (temp == NULL)
		return NULL;
	temp->prev = NULL;
	temp->next = NULL;
	temp->upr = NULL;
	temp->lwr = NULL;
	temp->index = 0;
	temp->data = NULL;
	return temp;
}

int kks_analog_trend_data_wrapper_add
	(struct kks_analog_trend_data_wrapper* chain,
	struct kks_analog_trend_data_wrapper* chunk)
{
	if (chunk == NULL)
		return -1;
	if (chain == NULL)
		return 1;
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

int kks_analog_trend_data_wrapper_join
	(struct kks_analog_trend_data_wrapper* chain,
	struct kks_analog_trend_data_wrapper* chunk)
{
	if (chunk == NULL)
		return -1;
	if (chain == NULL)
		return -1;
	if (chain != NULL)
	{
		while (chain->lwr != NULL)
			chain = chain->lwr;
		chunk->index = chain->index;
		chunk->index++;
		chain->lwr = chunk;
		chunk->upr = chain;
	}
	return 0;		
}

int kks_analog_trend_data_wrapper_delete
	(struct kks_analog_trend_data_wrapper* arg)
{
	/*
	Pada prinsipnya, kita tidak perlu
	free data chunk dari analog trend,
	karena yang kita masukkan adalah pointer
	data original. Kita hanya free wrappernya
	saja.
	*/
	struct kks_analog_trend_data_wrapper* temp;
	if (arg == NULL)
		return -1;
	while (arg->prev != NULL)
		arg = arg->prev;
	while (arg!=NULL)
	{
		temp = arg->next;
		arg->data = NULL;
		arg->prev = NULL;
		arg->next = NULL;
		free (arg);
		arg = temp;
	}
	return 0;
}

struct kks_analog_trend_data_wrapper*
	kks_analog_trend_data_wrapper_find_from_kks
	(struct kks_analog_trend_data_wrapper* data,
	char* kks)
{
	struct kks_analog_trend_data_wrapper* temp;
	if (data == NULL || kks == NULL)
		return NULL;
	if (apakah_kks(kks) != 1)
		return NULL;
	temp = data;
	if (temp->prev != NULL)
		temp = temp->prev;
	while (temp != NULL)
	{
		if (!strcmp (kks, temp->data->kks))
			break;
		temp = temp->next;
	}
	return temp;
}

double kks_analog_trend_data_wrapper_get_value
	(struct kks_analog_trend_data_wrapper* data)
{
	if (data == NULL)
		return 666666;
		/* Tidak ada nilai di paiton yang seperti ini */
	return data->data->val_f;
}

/* Mengembalikan nilai return dari data asli, jangan dimodifikasi!!!  */
char* kks_analog_trend_find_time_start (struct kks_analog_trend* data)
{
	char* temp;
	struct kks_analog_trend_data* temp_data;
	if (data == NULL)
		return NULL;
	temp_data = data->data;
	if (temp_data == NULL)
		return NULL;
	while (temp_data->prev != NULL)
		temp_data = temp_data->prev;
	temp = temp_data->time_str;
	return temp;
}

struct kks_analog_trend_data*
kks_analog_trend_find_time_start_chunk (struct kks_analog_trend* data)
{
	struct kks_analog_trend_data* temp_data;
	if (data == NULL)
		return NULL;
	temp_data = data->data;
	if (temp_data == NULL)
		return NULL;
	while (temp_data->prev != NULL)
		temp_data = temp_data->prev;
	return temp_data;
}

/* Mengembalikan nilai return dari data asli, jangan dimodifikasi!!! */
char* kks_analog_trend_get_time_ptr (struct kks_analog_trend_data* data)
{
	if (data == NULL)
		return NULL;
	return data->time_str;
}

/* fungsi ini pada dasarnya akan mencari apakah ada waktu start dan
waktu end pada data yang ada, berdasarkan masukan waktu start dan
akan dihitung waktu end, kemudian dicari apakah kedua waktu itu ada.
Hasilnya akan disimpan di result. Result adalah memori selebar
1024byte yang siap pakai dan diinisialisasi dengan karakter NULL.
Nilai yang dikembalikan jika operasi sukses adalah pointer result,
jika gagal nilai yang dikembalikan adalah NULL.
Hati-hati, yang dikembalikan adalah pointer data asli,
jangan memodifikasi!
*/
char* kks_analog_trend_find_time_pair (struct kks_analog_trend* data, 
	char* timestart, double duration, char* result)
{
	struct kks_analog_trend_data* temp_data;
	int cari_waktu_start,
		waktu_start_ketemu;
	int jumlah_menit;
	char* time_end;
	if (timestart == NULL || data == NULL || result == NULL)
		return NULL;
	if (duration < DROPTEST_DURATION_MINIMUM)
		return NULL;
	/* cek apakah format tanggal benar   */
	if (datetime_binary_correcting (timestart) == NULL)
		return NULL;
	temp_data = data->data;
	if (temp_data == NULL)
		return NULL;
	/*
	Bikin kalkulasi time end terlebih dahulu,
	baru hasil kalkulasinya digunakan untuk mencari
	time end apakah ada atau tidak
	*/
	time_end = result;
	strcpy (time_end, timestart);
	jumlah_menit = duration;
	if ((time_end = add_minutes (time_end, jumlah_menit)) == NULL)
		return NULL;
	/* Mulai mencari */
	while (temp_data->prev != NULL)
		temp_data = temp_data->prev;
	cari_waktu_start = 1;
	waktu_start_ketemu = 0;
	while (temp_data != NULL)
	{
		if (cari_waktu_start == 1 
			&& !strcmp (timestart, temp_data->time_str))
		{
			waktu_start_ketemu = 1;
			cari_waktu_start = 0;
		}
		if (waktu_start_ketemu == 1)
		{
			if (!strcmp (time_end,
				temp_data->time_str))
					break;
		}
		temp_data = temp_data->next;
	}
	if (temp_data == NULL)
		return NULL;
	return temp_data->time_str;
}

struct kks_analog_trend_data*
kks_analog_trend_find_time_pair_chunk
	(struct kks_analog_trend* data, char* timestart,
	double duration)
{
	struct kks_analog_trend_data* temp_data;
	int cari_waktu_start,
		waktu_start_ketemu;
	int jumlah_menit;
	char time_end[1024];
	char* time_end1;
	if (timestart == NULL || data == NULL)
		return NULL;
	if (duration < DROPTEST_DURATION_MINIMUM)
		return NULL;
	/* cek apakah format tanggal benar   */
	if (datetime_binary_correcting(timestart) == NULL)
		return NULL;
	temp_data = data->data;
	if (temp_data == NULL)
		return NULL;
	/*
	Bikin kalkulasi time end terlebih dahulu,
	baru hasil kalkulasinya digunakan untuk mencari
	time end apakah ada atau tidak
	*/
	memset (time_end, 0, 1024);
	strcpy (time_end, timestart);
	jumlah_menit = duration;
	if ((time_end1 = add_minutes (time_end, jumlah_menit)) == NULL)
		return NULL;
	/* Mulai mencari */
	while (temp_data->prev != NULL)
		temp_data = temp_data->prev;
	cari_waktu_start = 1;
	waktu_start_ketemu = 0;
	while (temp_data != NULL)
	{
		if (cari_waktu_start == 1 
			&& !strcmp (timestart, temp_data->time_str))
		{
			waktu_start_ketemu = 1;
			cari_waktu_start = 0;
		}
		if (waktu_start_ketemu == 1)
		{
			if (!strcmp (time_end1,
				temp_data->time_str))
					break;
		}
		temp_data = temp_data->next;
	}
	if (temp_data == NULL)
		return NULL;
	return temp_data;
}


/*
Fungsi consistent decrease ini akan melakukan
beberapa aktivitas:
Mengecek apakah timestart ada di trend
Mengecek apakah timeend ada di trend, berdasarkan
timestart dan duration
Mengecek apakah trend dengan kks dimaksud pada
jangka waktu timestart dan timeend nilai penurunannya
konsisten
*/
char* kks_analog_trend_consistent_decrease
	(struct kks_analog_trend_data* data, char* timestart,
	double duration, char* result)
{
	struct kks_analog_trend_data* temp_data;
	int jumlah_menit;
	double nilai_awal;
	int cari_waktu_start,
		waktu_start_ketemu,
			trend_naik;
	char* time_end;
	if (timestart == NULL || data == NULL || result == 0)
		return NULL;
	if (duration < DROPTEST_DURATION_MINIMUM)
		return NULL;
	/* cek apakah format tanggal benar   */
	if (datetime_binary_correcting(timestart) == NULL)
		return NULL;
	temp_data = data;
	if (temp_data == NULL)
		return NULL;
	/*
	Bikin kalkulasi time end terlebih dahulu,
	baru hasil kalkulasinya digunakan untuk mencari
	time end apakah ada atau tidak
	*/
	time_end = result;
	strcpy (time_end, timestart);
	jumlah_menit = duration;
	if ((time_end = add_minutes (time_end, jumlah_menit)) == NULL)
		return NULL;
	/* Mulai mencari */
	while (temp_data->prev != NULL)
		temp_data = temp_data->prev;
	cari_waktu_start = 1;
	waktu_start_ketemu = 0;
	trend_naik = 0;
	if (temp_data != NULL)
		nilai_awal = temp_data->val_f;
	while (temp_data != NULL)
	{
		/* Deteksi apakah ada kenaikan trend */
		if (temp_data->val_f > nilai_awal)
		{
			trend_naik = 1;
			break;
		}
		if (cari_waktu_start == 1 
			&& !strcmp (timestart, temp_data->time_str))
		{
			waktu_start_ketemu = 1;
			cari_waktu_start = 0;
		}
		if (waktu_start_ketemu == 1)
		{
			if (!strcmp (time_end,
				temp_data->time_str))
					break;
		}
		nilai_awal = temp_data->val_f;
		temp_data = temp_data->next;
	}
	if (temp_data == NULL || trend_naik == 1)
		return NULL;
	return temp_data->time_str;	
}

struct kks_analog_trend_data*
kks_analog_trend_data_get_data_at_next
	(struct kks_analog_trend_data* data, double duration)
{
	struct kks_analog_trend_data* temp_data;
	char time_temp1[1024];
	char* time_temp;
	int jumlah_menit;
	if (data == NULL)
		return NULL;
	if (duration < DROPTEST_DURATION_MINIMUM || duration > DROPTEST_DURATION_MAXIMUM)
		return NULL;
	memset (time_temp1, 0, 1024);
	strcpy (time_temp1, data->time_str);
	jumlah_menit = (int) duration;
	if ((time_temp = add_minutes (time_temp1, jumlah_menit)) == NULL)
		return NULL;
	temp_data = data;
	while (temp_data != NULL)
	{
		if (!strcmp (temp_data->time_str, time_temp))
			break;
		temp_data = temp_data->next;
	}
	return temp_data;
}

struct kks_analog_trend_data* 
kks_analog_trend_consistent_decrease_chunk
	(struct kks_analog_trend_data* data, char* timestart,
	double duration)
{
	struct kks_analog_trend_data* temp_data;
	int cari_waktu_start,
		waktu_start_ketemu,
			trend_naik;
	int jumlah_menit;
	double nilai_awal;
	char time_end[1024];
	char* time_end1;

	if (timestart == NULL || data == NULL)
		return NULL;

	if (duration < DROPTEST_DURATION_MINIMUM || duration > DROPTEST_DURATION_MAXIMUM)
		return NULL;
	/* cek apakah format tanggal benar   */
	if (datetime_binary_correcting(timestart) == NULL)
		return NULL;
	temp_data = data;
	if (temp_data == NULL)
		return NULL;

	/*
	Bikin kalkulasi time end terlebih dahulu,
	baru hasil kalkulasinya digunakan untuk mencari
	time end apakah ada atau tidak
	*/
	
	memset (time_end, 0, 1024);
	strcpy (time_end, timestart);
	jumlah_menit = duration;
	if ((time_end1 = add_minutes (time_end, jumlah_menit)) == NULL)
		return NULL;
	/* Mulai mencari */
	cari_waktu_start = 1;
	waktu_start_ketemu = 0;
	trend_naik = 0;
	if (temp_data != NULL)
		nilai_awal = temp_data->val_f;

	while (temp_data != NULL)
	{
		/* Deteksi apakah ada kenaikan trend */
		if (temp_data->val_f > nilai_awal)
		{
			trend_naik = 1;
			break;
		}
		if (cari_waktu_start == 1 
			&& !strcmp (timestart, temp_data->time_str))
		{
			waktu_start_ketemu = 1;
			cari_waktu_start = 0;
		}
		if (waktu_start_ketemu == 1)
		{
			if (!strcmp (time_end1,
				temp_data->time_str))
					break;
		}
		nilai_awal = temp_data->val_f;
		temp_data = temp_data->next;
	}
	if (temp_data == NULL || trend_naik == 1)
		return NULL;
	return temp_data;	
}

struct tmline* tmline_new()
{
	struct tmline* temp;
	temp = (struct tmline*) malloc (sizeof (struct tmline));
	if (temp!= NULL)
	{
		temp->next = NULL;
		temp->prev = NULL;
		temp->index = 0;
		temp->time_str = NULL;
	}
	return temp;
}


int kks_analog_trend_print_one_file (struct kks_analog_trend_list* arg)
{
	struct kks_analog_trend* list_content;
	struct kks_analog_trend_data* data_temp;
	struct kks_analog_trend_data* data_temp1;
	struct kks_analog_trend_data* data_dummy;
	struct kks_analog_trend_data* data_temp_head;
	struct kks_analog_trend_data_wrapper* all_data_head = NULL;
	struct kks_analog_trend_data_wrapper* wrapper_chunk;
	struct kks_analog_trend_data_wrapper* wrapper_chain = NULL;
	struct kks_analog_trend_data_wrapper* wrapper_kks;
	struct kks_analog_trend_data_wrapper* wrapper_temp;
	struct kks_analog_trend_data_wrapper* wrapper_chain1;
	struct kks_analog_trend_data_wrapper* wrapper_chain2;
	struct tmline* timeline_head;
	struct tmline* timeline;
	int mulai = 0, list_mulai = 0;
	int data_ketemu = 0;
	int data_ketemu1;
	char* tabel_null = "v";
	char* isi_tabel;
	int i;
	FILE* output_file;
	char* output_fnam = "OUTPUT_ANALOG_VALUE_SEQUENCE.csv";
	char* tanggal;
	if (arg == NULL)
		return -1;
	if (arg->list_content == NULL)
		return -1;
	list_content = arg->list_content;
	while (list_content->prev != NULL)
		list_content = list_content->prev;
	timeline = arg->timeline;
		while (timeline->prev != NULL)
			timeline = timeline->prev;
	timeline_head = timeline;
	while (list_content != NULL)
	{ /* tambah kolom, prev-next (kks)  */
		timeline = timeline_head;
		while (timeline != NULL)
		{ /* tambah anggota dalam satu kolom  , upr-lwr (waktu) */
			// printf ("debug timeline print %s\n", timeline->time_str);
			wrapper_chunk = kks_analog_trend_data_wrapper_new();
			if (mulai == 1)
				kks_analog_trend_data_wrapper_join
							(wrapper_chain, wrapper_chunk);
			if (mulai == 0)
			{
				mulai = 1;
				wrapper_chain = wrapper_chunk;
			}
			data_ketemu1 = 0;			
			data_temp = list_content->data;
			while (data_temp->prev != NULL)
				data_temp = data_temp->prev;
				data_temp1 = data_temp;
				// printf ("debug data_temp1 description %s\n", data_temp1->description);
			while (data_temp != NULL)
			{
				if (!strcmp (data_temp->time_str, timeline->time_str))
				{
					data_ketemu1 = 1;
				// printf ("debug data dummy %s\n", data_temp->val);
					wrapper_chunk->data = data_temp;
					break;
				}
				data_temp = data_temp->next;
			}

			if (data_ketemu1 == 0)
			{
				// printf ("debug timeline print data = NULL\n");
				data_dummy = kks_analog_trend_data_new();
				data_dummy->time_str = timeline->time_str;
				data_dummy->kks = data_temp1->kks;
				data_dummy->description = data_temp1->description;
				data_dummy->eng_unit = data_temp1->eng_unit;
				data_dummy->val = "v";
				wrapper_chunk->data = data_dummy;
			}

			timeline = timeline->next;
		}
		if (list_mulai == 1)
			kks_analog_trend_data_wrapper_add
				(all_data_head, wrapper_chain);
		if (list_mulai == 0)
		{
			list_mulai = 1;
			all_data_head = wrapper_chain;
		}
		mulai = 0;
		list_content = list_content->next;
	}
	/* mulai mencetak... */
	if ((output_file = fopen (output_fnam, "w")) == NULL)
	{
		printf ("Error operasi file. Hentikan program.\n");
		exit (-1);
	}
	wrapper_chunk = all_data_head;
	while (wrapper_chunk->prev != NULL)
		wrapper_chunk = wrapper_chunk->prev;
	wrapper_temp = wrapper_chunk;
	wrapper_chain1 = all_data_head;
	wrapper_chain2 = wrapper_chain1;
	while (wrapper_chain2->lwr != NULL)
		wrapper_chain2 = wrapper_chain2->lwr;
	fprintf (output_file, "Analog Data Sequence;");
	/* membuat header KKS
	(untuk sumbu X adalah KKS, sumbu Y adalah waktu) */

	while (wrapper_chunk != NULL)
	{
		if (wrapper_chunk->data->kks == NULL)
			isi_tabel = tabel_null;
		else
			isi_tabel = wrapper_chunk->data->kks;
		fprintf (output_file, "%s;", isi_tabel);
		wrapper_chunk = wrapper_chunk->next;
	}
	fprintf (output_file, "\n");
	fprintf (output_file, "From: %s;", wrapper_chain1->data->time_str);
	wrapper_chunk = wrapper_temp;
	while (wrapper_chunk != NULL)
	{
		if (wrapper_chunk->data->eng_unit == NULL)
			isi_tabel = tabel_null;
		else
			isi_tabel = wrapper_chunk->data->eng_unit;
		fprintf (output_file, "%s;", isi_tabel);
		wrapper_chunk = wrapper_chunk->next;
	}
	fprintf (output_file, "\n");

	fprintf (output_file, "To: %s;", wrapper_chain2->data->time_str);
	wrapper_chunk = wrapper_temp;
	i = 0;
	while (wrapper_chunk != NULL)
	{
		if (wrapper_chunk->data->description == NULL)
			isi_tabel = tabel_null;
		else
			isi_tabel = wrapper_chunk->data->description;
		fprintf (output_file, "{%d} %s;", i, isi_tabel);
		i++;
		wrapper_chunk = wrapper_chunk->next;
	}
	fprintf (output_file, "\n");

	wrapper_chain = all_data_head;
	/*
	posisi di head.
	catat tanggal
	tulis tanggal di file dan beri tanda koma
	catat nilai
	tulis nilai di file dan beri tanda koma
	next kks
	cari apakah tanggal cocok (upr_lwr)
	jika tanggal cocok tulis nilai di file dan beri tanda koma
	jika tanggal tidak ada yang cocok maka bari tanda koma saja
	next kks... (sampai next kks NULL)
	cari apakah tanggal cocok (upr_lwr)
	jika tanggal cocok tulis nilai di file dan beri tanda koma
	jika tanggal tidak ada yang cocok maka bari tanda koma saja
	*/
	while (wrapper_chain != NULL)
	{
		wrapper_kks = all_data_head;
		tanggal = wrapper_chain->data->time_str;
		fprintf (output_file, "*%s;", tanggal);
		while (wrapper_kks != NULL)
		{
			wrapper_chunk = wrapper_kks;
			while (wrapper_chunk != NULL)
			{
				if (!strcmp(wrapper_chunk->data->time_str, tanggal))
				{
					fprintf (output_file,"%s;", wrapper_chunk->data->val);
					data_ketemu = 1;
					break;
				}
				wrapper_chunk = wrapper_chunk->lwr;
			}
			if (data_ketemu != 1)
				fprintf (output_file, ";");
			data_ketemu = 0;
			wrapper_kks = wrapper_kks->next;
		}
		fprintf(output_file, "\n");
		wrapper_chain = wrapper_chain->lwr;
	}
	// fprintf (output_file, "That's all, Folks! Reformatted csv generated using Tilmec version %s. Disclaimer: No warranty of anykind is expressed or implied.\n", TILMEC_VERSION);
	print_tilmec_version(output_file);
	if (fclose (output_file)!= 0)
	{
		printf
		("Error close file output %s, program dihentikan.\n",
			output_fnam);
		exit (-1);
	}
	return 0;
}
