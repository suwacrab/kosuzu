#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdbool>

#include <vector>
#include <string>

#include "kosuzu.h"

#define OFFSET_ALIGNMENT (0x100)

static uint32_t align_offset(size_t offset);

/* types --------------------------------------------------------------------*/
typedef struct FOLDER_INFO {
	size_t id;		// id in folder list
	int entry_id;	// id in entry array (-1 if root)
	uint32_t name_hash;	// hashed ver. of local path (ex. [b])
	uint32_t path_hash;	// hashed ver. of absolute path (ex. [root\a\b\])
	size_t block_count;
} FOLDER_INFO;
typedef struct FOLDER_BLOCKS {
	const KOSUZU_SAVEENTRY *entries[256];
} FOLDER_BLOCKS;

/* misc functions -----------------------------------------------------------*/
static uint32_t align_offset(size_t offset) {
	return OFFSET_ALIGNMENT * (((OFFSET_ALIGNMENT-1) + offset) / OFFSET_ALIGNMENT);
}

/* functions ----------------------------------------------------------------*/
int kosuzu_save(void **out_buffer, const KOSUZU_SAVEENTRY entries[],const size_t entry_count) {
	KOSUZU_SAVEINFO info = {
		.out_buffer = out_buffer,
		.entries = entries,
		.entry_count = entry_count,
		.out_filename = NULL
	};

	return kosuzu_saveEX(&info);
}
int kosuzu_saveFile(const char *out_filename,const KOSUZU_SAVEENTRY entries[],const size_t entry_count) {
	KOSUZU_SAVEINFO info = {
		.out_buffer = NULL,
		.entries = entries,
		.entry_count = entry_count,
		.out_filename = out_filename
	};

	//info.flag_verbose = true;

	return kosuzu_saveEX(&info);
}
int kosuzu_saveEX(const KOSUZU_SAVEINFO *save_info) {
	if(save_info == NULL) {
		printf("kosuzu: save error: NULL save info\n");
		return false;
	}
	const auto entries = save_info->entries;
	const auto entry_count = save_info->entry_count;
	const auto out_buffer = save_info->out_buffer;

	if(entries == NULL) {
		std::puts("kosuzu: save error: save info has null entries");
		return false;
	}
	if(entry_count == 0) {
		std::puts("kosuzu: save error: save info has 0 entries");
		return false;
	}
	if(entry_count > 0x7FFF) {
		std::puts("kosuzu: save error: save info has too many entries");
		return false;
	}

	/* setup folder list --------------------------------*/
	std::vector<FOLDER_INFO> folder_infolist;
	std::vector<FOLDER_BLOCKS> folder_blocklist;

	auto entry_getFolder = [&](const int entry_id) {
		for(int i=0; i<folder_infolist.size(); i++) {
			auto folder = &folder_infolist.at(i);
			if(folder->entry_id == entry_id) return folder;
		}
		return (FOLDER_INFO*)NULL;
	};

	// add root folder
	{
		FOLDER_INFO info = {
			.id = folder_infolist.size(),
			.entry_id = -1,
			.name_hash = kosuzu_hashString("root"),
			.path_hash = kosuzu_hashString("root\\"),
			.block_count = 0,
		};
		FOLDER_BLOCKS blocks = {};
		folder_infolist.push_back(info);
		folder_blocklist.push_back(blocks);
	}

	// add other folders
	for(int i=0; i<entry_count; i++) {
		const auto &entry = entries[i];
		if(entry.out_type == KOSUZU_NODETYPE_FOLDER) {
			std::string path = "root";
			path.append(entry.arc_path);
			path.append(entry.name);
			path.append("\\");
			FOLDER_INFO info = {
				.id = folder_infolist.size(),
				.entry_id = i,
				.name_hash = kosuzu_hashString(entry.name),
				.path_hash = kosuzu_hashString(path.c_str()),
				.block_count = 0,
			};
			FOLDER_BLOCKS blocks = {};
			folder_infolist.push_back(info);
			folder_blocklist.push_back(blocks);
		};
	}

	/* assign blocks to all folders -----------------------------------------*/
	for(size_t ent_id = 0; ent_id < entry_count; ent_id++) {
		const auto &entry = entries[ent_id];
		
		/* find folder ----------------------------------*/
		FOLDER_INFO *cur_folder = NULL;
		std::string cur_path = "root";
		cur_path.append(entry.arc_path);
		auto cur_path_hash = kosuzu_hashString(cur_path.c_str());

		//printf("entry %2d a:%08Xh '%s'\n",ent_id,cur_path_hash,entry.name);

		for(size_t i=0; i<folder_infolist.size(); i++) {
			auto folder = &folder_infolist.at(i);
			if(folder->path_hash == cur_path_hash) {
				cur_folder = folder;
				break;
			}
		}
		if(cur_folder == NULL) {
			printf("kosuzu: save error: failed to find folder '%s'\n",cur_path.c_str());
			std::exit(-1);
		}

		/* add current entry to the folder --------------*/
		if(cur_folder->block_count >= 32) {
			puts("kosuzu: save error: block count over");
		}

		folder_blocklist.at(cur_folder->id).entries[cur_folder->block_count++] = &entry;
	}
	
	/* calculate size of all file sections ----------------------------------*/
	size_t file_headsection_size = align_offset(sizeof(KOSUZU_FILEHEADER));
	size_t file_nodesection_size = align_offset(sizeof(KOSUZU_FILENODE) * (entry_count+1));
	size_t file_treesection_size = 0;
	size_t file_datasection_size = 0;
	size_t file_allsize = 0;
	size_t tree_count = 0;

	for(int i=0; i<entry_count; i++) {
		const auto &entry = entries[i];
		switch(entry.out_type) {
			case KOSUZU_NODETYPE_USERDATA: {
				size_t file_size = entry.i.udata_size;
				if(entry.is_file) {
					auto file = std::fopen(entry.i.src_filename,"rb");
					if(!file) {
						printf("kosuzu: save error: unable to read file '%s'\n",
							entry.i.src_filename
						);
						std::exit(-1);
					}
					std::fseek(file,0,SEEK_END);
					file_size = std::ftell(file);
					std::fclose(file);
				}
				file_datasection_size += align_offset(file_size);
				break;
			}
			case KOSUZU_NODETYPE_FOLDER: { break; }
			default: { break; }
		}
	}

	for(int f=0; f<folder_infolist.size(); f++) {
		const auto folder = folder_infolist.at(f);
		tree_count += folder.block_count;
	};

	file_treesection_size = align_offset(sizeof(uint16_t) * tree_count);
	
	file_allsize = 
		file_headsection_size +
		file_nodesection_size +
		file_treesection_size +
		file_datasection_size;

	/* write to a file ------------------------------------------------------*/
	KOSUZU_FILEHEADER file_header;
	std::vector<uint8_t> file_buffer;
	file_buffer.reserve(file_allsize);

	auto fbuffer_writeData = [&](const void *src_data,size_t dat_size) {
		for(size_t i=0; i<dat_size; i++) {
			file_buffer.push_back(((uint8_t*)src_data)[i]);
		}
	};
	auto fbuffer_seekPos = [&](size_t dst_offset) {
		uint8_t tempdata[4] = { 0xAB,0xAB,0xAB,0xAB };
		while(file_buffer.size() != dst_offset) {
			const size_t index = file_buffer.size() & 3;
			fbuffer_writeData(&tempdata[index],sizeof(char));
		}
	};

	// write header
	{
		const char *magic_string = "kosuzu!";
		file_header = {
			.node_count = entry_count + 1,
			.tree_count = tree_count,
			.data_size = file_datasection_size,
			.offset_nodes = file_headsection_size,
			.offset_trees = file_headsection_size + file_nodesection_size,
			.offset_data = file_headsection_size + file_nodesection_size + file_treesection_size
		};

		std::memcpy(file_header.magic,magic_string,8);
		fbuffer_writeData(&file_header,sizeof(file_header));
	}

	// write nodes
	{
		size_t file_treeindex = 0;
		size_t file_dataoffset = 0;
		std::vector<KOSUZU_FILENODE> list_filenodes;
		
		// add root node
		{
			const auto folder = folder_infolist.front();
			KOSUZU_FILENODE node = {
				.name_hash = folder.name_hash,
				.node_type = KOSUZU_NODETYPE_FOLDER
			};
			node.d.folder_size = folder.block_count;
			node.d.folder_treeindex = file_treeindex;
			file_treeindex += folder.block_count;
			list_filenodes.push_back(node);
		}

		// creating node array, slightly adding to data offset
		for(int e=0; e<entry_count; e++) {
			const auto &entry = entries[e];
			
			auto node_type = entry.out_type;
			KOSUZU_FILENODE node = {
				.name_hash = kosuzu_hashString(entry.name),
				.node_type = static_cast<uint32_t>(entry.out_type)
			};

			switch(node_type) {
				case KOSUZU_NODETYPE_INT: {
					node.d.value_sint = entry.i.value_sint;
					break;
				};
				case KOSUZU_NODETYPE_UINT: {
					node.d.value_uint = entry.i.value_uint;
					break;
				}
				case KOSUZU_NODETYPE_USERDATA: {
					// get file size, if any
					size_t file_size = entry.i.udata_size;
					if(entry.is_file) {
						auto file = std::fopen(entry.i.src_filename,"rb");
						if(!file) {
							puts("kosuzu: save error: invalid file");
							std::exit(-1);
						}
						std::fseek(file,0,SEEK_END);
						file_size = std::ftell(file);
						std::fclose(file);
					}
					node.d.udata_size = file_size;
					node.d.udata_offset = file_dataoffset;
					file_dataoffset += align_offset(file_size);
					break;
				}
				case KOSUZU_NODETYPE_FOLDER: {
					const auto folder = entry_getFolder(e);
					if(folder) {
						node.d.folder_size = folder->block_count;
						node.d.folder_treeindex = file_treeindex;
						file_treeindex += folder->block_count;
					} else {
						std::puts("kosuzu: save error: couldn't find corresponding folder.");
						std::exit(-1);
					}
					break;
				}
				default: {
					std::printf("kosuzu: save error: entry %d has unknown node type\n",e);
					std::exit(-1);
					break;
				}
			}

			list_filenodes.push_back(node);
		}
		fbuffer_seekPos(file_header.offset_nodes);
		fbuffer_writeData(list_filenodes.data(),list_filenodes.size() * sizeof(KOSUZU_FILENODE));
		
		// writing trees to the file
		fbuffer_seekPos(file_header.offset_trees);
		
		for(int f=0; f<folder_infolist.size(); f++) {
			const auto folder = folder_infolist.at(f);
			const auto blocklist = folder_blocklist.at(f);
			//printf("folder %2d:\n",f);
			for(int e=0; e<folder.block_count; e++) {
				const auto entry = blocklist.entries[e];
				uint16_t real_offset;
				size_t offset = ((size_t)entry) - ((size_t)entries);
				offset /= sizeof(KOSUZU_SAVEENTRY); // regular integer.
				offset += 1; // since 1st is root node.
				real_offset = offset & 0xFFFF;
				if(offset > 0x7FFF) {
					puts("kosuzu: save error: too many nodes");
					std::exit(-1);
				}
				//printf("\tchild [%2d]: node %d\n",e,real_offset);
				fbuffer_writeData(&offset,sizeof(real_offset));
			}
		}
	}
	
	// write data
	{
		fbuffer_seekPos(file_header.offset_data);
		for(int e=0; e<entry_count; e++) {
			const auto &entry = entries[e];
			if(entry.out_type == KOSUZU_NODETYPE_USERDATA) {
				if(entry.is_file) {
					char chr_buf;
					size_t file_size = 0;
					auto file = std::fopen(entry.i.src_filename,"rb");
					if(!file) {
						std::puts("kosuzu: save error: invalid file for reading");
						std::exit(-1);
					}
					std::fseek(file,0,SEEK_END);
					file_size = std::ftell(file);
					std::rewind(file);

					size_t size_real = (file_size);
					size_t dst_pos = align_offset(file_buffer.size() + size_real);

					for(size_t i=0; i<file_size; i++) {
						std::fread(&chr_buf,sizeof(char),1,file);
						fbuffer_writeData(&chr_buf,sizeof(char));
					}
					std::fclose(file);
					
					fbuffer_seekPos(dst_pos);
				} else {
					size_t size_real = (entry.i.udata_size);
					size_t dst_pos = align_offset(file_buffer.size() + size_real);
					
					fbuffer_writeData(entry.i.udata_ptr,entry.i.udata_size);
					fbuffer_seekPos(dst_pos);
				}
			}
		}
	}

	/* copy to buffer and/or file -------------------------------------------*/
	if(out_buffer) {
		void *cpy_buffer = std::calloc(sizeof(char),file_allsize);
		std::memcpy(cpy_buffer,file_buffer.data(),file_allsize);
		
		*out_buffer = cpy_buffer;
	}
	if(save_info->out_filename) {
		const auto filename = save_info->out_filename;
		auto file = std::fopen(filename,"wb");
		if(!file) {
			std::printf("kosuzu: save error: can't write to file '%s'",
				filename
			);
			return false;
		}
		std::fwrite(file_buffer.data(),sizeof(char),file_allsize,file);
		std::fclose(file);
	}
	
	if(save_info->flag_verbose) {
		for(int i=0; i<folder_infolist.size(); i++) {
			const auto folder = folder_infolist.at(i);
			std::string folder_name = "root";
			if(folder.entry_id != -1) {
				folder_name = entries[folder.entry_id].name;
			}
			printf("folder %2d: l:%08Xh a:%08Xh '%s'\n",i,folder.name_hash,folder.path_hash,folder_name.c_str());
			
			// go through folder
			const auto block_list = folder_blocklist.at(folder.id);
			for(int j=0; j<folder.block_count; j++) {
				printf("\tnode[%2d]: '%s'\n",j,block_list.entries[j]->name);
			}
		}
	}

	return true;
}

