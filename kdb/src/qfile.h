#ifndef _QFILE_H_
#define _QFILE_H_

#include <type_traits>
#include <string>
#include <vector>

#include "DolphinDBEverything.h"
#include <CoreConcept.h>

#include "k.h"
#include "q2ddb.h"

namespace kdb {
ConstantSP extractSchema(const string &tablePath, const vector<string> &symList, const vector<string> &colNames,
                         const string &symPath, const string &symName);
ConstantSP loadFileEx(DatabaseUpdater &dbUpdater, const string &tablePath, const vector<string> &symList,
                      const string &symName, const vector<string> &colNames, long long batchSize);
class BinFile {
  public:
    static constexpr std::size_t MAGIC_BYTES = 8;
    constexpr static std::size_t LONG_BYTES = 8;
    constexpr static std::size_t INT_BYTES = 4;

  public:
    BinFile(const std::string &path, const std::string &filename);
    ~BinFile();

    operator bool() const;

    std::size_t readInto(std::vector<byte> &buffer);

  private:
    std::size_t getFileLen() const;
    std::size_t getBodyLen() const;

    std::size_t readAll(std::vector<byte> &buffer, std::ptrdiff_t offset = 0);
    // std::size_t inflateBody(
    //     std::vector<byte>& buffer);
    std::size_t inflateBody(std::vector<byte> &buffer);

  private:
    std::string filename_;
    FILE *fp_;

    };//class BinFile

    //////////////////////////////////////////////////////////////////////////

//     class ZLibStream {
//     public:
// #       pragma pack(push, 1)
//         //@see https://www.ietf.org/rfc/rfc1950.txt
//         union Header {
//             byte magic[2];  // 0x7801 | 0x785E | 0x789C | 0x78DA
//             struct {
//                 union {
//                     byte CMF;  // Compress Method & Flags
//                     struct {
//                         byte compressMethod: 4;
//                         byte compressInfo: 4;
//                     };
//                 };
//                 union {
//                     byte FLG;  // FLaGs
//                     struct {
//                         byte checkBits: 5;  //so that `magic` is multiple of 31
//                         bool presetDict: 1;
//                         byte compressLevel: 2;
//                     };
//                 };
//             };
//         };
//         static_assert(
//             sizeof(Header) == 2,
//             "RFC 1950 zlib header format"
//         );
// #       pragma pack(pop)

//         enum CompressMethod : byte {
//             CM_DEFLATE = 8,
//         };

//     public:
//         struct Trailer;

//     public:
//         ZLibStream(FILE* fp, std::ptrdiff_t offset);

//         bool isDeflated(Header* ph = nullptr) const;

//         std::size_t inflate(std::vector<byte>& buffer);

//     private:
//         int inflateChunks(std::vector<byte>& buffer);

//     private:
//         FILE* const fp_;
//         const std::ptrdiff_t offset_;

//     };//class ZLibStream

}//namespace kdb

#endif//_QFILE_H_