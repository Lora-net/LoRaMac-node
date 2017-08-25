/**
 * Copyright (c) 2016 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#ifndef APP_USBD_HID_GENERIC_INTERNAL_H__
#define APP_USBD_HID_GENERIC_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "app_usbd_hid.h"
#include "nrf_queue.h"

/**
 * @defgroup app_usbd_hid_generic_internal USB HID generic internals
 * @ingroup app_usbd_hid_generic
 *
 * @brief @tagAPI52840 Module with types, definitions, and API used by the HID generic protocol.
 * @{
 */


/**
 * @brief Forward declaration of HID generic class type.
 *
 */
APP_USBD_CLASS_FORWARD(app_usbd_hid_generic);

/**
 * @brief HID generic part of class instance data.
 *
 */
typedef struct {
    app_usbd_hid_inst_t hid_inst;       //!< HID instance data.
    nrf_queue_t const * p_rep_in_queue; //!< Input report queue.
} app_usbd_hid_generic_inst_t;

/**
 * @brief HID generic context
 *
 */
typedef struct {
    app_usbd_hid_ctx_t hid_ctx;          //!< HID class context.
} app_usbd_hid_generic_ctx_t;


/**
 * @brief HID generic configuration macro.
 *
 * Used by @ref APP_USBD_HID_GENERIC_GLOBAL_DEF.
 *
 * @param iface         Interface number.
 * @param endpoints     Endpoint list.
 */
#define APP_USBD_HID_GENERIC_CONFIG(iface, endpoints) ((iface, BRACKET_EXTRACT(endpoints)))


/**
 * @brief Specific class constant data for HID generic class.
 */
#define APP_USBD_HID_GENERIC_INSTANCE_SPECIFIC_DEC app_usbd_hid_generic_inst_t inst;

/**
 * @brief Specific class data for HID generic class.
 */
#define APP_USBD_HID_GENERIC_DATA_SPECIFIC_DEC app_usbd_hid_generic_ctx_t ctx;


/**
 * @brief Configure internal part of HID generic instance.
 *
 * @param descriptors       Raw descriptors buffer.
 * @param report_buff_in    Input report buffers array.
 * @param report_buff_out   Output report buffer.
 * @param user_ev_handler   User event handler.
 * @param in_report_queue   IN report queue.
 * @param ...               Hid descriptors list.
 */
#define APP_USBD_HID_GENERIC_INST_CONFIG(descriptors,                         \
                                         report_buff_in,                      \
                                         report_buff_out,                     \
                                         user_ev_handler,                     \
                                         in_report_queue,                     \
                                         ...)                                 \
    .inst = {                                                                 \
         .hid_inst = APP_USBD_HID_INST_CONFIG(descriptors,                    \
                                              GET_VA_ARG_1(__VA_ARGS__),      \
                                              report_buff_in,                 \
                                              report_buff_out,                \
                                              user_ev_handler,                \
                                              &app_usbd_hid_generic_methods), \
        .p_rep_in_queue = in_report_queue,                                    \
    }

/**
 * @brief Public HID generic interface.
 */
extern const app_usbd_hid_methods_t app_usbd_hid_generic_methods;

/**
 * @brief Public HID generic class interface.
 */
extern const app_usbd_class_methods_t app_usbd_generic_class_methods;

/**
 * @brief Global definition of @ref app_usbd_hid_generic_t class.
 *
 * @ref APP_USBD_HID_GENERIC_GLOBAL_DEF
 */
#define APP_USBD_HID_GENERIC_GLOBAL_DEF_INTERNAL(instance_name,                     \
                                                 interface_number,                  \
                                                 user_ev_handler,                   \
                                                 endpoint_list,                     \
                                                 class_descriptors,                 \
                                                 report_descriptor,                 \
                                                 report_in_queue_size,              \
                                                 report_out_maxsize)                \
    static app_usbd_hid_report_buffer_t CONCAT_2(instance_name, _in);               \
    APP_USBD_HID_GENERIC_GLOBAL_OUT_REP_DEF(CONCAT_2(instance_name, _out),          \
                                            report_out_maxsize + 1);                \
    NRF_QUEUE_DEF(app_usbd_hid_report_buffer_t,                                     \
                  instance_name##_queue,                                            \
                  report_in_queue_size,                                             \
                  NRF_QUEUE_MODE_OVERFLOW);                                         \
    APP_USBD_CLASS_INST_GLOBAL_DEF(                                                 \
        instance_name,                                                              \
        app_usbd_hid_generic,                                                       \
        &app_usbd_generic_class_methods,                                            \
        APP_USBD_HID_GENERIC_CONFIG(interface_number, endpoint_list),               \
        (APP_USBD_HID_GENERIC_INST_CONFIG(class_descriptors,                        \
                                          &CONCAT_2(instance_name, _in),            \
                                          &CONCAT_2(instance_name, _out),           \
                                          user_ev_handler,                          \
                                          &instance_name##_queue,                   \
                                          report_descriptor))                       \
    )


/** @} */


#ifdef __cplusplus
}
#endif

#endif /* APP_USBD_HID_GENERIC_INTERNAL_H__ */
