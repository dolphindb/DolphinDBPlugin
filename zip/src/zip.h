#ifndef ZIP_H_
#define ZIP_H_


#include "Exceptions.h"
#include "Logger.h"
#include <string>

#if (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
        #ifndef __USE_FILE_OFFSET64
                #define __USE_FILE_OFFSET64
        #endif
        #ifndef __USE_LARGEFILE64
                #define __USE_LARGEFILE64
        #endif
        #ifndef _LARGEFILE64_SOURCE
                #define _LARGEFILE64_SOURCE
        #ifndef _FILE_OFFSET_BIT
        #endif
                #define _FILE_OFFSET_BIT 64
        #endif
#endif

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
#include "ScalarImp.h"
#include "Util.h"

#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)

#ifdef _WIN32
#define mkdir(name, mode) mkdir(name)
#endif

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)


static string ZIP_PREFIX = "[PLUGIN::ZIP] ";

extern "C" ConstantSP unzip(Heap* heap, vector<ConstantSP>& args);

namespace OperatorImp{
    ConstantSP convertEncode(Heap* heap, vector<ConstantSP>& arguments);
}

enum class zipEncode {DEFAULT, UTF8, GBK};

string gbkToUtf8(Heap* heap, string str) {
    ConstantSP line = new String(str);
    ConstantSP src = new String("gbk");
    ConstantSP dest = new String("utf-8");
    vector<ConstantSP> args = {line, src, dest};
    ConstantSP result = OperatorImp::convertEncode(heap, args);
    return result->getString();
}

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
    ret = mkdir(dirname,0775);
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
                LOG_ERR("Error allocating memory\n");
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
          LOG_ERR("couldn't create directory %s\n",buffer);
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

int getFilenames(unzFile uf, vector<string>& filenames){
    uLong i;
    unz_global_info64 gi;
    int err;

    err = unzGetGlobalInfo64(uf,&gi);
    if (err != UNZ_OK)
        LOG_ERR("error %d with zipfile in unzGetGlobalInfo \n",err);
    for (i = 0; i < gi.number_entry; i++)
    {
        char filename_inzip[256];
        err = unzGetCurrentFileInfo64(uf, 0, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
        if (err!=UNZ_OK)
        {
            LOG_ERR("error %d with zipfile in unzGetCurrentFileInfo\n",err);
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
                LOG_ERR("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    return 0;
}

// print binary string's hex data
void printBinary(string word, string outputPath) {
    std::cout << word << "     " << outputPath << "     ";
    for(size_t i = 0; i < outputPath.size(); ++i) {
        std::cout << (unsigned int)(unsigned char)outputPath.c_str()[i] << " ";
    }
    std::cout << "\n";
}

//int do_extract_currentfile(unzFile uf, const int* popt_extract_without_path, int* popt_overwrite, const char* password)
int do_extract_currentfile(unzFile uf, const int* popt_extract_without_path, int* popt_overwrite, const char* password, const string& outputPath, Heap* heap, zipEncode encode)
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

    string fileTmp = filename_inzip;
#ifdef WIN32
	fileTmp = Util::replace(fileTmp, '/', '\\');
    if(encode == zipEncode::DEFAULT || encode == zipEncode::GBK) {
        fileTmp = gbkToUtf8(heap, fileTmp);
    }
#else
    if(encode == zipEncode::GBK) {
        fileTmp = gbkToUtf8(heap, fileTmp);
    }
#endif
    memcpy(filename_inzip, fileTmp.c_str(), fileTmp.size());
    filename_inzip[fileTmp.size()] = '\0';

    if (err!=UNZ_OK)
    {
        LOG_ERR(ZIP_PREFIX, "error ", err, " with zipfile in unzGetCurrentFileInfo\n");
        return err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    if (buf==NULL)
    {
        LOG_ERR(ZIP_PREFIX, "Error allocating memory\n");
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
            // LOG_ERR("creating directory: %s\n",filename_inzip);
            // mymkdir(filename_inzip);
            string fileName = outputPath + filename_inzip;
            // LOG_ERR("creating directory: %s\n",fileName.c_str()); // memo: creating directory
            string errMsg;
        #ifdef WIN32
            fileName = Util::replace(fileName, "/", "\\");
        #else
        #endif
            // printBinary("0st create dir ", fileName);
            // mymkdir(fileName.c_str());
            Util::createDirectoryRecursive(fileName, errMsg);
            if(errMsg.size() != 0) {
                LOG_ERR(ZIP_PREFIX, errMsg);
            }
        }
    }
    else
    {
        string write_filename;
        int skip=0;

        if ((*popt_extract_without_path)==0)
            write_filename = outputPath + filename_inzip;
        else
            write_filename = outputPath + filename_withoutpath;

        err = unzOpenCurrentFilePassword(uf,password);
        if (err!=UNZ_OK)
        {
            LOG_ERR(ZIP_PREFIX, "error ", err, " with zipfile in unzOpenCurrentFilePassword\n");
        }

        if (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            char rep=0;
            FILE* ftestexist;
        #ifdef WIN32
            write_filename = Util::replace(write_filename, "/", "\\");
        #else
        #endif
            // printBinary("1st open file ", write_filename);
            ftestexist = Util::fopen(write_filename.c_str(),"rb");
            if (ftestexist!=NULL)
            {
                fclose(ftestexist);
                do
                {
                    char answer[128];
                    int ret;

                    LOG_ERR(ZIP_PREFIX, "The file ", write_filename, " exists. Overwrite ? [y]es, [n]o, [A]ll: ");
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
        #ifdef WIN32
            write_filename = Util::replace(write_filename, "/", "\\");
        #else
        #endif
            // printBinary("2nd open file ", write_filename);
            fout=Util::fopen(write_filename.c_str(),"wb");
            /* some zipfile don't contain directory alone before file */
            if ((fout==NULL) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(char*)filename_inzip))
            {
                char c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                //makedir(write_filename);
                write_filename = outputPath + filename_inzip;
                string errMsg;
            #ifdef WIN32
                write_filename = Util::replace(write_filename, "/", "\\");
            #else
            #endif
                // printBinary("3th create dir ", write_filename);
                // makedir(write_filename.c_str());
                Util::createDirectoryRecursive(write_filename, errMsg);
                if(errMsg.size() != 0) {
                    LOG_ERR(ZIP_PREFIX, errMsg);
                }
                *(filename_withoutpath-1)=c;
                //fout=Util::fopen(write_filename,"wb");
                write_filename = outputPath + filename_inzip;
            #ifdef WIN32
                write_filename = Util::replace(write_filename, "/", "\\");
            #else
            #endif
                // printBinary("4th open file ", write_filename);
                fout=Util::fopen(write_filename.c_str(),"wb");
            }

            if (fout==NULL)
            {
                LOG_ERR(ZIP_PREFIX, "error opening ", write_filename);
            }
        }

        if (fout!=NULL)
        {
            // LOG_ERR(" extracting: %s\n",write_filename);
            // LOG_ERR("        extracting: %s\n",write_filename.c_str()); // memo: extracting
            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                if (err<0)
                {
                    LOG_ERR(ZIP_PREFIX, "error", err, "with zipfile in unzReadCurrentFile\n");
                    break;
                }
                if (err>0)
                    if (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        LOG_ERR(ZIP_PREFIX, "error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        break;
                    }
            }
            while (err>0);
            if (fout)
                    fclose(fout);

            if (err==0)
                change_file_date(write_filename.c_str(),file_info.dosDate,
                                 file_info.tmu_date);
        }

        if (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            if (err!=UNZ_OK)
            {
                LOG_ERR(ZIP_PREFIX, "error", err, "with zipfile in unzCloseCurrentFile\n");
            }
        }
        else
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    free(buf);
    return err;
}


int do_extract(unzFile uf, int opt_extract_without_path, int opt_overwrite, const char* password,
               const string& outputDir, Heap* heap, const FunctionDefSP& function, zipEncode code)
{
    uLong i;
    unz_global_info64 gi;
    int err;

    err = unzGetGlobalInfo64(uf,&gi);
    if (err != UNZ_OK)
        throw RuntimeException(ZIP_PREFIX + "error " + std::to_string(err) + " with zipfile in unzGetGlobalInfo \n");

    for (i = 0; i < gi.number_entry; i++)
    {
        // Extract current file
        if (do_extract_currentfile(uf, &opt_extract_without_path,
                                   &opt_overwrite, password,
                                   outputDir, heap, code) != UNZ_OK) {
            throw RuntimeException(ZIP_PREFIX + "Failed to extract file in zip file.");
        }

        if (!function.isNull()) {
            char filename_inzip[256];
            err = unzGetCurrentFileInfo64(uf, NULL, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
            if (err != UNZ_OK)
            {
                throw RuntimeException(ZIP_PREFIX + "error " + std::to_string(err) + " with zipfile in unzGetCurrentFileInfo\n");
            }
            string abPath = outputDir + filename_inzip;
        #ifdef WIN32
	        abPath = Util::replace(abPath, "/", "\\");
            if (abPath.back() != '\\') {
                ConstantSP arg = Util::createConstant(DT_STRING);
                arg->setString(abPath);
                vector<ConstantSP> args = {arg};
                function->call(heap, args);
            }
        #else
            if (abPath.back() != '/') {
                ConstantSP arg = Util::createConstant(DT_STRING);
                arg->setString(abPath);
                vector<ConstantSP> args = {arg};
                function->call(heap, args);
            }
        #endif
        }

        // handle next file
        if ((i + 1) < gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err != UNZ_OK)
            {
                throw RuntimeException(ZIP_PREFIX + "error " + std::to_string(err) + " with zipfile in unzGoToNextFile\n");
            }
        }
    }
    return 0;
}


#endif // ZIP_H_