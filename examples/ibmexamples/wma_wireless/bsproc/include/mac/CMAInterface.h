/******************************************************************************
 (C) Copyright IBM Corp. 2011 All Rights Reserved
     IoT Key Management
     Contact: jipeng@cn.ibm.com
*******************************************************************************/

#ifndef _CMA_INTERFACE_H_
#define _CMA_INTERFACE_H_

#if defined(__cplusplus)
extern "C" {
#endif
	
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <openssl/ssl.h>
#include <openssl/bio.h> 
#include <openssl/conf.h> 
//#include <openssl/err.h> 
#include <openssl/pem.h> 
#include <openssl/x509.h>

#include "Def.h"

/*
 * The method is to verify the given certificate on its authenticity, expiration and revocation status
 * 1. check issuer
 * 2. check signature
 * 3. check expiration
 * 4. check revocation status
 * @cert, address of the input certficate in X.509 format
 * @return, if the certificate is valid, return 1 (_RESULT_OK_), otherwise return negative result which stands for detail error
 */
int CMA_Verify_Certificate(X509* cert);



X509* CMA_Get_Certificate_From_Stream(const u_char* certContent, int certContentLength);

/*
 * The method is to publish error message according to the given error code.
 * @errorCode, error code which should be negative
 * @return, error message if error code is valid, otherwise return null
 */
char* CMA_GetError(int errorCode);

#if defined(__cplusplus)
} /* extern "C" */
#endif
#endif	/* _CMA_INTERFACE_H_ */
