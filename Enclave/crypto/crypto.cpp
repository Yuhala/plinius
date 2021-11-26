/**
 * Author: xxx xxxx
 * Cryptography fxnality
 * Encrypt and Decrypt: AES-GCM and CTR modes with 128 bit keys
 */

#include "crypto.h"
#include "dnet_types.h"

/**
 *  Galois Counter Mode encryption is used with 128bit key 
 *  GCM provides both confidentiality and integrity
 *  CTR does not provide integrity
 */
//static sgx_aes_gcm_128bit_key_t key = {0x76, 0x39, 0x79, 0x24, 0x42, 0x26, 0x45, 0x28, 0x48, 0x2b, 0x4d, 0x3b, 0x62, 0x51, 0x5e, 0x8f};
sgx_aes_gcm_128bit_key_t key;
void encryptData(void *dataIn, size_t len, char *dataOut, size_t lenOut, AES_ALGO algo)
{
	memcpy(key, enc_key, 16);
	uint8_t *clairText = (uint8_t *)dataIn;
	uint8_t p_dst[BUFLEN] = {0};
	const uint32_t num_inc_bits = 128;
	//Algorithm: 0-GCM 1-CTR

	switch (algo)
	{
	case GCM /*GCM mode*/:
		// Generate the IV (nonce)
		/* 
		 If the IV/nonce is random, it can be combined together with the counter (XOR, addition etc)				
		to produce the actual unique counter block for encryption */

		sgx_read_rand(p_dst + SGX_AESGCM_MAC_SIZE, SGX_AESGCM_IV_SIZE);

		sgx_rijndael128GCM_encrypt(
			&key,
			clairText, len,
			p_dst + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE,
			p_dst + SGX_AESGCM_MAC_SIZE, SGX_AESGCM_IV_SIZE,
			NULL, 0,
			(sgx_aes_gcm_128bit_tag_t *)(p_dst));
		//dataOut[lenOut] = '\0'; //Terminate encrypted data
		memcpy(dataOut, p_dst, lenOut);
		break;

	case CTR /*CTR mode*/:
		lenOut -= SGX_AESGCM_MAC_SIZE;
		sgx_read_rand(p_dst, SGX_AESGCM_IV_SIZE);

		sgx_aes_ctr_encrypt(
			&key,
			clairText,
			len,
			p_dst,
			num_inc_bits,
			p_dst + SGX_AESGCM_IV_SIZE);
		//dataOut[lenOut] = '\0';
		memcpy(dataOut, p_dst, lenOut);
		break;
	}
}

void decryptData(char *dataIn, size_t len, void *dataOut, size_t lenOut, AES_ALGO algo)
{
	memcpy(key, enc_key, 16);
	uint8_t *cipherText = (uint8_t *)dataIn;
	uint8_t p_dst[BUFLEN] = {0};
	const uint32_t num_inc_bits = 128;
	switch (algo)
	{
	case GCM /*GCM mode*/:
		sgx_rijndael128GCM_decrypt(
			&key,
			cipherText + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE,
			lenOut,
			p_dst,
			cipherText + SGX_AESGCM_MAC_SIZE, SGX_AESGCM_IV_SIZE,
			NULL, 0,
			(sgx_aes_gcm_128bit_tag_t *)cipherText);
		//dataOut[lenOut] = '\0';
		memcpy(dataOut, p_dst, lenOut);
		//emit_debug((char *) p_dst);
		break;

	case CTR /*CTR mode*/:
		lenOut -= (SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE);
		sgx_aes_ctr_decrypt(
			&key,
			cipherText + SGX_AESGCM_IV_SIZE,
			lenOut,
			cipherText,
			num_inc_bits,
			p_dst);
		memcpy(dataOut, p_dst, lenOut);
		break;
	}
}

/**
 * Author: Peterson Yuhala
 * Flags: ENCRYPT, DECRYPT, DEFAULT
 * ENCRYPT: encrypt src and copy result to dest
 * DECRYPT: decrypt src and copy result to dest
 * DEFAULT: do default memcpy
 */
void enc_memcpy(void *dest, void *src, size_t n, ENC_FLAG flag)
{

	switch (flag)
	{
	case ENCRYPT:
	{
#ifdef ENABLE_CRYPTO
		char temp[BUFLEN];
		encryptData(src, n, temp, n + ADD_ENC_DATA_SIZE, GCM);
		memcpy(dest, temp, n + ADD_ENC_DATA_SIZE);

#else
		memcpy(dest, src, n);
#endif
		break;
	}

	case DECRYPT:
	{
#ifdef ENABLE_CRYPTO
		decryptData((char *)src, n + ADD_ENC_DATA_SIZE, dest, n, GCM);
#else
		memcpy(dest, src, n);
#endif
		break;
	}

	default:
		memcpy(dest, src, n);
		break;
	}
}
