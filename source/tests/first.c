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
		*	udata	: build
		+	folder	: text
			+	folder	: marina
		+	folder	: cg
			+	marina
				*	udata	: idle
				*	udata	: stand
				*	udata	: walk
	*/
	KOSUZU_SAVEENTRY entries[32] = {};
	KOSUZU_SAVEQUEUE queue = {};

	kosuzu_savequeue_setup(&queue,entries,32);
	kosuzu_savequeue_addFolder(&queue,"\\","cg");
	kosuzu_savequeue_addFolder(&queue,"\\","text");
	kosuzu_savequeue_addFolder(&queue,"\\cg\\","marina");
	kosuzu_savequeue_addFolder(&queue,"\\text\\","marina");
	kosuzu_savequeue_addFile(&queue,"\\cg\\marina\\","idle","workdata\\mrn_idle.gif");
	kosuzu_savequeue_addFile(&queue,"\\cg\\marina\\","stand","workdata\\mrn_stand.gif");
	kosuzu_savequeue_addFile(&queue,"\\cg\\marina\\","walk","workdata\\mrn_walk.gif");
	kosuzu_savequeue_addFile(&queue,"\\","build","build.lua");
	kosuzu_savequeue_addUint(&queue,"\\","width",0x0A55DEAD);
	kosuzu_savequeue_addUint(&queue,"\\","height",0xDEADA55);

	kosuzu_savequeue_saveFile(&queue,"data\\testF.ksz");
}
static void read() {
	KOSUZU_ARCHIVE archive;
	if(!kosuzu_archiveOpenFile(&archive,"data\\testF.ksz")) {
		puts("test 'first' failed: couldn't open archive");
		exit(-1);
		return;
	}

	/* read da gifs -------------------------------------*/
	kosuzu_archiveChdir(&archive,NULL);
	kosuzu_archiveChdir(&archive,"cg\\marina");

	const char *filenames[2][3] = {
		{ "idle","stand","walk" },
		{ 
			"data\\testF_idle.gif",
			"data\\testF_stand.gif",
			"data\\testF_walk.gif"
		}
	};

	for(int f=0; f<3; f++) {
		const char *src_name = filenames[0][f];
		const char *out_filename = filenames[1][f];
		KOSUZU_FILE *img_file = kosuzu_archiveFileOpen(&archive,src_name);
		if(img_file) {
			FILE *out_file = fopen(out_filename,"wb");
			if(!out_file) {
				puts("test 'first' failed: couldn't open output file");
				exit(-1);
			}

			char dat_buf;
			for(size_t i=0; i<img_file->file_size; i++) {
				kosuzu_file_read(img_file,&dat_buf,sizeof(char));
				fwrite(&dat_buf,sizeof(char),1,out_file);
			}
			fclose(out_file);
			kosuzu_file_close(img_file);
		}
	}
	
	/* read a number ------------------------------------*/
	kosuzu_archiveChdir(&archive,NULL);
	const KOSUZU_NODE *num_node = kosuzu_archiveNodeFind(&archive,"height");
	if(num_node) {
		printf("number: %08Xh\n",num_node->d.value_uint);
	}

	/* close archive ------------------------------------*/
	kosuzu_archiveClose(&archive);
}

