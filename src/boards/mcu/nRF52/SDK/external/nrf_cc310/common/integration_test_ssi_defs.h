// Copyright (c) 2016-2017, ARM Limited or its affiliates. All rights reserved 
// 
// This file and the related binary are licensed under the ARM Object Code and 
// Headers License; you may not use these files except in compliance with this 
// license. 
// 
// You may obtain a copy of the License at <.../external/nrf_cc310/license.txt> 
// 
// See the License for the specific language governing permissions and 
// limitations under the License.
#define SHARED_SECRET_MAX_LENGHT         250
#define ECC_KEY_MAX_LENGHT               0x256
#define AES_KEY_MAX_LENGHT_IN_BYTES      0x10



#define HASH_TESTS_NUMBER                0x3
#define AES_TESTS_NUMBER                 0x4
#define AES_CCM_TESTS_NUMBER             0x1
#define ECC_SIGN_TESTS_NUMBER            0x2
#define ECC_DH_TESTS_NUMBER              0x2
#define CHACHA_TESTS_NUMBER              0xA
#define CHACHA_POLY_TESTS_NUMBER         0x2
#define HMAC_TESTS_NUMBER                0x2
#define RSA_ENCDEC_OEP_TESTS_NUMBER      0x2
#define RSA_SIGN_VERIFY_OEP_TESTS_NUMBER 0x1
#define HKDF_TESTS_NUMBER                0x8
#define OPERATIONS_NUMBER                0x2//Integrated/noniNtegrated


typedef enum OperationType_enum
{
	 INTEGRATED_OPERATION   ,
	 NON_INTEGRATED_OPERATION  ,
	 OperationTypeLast= 0x7FFFFFFF,

}OperationType_t;

typedef enum BufferType_enum
{
	 NON_INPLACE_BUFFER   ,
	 INPLACE_BUFFER  ,
	 BufferTypeLast= 0x7FFFFFFF,

}BufferType_t;

typedef enum RSAType_enum
{
	 NON_CRT_MODE   ,
	 CRT_MODE  ,
	 RSATypeLast= 0x7FFFFFFF,

}RSAType_enum;

