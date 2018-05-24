#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "att_kks_binary_event_extractor.h"
#include "att_word_extractor.h"
#include "att_kks_analog_trend_extractor.h"
#include "att_word_extractor_main.h"
#include "tilmec_version.h"


struct binary_event_extractor*
	binary_event_extractor_new(char* fnam)
{
	struct word_extractor* words;
	struct binary_event_extractor* temp;
	if (fnam ==NULL)
		return NULL;
	temp = (struct binary_event_extractor*) malloc
			(sizeof(struct binary_event_extractor));
	if (temp!=NULL)
	{
		words = word_extractor_new();
		if(words!= NULL)
		{
			temp->contents = NULL;
			temp->source_data = words;
			temp->all_content_data = words->contents;
			temp->plant_unit = '-';
			word_extractor_read_file(temp->source_data, fnam);
			binary_event_extractor_read_data(temp);
		}		
	}
	return temp;
}

char binary_event_extractor_get_plant_unit (struct binary_event_extractor* arg)
{
	if (arg == NULL)
		return '-';
	return arg->plant_unit;
}

int binary_event_extractor_apakah_operation_binary(struct content_word* arg)
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
					FILE_SIGNATURE_BINARY_OPERATION_SEQUENCE))
				adalah_file_event = 1;
		if (adalah_t_3000 == 1 && adalah_file_event == 1)
			return 1;
	temp = temp->next;
	}
	// printf ("debug adalah file operation binary\n");
	return 0;
}

int binary_event_extractor_apakah_event_binary(struct content_word* arg)
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
					FILE_SIGNATURE_BINARY))
				adalah_file_event = 1;
		if (adalah_t_3000 == 1 && adalah_file_event == 1)
			return 1;
	temp = temp->next;
	}
	// printf ("debug adalah file event binary\n");
	return 0;
}

int binary_event_extractor_read_data_operation_sequence
	(struct binary_event_extractor* arg)
{
	struct content_word* temp;
	struct content_word* temp1;
	struct binary_event* binevt;
	int hitung;
	char* tempstr;
	int adalah_t_3000 = 0, adalah_file_event = 0, time_word_ketemu = 0;
	int kks_ditemukan = 0, unit_ditemukan = 0, normal_binary_event;
	int cari_user = 0;
	if (arg == NULL)
		return -1;	
	if (arg->source_data == NULL)
		return -1;
	if (arg->source_data->contents == NULL)
		return -1;	
	temp = arg->source_data->contents;
	while (temp->prev != NULL)
		temp = temp->prev;
	while (temp!=NULL)
	{
		if (adalah_t_3000 != 1 || adalah_file_event != 1)
		{
			if (!strcmp (temp->the_word,
					FILE_SIGNATURE_T3000))
				adalah_t_3000 = 1;
			if (!strcmp (temp->the_word,
					FILE_SIGNATURE_BINARY_OPERATION_SEQUENCE))
				adalah_file_event = 1;
			if (temp->next == NULL)
				return -1;
			temp = temp->next;
			// printf ("debug find file signature\n");
			continue;			
		}
		if (time_word_ketemu == 0)
		{
			if (!strcmp (temp->the_word, TIME_WORD))
				time_word_ketemu = 1;
			temp = temp->next;
			continue;
		}		
		if (cari_user == 0)
		{
			cari_user = 1;
			while (temp!=NULL)
			{
				if (!strcmp (temp->the_word, "User"))
				{
					if (temp->next == NULL)
						return -1;
					else
					{
						temp = temp->next;
						break;
					}
				}
				temp = temp->next;
			}
		}
		if (temp == NULL)
			return -1;
		// printf ("debug masuk operation seq %s\n", temp->the_word);
		if (apakah_datetime(temp->the_word)> 0)
		{
			// hitung = 0;
			// printf ("debug masuk if \n");
			hitung = 0;
			normal_binary_event = 1;
			temp1 = temp;
			while (temp1->next !=NULL)
			{
				if (apakah_kks(temp1->the_word)==1)
				{
					normal_binary_event = 1;
					break;
				}
				// printf ("Debug isi: %s\n", temp1->the_word);

				if (hitung > 3)
				{
					normal_binary_event = 0;
					break;
				}
				hitung++;				
				temp1 = temp1->next;
			}
			if (normal_binary_event == 0)
			{
				// printf ("debug normal samadengan nol\n");
				binevt = binary_event_new();
				binevt->seqmod = BINARY_OPERATION_SEQUENCE;
				temp1 = temp;
				tempstr = (char*) malloc (1024);
				memset (tempstr, 0, 1024);
				strcpy(tempstr, LINE_NOT_VALID);
				while (temp1->next!=NULL)
				{
					// blok ini adalah untuk data biner yang
					// formatnya tidak standar. Hanya ada satu
					// baris terus menerus
					temp1 = temp1->next;
					// if (apakah_datetime_binary (temp1->the_word) == 1)
					if (apakah_datetime(temp1->the_word) > 0)
						break;
					strcat (tempstr, ":");
					strcat (tempstr, temp1->the_word);
					// printf ("debug tempstr %s\n", tempstr);
				}
				binevt->kks = "v";
				binevt->datetime_str = temp->the_word;
				// tambahan baru
				binevt->datetime_str = datetime_binary_correcting(binevt->datetime_str);
				binevt->description = tempstr;
				binevt->val = "v";
				binary_event_add(arg, binevt);
				temp = temp1;
				continue;
			}
			// printf ("debug kks %s\n", temp1->the_word);
			if (apakah_kks(temp1->the_word)==1)
			{
				kks_ditemukan = 1;
				if (unit_ditemukan == 0)
				{
					unit_ditemukan = 1;
					arg->plant_unit = unit_manakah(temp1->the_word);
				}
				binevt = binary_event_new();
				if (binevt != NULL)
				{
					binevt->datetime_str = temp->the_word;
					// ini fungsi baru untuk mengubah ampm (jika ada) ke biasa
					binevt->datetime_str = datetime_binary_correcting(binevt->datetime_str);
					// printf ("debug datetime binary operations %s\n", binevt->datetime_str);
					binevt->seqmod = BINARY_OPERATION_SEQUENCE;
					// printf ("debug binary datetime %s\n", binevt->datetime_str);
					/* apakah kata berikutnya kks?  */
					if (apakah_kks (temp->next->the_word)!=1)
					{
						/* Jika bukan, apakah kata berikutnya karakter pertama angka?*/
						if (isdigit(temp->next->the_word[0])==0)
						{
							/* Jika bukan, maka kata itu adalah event type   */
							binevt->evt_type = temp->next->the_word;
						}
					}
					while (temp->next !=NULL)
					{
						if (apakah_kks (temp->the_word)==1)
						{
							// printf ("debug seek\n");
							binevt->kks = temp->the_word;
							binevt->evt_type = "OE";
							/* apakah kata sebelum KKS huruf pertama adalah digit   */
							if (isdigit(temp->prev->the_word[0])==1)
							{
								/* Jika bukan KKS berarti prio     */
								if (apakah_kks(temp->prev->the_word)!=1)
								{
									binevt->prio = temp->the_word;
								}
							}
							break;
						}
						// printf ("debug seek prio\n");
						temp = temp->next;
					}
					temp = temp->next;
					binevt->description = temp->the_word;
					temp = temp->next;
					binevt->val = temp->the_word;
					temp = temp->next;
					if (temp->next == NULL)
						binevt->usern = temp->the_word;
					else if(temp->next!=NULL)
					{
						if (apakah_datetime(temp->next->the_word) > 0)
						{
							binevt->usern = temp->the_word;
						}
						else
						{
							binevt->eng_unit = temp->the_word;
							binevt->usern = temp->next->the_word;
						}
					}					
				}				
				binary_event_add (arg, binevt);
			}
		}
		while (temp!=NULL)
		{
			if (apakah_datetime(temp->the_word) > 0)
				break;
			temp = temp ->next;
		}		
	}
	if (adalah_t_3000 != 1 || adalah_file_event != 1 || kks_ditemukan != 1)
	{
		// printf ("debug operation event canceled\n");
		printf ("File event sequence harap dicek lagi. Program dihentikan.\n");
		return -1;
	}
	return 0;
}



int binary_event_extractor_read_data
	(struct binary_event_extractor* arg)
{
	struct content_word* temp;
	struct content_word* temp1;
	struct binary_event* binevt;
	int hitung;
	char* tempstr;
	int adalah_t_3000 = 0, adalah_file_event = 0, normal_binary_event;
	int kks_ditemukan = 0, unit_ditemukan = 0, time_word_ketemu = 0;
	if (binary_event_extractor_read_data_operation_sequence(arg) == 0)
		return 0;
	// printf ("Debug not operation sequence\n");
	if (arg == NULL)
		return -1;	
	if (arg->source_data == NULL)
		return -1;
	if (arg->source_data->contents == NULL)
		return -1;	
	temp = arg->source_data->contents;
	while (temp->prev != NULL)
		temp = temp->prev;
	while (temp!=NULL)
	{
		if (adalah_t_3000 != 1 || adalah_file_event != 1)
		{
			if (!strcmp (temp->the_word,
					FILE_SIGNATURE_T3000))
				adalah_t_3000 = 1;
			if (!strcmp (temp->the_word,
					FILE_SIGNATURE_BINARY))
				adalah_file_event = 1;
			if (temp->next == NULL)
				return -1;
			temp = temp->next;
			continue;			
		}
		// cari kata-kata "time"
		if (time_word_ketemu == 0)
		{
			if (!strcmp (temp->the_word, TIME_WORD))
				time_word_ketemu = 1;
			temp = temp->next;
			continue;
		}
		// if (apakah_datetime_binary(temp->the_word)==1)
		if (apakah_datetime(temp->the_word)>0)
		{
			hitung = 0;
			normal_binary_event = 1;
			temp1 = temp;
			while (temp1->next !=NULL)
			{
				if (apakah_kks(temp1->the_word)==1)
				{
					normal_binary_event = 1;
					break;
				}
				// printf ("Debug isi: %s\n", temp1->the_word);

				if (hitung > 3)
				{
					normal_binary_event = 0;
					break;
				}
				hitung++;				
				temp1 = temp1->next;
			}
			if (normal_binary_event == 0)
			{
				// printf ("debug normal samadengan nol\n");
				binevt = binary_event_new();
				binevt->seqmod = BINARY_EVENT_SEQUENCE;
				temp1 = temp;
				tempstr = (char*) malloc (1024);
				memset (tempstr, 0, 1024);
				strcpy(tempstr, LINE_NOT_VALID);
				while (temp1->next!=NULL)
				{
					temp1 = temp1->next;
					// if (apakah_datetime_binary(temp1->the_word) == 1)
					if (apakah_datetime(temp1->the_word) > 0)
						break;
					strcat (tempstr, ":");
					strcat (tempstr, temp1->the_word);
					// printf ("debug tempstr %s\n", tempstr);
				}
				binevt->kks = "v";
				// binevt->datetime_str = temp->the_word+1;
				binevt->datetime_str = temp->the_word;
				// ini adalah fungsi baru untuk mengubah tanggal ke standar
				binevt->datetime_str = datetime_binary_correcting(binevt->datetime_str);
				// printf ("debug datetime binary biasa %s\n", binevt->datetime_str);
				binevt->description = tempstr;
				binevt->val = "v";
				binary_event_add(arg, binevt);
				temp = temp1;
				continue;
			}
			// printf ("debug kks %s\n", temp1->the_word);
			if (apakah_kks(temp1->the_word)==1)
			{
				if (apakah_kks(temp1->the_word)==1)
					kks_ditemukan = 1;
				if (unit_ditemukan == 0)
				{
					unit_ditemukan = 1;
					arg->plant_unit = unit_manakah(temp1->the_word);
				}
				binevt = binary_event_new();
				binevt->seqmod = BINARY_EVENT_SEQUENCE;
				if (binevt != NULL)
				{
					binevt->datetime_str = temp->the_word;
					binevt->datetime_str = datetime_binary_correcting (binevt->datetime_str);
					// printf ("debug binary datetime %s\n", binevt->datetime_str);
					/* apakah kata berikutnya kks?  */
					
					if (apakah_kks (temp->next->the_word)!=1)
					{
						/* Jika bukan, apakah kata berikutnya karakter pertama angka?*/
						if (isdigit(temp->next->the_word[0])==0)
						{
							/* Jika bukan, maka kata itu adalah event type   */
							binevt->evt_type = temp->next->the_word;
						}
					}
					while (temp->next !=NULL)
					{
						if (apakah_kks (temp->the_word)==1)
						{
							// printf ("debug seek\n");
							binevt->kks = temp->the_word;
							/* apakah kata sebelum KKS huruf pertama adalah digit   */
							if (isdigit(temp->prev->the_word[0])==1)
							{
								/* Jika bukan KKS berarti prio     */
								if (apakah_kks(temp->prev->the_word)!=1)
								{
									binevt->prio = temp->the_word;
								}
							}
							break;
						}
						temp = temp->next;
					}
					temp = temp->next;
					binevt->description = temp->the_word;
					temp = temp->next;
					binevt->val = temp->the_word;
					while (temp!=NULL)
					{
						if (!strcmp (temp->the_word, "GOD") ||
							!strcmp (temp->the_word, "BAD") ||
							!strcmp (temp->the_word, "UNC")
						)
						{
							binevt->val_qlty = temp->the_word;
							break;
						}
						// if (apakah_datetime_binary(temp->the_word) == 1)
						if (apakah_datetime(temp->the_word) > 0)
							break;
						temp = temp ->next;
					}
				}
				binary_event_add (arg, binevt);
			}
		}
		while (temp!=NULL)
		{
			// if (apakah_datetime_binary(temp->the_word) == 1)
			if (apakah_datetime(temp->the_word) > 0)
				break;
			temp = temp ->next;
		}
	}
	if (adalah_t_3000 != 1 || adalah_file_event != 1 || kks_ditemukan != 1)
	{
		// printf ("debug canceled\n");
		printf ("File event sequence harap dicek lagi. Program dihentikan.\n");
		exit (-1);
	}
	return 0;
}

struct binary_event* binary_event_new()
{
	struct binary_event* temp;
	
	temp = (struct binary_event*) malloc
			(sizeof (struct binary_event));
	if (temp!= NULL)
	{
		temp->index = 0;
		temp->prev = NULL;
		temp->next = NULL;
		temp->datetime_str = NULL;
		temp->kks = NULL;
		temp->evt_type = "";
		temp->prio = "";
		temp->description = NULL;
		temp->val = NULL;
		temp->usern = NULL;
		temp->eng_unit = NULL;
		temp->val_qlty = NULL;
		temp->seqmod = -1;
	}
	return temp;
}

int binary_event_add (struct binary_event_extractor* chain,
		struct binary_event* chunk)
{
	if (chunk == NULL)
		return -1;
	if (chain->contents == NULL)
		chain->contents = chunk;
	else
	{
		while (chain->contents->next != NULL)
			chain->contents = chain->contents->next;
		chunk->index = chain->contents->index;
		chunk->index++;
		chain->contents->next = chunk;
		chunk->prev = chain->contents;
		chunk->upr = chain;
	}
	return 0;
}

int binary_event_extractor_print
	(struct binary_event_extractor* arg)
{
	int spasi, i;
	struct binary_event* temp;
	if (arg == NULL)
		return -1;
	temp = arg->contents;
	while (temp->prev != NULL)
		temp = temp->prev;
	while (temp!=NULL)
	{
		spasi = 57-(strlen(temp->kks)
						+strlen(temp->description)
						+strlen(temp->val));
		printf ("%s %s -> %s ", temp->datetime_str,
					temp->kks, temp->description);
		for (i=0;i<spasi; i++)
			printf ("-");
		printf ("  %s\n", temp->val);
		temp = temp->next;
	}
	return 0;	
}

int binary_event_extractor_print_file_formatted
		(struct binary_event_extractor* arg)
{
	FILE* file_output;
	char* fnam = "BINARY_EVENT_SEQUENCE.txt";
	int spasi, i;
	struct binary_event* temp;
	if (arg == NULL)
		return -1;
	file_output = fopen (fnam, "w");
	if (file_output == NULL)
	{
		printf ("Error operasi file. Program dihentikan!\n");
		exit(1);
	}
	temp = arg->contents;
	while (temp->prev != NULL)
		temp = temp->prev;
	while (temp!=NULL)
	{
		spasi = 57-(strlen(temp->kks)
					+strlen(temp->description)
					+strlen(temp->val));
		fprintf (file_output, "%s %s -> %s ", 
			temp->datetime_str, temp->kks, temp->description, temp->val);
		for (i=0;i<spasi; i++)
			fprintf (file_output, "-");
		fprintf (file_output,"  %s\n", temp->val);
		temp = temp->next;
	}
	if (fclose (file_output) != 0)
	{
		printf ("Error close file. Program dihentikan.\n");
		exit (-1);
	}
	return 0;	
}


int binary_event_extractor_print_file
		(struct binary_event_extractor* arg)
{
	FILE* file_output;
	char* fnam = "OUTPUT_BINARY_EVENT_SEQUENCE.csv";
	int i;
	struct binary_event* temp;
	struct binary_event* temp_head;
	char* tanggal_mulai;
	char* tanggal_selesai;
	char evt_type[1024];
	if (arg == NULL)
		return -1;
	file_output = fopen (fnam, "w");
	if (file_output == NULL)
	{
		printf ("Error operasi file. Program dihentikan!\n");
		exit(1);
	}
	temp = arg->contents;
	while (temp->prev != NULL)
		temp = temp->prev;
	temp_head = temp;
	tanggal_mulai = temp_head->datetime_str;
	while (temp->next!=NULL)
		temp = temp->next;
	tanggal_selesai = temp->datetime_str;
	fprintf (file_output, "Binary Event Sequence;\n");
	fprintf (file_output, "From;:;*%s;\n", tanggal_mulai);
	fprintf (file_output, "To;:;*%s\n", tanggal_selesai);
	fprintf (file_output, "DateTime;Type;KKS;Description;Value;Qlty;\n");
	temp = temp_head;
	i = 0;
	while (temp!=NULL)
	{
		if (temp->usern!=NULL)
		{
			memset (evt_type, 0, 1024);
			strcpy (evt_type, temp->evt_type);
			strcat (evt_type, "/");
			strcat (evt_type, temp->usern);
		}
		else
		{
			memset (evt_type, 0, 1024);
			strcpy (evt_type, temp->evt_type);			
		}

		if (temp->val_qlty!=NULL)
			fprintf (file_output, "*%s;%s;%s;{%d} %s;%s;%s\n", 
				temp->datetime_str, evt_type, temp->kks, i,
					temp->description, temp->val, temp->val_qlty);
		else
			fprintf (file_output, "*%s;%s;%s;{%d} %s;%s\n", 
				temp->datetime_str, evt_type, temp->kks, i,
					temp->description, temp->val);
		i++;
		temp = temp->next;
	}
		// fprintf (file_output, "That's all, Folks! Reformatted csv generated using Tilmec version %s. Disclaimer: No warranty of anykind is expressed or implied.\n", TILMEC_VERSION);
		print_tilmec_version(file_output);
	if (fclose (file_output) != 0)
	{
		printf ("Error close file. Program dihentikan.\n");
		exit (-1);
	}
	return i;	
}
/*
Memberitahukan waktu trend pada satu chunk data trend
binary.
*/
char* binary_event_get_time (struct binary_event* data)
{
	char* temp;
	if (data == NULL)
		return NULL;
	temp = (char*) malloc (1024);
	memset (temp, '\0', 1024);
	strcpy (temp, data->datetime_str);
	temp = datetime_binary_correcting (temp);
	return temp;
}

struct kks_occurence* kks_occurence_new()
{
	struct kks_occurence* temp;
	temp = (struct kks_occurence*) malloc (sizeof (struct kks_occurence));
	if (temp == NULL)
		return NULL;
	temp->next = NULL;
	temp->prev = NULL;
	temp->time_occurence = (char*) malloc (1024);
	temp->index = 0;
	return temp;
}

int kks_occurence_add 
	(struct kks_event_search* chain, struct kks_occurence* chunk)
{
	if (chunk == NULL)
		return -1;
	if (chain->occurence == NULL)
		chain->occurence = chunk;
	else
	{
		while (chain->occurence->next != NULL)
			chain->occurence = chain->occurence->next;
		chunk->index = chain->occurence->index;
		chunk->index++;
		chain->occurence->next = chunk;
		chunk->prev = chain->occurence;
		chunk->upr = chain;
	}
	return 0;	
}

/*
Untuk mencari kks pada trend binary, mulailah
dengan menginisialisasi mesinnya di sini.
*/
struct kks_event_search*
	kks_event_search_new
	(struct binary_event_extractor* data, char* kks, char* signal)
{
	struct kks_event_search* temp;
	if (kks== NULL
		|| signal == NULL
		|| data == NULL)
		return NULL;
	if (apakah_kks(kks)!= 1)
		return NULL;
	temp = (struct kks_event_search*)
		malloc (sizeof (struct kks_event_search));
	if (temp == NULL)
		return NULL;
	temp->kks = (char*) malloc (1024);
	memset (temp->kks, 0, 1024);
	temp->signal = (char*) malloc (1024);
	memset (temp->signal, 0, 1024);
	strcpy (temp->kks, kks);
	strcpy (temp->signal, signal);
	temp->binary_data = data;
	temp->total_occurence = -1;
	temp->occurence = NULL;
	return temp;
}

/*
Ternyata sudah saya bikin. Fungsi ini digunakan
untuk mencari kks tertentu pada data binary yang sudah
diparsing. Jika ada, maka nilai return akan lebih
dari nol. Jika tidak ada, maka nilai return akan nol.
Hasil search akan disimpan struktur dengan tipe
kks_occurence. Untuk menggunakan mesin ini,
buat objek baru, dan run fungsi search. Hanya dua step
saja.
*/
int kks_event_search_event (struct kks_event_search* arg)
{
	struct binary_event* data;
	struct kks_occurence* kks_chunk;
	int occurence = 0;
	if (arg == NULL)
		return -1;
	data = arg->binary_data->contents;
	if (data == NULL)
		return -1;
	while (data->prev != NULL)
		data = data->prev;
	while (data != NULL)
	{
		if (!strcmp (arg->kks, data->kks)
			&& !strcmp (arg->signal, data->val))
		{
			occurence++;
			kks_chunk = kks_occurence_new();
			strcpy (kks_chunk->time_occurence, data->datetime_str);
			kks_occurence_add (arg, kks_chunk);
		}
		data = data->next;
	}
	arg->total_occurence = occurence;
	return occurence;
}

/*
Ini sepertinya untuk mencari waktu trend
pada kali keberapa kks muncul. Misalnya
kks A muncul 5 kali, maka fungsi ini akan
mengembalikan waktu trend pada kemunculan
ketiga untuk kks A.
*/
char* kks_event_search_get_time_occurence
	(struct kks_event_search* arg, int t_occ)
{
	struct kks_occurence* data;
	char* time_temp = NULL;
	if (arg == NULL)
		return NULL;
	data = arg->occurence;
	if (data == NULL)
		return NULL;
	while (data->prev != NULL)
		data = data->prev;
	if (t_occ < 0 || t_occ > arg->total_occurence)
		return NULL;
	while (data != NULL)
	{
		if (data->index == t_occ)
		{
			time_temp = (char*) malloc (1024);
			if (time_temp != NULL)
			{
				memset (time_temp, 0, 1024);
				strcpy (time_temp, data->time_occurence);
			}
		}
		data = data->next;
	}
	return time_temp;
}

/*
Fungsi ini hanya akan memberitahukan total
kemunculan untuk kks tertentu.
*/
int kks_event_search_get_total_event_occurence
	(struct kks_event_search* arg)
{
	if (arg == NULL)
		return -1;
	return arg->total_occurence;
}

int apakah_binary_chunk_sama(struct binary_event* bin1, struct binary_event* bin2)
{
	if (bin1 == NULL || bin2 == NULL)
		return -1;
	if (!strcmp (bin1->datetime_str, bin2->datetime_str) &&
			!strcmp (bin1->kks, bin2->kks) &&
			!strcmp (bin1->description, bin2->description) &&
			!strcmp (bin1->val, bin2->val))
			{
				if (bin1->usern!=NULL && bin2->usern!=NULL)
				{
					if (!strcmp(bin1->usern,bin2->usern))
						return 1;
					else
						return 0;
				}
				if (bin1->val_qlty!=NULL && bin2->val_qlty!=NULL)
				{
					if (!strcmp(bin1->val_qlty, bin2->val_qlty))
						return 1;
					else
						return 0;
				}
			}
	return 0;
}
