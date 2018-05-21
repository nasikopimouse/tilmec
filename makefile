###############################################################################
#  *** Makefile TILMEC.exe ***
#  *** Integrated Utilities for Main-Plant Engineers and Unit-Controllers ***
###############################################################################

PROJECT = TILMEC
CC = "C:\BCC101\bin\bcc32c.exe"
LNK = "C:\BCC101\bin\ilink32.exe"
SOURCE_DIR = "C:\Users\jambubiji\Documents\att_word_extractor"
TARGET_DIR = "C:\Users\jambubiji\Desktop\output"
LIB_DIR_D = "C:\BCC101\lib\win32c\debug"
LIB_DIR_R = "C:\BCC101\lib\win32c\release"
# CCPARAMS = -c -v -3
CCPARAMS = -c -v
LNKPARAMS = -c -v
LIB = $(LIB_DIR_D)\cw32.lib $(LIB_DIR_D)\import32.lib
STARTUP = $(LIB_DIR_D)\c0x32.obj
OBJECTFILES = att.obj att_kks_binary_event_extractor.obj \
					att_kks_analog_trend_extractor.obj \
					dcs_time.obj droptest.obj att_word_extractor.obj \
					att_word_extractor_main.obj cmd_param.obj excel.obj \
					fnam_handling.obj multi_file.obj autonotes.obj tilmec_version.obj
EXE_FILE_NAME = $(TARGET_DIR)\tilmec.exe
DEBUG_FILES = tilmec

$(EXE_FILE_NAME): $(OBJECTFILES)
	$(LNK) $(LNKPARAMS) $(STARTUP) $(OBJECTFILES), $(EXE_FILE_NAME),,$(LIB),,

tilmec_version.obj: tilmec_version.c \
					tilmec_version.h
	$(CC) $(CCPARAMS) tilmec_version.c	
	
att_word_extractor_main.obj: att_word_extractor_main.c \			
							att_word_extractor_main.h \
							att_word_extractor.h att_kks_analog_trend_extractor.h \
							att_kks_binary_event_extractor.h dcs_time.h att.h \
							cmd_param.h tilmec_version.h \
							droptest.h excel.h fnam_handling.h multi_file.h
	$(CC) $(CCPARAMS) att_word_extractor_main.c

fnam_handling.obj: fnam_handling.c \
					fnam_handling.h att_word_extractor.h \
					att_kks_analog_trend_extractor.h \
					att_kks_binary_event_extractor.h
					
	$(CC) $(CCPARAMS) fnam_handling.c

multi_file.obj: multi_file.c \
				multi_file.h att_word_extractor.h \
				att_kks_analog_trend_extractor.h \
				att_kks_binary_event_extractor.h \
				dcs_time.h autonotes.h tilmec_version.h
	$(CC) $(CCPARAMS) multi_file.c
	
droptest.obj: droptest.c \
				att_word_extractor_main.h \
				droptest.h \
				att_kks_analog_trend_extractor.h \
				att_word_extractor.h
	$(CC) $(CCPARAMS) droptest.c

excel.obj: excel.c excel.h att_word_extractor.h \
			att_kks_analog_trend_extractor.h \
			dcs_time.h \
			att_word_extractor_main.h \
			att_kks_binary_event_extractor.h \
			fnam_handling.h multi_file.h
	$(CC) $(CCPARAMS) excel.c

cmd_param.obj: cmd_param.c cmd_param.h \
				att_word_extractor_main.h
	$(CC) $(CCPARAMS) cmd_param.c
		
att.obj: att.c att.h dcs_time.h \
			att_kks_analog_trend_extractor.h \
			att_kks_binary_event_extractor.h \
			att_word_extractor_main.h
	$(CC) $(CCPARAMS) att.c

att_kks_binary_event_extractor.obj: att_kks_binary_event_extractor.c \
									att_kks_binary_event_extractor.h \
									att_word_extractor.h \
									att_kks_analog_trend_extractor.h \
									att_word_extractor_main.h tilmec_version.h
	$(CC) $(CCPARAMS) att_kks_binary_event_extractor.c

att_kks_analog_trend_extractor.obj: att_kks_analog_trend_extractor.c \
									att_kks_analog_trend_extractor.h \
									att_word_extractor.h \
									att_word_extractor_main.h \
									droptest.h tilmec_version.h \
									dcs_time.h
	$(CC) $(CCPARAMS) att_kks_analog_trend_extractor.c
	
att_word_extractor.obj: att_word_extractor.c \
						att_word_extractor.h \
						att_word_extractor_main.h
	$(CC) $(CCPARAMS) att_word_extractor.c

dcs_time.obj: dcs_time.c \
				att_word_extractor_main.h \
				dcs_time.h
	$(CC) $(CCPARAMS) dcs_time.c

autonotes.obj: autonotes.c \
				autonotes.h att_word_extractor.h \
				att_kks_analog_trend_extractor.h
	$(CC) $(CCPARAMS) autonotes.c
	

clean:
	erase $(SOURCE_DIR)\*.obj
	erase $(TARGET_DIR)\$(DEBUG_FILES).*
#	erase $(TARGET_DIR)\$(EXE_FILE_NAME)
