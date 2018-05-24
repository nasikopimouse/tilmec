#ifndef BENCHMARK_H
#define BENCHMARK_H

#define BENCHMARK_OTHER_KKS		10
#define BENCHMARK_MS_FLOW		20
#define BENCHMARK_MS_PRES		30
#define BENCHMARK_MS_TEMP		40


struct benchmark
{
	/* header  */
	struct benchmark *prev, *next, *up, *down;
	int index;
	
	/* konten */
	
	// deretan file data
	struct benchmark_data *data;
		
};

struct benchmark_data
{
	struct benchmark_data *prev, *next, *up, *down;
	int index;
	
	// list kks untuk satu file data
	struct kks_analog_trend_list* list_data;
};

/*
Struktur data di bawah digunakan untuk memudahkan
membuat csv.

*/
struct data_wrapper
{
	struct data_wrapper *prev, *next, *up, *down;
	int index;
	// ini adalah pointer satu biji data analog
	struct kks_analog_trend* data;
};

/*

Struktur data benchmark_data akan disatukan
dengan fungsi sehingga menjadi data yang siap diubah menjadi
csv, disimpan di rangkaian struktur data data_wrapper
dengan 4 valensi.

*/


/* objek   */
struct benchmark* benchmark_new();
int benchmark_delete(struct benchmark*);

/* proses */
int benchmark_read_file(struct benchmark*, fnam);
int benchmark_process_data(struct benchmark*);
int benchmark_print(struct benchmark*);

#endif

/*
Problem:

Interval sumbu X adalah 1. Jika ada data
yang kaitannya dengan sumbu X adalah di antara
1 dan 2, di mana tempat yang pas?

Solusi:

Kita dapat menempatkannya berdasarkan kaidah
pembulatan. Akan Nilai Y akan dipasangkan dengan
nilai X pada sumbu X berdasarkan pembulatan dari
nilai X pada file data.
Misalnya, data Y adalah 9.5, dan berada di nilai
X 4.67. Maka pada grafik, angka 9.5 pada Y akan
dipasangkan dengan 5 pada sumbu X.

Problem:

Bagaimana dengan data Y yang tidak ada nilai X-nya?

Solusi:

Cell akan dikosongi, dan diserahkan kepada MS excel untuk
memproses data yang kosong. Biasanya akan diberikan pilihan
untuk trend tipe garis, apakah diteruskan ke nilai
terdekat dengan garis lurus atau interpolasi, ataukah
diberikan nilai 0.


algoritma:

- Semua file akan diproses dan disimpan di memori.
- Setiap file tidak akan dimerge.
- Setiap set data dicari sumbu X, apakah ada.
- Jika tidak ada sumbu X, maka data akan dihapus di memori.
- Jika tidak ada sumbu X sama sekali, maka program
dihentikan.
- Jika sumbu X ada di semua data, maka ditentukan
sumbu X dengan nilai terendah dan sumbu X dengan nilai
tertinggi.
- Membuat (print) trend dengan sumbu X yang sudah ditentukan
dan memasukkan semua data yang ada.
- Buat set data dengan sumbu X range terendah ke tertinggi
berdasarkan data nilai yang sudah didapat di atas, dengan
interval 1.
- Lihat data yang akan dibenchmark.

Problem:

Jika datanya adalah sumbu X sama, dengan sumbu Y yang berbeda
beda, maka bagaimana menyajikannya dalam csv?
Apakah harus dengan sumbu X yang sama? Padahal sumbu X
seharusnya tidak ada yang sama...

Solusi:

Dibuatkan linked list 4 valensi. Bentuknya akan seperti cell
pada MS excel.
Untuk chart tipe tertentu, sumbu Y untuk satu data series bisa
lebih dari satu kolom dan tentu saja lebih dari satu baris.
Sedangkan untuk sumbu X tetap satu kolom saja dengan banyak baris.
Chart dengan tipe seperti ini dapat membentuk chart dengan
sumbu X dengan nilai yang sama dengan sumbu Y yang nilainya
berbeda-beda. Misalnya, pada saat main steam pressure tertentu,
flow fuel oil berubah-ubah, representasi datanya bisa

Algoritma:




*/
