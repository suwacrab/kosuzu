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
	rewind(file_ptr);
	fread(&file_header,sizeof(KOSUZU_FILEHEADER),1,file_ptr);

	archive->node_count = file_header.node_count;
	archive->tree_count = file_header.tree_count;
	archive->fldr_current = 0;
	archive->data_offset = file_header.offset_data;
	archive->file_ptr = file_ptr;
	archive->did_openFile = false;

	for(size_t i=0; i<KOSUZU_FILE_MAXOPEN; i++) {
		archive->file_list[i].is_open = false;
	}

	/* allocate stuff -----------------------------------*/
	KOSUZU_NODE *data_nodes = NULL;
	uint16_t *data_trees = NULL;
	data_nodes = malloc(sizeof(KOSUZU_NODE) * archive->node_count);
	data_trees = malloc(sizeof(uint16_t) * archive->tree_count);

	fseek(file_ptr,file_header.offset_trees,SEEK_SET);
	fread(data_trees,sizeof(uint16_t),archive->tree_count,file_ptr);

	fseek(file_ptr,file_header.offset_nodes,SEEK_SET);
	for(size_t i=0; i<archive->node_count; i++) {
		KOSUZU_NODE *out_node = &data_nodes[i];
		KOSUZU_FILENODE src_node = {};
		fread(&src_node,sizeof(src_node),1,file_ptr);
		out_node->name_hash = src_node.name_hash;
		out_node->node_type = src_node.node_type;
		out_node->node_index = i;
		memcpy(&out_node->d,&src_node.d,sizeof(out_node->d));
	}

	archive->data_nodes = data_nodes;
	archive->data_trees = data_trees;

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
const KOSUZU_NODE *kosuzu_archiveNodeGetCurFldr(KOSUZU_ARCHIVE *archive) {
	if(!archive) return NULL;
	return &archive->data_nodes[archive->fldr_current];
}
const KOSUZU_NODE *kosuzu_archiveNodeGet(KOSUZU_ARCHIVE *archive,size_t index) {
	if(!archive) return NULL;
	if(index >= archive->node_count) return NULL;
	if(index < 0) return NULL;
	return &archive->data_nodes[index];
}
const KOSUZU_NODE *kosuzu_archiveNodeFind(KOSUZU_ARCHIVE *archive,const char *name) {
	if(!archive) return NULL;

	const uint32_t name_hash = kosuzu_hashString(name);
	const KOSUZU_NODE *cur_fldr = kosuzu_archiveNodeGetCurFldr(archive);

	/* loop through tree --------------------------------*/
	const size_t fldr_size = cur_fldr->d.folder_size;
	for(int i=0; i<fldr_size; i++) {
		const size_t node_index = archive->data_trees[
			cur_fldr->d.folder_treeindex + i
		];
		const KOSUZU_NODE *node = kosuzu_archiveNodeGet(archive,node_index);
		if(node->name_hash == name_hash) {
			return node;
		}
	}

	return NULL;
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
			const KOSUZU_NODE *node = kosuzu_archiveNodeFind(archive,name_buffer);
			if(node && node->node_type == KOSUZU_NODETYPE_FOLDER) {
				archive->fldr_current = node->node_index;
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
KOSUZU_FILE *kosuzu_archiveFileOpen(KOSUZU_ARCHIVE *archive,const char *name) {
	if(!archive) return NULL;

	/* search for available file handle -----------------*/
	KOSUZU_FILE *file = NULL;
	for(size_t i=0; i<KOSUZU_FILE_MAXOPEN; i++) {
		KOSUZU_FILE *cur_file = &archive->file_list[i];
		if(!cur_file->is_open) {
			file = cur_file;
			break;
		}
	}
	if(!file) return NULL;

	/* search for node ----------------------------------*/
	const KOSUZU_NODE *node = kosuzu_archiveNodeFind(archive,name);
	if(node) {
		switch(node->node_type) {
			case KOSUZU_NODETYPE_USERDATA: {
				FILE *main_file = archive->file_ptr;
				fseek(main_file,
					node->d.udata_offset + archive->data_offset,
					SEEK_SET
				);
				break;
			}
			default: { break; }
		}
		file->is_open = true;
		file->archive_ptr = archive;
		file->file_node = node;
		file->file_offset = 0;
		file->file_size = node->d.udata_size;
		return file;
	}
	return NULL;
}

