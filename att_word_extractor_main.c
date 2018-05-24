#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "att_word_extractor_main.h"
#include "droptest.h"
#include "cmd_param.h"
#include "att.h"
#include "excel.h"
#include "multi_file.h"
#include "tilmec_version.h"

/*
Dikarenakan saat ini kegunaan fungsi main() adalah
untuk merunning engine att, maka ke depannya
kode di main() ini harus dipindahkan ke att.c.
Untuk header dan preprocessor harus dipindahkan ke
att.h.

Apa kegunaan fungsi main()?
Fungsi ini digunakan hanya untuk memilih
untuk aplikasi apa program ini dirunning.
Pada saat ini, ada tiga aplikasi, yaitu
aplikasi ATT, aplikasi untuk membuat
trend bisa dipindahkan dengan mudah ke excel,
dan aplikasi untuk droptest.

Ke depannya, file data tidak harus berupa file
yang harus diubah dahulu ke file dengan nama
analog.csv atau binary.csv. Program akan
langsung melihat file yang ada di direktori
yang sama dengan program, berdasarkan file yang
ketemu, maka akan dipilih aplikasi yang sesuai.
File akan dilihat data di dalamnya, dan akan
ditentukan aplikasi apa yang akan digunakan
berdasarkan KKS dan kemunculan, tulisan pada
header, dan sebagainya.

Seleksi untuk aplikasi yang harus dipilih
adalah dengan parameter. Akan dibuatkan loop
linked list untuk menganalisa parameter apa
yang ada. Targetnya adalah, penulisan
parameter dibuat semudah mungkin, tanpa
harus mengurutkan, tanpa harus mendahulukan,
dan sabagainya.

Ingat, pemilihan unit untuk list kks sudah dibuat
secara otomatis. Tempatnya mungkin di droptest.c.

****

Pemindahan sudah dilakukan.
Berikutnya adalah:
menambahkan fungsi print dan
	fungsi eksekusi pada modul droptest.
Testing modul droptest.
menambahkan fitur seleksi berdasarkan parameter
	pada fungsi main.
Mengintegrasikan semua sehingga program
bisa siap pakai.

fitur baru, pada setiap list analog dan list
digital ada properti untuk plant unit.
Properti ini mestinya digunakan pada modul
fungsi tertentu, dengan file analog dan file
biner. Jika ada perbedaan unit, maka
program akan dihentikan.

Untuk mempersiapkan program yang bisa menanngani
file dalam jumlah banyak, maka diperlukan
manajemen pemakaian memori agar efisien dalam
hal pemakaian dan kecepatan. Berarti mengoptimisasi
kode untuk analog dan binary extractor.
Caranya adalah
- meminimalisasi pemakaian malloc
- memperkecil panjang space untuk variabel string sementara
- data yang sebenarnya ada di word extractor. Untuk
analog dan binary list dan linked list, hanya berisi
pointer ke word extractor.
- Untuk melindungi data dari modifikasi, jika kita ingin
mengambil data untuk diubah, maka kita harus membuat
alokasi memori baru dan MENYALIN data itu. Jangan mengubah
data dari word extractor.

*/

#include "att_word_extractor.h"
#include "att_kks_analog_trend_extractor.h"
#include "att_kks_binary_event_extractor.h"
#include "dcs_time.h"
#include "att.h"
#include "att_word_extractor_main.h"
#include "cmd_param.h"

char* analog_fnam = "analog.csv";
char* binary_fnam = "binary.csv";

int apakah_file_ada (char*);
int old_style_print (int, char**);
int display_help (void);
int display_header(void);
int display_working(void);
int display_done(void);
int display_program_name(void);
int display_keterangan(void);
int under_development();



int main(int argc, char** argv)
{
	int i, j;
	int excel_status = -1;
	int sudah_dilakukan = 0;
	FILE* file_att_result;
	FILE* file_att_form;
	struct cmd_param* params;
	double waktu1, waktu2;
	waktu1 = clock();
	/* Kode utama ada di sini...   */
	if (argc <=1 )
	{
		display_program_name();
		exit (-1);
	}
	else
	{
		params = cmd_param_new (argc, argv);
	}
	if (cmd_param_search (params, "-att") > 0)
	{
		display_working();		
		att_main(analog_fnam, binary_fnam);
		sudah_dilakukan = 1;
		goto selesai;
		/* panggil fungsi att     */
	}
	else if (cmd_param_search (params, "-fwdroptest") > 0)
	{
		/* panggil fungsi feedwater droptest    */
		display_working();
		droptest_main(analog_fnam, 60);
		sudah_dilakukan = 1;
		goto selesai;
	}	
	else if (cmd_param_search (params, "-fwdroptest-half") > 0)
	{
		/* panggil fungsi feedwater droptest    */
		display_working();
		droptest_main(analog_fnam, 30);
		sudah_dilakukan = 1;
		goto selesai;
	}

	else if (cmd_param_search (params, "-excel-benchmark") > 0)
	{
		/* panggil fungsi benchmark dengan kks sembarang */
		display_working();
		under_development();
		// benchmark_main(analog_fnam, BENCHMARK_OTHER_KKS, benchmark_get_kks());
		sudah_dilakukan = 1;
		goto selesai;
	}

	else if (cmd_param_search (params, "-excel-benchmark-msflow") > 0)
	{
		/* panggil fungsi benchmark dengan kks sembarang */
		display_working();
		under_development();
		// benchmark_main(analog_fnam, BENCHMARK_MS_FLOW, NULL);
		sudah_dilakukan = 1;
		goto selesai;
	}

	else if (cmd_param_search (params, "-excel-benchmark-mspressure") > 0)
	{
		/* panggil fungsi benchmark dengan kks sembarang */
		display_working();
		under_development();
		// benchmark_main(analog_fnam, BENCHMARK_MS_PRES, NULL);
		sudah_dilakukan = 1;
		goto selesai;
	}

	else if (cmd_param_search (params, "-excel-benchmark-mstemperature") > 0)
	{
		/* panggil fungsi benchmark dengan kks sembarang */
		display_working();
		under_development();
		// benchmark_main(analog_fnam, BENCHMARK_MS_TEMP, NULL);
		sudah_dilakukan = 1;
		goto selesai;
	}
	
	else if (cmd_param_search (params, "-excel" ) > 0)
	{
		display_working();
		excel_status = output_excel (analog_fnam, binary_fnam, NO_INDIVIDUAL_FILE);
		sudah_dilakukan = 1;
		goto selesai;
		/* panggil fungsi untuk mengurai trend menjadi format csv */
	}
	else if (cmd_param_search (params, "-excel-individual" ) > 0)
	{
		display_working();		
		excel_status = output_excel (analog_fnam, binary_fnam, INDIVIDUAL_FILE);
		sudah_dilakukan = 1;
		goto selesai;
		/* panggil fungsi untuk mengurai trend menjadi format csv */
	}
	else if (cmd_param_search (params, "-excel-merge" ) > 0)
	{
		display_working();		
		excel_merge();
		sudah_dilakukan = 1;
		goto selesai;
		/* panggil fungsi untuk mengurai trend menjadi format csv */
	}

	else if (cmd_param_search (params, "-excel-startup" ) > 0)
	{
		display_working();
		susd();
		sudah_dilakukan = 1;
		goto selesai;
		/* panggil fungsi untuk mengurai trend menjadi format csv */
	}
	else if (cmd_param_search (params, "-excel-startup-mobile" ) > 0)
	{
		display_working();
		susd_mobile();
		sudah_dilakukan = 1;
		goto selesai;
		/* panggil fungsi untuk mengurai trend menjadi format csv */
	}
	
	else if (cmd_param_search (params, "-help" ) > 0)
	{
		display_header();
		display_help();
		sudah_dilakukan = 1;
		goto selesai;
		/* panggil fungsi untuk mengurai trend menjadi format csv */
	}	
	else
	{
		display_program_name();
		exit (-1);
	}
	selesai:
	if (sudah_dilakukan == 1)
	{
		switch (excel_status)
		{
			case FILE_ANALOG_TIDAK_VALID:
				printf ("File analog tidak ada.\n");
				break;
			case FILE_BINARY_TIDAK_VALID:
				printf ("File binary tidak ada.\n");
				break;
			case FILE_ANALOG_BINARY_TIDAK_VALID:
				printf ("Tidak ada file yang bisa diproses.\n");
				break;
			case SUKSES:
				printf ("Kedua file berhasil diproses.\n");
				break;
			default:
				printf ("\n--");
		}
		display_done();
		waktu2 = clock();
		printf ("(Waktu: %4.3f detik)\n\n",
			(waktu2 - waktu1)/(double) CLOCKS_PER_SEC);
	}
	return 0;
}

int display_header()
{
	printf ("\nTILMEC version %s\n", TILMEC_VERSION);
	printf ("inTegrated utILities for Main-plant Engineers and unit-Controllers\n");
	printf ("Prepared by: Ibrahim Mardiantoro\n");
	printf ("Bug reports: ibrahim.mardiantoro@outlook.com\n\n");
	printf ("Disclaimer:\n");
	printf ("No warranty of any kind is expressed or implied.\n\n");

	return 0;
}


int display_program_name()
{
	display_header();
	display_keterangan();
	return 0;
}

int display_keterangan()
{
	printf ("Ketik <nama_program> -help [enter] untuk keterangan.\n\n");
	return 0;
}

int display_working()
{
	display_header();
	printf ("Sedang mengerjakan...\n");
	return 0;
}

int display_done()
{
	printf ("Selesai. ");
	return 0;
}

int display_help()
{
	printf ("Parameter:\n");
	printf ("-att                  memproses data ATT T3000.\n");
	printf ("-fwdroptest           memproses data FW Droptest T3000.\n");
	printf ("-fwdroptest-half      memproses data 30min FW Droptest T3000.\n");
	printf ("-excel                data T3000 csv ke Excel csv.\n");
	printf ("-excel-individual     satu KKS satu file.\n");
	printf ("-excel-merge          banyak file, analog digabung, binary digabung.\n");
	printf ("-excel-startup        banyak file, analog binary jadi satu file.\n");
	printf ("-excel-startup-mobile -excel-startup untuk Excel-Android.\n\n");
	printf ("Penggunaan:\n");
	printf (" 0. Pastikan file data yang akan diproses berformat csv,\n");
	printf ("    isi file tidak berubah, dan data terbaca semua oleh DCS,\n");	
	printf ("    tidak ada data kosong atau item KKS kosong.\n");	
	printf (" 1. Letakkan file data analog dan file event di folder lokasi program ini.\n");
	printf (" 2. Ubah nama file data analog grid report 1s menjadi analog.csv.\n");
	printf ("    Tidak perlu untuk -excel-merge dan -excel-startup\n");
	printf (" 3. Ubah nama file data event sequence report menjadi binary.csv.\n");
	printf ("    Tidak perlu untuk -excel-merge dan -excel-startup\n");
	printf (" 4. Untuk -excel-merge dan -excel-startup, bisa dengan banyak file data.\n");
	printf ("    -excel-startup file analog disarankan interval 1min, untuk\n");
	printf ("    memudahkan menghitung fuel oil totaliser.\n");
	printf (" 5. Eksekusi program: <nama_program> <parameter>. Cek hasil\n");
	printf ("    di folder lokasi program ini.\n");
	printf (" 6. Untuk menampilkan help, ketik <nama_program> -help\n");
	printf (" 7. Biasanya nama programnya tilmec.exe.\n");
	printf (" 8. Program ini untuk -excel-startup akan mencari file autonotes.csv\n");
	printf ("    untuk mencatat otomatis di kolom notes.\n");
	printf ("    Jika tidak ada maka akan membuat sendiri.\n");
	printf ("    File ini dapat diedit oleh user sesuai keperluan.\n");
	printf (" 9. Mobile, konversi ke Excel-Android, gunakan:\n");
	printf ("    PC: LibreOffice: Office Open XML Spreadsheet format.\n");
	printf ("    Android: MS-Excel: Save a copy.\n");
	printf ("10. -excel-startup, Freeze pane, PC: G4 Mobile: E4. Atur lebar optimal.\n");
	printf ("11. Data analog startup/shutdown:\n");
	printf ("    a. Report: analog grid sequence\n");
	printf ("    b. Interval: 1 menit\n");
	printf ("    c. KKS list setup di project:reports:groups\n");
	printf ("12. Data binary startup/shutdown:\n");
	printf ("    a. Report: event sequence\n");
	printf ("    b. Selection: HA HB HF HJ HL HY\n");
	printf ("    c. Selection: MAY MYA01-DE MYA01-DG MYA01-DP\n");
	printf ("    d. Selection: LA LB B ABA CJ 00B\n");
	printf ("13. Selection untuk data ATT:\n");
	printf ("    Selection untuk data binary event sequence ATT:\n");
	printf ("    ? 0MA??? FG?51||Q1, ? 0MA??? FG?51||Q2, ? 0MA??? AA001||XT10,\n");
	printf ("    ? 0MAY01 EC200||XS01, ? 0MAY01 EC200||XS56, ? 0MA??? AA01?||XB??.\n");
	printf ("    Selection untuk data analog value sequence ATT:\n");
	printf ("    ? 0MKA01 CE901||XQ01, ? 0HJA00 EY001||XQ09, ? 0HJA00 EY001||XV54,\n");
	printf ("    ? 0LBA30 CT001||XQ01, ? 0LBA30 CP901||XQ01, ? 0LBA30 CF901||XQ01,\n");
	printf ("    ? 0LBB11 CT903||XQ01, ? 0LBB12 CT903||XQ01, ? 0MAA12 FG151||XQ01,\n");
	printf ("    ? 0MAA22 FG151||XQ01, ? 0MAB12 FG151||XQ01, ? 0MAB22 FG151||XQ01,\n");
	printf ("    ? 0MKA01 CE005||XQ01, ? 0CJA04 DU001||XQ03.\n");
	printf ("14. Selection untuk data analog Feedwater Drop Test:\n");
	printf ("    ? 0MKA01 CE901||XQ01, ? 0HAD10 FL901||XQ01, ? 0LAA10 CT001||XQ01,\n");
	printf ("    ? 0LAA10 FL901||XQ01, ? 0MAG20 CL901||XQ02.\n");

	return 0;
}

int apakah_file_ada (char* fnam)
{
	FILE* filep;
	if ((filep = fopen (fnam, "r")) != NULL)
	{
		fclose (filep);
		return 1;
	}
	return 0;
}

int under_development()
{
	printf ("\nFitur ini sedang dalam tahap pengembangan.\n");
	return 0;
}
