#include "MinizipWrapper.h"

namespace modworks
{
  void extract(string zip_path, string directory_path)
  {
    writeLogLine(string("Extracting ") + zip_path);
    unzFile zipfile = unzOpen( zip_path.c_str() );
    unz_global_info global_info;
    unzGetGlobalInfo( zipfile, &global_info );
    char read_buffer[ READ_SIZE ];

    uLong i;
    for ( i = 0; i < global_info.number_entry; ++i )
    {
      unz_file_info file_info;
      char filename[ MAX_FILENAME ];
      char final_filename[ MAX_FILENAME ];
      if ( unzGetCurrentFileInfo(
          zipfile,
          &file_info,
          filename,
          MAX_FILENAME,
          NULL, 0, NULL, 0 ) != UNZ_OK)
      {
        unzClose(zipfile);
        return;
      }

      strcpy(final_filename,directory_path.c_str());
      strcat(final_filename,"/");
      strcat(final_filename,filename);

      const size_t filename_length = strlen(filename);
      if (filename[ filename_length-1 ] == dir_delimter)
      {
        createDirectory(final_filename);
      }
      else
      {
  		unzOpenCurrentFile( zipfile );

  		string new_file_path = filename;
  		FILE *out = fopen( final_filename, "wb" );

  		int error = UNZ_OK;
  		do
  		{
  			error = unzReadCurrentFile( zipfile, read_buffer, READ_SIZE );
  			if ( error < 0 )
  			{
  				unzCloseCurrentFile( zipfile );
  				unzClose( zipfile );
  				return;
  			}
  			if ( error > 0 )
  			{
  				fwrite( read_buffer, error, 1, out );
  			}
  		} while ( error > 0 );

  		fclose( out );
      }

      unzCloseCurrentFile( zipfile );

      if((i+1) < global_info.number_entry)
      {
        if(unzGoToNextFile(zipfile) != UNZ_OK)
        {
          unzClose(zipfile);
          return;
        }
      }
    }
    unzClose(zipfile);
    writeLogLine(zip_path + " extracted");
  }

  void compress(string directory, string zip_path)
  {
    if(directory[directory.size()-1]!='/')
      directory += '/';

    zipFile zf = NULL;
    #ifdef USEWIN32IOAPI
      zlib_filefunc64_def ffunc = {0};
    #endif
    char *zipfilename = (char*)zip_path.c_str();
    const char* password = NULL;
    void* buf = NULL;
    int size_buf = WRITEBUFFERSIZE;
    int errclose = 0;
    int err = 0;
    int opt_overwrite = APPEND_STATUS_CREATE;
    int opt_compress_level = 9;
    int opt_exclude_path = 0;

    buf = (void*)malloc(size_buf);
    if (buf == NULL)
    {
        printf("Error allocating memory\n");
        //return ZIP_INTERNALERROR;
    }

    #ifdef USEWIN32IOAPI
      fill_win32_filefunc64A(&ffunc);
      zf = zipOpen2_64(zipfilename, opt_overwrite, NULL, &ffunc);
    #else
      zf = zipOpen64(zipfilename, opt_overwrite);
    #endif

    if (zf == NULL)
    {
        printf("error opening %s\n", zipfilename);
        err = ZIP_ERRNO;
    }
    else
        printf("creating %s\n", zipfilename);

    vector<string> filenames = getFilenames(directory);
    for(int i=0;i<(int)filenames.size();i++)
    {
      string filename = filenames[i];
      string complete_file_path = directory + filename;
      FILE *fin = NULL;
      int size_read = 0;
      const char* filenameinzip = filename.c_str();
      const char *savefilenameinzip;
      zip_fileinfo zi = {0};
      unsigned long crcFile = 0;
      int zip64 = 0;

      /* Get information about the file on disk so we can store it in zip */
      filetime(complete_file_path.c_str(), &zi.tmz_date, &zi.dosDate);
      zip64 = is_large_file(complete_file_path.c_str());

      /* Construct the filename that our file will be stored in the zip as.
         The path name saved, should not include a leading slash.
         If it did, windows/xp and dynazip couldn't read the zip file. */
      savefilenameinzip = filenameinzip;
      while (savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/')
        savefilenameinzip++;
      /* Should the file be stored with any path info at all? */
      if (opt_exclude_path)
      {
        const char *tmpptr = NULL;
        const char *lastslash = 0;

        for (tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
        {
          if (*tmpptr == '\\' || *tmpptr == '/')
            lastslash = tmpptr;
        }

        if (lastslash != NULL)
          savefilenameinzip = lastslash + 1; /* base filename follows last slash. */
      }

      /* Add to zip file */
      err = zipOpenNewFileInZip3_64(zf, savefilenameinzip, &zi,
                    NULL, 0, NULL, 0, NULL /* comment*/,
                    (opt_compress_level != 0) ? Z_DEFLATED : 0,
                    opt_compress_level,0,
                    /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                    -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                    password, crcFile, zip64);

      if (err != ZIP_OK)
        printf("error in opening %s in zipfile (%d)\n", filenameinzip, err);
      else
      {
        fin = FOPEN_FUNC(complete_file_path.c_str(), "rb");
        if (fin == NULL)
        {
          err = ZIP_ERRNO;
          printf("error in opening %s for reading\n", filenameinzip);
        }
      }

      if (err == ZIP_OK)
      {
        /* Read contents of file and write it to zip */
        do
        {
          size_read = (int)fread(buf, 1, size_buf, fin);
          if ((size_read < size_buf) && (feof(fin) == 0))
          {
            printf("error in reading %s\n",filenameinzip);
            err = ZIP_ERRNO;
          }

          if (size_read > 0)
          {
            err = zipWriteInFileInZip(zf, buf, size_read);
            if (err < 0)
              printf("error in writing %s in the zipfile (%d)\n", filenameinzip, err);
          }
        }
        while ((err == ZIP_OK) && (size_read > 0));
      }

      if (fin)
        fclose(fin);

      if (err < 0)
        err = ZIP_ERRNO;
      else
      {
        err = zipCloseFileInZip(zf);
        if (err != ZIP_OK)
          printf("error in closing %s in the zipfile (%d)\n", filenameinzip, err);
      }
    }

    errclose = zipClose(zf, NULL);

    if (errclose != ZIP_OK)
      printf("error in closing %s (%d)\n", zipfilename, errclose);

    free(buf);
    //return err;
  }
}
