#ifndef ZIP_H_
#define ZIP_H_


#if (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
        #ifndef __USE_FILE_OFFSET64
                #define __USE_FILE_OFFSET64
        #endif
        #ifndef __USE_LARGEFILE64
                #define __USE_LARGEFILE64
        #endif
        #ifndef _LARGEFILE64_SOURCE
                #define _LARGEFILE64_SOURCE
        #endif
        #ifndef _FILE_OFFSET_BIT
                #define _FILE_OFFSET_BIT 64
        #endif
#endif

#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

# include <unistd.h>
# include <utime.h>

#include "unzip.h"
#include "CoreConcept.h"
#include "Util.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

/* change_file_date : change the date/time of a file
    filename : the filename of the file where date/time must be modified
    dosdate : the new date at the MSDos format (4 bytes)
    tmu_date : the SAME new date at the tm_unz format */
void change_file_date(const char *filename, uLong dosdate, tm_unz tmu_date)
{
  (void)dosdate;
  struct utimbuf ut;
  struct tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  if (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  else
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
}

int mymkdir(const char* dirname)
{
    int ret=0;
    ret = mkdir (dirname,0775);
    return ret;
}

int makedir(const char *newdir)
{
  char *buffer ;
  char *p;
  size_t len = strlen(newdir);

  if (len == 0)
    return 0;

  buffer = (char*)malloc(len+1);
        if (buffer==NULL)
        {
                printf("Error allocating memory\n");
                return UNZ_INTERNALERROR;
        }
  strcpy(buffer,newdir);

  if (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  if (mymkdir(buffer) == 0)
    {
      free(buffer);
      return 1;
    }

  p = buffer+1;
  while (1)
    {
      char hold;

      while(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      if ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          free(buffer);
          return 0;
        }
      if (hold == 0)
        break;
      *p++ = hold;
    }
  free(buffer);
  return 1;
}

int getFilenames(unzFile uf, vector<string>& filenames)
{
    uLong i;
    unz_global_info64 gi;
    int err;

    err = unzGetGlobalInfo64(uf,&gi);
    if (err != UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo \n",err);
    for (i = 0; i < gi.number_entry; i++)
    {
        char filename_inzip[256];
        err = unzGetCurrentFileInfo64(uf, 0, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
        if (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            break;
        }
        string str = filename_inzip;
        if (str.back() != '/') {
            filenames.push_back(filename_inzip);
        }
        if ((i + 1) < gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err != UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    return 0;
}

//int do_extract_currentfile(unzFile uf, const int* popt_extract_without_path, int* popt_overwrite, const char* password)
int do_extract_currentfile(unzFile uf, const int* popt_extract_without_path, int* popt_overwrite, const char* password, const string& outputPath)
{
    char filename_inzip[256];
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);

    if (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    if (buf==NULL)
    {
        printf("Error allocating memory\n");
        return UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    while ((*p) != '\0')
    {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    if ((*filename_withoutpath)=='\0')
    {
        if ((*popt_extract_without_path)==0)
        {
            // printf("creating directory: %s\n",filename_inzip);
            // mymkdir(filename_inzip);
            string fileName = outputPath + filename_inzip;
            printf("creating directory: %s\n",fileName.c_str());
            mymkdir(fileName.c_str());
        }
    }
    else
    {
        //const char* write_filename;
        string write_filename;
        int skip=0;

        if ((*popt_extract_without_path)==0)
            //write_filename = filename_inzip;
            write_filename = outputPath + filename_inzip;
        else
            //write_filename = filename_withoutpath;
            write_filename = outputPath + filename_withoutpath;

        err = unzOpenCurrentFilePassword(uf,password);
        if (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        if (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            char rep=0;
            FILE* ftestexist;
            //ftestexist = FOPEN_FUNC(write_filename,"rb");
            ftestexist = FOPEN_FUNC(write_filename.c_str(),"rb");
            if (ftestexist!=NULL)
            {
                fclose(ftestexist);
                do
                {
                    char answer[128];
                    int ret;

                    //printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename.c_str());
                    ret = scanf("%1s",answer);
                    if (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    if ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                while ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            if (rep == 'N')
                skip = 1;

            if (rep == 'A')
                *popt_overwrite=1;
        }

        if ((skip==0) && (err==UNZ_OK))
        {
            //fout=FOPEN_FUNC(write_filename,"wb");
            fout=FOPEN_FUNC(write_filename.c_str(),"wb");
            /* some zipfile don't contain directory alone before file */
            if ((fout==NULL) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(char*)filename_inzip))
            {
                char c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                //makedir(write_filename);
                makedir(write_filename.c_str());
                *(filename_withoutpath-1)=c;
                //fout=FOPEN_FUNC(write_filename,"wb");
                fout=FOPEN_FUNC(write_filename.c_str(),"wb");
            }

            if (fout==NULL)
            {
                //printf("error opening %s\n",write_filename);
                printf("error opening %s\n",write_filename.c_str());
            }
        }

        if (fout!=NULL)
        {
            //printf(" extracting: %s\n",write_filename);
            printf(" extracting: %s\n",write_filename.c_str());

            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                if (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }
                if (err>0)
                    if (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        break;
                    }
            }
            while (err>0);
            if (fout)
                    fclose(fout);

            if (err==0)
            // change_file_date(write_filename,file_info.dosDate,
            //                      file_info.tmu_date);
                change_file_date(write_filename.c_str(),file_info.dosDate,
                                 file_info.tmu_date);
        }

        if (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        else
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    free(buf);
    return err;
}


int do_extract(unzFile uf, int opt_extract_without_path, int opt_overwrite, const char* password, 
               const string& outputDir, Heap* heap, FunctionDefSP function)
{
    uLong i;
    unz_global_info64 gi;
    int err;

    err = unzGetGlobalInfo64(uf,&gi);
    if (err != UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo \n",err);

    for (i = 0; i < gi.number_entry; i++)
    {
        // 解压当前文件
        if (do_extract_currentfile(uf, &opt_extract_without_path,
                                       &opt_overwrite,
                                       password, outputDir) != UNZ_OK) {
            break;
        }
        
        // 获取当前解压文件名字，并使用回调函数进行处理
        if (!function.isNull()) {
            char filename_inzip[256];
            err = unzGetCurrentFileInfo64(uf, NULL, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
            if (err != UNZ_OK)
            {
                printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
                break;
            }
            string abPath = outputDir + filename_inzip;
            if (abPath.back() != '/') {
                ConstantSP arg = Util::createConstant(DT_STRING);
                arg->setString(abPath);
                vector<ConstantSP> args = {arg};
                function->call(heap, args);
            }
        }

        // 处理下一个解压文件
        if ((i + 1) < gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err != UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    return 0;
}

extern "C" ConstantSP unzip(Heap* heap, vector<ConstantSP>& args);

#endif // ZIP_H_