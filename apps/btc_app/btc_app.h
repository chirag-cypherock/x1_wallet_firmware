/**
 * @file    btc_app.h
 * @author  Cypherock X1 Team
 * @brief
 * @details

 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef BTC_APP_H
#define BTC_APP_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <btc/core.pb.h>
#include <stddef.h>
#include <stdint.h>

#include "events.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
typedef enum {
  MANAGER_TASK_SUCCESS = 0,
  MANAGER_TASK_FAILED,
  MANAGER_TASK_REJECTED,
  MANAGER_TASK_INVALID_ARGS,
  MANAGER_TASK_DECODING_FAILED,
  MANAGER_TASK_ENCODING_FAILED,
  MANAGER_TASK_P0_ABORT_OCCURED,
  MANAGER_TASK_P0_TIMEOUT_OCCURED,
  MANAGER_TASK_UNKNOWN_QUERY_REQUEST,
  MANAGER_TASK_INVALID_STATE,

  MANAGER_TASK_INVALID_DEFAULT = 0xFF,
} manager_error_code_t;
/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/
/**
 * @brief Entry point for the manager application of the X1 vault. It is invoked
 * by the X1 vault firmware, as soon as there is a USB request raised for the
 * manager app.
 *
 * @param usb_evt The USB event which triggered invocation of the manager app
 */
void manager_app_main(usb_event_t usb_evt);

#endif /* BTC_APP_H */
