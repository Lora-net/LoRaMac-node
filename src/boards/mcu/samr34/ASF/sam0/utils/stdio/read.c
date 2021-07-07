/**
 * \file
 *
 * \brief System-specific implementation of the \ref _read function used by
 *         the standard library.
 *
 * Copyright (c) 2009-2018 Microchip Technology Inc. and its subsidiaries.
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

#include "compiler.h"

/**
 * \defgroup group_common_utils_stdio Standard I/O (stdio)
 *
 * Common standard I/O driver that implements the stdio
 * read and write functions on AVR and SAM devices.
 *
 * \{
 */

extern volatile void *volatile stdio_base;
void (*ptr_get)(void volatile*, char*);


// IAR common implementation
#if ( defined(__ICCARM__) )

#include <yfuns.h>

#if (__VER__ < 8010000)
// Refer http://ftp.iar.se/WWWfiles/arm/webic/doc/EWARM_MigrationGuide.ENU.pdf
_STD_BEGIN
#endif

#pragma module_name = "?__read"

/*! \brief Reads a number of bytes, at most \a size, into the memory area
 *         pointed to by \a buffer.
 *
 * \param handle File handle to read from.
 * \param buffer Pointer to buffer to write read bytes to.
 * \param size Number of bytes to read.
 *
 * \return The number of bytes read, \c 0 at the end of the file, or
 *         \c _LLIO_ERROR on failure.
 */
size_t __read(int handle, unsigned char *buffer, size_t size)
{
	int nChars = 0;
	// This implementation only reads from stdin.
	// For all other file handles, it returns failure.
	if (handle != _LLIO_STDIN) {
		return _LLIO_ERROR;
	}
	for (; size > 0; --size) {
		ptr_get(stdio_base, (char*)buffer);
		buffer++;
		nChars++;
	}
	return nChars;
}

/*! \brief This routine is required by IAR DLIB library since EWAVR V6.10
 * the implementation is empty to be compatible with old IAR version.
 */
int __close(int handle)
{
	UNUSED(handle);
	return 0;
}

/*! \brief This routine is required by IAR DLIB library since EWAVR V6.10
 * the implementation is empty to be compatible with old IAR version.
 */
int remove(const char* val)
{
	UNUSED(val);
	return 0;
}

/*! \brief This routine is required by IAR DLIB library since EWAVR V6.10
 * the implementation is empty to be compatible with old IAR version.
 */
long __lseek(int handle, long val, int val2)
{
	UNUSED(handle);
	UNUSED(val2);
	return val;
}

#if (__VER__ < 8010000)
// Refer http://ftp.iar.se/WWWfiles/arm/webic/doc/EWARM_MigrationGuide.ENU.pdf
_STD_END
#endif

// GCC implementation
#elif (defined(__GNUC__))

int __attribute__((weak))
_read (int file, char * ptr, int len); // Remove GCC compiler warning

int __attribute__((weak))
_read (int file, char * ptr, int len)
{
	int nChars = 0;

	if (file != 0) {
		return -1;
	}

	for (; len > 0; --len) {
		ptr_get(stdio_base, ptr);
		ptr++;
		nChars++;
	}
	return nChars;
}

#endif

/**
 * \}
 */

