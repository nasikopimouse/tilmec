#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fnam_handling.h"
#include "multi_file.h"
#include "dcs_time.h"
#include "autonotes.h"
#include <conio.h>
#include "tilmec_version.h"
	
	/*
	Indeks yang ada pada setiap chunk harus dalam satu
	row. Ini untuk memudahkan pencetakan data.
	Sehingga, data yang baru yang dimasukkan akan
	mempengaruhi susunan indeks dan kolom.
	Jika ada data kks analog yang baru, maka
	jika data ini waktunya antara trend sebelumnya,
	maka trend sebelumnya harus dipisah dengan
	menyisipkan chunk kosong dengan isi field
	value NULL. Hasil akhirnya adalah waktu tetap
	inline.
	Karena KKS pertama (yang prev adalah NULL)
	adalah yang dijadikan patokan time, maka
	trend pertama ini yang harus dimodifikasi dengan
	menyisipkan chunk kosong dengan sisipan waktu
	dari chunk data baru (chunk kosong karena 
	tentu saja kksnya berbeda. Jika kks sama maka
	data akan disisipkan dengan chunk yang isinya
	value).
	Pastikan mengecek plant unit apakah sama.
	Untuk KKS selanjutnya tidak perlu disisipi
	dengan chunk kosong, karena algoritma
	pengubahan ke CSV yang akan menyisipkannya.
	Jadi algoritma untuk data baru adalah:
	Lihat data yang ada di data baru.
	Apakah kks sama dengan kks pertama.
	Jika sama maka sisipkan ke kks eksisting.
	Menyisipkan dengan cara mencari dua waktu
	antara data kks lama yang mengapit data kks
	baru. Kemudian sisipkan.
	Jika kks tidak sama maka scanning ke kks selanjutnya.
	Jika ketemu di kks selanjutnya maka sisipkan
	ke kks eksisting selanjutnya. Cara penyisipannya
	sama dengan cara penyisipan di atas.
	Cek apakah waktunya berbeda dengan
	waktu kks pertama. Jika berbeda maka sisipkan
	chunk kosong (dengan waktu data kks baru) pada
	kks pertama. 
	*/


int susd ()
{
	struct kks_analog_trend_list* analog_new;
	struct binary_event_extractor* binary_new;
	analog_new = kks_analog_trend_list_multi_new();
	binary_new = binary_event_extractor_multi_new();
	if (analog_new == NULL || binary_new ==NULL)
	{
		printf ("Diperlukan file analog dan binary. Program dihentikan.\n");
		exit (-1);
	}
	if (analog_binary_joint_print_file_gold(analog_new, binary_new) != 0)
	{
		printf ("Gagal membuat file output. Progam dihentikan.\n");
		exit (-1);
	}
	return 0;
}	

int susd_mobile ()
{
	struct kks_analog_trend_list* analog_new;
	struct binary_event_extractor* binary_new;
	analog_new = kks_analog_trend_list_multi_new();
	binary_new = binary_event_extractor_multi_new();
	if (analog_new == NULL || binary_new ==NULL)
	{
		printf ("Diperlukan file analog dan binary. Program dihentikan.\n");
		exit (-1);
	}
	if (analog_binary_joint_print_file_gold_mobile(analog_new, binary_new) != 0)
	{
		printf ("Gagal membuat file output. Progam dihentikan.\n");
		exit (-1);
	}
	return 0;
}	


int excel_merge ()
{
	int a, b;
	struct kks_analog_trend_list* analog_new;
	struct binary_event_extractor* binary_new;
	analog_new = kks_analog_trend_list_multi_new();
	binary_new = binary_event_extractor_multi_new();
	a = 0;
	if (analog_new!=NULL)
	{
		if(analog_merge_print_file(analog_new)<0)
			a = 1;
	}
	else
		a = 1;
	b = 0;
	if (binary_new!=NULL)
	{
		if(binary_merge_print_file(binary_new)<0)
			b = 1;
	}
	else
		b = 1;
	
	if (a==1 && b==1)
	{
		printf ("Operasi merger error. Program dihentikan.\n");
		exit (-1);		
	}
	
	
	if (a == 1)
	{
		printf ("Tidak berhasil membuat file output analog.\n");
		return -1;
	}
	if (b == 1)
	{
		printf ("Tidak berhasil membuat file output binary.\n");
		return -1;
	}
	return 0;
}	



struct binary_event_extractor*
	binary_event_extractor_multi_new()
{
	struct file_list* listf;
	struct binary_event_extractor* temp;
	struct binary_event_extractor* temp_head = NULL;
	char* eventtyp;
	// listf = file_list_new(BINARY);
	listf = file_list_auto_new(BINARY);
	if (listf == NULL)
	{
		printf ("File data DCS binary tidak ditemukan.\n");
		return NULL;
	}
	while (listf->prev != NULL)
		listf = listf->prev;
	
	while (listf != NULL)
	{
		if (listf->file_mode == BINARY)
			eventtyp = "binary";
		else
			eventtyp = "analog";
		printf ("Memproses data %s %s... ", eventtyp, listf->file_name);		
		temp = binary_event_extractor_new(listf->file_name);
		if (temp_head == NULL)
			temp_head = temp;
		else
			binary_event_extractor_join(temp_head, temp);
		if (temp != NULL)
			printf ("selesai.\n");
		listf = listf->next;
	}
	return temp_head;
}


int binary_event_extractor_join (struct binary_event_extractor* chain,
	struct binary_event_extractor* chunk)
{
	struct binary_event *temp, *temp_head, *temp_tail;
	struct binary_event *temp1, *temp1_head, *temp1_tail;
	struct binary_event *temp2, *temp5, *temp6, *temp3;
	struct binary_event** array;
	int i, j, k, tidak_urut;
	
	if (chain == NULL || chunk == NULL)
		return -1;
	content_word_join (chain->all_content_data,
			chunk->all_content_data);
	temp = chain->contents;
	while (temp->next !=NULL)
		temp = temp->next;
	temp_tail = temp;
	while (temp->prev !=NULL)
		temp = temp->prev;
	temp_head = temp;
	// if (temp_head->prev != NULL)
	// printf ("debug ini adalah temp head %s\n", temp_head->datetime_str);
	temp1 = chunk->contents;
	while (temp1->prev != NULL)
		temp1 = temp1->prev;
	temp1_head = temp1;
	while (temp1->next !=NULL)
		temp1 = temp1->next;
	temp1_tail = temp1;
	if (apakah_waktu_urut
		(temp_tail->datetime_str, temp1_head->datetime_str) > 0)
	{
		// printf ("debug temp lebih dulu\n");
		temp_tail->next = temp1_head;
		temp1_head->prev = temp_tail;
		temp = temp_head;
	}
	else if (apakah_waktu_urut
		(temp1_tail->datetime_str, temp_head->datetime_str) > 0)
	{
		// printf ("debug temp1 lebih dulu\n");
		temp1_tail->next = temp_head;
		temp_head->prev = temp1_tail;
		temp = temp1_head;
		while (temp!=NULL)
		{
			// printf ("debug data temp1 lebih dulu: %s\n", temp->datetime_str);
			temp = temp->next;
		}
		temp = temp1_head;
	}
	else
	{
		temp2 = temp_head;
		while (temp2!=NULL)
		{
			if (apakah_waktu_urut (temp1_tail->datetime_str, temp2->datetime_str) == 0)
			{
				// printf ("debug menyisipkan data\n");
				if (temp2->next !=NULL)
					temp3 = temp2->next;
				else
					break;
				temp2->next = temp1_head;
				temp1_head->prev = temp2;
				temp1_tail->next = temp3;
				temp3->prev = temp1_tail;
				break;
			}
			temp2 = temp2->next;
		}
		// printf ("debug terserahlah\n");
		/*
		temp_tail->next = temp1_head;
		temp1_head->prev = temp_tail;
		*/
		temp = temp_head;
		while (temp!=NULL)
		{
			// printf ("debug data terserahlah: %s %s\n", temp->datetime_str, temp->usern);
			temp = temp->next;
		}
		temp = temp_head;
	}
	temp_head = temp;
	if (temp->prev != NULL)
	// printf ("debug mengurut secara blok selesai %s\n", temp->datetime_str);
	while (temp->prev!=NULL)
	{
		// printf ("debug mestinya tidak lama %s\n", temp->datetime_str);
		temp = temp->prev;
	}
	// printf ("debug lha tapi kok lama\n");
	while (temp!=NULL)
	{
		if (temp->next !=NULL)
			temp1 = temp->next;
		else
			break;
		while (temp1 !=NULL)
		{
			if (apakah_binary_chunk_sama(temp, temp1))
			{
				//printf ("debug binary sama %s %s\n", temp->datetime_str, temp1->datetime_str);
				if (temp1->next != NULL)
				{
					temp2 = temp1->prev;
					temp3 = temp1->next;
					if (temp3!=NULL)
					{
						temp2->next = temp3;
						temp3->prev = temp2;
						temp1 = temp3;
						continue;
					}
					else
					{
						temp2->next = NULL;
						break;
					}
				}
				else if (temp1->next == NULL)
				{
					temp2 = temp1->prev;
					temp2->next = NULL;
					temp= temp2;
				}
			}
			temp1 = temp1->next;
		}
		temp = temp->next;
	}
	// printf ("debug binary menghilangkan yang sama selesai\n");
	temp = temp_head;
	tidak_urut = 0;
	while (temp!=NULL)
	{
		if (temp->next!=NULL)
			temp1 = temp->next;
		else
			break;
		if (apakah_waktu_urut (temp1->datetime_str, temp->datetime_str) == 1)
		{
			tidak_urut = 1;
			break;
		}
		temp = temp->next;
	}
	if (tidak_urut == 0)
		goto sudah_urut;
	
	temp = temp_head;
	i = 0;
	while (temp!=NULL)
	{
		i++;
		temp = temp->next;
	}
	temp = temp_head;
	if (i< 2)
		return -1;
	array = (struct binary_event**) calloc
		(i, sizeof(struct binary_event*));
	j = 0;
	while (temp!=NULL)
	{
		*(array+j) = temp;
		j++;
		temp = temp->next;
	}
	qsort (array, i, sizeof(struct binary_event*), 
		tmline_urutkan_waktu_binary);
	array[0]->prev = NULL;
	array[i-1]->next = NULL;
	// printf ("debug isi array i-1 %s\n", array[i-1]->usern);
	for (k = 0; k <i-1; k++)
	{
		(*(array+k))->next = (*(array+k+1));
		(*(array+k+1))->prev = (*(array+k));
		// printf ("debug audrey mana %s\n", (*(array+k+1))->usern);
	}
	temp_head = array[0];
	temp = temp_head;
	free (array);
	sudah_urut:
	j = 0;
	while (temp!=NULL)
	{
		temp->index = j;
		j++;
		temp = temp->next;
	}
	/*
	temp = temp_head;
	while (temp!=NULL)
	{
		// printf ("debug add index %s\n", temp->usern);
		temp = temp->next;
	}
	temp = temp_head;

	// printf ("debug binary sorting selesai\n");
	while (temp!=NULL)
	{
		//printf ("debug result %d %s, %s %s %s\n",
			//temp->index,
			//temp->datetime_str,
			//temp->kks,
			//temp->description,
			//temp->val
			//);
		temp = temp->next;
	}

	*/
	return 0;
}


struct kks_analog_trend_list*
	kks_analog_trend_list_multi_new()
{
	struct file_list* listf;
	struct word_extractor* worde;
	struct kks_analog_trend_list* kks_list;
	struct kks_analog_trend_list* kks_list_head = NULL;
	char* eventtyp;
	
	// listf = file_list_new (ANALOG);
	listf = file_list_auto_new(ANALOG);
	if (listf == NULL)
	{
		printf ("file analog tidak ditemukan.\n");
		return NULL;
	}
	while (listf->prev != NULL)
		listf = listf->prev;
	while (listf != NULL)
	{
		if (listf->file_mode == ANALOG)
			eventtyp = "analog";
		else
			eventtyp = "binary";
		printf ("Memproses data %s %s... ", eventtyp, listf->file_name);
		worde = word_extractor_new();
		// printf ("debug mau akses ke fungsi add\n");

		word_extractor_read_file (worde, listf->file_name);

		kks_list = kks_analog_trend_list_new (worde);
		if (kks_list_head == NULL)
			kks_list_head = kks_list;
		else
		{
			/* ini yang penting, harus didevelop dengan hati2 bos */
			kks_analog_trend_list_add (kks_list_head, kks_list);
			/*
			Yang digabung dalam chain adalah kks saja,
			bukan struktur data chunk. Jadi, harus ada kode
			untuk garbage collecting untuk chunk.
			Kode di bawah ini adalah untuk purge memori.
			Hati-hati, karena ada beberapa field yang masih dipakai
			karena telah dijoin.
			Nanti saja dibikin. */
			// kks_analog_trend_list_delete_from_multi (kks_list);
		}
		if (kks_list != NULL)
			printf ("selesai.\n");
		listf = listf->next;
	}
	return kks_list_head;
}


int tmline_urutkan_waktu (const void* data1x, const void* data2x)
{
	char *tm1, *tm2;
	struct tmline** d1 = (struct tmline**)data1x;
	struct tmline** d2 = (struct tmline**)data2x;
	// printf ("debug masuk ke tmline urutkan waktu\n");
	tm1 = (*d1)->time_str;
	// printf ("debug time1 %d %s\n", data1->index, tm1);
	tm2 = (*d2)->time_str;
	// printf ("debug time2 %d %s\n", data2->index, tm2);	
	if (apakah_waktu_urut (tm1, tm2) == 1)
		return -1;
	else if (apakah_waktu_urut (tm1, tm2) == 0)
		return 1;
	return 0;
}

int tmline_urutkan_waktu_binary (const void* data1x, const void* data2x)
{
	char *tm1, *tm2;
	struct binary_event** d1 = (struct binary_event**)data1x;
	struct binary_event** d2 = (struct binary_event**)data2x;
	// printf ("debug masuk ke tmline urutkan waktu\n");
	tm1 = (*d1)->datetime_str;
	// printf ("debug time1 %d %s\n", data1->index, tm1);
	tm2 = (*d2)->datetime_str;
	// printf ("debug time2 %d %s\n", data2->index, tm2);	
	if (apakah_waktu_urut (tm1, tm2) == 1)
		return -1;
	else if (apakah_waktu_urut (tm1, tm2) == 0)
		return 1;
	return 0;
}



int kks_analog_trend_list_add
	(struct kks_analog_trend_list* chain,
		struct kks_analog_trend_list* chunk)
{
	struct kks_analog_trend* trend_temp_chain;
	struct kks_analog_trend* trend_temp_chain_head;
	struct kks_analog_trend* trend_temp_chunk;
	struct kks_analog_trend* trend_temp_chunk1;
	// struct kks_analog_trend* trend_temp_chunk_head;
	struct kks_analog_trend_data* data_temp;
	// struct kks_analog_trend_data* data_temp_head;
	struct tmline *timeline, *timeline1, *timeline2, *timeline3;
	struct tmline* timeline_head;
	struct tmline* timeline_tail;
	struct tmline* timeline_temp;
	struct content_word* all_content_data_temp;
	struct tmline **array;
	struct tmline* temp;
	struct tmline* temp1;
	struct tmline* temp_head;
	struct tmline* temp_tail;
	int i, j, k, pertamakali, tidak_urut;
	if (chain == NULL || chunk == NULL)
		return -1;
	if (chain->plant_unit != chunk->plant_unit)
	{
		printf ("Plant unit harus sama di semua file data. Program dihentikan.\n");
		exit (-1);
	}
	// printf ("debug masuk ke fungsi add\n");
	timeline = chain->timeline;
	data_temp = chunk->list_content->data;
	/* update timeline   */
	/* reset posisi data*/
	while (data_temp->prev != NULL)
		data_temp = data_temp->prev;
	// data_temp_head = data_temp;
	while (timeline->prev != NULL)
		timeline = timeline->prev;
	timeline_head = timeline;
	while(timeline->next!=NULL)
		timeline = timeline->next;
	timeline_tail = timeline;
	/* buat timeline baru berdasarkan data data_temp  */
	temp_head = NULL;
	while (data_temp!=NULL)
	{
		// printf ("debug bikin chunk baru\n");
		temp = tmline_new();
		temp->time_str = data_temp->time_str;
		if (temp_head == NULL)
			temp_head = temp;
		else
		{
			while (temp_head->next!=NULL)
				temp_head = temp_head->next;
			temp_head->next = temp;
			temp->prev = temp_head;
		}
		data_temp = data_temp->next;
	}
	while (temp_head->prev!=NULL)
		temp_head = temp_head->prev;
	temp = temp_head;
	while (temp->next!=NULL)
		temp = temp->next;
	temp_tail = temp;
	if (apakah_waktu_urut(timeline_tail->time_str, temp_head->time_str) > 0)
	{
		// printf ("debug timeline pertama\n");
		timeline_tail->next = temp_head;
		temp_head->prev = timeline_tail;
		timeline = timeline_head;
	}
	else if (apakah_waktu_urut(temp_tail->time_str, timeline_head->time_str) > 0)
	{
		// printf ("debug temp_tail pertama\n");
		temp_tail->next = timeline_head;
		timeline_head->prev = temp_tail;
		timeline = temp_head;
	}
	else
	{
		// printf ("debug analog terserahlah\n");
		timeline2 = timeline_head;
		while (timeline2!=NULL)
		{
			if (apakah_waktu_urut(temp_tail->time_str, timeline2->time_str) == 0 )
			{
				if (timeline2->next != NULL)
					timeline3 = timeline2->next;
				else
					break;
				timeline2->next = temp_head;
				temp_head->prev = timeline2;
				temp_tail->next = timeline3;
				timeline3->prev = temp_tail;
				break;
			}
			timeline2 = timeline2->next;
		}
		/*
		timeline_tail->next = temp_head;
		temp_head->prev = timeline_tail;
		*/
		timeline = timeline_head;		
	}
	// printf ("debug bikin chunk baru selesai\n");
	// printf ("debug hitung total jumlah chunk\n");
	while (timeline->prev!=NULL)
		timeline = timeline->prev;
	timeline_head = timeline;
	
	while (timeline!=NULL)
	{
		if (timeline->next!=NULL)
		{
			timeline1 = timeline->next;
		}
		else
			break;
		while (timeline1!=NULL)
		{
			if (!strcmp(timeline->time_str, timeline1->time_str))
			{
				if (timeline1->next!=NULL)
				{
					timeline2 = timeline1->prev;
					timeline3 = timeline1->next;
					if (timeline3!=NULL)
					{
						timeline2->next = timeline3;
						timeline3->prev = timeline2;
						timeline1 = timeline3;
						continue;
					}
					else
					{
						timeline2->next = NULL;
						break;
					}
				}
				else if (timeline1->next == NULL)
				{
					timeline2 = timeline1->prev;
					timeline2->next = NULL;
					timeline = timeline2;
				}
			}
			timeline1 = timeline1->next;
		}
		timeline = timeline->next;
	}
	// printf ("debug selesai membuang yang sama\n");
	timeline = timeline_head;
	tidak_urut = 0;
	while (timeline!=NULL)
	{
		if (timeline->next!=NULL)
			timeline2 = timeline->next;
		else
			break;
		if (apakah_waktu_urut (timeline2->time_str, timeline->time_str) == 1)
		{
			tidak_urut = 1;
			break;
		}
		timeline = timeline->next;
	}
	if (tidak_urut == 0)
		goto sudah_urut1;
	
	timeline = timeline_head;
	i = 0;
	while (timeline!=NULL)
	{
		i++;
		timeline = timeline->next;
	}
	// i--;
	if (i<2)
		return -1;
	/*
	timeline = timeline_head;
	j = 0;
	while (timeline!=NULL)
	{
		//printf ("debug result %d %s\n", j, timeline->time_str);
		j++;
		timeline = timeline->next;
	}
	*/
	// printf ("debug bikin array, masukkan pointer ke array besar %d %d\n", i, i*sizeof(struct tmline*));
	array = (struct tmline**) calloc (i, sizeof(struct tmline*));
	timeline = timeline_head;
	j = 0;
	while (timeline!=NULL)
	{
		*(array+j) = timeline;
		// printf("debug memasukkan data ke array %d %s\n", j, (*(array+j))->time_str);
		j++;
		timeline = timeline->next;
	}
	// printf ("debug memasukkan data selesai\n");
	// timeline = timeline_head;
	for (k = 0; k < i; k++)
	{
		// printf ("debug kok tidak mau masuk\n");
		// printf ("debug hasil array %d %s\n", k, (*(array+k))->time_str);
		//printf ("debug hasil array %d %s\n", k, array[k]->time_str);

	}
	//printf ("debug urutkan data\n");
	qsort (array, i, sizeof (struct tmline*), tmline_urutkan_waktu);
	//printf ("debug rangkai semua data\n");
	array[0]->prev = NULL;
	array[i-1]->next = NULL;
	for (k = 0; k < i-1; k++)
	{
		// printf ("debug merangkai semua data %d %s\n", k, array[k]->time_str);
		(*(array+k))->next = (*(array+k+1));
		(*(array+k+1))->prev = (*(array+k));
	}
	timeline_head = array[0];
	if (array!=NULL)
	{
		//printf ("debug tidak null\n");
		memset (array, 0, (i*sizeof(struct tmline*)));		
		free (array);
	}
	// printf ("debug ini urusan array\n");
	/*
	timeline = timeline_head;
	k = 0;
	while (timeline!=NULL)
	{
		// printf ("debug hasil pengurutan %d %s\n", k, timeline->time_str);
		k++;
		timeline = timeline->next;
	}
	*/
	sudah_urut1:
	j = 0;
	timeline = timeline_head;
	while (timeline!=NULL)
	{
		timeline->index = j;
		j++;
		// printf ("hasil final timeline add %d %s\n", timeline->index, timeline->time_str);
		timeline = timeline->next;
	}

	chain->timeline = timeline_head;
	// printf ("debug ya, betul, ini urusan array\n");

	trend_temp_chain = chain->list_content;
	trend_temp_chunk = chunk->list_content;
	/* update trend, list, jika kks sama gabungkan,
	jika tidak tambahkan */
	while (trend_temp_chain->prev != NULL)
		trend_temp_chain = trend_temp_chain->prev;
	trend_temp_chain_head = trend_temp_chain;
	while (trend_temp_chunk ->prev != NULL)
		trend_temp_chunk = trend_temp_chunk->prev;
	// trend_temp_chunk_head = trend_temp_chunk;

	while (trend_temp_chunk != NULL)
	{
		while (trend_temp_chain!=NULL)
		{
			if (!strcmp(trend_temp_chunk->kks, trend_temp_chain->kks))
			{
				kks_analog_trend_data_join (trend_temp_chain, trend_temp_chunk->data);
				trend_temp_chunk = trend_temp_chunk->next;
				trend_temp_chain = trend_temp_chain_head;
				break;
			}
			trend_temp_chain = trend_temp_chain->next;
		}

		if (trend_temp_chain == NULL)
		{
			trend_temp_chunk1 = trend_temp_chunk->next;
			kks_analog_trend_add (chain, trend_temp_chunk);
			trend_temp_chain = trend_temp_chain_head;
			trend_temp_chunk = trend_temp_chunk1;
		}
	}

	/* gabungkan all content data */
	content_word_join (chain->all_content_data, chunk->all_content_data);
	return 0;
}


int kks_analog_trend_data_join (struct kks_analog_trend* chainx,
	struct kks_analog_trend_data* chunkx)
{
	struct kks_analog_trend_data *chain, *chain1,
			*chain2, *chain3, *chain_head;
	struct kks_analog_trend_data *chunk;
	struct kks_analog_trend_data** array;
	int i, j, k;
	chain = chainx->data;
	chunk = chunkx;
	if (chain==NULL || chunk == NULL)
		return -1;
	while (chain->next !=NULL)
		chain = chain->next;
	while (chunk->prev !=NULL)
		chunk = chunk->prev;
	chain->next = chunk;
	chunk->prev = chain;
	while (chain->prev!=NULL)
		chain = chain->prev;
	chain_head = chain;
	i = 0;
	while (chain!=NULL)
	{
		i++;
		chain = chain->next;
	}
	if (i < 2)
		return -1;
	chain = chain_head;
	array = (struct kks_analog_trend_data**) calloc
		(i, sizeof(struct kks_analog_trend_data*));
	j = 0;
	while (chain!=NULL)
	{
		*(array+j) = chain;
		j++;
		chain = chain->next;
	}
	// chain = chain_head;
	qsort (array, i,
		sizeof(struct kks_analog_trend_data*),
		tmline_urutkan_waktu);
	array[0]->prev = NULL;
	array[i-1]->next = NULL;
	for (k = 0; k < i-2; k++)
	{
		(*(array+k))->next = (*(array+k+1));
		(*(array+k+1))->prev = (*(array+k));
	}
	chain_head = array[0];
	chain = chain_head;
	free (array);
	j = 0;
	while (chain!=NULL)
	{
		chain->index = j;
		j++;
		chain = chain->next;
	}
	// Menghapus yang sama tidak diperlukan, karena akan
	// memperpanjang waktu jalannya program.
	// Untuk mengimplementasikan ini, harus dibikinkan pula
	// kode untuk membebaskan memori.
	/*
	chain = chain_head;
	while (chain!=NULL)
	{
		if (chain->next!=NULL)
		{
			chain1 = chain->next;
		}
		else
			break;
		while (chain1!=NULL)
		{
			if (!strcmp(chain->time_str, chain1->time_str))
			{
				if (chain1->next!=NULL)
				{
					chain2 = chain1->prev;
					chain3 = chain1->next;
					if (chain3!=NULL)
					{
						chain2->next = chain3;
						chain3->prev = chain2;
						chain1 = chain3;
						continue;
					}
					else
					{
						chain2->next = NULL;
						break;
					}
				}
			}
			chain1 = chain1->next;
		}
		chain = chain->next;
	}
	*/
	chainx->data = chain_head;
	return 0;
}

int content_word_join (struct content_word* chain, struct content_word* chunk)
{
	struct content_word* chain_temp;
	struct content_word* chunk_temp;
	if (chain == NULL || chunk == NULL)
		return -1;
	chain_temp = chain;
	chunk_temp = chunk;
	while (chain_temp->next != NULL)
		chain_temp = chain_temp->next;
	while (chunk_temp->prev != NULL)
		chunk_temp = chunk_temp->prev;
	chain_temp->next = chunk_temp;
	chunk_temp->prev = chain_temp;
	return 0;
}

int analog_merge_print_file(struct kks_analog_trend_list* arg)
{
	char* tanggal_from;
	char* tanggal_to;
	struct kks_analog_trend* list_content;
	struct kks_analog_trend_data* data_temp;
	struct kks_analog_trend_data* data_temp1;
	struct kks_analog_trend_data* data_dummy;
	struct kks_analog_trend_data* data_temp_head;
	struct kks_analog_trend_data_wrapper* all_data_head = NULL;
	struct kks_analog_trend_data_wrapper* wrapper_chunk;
	struct kks_analog_trend_data_wrapper* wrapper_chain = NULL;
	struct kks_analog_trend_data_wrapper* wrapper_kks;
	struct kks_analog_trend_data_wrapper* wrapper_temp;
	struct kks_analog_trend_data_wrapper* wrapper_chain1;
	struct kks_analog_trend_data_wrapper* wrapper_chain2;	
	struct tmline* timeline_head;
	struct tmline* timeline;
	char* tabel_null = "v";
	char* isi_tabel;
	int index;
	struct dcs_time* time_calc;
	struct dcs_time* time_calc1;
	// int print_pertama_kali = 1;
	int j, l;
	char* tanggal1;
	char oe_user[1024];
	int mulai = 0, list_mulai = 0;
	int data_ketemu = 0;
	int data_ketemu1;
	FILE* output_file;
	char* output_fnam = "OUTPUT_ANALOG_VALUE_SEQUENCE.csv";
	char* tanggal;
	if (arg == NULL)
		return -1;
	if (arg->list_content == NULL)
		return -1;
	list_content = arg->list_content;
	while (list_content->prev != NULL)
		list_content = list_content->prev;
	timeline = arg->timeline;
		while (timeline->prev != NULL)
			timeline = timeline->prev;
	timeline_head = timeline;
	printf ("\nMulai memproses file output %s... \n", output_fnam);
	while (list_content != NULL)
	{ /* tambah kolom, prev-next (kks)  */
		printf ("Mulai memproses KKS %s... ", list_content->data->kks);
		timeline = timeline_head;
		while (timeline != NULL)
		{ /* tambah anggota dalam satu kolom  , upr-lwr (waktu) */
			// printf ("debug timeline print %s\n", timeline->time_str);
			wrapper_chunk = kks_analog_trend_data_wrapper_new();
			if (mulai == 1)
				kks_analog_trend_data_wrapper_join
							(wrapper_chain, wrapper_chunk);
			if (mulai == 0)
			{
				mulai = 1;
				wrapper_chain = wrapper_chunk;
			}
			data_ketemu1 = 0;			
			data_temp = list_content->data;
			while (data_temp->prev != NULL)
				data_temp = data_temp->prev;
				data_temp1 = data_temp;
				// printf ("debug data_temp1 description %s\n", data_temp1->description);
			while (data_temp != NULL)
			{
				if (!strcmp (data_temp->time_str, timeline->time_str))
				{
					data_ketemu1 = 1;
				// printf ("debug data dummy %s\n", data_temp->val);
					wrapper_chunk->data = data_temp;
					break;
				}
				data_temp = data_temp->next;
			}

			if (data_ketemu1 == 0)
			{
				// printf ("debug timeline print data = NULL\n");
				data_dummy = kks_analog_trend_data_new();
				data_dummy->time_str = timeline->time_str;
				data_dummy->kks = data_temp1->kks;
				data_dummy->description = data_temp1->description;
				data_dummy->eng_unit = data_temp1->eng_unit;
				data_dummy->val = "v";
				wrapper_chunk->data = data_dummy;
			}

			timeline = timeline->next;
		}
		if (list_mulai == 1)
			kks_analog_trend_data_wrapper_add
				(all_data_head, wrapper_chain);
		if (list_mulai == 0)
		{
			list_mulai = 1;
			all_data_head = wrapper_chain;
		}
		mulai = 0;
		printf ("selesai.\n");
		list_content = list_content->next;
	}
	/* mulai mencetak... */
	printf ("Mulai menulis di storage... \n");
	if ((output_file = fopen (output_fnam, "w")) == NULL)
	{
		printf ("Error operasi file. Hentikan program.\n");
		exit (-1);
	}
	printf ("Membuat header tabel data analog... ");
	wrapper_chunk = all_data_head;
	while (wrapper_chunk->prev != NULL)
		wrapper_chunk = wrapper_chunk->prev;
	wrapper_temp = wrapper_chunk;
	
	wrapper_chain1 = all_data_head;
	wrapper_chain2 = all_data_head;
	while (wrapper_chain2->lwr != NULL)
		wrapper_chain2 = wrapper_chain2->lwr;
	tanggal_from = wrapper_chain1->data->time_str;
	tanggal_to = wrapper_chain2->data->time_str;
	
	fprintf (output_file, "Analog Event Sequence;");
	

	while (wrapper_chunk != NULL)
	{
		if (wrapper_chunk->data->kks == NULL)
			isi_tabel = tabel_null;
		else
			isi_tabel = wrapper_chunk->data->kks;
		fprintf (output_file, "%s;", isi_tabel);
		wrapper_chunk = wrapper_chunk->next;
	}
	fprintf (output_file, "\n");
	fprintf (output_file, "From: %s;", tanggal_from);
	
	wrapper_chunk = wrapper_temp;
	while (wrapper_chunk != NULL)
	{
		if (wrapper_chunk->data->eng_unit == NULL)
			isi_tabel = tabel_null;
		else
			isi_tabel = wrapper_chunk->data->eng_unit;
		fprintf (output_file, "%s;", isi_tabel);
		wrapper_chunk = wrapper_chunk->next;
	}
	fprintf (output_file, "\n");
	fprintf (output_file, "To: %s;", tanggal_to);
	wrapper_chunk = wrapper_temp;
	index = 0;
	while (wrapper_chunk != NULL)
	{
		if (wrapper_chunk->data->description == NULL)
			isi_tabel = tabel_null;
		else
			isi_tabel = wrapper_chunk->data->description;
		fprintf (output_file, "{%d} %s;", index, isi_tabel);
		index++;
		wrapper_chunk = wrapper_chunk->next;
	}
	fprintf (output_file, "\n");
	printf ("selesai.\n");
	wrapper_chain = all_data_head;


	printf ("Membuat tabel data analog...\n");
	j = 0;
	l = 0;
	while (wrapper_chain != NULL)
	{
		wrapper_kks = all_data_head;
		// binary_data_temp = binary_data_temp_head;
		tanggal = wrapper_chain->data->time_str;
		
		if (j == 1000)
		{
			j = 0;
			printf ("%d baris data analog sudah selesai. Lanjutkan...\n", l);
		}
		///////////////////////////////////////////////
		fprintf (output_file, "*%s;", tanggal);
		while (wrapper_kks != NULL)
		{
			wrapper_chunk = wrapper_kks;
			while (wrapper_chunk != NULL)
			{
				if (!strcmp(wrapper_chunk->data->time_str, tanggal))
				{
					fprintf (output_file,"%s;", wrapper_chunk->data->val);
					data_ketemu = 1;
					break;
				}
				wrapper_chunk = wrapper_chunk->lwr;
			}
			if (data_ketemu != 1)
				fprintf (output_file, ";");
			data_ketemu = 0;
			wrapper_kks = wrapper_kks->next;
		}
		fprintf(output_file, "\n");
		//////////////////////////////////////////////

		wrapper_chain = wrapper_chain->lwr;
		j++, l++;
	}

		// fprintf (output_file, "That's all, Folks! Reformatted csv generated using Tilmec version %s. Disclaimer: No warranty of anykind is expressed or implied.\n", TILMEC_VERSION);
		print_tilmec_version(output_file);
	printf ("Tabel data analog selesai.\n");
	printf ("Total:\n%d baris data analog kumplit diproses.\n", l);
	if (fclose (output_file)!= 0)
	{
		printf
		("Error close file output %s, program dihentikan.\n",
			output_fnam);
		exit (-1);
	}	
	return 0;
}

int binary_merge_print_file(struct binary_event_extractor* arg_bin)
{
	int i;
	i = binary_event_extractor_print_file (arg_bin);
	printf ("%d baris data binary kumplit diproses.\n", i);
	return i;
}


int analog_binary_joint_print_file (struct kks_analog_trend_list* arg, struct binary_event_extractor* arg_bin)
{
	struct binary_event* binary_data_temp;
	struct binary_event* binary_data_temp_head;
	struct binary_event* binary_data_temp1;
	struct binary_event* binary_data_temp2;
	char* tanggal_from;
	char* tanggal_to;
	struct kks_analog_trend* list_content;
	struct kks_analog_trend_data* data_temp;
	struct kks_analog_trend_data* data_temp1;
	struct kks_analog_trend_data* data_dummy;
	struct kks_analog_trend_data* data_temp_head;
	struct kks_analog_trend_data_wrapper* all_data_head = NULL;
	struct kks_analog_trend_data_wrapper* wrapper_chunk;
	struct kks_analog_trend_data_wrapper* wrapper_chain = NULL;
	struct kks_analog_trend_data_wrapper* wrapper_kks;
	struct kks_analog_trend_data_wrapper* wrapper_temp;
	struct kks_analog_trend_data_wrapper* wrapper_chain1;
	struct kks_analog_trend_data_wrapper* wrapper_chain2;
	struct kks_analog_trend_data_wrapper* wrapper_chain3;
	struct tmline* timeline_head;
	struct tmline* timeline;
	time_t tanggal_sekarang;
	struct dcs_time* time_calc;
	struct dcs_time* time_calc1;
	struct autonote* note;
	char *note_buf, *note_buf1;
	// int print_pertama_kali = 1;
	int j, k, l, m;
	char *tanggal1, *tanggal_prev;
	char oe_user[1024];
	char* tabel_null = "v";
	char* isi_tabel;
	int mulai = 0, list_mulai = 0;
	int data_ketemu = 0;
	int data_ketemu1, index;
	FILE* output_file;
	FILE* output_file_resume;
	char* output_fnam = "OUTPUT_STARTUP_SEQUENCE.csv";
	char* output_resume_fnam = "OUTPUT_STARTUP_RESUME.csv";
	char* tanggal;
	if (arg == NULL)
		return -1;
	if (arg->list_content == NULL)
		return -1;
	list_content = arg->list_content;
	while (list_content->prev != NULL)
		list_content = list_content->prev;
	timeline = arg->timeline;
		while (timeline->prev != NULL)
			timeline = timeline->prev;
	timeline_head = timeline;
	note = autonote_read_file();
	printf ("\nMulai memproses file output %s... \n", output_fnam);
	while (list_content != NULL)
	{ /* tambah kolom, prev-next (kks)  */
		printf ("Mulai memproses KKS %s... ", list_content->data->kks);
		timeline = timeline_head;
		while (timeline != NULL)
		{ /* tambah anggota dalam satu kolom  , upr-lwr (waktu) */
			// printf ("debug timeline print %s\n", timeline->time_str);
			wrapper_chunk = kks_analog_trend_data_wrapper_new();
			if (mulai == 1)
				kks_analog_trend_data_wrapper_join
							(wrapper_chain, wrapper_chunk);
			if (mulai == 0)
			{
				mulai = 1;
				wrapper_chain = wrapper_chunk;
			}
			data_ketemu1 = 0;			
			data_temp = list_content->data;
			while (data_temp->prev != NULL)
				data_temp = data_temp->prev;
				data_temp1 = data_temp;
				// printf ("debug data_temp1 description %s\n", data_temp1->description);
			while (data_temp != NULL)
			{
				if (!strcmp (data_temp->time_str, timeline->time_str))
				{
					data_ketemu1 = 1;
				// printf ("debug data dummy %s\n", data_temp->val);
					wrapper_chunk->data = data_temp;
					break;
				}
				data_temp = data_temp->next;
			}

			if (data_ketemu1 == 0)
			{
				// printf ("debug timeline print data = NULL\n");
				data_dummy = kks_analog_trend_data_new();
				data_dummy->time_str = timeline->time_str;
				data_dummy->kks = data_temp1->kks;
				data_dummy->description = data_temp1->description;
				data_dummy->eng_unit = data_temp1->eng_unit;
				data_dummy->val = "v";
				wrapper_chunk->data = data_dummy;
			}

			timeline = timeline->next;
		}
		if (list_mulai == 1)
			kks_analog_trend_data_wrapper_add
				(all_data_head, wrapper_chain);
		if (list_mulai == 0)
		{
			list_mulai = 1;
			all_data_head = wrapper_chain;
		}
		mulai = 0;
		printf ("selesai.\n");
		list_content = list_content->next;
	}
	/* mulai mencetak... */
	wrapper_chunk = all_data_head;
	while (wrapper_chunk->prev != NULL)
		wrapper_chunk = wrapper_chunk->prev;
	wrapper_temp = wrapper_chunk;
	
	binary_data_temp = arg_bin->contents;
	while (binary_data_temp->prev != NULL)
		binary_data_temp = binary_data_temp->prev;
	binary_data_temp_head = binary_data_temp;
	binary_data_temp1 = binary_data_temp_head;
	binary_data_temp2 = binary_data_temp_head;
	while (binary_data_temp2->next!=NULL)
		binary_data_temp2 = binary_data_temp2->next;
	wrapper_chain1 = all_data_head;
	wrapper_chain2 = all_data_head;
	while (wrapper_chain2->lwr != NULL)
		wrapper_chain2 = wrapper_chain2->lwr;
	if (apakah_waktu_urut(binary_data_temp1->datetime_str, wrapper_chain1->data->time_str) == 1)
		tanggal_from = binary_data_temp1->datetime_str;
	else
		tanggal_from = wrapper_chain1->data->time_str;
	if (apakah_waktu_urut(binary_data_temp2->datetime_str, wrapper_chain2->data->time_str) == 1)
		tanggal_to = wrapper_chain2->data->time_str;
	else
		tanggal_to = binary_data_temp2->datetime_str;
	printf ("Mulai menulis di storage... \n");
	if
	(
			((output_file = fopen (output_fnam, "w")) == NULL)
			|| ((output_file_resume = fopen (output_resume_fnam, "w")) == NULL)
	)
	{
		printf ("Error operasi file. Hentikan program.\n");
		exit (-1);
	}
	printf ("Membuat header tabel data analog... ");	
	fprintf (output_file, "Sequence of Events;Binary [v];Analog [>];From;:;*%s;", tanggal_from);
	fprintf (output_file_resume, "Resume of Events;Binary [v];Analog [>];From;:;*%s;", tanggal_from);
	// fprintf (output_file, ",,,,,");
	/* membuat header KKS
	(untuk sumbu X adalah KKS, sumbu Y adalah waktu) */

	while (wrapper_chunk != NULL)
	{
		if (wrapper_chunk->data->kks == NULL)
			isi_tabel = tabel_null;
		else
			isi_tabel = wrapper_chunk->data->kks;
		fprintf (output_file, "%s;", isi_tabel);
		fprintf (output_file_resume, "%s;", isi_tabel);
		wrapper_chunk = wrapper_chunk->next;
	}
	fprintf (output_file, "\n");
	fprintf (output_file_resume, "\n");	
	tanggal_sekarang = time(NULL);
	fprintf (output_file, "Created: %s;$$$$$$$$$$$$$$$$$$$$$$;$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$;To;:;*%s;", ctime_hilangkan_newline(ctime(&tanggal_sekarang)), tanggal_to);
	fprintf (output_file_resume, "Created: %s;$$$$$$$$$$$$$$$$$$$$$$;$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$;To;:;*%s;", ctime_hilangkan_newline(ctime(&tanggal_sekarang)), tanggal_to);
	
	// fprintf (output_file, ",,,,,");
	wrapper_chunk = wrapper_temp;
	while (wrapper_chunk != NULL)
	{
		if (wrapper_chunk->data->eng_unit == NULL)
			isi_tabel = tabel_null;
		else
			isi_tabel = wrapper_chunk->data->eng_unit;
		fprintf (output_file, "%s;", isi_tabel);
		fprintf (output_file_resume, "%s;", isi_tabel);		
		wrapper_chunk = wrapper_chunk->next;
	}
	fprintf (output_file, "\n");
	fprintf (output_file, "Notes;KKS;Description;Value;Type;DateTime;");
	fprintf (output_file_resume, "\n");
	fprintf (output_file_resume, "Notes;KKS;Description;Value;Type;DateTime;");

	// fprintf (output_file, ",,,,,");
	wrapper_chunk = wrapper_temp;
	index = 0;
	while (wrapper_chunk != NULL)
	{
		if (wrapper_chunk->data->description == NULL)
			isi_tabel = tabel_null;
		else
			isi_tabel = wrapper_chunk->data->description;
		fprintf (output_file, "[%d]%s;", index, isi_tabel);
		fprintf (output_file_resume, "[%d]%s;", index, isi_tabel);
		index++;
		wrapper_chunk = wrapper_chunk->next;
	}
	fprintf (output_file, "\n");
	fprintf (output_file_resume, "\n");
	printf ("selesai.\n");
	wrapper_chain = all_data_head;
	/*
	posisi di head.
	catat tanggal
	tulis tanggal di file dan beri tanda koma
	catat nilai
	tulis nilai di file dan beri tanda koma
	next kks
	cari apakah tanggal cocok (upr_lwr)
	jika tanggal cocok tulis nilai di file dan beri tanda koma
	jika tanggal tidak ada yang cocok maka bari tanda koma saja
	next kks... (sampai next kks NULL)
	cari apakah tanggal cocok (upr_lwr)
	jika tanggal cocok tulis nilai di file dan beri tanda koma
	jika tanggal tidak ada yang cocok maka bari tanda koma saja
	*/

	printf ("Membuat tabel data analog...\n");
	j = 0;
	k = 0;
	l = 0;
	m = 0;
	while (wrapper_chain != NULL)
	{
		wrapper_kks = all_data_head;
		// binary_data_temp = binary_data_temp_head;
		tanggal = wrapper_chain->data->time_str;
		if (wrapper_chain->prev!=NULL)
		{
			wrapper_chain3 = wrapper_chain->prev;
			tanggal_prev = wrapper_chain3->data->time_str;
		}
		else
			tanggal_prev = NULL;
		time_calc = dcs_time_new();
		time_calc1 = dcs_time_new();	
		dcs_time_set_timestr_1(time_calc, tanggal);		
		if (wrapper_chain->lwr!=NULL)
		{
			tanggal1 = wrapper_chain->lwr->data->time_str;
			dcs_time_set_timestr_1(time_calc1, tanggal1);
		}
		else
			tanggal1 = NULL;
		
		// binary_data_temp = binary_data_temp_head;
		while (binary_data_temp!=NULL)
		{
			// printf ("Debug analog1: %s binary: %s analog2: %s\n",
				// tanggal, binary_data_temp->datetime_str, tanggal1);
			
			// printf ("Debug sisip %s %s %s\n", 
			//	binary_data_temp->datetime_str,
			//	binary_data_temp->kks,
			//	binary_data_temp->description);
			dcs_time_set_timestr_2(time_calc, binary_data_temp->datetime_str);
			dcs_time_get_diff_time(time_calc);
			dcs_time_set_timestr_2(time_calc1, binary_data_temp->datetime_str);
			
			if (dcs_time_get_status(time_calc)== 1)
			{
				//printf ("Debug scan tanggal continue %s %s\n",
				//	tanggal, binary_data_temp->datetime_str
				//);
				break;
			}

			{
				note_buf = autonote_find_note(note, binary_data_temp->kks, binary_data_temp->val);
				if (note_buf == NULL)
					note_buf1 = "";
				else
					note_buf1 = note_buf;
				if (l == 0)
				{
					printf ("Menyisipkan event binary...\n");
					l = 1;
				}
				m++;
				memset (oe_user, 0, 1024);

				if (binary_data_temp->seqmod == BINARY_OPERATION_SEQUENCE)
				{
					strcpy (oe_user, binary_data_temp->evt_type);
					strcat (oe_user, "/");
					strcat (oe_user, binary_data_temp->usern);
				}
				else
				{
					strcpy (oe_user, binary_data_temp->evt_type);
				}
				if (binary_data_temp->val_qlty!= NULL)
				{
					fprintf (output_file, "%s;%s;[%s][%d]%s;%s;%s;*%s\n",
						note_buf1,
						binary_data_temp->kks,
						binary_data_temp->val_qlty,
						binary_data_temp->index,
						binary_data_temp->description,
						binary_data_temp->val,
						oe_user,
						binary_data_temp->datetime_str);
					if (note_buf!=NULL)
						fprintf (output_file_resume, "%s;%s;[%s][%d]%s;%s;%s;*%s\n",
							note_buf1,
							binary_data_temp->kks,
							binary_data_temp->val_qlty,
							binary_data_temp->index,
							binary_data_temp->description,
							binary_data_temp->val,
							oe_user,
							binary_data_temp->datetime_str);
				}
				else
				{
					fprintf (output_file, "%s;%s;[%d]%s;%s;%s;*%s\n",
						note_buf1,
						binary_data_temp->kks,
						binary_data_temp->index,
						binary_data_temp->description,
						binary_data_temp->val,
						oe_user,
						binary_data_temp->datetime_str);
					if (note_buf!=NULL)
						fprintf (output_file_resume, "%s;%s;[%d]%s;%s;%s;*%s\n",
							note_buf1,
							binary_data_temp->kks,
							binary_data_temp->index,
							binary_data_temp->description,
							binary_data_temp->val,
							oe_user,
							binary_data_temp->datetime_str);
				}
				///////////////////////////////////////////////
				if (note_buf!=NULL)
				{
					note_buf = NULL;
					if (tanggal_prev!=NULL)
					{
						data_ketemu = 0;
						wrapper_kks = all_data_head;
						fprintf (output_file_resume, ";;;;;*%s;", tanggal_prev);
						while (wrapper_kks != NULL)
						{
							wrapper_chunk = wrapper_kks;
							while (wrapper_chunk != NULL)
							{
								if (!strcmp(wrapper_chunk->data->time_str, tanggal_prev))
								{
									fprintf (output_file_resume,"%s;", wrapper_chunk->data->val);					
									data_ketemu = 1;
									break;
								}
								wrapper_chunk = wrapper_chunk->lwr;
							}
							if (data_ketemu != 1)
								fprintf (output_file_resume, ";");
							data_ketemu = 0;
							wrapper_kks = wrapper_kks->next;
						}
						fprintf(output_file_resume, "\n");
					}										
					data_ketemu = 0;
					wrapper_kks = all_data_head;
					fprintf (output_file_resume, ";;;;;*%s;", tanggal);
					while (wrapper_kks != NULL)
					{
						wrapper_chunk = wrapper_kks;
						while (wrapper_chunk != NULL)
						{
							if (!strcmp(wrapper_chunk->data->time_str, tanggal))
							{
								fprintf (output_file_resume,"%s;", wrapper_chunk->data->val);					
								data_ketemu = 1;
								break;
							}
							wrapper_chunk = wrapper_chunk->lwr;
						}
						if (data_ketemu != 1)
							fprintf (output_file_resume, ";");
						data_ketemu = 0;
						wrapper_kks = wrapper_kks->next;
					}
					fprintf(output_file_resume, "\n");
				}
				//////////////////////////////////////////////
				// seharusnya untuk resume data analog diletakkan di sini.
			}
			if (tanggal1!=NULL)
			{
				dcs_time_get_diff_time(time_calc1);
				if (dcs_time_get_status(time_calc1)  > 0)
				{
					// printf ("Debug scan tanggal break\n");
					break;
				}
			}
			binary_data_temp = binary_data_temp->next;
		}		
		if (j == 1000)
		{
			j = 0;
			printf ("%d baris data analog sudah selesai. Lanjutkan...\n", k);
			if (l == 1)
				l = 0;
		}
		///////////////////////////////////////////////
		fprintf (output_file, ";;;;;*%s;", tanggal);
		wrapper_kks = all_data_head;
		data_ketemu = 0;
		while (wrapper_kks != NULL)
		{
			wrapper_chunk = wrapper_kks;
			while (wrapper_chunk != NULL)
			{
				if (!strcmp(wrapper_chunk->data->time_str, tanggal))
				{
					fprintf (output_file,"%s;", wrapper_chunk->data->val);
					data_ketemu = 1;
					break;
				}
				wrapper_chunk = wrapper_chunk->lwr;
			}
			if (data_ketemu != 1)
				fprintf (output_file, ";");
			data_ketemu = 0;
			wrapper_kks = wrapper_kks->next;
		}
		fprintf(output_file, "\n");
		//////////////////////////////////////////////
		wrapper_chain = wrapper_chain->lwr;
		j++; k++;
	}
	while (binary_data_temp!=NULL)
	{
		{
			// printf ("debug sisipkan event binary\n");
			note_buf = autonote_find_note(note, binary_data_temp->kks, binary_data_temp->val);
			if (note_buf == NULL)
				note_buf1 = "";
			else
				note_buf1 = note_buf;
			if (l == 0)
			{
				printf ("Menyisipkan event binary...\n");
				l = 1;
			}
			m++;
			memset (oe_user, 0, 1024);
			if (binary_data_temp->seqmod == BINARY_OPERATION_SEQUENCE)
			{
				strcpy (oe_user, binary_data_temp->evt_type);
				strcat (oe_user, "/");
				strcat (oe_user, binary_data_temp->usern);
			}
			else
			{
				strcpy (oe_user, binary_data_temp->evt_type);
			}
			if (binary_data_temp->val_qlty!= NULL)
			{
				fprintf (output_file, "%s;%s;[%s][%d]%s;%s;%s;*%s\n",
					note_buf1,
					binary_data_temp->kks,
					binary_data_temp->val_qlty,
					binary_data_temp->index,
					binary_data_temp->description,
					binary_data_temp->val,
					oe_user,
					binary_data_temp->datetime_str);
				if (note_buf!=NULL)
					fprintf (output_file_resume, "%s;%s;[%s][%d]%s;%s;%s;*%s\n",
						note_buf1,
						binary_data_temp->kks,
						binary_data_temp->val_qlty,
						binary_data_temp->index,
						binary_data_temp->description,
						binary_data_temp->val,
						oe_user,
						binary_data_temp->datetime_str);					
			}
			else
			{
				fprintf (output_file, "%s;%s;[%d]%s;%s;%s;*%s\n",
					note_buf1,
					binary_data_temp->kks,
					binary_data_temp->index,
					binary_data_temp->description,
					binary_data_temp->val,
					oe_user,
					binary_data_temp->datetime_str);
				if (note_buf!=NULL)
					fprintf (output_file_resume, "%s;%s;[%s][%d]%s;%s;%s;*%s\n",
						note_buf1,
						binary_data_temp->kks,
						binary_data_temp->val_qlty,
						binary_data_temp->index,
						binary_data_temp->description,
						binary_data_temp->val,
						oe_user,
						binary_data_temp->datetime_str);					
			}
		}
		binary_data_temp = binary_data_temp->next;
	}		
		// fprintf (output_file, "That's all, Folks! Reformatted csv generated using Tilmec version %s. Disclaimer: No warranty of anykind is expressed or implied.\n", TILMEC_VERSION);
		print_tilmec_version(output_file);
	fprintf(output_file_resume, ">>>>>>>That's all the resume, folks!\n");
	printf ("Total:\n%d baris data analog sudah kumplit diproses.\n", k);
	printf ("%d baris data binary sudah juga kumplit diproses.\n", m);	
	if ((fclose (output_file)!= 0)||(fclose (output_file_resume)!= 0))
	{
		printf
		("Error close file output %s atau %s, program dihentikan.\n",
			output_fnam, output_resume_fnam);
		exit (-1);
	}
	return 0;
}

char* ctime_hilangkan_newline(char* in)
{
	if (in == NULL)
		return NULL;
	memset (in+24, 0, 1);
	return in;
}

int analog_binary_joint_print_file_gold (struct kks_analog_trend_list* arg, struct binary_event_extractor* arg_bin)
{
	struct binary_event* binary_data_temp;
	struct binary_event* binary_data_temp_head;
	struct binary_event* binary_data_temp1;
	struct binary_event* binary_data_temp2;
	char* tanggal_from;
	char* tanggal_to;
	struct kks_analog_trend* list_content;
	struct kks_analog_trend_data* data_temp;
	struct kks_analog_trend_data* data_temp1;
	struct kks_analog_trend_data* data_dummy;
	struct kks_analog_trend_data* data_temp_head;
	struct kks_analog_trend_data_wrapper* all_data_head = NULL;
	struct kks_analog_trend_data_wrapper* wrapper_chunk;
	struct kks_analog_trend_data_wrapper* wrapper_chain = NULL;
	struct kks_analog_trend_data_wrapper* wrapper_kks;
	struct kks_analog_trend_data_wrapper* wrapper_temp;
	struct kks_analog_trend_data_wrapper* wrapper_chain1;
	struct kks_analog_trend_data_wrapper* wrapper_chain2;	
	struct tmline* timeline_head;
	struct tmline* timeline;
	time_t tanggal_sekarang;
	struct dcs_time* time_calc;
	struct dcs_time* time_calc1;
	struct autonote* note;
	char *note_buf, *note_buf1;
	// int print_pertama_kali = 1;
	int j, k, l, m;
	char* tanggal1;
	char oe_user[1024];
	char* tabel_null = "v";
	char* isi_tabel;
	int mulai = 0, list_mulai = 0;
	int data_ketemu = 0;
	int data_ketemu1, index;
	FILE* output_file;
	char* output_fnam = "OUTPUT_STARTUP_SEQUENCE.csv";
	char* tanggal;
	if (arg == NULL)
		return -1;
	if (arg->list_content == NULL)
		return -1;
	list_content = arg->list_content;
	while (list_content->prev != NULL)
		list_content = list_content->prev;
	timeline = arg->timeline;
		while (timeline->prev != NULL)
			timeline = timeline->prev;
	timeline_head = timeline;
	note = autonote_read_file();
	printf ("\nMulai memproses file output %s... \n", output_fnam);
	while (list_content != NULL)
	{ /* tambah kolom, prev-next (kks)  */
		printf ("Mulai memproses KKS %s... ", list_content->data->kks);
		timeline = timeline_head;
		while (timeline != NULL)
		{ /* tambah anggota dalam satu kolom  , upr-lwr (waktu) */
			// printf ("debug timeline print %s\n", timeline->time_str);
			wrapper_chunk = kks_analog_trend_data_wrapper_new();
			if (mulai == 1)
				kks_analog_trend_data_wrapper_join
							(wrapper_chain, wrapper_chunk);
			if (mulai == 0)
			{
				mulai = 1;
				wrapper_chain = wrapper_chunk;
			}
			data_ketemu1 = 0;			
			data_temp = list_content->data;
			while (data_temp->prev != NULL)
				data_temp = data_temp->prev;
				data_temp1 = data_temp;
				// printf ("debug data_temp1 description %s\n", data_temp1->description);
			while (data_temp != NULL)
			{
				if (!strcmp (data_temp->time_str, timeline->time_str))
				{
					data_ketemu1 = 1;
				// printf ("debug data dummy %s\n", data_temp->val);
					wrapper_chunk->data = data_temp;
					break;
				}
				data_temp = data_temp->next;
			}

			if (data_ketemu1 == 0)
			{
				// printf ("debug timeline print data = NULL\n");
				data_dummy = kks_analog_trend_data_new();
				data_dummy->time_str = timeline->time_str;
				data_dummy->kks = data_temp1->kks;
				data_dummy->description = data_temp1->description;
				data_dummy->eng_unit = data_temp1->eng_unit;
				data_dummy->val = "v";
				wrapper_chunk->data = data_dummy;
			}

			timeline = timeline->next;
		}
		if (list_mulai == 1)
			kks_analog_trend_data_wrapper_add
				(all_data_head, wrapper_chain);
		if (list_mulai == 0)
		{
			list_mulai = 1;
			all_data_head = wrapper_chain;
		}
		mulai = 0;
		printf ("selesai.\n");
		list_content = list_content->next;
	}
	/* mulai mencetak... */
	wrapper_chunk = all_data_head;
	while (wrapper_chunk->prev != NULL)
		wrapper_chunk = wrapper_chunk->prev;
	wrapper_temp = wrapper_chunk;
	
	binary_data_temp = arg_bin->contents;
	while (binary_data_temp->prev != NULL)
		binary_data_temp = binary_data_temp->prev;
	binary_data_temp_head = binary_data_temp;
	binary_data_temp1 = binary_data_temp_head;
	binary_data_temp2 = binary_data_temp_head;
	while (binary_data_temp2->next!=NULL)
		binary_data_temp2 = binary_data_temp2->next;
	wrapper_chain1 = all_data_head;
	wrapper_chain2 = all_data_head;
	while (wrapper_chain2->lwr != NULL)
		wrapper_chain2 = wrapper_chain2->lwr;
	if (apakah_waktu_urut(binary_data_temp1->datetime_str, wrapper_chain1->data->time_str) == 1)
		tanggal_from = binary_data_temp1->datetime_str;
	else
		tanggal_from = wrapper_chain1->data->time_str;
	if (apakah_waktu_urut(binary_data_temp2->datetime_str, wrapper_chain2->data->time_str) == 1)
		tanggal_to = wrapper_chain2->data->time_str;
	else
		tanggal_to = binary_data_temp2->datetime_str;
	printf ("Mulai menulis di storage... \n");
	if ((output_file = fopen (output_fnam, "w")) == NULL)
	{
		printf ("Error operasi file. Hentikan program.\n");
		exit (-1);
	}
	printf ("Membuat header tabel data analog... ");	
	fprintf (output_file, "Sequence of Events;Binary [v] Analog [>];;From;:;*%s;", tanggal_from);
	
	// fprintf (output_file, ",,,,,");
	/* membuat header KKS
	(untuk sumbu X adalah KKS, sumbu Y adalah waktu) */

	while (wrapper_chunk != NULL)
	{
		if (wrapper_chunk->data->kks == NULL)
			isi_tabel = tabel_null;
		else
			isi_tabel = wrapper_chunk->data->kks;
		fprintf (output_file, "%s;", isi_tabel);
		wrapper_chunk = wrapper_chunk->next;
	}
	fprintf (output_file, "\n");
	tanggal_sekarang = time(NULL);
	fprintf (output_file, "Created: %s;;;To;:;*%s;", ctime_hilangkan_newline(ctime(&tanggal_sekarang)), tanggal_to);
	
	// fprintf (output_file, ",,,,,");
	wrapper_chunk = wrapper_temp;
	while (wrapper_chunk != NULL)
	{
		if (wrapper_chunk->data->eng_unit == NULL)
			isi_tabel = tabel_null;
		else
			isi_tabel = wrapper_chunk->data->eng_unit;
		fprintf (output_file, "%s;", isi_tabel);
		wrapper_chunk = wrapper_chunk->next;
	}
	fprintf (output_file, "\n");
	fprintf (output_file, "Notes;KKS;Description;Value;Type;DateTime;");
	// fprintf (output_file, ",,,,,");
	wrapper_chunk = wrapper_temp;
	index = 0;
	while (wrapper_chunk != NULL)
	{
		if (wrapper_chunk->data->description == NULL)
			isi_tabel = tabel_null;
		else
			isi_tabel = wrapper_chunk->data->description;
		fprintf (output_file, "{%d} %s;", index, isi_tabel);
		index++;
		wrapper_chunk = wrapper_chunk->next;
	}
	fprintf (output_file, "\n");
	printf ("selesai.\n");
	wrapper_chain = all_data_head;
	/*
	posisi di head.
	catat tanggal
	tulis tanggal di file dan beri tanda koma
	catat nilai
	tulis nilai di file dan beri tanda koma
	next kks
	cari apakah tanggal cocok (upr_lwr)
	jika tanggal cocok tulis nilai di file dan beri tanda koma
	jika tanggal tidak ada yang cocok maka bari tanda koma saja
	next kks... (sampai next kks NULL)
	cari apakah tanggal cocok (upr_lwr)
	jika tanggal cocok tulis nilai di file dan beri tanda koma
	jika tanggal tidak ada yang cocok maka bari tanda koma saja
	*/

	printf ("Membuat tabel data analog...\n");
	j = 0;
	k = 0;
	l = 0;
	m = 0;
	while (wrapper_chain != NULL)
	{
		wrapper_kks = all_data_head;
		// binary_data_temp = binary_data_temp_head;
		tanggal = wrapper_chain->data->time_str;

		time_calc = dcs_time_new();
		time_calc1 = dcs_time_new();	
		dcs_time_set_timestr_1(time_calc, tanggal);		
		if (wrapper_chain->lwr!=NULL)
		{
			tanggal1 = wrapper_chain->lwr->data->time_str;
			dcs_time_set_timestr_1(time_calc1, tanggal1);
		}
		else
			tanggal1 = NULL;
		
		// binary_data_temp = binary_data_temp_head;
		while (binary_data_temp!=NULL)
		{
			// printf ("Debug analog1: %s binary: %s analog2: %s\n",
				// tanggal, binary_data_temp->datetime_str, tanggal1);
			
			// printf ("Debug sisip %s %s %s\n", 
			//	binary_data_temp->datetime_str,
			//	binary_data_temp->kks,
			//	binary_data_temp->description);
			dcs_time_set_timestr_2(time_calc, binary_data_temp->datetime_str);
			dcs_time_get_diff_time(time_calc);
			dcs_time_set_timestr_2(time_calc1, binary_data_temp->datetime_str);
			
			if (dcs_time_get_status(time_calc)== 1)
			{
				//printf ("Debug scan tanggal continue %s %s\n",
				//	tanggal, binary_data_temp->datetime_str
				//);
				break;
			}

			{
				if (l == 0)
				{
					printf ("Menyisipkan event binary...\n");
					l = 1;
				}
				m++;
				memset (oe_user, 0, 1024);

				if (binary_data_temp->seqmod == BINARY_OPERATION_SEQUENCE)
				{
					strcpy (oe_user, binary_data_temp->evt_type);
					strcat (oe_user, "/");
					strcat (oe_user, binary_data_temp->usern);
				}
				else
				{
					strcpy (oe_user, binary_data_temp->evt_type);
				}
				note_buf = autonote_find_note(note, binary_data_temp->kks, binary_data_temp->val);
				if (note_buf == NULL)
					note_buf1 = "";
				else
					note_buf1 = note_buf;
				if (binary_data_temp->val_qlty!= NULL)
				{
					fprintf (output_file, "%s;%s;{%s} {%d} %s;%s;%s;*%s\n",
						note_buf1,
						binary_data_temp->kks,
						binary_data_temp->val_qlty,
						binary_data_temp->index,
						binary_data_temp->description,
						binary_data_temp->val,
						oe_user,
						binary_data_temp->datetime_str);					
				}
				else
				{
					fprintf (output_file, "%s;%s;{%d} %s;%s;%s;*%s\n",
						note_buf1,
						binary_data_temp->kks,
						binary_data_temp->index,
						binary_data_temp->description,
						binary_data_temp->val,
						oe_user,
						binary_data_temp->datetime_str);
				}
			}
			if (tanggal1!=NULL)
			{
				dcs_time_get_diff_time(time_calc1);
				if (dcs_time_get_status(time_calc1)  > 0)
				{
					// printf ("Debug scan tanggal break\n");
					break;
				}
			}			
			binary_data_temp = binary_data_temp->next;
		}		
		if (j == 1000)
		{
			j = 0;
			printf ("%d baris data analog sudah selesai. Lanjutkan...\n", k);
			if (l == 1)
				l = 0;
		}
		///////////////////////////////////////////////
		fprintf (output_file, ";;;;;*%s;", tanggal);
		while (wrapper_kks != NULL)
		{
			wrapper_chunk = wrapper_kks;
			while (wrapper_chunk != NULL)
			{
				if (!strcmp(wrapper_chunk->data->time_str, tanggal))
				{
					fprintf (output_file,"%s;", wrapper_chunk->data->val);
					data_ketemu = 1;
					break;
				}
				wrapper_chunk = wrapper_chunk->lwr;
			}
			if (data_ketemu != 1)
				fprintf (output_file, ";");
			data_ketemu = 0;
			wrapper_kks = wrapper_kks->next;
		}
		fprintf(output_file, "\n");
		//////////////////////////////////////////////

		wrapper_chain = wrapper_chain->lwr;
		j++; k++;
	}
	while (binary_data_temp!=NULL)
	{
		{
			// printf ("debug sisipkan event binary\n");
			if (l == 0)
			{
				printf ("Menyisipkan event binary...\n");
				l = 1;
			}
			m++;
			memset (oe_user, 0, 1024);
			if (binary_data_temp->seqmod == BINARY_OPERATION_SEQUENCE)
			{
				strcpy (oe_user, binary_data_temp->evt_type);
				strcat (oe_user, "/");
				strcat (oe_user, binary_data_temp->usern);
			}
			else
			{
				strcpy (oe_user, binary_data_temp->evt_type);
			}
			note_buf = autonote_find_note(note, binary_data_temp->kks, binary_data_temp->val);
			if (note_buf==NULL)
				note_buf1 = "";
			else
				note_buf1 = note_buf;
			if (binary_data_temp->val_qlty!= NULL)
			{
				fprintf (output_file, "%s;%s;[%s][%d]%s;%s;%s;*%s\n",
					note_buf1,
					binary_data_temp->kks,
					binary_data_temp->val_qlty,
					binary_data_temp->index,
					binary_data_temp->description,
					binary_data_temp->val,
					oe_user,
					binary_data_temp->datetime_str);					
			}
			else
			{
				fprintf (output_file, "%s;%s;[%d]%s;%s;%s;*%s\n",
					note_buf1,
					binary_data_temp->kks,
					binary_data_temp->index,
					binary_data_temp->description,
					binary_data_temp->val,
					oe_user,
					binary_data_temp->datetime_str);
			}
		}
		binary_data_temp = binary_data_temp->next;
	}		
		// fprintf (output_file, "That's all, Folks! Reformatted csv generated using Tilmec version %s. Disclaimer: No warranty of anykind is expressed or implied.\n", TILMEC_VERSION);
		print_tilmec_version(output_file);
	printf ("Total:\n%d baris data analog sudah kumplit diproses.\n", k);
	printf ("%d baris data binary sudah juga kumplit diproses.\n", m);	
	if (fclose (output_file)!= 0)
	{
		printf
		("Error close file output %s, program dihentikan.\n",
			output_fnam);
		exit (-1);
	}
	return 0;
}



int analog_binary_joint_print_file_gold_mobile (struct kks_analog_trend_list* arg, struct binary_event_extractor* arg_bin)
{
	struct binary_event* binary_data_temp;
	struct binary_event* binary_data_temp_head;
	struct binary_event* binary_data_temp1;
	struct binary_event* binary_data_temp2;
	char* tanggal_from;
	char* tanggal_to;
	struct kks_analog_trend* list_content;
	struct kks_analog_trend_data* data_temp;
	struct kks_analog_trend_data* data_temp1;
	struct kks_analog_trend_data* data_dummy;
	struct kks_analog_trend_data* data_temp_head;
	struct kks_analog_trend_data_wrapper* all_data_head = NULL;
	struct kks_analog_trend_data_wrapper* wrapper_chunk;
	struct kks_analog_trend_data_wrapper* wrapper_chain = NULL;
	struct kks_analog_trend_data_wrapper* wrapper_kks;
	struct kks_analog_trend_data_wrapper* wrapper_temp;
	struct kks_analog_trend_data_wrapper* wrapper_chain1;
	struct kks_analog_trend_data_wrapper* wrapper_chain2;	
	struct tmline* timeline_head;
	struct tmline* timeline;
	time_t tanggal_sekarang;
	struct dcs_time* time_calc;
	struct dcs_time* time_calc1;
	struct autonote* note;
	char *note_buf, *note_buf1;
	// int print_pertama_kali = 1;
	int j, k, l, m;
	char* tanggal1;
	char oe_user[1024];
	char* tabel_null = "v";
	char* isi_tabel;
	int mulai = 0, list_mulai = 0;
	int data_ketemu = 0;
	int data_ketemu1, index;
	FILE* output_file;
	char* output_fnam = "OUTPUT_STARTUP_SEQUENCE_MOBILE.csv";
	char* tanggal;
	if (arg == NULL)
		return -1;
	if (arg->list_content == NULL)
		return -1;
	list_content = arg->list_content;
	while (list_content->prev != NULL)
		list_content = list_content->prev;
	timeline = arg->timeline;
		while (timeline->prev != NULL)
			timeline = timeline->prev;
	timeline_head = timeline;
	note = autonote_read_file();
	printf ("\nMulai memproses file output untuk Windows Phone %s... \n", output_fnam);
	while (list_content != NULL)
	{ /* tambah kolom, prev-next (kks)  */
		printf ("Mulai memproses KKS %s... ", list_content->data->kks);
		timeline = timeline_head;
		while (timeline != NULL)
		{ /* tambah anggota dalam satu kolom  , upr-lwr (waktu) */
			// printf ("debug timeline print %s\n", timeline->time_str);
			wrapper_chunk = kks_analog_trend_data_wrapper_new();
			if (mulai == 1)
				kks_analog_trend_data_wrapper_join
							(wrapper_chain, wrapper_chunk);
			if (mulai == 0)
			{
				mulai = 1;
				wrapper_chain = wrapper_chunk;
			}
			data_ketemu1 = 0;			
			data_temp = list_content->data;
			while (data_temp->prev != NULL)
				data_temp = data_temp->prev;
				data_temp1 = data_temp;
				// printf ("debug data_temp1 description %s\n", data_temp1->description);
			while (data_temp != NULL)
			{
				if (!strcmp (data_temp->time_str, timeline->time_str))
				{
					data_ketemu1 = 1;
				// printf ("debug data dummy %s\n", data_temp->val);
					wrapper_chunk->data = data_temp;
					break;
				}
				data_temp = data_temp->next;
			}

			if (data_ketemu1 == 0)
			{
				// printf ("debug timeline print data = NULL\n");
				data_dummy = kks_analog_trend_data_new();
				data_dummy->time_str = timeline->time_str;
				data_dummy->kks = data_temp1->kks;
				data_dummy->description = data_temp1->description;
				data_dummy->eng_unit = data_temp1->eng_unit;
				data_dummy->val = "v";
				wrapper_chunk->data = data_dummy;
			}

			timeline = timeline->next;
		}
		if (list_mulai == 1)
			kks_analog_trend_data_wrapper_add
				(all_data_head, wrapper_chain);
		if (list_mulai == 0)
		{
			list_mulai = 1;
			all_data_head = wrapper_chain;
		}
		mulai = 0;
		printf ("selesai.\n");
		list_content = list_content->next;
	}
	/* mulai mencetak... */
	wrapper_chunk = all_data_head;
	while (wrapper_chunk->prev != NULL)
		wrapper_chunk = wrapper_chunk->prev;
	wrapper_temp = wrapper_chunk;
	
	binary_data_temp = arg_bin->contents;
	while (binary_data_temp->prev != NULL)
		binary_data_temp = binary_data_temp->prev;
	binary_data_temp_head = binary_data_temp;
	binary_data_temp1 = binary_data_temp_head;
	binary_data_temp2 = binary_data_temp_head;
	while (binary_data_temp2->next!=NULL)
		binary_data_temp2 = binary_data_temp2->next;
	wrapper_chain1 = all_data_head;
	wrapper_chain2 = all_data_head;
	while (wrapper_chain2->lwr != NULL)
		wrapper_chain2 = wrapper_chain2->lwr;
	if (apakah_waktu_urut(binary_data_temp1->datetime_str, wrapper_chain1->data->time_str) == 1)
		tanggal_from = binary_data_temp1->datetime_str;
	else
		tanggal_from = wrapper_chain1->data->time_str;
	if (apakah_waktu_urut(binary_data_temp2->datetime_str, wrapper_chain2->data->time_str) == 1)
		tanggal_to = wrapper_chain2->data->time_str;
	else
		tanggal_to = binary_data_temp2->datetime_str;
	printf ("Mulai menulis di storage... \n");
	if ((output_file = fopen (output_fnam, "w")) == NULL)
	{
		printf ("Error operasi file. Hentikan program.\n");
		exit (-1);
	}
	printf ("Membuat header tabel data analog... ");	
	fprintf (output_file, "Sequence of Events;;From:;*%s;", tanggal_from);
	
	// fprintf (output_file, ",,,,,");
	/* membuat header KKS
	(untuk sumbu X adalah KKS, sumbu Y adalah waktu) */

	while (wrapper_chunk != NULL)
	{
		if (wrapper_chunk->data->kks == NULL)
			isi_tabel = tabel_null;
		else
			isi_tabel = wrapper_chunk->data->kks;
		fprintf (output_file, "%s;", isi_tabel);
		wrapper_chunk = wrapper_chunk->next;
	}
	fprintf (output_file, "\n");
	tanggal_sekarang = time(NULL);
	fprintf (output_file, "Created: %s;;To:;*%s;", ctime_hilangkan_newline(ctime(&tanggal_sekarang)), tanggal_to);
	
	// fprintf (output_file, ",,,,,");
	wrapper_chunk = wrapper_temp;
	while (wrapper_chunk != NULL)
	{
		if (wrapper_chunk->data->eng_unit == NULL)
			isi_tabel = tabel_null;
		else
			isi_tabel = wrapper_chunk->data->eng_unit;
		fprintf (output_file, "%s;", isi_tabel);
		wrapper_chunk = wrapper_chunk->next;
	}
	fprintf (output_file, "\n");
	fprintf (output_file, "Notes;Binary [v] Analog [>];TDVQI;DateTime;");
	// fprintf (output_file, ",,,,,");
	wrapper_chunk = wrapper_temp;
	index = 0;
	while (wrapper_chunk != NULL)
	{
		if (wrapper_chunk->data->description == NULL)
			isi_tabel = tabel_null;
		else
			isi_tabel = wrapper_chunk->data->description;
		fprintf (output_file, "[%d]%s;", index, isi_tabel);
		index++;
		wrapper_chunk = wrapper_chunk->next;
	}
	fprintf (output_file, "\n");
	printf ("selesai.\n");
	wrapper_chain = all_data_head;
	/*
	posisi di head.
	catat tanggal
	tulis tanggal di file dan beri tanda koma
	catat nilai
	tulis nilai di file dan beri tanda koma
	next kks
	cari apakah tanggal cocok (upr_lwr)
	jika tanggal cocok tulis nilai di file dan beri tanda koma
	jika tanggal tidak ada yang cocok maka bari tanda koma saja
	next kks... (sampai next kks NULL)
	cari apakah tanggal cocok (upr_lwr)
	jika tanggal cocok tulis nilai di file dan beri tanda koma
	jika tanggal tidak ada yang cocok maka bari tanda koma saja
	*/

	printf ("Membuat tabel data analog...\n");
	j = 0;
	k = 0;
	l = 0;
	m = 0;
	while (wrapper_chain != NULL)
	{
		wrapper_kks = all_data_head;
		// binary_data_temp = binary_data_temp_head;
		tanggal = wrapper_chain->data->time_str;

		time_calc = dcs_time_new();
		time_calc1 = dcs_time_new();	
		dcs_time_set_timestr_1(time_calc, tanggal);		
		if (wrapper_chain->lwr!=NULL)
		{
			tanggal1 = wrapper_chain->lwr->data->time_str;
			dcs_time_set_timestr_1(time_calc1, tanggal1);
		}
		else
			tanggal1 = NULL;
		
		// binary_data_temp = binary_data_temp_head;
		while (binary_data_temp!=NULL)
		{
			// printf ("Debug analog1: %s binary: %s analog2: %s\n",
				// tanggal, binary_data_temp->datetime_str, tanggal1);
			
			// printf ("Debug sisip %s %s %s\n", 
			//	binary_data_temp->datetime_str,
			//	binary_data_temp->kks,
			//	binary_data_temp->description);
			dcs_time_set_timestr_2(time_calc, binary_data_temp->datetime_str);
			dcs_time_get_diff_time(time_calc);
			dcs_time_set_timestr_2(time_calc1, binary_data_temp->datetime_str);
			
			if (dcs_time_get_status(time_calc)== 1)
			{
				//printf ("Debug scan tanggal continue %s %s\n",
				//	tanggal, binary_data_temp->datetime_str
				//);
				break;
			}

			{
				if (l == 0)
				{
					printf ("Menyisipkan event binary...\n");
					l = 1;
				}
				m++;
				memset (oe_user, 0, 1024);

				if (binary_data_temp->seqmod == BINARY_OPERATION_SEQUENCE)
				{
					strcpy (oe_user, binary_data_temp->evt_type);
					strcat (oe_user, "/");
					strcat (oe_user, binary_data_temp->usern);
				}
				else
				{
					strcpy (oe_user, binary_data_temp->evt_type);
				}
				note_buf = autonote_find_note(note, binary_data_temp->kks, binary_data_temp->val);
				if (note_buf == NULL)
					note_buf1 = "";
				else
					note_buf1 = note_buf;
				if (binary_data_temp->val_qlty!= NULL)
				{
					fprintf (output_file, "%s;%s;{%s} {%s} {%s} {%s} {%d};*%s;\n",
						note_buf1,
						binary_data_temp->kks,
						oe_user,
						binary_data_temp->description,
						binary_data_temp->val,
						binary_data_temp->val_qlty,
						binary_data_temp->index,
						binary_data_temp->datetime_str
						);					
				}
				else
				{
					fprintf (output_file, "%s;%s;{%s} {%s} {%s} {%d};*%s;\n",
						note_buf1,
						binary_data_temp->kks,
						oe_user,
						binary_data_temp->description,
						binary_data_temp->val,
						binary_data_temp->index,
						binary_data_temp->datetime_str
						);					
				}
			}
			if (tanggal1!=NULL)
			{
				dcs_time_get_diff_time(time_calc1);
				if (dcs_time_get_status(time_calc1)  > 0)
				{
					// printf ("Debug scan tanggal break\n");
					break;
				}
			}			
			binary_data_temp = binary_data_temp->next;
		}		
		if (j == 1000)
		{
			j = 0;
			printf ("%d baris data analog sudah selesai. Lanjutkan...\n", k);
			if (l == 1)
				l = 0;
		}
		///////////////////////////////////////////////
		fprintf (output_file, ";;;*%s;", tanggal);
		while (wrapper_kks != NULL)
		{
			wrapper_chunk = wrapper_kks;
			while (wrapper_chunk != NULL)
			{
				if (!strcmp(wrapper_chunk->data->time_str, tanggal))
				{
					fprintf (output_file,"%s;", wrapper_chunk->data->val);
					data_ketemu = 1;
					break;
				}
				wrapper_chunk = wrapper_chunk->lwr;
			}
			if (data_ketemu != 1)
				fprintf (output_file, ";");
			data_ketemu = 0;
			wrapper_kks = wrapper_kks->next;
		}
		fprintf(output_file, "\n");
		//////////////////////////////////////////////

		wrapper_chain = wrapper_chain->lwr;
		j++; k++;
	}
	while (binary_data_temp!=NULL)
	{
		{
			// printf ("debug sisipkan event binary\n");
			if (l == 0)
			{
				printf ("Menyisipkan event binary...\n");
				l = 1;
			}
			m++;
			memset (oe_user, 0, 1024);
			if (binary_data_temp->seqmod == BINARY_OPERATION_SEQUENCE)
			{
				strcpy (oe_user, binary_data_temp->evt_type);
				strcat (oe_user, "/");
				strcat (oe_user, binary_data_temp->usern);
			}
			else
			{
				strcpy (oe_user, binary_data_temp->evt_type);
			}
			note_buf = autonote_find_note(note, binary_data_temp->kks, binary_data_temp->val);
			if (note_buf==NULL)
				note_buf1 = "";
			else
				note_buf1 = note_buf;
				if (binary_data_temp->val_qlty!= NULL)
				{
					fprintf (output_file, "%s;%s;{%s} {%s} {%s} {%s} {%d};*%s;\n",
						note_buf1,
						binary_data_temp->kks,
						oe_user,
						binary_data_temp->description,
						binary_data_temp->val,
						binary_data_temp->val_qlty,
						binary_data_temp->index,
						binary_data_temp->datetime_str
						);					
				}
				else
				{
					fprintf (output_file, "%s;%s;{%s} {%s} {%s} {%d};*%s;\n",
						note_buf1,
						binary_data_temp->kks,
						oe_user,
						binary_data_temp->description,
						binary_data_temp->val,
						binary_data_temp->index,
						binary_data_temp->datetime_str
						);					
				}

		}
		binary_data_temp = binary_data_temp->next;
	}		
		// fprintf (output_file, "That's all, Folks! Reformatted csv generated using Tilmec version %s. Disclaimer: No warranty of anykind is expressed or implied.\n", TILMEC_VERSION);
		print_tilmec_version(output_file);
	printf ("Total:\n%d baris data analog sudah kumplit diproses.\n", k);
	printf ("%d baris data binary sudah juga kumplit diproses.\n", m);	
	if (fclose (output_file)!= 0)
	{
		printf
		("Error close file output %s, program dihentikan.\n",
			output_fnam);
		exit (-1);
	}
	return 0;
}

