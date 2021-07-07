/**
 * \file
 *
 * \brief SAM Read While Write EEPROM Emulator
 *
 * Copyright (c) 2014-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include "rww_eeprom.h"
#include <string.h>
#include <nvm.h>
#include "conf_rwwee.h"

#if !defined(CONF_LOGICAL_PAGE_NUM_IN_ROW) || defined(__DOXYGEN__)
#  warning  CONF_LOGICAL_PAGE_NUM_IN_ROW is not defined, assuming RWWEE_LOGICAL_PAGE_NUM_1.

/** Configuration option, one logical page stored in a physical row.
 */
#  define CONF_LOGICAL_PAGE_NUM_IN_ROW  RWWEE_LOGICAL_PAGE_NUM_1
#endif

#if !defined(CONF_PAGE_CHECKSUM_ENABLE) || defined(__DOXYGEN__)
#  warning  CONF_PAGE_CHECKSUM_ENABLE is not defined, assuming false.

/** Configuration option, page checksum enabled.
 */
#  define CONF_PAGE_CHECKSUM_ENABLE  false
#endif

/**
 * \internal
 * Magic key is the sequence "AtEEPROMEmu_RWW." in ASCII. The key is encoded as a
 * sequence of 32-bit values to speed up checking of the key, which can be
 * implemented as a number of simple integer comparisons,
 */
#define RWW_EEPROM_MAGIC_KEY                 {0x41744545, 0x50524f4d, 0x456d752d,0x5257572e}

/** \internal
 *  Length of the magic key, in 32-bit elements.
 */
#define RWW_EEPROM_MAGIC_KEY_COUNT           4

/** \internal
 *  RWW EEPROM max logical page num.
 */
#define RWW_EEPROM_MAX_LOGICAL_PAGES (((NVMCTRL_RWWEE_PAGES - (2 * NVMCTRL_ROW_PAGES)) \
										/ NVMCTRL_ROW_PAGES) * CONF_LOGICAL_PAGE_NUM_IN_ROW)

COMPILER_PACK_SET(1);
/**
 * \internal
 * \brief Structure describing the EEPROM Emulation master page.
 */
struct _rww_eeprom_master_page {
	/** Magic key which in ASCII will show as AtEEPROMEmu_RWW. */
	uint32_t magic_key[RWW_EEPROM_MAGIC_KEY_COUNT];

	/** Emulator major version information. */
	uint8_t  major_version;
	/** Emulator minor version information. */
	uint8_t  minor_version;
	/** Emulator revision version information. */
	uint8_t  revision;

	/** Emulator identification value (to distinguish between different emulator
	 *  schemes that carry the same version numbers). */
	uint8_t  emulator_id;

	/** Unused reserved bytes in the master page. */
	uint8_t  reserved[44];
};

/**
 * \internal
 * \brief Structure describing emulated pages of RWW EEPROM data.
 */
struct _rww_eeprom_page {
	/** Header information of the RWW EEPROM page. */
	struct {
		uint8_t logical_page;
		uint8_t page_checksum;
		uint8_t reserved[RWW_EEPROM_HEADER_SIZE - 2];
	} header;

	/** Data content of the RWW EEPROM page. */
	uint8_t data[RWW_EEPROM_PAGE_SIZE];
};
COMPILER_PACK_RESET();

/**
 * \internal
 * \brief Internal device instance struct.
 */
struct _rww_eeprom_module {
	/** Initialization state of the RWW EEPROM emulator. */
	bool initialized;

	/** Absolute byte pointer to the first byte of memory where the emulated
	 *  RWW EEPROM is stored. */
	const struct _rww_eeprom_page *rwwee_addr;

	/** Number of physical pages occupied by the RWW EEPROM emulator. */
	uint16_t physical_pages;
	/** Number of logical pages occupied by the RWW EEPROM emulator. */
	uint8_t  logical_pages;

	/** Mapping array from logical RWW EEPROM pages to physical pages. */
	uint8_t page_map[RWW_EEPROM_MAX_LOGICAL_PAGES];

	/** Row number for the spare row (used by next write). */
	uint8_t spare_row;

	/** Buffer to hold the currently cached page. */
	struct _rww_eeprom_page cache;
	/** Indicates if the cache contains valid data. */
	bool cache_active;
};

/**
 * \internal
 * \brief Internal RWW EEPROM emulator instance.
 */
static struct _rww_eeprom_module _eeprom_instance = {
	.initialized = false,
};

/** \internal
 *  \brief RWW EEPROM Page Checksum.
 *
 *  \param[in] buffer Pointer of page data
 *
 * \return Checksum result.
 */
static uint8_t _rww_eeprom_emulator_page_checksum(
		const uint8_t * buffer)
{
	uint16_t sum=0;
	for(uint8_t i = 0; i < RWW_EEPROM_PAGE_SIZE;i++){
		sum += buffer[i];
	}
	return (uint8_t)(sum & 0xff);
}

/** \internal
 *  \brief Erases a given row within the physical RWW EEPROM memory space.
 *
 *  \param[in] row  Physical row in RWW EEPROM space to erase
 */
static void _rww_eeprom_emulator_nvm_erase_row(
		const uint8_t row)
{
	enum status_code error_code = STATUS_OK;

	do {
		error_code = nvm_erase_row(
				(uint32_t)&_eeprom_instance.rwwee_addr[row * NVMCTRL_ROW_PAGES]);
	} while (error_code == STATUS_BUSY);
}

/** \internal
 *  \brief Fills the internal NVM controller page buffer in physical RWW EEPROM memory space.
 *
 *  \param[in] physical_page  Physical page in RWW EEPROM space to fill
 *  \param[in] data           Data to write to the physical memory page
 */
static void _rww_eeprom_emulator_nvm_fill_cache(
		const uint16_t physical_page,
		const void* const data)
{
	enum status_code error_code = STATUS_OK;

	if (CONF_PAGE_CHECKSUM_ENABLE && (_eeprom_instance.initialized)){
		struct _rww_eeprom_page *temp = (struct _rww_eeprom_page *)data;
		temp->header.page_checksum = _rww_eeprom_emulator_page_checksum(temp->data);
	}

	do {
		error_code = nvm_write_buffer(
				(uint32_t)&_eeprom_instance.rwwee_addr[physical_page],
				(uint8_t*)data,
				NVMCTRL_PAGE_SIZE);
	} while (error_code == STATUS_BUSY);
}

/** \internal
 *  \brief Commits the internal NVM controller page buffer to physical memory.
 *
 *  \param[in] physical_page  Physical page in RWW EEPROM space to commit
 */
static void _rww_eeprom_emulator_nvm_commit_cache(
		const uint16_t physical_page)
{
	enum status_code error_code = STATUS_OK;

	do {
		error_code = nvm_execute_command(
				NVM_COMMAND_RWWEE_WRITE_PAGE,
				(uint32_t)&_eeprom_instance.rwwee_addr[physical_page], 0);
	} while (error_code == STATUS_BUSY);
}

/** \internal
 *  \brief Reads a page of data stored in physical RWW EEPROM memory space.
 *
 *  \param[in]  physical_page  Physical page in RWW EEPROM space to read
 *  \param[out] data           Destination buffer to fill with the read data
 *
 * \return Whether the page data checksum is correct.
 *
 * \retval \c true   The page data checksum is correct
 * \retval \c false  The page data checksum is wrong
 */
static bool _rww_eeprom_emulator_nvm_read_page(
		const uint16_t physical_page,
		void* const data)
{
	enum status_code error_code = STATUS_OK;

	do {
		error_code = nvm_read_buffer(
				(uint32_t)&_eeprom_instance.rwwee_addr[physical_page],
				(uint8_t*)data,
				NVMCTRL_PAGE_SIZE);
	} while (error_code == STATUS_BUSY);

	if (CONF_PAGE_CHECKSUM_ENABLE && (_eeprom_instance.initialized)){
		struct _rww_eeprom_page *temp= (struct _rww_eeprom_page *) data;
		if (temp->header.page_checksum ==
			_rww_eeprom_emulator_page_checksum(temp->data)){
			return true;
		} else {
			return false;
		}
	}
	return true;
}

/**
 * \brief Initializes the emulated RWW EEPROM memory, destroying the current contents.
 */
static void _rww_eeprom_emulator_format_memory(void)
{
	uint16_t logical_page = 0;

	/* Set row 0 as the spare row */
	_eeprom_instance.spare_row = 0;
	_rww_eeprom_emulator_nvm_erase_row(_eeprom_instance.spare_row);

	for (uint16_t physical_page = NVMCTRL_ROW_PAGES;
			physical_page < _eeprom_instance.physical_pages; physical_page++) {

		if (physical_page == RWW_EEPROM_MASTER_PAGE_NUMBER) {
			continue;
		}

		/* If we are at the first page in a new row, erase the entire row */
		if ((physical_page % NVMCTRL_ROW_PAGES) == 0) {
			_rww_eeprom_emulator_nvm_erase_row(physical_page / NVMCTRL_ROW_PAGES);
		}

		/* One or two logical pages are stored in each physical row; program in a
		 * pair of initialized but blank set of emulated RWW EEPROM pages */
		if ((physical_page % NVMCTRL_ROW_PAGES) < CONF_LOGICAL_PAGE_NUM_IN_ROW) {
			/* Make a buffer to hold the initialized EEPROM page */
			struct _rww_eeprom_page data;
			memset(&data, 0xFF, sizeof(data));

			/* Set up the new EEPROM row's header */
			data.header.logical_page = logical_page;

			/* Write the page out to physical memory */
			_rww_eeprom_emulator_nvm_fill_cache(physical_page, &data);
			_rww_eeprom_emulator_nvm_commit_cache(physical_page);

			/* Increment the logical RWW EEPROM page address now that the current
			 * address' page has been initialized */
			logical_page++;
		}
	}
}

/**
 * \brief Check if a row is a full row
 *  because the page is a invalid page, so if two pages have data,
 *  it is the full row.
 *
 *  \param[in]  phy_page  Physical page that in a row
 */
static bool _rww_eeprom_emulator_is_full_row(uint16_t phy_page)
{
	if (CONF_LOGICAL_PAGE_NUM_IN_ROW == RWWEE_LOGICAL_PAGE_NUM_1) {
		if(_eeprom_instance.rwwee_addr[phy_page].header.logical_page
			== _eeprom_instance.rwwee_addr[phy_page+1].header.logical_page) {
			return true;
		} else {
			return false;
		}
	} else {
		if((_eeprom_instance.rwwee_addr[phy_page].header.logical_page
			== _eeprom_instance.rwwee_addr[phy_page+2].header.logical_page)
			|| (_eeprom_instance.rwwee_addr[phy_page+1].header.logical_page
			== _eeprom_instance.rwwee_addr[phy_page+2].header.logical_page )) {
			return true;
		} else {
			return false;
		}
	}
}

/**
 * \brief Erase one invalid page according to two invalid physical page
 *
 *  \param[in]  pre_phy_page  One physical invalid page
 *  \param[in]  next_phy_page Another physical invalid page
 */
static void _rww_eeprom_emulator_erase_invalid_page(uint16_t pre_phy_page,uint16_t next_phy_page)
{
	bool flag;
	struct _rww_eeprom_page temp;
	flag =  _rww_eeprom_emulator_is_full_row(pre_phy_page);
	 if(CONF_PAGE_CHECKSUM_ENABLE) {
	 	if(flag) {
			/* If the new row checksum is ok, erase the old one*/
			if(_rww_eeprom_emulator_nvm_read_page(next_phy_page,&temp)) {
				_rww_eeprom_emulator_nvm_erase_row(pre_phy_page/4);
			} else {
				_rww_eeprom_emulator_nvm_erase_row(next_phy_page/4);
			}
		} else {
			if(_rww_eeprom_emulator_nvm_read_page(pre_phy_page,&temp)) {
				_rww_eeprom_emulator_nvm_erase_row(next_phy_page/4);
			} else {
				_rww_eeprom_emulator_nvm_erase_row(pre_phy_page/4);
			}
		}
	 } else {
	 	/* Erase the old/full row*/
		if(flag) {
			 _rww_eeprom_emulator_nvm_erase_row(pre_phy_page/4);
		} else {
			_rww_eeprom_emulator_nvm_erase_row(next_phy_page/4);
		}
	 }
}

/**
 * \brief Check if there exist rows with same logical pages due to power drop
 *  when writing or erasing page.
 *  when existed same logical page, if the new row checksums is valid,
 *  the old(full) row will be erased, or the new row will be erased.
 *  If page checksum disabled, the old(full) row will be erased.
 */
static void _rww_eeprom_emulator_check_logical_page(void)
{
	uint16_t i = 0, j = 0;
	for (i = 0; i < _eeprom_instance.physical_pages; i=i+4) {

		uint16_t pre_logical_page = _eeprom_instance.rwwee_addr[i].header.logical_page;
		if( pre_logical_page == RWW_EEPROM_INVALID_PAGE_NUMBER) {
			continue;
		}

		for (j = NVMCTRL_ROW_PAGES+i; j < _eeprom_instance.physical_pages; j=j+4) {

			if (j == RWW_EEPROM_MASTER_PAGE_NUMBER) {
				continue;
			}
			uint16_t next_logical_page = _eeprom_instance.rwwee_addr[j].header.logical_page;
			if( next_logical_page == RWW_EEPROM_INVALID_PAGE_NUMBER) {
				continue;
			}

			if(pre_logical_page == next_logical_page) {
				/* Found invalid logical page and erase it */
				_rww_eeprom_emulator_erase_invalid_page(i,j);
			}
		}
	}
}

/**
 * \brief Creates a map in SRAM to translate logical RWW EEPROM pages to physical pages.
 */
static void _rww_eeprom_emulator_update_page_mapping(void)
{
	/* Check if exists invalid logical page */
	_rww_eeprom_emulator_check_logical_page();

	/* Scan through all physical pages, to map physical and logical pages */
	for (uint16_t c = 0; c < _eeprom_instance.physical_pages; c++) {
		if (c == RWW_EEPROM_MASTER_PAGE_NUMBER) {
			continue;
		}

		/* Read in the logical page stored in the current physical page */
		uint16_t logical_page = _eeprom_instance.rwwee_addr[c].header.logical_page;

		/* If the logical page number is valid, add it to the mapping */
		if ((logical_page != RWW_EEPROM_INVALID_PAGE_NUMBER) &&
				(logical_page < _eeprom_instance.logical_pages)) {
			_eeprom_instance.page_map[logical_page] = c;
		}
	}

	/* Use an invalid page number as the spare row until a valid one has been
	 * found */
	_eeprom_instance.spare_row = RWW_EEPROM_INVALID_ROW_NUMBER;

	/* Scan through all physical rows, to find an erased row to use as the
	 * spare */
	for (uint16_t c = 0; c < (_eeprom_instance.physical_pages / NVMCTRL_ROW_PAGES); c++) {
		bool spare_row_found = true;

		/* Look through pages within the row to see if they are all erased */
		for (uint8_t c2 = 0; c2 < NVMCTRL_ROW_PAGES; c2++) {
			uint16_t physical_page = (c * NVMCTRL_ROW_PAGES) + c2;

			if (physical_page == RWW_EEPROM_MASTER_PAGE_NUMBER) {
				continue;
			}

			if (_eeprom_instance.rwwee_addr[physical_page].header.logical_page !=
					RWW_EEPROM_INVALID_PAGE_NUMBER) {
				spare_row_found = false;
			}
		}

		/* If we've now found the spare row, store it and abort the search */
		if (spare_row_found == true) {
			_eeprom_instance.spare_row = c;
			break;
		}
	}
}

/**
 * \brief Finds the next free page in the given row if one is available.
 *
 * \param[in]  start_physical_page  Physical page index of the row to
 *                                  search
 * \param[out] free_physical_page   Index of the physical page that is
 *                                  currently free (if one was found)
 *
 * \return Whether a free page was found in the specified row.
 *
 * \retval \c true   If a free page was found
 * \retval \c false  If the specified row was full and needs an erase
 */
static bool _rww_eeprom_emulator_is_page_free_on_row(
		const uint8_t start_physical_page,
		uint8_t *const free_physical_page)
{
	/* Convert physical page number to a RWWEE row and page within the row */
	uint8_t row         = (start_physical_page / NVMCTRL_ROW_PAGES);
	uint8_t page_in_row = (start_physical_page % NVMCTRL_ROW_PAGES);

	/* Look in the current row for a page that isn't currently used */
	for (uint8_t c = page_in_row; c < NVMCTRL_ROW_PAGES; c++) {
		/* Calculate the page number for the current page being examined */
		uint8_t page = (row * NVMCTRL_ROW_PAGES) + c;

		/* If the page is free, pass it to the caller and exit */
		if (_eeprom_instance.rwwee_addr[page].header.logical_page ==
				RWW_EEPROM_INVALID_PAGE_NUMBER) {
			*free_physical_page = page;
			return true;
		}
	}

	/* No free page in the current row was found */
	return false;
}

/**
 * \brief Moves data from the specified logical page to the spare row.
 *
 * Moves the contents of the specified row into the spare row, so that the
 * original row can be erased and re-used. The contents of the given logical
 * page is replaced with a new buffer of data.
 *
 * \param[in] row_number    Physical row to examine
 * \param[in] logical_page  Logical RWW EEPROM page number in the row to update
 * \param[in] data          New data to replace the old in the logical page
 *
 * \return Status code indicating the status of the operation.
 */
static enum status_code _rww_eeprom_emulator_move_data_to_spare(
		const uint8_t row_number,
		const uint8_t logical_page,
		const uint8_t *const data)
{
	enum status_code error_code = STATUS_OK;
	struct {
		uint8_t logical_page;
		uint8_t physical_page;
	} page_trans[CONF_LOGICAL_PAGE_NUM_IN_ROW];

	const struct _rww_eeprom_page *row_data =
			(struct _rww_eeprom_page *)&_eeprom_instance.rwwee_addr[row_number * NVMCTRL_ROW_PAGES];

	/* There should be one or two logical pages of data in each row, possibly with
	 * multiple revisions (right-most version is the newest). Start by assuming
	 * the left-most two pages contain the newest page revisions. */
	for (uint8_t i = 0; i < CONF_LOGICAL_PAGE_NUM_IN_ROW; i++){
		page_trans[i].logical_page  = row_data[i].header.logical_page;
		page_trans[i].physical_page = (row_number * NVMCTRL_ROW_PAGES) + i;
	}

	/* Look for newer revisions of the one or two logical pages stored in the row */
	for (uint8_t c = 0; c < CONF_LOGICAL_PAGE_NUM_IN_ROW; c++) {
		/* Look through the remaining pages in the row for any newer revisions */
		for (uint8_t c2 = CONF_LOGICAL_PAGE_NUM_IN_ROW; c2 < NVMCTRL_ROW_PAGES; c2++) {
			if (page_trans[c].logical_page == row_data[c2].header.logical_page) {
				page_trans[c].physical_page =
						(row_number * NVMCTRL_ROW_PAGES) + c2;
			}
		}
	}

	/* Need to move saved logical pages stored in the same row */
	for (uint8_t c = 0; c < CONF_LOGICAL_PAGE_NUM_IN_ROW; c++) {
		/* Find the physical page index for the new spare row pages */
		uint32_t new_page =
				((_eeprom_instance.spare_row * NVMCTRL_ROW_PAGES) + c);

		/* Commit any cached data to physical non-volatile memory */
		rww_eeprom_emulator_commit_page_buffer();

		/* Check if we we are looking at the page the calling function wishes
		 * to change during the move operation */
		if (logical_page == page_trans[c].logical_page) {
			/* Fill out new (updated) logical page's header in the cache */
			_eeprom_instance.cache.header.logical_page = logical_page;

			/* Write data to SRAM cache */
			memcpy(_eeprom_instance.cache.data, data, RWW_EEPROM_PAGE_SIZE);
		} else {
			/* Copy existing RWW EEPROM page to cache buffer wholesale */
			_rww_eeprom_emulator_nvm_read_page(
					page_trans[c].physical_page, &_eeprom_instance.cache);
		}

		/* Fill the physical NVM buffer with the new data so that it can be
		 * quickly committed in the future if needed due to a low power
		 * condition */
		_rww_eeprom_emulator_nvm_fill_cache(new_page, &_eeprom_instance.cache);

		/* Update the page map with the new page location and indicate that
		 * the cache now holds new data */
		_eeprom_instance.page_map[page_trans[c].logical_page] = new_page;
		_eeprom_instance.cache_active = true;
	}
#ifdef SAMD21_64K
	rww_eeprom_emulator_commit_page_buffer();
#endif
	/* Erase the row that was moved and set it as the new spare row */
	_rww_eeprom_emulator_nvm_erase_row(row_number);

	/* Keep the index of the new spare row */
	_eeprom_instance.spare_row = row_number;

	return error_code;
}

/**
 * \brief Create master emulated RWW EEPROM management page.
 *
 * Creates a new master page in emulated RWW EEPROM, giving information on the
 * emulator used to store the RWW EEPROM data.
 */
static void _rww_eeprom_emulator_create_master_page(void)
{
	const uint32_t magic_key[] = RWW_EEPROM_MAGIC_KEY;

	struct _rww_eeprom_master_page master_page;
	memset(&master_page, 0xFF, sizeof(master_page));

	/* Fill out the magic key header to indicate an initialized master page */
	for (uint8_t c = 0; c < RWW_EEPROM_MAGIC_KEY_COUNT; c++) {
		master_page.magic_key[c] = magic_key[c];
	}

	/* Update master header with version information of this emulator */
	master_page.emulator_id   = RWW_EEPROM_EMULATOR_ID;
	master_page.major_version = RWW_EEPROM_MAJOR_VERSION;
	master_page.minor_version = RWW_EEPROM_MINOR_VERSION;
	master_page.revision      = RWW_EEPROM_REVISION;

	_rww_eeprom_emulator_nvm_erase_row(
			RWW_EEPROM_MASTER_PAGE_NUMBER / NVMCTRL_ROW_PAGES);

	/* Write the new master page data to physical memory */
	_rww_eeprom_emulator_nvm_fill_cache(RWW_EEPROM_MASTER_PAGE_NUMBER, &master_page);
	_rww_eeprom_emulator_nvm_commit_cache(RWW_EEPROM_MASTER_PAGE_NUMBER);
}

/**
 * \brief Verify the contents of a master RWW EEPROM page.
 *
 * Verify the contents of a master RWW EEPROM page to ensure that it contains the
 * correct information for this version of the RWW EEPROM emulation service.
 *
 * \retval STATUS_OK              Given master page contents is valid
 * \retval STATUS_ERR_BAD_FORMAT  Master page contents was invalid
 * \retval STATUS_ERR_IO          Master page indicates the data is incompatible
 *                                with this version of the RWW EEPROM emulator
 */
static enum status_code _rww_eeprom_emulator_verify_master_page(void)
{
	const uint32_t magic_key[] = RWW_EEPROM_MAGIC_KEY;
	struct _rww_eeprom_master_page master_page;

	/* Copy the master page to the RAM buffer so that it can be inspected */
	_rww_eeprom_emulator_nvm_read_page(RWW_EEPROM_MASTER_PAGE_NUMBER, &master_page);

	/* Verify magic key is correct in the master page header */
	for (uint8_t c = 0; c < RWW_EEPROM_MAGIC_KEY_COUNT; c++) {
		if (master_page.magic_key[c] != magic_key[c]) {
			return STATUS_ERR_BAD_FORMAT;
		}
	}

	/* Verify emulator ID in header to ensure the same scheme is used */
	if (master_page.emulator_id   != RWW_EEPROM_EMULATOR_ID) {
		return STATUS_ERR_IO;
	}

	/* Verify major version in header to ensure the same version is used */
	if (master_page.major_version != RWW_EEPROM_MAJOR_VERSION) {
		return STATUS_ERR_IO;
	}

	/* Verify minor version in header to ensure the same version is used */
	if (master_page.minor_version != RWW_EEPROM_MINOR_VERSION) {
		return STATUS_ERR_IO;
	}

	/* Don't verify revision number - same major/minor is considered enough
	 * to ensure the stored data is compatible. */

	return STATUS_OK;
}


/**
 * \brief Retrieves the parameters of the RWW EEPROM Emulator memory layout.
 *
 * Retrieves the configuration parameters of the RWW EEPROM Emulator, after it has
 * been initialized.
 *
 * \param[out] parameters  RWW EEPROM Emulator parameter struct to fill
 *
 * \return Status of the operation.
 *
 * \retval STATUS_OK                    If the emulator parameters were retrieved
 *                                      successfully
 * \retval STATUS_ERR_NOT_INITIALIZED   If the RWW EEPROM Emulator is not initialized
 */
enum status_code rww_eeprom_emulator_get_parameters(
	struct rww_eeprom_emulator_parameters *const parameters)
{
	if (_eeprom_instance.initialized == false) {
		return STATUS_ERR_NOT_INITIALIZED;
	}

	parameters->page_size              = RWW_EEPROM_PAGE_SIZE;
	parameters->eeprom_number_of_pages = _eeprom_instance.logical_pages;

	return STATUS_OK;
}

/**
 * \brief Initializes the RWW EEPROM Emulator service.
 *
 * Initializes the emulated RWW EEPROM memory space. If the emulated RWW EEPROM memory
 * has not been previously initialized, it will need to be explicitly formatted
 * via \ref rww_eeprom_emulator_erase_memory(). The RWW EEPROM memory space will \b not
 * be automatically erased by the initialization function. Partial data
 * may be recovered by the user application manually if the service is unable to
 * initialize successfully.
 *
 * \return Status code indicating the status of the operation.
 *
 * \retval STATUS_OK              RWW EEPROM emulation service was successfully
 *                                initialized
 * \retval STATUS_ERR_BAD_FORMAT  Emulated RWW EEPROM memory is corrupt or not
 *                                formatted
 * \retval STATUS_ERR_IO          RWW EEPROM data is incompatible with this version
 *                                or scheme of the RWW EEPROM emulator
 * \retval STATUS_ERR_INVALID_ARG Invalid logical page configuration
 */
enum status_code rww_eeprom_emulator_init(void)
{
	enum status_code error_code = STATUS_OK;
	struct nvm_config config;
	struct nvm_parameters parameters;

	/* Mark initialization as start */
	_eeprom_instance.initialized = false;

	if (!(CONF_LOGICAL_PAGE_NUM_IN_ROW == RWWEE_LOGICAL_PAGE_NUM_1 || 
		CONF_LOGICAL_PAGE_NUM_IN_ROW == RWWEE_LOGICAL_PAGE_NUM_2)){
		return STATUS_ERR_INVALID_ARG;
	}
	/* Retrieve the NVM controller configuration - enable manual page writing
	 * mode so that the emulator has exclusive control over page writes to
	 * allow for caching */
	nvm_get_config_defaults(&config);
	config.manual_page_write = true;

	/* Apply new NVM configuration */
	do {
		error_code = nvm_set_config(&config);
	} while (error_code == STATUS_BUSY);

	/* Get the NVM controller configuration parameters */
	nvm_get_parameters(&parameters);

	/* Configure the RWW EEPROM instance physical and logical number of pages:
	 *  - One row is reserved for the master page
	 *  - One row is reserved for the spare row
	 *  - One or two logical pages can be stored in one physical row
	 */
	_eeprom_instance.physical_pages =
			parameters.rww_eeprom_number_of_pages;
	_eeprom_instance.logical_pages  = RWW_EEPROM_MAX_LOGICAL_PAGES;

	/* Configure the RWW EEPROM instance starting physical address and
	 * pre-compute the index of the first page used for RWW EEPROM */
	_eeprom_instance.rwwee_addr = (void*)NVMCTRL_RWW_EEPROM_ADDR;

	/* Clear RWW EEPROM page write cache on initialization */
	_eeprom_instance.cache_active = false;

	/* Scan physical memory and re-create logical to physical page mapping
	 * table to locate logical pages of RWW EEPROM data */
	_rww_eeprom_emulator_update_page_mapping();

	/* Could not find spare row - abort as the memory appears to be corrupt */
	if (_eeprom_instance.spare_row == RWW_EEPROM_INVALID_ROW_NUMBER) {
		return STATUS_ERR_BAD_FORMAT;
	}

	/* Verify that the master page contains valid data for this service */
	error_code = _rww_eeprom_emulator_verify_master_page();
	if (error_code != STATUS_OK) {
		return error_code;
	}

	/* Mark initialization as complete */
	_eeprom_instance.initialized = true;

	return error_code;
}

/**
 * \brief Erases the entire emulated RWW EEPROM memory space.
 *
 * Erases and re-initializes the emulated RWW EEPROM memory space, destroying any
 * existing data.
 */
void rww_eeprom_emulator_erase_memory(void)
{
	/* Create new RWW EEPROM memory block in EEPROM emulation section */
	_rww_eeprom_emulator_format_memory();

	bool temp = _eeprom_instance.initialized;

	/** Master page should be created during uninitialized status*/
	_eeprom_instance.initialized = false;

	/* Write RWW EEPROM emulation master block */
	_rww_eeprom_emulator_create_master_page();
	_eeprom_instance.initialized = temp;

	/* Map the newly created RWW EEPROM memory block */
	_rww_eeprom_emulator_update_page_mapping();
}

/**
 * \brief Writes a page of data to an emulated RWW EEPROM memory page.
 *
 * Writes an emulated RWW EEPROM page of data to the emulated RWW EEPROM memory space.
 *
 * \note Data stored in pages may be cached in volatile RAM memory; to commit
 *       any cached data to physical non-volatile memory, the
 *       \ref rww_eeprom_emulator_commit_page_buffer() function should be called.
 *
 * \param[in] logical_page  Logical RWW EEPROM page number to write to
 * \param[in] data          Pointer to the data buffer containing source data to
 *                          write
 *
 * \return Status code indicating the status of the operation.
 *
 * \retval STATUS_OK                    If the page was successfully read
 * \retval STATUS_ERR_NOT_INITIALIZED   If the RWW EEPROM emulator is not initialized
 * \retval STATUS_ERR_BAD_ADDRESS       If an address outside the valid emulated
 *                                      RWW EEPROM memory space was supplied
 */
enum status_code rww_eeprom_emulator_write_page(
		const uint8_t logical_page,
		const uint8_t *const data)
{
	/* Ensure the emulated RWW EEPROM has been initialized first */
	if (_eeprom_instance.initialized == false) {
		return STATUS_ERR_NOT_INITIALIZED;
	}

	/* Make sure the write address is within the allowable address space */
	if (logical_page >= _eeprom_instance.logical_pages) {
		return STATUS_ERR_BAD_ADDRESS;
	}

	/* Check if the cache is active and the currently cached page is not the
	 * page that is being written (if not, we need to commit and cache the new
	 * page) */
	if ((_eeprom_instance.cache_active == true) &&
			(_eeprom_instance.cache.header.logical_page != logical_page)) {
		/* Commit the currently cached data buffer to non-volatile memory */
		rww_eeprom_emulator_commit_page_buffer();
	}

	/* Check if we have space in the current page location's physical row for
	 * a new version, and if so get the new page index */
	uint8_t new_page = 0;
	bool page_spare  = _rww_eeprom_emulator_is_page_free_on_row(
			_eeprom_instance.page_map[logical_page], &new_page);

	/* Check if the current row is full, and we need to swap it out with a
	 * spare row */
	if (page_spare == false) {
		/* Move the other page we aren't writing that is stored in the same
		 * page to the new row, and replace the old current page with the
		 * new page contents (cache is updated to match) */
		_rww_eeprom_emulator_move_data_to_spare(
				_eeprom_instance.page_map[logical_page] / NVMCTRL_ROW_PAGES,
				logical_page,
				data);

		/* New data is now written and the cache is updated, exit */
		return STATUS_OK;
	}

	/* Update the page cache header section with the new page header */
	_eeprom_instance.cache.header.logical_page = logical_page;

	/* Update the page cache contents with the new data */
	memcpy(&_eeprom_instance.cache.data,
			data,
			RWW_EEPROM_PAGE_SIZE);

	/* Fill the physical NVM buffer with the new data so that it can be quickly
	 * committed in the future if needed due to a low power condition */
	_rww_eeprom_emulator_nvm_fill_cache(new_page, &_eeprom_instance.cache);

	/* Update the cache parameters and mark the cache as active */
	_eeprom_instance.page_map[logical_page] = new_page;
	barrier(); // Enforce ordering to prevent incorrect cache state
	_eeprom_instance.cache_active           = true;

	return STATUS_OK;
}

/**
 * \brief Reads a page of data from an emulated RWW EEPROM memory page.
 *
 * Reads an emulated RWW EEPROM page of data from the emulated RWW EEPROM memory space.
 *
 * \param[in]  logical_page  Logical RWW EEPROM page number to read from
 * \param[out] data          Pointer to the destination data buffer to fill
 *
 * \return Status code indicating the status of the operation.
 *
 * \retval STATUS_OK                    If the page was successfully read
 * \retval STATUS_ERR_NOT_INITIALIZED   If the RWW EEPROM emulator is not initialized
 * \retval STATUS_ERR_BAD_ADDRESS       If an address outside the valid emulated
 *                                      RWW EEPROM memory space was supplied
 * \retval STATUS_ERR_BAD_FORMAT        Page data checksum is not correct, maybe data
 *                                      is damaged
 */
enum status_code rww_eeprom_emulator_read_page(
		const uint8_t logical_page,
		uint8_t *const data)
{
	/* Ensure the emulated RWW EEPROM has been initialized first */
	if (_eeprom_instance.initialized == false) {
		return STATUS_ERR_NOT_INITIALIZED;
	}

	/* Make sure the read address is within the allowable address space */
	if (logical_page >= _eeprom_instance.logical_pages) {
		return STATUS_ERR_BAD_ADDRESS;
	}

	/* Check if the page to read is currently cached (and potentially out of
	 * sync/newer than the physical memory) */
	if ((_eeprom_instance.cache_active == true) &&
		 (_eeprom_instance.cache.header.logical_page == logical_page)) {
		/* Copy the potentially newer cached data into the user buffer */
		memcpy(data, _eeprom_instance.cache.data, RWW_EEPROM_PAGE_SIZE);
	} else {
		struct _rww_eeprom_page temp;

		/* Copy the data from non-volatile memory into the temporary buffer */
		if (_rww_eeprom_emulator_nvm_read_page(
				_eeprom_instance.page_map[logical_page], &temp)){
			/* Copy the data portion of the read page to the user's buffer */
			memcpy(data, temp.data, RWW_EEPROM_PAGE_SIZE);
		}else{
			/* Copy data even the checksum is not correct */
			memcpy(data, temp.data, RWW_EEPROM_PAGE_SIZE);
			return STATUS_ERR_BAD_FORMAT;
		}
	}

	return STATUS_OK;
}

/**
 * \brief Writes a buffer of data to the emulated RWW EEPROM memory space.
 *
 * Writes a buffer of data to a section of emulated RWW EEPROM memory space. The
 * source buffer may be of any size, and the destination may lie outside of an
 * emulated RWW EEPROM page boundary.
 *
 * \note Data stored in pages may be cached in volatile RAM memory; to commit
 *       any cached data to physical non-volatile memory, the
 *       \ref rww_eeprom_emulator_commit_page_buffer() function should be called.
 *
 * \param[in] offset  Starting byte offset to write to, in emulated RWW EEPROM
 *                    memory space
 * \param[in] data    Pointer to the data buffer containing source data to write
 * \param[in] length  Length of the data to write, in bytes
 *
 * \return Status code indicating the status of the operation.
 *
 * \retval STATUS_OK                    If the page was successfully read
 * \retval STATUS_ERR_NOT_INITIALIZED   If the RWW EEPROM emulator is not initialized
 * \retval STATUS_ERR_BAD_ADDRESS       If an address outside the valid emulated
 *                                      RWW EEPROM memory space was supplied
 */
enum status_code rww_eeprom_emulator_write_buffer(
		const uint16_t offset,
		const uint8_t *const data,
		const uint16_t length)
{
	enum status_code error_code = STATUS_OK;
	uint8_t buffer[RWW_EEPROM_PAGE_SIZE];
	uint8_t logical_page = offset / RWW_EEPROM_PAGE_SIZE;
	uint16_t c = offset;
	/* Keep track of whether the currently updated page has been written */
	bool page_dirty = false;
	/** Perform the initial page read if necessary*/
	if ((offset % RWW_EEPROM_PAGE_SIZE) || length < RWW_EEPROM_PAGE_SIZE) {
		error_code = rww_eeprom_emulator_read_page(logical_page, buffer);

		if (error_code != STATUS_OK) {
			return error_code;
		}
	}

	/* To avoid entering into the initial if in the loop the first time */
	if ((offset % RWW_EEPROM_PAGE_SIZE) == 0) {
		buffer[c % RWW_EEPROM_PAGE_SIZE] = data[c - offset];
		page_dirty = true;
		c=c+1;
	}

	/* Write the specified data to the emulated RWW EEPROM memory space */
	for (; c < (length + offset); c++) {
		/* Check if we have written up to a new RWW EEPROM page boundary */
		if ((c % RWW_EEPROM_PAGE_SIZE) == 0) {
			/* Write the current page to non-volatile memory from the temporary
			 * buffer */
			error_code = rww_eeprom_emulator_write_page(logical_page, buffer);
			page_dirty = false;

			if (error_code != STATUS_OK) {
				break;
			}

			/* Increment the page number we are looking at */
			logical_page++;

			/* Read the next page from non-volatile memory into the temporary
			 * buffer in case of a partial page write */
			error_code = rww_eeprom_emulator_read_page(logical_page, buffer);

			if (error_code != STATUS_OK) {
				return error_code;
			}
		}
		/* Copy the next byte of data from the user's buffer to the temporary
		 * buffer */
		buffer[c % RWW_EEPROM_PAGE_SIZE] = data[c - offset];
		page_dirty = true;
	}

	/* If the current page is dirty, write it */
	if (page_dirty) {
		error_code = rww_eeprom_emulator_write_page(logical_page, buffer);
	}

	return error_code;
}

/**
 * \brief Reads a buffer of data from the emulated RWW EEPROM memory space.
 *
 * Reads a buffer of data from a section of emulated RWW EEPROM memory space. The
 * destination buffer may be of any size, and the source may lie outside of an
 * emulated RWW EEPROM page boundary.
 *
 * \param[in]  offset  Starting byte offset to read from, in emulated RWW EEPROM
 *                     memory space
 * \param[out] data    Pointer to the data buffer containing source data to read
 * \param[in]  length  Length of the data to read, in bytes
 *
 * \return Status code indicating the status of the operation.
 *
 * \retval STATUS_OK                    If the page was successfully read
 * \retval STATUS_ERR_NOT_INITIALIZED   If the RWW EEPROM emulator is not initialized
 * \retval STATUS_ERR_BAD_ADDRESS       If an address outside the valid emulated
 *                                      RWW EEPROM memory space was supplied
 */
enum status_code rww_eeprom_emulator_read_buffer(
		const uint16_t offset,
		uint8_t *const data,
		const uint16_t length)
{
	enum status_code error_code;
	uint8_t buffer[RWW_EEPROM_PAGE_SIZE];
	uint8_t logical_page = offset / RWW_EEPROM_PAGE_SIZE;
	uint16_t c = offset;

	/** Perform the initial page read  */
	error_code = rww_eeprom_emulator_read_page(logical_page, buffer);
	if (error_code != STATUS_OK) {
		return error_code;
	}

	/* To avoid entering into the initial if in the loop the first time */
	if ((offset % RWW_EEPROM_PAGE_SIZE) == 0) {
		data[0] = buffer[0];
		c=c+1;
	}

	/* Read in the specified data from the emulated RWW EEPROM memory space */
	for (; c < (length + offset); c++) {
		/* Check if we have read up to a new RWW EEPROM page boundary */
		if ((c % RWW_EEPROM_PAGE_SIZE) == 0) {
			/* Increment the page number we are looking at */
			logical_page++;

			/* Read the next page from non-volatile memory into the temporary
			 * buffer */
			error_code = rww_eeprom_emulator_read_page(logical_page, buffer);

			if (error_code != STATUS_OK) {
				return error_code;
			}
		}

		/* Copy the next byte of data from the temporary buffer to the user's
		 * buffer */
		data[c - offset] = buffer[c % RWW_EEPROM_PAGE_SIZE];
	}

	return error_code;
}

/**
 * \brief Commits any cached data to physical non-volatile memory.
 *
 * Commits the internal SRAM caches to physical non-volatile memory, to ensure
 * that any outstanding cached data is preserved. This function should be called
 * prior to a system reset or shutdown to prevent data loss.
 *
 * \note This should be the first function executed in a BOD33 Early Warning
 *       callback to ensure that any outstanding cache data is fully written to
 *       prevent data loss.
 *
 *
 * \note This function should also be called before using the NVM controller
 *       directly in the user-application for any other purposes to prevent
 *       data loss.
 *
 * \return Status code indicating the status of the operation.
 */
enum status_code rww_eeprom_emulator_commit_page_buffer(void)
{
	enum status_code error_code = STATUS_OK;

	/* If cache is inactive, no need to commit anything to physical memory */
	if (_eeprom_instance.cache_active == false) {
		return STATUS_OK;
	}

	uint8_t cached_logical_page = _eeprom_instance.cache.header.logical_page;

	/* Perform the page write to commit the NVM page buffer to physical memory */
	_rww_eeprom_emulator_nvm_commit_cache(
			_eeprom_instance.page_map[cached_logical_page]);

	barrier(); // Enforce ordering to prevent incorrect cache state
	_eeprom_instance.cache_active = false;

	return error_code;
}
