#ifndef MD5_H
#define MD5_H

// source code from https://github.com/joyeecheung/md5

#include <string>
using std::string;

namespace joyee {

#define BLOCK_SIZE 64  // 16-word referece RFC 1321 3.4
typedef unsigned char uint8_t;  // 8-bit
typedef unsigned int uint32_t;  // 32-bit
typedef unsigned int size_t;  // size type

class MD5 {
public:
  MD5();
  MD5& update(const unsigned char* in, size_t inputLen);
  MD5& update(const char* in, size_t inputLen);
  MD5& finalize();
  string toString() const;

#ifndef SAMPLE_TEST
private:
#endif
  void init();
  void transform(const uint8_t block[BLOCK_SIZE]);

  uint8_t buffer[BLOCK_SIZE];  // buffer of the raw data
  uint8_t digest[16];  // result hash, little endian

  uint32_t state[4];  // state (ABCD)
  uint32_t lo, hi;    // number of bits, modulo 2^64 (lsb first)
  bool finalized;  // if the context has been finalized
};

string md5(const string str);

}

#endif