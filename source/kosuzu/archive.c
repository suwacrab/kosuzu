#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "kosuzu.h"

/* main functions -----------------------------------------------------------*/
int kosuzu_archiveOpen(KOSUZU_ARCHIVE *archive,FILE *file_ptr) {
	if(!archive) return false;
	if(!file_ptr) return false;

	/* setup archive info -------------------------------*/
	KOSUZU_FILEHEADER file_header = {};
	fseek(file_ptr,0,SEEK_SET);
	fread(&file_header,sizeof(KOSUZU_FILEHEADER),1,file_ptr);

	archive->node_count = file_header.node_count;
	archive->tree_count = file_header.tree_count;
	archive->fldr_current = 0;
	archive->data_offset = file_header.offset_data;
	archive->file_ptr = file_ptr;
	archive->did_openFile = false;

	/* allocate stuff -----------------------------------*/
	archive->data_nodes = malloc(sizeof(KOSUZU_FILENODE) * archive->node_count);
	archive->data_trees = malloc(sizeof(uint16_t) * archive->tree_count);

	fseek(file_ptr,file_header.offset_nodes,SEEK_SET);
	fread(archive->data_nodes,sizeof(KOSUZU_FILENODE),archive->node_count,file_ptr);
	fseek(file_ptr,file_header.offset_trees,SEEK_SET);
	fread(archive->data_trees,sizeof(uint16_t),archive->tree_count,file_ptr);

	return true;
}
int kosuzu_archiveOpenFile(KOSUZU_ARCHIVE *archive,const char *src_filename) {
	if(!archive) return false;
	FILE *file_ptr = fopen(src_filename,"rb");
	if(!file_ptr) return false;

	kosuzu_archiveOpen(archive,file_ptr);
	archive->did_openFile = true;
	return true;
}
int kosuzu_archiveClose(KOSUZU_ARCHIVE *archive) {
	if(!archive) return false;

	free(archive->data_nodes);
	free(archive->data_trees);
	
	if(archive->did_openFile) {
		fclose(archive->file_ptr);
	}

	return true;
}

/* node functions -----------------------------------------------------------*/
const KOSUZU_FILENODE *kosuzu_archiveNodeGetCurFldr(KOSUZU_ARCHIVE *archive) {
	if(!archive) return NULL;
	return &archive->data_nodes[archive->fldr_current];
}
const KOSUZU_FILENODE *kosuzu_archiveNodeGet(KOSUZU_ARCHIVE *archive,size_t index) {
	if(!archive) return NULL;
	if(index >= archive->node_count) return NULL;
	if(index < 0) return NULL;
	return &archive->data_nodes[index];
}
int kosuzu_archiveNodeFind(KOSUZU_ARCHIVE *archive,const char *name) {
	if(!archive) return -1;

	const uint32_t name_hash = kosuzu_hashString(name);
	const KOSUZU_FILENODE *cur_fldr = kosuzu_archiveNodeGetCurFldr(archive);

	// loop through tree
	const size_t fldr_size = cur_fldr->d.folder_size;
	if(fldr_size == 0) return -1;
	for(int i=0; i<fldr_size; i++) {
		const size_t node_index = archive->data_trees[
			cur_fldr->d.folder_treeindex + i
		];
		const KOSUZU_FILENODE *node = kosuzu_archiveNodeGet(archive,node_index);
		if(node->name_hash == name_hash) {
			return node_index;
		}
	}

	return -1;
}

/* folder functions ---------------------------------------------------------*/
int kosuzu_archiveChdir(KOSUZU_ARCHIVE *archive,const char *dir_name) {
	if(!archive) return false;

	/* if dir_name is null, reset cwd to root. ----------*/
	if(dir_name == NULL) {
		archive->fldr_current = 0;
		return true;
	}

	/* otherwise, set cwd like usual. -------------------*/
	char name_buffer[256] = {};
	size_t out_index = 0;
	size_t inp_index = 0;

	const int orig_folder = archive->fldr_current;

	while(out_index < 256) {
		char cur_chara = dir_name[inp_index++];
		if(cur_chara == '\\' || cur_chara == '\0') {
			name_buffer[out_index++] = '\0';
			const int node_index = kosuzu_archiveNodeFind(archive,name_buffer);
			if(node_index != -1) {
				const KOSUZU_FILENODE *node = kosuzu_archiveNodeGet(archive,node_index);
				if(node->node_type == KOSUZU_NODETYPE_FOLDER) {
					archive->fldr_current = node_index;
					printf("changed directory -> %s\n",name_buffer);
				}
			} else {
				archive->fldr_current = orig_folder;
				return false;
			}
			out_index = 0;
			if(cur_chara == '\0') return true;
		} else {
			name_buffer[out_index++] = cur_chara;
		}
	}

	archive->fldr_current = orig_folder;
	return false;
}
const KOSUZU_FILENODE *kosuzu_archiveFileSeek(KOSUZU_ARCHIVE *archive,const char *name) {
	if(!archive) return NULL;

	const int node_index = kosuzu_archiveNodeFind(archive,name);
	if(node_index != -1) {
		const KOSUZU_FILENODE *node = kosuzu_archiveNodeGet(archive,node_index);
		switch(node->node_type) {
			case KOSUZU_NODETYPE_USERDATA: {
				FILE *file = archive->file_ptr;
				fseek(file,
					node->d.udata_offset + archive->data_offset,
					SEEK_SET
				);
				break;
			}
			default: { break; }
		}
		return node;
	}
	return NULL;
}

