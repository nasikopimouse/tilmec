#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <assert.h>
#include "att_word_extractor.h"
#include "att_word_extractor_main.h"



struct word_extractor* word_extractor_new(void)
{
	struct word_extractor* temp;
	// printf ("debug sepertinya gagal mengalokasikan memori\n");
	temp = (struct word_extractor*) malloc
		(sizeof (struct word_extractor));
	// printf ("debug masuk ke word extractor\n");

	if (temp!=NULL)
	{
		temp->contents = NULL;
		temp->file_input = NULL;
	}
	return temp;
}

int word_extractor_read_file
	(struct word_extractor* temp, char* fnam)
{
	int panjang_string = 0;
	char word_tmp[PANJANG_TMP];
	FILE* file_temp;
	char ch;
	int ptr_word_tmp = 0;
	// int assertvar = 1;
	int word_ketemu = 0;
	int separator_count = 0, separator_store = 0;
	struct content_word* content_tmp;
	struct content_word* content;
	if (temp ==NULL || fnam == NULL)
		return -1;
	file_temp = (FILE*) fopen (fnam, "r");	
	if (file_temp == NULL)
	{
		printf ("Error membaca file %s, program dihentikan!\n", fnam);
		exit(-1);
	}
	memset(word_tmp, '\0', PANJANG_TMP);
	// printf ("debug masuk ke read file\n");
	while (!feof(file_temp))
	{
		ch = getc(file_temp);
		if (!(ch == KARAKTER_NEWLINE
			|| ch == KARAKTER_SEPARATOR
			|| ch == KARAKTER_CR
			/* || (int)ch < 32 || (int)ch > 127 */
			))
		{			
			word_ketemu = 1;
			/* memotong panjang kata jika melebihi 1023byte */
			if (panjang_string == (PANJANG_TMP - 1))
				continue;
			if ((int)ch < 32 || (int)ch > 127)
				ch = '?';
			memset(word_tmp + ptr_word_tmp, ch, 1);
			ptr_word_tmp++;
			panjang_string++;
			// assert(panjang_string < PANJANG_TMP);
			separator_store = separator_count;
		}
		else
		{
			// assert (0);
			if (word_ketemu == 1)
			{
				word_ketemu = 0;
				panjang_string = 0;
				// printf ("debug start record\n");
				content_tmp = content_word_new();
				// printf ("debug mungkin di sini\n");

				// assert (content_tmp != NULL);
				content = content_tmp;
				// assertvar = 
				content_word_fill (content, word_tmp, separator_store);
				// assert (assertvar == 0);
				// assertvar = 
				word_extractor_add_word (temp, content);
				// assert (assertvar == 0);
				ptr_word_tmp = 0;
				// printf ("debug word ketemu %s\n", word_tmp);
				memset(word_tmp, '\0', PANJANG_TMP);
				if (ch==KARAKTER_SEPARATOR)
					separator_count=1;
				
				continue;
			}
		}
		if (ch==KARAKTER_SEPARATOR)
			separator_count++;
	}
	// printf ("debug masuk read file1\n");
	if (fclose (file_temp) != 0)
	{
		printf ("Error close file. Program dihentikan.\n");
		exit (-1);
	}
	return 0;
}

int word_extractor_read_file_with_comment
	(struct word_extractor* temp, char* fnam, char cch)
{
	int panjang_string = 0;
	char word_tmp[PANJANG_TMP];
	FILE* file_temp;
	char ch, comment_ch;
	int ptr_word_tmp = 0;
	int word_ketemu = 0, comment_ketemu = 0;
	int separator_count = 0, separator_store = 0;
	struct content_word* content_tmp;
	struct content_word* content;
	comment_ch = cch;
	if (temp ==NULL || fnam == NULL)
		return -1;
	file_temp = (FILE*) fopen (fnam, "r");	
	if (file_temp == NULL)
		return -2;
	memset(word_tmp, '\0', PANJANG_TMP);
	while (!feof(file_temp))
	{
		ch = getc(file_temp);
		if (!(ch == KARAKTER_NEWLINE
			|| ch == KARAKTER_SEPARATOR
			|| ch == comment_ch))
		{
			// hanya mengisi jika tidak ditemukan tanda komen
			if (comment_ketemu == 1)
				continue;
			word_ketemu = 1;
			/* memotong panjang kata jika melebihi 1023byte */
			if (panjang_string == (PANJANG_TMP - 1))
				continue;
			memset(word_tmp + ptr_word_tmp, ch, 1);
			ptr_word_tmp++;
			panjang_string++;
			separator_store = separator_count;
		}
		else
		{
			if (ch == comment_ch)
				comment_ketemu = 1;
			if (ch == KARAKTER_NEWLINE)
				comment_ketemu = 0;
			if (word_ketemu == 1)
			{
				word_ketemu = 0;
				panjang_string = 0;
				content_tmp = content_word_new();
				content = content_tmp;
				content_word_fill (content, word_tmp, separator_store);
				word_extractor_add_word (temp, content);
				ptr_word_tmp = 0;
				memset(word_tmp, '\0', sizeof(word_tmp));
				if (ch==KARAKTER_SEPARATOR)
					separator_count=1;
				continue;
			}
		}
		if (ch==KARAKTER_SEPARATOR)
			separator_count++;
	}
	
	if (fclose (file_temp) != 0)
	{
		printf ("Error close file. Program dihentikan.\n");
		exit (-1);
	}
	return 0;
}


int content_word_fill (struct content_word* tmp, char* str, int sep)
{
	char* temp;
	if (tmp == NULL || str == NULL)
		return -1;
	temp = (char*) malloc (1024);
	if (temp == NULL)
		return -1;
	// printf ("debug masuk ke word fill\n");
	memset (temp, '\0', 1024);
	strcpy(temp, str);
	tmp->the_word = temp;
	tmp->separator = sep;
	return 0;
}

struct content_word* content_word_new()
{
	struct content_word* temp;
	temp = (struct content_word*) malloc
		(sizeof (struct content_word));
	if (temp!=NULL)
	{
		temp->index = 0;
		temp->prev = NULL;
		temp->next = NULL;
		temp->the_word = NULL;
		temp->separator = 0;
	}
	return temp;
}

int word_extractor_add_word(struct word_extractor* chain,
			struct content_word* chunk)
{
	if (chunk == NULL || chain == NULL)
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
	}
	return 0;
}

int content_word_delete_chain (struct content_word* temp)
{
	struct content_word* tmp;
	if (temp == NULL)
		return -1;
	while (temp->prev !=NULL)
		temp = temp->prev;
	while (temp!=NULL)
	{
		tmp = temp;
		temp = temp->next;
		tmp->prev = NULL;
		tmp->next = NULL;
		free (tmp->the_word);
		tmp->the_word = NULL;
		free(tmp);
		tmp = NULL;
	}
	return 0;
}

int word_extractor_delete (struct word_extractor* arg)
{
	if (arg == NULL)
		return -1;
	if (arg->contents!=NULL)
		content_word_delete_chain(arg->contents);
	arg->contents = NULL;
	arg->file_input = NULL;
	free(arg);
	return 0;
}

int word_extractor_print_content(struct word_extractor* tmp)
{
	struct content_word* temp;
	if (tmp == NULL)
		return -1;
	temp = tmp->contents;
	if (temp == NULL)
		return -1;
	if (temp != NULL)
	{
		while (temp ->prev != NULL)
			temp = temp->prev ;
		while (temp != NULL)
		{
			printf ("%d, %d, %s\n",
				temp->index, temp->separator, temp->the_word);
			temp = temp->next;
		}
	}
	return 0;
}

int word_extractor_test(char* fnam)
{
	struct word_extractor* temp;
	temp = word_extractor_new();
	if (temp == NULL)
		return -1;
	word_extractor_read_file (temp, fnam);
	word_extractor_print_content(temp);
	return 0;
}