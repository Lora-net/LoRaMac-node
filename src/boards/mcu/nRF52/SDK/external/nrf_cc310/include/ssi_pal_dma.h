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



#ifndef _SSI_PAL_DMA_H
#define _SSI_PAL_DMA_H

/*!
@file
@brief This file contains definitions that are used for the DMA related APIs. The implementation of these functions
need to be replaced according to Platform and OS.
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "ssi_pal_types.h"
#include "ssi_pal_dma_plat.h"
#include "ssi_pal_dma_defs.h"

/*! User buffer scatter information. */
typedef struct {
	SaSiDmaAddr_t		blockPhysAddr;
	uint32_t		blockSize;
}SaSi_PalDmaBlockInfo_t;

#ifdef BIG__ENDIAN
#define  SET_WORD_LE(val) cpu_to_le32(val)
#else
#define  SET_WORD_LE
#endif

/**
 * @brief   This function is called by the ARM TrustZone CryptoCell TEE runtime library before the HW is used.
 *      It maps a given data buffer (virtual address) for ARM TrustZone CryptoCell TEE HW DMA use (physical address), and returns the list of
 *      one or more DMA-able (physical) blocks. It may lock the buffer for ARM TrustZone CryptoCell TEE HW use. Once it is called,
 *      only ARM TrustZone CryptoCell TEE HW access to the buffer is allowed, until it is unmapped.
 *      If the data buffer was already mapped by the Secure OS prior to calling the ARM TrustZone CryptoCell TEE runtime library,
 *      this API does not have to perform any actual mapping operation, but only return the list of DMA-able blocks.
 *
 * @return A non-zero value in case of failure.
 */
uint32_t SaSi_PalDmaBufferMap(uint8_t                	  *pDataBuffer, 	/*!< [in] Address of the buffer to map. */
			     uint32_t                     buffSize,		/*!< [in] Buffer size in bytes. */
			     SaSi_PalDmaBufferDirection_t  copyDirection,	/*!< [in] Copy direction of the buffer, according to ::SaSi_PalDmaBufferDirection_t,
											  <ul><li>TO_DEVICE - the original buffer is the input to the operation,
											  and this function should copy it to the temp buffer,
											  prior to the activating the HW on the temp buffer.</li>
											  <li>FROM_DEVICE - not relevant for this API.</li>
											  <li>BI_DIRECTION - used when the crypto operation is "in-place", meaning
											  the result of encryption or decryption is written over the original data
											  at the same address. Should be treated by this API same as
											  TO_DEVICE. </li></ul> */
			     uint32_t                     *pNumOfBlocks,	/*!< [in/out]  Maximum numOfBlocks to fill, as output the actual number. */
			     SaSi_PalDmaBlockInfo_t        *pDmaBlockList,	/*!< [out] List of DMA-able blocks that the buffer maps to. */
			     SaSi_PalDmaBufferHandle       *dmaBuffHandle	/*!< [out] A handle to the mapped buffer private resources.*/ );


/**
 * @brief   This function is called by the ARM TrustZone CryptoCell TEE runtime library after the HW is used.
 *  	It unmaps a given buffer, and frees its associated resources, if needed. It may unlock the buffer and flush it for CPU use.
 *  	Once it is called, ARM TrustZone CryptoCell TEE HW does not require access to this buffer anymore.
 *  	If the data buffer was already mapped by the Secure OS prior to calling the ARM TrustZone CryptoCell TEE runtime library, this API does
 *  	not have to perform any un-mapping operation, and the actual un-mapping can be done by the Secure OS outside the context
 *  	of the ARM TrustZone CryptoCell TEE runtime library.
 * @return A non-zero value in case of failure.
 */
uint32_t SaSi_PalDmaBufferUnmap(uint8_t                	     *pDataBuffer,	/*!< [in] Address of the buffer to unmap. */
			        uint32_t                     buffSize,		/*!< [in] Buffer size in bytes. */
			        SaSi_PalDmaBufferDirection_t copyDirection,	/*!< [in] Copy direction of the buffer, according to ::SaSi_PalDmaBufferDirection_t
											  <ul><li>TO_DEVICE - not relevant for this API. </li>
											  <li>FROM_DEVICE - the temp buffer holds the output of the HW, and this
											  API should copy it to the actual output buffer.</li>
											  <li>BI_DIRECTION - used when the crypto operation is "in-place", meaning
											  the result of encryption or decryption is written over the original data
											  at the same address. Should be treated by this API same as
											  FROM_DEVICE.</li></ul> */
			        uint32_t                     numOfBlocks,	/*!< [in] Number of DMA-able blocks that the buffer maps to. */
			        SaSi_PalDmaBlockInfo_t       *pDmaBlockList,	/*!< [in] List of DMA-able blocks that the buffer maps to. */
			        SaSi_PalDmaBufferHandle      dmaBuffHandle	/*!< [in] A handle to the mapped buffer private resources. */);


/**
 * @brief Allocates a DMA-contiguous buffer for CPU use, and returns its virtual address.
 * 	Before passing the buffer to the ARM TrustZone CryptoCell TEE HW, ::SaSi_PalDmaBufferMap should be called.
 * 	\note The returned address must be aligned to 32 bits.
 *
 *
 * @return A non-zero value in case of failure.
 */
uint32_t SaSi_PalDmaContigBufferAllocate(uint32_t          buffSize, /*!< [in] Buffer size in bytes.*/
					 uint8_t           **ppVirtBuffAddr /*!< [out]  Virtual address of the allocated buffer.*/);



/**
 * @brief Frees resources previously allocated by ::SaSi_PalDmaContigBufferAllocate.
 *
 *
 * @return A non-zero value in case of failure.
 */
uint32_t SaSi_PalDmaContigBufferFree(uint32_t          buffSize, /*!< [in] Buffer size in Bytes. */
				     uint8_t           *pVirtBuffAddr /*!< [in] Virtual address of the buffer to free. */);



/**
 * @brief Checks whether the buffer is guaranteed to be a single contiguous DMA block.
 *
 *
 * @return Returns TRUE if the buffer is guaranteed to be a single contiguous DMA block, and FALSE otherwise.
 */
uint32_t SaSi_PalIsDmaBufferContiguous(uint8_t                	  *pDataBuffer, /*!< [in] User buffer address. */
				       uint32_t                    buffSize   /*!< [in] User buffer size. */);


/**
 * @brief Maps virtual address to physical address.
 *
 *
 * @return Physical address.
 */
SaSiDmaAddr_t SaSi_PalMapVirtualToPhysical(uint8_t *pVirtualAddr /*!< [in] Pointer to virtual address. */);


#ifdef __cplusplus
}
#endif

#endif


