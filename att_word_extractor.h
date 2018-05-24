#ifndef ATT_WORD_EXTRACTOR_H
#define ATT_WORD_EXTRACTOR_H
#include <stdio.h>
#include <String.h>
#include <stdlib.h>
#define KARAKTER_SEPARATOR		';'
#define KARAKTER_NEWLINE		'\n'
#define KARAKTER_CR				'\r'
#define PANJANG_TMP 256

/*  satu kata atau beberapa kata yang dipisah dengan pemisah */
/*  disimpan sebagai  rantai linked list  */
struct content_word
{
	int index;
	struct content_word* prev;
	struct content_word* next;
	char* the_word;
	int separator;
};

struct word_extractor
{
	FILE* file_input;
	struct content_word* contents;
};

struct word_extractor* word_extractor_new(void);
int word_extractor_read_file (struct word_extractor*, char*);
int word_extractor_read_file_with_comment
	(struct word_extractor*, char*, char);
int word_extractor_print_content (struct word_extractor*);
int content_word_fill (struct content_word*, char*, int);
struct content_word* content_word_new(void);
int word_extractor_add_word (struct word_extractor*, struct content_word*);
int content_word_delete_chain (struct content_word*);
int word_extractor_delete(struct word_extractor*);
int word_extractor_test(char*);

#endif
