#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "kosuzu.h"
#include "tests.h"

static void write();
static void read();

/* functions ----------------------------------------------------------------*/
void kosuzutest_first() {
	write();
	read();
	puts("first test complete.");
}

static void write() {
	/*
		*	creates an archive with the follwing hierarchy:
		-	int		: width		0x0A55DEAD
		-	int		: height	0x0DEADA55
		+	folder	: text
			*	udata	: message
		+	folder	: cg
			+	kosuzu
				*	udata	: cucumber
				*	udata	: cucumber_orig
			+	marina
				*	udata	: idle
				*	udata	: stand
				*	udata	: walk
	*/
	const size_t ENTRY_MAX = 64;
	KOSUZU_SAVEENTRY entries[ENTRY_MAX];
	KOSUZU_SAVEQUEUE queue = {};

	kosuzu_savequeue_setup(&queue,entries,ENTRY_MAX);
	kosuzu_savequeue_addFolder(&queue,"\\","text");
	kosuzu_savequeue_addFolder(&queue,"\\","cg");
	kosuzu_savequeue_addFolder(&queue,"\\cg\\","marina");
	kosuzu_savequeue_addFolder(&queue,"\\cg\\","kosuzu");
	kosuzu_savequeue_addFile(&queue,"\\cg\\marina\\","idle","workdata\\mrn_idle.gif");
	kosuzu_savequeue_addFile(&queue,"\\cg\\marina\\","stand","workdata\\mrn_stand.gif");
	kosuzu_savequeue_addFile(&queue,"\\cg\\marina\\","walk","workdata\\mrn_walk.gif");
	kosuzu_savequeue_addFile(&queue,"\\cg\\kosuzu\\","cucumber","workdata\\cucumber.bmp");
	kosuzu_savequeue_addFile(&queue,"\\cg\\kosuzu\\","cucumber_orig","workdata\\cucumber_orig.png");
	kosuzu_savequeue_addUdata(&queue,"\\text\\","message","hello",6);
	kosuzu_savequeue_addUint(&queue,"\\","width",0x0A55DEAD);
	kosuzu_savequeue_addUint(&queue,"\\","height",0xDEADA55);

	kosuzu_savequeue_saveFile(&queue,"data\\testF.ksz");
	puts("[test 'first']: .ksz saved.");
}
static void read() {
	KOSUZU_RECORD archive;
	if(!kosuzu_recordOpenFile(&archive,"data\\testF.ksz")) {
		puts("test 'first' failed: couldn't open archive");
		exit(-1);
		return;
	}

	/* read da gifs -------------------------------------*/
	kosuzu_recordChdir(&archive,NULL);
	kosuzu_recordChdir(&archive,"cg\\marina");

	const char *filenames[2][3] = {
		{ "idle","stand","walk" },
		{ 
			"data\\testF_idle.gif",
			"data\\testF_stand.gif",
			"data\\testF_walk.gif"
		}
	};

	{	// check if entries exist
		KOSUZU_NODECHECK_ENTRY check_list[] = {
			{"idle",	KOSUZU_NODETYPE_USERDATA},
			{"stand",	KOSUZU_NODETYPE_USERDATA},
			{"walk",	KOSUZU_NODETYPE_USERDATA},
			{NULL,0}
		};
		int check_result = kosuzu_record_nodeCheck(&archive,check_list);
		if(check_result >= 0) {
			printf("[test 'first']: failed: node '%s' missing/incorrect\n",
				check_list[check_result].name
			);
			exit(-1);
		}
	}

	for(int f=0; f<3; f++) {
		const char *src_name = filenames[0][f];
		const char *out_filename = filenames[1][f];
		KOSUZU_FILE *img_file = kosuzu_record_fileOpen(&archive,src_name);
		if(img_file) {
			FILE *out_file = fopen(out_filename,"wb");
			if(!out_file) {
				puts("[test 'first']: failed: couldn't open output file");
				exit(-1);
			}

			char dat_buf;
			while(kosuzu_file_read(img_file,&dat_buf,sizeof(char)) == 1) {
				fwrite(&dat_buf,sizeof(char),1,out_file);
			}
			fclose(out_file);
			kosuzu_file_close(img_file);
		}
	}
	
	/* read a number ------------------------------------*/
	kosuzu_recordChdir(&archive,NULL);
	const KOSUZU_NODE *num_node = kosuzu_recordNodeFind(&archive,"height");
	if(num_node) {
		printf("number: %08Xh\n",num_node->d.value_uint);
	} else {
		puts("test 'first' failed: number node not found");
		exit(-1);
	}

	/* read a string ------------------------------------*/
	kosuzu_recordChdir(&archive,NULL);
	kosuzu_recordChdir(&archive,"text");
	KOSUZU_FILE *text_file = kosuzu_record_fileOpen(&archive,"message");
	if(text_file) {
		char str_buf[128] = {};
		kosuzu_file_read(text_file,str_buf,text_file->file_size);
		kosuzu_file_close(text_file);

		printf("message: '%s'\n",str_buf);
	} else {
		puts("test 'first' failed: message node not found");
		exit(-1);
	}

	/* close archive ------------------------------------*/
	kosuzu_recordClose(&archive);
}

