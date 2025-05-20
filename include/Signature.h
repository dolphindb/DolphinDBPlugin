/*
 * Signature.h
 *
 *  Created on: Dec 23, 2016
 *      Author: dzhou
 */

#ifndef SIGNATURE_H_
#define SIGNATURE_H_

#include <string>


#include "SmartPointer.h"
#include "CoreConcept.h"
namespace ddb {

using std::string;

class DigitalSign {
public:
	bool verifySignature(const string& publicKey, const string& plainText, const char* signatureBase64) const;
	bool signMessage(const string& privateKey, const string& plainText, string& signature) const;
	static string sha256(const string& content);
	static bool sha256(const string& filePath, string& output);
	static void base64Encode(const unsigned char* buffer, size_t length, string& base64Text, bool noWrap = false);
	static void base64Decode(const char* b64message, unsigned char** buffer, int& length, bool noWrap = false);
	/**
	 * @brief aes-256-cfb encrypt
	 * 
	 * @param key encrypt key, fill zero if key length < 32 bytes
	 * @param iv output 16 bytes random vector, it's ok to public it.
	 * @param ptext text need to be encrypted
	 * @param ctext output encrypted text
	 */
	static bool aesEncrypt(string key, string& iv, const string& ptext, string& ctext);
	/**
	 * @brief aes-256-cfb decrypt
	 * 
	 * @param key decrypt key, fill zero if key length < 32 bytes
	 * @param iv 16 bytes random vector
	 * @param ctext encrypted text
	 * @param rtext output decrypted text
	 */
	static bool aesDecrypt(string key, const string& iv, const string& ctext, string& rtext);
	static bool hmac(const char* key, size_t keyLen, const char* data, size_t dataLen, const string& digest, string& output);

private:
	static size_t calcDecodeLength(const char* b64input);
	bool rsaSign(const string& privateKey, const unsigned char* Msg, size_t MsgLen, unsigned char** EncMsg, size_t* MsgLenEnc) const;
	bool rsaVerifySignature(const string& publicKey, unsigned char* MsgHash, size_t MsgHashLen, const char* Msg, size_t MsgLen, bool* Authentic) const;
};

} // namespace ddb
#endif /* SIGNATURE_H_ */
