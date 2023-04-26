## building
lua not required but i used it for building

	lua build.lua build_lib build_tests

builds library and test program. omit any of the 2 last keywords if you want
to build only the test or the library.

## usage
```c++
KOSUZU_ARCHIVE archive = {};
if(kosuzu_archiveOpenFile(&archive,"character.ksz")) {
	kosuzu_archiveChdir(&archive,"cg");
	
	KOSUZU_FILE *src_file = kosuzu_archiveFileOpen(&archive,"walk.bmp");
	if(src_file) {
		FILE *out_file = fopen("output.bmp","wb");
		char file_buffer;
		for(size_t i=0; i<src_file->file_size; i++) {
			kosuzu_file_read(src_file,&file_buffer,sizeof(char));
			fwrite(&file_buffer,1,sizeof(char),out_file);
		}
		fclose(out_file);
		kosuzu_file_close(src_file);
	}

	kosuzu_archiveClose(&archive);
}
```

reads a file 'cg\walk.bmp' from the archive 'character.ksz', and then writes
it back to a file on disk 'output.bmp'.

```c++
KOSUZU_ARCHIVE archive = {};
if(kosuzu_archiveOpenFile(&archive,"map.ksz")) {
	const KOSUZU_NODE *width = kosuzu_archiveNodeFind(&archive,"width");
	const KOSUZU_NODE *height = kosuzu_archiveNodeFind(&archive,"height");

	// check if nodes were actually found
	if(width && height) {
		printf("width: %d\n",width->d.value_int);
		printf("height: %d\n",height->d.value_int);
	}

	kosuzu_archiveClose(&archive);
}
```

reading integers doesn't require using any file I/O: all you need to do is
access the a node's struct directly.
the above code reads two int values 'width' and 'height' from the archive
'map.ksz'.

