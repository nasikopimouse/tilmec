#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dir.h>
#include "fnam_handling.h"
#include "att_word_extractor.h"
#include "att_kks_analog_trend_extractor.h"
#include "att_kks_binary_event_extractor.h"

#define t3000_signature "SPPA-T3000"
#define analog_signature "Analog Grid Report"
#define binary_signature "Event Sequence Report"

int sample_main()
{
	struct ffblk analog_files;
	int i = 0;
	findfirst ("analog*.csv", &analog_files, FA_NORMAL);
	do
	{
		printf ("indeks: %d, hasil: %s\n", i, analog_files.ff_name);
		i++;
	}
	while (findnext (&analog_files) == 0 );
	findclose (&analog_files);
	return 0;
}


struct file_list* file_list_new1 ()
{
	struct file_list* temp;
	temp = (struct file_list*) malloc (sizeof (struct file_list));
	if (temp == NULL)
		return NULL;
	temp->index = 0;
	temp->prev = NULL;
	temp->next = NULL;
	temp->file_mode = -1;
	memset (temp->file_name, 0, MAXPATH);
	return temp;
}

int apakah_file_analog(char* fnam)
{
	struct word_extractor* words;
	struct content_word* contents;
	int apakah_analog;
	// printf ("debug masuk ke apakah analog\n");
	if (fnam == NULL)
		return -1;
	words = word_extractor_new();
	// printf ("debug mencari error\n");
	word_extractor_read_file(words, fnam);
	// printf ("debug mencari error1\n");
	contents = words->contents;
	apakah_analog = kks_analog_trend_apakah_analog_grid(contents);
	word_extractor_delete (words);
	// printf ("debug apakah analog selesai\n");
	return apakah_analog;
}

int apakah_file_binary(char* fnam)
{
	struct word_extractor* words;
	struct content_word* contents;
	int apakah_binary;
	// printf ("debug masuk ke apakah binary\n");
	if (fnam == NULL)
		return -1;
	words = word_extractor_new();
	word_extractor_read_file(words, fnam);
	contents = words->contents;
	if (binary_event_extractor_apakah_operation_binary(contents) == 1
		|| binary_event_extractor_apakah_event_binary(contents) == 1)
		apakah_binary = 1;
	else
		apakah_binary = 0;
	word_extractor_delete(words);
	return apakah_binary;
}

struct file_list* file_list_auto_new (int fmode)
{
	/* jangan lupa memakai compiler borland C
		dan include dir.h  */
	struct ffblk analog_files;
	struct file_list* temp;
	int file_ok;
	struct file_list* temp_head = NULL;
	char* fnam_mode = "*.csv";
	
	if (findfirst (fnam_mode, &analog_files, FA_NORMAL) != 0)
	{
		// printf ("Nama file tidak ditemukan,\n");
		return NULL;
	}
	do
	{
		if (fmode == ANALOG)
			file_ok = apakah_file_analog(analog_files.ff_name);
		else if (fmode == BINARY)
			file_ok = apakah_file_binary(analog_files.ff_name);
		else
			file_ok = 0;
		if (file_ok == 1)
		{
			temp = file_list_new1 ();
			if (temp == NULL)
			{
				findclose (&analog_files);
				return NULL;
			}
			strcpy (temp->file_name, analog_files.ff_name);
			temp->file_mode = fmode;
			if (temp_head == NULL)
				temp_head = temp;
			else
				file_list_add (temp_head, temp);
		}
		// printf
		//	("indeks: %d, hasil: %s\n", i, analog_files.ff_name);
	}
	while (findnext (&analog_files) == 0 );
	findclose (&analog_files);
	return temp_head;
}

/*
Untuk menjadikan filelisting auto, kita tinggal menjadikan
fungsi file_list_new() menjadi auto, tidak perlu
kata "analog dan "binary".

*/

struct file_list* file_list_new (int fmode)
{
	/* jangan lupa memakai compiler borland C
		dan include dir.h  */
	struct ffblk analog_files;
	struct file_list* temp;
	struct file_list* temp_head = NULL;
	char* fnam_analog = "analog";
	char* fnam_binary = "binary";
	char fnam_mode[MAXPATH];
	
	memset (fnam_mode, 0, MAXPATH);
	switch (fmode)
	{
		case ANALOG:
			strcpy (fnam_mode, fnam_analog);
			break;
		case BINARY:
			strcpy (fnam_mode, fnam_binary);
			break;
		default:
			strcpy (fnam_mode, fnam_analog);
	}
	strcat (fnam_mode, "*.csv");
	if (findfirst (fnam_mode, &analog_files, FA_NORMAL) != 0)
	{
		// printf ("Nama file tidak ditemukan,\n");
		return NULL;
	}
	do
	{
		temp = file_list_new1 ();
		if (temp == NULL)
		{
			findclose (&analog_files);
			return NULL;
		}
		strcpy (temp->file_name, analog_files.ff_name);
		temp->file_mode = fmode;
		if (temp_head == NULL)
			temp_head = temp;
		else
			file_list_add (temp_head, temp);
		// printf
		//	("indeks: %d, hasil: %s\n", i, analog_files.ff_name);
	}
	while (findnext (&analog_files) == 0 );
	findclose (&analog_files);
	return temp_head;
}

int file_list_add (struct file_list* chain, struct file_list* chunk)
{
	if (chain == NULL || chunk == NULL)
		return -1;
	while (chain->next != NULL)
		chain = chain->next;
	chunk->index = chain->index;
	chunk->index++;
	chain->next = chunk;
	chunk->prev = chain;
	return 0;
}

