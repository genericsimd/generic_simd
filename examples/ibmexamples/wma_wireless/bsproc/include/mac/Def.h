/******************************************************************************
 (C) Copyright IBM Corp. 2011 All Rights Reserved
     IoT Key Management
     Contact: jipeng@cn.ibm.com
*******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif
	
#ifndef _DEF_H_
#define _DEF_H_

#define _RESULT_OK_ 1
#define _MAXSIZE_BUF_1_ 1024

#define _RSA_KEY_PUBLIC_FILE_SUFFIX " _public.pem"
#define _RSA_KEY_PRIVATE_FILE_SUFFIX " _private.pem"
#define _X509_CERTIFICATE_FILE_SUFFIX ".crt"
#define _X509_CERTIFICATE_REQ_FILE_SUFFIX ".csr"
#define _X509_CA_PRIVATE_KEY_READ_PASSWORD "pw4test:"
#define _X509_PASSWORD_FOR_CERT_KEY "pw4test:"

#define _DAEMON_NAME_ "CMA"
//#define _DAEMON_INTERVAL_ (60*60*24-200)	//in seconds, interval is everyday, to prevent delaying, minus 200 seconds
#define _DAEMON_INTERVAL_ 10

//#define _OS_WINDOWS_
#define _OS_LINUX_

#ifdef _OS_WINDOWS_	
	#define _KEY_STORE_PATH_KEY "E:\\Works\\[Project]\\IOT\\Security\\_Key Management\\_Dev\\CACenter\\Debug\\IoTCA\\private\\"
	#define	_KEY_STORE_PATH_CERT "E:\\Works\\[Project]\\IOT\\Security\\_Key Management\\_Dev\\CACenter\\Debug\\IoTCA\\certs\\"	

	#define	_TRUST_STORE_PATH_CERT "E:\\Works\\[Project]\\IOT\\Security\\_Key Management\\_Dev\\CACenter\\Debug\\IoTCA\\certs\\"
	#define _TRUST_STORE_CA_CERT "E:\\Works\\[Project]\\IOT\\Security\\_Key Management\\_Dev\\CACenter\\Debug\\IoTCA\\cacert.crt"
	#define _TRUST_STORE_CA_PRIVATE "E:/Works/[Project]/IOT/Security/_Key Management/_Dev/CACenter/Debug/IoTCA/private/cakey.pem"

	#define _LOCAL_CRL_PATH_ "E:\\Works\\[Project]\\IOT\\Security\\_Key Management\\_Dev\\CACenter\\Debug\\IoTCA\\crl\\"
#endif
#ifdef _OS_LINUX_
	#define _KEY_STORE_PATH_KEY "/home/IOTCA/private/"
	#define	_KEY_STORE_PATH_CERT "/home/IOTCA/certs/"

	#define	_TRUST_STORE_PATH_CERT "/home/IOTCA/certs/"
	#define _TRUST_STORE_CA_CERT "/home/IOTCA/cacert.crt"
	#define _TRUST_STORE_CA_PRIVATE "/home/IOTCA/private/cakey.pem"

	#define _LOCAL_CRL_PATH_ "/home/IOTCA/crl/"	
#endif

#endif	/* _DEF_H_ */

#if defined(__cplusplus)
} /* extern "C" */
#endif
