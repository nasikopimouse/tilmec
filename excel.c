#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "excel.h"

int output_excel (char* analog_fnam, char* binary_fnam, int analog_output_mode)
{
	struct word_extractor* word_extractor_obj;
	struct kks_analog_trend_list* kks_list;
	struct kks_analog_trend* kks_analog;
	struct binary_event_extractor* binevt;
	FILE* file_all;
	int status = 0;
	if ((file_all = fopen (analog_fnam, "r"))!=NULL)
	{
		fclose (file_all);
		/*  step 1  */
		word_extractor_obj = word_extractor_new ();
		/*  step 2  */
		word_extractor_read_file(word_extractor_obj, analog_fnam);
		/*  step 3  */
		kks_list = kks_analog_trend_list_new (word_extractor_obj);
		/*  step 4  */
		if (analog_output_mode == INDIVIDUAL_FILE)
		{
			kks_analog = kks_list->list_content;
			while (kks_analog->prev!=NULL)
				kks_analog = kks_analog->prev;
			while (kks_analog!=NULL)
			{
				kks_analog_trend_print_file (kks_analog);
				kks_analog = kks_analog->next;
			}
		}
		kks_analog_trend_print_one_file(kks_list);
	}
	else
		status += 1;
	if ((file_all = fopen (binary_fnam, "r"))!=NULL)
	{
		fclose (file_all);
		/* step 1  */
		binevt = binary_event_extractor_new(binary_fnam);
		/* step 2  */
		binary_event_extractor_print_file(binevt);
	}
	else
		status += 3;
	/*
	Jika status 0 sukses.
	Jika status 1 file analog tidak valid.
	Jika status 3 file binary tidak valid.
	Jika status 4 kedua file tidak valid.
	*/
	return status;
}
