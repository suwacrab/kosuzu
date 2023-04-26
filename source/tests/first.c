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
	/*
	KOSUZU_SAVEENTRY entries[] = {
		{
			.name = "width", .arc_path = "\\",
			.out_type = KOSUZU_NODETYPE_INT,
			.i.value_sint = 0x0A55DEAD
		},
		{
			.name = "height", .arc_path = "\\",
			.out_type = KOSUZU_NODETYPE_INT,
			.i.value_sint = 0x0DEADA55
		},
		{ 
			.name = "text", .arc_path = "\\",
			.out_type = KOSUZU_NODETYPE_FOLDER,
		},
		{ 
			.name = "marina", .arc_path = "\\text\\",
			.out_type = KOSUZU_NODETYPE_FOLDER,
		},
		{ 
			.name = "cg", .arc_path = "\\",
			.out_type = KOSUZU_NODETYPE_FOLDER,
		},
		{ 
			.name = "marina", .arc_path = "\\cg\\",
			.out_type = KOSUZU_NODETYPE_FOLDER,
		},
		{
			.name = "idle", .arc_path = "\\cg\\marina\\",
			.is_file = true, .out_type = KOSUZU_NODETYPE_USERDATA,
			.i.src_filename = "workdata\\mrn_idle.gif"
		},
		{
			.name = "stand", .arc_path = "\\cg\\marina\\",
			.is_file = true, .out_type = KOSUZU_NODETYPE_USERDATA,
			.i.src_filename = "workdata\\mrn_stand.gif"
		},
		{
			.name = "walk", .arc_path = "\\cg\\marina\\",
			.is_file = true, .out_type = KOSUZU_NODETYPE_USERDATA,
			.i.src_filename = "workdata\\mrn_walk.gif"
		},
		{
			.name = "build", .arc_path = "\\",
			.is_file = true, .out_type = KOSUZU_NODETYPE_USERDATA,
			.i.src_filename = "build.lua"
		}
	};
	*/

	kosuzu_saveFile("data\\testF.ksz",entries,
		sizeof(entries) / sizeof(KOSUZU_SAVEENTRY)
	);
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
		const char *src_filename = filenames[0][f];
		const char *out_filename = filenames[1][f];
		const KOSUZU_NODE *img_node = kosuzu_archiveFileSeek(&archive,src_filename);
		if(img_node) {
			FILE *out_file = fopen(out_filename,"wb");
			if(!out_file) {
				puts("test 'first' failed: couldn't open output file");
				exit(-1);
			}

			char dat_buf;
			for(size_t i=0; i<img_node->d.udata_size; i++) {
				fread(&dat_buf,sizeof(char),1,archive.file_ptr);
				fwrite(&dat_buf,sizeof(char),1,out_file);
			}
			fclose(out_file);
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

