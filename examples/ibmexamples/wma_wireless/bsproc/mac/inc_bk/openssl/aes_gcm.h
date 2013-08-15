/* crypto/aes/aes_gcm.h */
/*----------------------------------------------------------------------------
// Licensed materials - Property of IBM                                      
//
// (C) Copyright IBM Corp.  2007
// This code was donated to the OpenSSL project under the terms of the 
// OpenSSL license.
//
// GCM table driven acceleration: Aaron Cristensen November 2007.
//---------------------------------------------------------------------------*/

#if !defined(AES_GCM_H)
#define AES_GCM_H

#define FAILURE 0
#define OK 1

#define AES_GCM_CTRL_SET_ACCEL 0
#define AES_GCM_CTRL_GET_ACCEL 1

/*! @enum GCM_ACCEL
    @brief The level of acceleration to use; a space-time trade-off
 */
typedef enum GCM_ACCEL {
  GCM_ACCEL_noaccel,    /*!< No acceleration and no additional space used */
  GCM_ACCEL_level1,     /*!< Uses a 256 byte table to speed up GHASH computation */
  GCM_ACCEL_level2,     /*!< Uses a 4 Kbyte table to speed up GHASH computation */
  GCM_ACCEL_level3,     /*!< Uses a 8 Kbyte table to speed up GHASH computation */
  GCM_ACCEL_level4      /*!< Uses a 64 Kbyte table to speed up GHASH computation */
} GCM_ACCEL;

/*! @brief A table holding lookup tables for accelerating Galois multiply operations */
typedef struct GCM_TAB {
  GCM_ACCEL accel;        /*!< Type of acceleration table */
  void *tabdata; /*!< How long tabdata is depends on the type of table used */
} GCM_TAB;

/*! @brief The structure of the AES_GCM context */
typedef struct AES_GCM_struct {
  GCM_TAB tab;                              /*!< GCM acceleration table */
  EVP_CIPHER_CTX *ctx;                      /*!< Cipher context used by GCM */
  unsigned char ecount_buf[AES_BLOCK_SIZE]; /*!< Counter buffer */
  unsigned char eZero[AES_BLOCK_SIZE];      /*!< Counter buffer '0' encrypted with the key */
  unsigned char Y[AES_BLOCK_SIZE];          /*!< Retained data buffer (aad and data) */
  unsigned char H[AES_BLOCK_SIZE];          /*!< "magic" hash key */
  unsigned char R[AES_BLOCK_SIZE];          /*!< retained data from last partial block */
  unsigned char S[AES_BLOCK_SIZE];          /*!< Temporary buffer. Useful for debug. This only has "local to one function" scope */
  unsigned int  state;                      /*!< 0 no data, 1 have aad, 2 processed aad but not mixed in, 3 aad mixed in handling data */
  unsigned int  Rlen;                       /*!< length of last partial aad or data block */ 
  unsigned int  n;                          /*!< Block count */
  unsigned long LenA;                       /*!< Length of the AAD */
  unsigned long LenC;                       /*!< Length of the ciphertext */
  EVP_CIPHER_CTX *IVctx;                    /*!< 64 bit encryptor context */
  unsigned char IVcounter[8];               /*!< IV generator state buffer */
  unsigned long count;                      /*!< The number of times we've called the IV routine */
} AES_GCM_CTX_t;





                                                                           
#endif
