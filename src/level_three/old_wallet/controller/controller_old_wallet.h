/**
 * @file    controller_old_wallet.h
 * @author  Cypherock X1 Team
 * @brief   Header for old wallet controller.
 *          This file contains the old wallet controller.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */

#ifndef CONTROLLER_OLD_WALLET_H
#define CONTROLLER_OLD_WALLET_H

#pragma once

#include "controller_main.h"

/**
 * @brief Controller to Constructs wallet seed  flow
 * @details This controller handles the post request processing of the view seed
 * flow. Based on the wallet configuration, it will skip some cases such as
 * PIN/passphrase entering and directly regenerate the wallet seed to view
 * mnemonics ti the user.
 *
 * @see VIEW_SEED_TASKS,
 * tap_threshold_cards_for_reconstruction_flow_controller(), view_seed_task()
 * @since v1.0.0
 *
 * @note
 */
void view_seed_controller();

/**
 * @brief Controller to handle post event processing of the delete wallet flow.
 * @details The function deletes the wallet from the device removing both the
 * basic information along with the device's share.
 *
 * @see DELETE_WALLET_TASKS, delete_from_cards_controller(),
 * delete_wallet_share_from_sec_flash(), delete_wallet_from_flash(),
 * delete_wallet_task()
 * @since v1.0.0
 *
 * @note The wallet entry is erased only after the wallet has been deleted from
 * all the cards.
 */
void delete_wallet_controller();

/**
 * @brief Controller to branch through the application state for processing the
 * exact active flow.
 * @details This controller is only responsible for branching through the
 * application state and processing the exact active flow. No actual processing
 * is done here.
 *
 * @see LEVEL_THREE_OLD_WALLET_TASKS, view_seed_controller(),
 * delete_wallet_controller(), export_wallet_controller(),
 * add_coin_controller(), send_transaction_controller(),
 * send_transaction_controller_eth(), receive_transaction_controller_eth(),
 * receive_transaction_controller(), wallet_locked_controller(),
 * verify_wallet_controller(), sync_cards_controller(),
 * level_three_old_wallet_tasks()
 * @since v1.0.0
 */
void level_three_old_wallet_controller();

/**
 * @brief Back button controller for level three old wallet flow.
 * @details This controller is only responsible for branching through the
 * application state and processing the exact active flow. No actual processing
 * is done here.
 *
 * @see LEVEL_THREE_OLD_WALLET_TASKS, sync_cards_controller_b(),
 * verify_wallet_controller_b(), wallet_locked_controller_b(),
 * receive_transaction_controller_b_eth(), receive_transaction_controller_b(),
 * send_transaction_controller_b_eth(), send_transaction_controller_b(),
 * level_three_old_wallet_tasks()
 * @since v1.0.0
 */
void level_three_old_wallet_controller_b();

#endif