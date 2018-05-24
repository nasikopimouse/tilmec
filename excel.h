#ifndef TREND_TO_EXCEL_H
#define TREND_TO_EXCEL_H

#include "att_word_extractor.h"
#include "att_kks_analog_trend_extractor.h"
#include "att_kks_binary_event_extractor.h"
#include "dcs_time.h"
#include "att.h"
#include "att_word_extractor_main.h"

#define INDIVIDUAL_FILE		69
#define NO_INDIVIDUAL_FILE	70
#define SUKSES				0
#define FILE_ANALOG_TIDAK_VALID	1
#define FILE_BINARY_TIDAK_VALID	3
#define FILE_ANALOG_BINARY_TIDAK_VALID 4

/*
Jika status 0 sukses.
Jika status 1 file analog tidak valid.
Jika status 3 file binary tidak valid.
Jika status 4 kedua file tidak valid.
*/


int output_excel (char*, char*, int);

#endif
