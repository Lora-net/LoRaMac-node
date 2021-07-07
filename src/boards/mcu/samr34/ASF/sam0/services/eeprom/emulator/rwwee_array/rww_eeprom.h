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
#ifndef RWW_EEPROM_H_INCLUDED
#define RWW_EEPROM_H_INCLUDED

/**
 * \defgroup asfdoc_sam0_rww_eeprom_group SAM Read While Write EEPROM (RWW EEPROM) Emulator Service
 *
 * This driver for Atmel&reg; | SMART ARM&reg;-based microcontrollers provides an RWW emulated EEPROM
 * memory area, for the storage and retrieval of user-application
 * configuration data into and out of non-volatile memory. The main array
 * can therefore run code while EEPROM data is written.
 *
 * The following peripheral is used by this module:
 *  - NVM (Non-Volatile Memory Controller)
 *
 * The following devices can use this module:
 *  - Atmel | SMART SAM L21/L22
 *  - Atmel | SMART SAM D21
 *  - Atmel | SMART SAM C20/C21
 *  - Atmel | SMART SAM DA1
 *  - Atmel | SMART SAM HA1
 *
 * The outline of this documentation is as follows:
 *  - \ref asfdoc_sam0_rww_eeprom_prerequisites
 *  - \ref asfdoc_sam0_rww_eeprom_module_overview
 *  - \ref asfdoc_sam0_rww_eeprom_special_considerations
 *  - \ref asfdoc_sam0_rww_eeprom_extra_info
 *  - \ref asfdoc_sam0_rww_eeprom_examples
 *  - \ref asfdoc_sam0_rww_eeprom_api_overview
 *
 *
 * \section asfdoc_sam0_rww_eeprom_prerequisites Prerequisites
 * There are no prerequisites for this module.
 *
 *
 * \section asfdoc_sam0_rww_eeprom_module_overview Module Overview
 *
 * SAM devices embeds a separate read while write EEPROM emulation (RWWEE) array
 * that can be programmed while the main array is not blocked.
 * To use RWWEE memory, data must be written as a number of physical memory pages
 * (of several bytes each) rather than being individually byte addressable, and
 * entire rows of RWWEE must be erased before new data may be stored.
 * To help abstract these characteristics away from the user application an
 * emulation scheme is implemented to present a more user-friendly API for
 * data storage and retrieval.
 *
 * This module provides an RWW EEPROM emulation layer on top of the device's
 * internal NVM controller, to provide a standard interface for the reading and
 * writing of non-volatile configuration data. This data is placed into the
 * RWW EEPROM emulated section. Emulated EEPROM is exempt from the usual device
 * NVM region lock bits, so that it may be read from or written to at any point
 * in the user application.
 *
 * There are many different algorithms that may be employed for EEPROM emulation,
 * to tune the write and read latencies, RAM usage, wear
 * levelling, and other characteristics. As a result, multiple different emulator
 * schemes may be implemented, so that the most appropriate scheme for a
 * specific application's requirements may be used.
 *
 * \subsection asfdoc_sam0_rww_eeprom_module_overview_implementation Implementation Details
 * The following information is relevant for <b>RWW EEPROM Emulator scheme 1,
 * version 1.0.0</b>, as implemented by this module. Other revisions or
 * emulation schemes may vary in their implementation details and may have
 * different wear-leveling, latency, and other characteristics.
 *
 * \subsubsection asfdoc_sam0_rww_eeprom_module_overview_implementation_ec Emulator Characteristics
 * This emulator is designed for <b>best reliability, with a good balance of
 * available storage and write-cycle limits</b>. It is designed to ensure that
 * page data is updated by an atomic operation, so that in the event of a failed update the
 * previous data is not lost (when used correctly). With the exception of a
 * system reset with data cached to the internal write-cache buffer, at most
 * only the latest write to physical non-volatile memory will be lost in the
 * event of a failed write.
 *
 * This emulator scheme is tuned to give best write-cycle longevity when writes
 * are confined to the same logical RWW EEPROM page (where possible) and when writes
 * across multiple logical RWW EEPROM pages are made in a linear fashion through the
 * entire emulated RWW EEPROM space.
 *
 * \subsubsection asfdoc_sam0_rww_eeprom_module_overview_implementation_pf Physical Memory
 * RWW EEPROM emulator is divided into a number of physical rows, each
 * containing four identically sized pages. Pages may be read or written
 * to individually, however, pages must be erased before being reprogrammed and
 * the smallest granularity available for erasure is one single row.
 *
 * This discrepancy results in the need for an emulator scheme that is able to
 * handle the versioning and moving of page data to different physical rows as
 * needed, erasing old rows ready for re-use by future page write operations.
 *
 * Physically, the emulated RWW EEPROM segment is a dedicated space that are memory
 * mapped, as shown in
 * \ref asfdoc_sam0_rww_eeprom_module_mem_layout "the figure below".
 *
 * \anchor asfdoc_sam0_rww_eeprom_module_mem_layout
 * \dot
 * digraph memory_layout {
 *  size="5,5"
 *  node [shape=plaintext]
 *  memory [label=<
 *   <table border="0" cellborder="1" cellspacing="0" >
 *    <tr>
 *     <td align="right" border="0"> End of RWW EEPROM Memory </td>
 *     <td rowspan="3" align="center">RWW EEPROM </td>
 *    </tr>
 *    <tr>
 *     <td align="right" border="0"> </td>
 *    </tr>
 *    <tr>
 *     <td align="right" border="0"> Start of RWW EEPROM Memory</td>
 *    </tr>
 *    <tr>
 *     <td align="right" border="0"> </td>
 *     <td rowspan="3" align="center"> Reserved </td>
 *    </tr>
 *    <tr>
 *     <td align="right" border="0"> </td>
 *    </tr>
 *    <tr>
 *     <td align="right" border="0"></td>
 *    </tr>
 *    <tr>
 *     <td align="right" border="0"> End of NVM Memory </td>
 *     <td rowspan="3" align="center"> Reserved EEPROM Section </td>
 *    </tr>
 *    <tr>
 *     <td align="right" border="0"> </td>
 *    </tr>
 *    <tr>
 *     <td align="right" border="0"> Start of EEPROM Memory </td>
 *    </tr>
 *    <tr>
 *     <td align="right" border="0"> End of Application Memory </td>
 *     <td rowspan="3" align="center"> Application Section </td>
 *    </tr>
 *    <tr>
 *     <td height="200" align="right" border="0"> </td>
 *    </tr>
 *    <tr>
 *     <td align="right" border="0"> Start of Application Memory </td>
 *    </tr>
 *    <tr>
 *     <td align="right" border="0"> End of Bootloader Memory </td>
 *     <td rowspan="3" align="center"> BOOT Section </td>
 *    </tr>
 *    <tr>
 *     <td align="right" border="0"> </td>
 *    </tr>
 *    <tr>
 *     <td align="right" border="0"> Start of NVM Memory</td>
 *    </tr>
 *   </table>
 *  >]
 * }
 * \enddot
 *
 * \subsubsection asfdoc_sam0_rww_eeprom_module_overview_implementation_mp Master Row
 * One physical row at the end of the emulated RWW EEPROM memory space is
 * reserved for use by the emulator to store configuration data. The master row
 * is not user-accessible, and is reserved solely for internal use by the
 * emulator.
 *
 * \subsubsection asfdoc_sam0_rww_eeprom_module_overview_implementation_sr Spare Row
 * As data needs to be preserved between row erasures, a single row is kept
 * unused to act as destination for copied data when a write request is made to
 * an already full row. When the write request is made, any logical pages of
 * data in the full row that need to be preserved are written to the spare row
 * along with the new (updated) logical page data, before the old row is erased
 * and marked as the new spare.
 *
 * \subsubsection asfdoc_sam0_rww_eeprom_module_overview_implementation_rc Row Contents
 * Each physical row initially stores the contents of one or two logical RWW EEPROM
 * memory pages (it depends on application configuration file). This quarters or
 * halves the available storage space for the emulated RWW EEPROM
 * but reduces the overall number of row erases that are required, by reserving
 * two or three pages within each row for updated versions of the logical page contents.
 * See \ref asfdoc_sam0_rww_eeprom_page_layout "here" for a visual layout of the RWW
 * EEPROM Emulator physical memory.
 *
 * As logical pages within a physical row are updated, the new data is filled
 * into the remaining unused pages in the row. Once the entire row is full, a
 * new write request will copy the logical page not being written to in the
 * current row to the spare row with the new (updated) logical page data, before
 * the old row is erased.
 *
 * When it is configured, each physical row stores the contents of one logical RWW EEPROM
 * memory page. This system will allow for the same logical page to be updated up
 * to four times into the physical memory before a row erasure procedure is needed.
 * In the case of multiple versions of the same logical RWW EEPROM page being stored in
 * the same physical row, the right-most (highest physical memory page address)
 * version is considered to be the most current.
 *
 * \subsubsection asfdoc_sam0_rww_eeprom_module_overview_implementation_wc Write Cache
 * As a typical EEPROM use case is to write to multiple sections of the same
 * EEPROM page sequentially, the emulator is optimized with a single logical
 * RWW EEPROM page write cache to buffer writes before they are written to the
 * physical backing memory store. The cache is automatically committed when a
 * new write request to a different logical RWW EEPROM memory page is requested, or
 * when the user manually commits the write cache.
 *
 * Without the write cache, each write request to an EEPROM memory page would
 * require a full page write, reducing the system performance and significantly
 * reducing the lifespan of the non-volatile memory.
 *
 * \subsection asfdoc_sam0_rww_eeprom_special_considerations_memlayout Memory Layout
 * A single logical RWW EEPROM page is physically stored as the page content and a
 * header inside a single physical page, as shown in
 * \ref asfdoc_sam0_rww_eeprom_page_layout "the following figure".
 *
 * \anchor asfdoc_sam0_rww_eeprom_page_layout
 * \image html page_layout.svg "Internal Layout of an Emulated RWW EEPROM Page"
 *
 * \note In the following memory layout example, each physical row stores the
 * contents of one logical RWW EEPROM page. Refer to
 * <a href="http://ww1.microchip.com/downloads/en/AppNotes/Atmel-42125-SAM-EEPROM-Emulator-Service-EEPROM_ApplicationNote_AT03265.pdf"> 
 * "AT03265: SAM EEPROM Emulator Service (EEPROM)"</a> 
 * for the example of two logical EEPROM pages in one row.
 *
 * Within the RWW EEPROM memory reservation section at the top of the NVM memory
 * space, this emulator will produce the layout as shown in
 * \ref asfdoc_sam0_rww_eeprom_init_layout "the figure below" when initialized for
 * the first time.
 *
 * \anchor asfdoc_sam0_rww_eeprom_init_layout
 * \image html init_layout.svg "Initial Physical Layout of the Emulated RWW EEPROM Memory"
 *
 * When an RWW EEPROM page needs to be committed to physical memory, the next free
 * page in the same row will be chosen. This makes recovery simple, as the
 * right-most version of a logical page in a row is considered the most current.
 * With four pages to a physical NVM row, this allows for up to four updates to
 * the same logical page to be made before an erase is needed.
 * \ref asfdoc_sam0_rww_eeprom_page_write1 "The figure below" shows the result of
 * the user writing an updated version of logical EEPROM page <tt>N-1</tt> to
 * the physical memory.
 *
 * \anchor asfdoc_sam0_rww_eeprom_page_write1
 * \image html nm1_page_write.svg "First Write to Logical RWW EEPROM Page N-1"
 *
 * A second write of the same logical RWW EEPROM page results in the layout shown
 * in \ref asfdoc_sam0_rww_eeprom_page_write2 "the figure below".
 *
 * \anchor asfdoc_sam0_rww_eeprom_page_write2
 * \image html nm1_page_write2.svg "Second Write to Logical RWW EEPROM Page N-1"
 *
 * A third write of the same logical RWW EEPROM page results in the layout shown
 * in \ref asfdoc_sam0_rww_eeprom_page_write3 "the figure below".
 *
 * \anchor asfdoc_sam0_rww_eeprom_page_write3
 * \image html nm1_page_write3.svg "Third Write to Logical RWW EEPROM Page N-1"
 *
 * A fourth write of the same logical page requires that the RWW EEPROM emulator
 * erase the row, as it has become full. Prior to this, the content of the
 * unmodified page in the same row as the page being updated will be copied into
 * the spare row, along with the new version of the page being updated. The old
 * (full) row is then erased, resulting in the layout shown in
 * \ref asfdoc_sam0_rww_eeprom_page_write4 "the figure below".
 *
 * \anchor asfdoc_sam0_rww_eeprom_page_write4
 * \image html nm1_page_write4.svg "Third Write to Logical RWW EEPROM Page N-1"
 *
 *
 * \section asfdoc_sam0_rww_eeprom_special_considerations Special Considerations
 *
 * \subsection asfdoc_sam0_rww_eeprom_special_considerations_nvm_config NVM Controller Configuration
 * The RWW EEPROM Emulator service will initialize the NVM controller as part of its
 * own initialization routine; the NVM controller will be placed in Manual Write
 * mode, so that explicit write commands must be sent to the controller to
 * commit a buffered page to physical memory. The manual write command must thus
 * be issued to the NVM controller whenever the user application wishes to write
 * to a NVM page for its own purposes.
 *
 * \subsection asfdoc_sam0_rww_eeprom_special_considerations_pagesize Logical RWW EEPROM Page Size
 * As a small amount of information needs to be stored in a header before the
 * content of a logical EEPROM page in memory (for use by the emulation
 * service), the available data in each RWW EEPROM page is less than the total size
 * of a single NVM memory page by several bytes.
 *
 * \subsection asfdoc_sam0_rww_eeprom_special_considerations_committing Committing of the Write Cache
 * A single-page write cache is used internally to buffer data written to pages
 * in order to reduce the number of physical writes required to store the user
 * data, and to preserve the physical memory lifespan. As a result, it is
 * important that the write cache is committed to physical memory <b>as soon as
 * possible after a BOD low power condition</b>, to ensure that enough power is
 * available to guarantee a completed write so that no data is lost.
 *
 * The write cache must also be manually committed to physical memory if the
 * user application is to perform any NVM operations using the NVM controller
 * directly.
 *
 * \subsection asfdoc_sam0_rww_eeprom_special_considerations_checksum RWW EEPROM Page Checksum
 * For each page, a checksum function is used to verify the integrity of
 * the page data. When reading the page data, using 
 * \ref rww_eeprom_emulator_read_page(). When its checksum is not correct, an error can be
 * detected. This function can be enabled or disabled through the configuration file.
 *
 * \section asfdoc_sam0_rww_eeprom_extra_info Extra Information
 *
 * For extra information, see \ref asfdoc_sam0_rww_eeprom_extra. This includes:
 *  - \ref asfdoc_sam0_rww_eeprom_extra_acronyms
 *  - \ref asfdoc_sam0_rww_eeprom_extra_dependencies
 *  - \ref asfdoc_sam0_rww_eeprom_extra_errata
 *  - \ref asfdoc_sam0_rww_eeprom_extra_history
 *
 *
 * \section asfdoc_sam0_rww_eeprom_examples Examples
 *
 * For a list of examples related to this driver, see
 * \ref asfdoc_sam0_rww_eeprom_exqsg.
 *
 *
 * \section asfdoc_sam0_rww_eeprom_api_overview API Overview
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <compiler.h>

#if !defined(__DOXYGEN__)
#  define RWW_EEPROM_MAX_PAGES            NVMCTRL_RWWEE_PAGES
#  define RWW_EEPROM_MASTER_PAGE_NUMBER   (_eeprom_instance.physical_pages - 1)
#  define RWW_EEPROM_INVALID_PAGE_NUMBER  0xFF
#  define RWW_EEPROM_INVALID_ROW_NUMBER   (RWW_EEPROM_INVALID_PAGE_NUMBER / NVMCTRL_ROW_PAGES)
#  define RWW_EEPROM_HEADER_SIZE          4
#endif


/**
 * \brief RWW EEPROM Logical Page in Each Row.
 *
 * Enum for the possible logical pages that are stored in each physical row.
 *
 */
enum rwwee_logical_page_num_in_row {
	/** One logical page stored in a physical row */
	RWWEE_LOGICAL_PAGE_NUM_1 = 1,
	/** Two logical pages stored in a physical row */
	RWWEE_LOGICAL_PAGE_NUM_2 = 2,
};


/** \name RWW EEPROM Emulator Information
 * @{
 */

/** Emulator scheme ID, identifying the scheme used to emulated EEPROM storage. */
#define RWW_EEPROM_EMULATOR_ID          1

/** Emulator major version number, identifying the emulator major version. */
#define RWW_EEPROM_MAJOR_VERSION        1

/** Emulator minor version number, identifying the emulator minor version. */
#define RWW_EEPROM_MINOR_VERSION        0

/** Emulator revision version number, identifying the emulator revision. */
#define RWW_EEPROM_REVISION             0

/** Size of the user data portion of each logical EEPROM page, in bytes. */
#define RWW_EEPROM_PAGE_SIZE            (NVMCTRL_PAGE_SIZE - RWW_EEPROM_HEADER_SIZE)

/**
 * \brief RWW EEPROM memory parameter structure.
 *
 * Structure containing the memory layout parameters of the EEPROM emulator module.
 */
struct rww_eeprom_emulator_parameters {
	/** Number of bytes per emulated EEPROM page */
	uint8_t  page_size;
	/** Number of emulated pages of EEPROM */
	uint16_t eeprom_number_of_pages;
};

/** @} */

/** \name Configuration and Initialization
 * @{
 */

enum status_code rww_eeprom_emulator_init(void);

void rww_eeprom_emulator_erase_memory(void);

enum status_code rww_eeprom_emulator_get_parameters(
		struct rww_eeprom_emulator_parameters *const parameters);

/** @} */


/** \name Logical RWW EEPROM Page Reading/Writing
 * @{
 */

enum status_code rww_eeprom_emulator_commit_page_buffer(void);

enum status_code rww_eeprom_emulator_write_page(
		const uint8_t logical_page,
		const uint8_t *const data);

enum status_code rww_eeprom_emulator_read_page(
		const uint8_t logical_page,
		uint8_t *const data);

/** @} */

/** \name Buffer RWW EEPROM Reading/Writing
 * @{
 */

enum status_code rww_eeprom_emulator_write_buffer(
		const uint16_t offset,
		const uint8_t *const data,
		const uint16_t length);

enum status_code rww_eeprom_emulator_read_buffer(
		const uint16_t offset,
		uint8_t *const data,
		const uint16_t length);

/** @} */

#ifdef __cplusplus
}
#endif

/** @} */

/**
 * \page asfdoc_sam0_rww_eeprom_extra Extra Information
 *
 * \section asfdoc_sam0_rww_eeprom_extra_acronyms Acronyms
 *
 * <table>
 *	<tr>
 *		<th>Acronym</th>
 *		<th>Description</th>
 *	</tr>
 *	<tr>
 *		<td>EEPROM</td>
 *		<td>Electronically Erasable Read-Only Memory</td>
 *	</tr>
 *	<tr>
 *		<td>RWWEE</td>
 *		<td>Read While Write EEPROM</td>
 *	</tr>
  *	<tr>
 *		<td>RWW</td>
 *		<td>Read While Write</td>
 *	</tr>
 *	<tr>
 *		<td>NVM</td>
 *		<td>Non-Volatile Memory</td>
 *	</tr>
 * </table>
 *
 *
 * \section asfdoc_sam0_rww_eeprom_extra_dependencies Dependencies
 * This driver has the following dependencies:
 *
 *  - \ref asfdoc_sam0_nvm_group "Non-Volatile Memory Controller Driver"
 *
 *
 * \section asfdoc_sam0_rww_eeprom_extra_errata Errata
 * There are no errata related to this driver.
 *
 *
 * \section asfdoc_sam0_rww_eeprom_extra_history Module History
 * An overview of the module history is presented in the table below, with
 * details on the enhancements and fixes made to the module since its first
 * release. The current version of this corresponds to the newest version in
 * the table.
 *
 * <table>
 *	<tr>
 *		<th>Changelog</th>
 *	</tr>
 *	<tr>
 *		<td>Initial Release</td>
 *	</tr>
 * </table>
 */

/**
 * \page asfdoc_sam0_rww_eeprom_exqsg Examples for Emulated RWW EEPROM Service
 *
 * This is a list of the available Quick Start guides (QSGs) and example
 * applications for \ref asfdoc_sam0_rww_eeprom_group. QSGs are simple examples with
 * step-by-step instructions to configure and use this driver in a selection of
 * use cases. Note that a QSG can be compiled as a standalone application or be
 * added to the user application.
 *
 *  - \subpage asfdoc_sam0_rww_eeprom_basic_use_case
 *
 * \page asfdoc_sam0_rww_eeprom_document_revision_history Document Revision History
 *
 * <table>
 *	<tr>
 *		<th>Doc. Rev.</th>
 *		<th>Date</th>
 *		<th>Comments</th>
 *	</tr>
 *	<tr>
 *		<td>42447B</td>
 *		<td>02/2016</td>
 *		<td>Added support for SAM L22, SAM DA1, and SAM C20/C21</td>
 *	</tr>
 *	<tr>
 *		<td>42447A</td>
 *		<td>06/2015</td>
 *		<td>Initial release</td>
 *	</tr>
 * </table>
 */

#endif /* EEPROM_H_INCLUDED */
