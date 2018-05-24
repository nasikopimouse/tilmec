#ifndef CMD_PARAM_H
#define CMD_PARAM_H

struct cmd_param
{
	struct cmd_param* next;
	struct cmd_param* prev;
	int index;
	char* param;
};

struct cmd_param*
cmd_param_new (int, char**);

/*
Mengambil parameter di argv berdasarkan
argc
*/
int
cmd_param_grab (struct cmd_param*, int, char**);

/*
method ini akan mencari parameter
di command line apakah ada atau tidak.
Jika ada maka akan return urutan arg di argc, jika tidak
ada maka akan return 0, jika fault maka -1.
*/
int
cmd_param_search (struct cmd_param*, char*);

/*
method ini akan mencari di command line
parameternya parameter. Parameter ada yang
mempunyai parameter lagi, seperti sub parameter,
atau parameternya parameter. Ini akan dideteksi
dengan lebih cepat dengan method ini, karena
tidak perlu mencari dari awal linked list untuk
parameternya parameter.
*/
int
cmd_param_param_search (struct cmd_param*, char*, char*);

int
cmd_param_delete (struct cmd_param*);


#endif