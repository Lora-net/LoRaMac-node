#ifdef __cplusplus
extern "C"
{
#endif

#ifndef LoraMac_h
#define LoraMac_h

    /* ==================================================================== */
    /* ========================== include files =========================== */
    /* ==================================================================== */

    /* Inclusion of system and local header files goes here */
#include "LoRaMacTypes.h"

    /* ==================================================================== */
    /* ============================ constants ============================= */
    /* ==================================================================== */

    /* #define and enum statements go here */

    /*!
 * LoRaMAC region enumeration
 */
    typedef enum eLoRaMacRegion_t
    {
        /*!
     * AS band on 923MHz
     */
        LORAMAC_REGION_AS923,
        /*!
     * Australian band on 915MHz
     */
        LORAMAC_REGION_AU915,
        /*!
     * Chinese band on 470MHz
     */
        LORAMAC_REGION_CN470,
        /*!
     * Chinese band on 779MHz
     */
        LORAMAC_REGION_CN779,
        /*!
     * European band on 433MHz
     */
        LORAMAC_REGION_EU433,
        /*!
     * European band on 868MHz
     */
        LORAMAC_REGION_EU868,
        /*!
     * South korean band on 920MHz
     */
        LORAMAC_REGION_KR920,
        /*!
     * India band on 865MHz
     */
        LORAMAC_REGION_IN865,
        /*!
     * North american band on 915MHz
     */
        LORAMAC_REGION_US915,
        /*!
     * Russia band on 864MHz
     */
        LORAMAC_REGION_RU864,
    } LoRaMacRegion_t;

    /* ==================================================================== */
    /* ========================== public data ============================= */
    /* ==================================================================== */

    /* Definition of public (external) data types go here */

    /* ==================================================================== */
    /* ======================= public functions =========================== */
    /* ==================================================================== */

    /* Function prototypes for public (external) functions go here */

#endif
#ifdef __cplusplus
}
#endif
