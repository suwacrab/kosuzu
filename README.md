## building
lua not required but i used it for building

	lua build.lua build_lib build_tests

builds library and test program. omit any of the 2 last keywords if you want
to build only the test or the library.

## usage
```c++
KOSUZU_RECORD archive = {};
if(kosuzu_recordOpenFile(&archive,"character.ksz")) {
	kosuzu_recordChdir(&archive,"cg");
	
	KOSUZU_FILE *src_file = kosuzu_recordFileOpen(&archive,"walk.bmp");
	if(src_file) {
		FILE *out_file = fopen("output.bmp","wb");
		char file_buffer;
		while(kosuzu_file_read(src_file,&file_buffer,sizeof(char)) == 1) {
			fwrite(&file_buffer,1,sizeof(char),out_file);
		}
		fclose(out_file);
		kosuzu_file_close(src_file);
	}

	kosuzu_recordClose(&archive);
}
```

reads a file 'cg\walk.bmp' from the archive 'character.ksz', and then writes
it back to a file on disk 'output.bmp'.

```c++
KOSUZU_RECORD archive = {};
if(kosuzu_recordOpenFile(&archive,"map.ksz")) {
	const KOSUZU_NODE *width = kosuzu_recordNodeFind(&archive,"width");
	const KOSUZU_NODE *height = kosuzu_recordNodeFind(&archive,"height");

	// check if nodes were actually found
	if(width && height) {
		printf("width: %d\n",width->d.value_int);
		printf("height: %d\n",height->d.value_int);
	}

	kosuzu_recordClose(&archive);
}
```

reading integers doesn't require using any file I/O: all you need to do is
access the a node's struct directly.
the above code reads two int values 'width' and 'height' from the archive
'map.ksz'.

