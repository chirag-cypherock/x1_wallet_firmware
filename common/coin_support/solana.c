/**
 * @file    solana.c
 * @author  Cypherock X1 Team
 * @brief   Solana coin support.
 *          Stores solana coin related class.
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 *target=_blank>https://mitcc.org/</a>
 *
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022 by HODL TECH PTE LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * "Commons Clause" License Condition v1.0
 *
 * The Software is provided to you by the Licensor under the License,
 * as defined below, subject to the following condition.
 *
 * Without limiting other conditions in the License, the grant of
 * rights under the License will not include, and the License does not
 * grant to you, the right to Sell the Software.
 *
 * For purposes of the foregoing, "Sell" means practicing any or all
 * of the rights granted to you under the License to provide to third
 * parties, for a fee or other consideration (including without
 * limitation fees for hosting or consulting/ support services related
 * to the Software), a product or service whose value derives, entirely
 * or substantially, from the functionality of the Software. Any license
 * notice or attribution required by the License must also include
 * this Commons Clause License Condition notice.
 *
 * Software: All X1Wallet associated files.
 * License: MIT
 * Licensor: HODL TECH PTE LTD
 *
 ******************************************************************************
 */

#include "solana.h"

uint16_t get_compact_array_size(const uint8_t *data, uint16_t *size) {
  uint16_t offset = 0;
  *size           = 0;
  while (offset < 3) {
    *size |= (*(data + offset) & 0x7F) << offset * 7;
    if ((*(data + offset) & 0x80) == 0)
      break;
    offset++;
  }
  return offset + 1;
}

int solana_byte_array_to_unsigned_txn(uint8_t *byte_array, uint16_t byte_array_size, solana_unsigned_txn *utxn) {
  uint16_t offset = 0;

  // Message header
  utxn->required_signatures_count                      = *(byte_array + offset++);
  utxn->read_only_accounts_require_signature_count     = *(byte_array + offset++);
  utxn->read_only_accounts_not_require_signature_count = *(byte_array + offset++);

  // Account addresses
  offset += get_compact_array_size(byte_array + offset, &(utxn->account_addresses_count));
  utxn->account_addresses = byte_array + offset;
  offset += utxn->account_addresses_count * SOLANA_ACCOUNT_ADDRESS_LENGTH;

  // Blockhash
  utxn->blockhash = byte_array + offset;
  offset += SOLANA_BLOCKHASH_LENGTH;

  // Instructions
  offset += get_compact_array_size(byte_array + offset, &(utxn->instructions_count));
  if (utxn->instructions_count != 1)
    return -1;

  utxn->instruction.program_id_index = *(byte_array + offset++);

  uint8_t system_program_id[SOLANA_ACCOUNT_ADDRESS_LENGTH] = {0};
  if (memcmp(utxn->account_addresses + utxn->instruction.program_id_index * SOLANA_ACCOUNT_ADDRESS_LENGTH,
             system_program_id, SOLANA_ACCOUNT_ADDRESS_LENGTH) != 0)
    return -1;

  offset += get_compact_array_size(byte_array + offset, &(utxn->instruction.account_addresses_index_count));
  utxn->instruction.account_addresses_index = byte_array + offset;
  offset += utxn->instruction.account_addresses_index_count;
  offset += get_compact_array_size(byte_array + offset, &(utxn->instruction.opaque_data_length));
  utxn->instruction.opaque_data = byte_array + offset;
  offset += utxn->instruction.opaque_data_length;

  utxn->instruction.program.transfer.funding_account   = utxn->account_addresses + (0 * SOLANA_ACCOUNT_ADDRESS_LENGTH);
  utxn->instruction.program.transfer.recipient_account = utxn->account_addresses + (1 * SOLANA_ACCOUNT_ADDRESS_LENGTH);
  utxn->instruction.program.transfer.lamports          = U64_READ_LE_ARRAY(utxn->instruction.opaque_data + 4);

  return (offset > 0 ? 0 : -1);
}

void solana_sig_unsigned_byte_array(const uint8_t *unsigned_txn_byte_array,
                                    uint64_t unsigned_txn_len,
                                    const txn_metadata *transaction_metadata,
                                    const char *mnemonics,
                                    const char *passphrase,
                                    uint8_t *sig) {
  uint32_t path[]  = {BYTE_ARRAY_TO_UINT32(transaction_metadata->purpose_index),
                      BYTE_ARRAY_TO_UINT32(transaction_metadata->coin_index),
                      BYTE_ARRAY_TO_UINT32(transaction_metadata->account_index),
                      BYTE_ARRAY_TO_UINT32(transaction_metadata->input[0].chain_index),
                      BYTE_ARRAY_TO_UINT32(transaction_metadata->input[0].address_index)};
  uint8_t seed[64] = {0};
  HDNode hdnode;
  mnemonic_to_seed(mnemonics, passphrase, seed, NULL);
  derive_hdnode_from_path(path, 5, ED25519_NAME, seed, &hdnode);

  ed25519_sign(unsigned_txn_byte_array, unsigned_txn_len, hdnode.private_key, hdnode.public_key + 1, sig);
  memzero(path, sizeof(path));
  memzero(seed, sizeof(seed));
  memzero(&hdnode, sizeof(hdnode));
}