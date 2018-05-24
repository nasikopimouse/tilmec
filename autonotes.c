#include "autonotes.h"


struct autonote* autonote_new()
{
	struct autonote* temp;
	temp = (struct autonote*) malloc (sizeof(struct autonote));
	if (temp == NULL)
		return NULL;
	temp->index = 0;
	temp->prev = NULL;
	temp->next = NULL;
	temp->kks = NULL;
	temp->description = NULL;
	temp->val = NULL;
	temp->data = NULL;
	return temp;
}

struct autonote* autonote_read_file()
{
	struct word_extractor* word_extr;
	struct autonote* note;
	struct autonote* note_head = NULL;
	struct content_word* words;
	struct content_word* words_head;
	int read_feedb;
	char* filename = FNAM;
	word_extr = word_extractor_new();
	read_feedb = word_extractor_read_file_with_comment (word_extr, filename, '#');
	/*
	jika file tidak ada, maka dibuat file contoh baru.
	Jika file ada, maka data dibaca.
	
	*/
	if (read_feedb!=0)
	{
		if (read_feedb == -2)
		{
			if (autonote_print_example_file()==0)
				word_extractor_read_file_with_comment (word_extr, filename, '#');
		}
		else
			return NULL;
	}
	words = word_extr->contents;
	while (words->prev !=NULL)
		words = words->prev;
	words_head = words;
	while (words!=NULL)
	{
		if (apakah_kks(words->the_word)==1)
		{
			note = autonote_new();
			if (note_head == NULL)
				note_head = note;
			else
			{
				while (note->next!=NULL)
					note_head = note_head->next;
				note->index = note_head->index;
				note->index++;
				note_head->next = note;
				note->prev = note_head;
				note_head = note;
			}
			note->kks = words->the_word;
			note->data = words_head;
			if (words->next!=NULL)
				words = words->next;
			else
				continue;
			if (apakah_kks(words->the_word) == 1)
				continue;
			note->val = words->the_word;
			if (words->next!=NULL)
				words = words->next;
			else
				continue;
			if (apakah_kks(words->the_word) == 1)
				continue;
			note->description = words->the_word;
		}	
		words = words->next;
	}
	/* word extractor tidak bisa mendeteksi adanya comment.
		Kecuali kita buat turunannya. */
	// debug
	// autonote_print_data (note_head);
	while (note_head->prev !=NULL)
		note_head = note_head->prev;
	return note_head;
}
int autonote_print_data(struct autonote* arg)
{
	struct word_extractor* word_extr;
	struct autonote* temp;
	if (arg == NULL)
		return -1;
	temp = arg;
	while (temp->prev!=NULL)
		temp = temp->prev;
	while (temp!=NULL)
	{
		printf ("Autonote: %s %s %s\n",
			temp->kks,
			temp->val,
			temp->description);
		temp = temp->next;
	}
	return 0;
}

int autonote_print_example_file()
{
	FILE* file_temp;
	if ((file_temp = fopen(FNAM, "w"))==NULL)
		return -1;
	fprintf (file_temp, "# File autonotes.csv. Otomatis mencatat.\n");
	fprintf (file_temp, "# ######################################\n");
	fprintf (file_temp, "# Note: semua yang ada setelah tanda \"#\"\n");
	fprintf (file_temp, "#       hingga baris baru akan tidak dianggap.\n\n");
	fprintf (file_temp, "5 0ABA00 GS000||XB01;ON;Synchronized\n");
	fprintf (file_temp, "5 0ABA00 GS000||XB02;OFF;Desynchronized\n");
	fprintf (file_temp, "5 0HJA11 EY001||XV01;PROVEN;Oil gun AB1 Proven\n");
	fprintf (file_temp, "5 0HJF11 AA101||XB01;[OPEN];Oil gun AB1 mungkin dishutdown\n");
	fprintf (file_temp, "5 0HJF11 AA101||XB02;CLOSE;Oil gun AB1 shutdown\n");
	fprintf (file_temp, "5 0HJA12 EY001||XV01;PROVEN;Oil gun AB2 Proven\n");
	fprintf (file_temp, "5 0HJF12 AA101||XB01;[OPEN];Oil gun AB2 mungkin dishutdown\n");
	fprintf (file_temp, "5 0HJF12 AA101||XB02;CLOSE;Oil gun AB2 shutdown\n");
	fprintf (file_temp, "5 0HJA13 EY001||XV01;PROVEN;Oil gun AB3 Proven\n");
	fprintf (file_temp, "5 0HJF13 AA101||XB01;[OPEN];Oil gun AB3 mungkin dishutdown\n");
	fprintf (file_temp, "5 0HJF13 AA101||XB02;CLOSE;Oil gun AB3 shutdown\n");
	fprintf (file_temp, "5 0HJA14 EY001||XV01;PROVEN;Oil gun AB4 Proven\n");
	fprintf (file_temp, "5 0HJF14 AA101||XB01;[OPEN];Oil gun AB4 mungkin dishutdown\n");
	fprintf (file_temp, "5 0HJF14 AA101||XB02;CLOSE;Oil gun AB4 shutdown\n");
	fprintf (file_temp, "5 0HJA10 EY001||XV01;PROVEN;Oil gun Elevasi AB 3/4 Proven\n");
	fprintf (file_temp, "5 0HJA10 EY001||XV01;[PROVEN];Oil gun Elevasi AB 3/4 TIDAK LAGI Proven\n");
	fprintf (file_temp, "5 0HJA31 EY001||XV01;PROVEN;Oil gun CD1 Proven\n");
	fprintf (file_temp, "5 0HJF31 AA101||XB01;[OPEN];Oil gun CD1 mungkin dishutdown\n");
	fprintf (file_temp, "5 0HJF31 AA101||XB02;CLOSE;Oil gun CD1 shutdown\n");
	fprintf (file_temp, "5 0HJA32 EY001||XV01;PROVEN;Oil gun CD2 Proven\n");
	fprintf (file_temp, "5 0HJF32 AA101||XB01;[OPEN];Oil gun CD2 mungkin dishutdown\n");
	fprintf (file_temp, "5 0HJF32 AA101||XB02;CLOSE;Oil gun CD2 shutdown\n");
	fprintf (file_temp, "5 0HJA33 EY001||XV01;PROVEN;Oil gun CD3 Proven\n");
	fprintf (file_temp, "5 0HJF33 AA101||XB01;[OPEN];Oil gun CD3 mungkin dishutdown\n");
	fprintf (file_temp, "5 0HJF33 AA101||XB02;CLOSE;Oil gun CD3 shutdown\n");
	fprintf (file_temp, "5 0HJA34 EY001||XV01;PROVEN;Oil gun CD4 Proven\n");
	fprintf (file_temp, "5 0HJF34 AA101||XB01;[OPEN];Oil gun CD4 mungkin dishutdown\n");
	fprintf (file_temp, "5 0HJF34 AA101||XB02;CLOSE;Oil gun CD4 shutdown\n");
	fprintf (file_temp, "5 0HJA30 EY001||XV01;PROVEN;Oil gun Elevasi CD 3/4 Proven\n");
	fprintf (file_temp, "5 0HJA30 EY001||XV01;[PROVEN];Oil gun Elevasi CD 3/4 TIDAK LAGI Proven\n");
	fprintf (file_temp, "5 0HJA51 EY001||XV01;PROVEN;Oil gun EF1 Proven\n");
	fprintf (file_temp, "5 0HJF51 AA101||XB01;[OPEN];Oil gun EF1 mungkin dishutdown\n");
	fprintf (file_temp, "5 0HJF51 AA101||XB02;CLOSE;Oil gun EF1 shutdown\n");
	fprintf (file_temp, "5 0HJA52 EY001||XV01;PROVEN;Oil gun EF2 Proven\n");
	fprintf (file_temp, "5 0HJF52 AA101||XB01;[OPEN];Oil gun EF2 mungkin dishutdown\n");
	fprintf (file_temp, "5 0HJF52 AA101||XB02;CLOSE;Oil gun EF2 shutdown\n");
	fprintf (file_temp, "5 0HJA53 EY001||XV01;PROVEN;Oil gun EF3 Proven\n");
	fprintf (file_temp, "5 0HJF53 AA101||XB01;[OPEN];Oil gun EF3 mungkin dishutdown\n");
	fprintf (file_temp, "5 0HJF53 AA101||XB02;CLOSE;Oil gun EF3 shutdown\n");
	fprintf (file_temp, "5 0HJA54 EY001||XV01;PROVEN;Oil gun EF4 Proven\n");
	fprintf (file_temp, "5 0HJF54 AA101||XB01;[OPEN];Oil gun EF4 mungkin dishutdown\n");
	fprintf (file_temp, "5 0HJF54 AA101||XB02;CLOSE;Oil gun EF4 shutdown\n");
	fprintf (file_temp, "5 0HJA50 EY001||XV01;PROVEN;Oil gun Elevasi EF 3/4 Proven\n");
	fprintf (file_temp, "5 0HJA50 EY001||XV01;[PROVEN];Oil gun Elevasi EF 3/4 TIDAK LAGI Proven\n");	
	fclose (file_temp);
	return 0;
}

char* autonote_find_note(struct autonote* note, char* kks, char* val)
{
	if (note == NULL || kks == NULL || val == NULL)
		return ""; // terpaksa pakah string const, agar tidak ditulis null
	while (note->prev!=NULL)
		note = note->prev;
	while (note!=NULL)
	{
		// printf ("debug mencari note %s %s %s %s\n", note->kks, kks, note->val, val);
		if (!strcmp(note->kks, kks)
			&& !strcmp(note->val, val))
		{
			// printf ("debug ketemu notenya\n");
			return note->description;
		}
		note = note->next;
	}
	return NULL;
}