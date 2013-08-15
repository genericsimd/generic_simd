#include "mac_auth.h"
#include <stdio.h>

int mac_des_crypto(unsigned char* data,int datalen, unsigned char* ivec, unsigned char* key, u_char* output, int enc)
{
	//argument enc takes value: 1 for encrypt. 0 for decrypt

//This function takes in three strings- plaintex, iv and keystring. If the plaintext length is not a multiple of 8, it does residual termination, else it simply DES_ncbc encrypts the input data.

	int last_block_size,num_complete_blocks;
	u_char temp_otp[8];int i;
	DES_cblock tempkey;
	DES_key_schedule key_schedule;


	last_block_size = datalen%8;		//size of residue
	num_complete_blocks = (int)(datalen/8);  //number of blocks going into ncbc, i.e number of complete blocks.

	memcpy(tempkey,key,8);
	//DES_set_odd_parity(&tempkey);
	//Creating a key schedule out of the shared key.
	DES_set_key_unchecked(&tempkey,&key_schedule);

//Using key schedule to encrypt the complete blocks of the data. (i.e except the last residual block which has size less than 8 bytes).
	DES_ncbc_encrypt(data,output,num_complete_blocks*8,&key_schedule,(DES_cblock *)ivec,enc);
	if (last_block_size > 0)
	{
		//If there is a residual block, residual termination should be done as described in WiMAX Standard Rev2-D5 Chapter 7.5 using ECB Encryption.

		//Encrypt the last but one block using ECB to get the right One-time-pad.
		//If encryption is happening, this is in the array data, else it is in the array output.
		if (enc) DES_ecb_encrypt((const_DES_cblock *) (data+((num_complete_blocks-1)*8)),(DES_cblock *)temp_otp,&key_schedule,1);
		else DES_ecb_encrypt((const_DES_cblock *)(output+((num_complete_blocks-1)*8)),(DES_cblock *)temp_otp,&key_schedule,1);
		
		for(i=0;i<last_block_size;i++)
		{
			//XOR the OTP obtained with the last block (of raw data during encryption, or encrypted data during decryption).
			temp_otp[i] = *(data+(num_complete_blocks*8)+i)^temp_otp[i];
			
		}
		memcpy(output+(num_complete_blocks*8),temp_otp,last_block_size);
	}
	//Residual termination completed.
	return 1;
}
