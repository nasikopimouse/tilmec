#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_param.h"


struct cmd_param* cmd_param_new (int arg_total, char** arg_content)
{
	struct cmd_param* temp;
	struct cmd_param* prev;
	struct cmd_param* head;
	int i;
	int mulai = 1;
	if (arg_total < 1 || arg_content == NULL)
		return NULL;
	for (i = 0; i<arg_total; i++) 
	{
		temp = (struct cmd_param*) malloc (sizeof (struct cmd_param));
		if (temp == NULL)
			return NULL;
		temp->index = i;
		temp->prev = NULL;
		temp->next = NULL;
		temp->param = (char*) malloc (1024);
		if (temp->param != NULL)
		{
			memset (temp->param, 0, 1024);
			strcpy (temp->param, arg_content[i]);
		}
		if (mulai == 0)
		{
			temp->prev = prev;
			prev->next = temp;
		}
		prev = temp;
		if (mulai == 1 )
		{
			head = temp;
			mulai = 0;
		}
	}
	return head;
}

int cmd_param_search (struct cmd_param* arg, char* str)
{
	struct cmd_param* temp;
	if (arg == NULL || str == NULL)
		return -1;
	temp = arg;
	while (temp->prev != NULL)
		temp = temp->prev;
	while (temp != NULL)
	{
		if (!strcmp (temp->param, str))
		{
			return temp->index;
		}
		temp = temp->next;
	}
	return 0;
}

int cmd_param_param_search (struct cmd_param* arg,
	char* param1, char* param2)
{
	struct cmd_param* temp;
	int urutan;
	if (arg == NULL ||
		param1 == NULL ||
		param2 == NULL)
		return -1;
	temp = arg;
	while (temp->prev != NULL)
		temp = temp->prev;
	urutan = cmd_param_search (temp, param1);
	if (urutan < 1)
		return 0;
	while (temp->index != urutan)
		temp = temp->next;
	while (temp != NULL)
	{
		if (!strcmp (temp->param, param2))
			return temp->index;
		temp = temp->next;
	}
	return 0;
}

int cmd_param_delete (struct cmd_param* arg)
{
	struct cmd_param* temp;
	struct cmd_param* temp1;
	if (arg == NULL)
		return -1;
	temp = arg;
	while (temp->prev != NULL)
		temp = temp->prev;
	while (temp != NULL)
	{
		temp1 = temp;
		memset (temp1->param, 0, 1024);
		free (temp1->param);
		temp = temp->next;
		free (temp1);
	}
	return 0;
}
