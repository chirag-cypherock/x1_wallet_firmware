/**
 * @file    coin_utils.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
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
#include "coin_utils.h"

#include "arbitrum.h"
#include "avalanche.h"
#include "bsc.h"
#include "etc.h"
#include "eth.h"
#include "fantom.h"
#include "harmony.h"
#include "near.h"
#include "optimism.h"
#include "polygon.h"
#include "segwit_addr.h"
#include "solana.h"

void s_memcpy(uint8_t *dst,
              const uint8_t *src,
              uint32_t size,
              uint64_t len,
              int64_t *offset) {
  if (*offset == -1 || (*offset + len > size)) {
    *offset = -1;
    return;
  }
  memcpy(dst, &src[*offset], len);
  *offset += len;
}

int64_t byte_array_to_txn_metadata(const uint8_t *metadata_byte_array,
                                   const uint32_t size,
                                   txn_metadata *txn_metadata_ptr) {
  if (metadata_byte_array == NULL || txn_metadata_ptr == NULL)
    return -1;
  memzero(txn_metadata_ptr, sizeof(txn_metadata));

  int64_t offset = 0, len = 0;

  s_memcpy(txn_metadata_ptr->purpose_index,
           metadata_byte_array,
           size,
           sizeof(txn_metadata_ptr->purpose_index),
           &offset);
  s_memcpy(txn_metadata_ptr->coin_index,
           metadata_byte_array,
           size,
           sizeof(txn_metadata_ptr->coin_index),
           &offset);
  s_memcpy(txn_metadata_ptr->account_index,
           metadata_byte_array,
           size,
           sizeof(txn_metadata_ptr->account_index),
           &offset);
  s_memcpy(txn_metadata_ptr->input_count,
           metadata_byte_array,
           size,
           sizeof(txn_metadata_ptr->input_count),
           &offset);

  len = (*txn_metadata_ptr->input_count) * sizeof(address_type);
  txn_metadata_ptr->input = (address_type *)cy_malloc(len);

  uint8_t metadataInputIndex = 0;
  for (; metadataInputIndex < *txn_metadata_ptr->input_count;
       metadataInputIndex++) {
    address_type *input = &txn_metadata_ptr->input[metadataInputIndex];
    s_memcpy(input->change_index,
             metadata_byte_array,
             size,
             sizeof(input->change_index),
             &offset);
    s_memcpy(input->address_index,
             metadata_byte_array,
             size,
             sizeof(input->address_index),
             &offset);
  }

  s_memcpy(txn_metadata_ptr->output_count,
           metadata_byte_array,
           size,
           sizeof(txn_metadata_ptr->output_count),
           &offset);

  len = sizeof(address_type);    // decode 1 output address
  txn_metadata_ptr->output = (address_type *)cy_malloc(len);

  uint8_t metadataOutputIndex = 0;
  for (; metadataOutputIndex < 1; metadataOutputIndex++) {
    address_type *output = &txn_metadata_ptr->output[metadataOutputIndex];
    s_memcpy(output->change_index,
             metadata_byte_array,
             size,
             sizeof(output->change_index),
             &offset);
    s_memcpy(output->address_index,
             metadata_byte_array,
             size,
             sizeof(output->address_index),
             &offset);
  }

  s_memcpy(txn_metadata_ptr->change_count,
           metadata_byte_array,
           size,
           sizeof(txn_metadata_ptr->change_count),
           &offset);

  len = sizeof(address_type);    // decode 1 change address
  txn_metadata_ptr->change = (address_type *)cy_malloc(len);

  uint8_t metadataChangeIndex = 0;
  for (; metadataChangeIndex < *txn_metadata_ptr->change_count;
       metadataChangeIndex++) {
    address_type *change = &txn_metadata_ptr->change[metadataChangeIndex];
    s_memcpy(change->change_index,
             metadata_byte_array,
             size,
             sizeof(change->change_index),
             &offset);
    s_memcpy(change->address_index,
             metadata_byte_array,
             size,
             sizeof(change->address_index),
             &offset);
  }

  s_memcpy(txn_metadata_ptr->transaction_fees,
           metadata_byte_array,
           size,
           sizeof(txn_metadata_ptr->transaction_fees),
           &offset);

  // Should be set in decoding flow
  txn_metadata_ptr->eth_val_decimal[0] = 0xFF;
  offset++;

  size_t token_name_len =
      strnlen((const char *)(metadata_byte_array + offset), size - offset) + 1;

  if (metadata_byte_array[offset + token_name_len - 1] != 0)
    return -1;

  txn_metadata_ptr->token_name = (char *)cy_malloc(token_name_len);

  s_memcpy((uint8_t *)txn_metadata_ptr->token_name,
           metadata_byte_array,
           size,
           token_name_len,
           &offset);

  txn_metadata_ptr->is_token_transfer = strncmp(txn_metadata_ptr->token_name,
                                                ETHEREUM_TOKEN_SYMBOL,
                                                token_name_len) != 0;

  if (offset + sizeof(txn_metadata_ptr->network_chain_id) > size)
    return -1;
  txn_metadata_ptr->network_chain_id =
      U64_READ_BE_ARRAY(metadata_byte_array + offset);
  offset += sizeof(txn_metadata_ptr->network_chain_id);

  txn_metadata_ptr->is_token_transfer =
      strncmp(
          txn_metadata_ptr->token_name,
          get_coin_symbol(BYTE_ARRAY_TO_UINT32(txn_metadata_ptr->coin_index),
                          txn_metadata_ptr->network_chain_id),
          token_name_len) != 0;

  if (offset + 1 <= size)
    txn_metadata_ptr->is_harmony_address = metadata_byte_array[offset++];

  if (offset + sizeof(txn_metadata_ptr->address_tag) > size)
    return -1;
  txn_metadata_ptr->address_tag =
      U16_READ_BE_ARRAY(metadata_byte_array + offset);
  offset += sizeof(txn_metadata_ptr->address_tag);

  return offset;
}

int64_t byte_array_to_recv_txn_data(Receive_Transaction_Data *txn_data_ptr,
                                    const uint8_t *data_byte_array,
                                    const uint32_t size) {
  if (txn_data_ptr == NULL || data_byte_array == NULL)
    return -1;
  memzero(txn_data_ptr, sizeof(Receive_Transaction_Data));

  int64_t offset = 0;

  s_memcpy(txn_data_ptr->wallet_id,
           data_byte_array,
           size,
           sizeof(txn_data_ptr->wallet_id),
           &offset);
  s_memcpy(txn_data_ptr->purpose,
           data_byte_array,
           size,
           sizeof(txn_data_ptr->purpose),
           &offset);
  s_memcpy(txn_data_ptr->coin_index,
           data_byte_array,
           size,
           sizeof(txn_data_ptr->coin_index),
           &offset);
  s_memcpy(txn_data_ptr->account_index,
           data_byte_array,
           size,
           sizeof(txn_data_ptr->account_index),
           &offset);
  s_memcpy(txn_data_ptr->change_index,
           data_byte_array,
           size,
           sizeof(txn_data_ptr->change_index),
           &offset);
  s_memcpy(txn_data_ptr->address_index,
           data_byte_array,
           size,
           sizeof(txn_data_ptr->address_index),
           &offset);

  size_t token_name_len =
      strnlen((const char *)(data_byte_array + offset), size - offset) + 1;

  if (data_byte_array[offset + token_name_len - 1] != 0)
    return -1;

  txn_data_ptr->token_name = (char *)cy_malloc(token_name_len);

  s_memcpy((uint8_t *)txn_data_ptr->token_name,
           data_byte_array,
           size,
           token_name_len,
           &offset);

  if (offset + sizeof(txn_data_ptr->network_chain_id) > size)
    return -1;
  txn_data_ptr->network_chain_id = U64_READ_BE_ARRAY(data_byte_array + offset);
  offset += sizeof(txn_data_ptr->network_chain_id);

  if (offset + sizeof(txn_data_ptr->address_tag) > size)
    return -1;
  txn_data_ptr->address_tag = U16_READ_BE_ARRAY(data_byte_array + offset);
  offset += sizeof(txn_data_ptr->address_tag);

  return offset;
}

int64_t byte_array_to_add_coin_data(Add_Coin_Data *data_ptr,
                                    const uint8_t *byte_array,
                                    size_t size) {
  if (data_ptr == NULL || byte_array == NULL)
    return -1;
  memzero(data_ptr, sizeof(Add_Coin_Data));

  int64_t offset = 0;

  data_ptr->derivation_depth = byte_array[offset++];
  if (data_ptr->derivation_depth < 2 ||
      data_ptr->derivation_depth >
          (sizeof(data_ptr->derivation_path) / sizeof(uint32_t))) {
    return -1;
  }

  for (int levelIndex = 0; levelIndex < data_ptr->derivation_depth;
       levelIndex++) {
    if (offset + sizeof(uint32_t) > size)
      return -1;
    data_ptr->derivation_path[levelIndex] =
        U32_READ_BE_ARRAY(byte_array + offset);
    offset += sizeof(uint32_t);
  }

  if (offset + sizeof(uint64_t) > size)
    return -1;
  data_ptr->network_chain_id = U64_READ_BE_ARRAY(&byte_array[offset]);
  offset += sizeof(uint64_t);
  return offset;
}

void generate_xpub(const uint32_t *path,
                   const size_t path_length,
                   const char *curve,
                   const uint8_t *seed,
                   char *str) {
  uint32_t fingerprint = 0x0, version;
  HDNode t_node;

  derive_hdnode_from_path(path, path_length - 1, curve, seed, &t_node);
  fingerprint = hdnode_fingerprint(&t_node);
  hdnode_private_ckd(&t_node, path[path_length - 1]);
  hdnode_fill_public_key(&t_node);

  get_version(path[0], path[1], NULL, &version);
  hdnode_serialize_public(&t_node, fingerprint, version, str, 113);
  memzero(&t_node, sizeof(HDNode));
}

void derive_hdnode_from_path(const uint32_t *path,
                             const size_t path_length,
                             const char *curve,
                             const uint8_t *seed,
                             HDNode *hdnode) {
  hdnode_from_seed(seed, 512 / 8, curve, hdnode);
  for (size_t i = 0; i < path_length; i++)
    hdnode_private_ckd(hdnode, path[i]);
  hdnode_fill_public_key(hdnode);
}

void get_address_node(const txn_metadata *txn_metadata_ptr,
                      const int16_t index,
                      const char *mnemonic,
                      const char *passphrase,
                      HDNode *hdnode) {
  uint8_t bip39seed[512 / 8];
  mnemonic_to_seed(mnemonic, passphrase, bip39seed, 0);
  hdnode_from_seed(bip39seed, 512 / 8, SECP256K1_NAME, hdnode);
  hdnode_private_ckd(hdnode,
                     BYTE_ARRAY_TO_UINT32(txn_metadata_ptr->purpose_index));
  hdnode_private_ckd(hdnode,
                     BYTE_ARRAY_TO_UINT32(txn_metadata_ptr->coin_index));
  hdnode_private_ckd(hdnode,
                     BYTE_ARRAY_TO_UINT32(txn_metadata_ptr->account_index));
  if (index == -1) {
    hdnode_private_ckd(
        hdnode, BYTE_ARRAY_TO_UINT32(txn_metadata_ptr->change[0].change_index));
    hdnode_private_ckd(
        hdnode,
        BYTE_ARRAY_TO_UINT32(txn_metadata_ptr->change[0].address_index));
  } else if (index >= 0) {
    hdnode_private_ckd(
        hdnode,
        BYTE_ARRAY_TO_UINT32(txn_metadata_ptr->input[index].change_index));
    hdnode_private_ckd(
        hdnode,
        BYTE_ARRAY_TO_UINT32(txn_metadata_ptr->input[index].address_index));
  }
  hdnode_fill_public_key(hdnode);
  memzero(bip39seed, sizeof(bip39seed));
}

const char *get_coin_symbol(uint32_t coin_index, uint64_t chain_id) {
  switch (coin_index) {
    case 0x80000000U:
    case 0x80000001:
      return "BTC";
    case 0x80000002:
      return "LTC";
    case 0x80000003:
      return "DOGE";
    case 0x80000005:
      return "DASH";
    case ETHEREUM: {
      switch (chain_id) {
        case ETHEREUM_MAINNET_CHAIN:
          return ETHEREUM_TOKEN_SYMBOL;
        case POLYGON_MAINNET_CHAIN:
          return POLYGON_TOKEN_SYMBOL;
        case BSC_MAINNET_CHAIN:
          return BSC_TOKEN_SYMBOL;
        case FANTOM_MAINNET_CHAIN:
          return FANTOM_TOKEN_SYMBOL;
        case AVALANCHE_MAINNET_CHAIN:
          return AVALANCHE_TOKEN_SYMBOL;
        case OPTIMISM_MAINNET_CHAIN:
          return OPTIMISM_TOKEN_SYMBOL;
        case ETC_MAINNET_CHAIN:
          return ETC_TOKEN_SYMBOL;
        case HARMONY_MAINNET_CHAIN:
          return HARMONY_TOKEN_SYMBOL;
        case ARBITRUM_MAINNET_CHAIN:
          return ARBITRUM_TOKEN_SYMBOL;
        default: {
          ASSERT(false);
          return "invalid";
        }
      }
    }
    case NEAR_COIN_INDEX:
      return NEAR_TOKEN_SYMBOL;
    case SOLANA:
      return "SOL";
    default: {
      ASSERT(false);
      return "invalid";
    }
  }
}

const char *get_coin_name(uint32_t coin_index, uint64_t chain_id) {
  switch (coin_index) {
    case 0x80000000:
      return "Bitcoin";
    case 0x80000001:
      return "Bitcoin Testnet";
    case 0x80000002:
      return "Litecoin";
    case 0x80000003:
      return "Dogecoin";
    case 0x80000005:
      return "Dash";
    case ETHEREUM: {
      switch (chain_id) {
        case ETHEREUM_MAINNET_CHAIN:
          return ETHEREUM_MAINNET_NAME;
        case POLYGON_MAINNET_CHAIN:
          return POLYGON_MAINNET_NAME;
        case BSC_MAINNET_CHAIN:
          return BSC_MAINNET_NAME;
        case FANTOM_MAINNET_CHAIN:
          return FANTOM_MAINNET_NAME;
        case AVALANCHE_MAINNET_CHAIN:
          return AVALANCHE_MAINNET_NAME;
        case OPTIMISM_MAINNET_CHAIN:
          return OPTIMISM_MAINNET_NAME;
        case ETC_MAINNET_CHAIN:
          return ETC_MAINNET_NAME;
        case HARMONY_MAINNET_CHAIN:
          return HARMONY_MAINNET_NAME;
        case ARBITRUM_MAINNET_CHAIN:
          return ARBITRUM_MAINNET_NAME;
        default: {
          ASSERT(false);
          return "invalid";
        }
      }
    }
    case NEAR_COIN_INDEX:
      return NEAR_TOKEN_NAME;
    case SOLANA:
      return "Solana";
    default: {
      ASSERT(false);
      return "invalid";
    }
  }
}

void get_version(const uint32_t purpose_id,
                 const uint32_t coin_index,
                 uint8_t *address_version,
                 uint32_t *pub_version) {
  uint8_t assigned_add_version = 0x0;
  uint32_t assigned_pub_version = 0x0;

  switch (purpose_id) {
    case NATIVE_SEGWIT:
      switch (coin_index) {
        case BTC_TEST:
          assigned_pub_version = 0x045f1cf6;
          assigned_add_version = 0x6f;
          break;
        case BITCOIN:
          assigned_pub_version = 0x04b24746;
          assigned_add_version = 0x00;
          break;
        case LITCOIN:
          assigned_pub_version = 0x0488b21e;
          assigned_add_version = 0x30;
          break;
        case DOGE:
          assigned_pub_version = 0x02facafd;
          assigned_add_version = 0x1E;
          break;
        case DASH:
          assigned_pub_version = 0x0488b21e;
          assigned_add_version = 0x4c;
          break;
        default:
          break;
      }
      break;

    case NON_SEGWIT:
      switch (coin_index) {
        case BTC_TEST:
          assigned_pub_version = 0x043587cf;
          assigned_add_version = 0x6f;
          break;
        case BITCOIN:
          assigned_pub_version = 0x0488b21e;
          assigned_add_version = 0x00;
          break;
        case LITCOIN:
          assigned_pub_version = 0x0488b21e;
          assigned_add_version = 0x30;
          break;
        case DOGE:
          assigned_pub_version = 0x02facafd;
          assigned_add_version = 0x1E;
          break;
        case DASH:
          assigned_pub_version = 0x0488b21e;
          assigned_add_version = 0x4c;
          break;
        case ETHEREUM:
        case NEAR:
          assigned_pub_version = 0x0488b21e;
          assigned_add_version = 0x00;
          break;
        case SOLANA:
          assigned_pub_version = 0x0488b21e;
          assigned_add_version = 0x00;
          break;
        default:
          break;
      }
      break;

    default:
      break;
  }

  if (address_version) {
    *address_version = assigned_add_version;
  }

  if (pub_version) {
    *pub_version = assigned_pub_version;
  }
}

bool validate_txn_metadata(const txn_metadata *mdata_ptr) {
  if (mdata_ptr->purpose_index[0] < 0x80 || mdata_ptr->coin_index[0] < 0x80 ||
      mdata_ptr->account_index[0] < 0x80)
    return false;
  if (BYTE_ARRAY_TO_UINT32(mdata_ptr->purpose_index) == NON_SEGWIT &&
      (BYTE_ARRAY_TO_UINT32(mdata_ptr->coin_index) == NEAR ||
       BYTE_ARRAY_TO_UINT32(mdata_ptr->coin_index) == SOLANA)) {
    if (mdata_ptr->input_count[0] > 0 &&
        (mdata_ptr->input->change_index[0] < 0x80 ||
         mdata_ptr->input->address_index[0] < 0x80))
      return false;
    return true;
  }
  if (mdata_ptr->input_count[0] > 0 &&
      (mdata_ptr->input->change_index[0] >= 0x80 ||
       mdata_ptr->input->address_index[0] >= 0x80))
    return false;
  if (mdata_ptr->output_count[0] > 0 &&
      (mdata_ptr->output->change_index[0] >= 0x80 ||
       mdata_ptr->output->address_index[0] >= 0x80))
    return false;
  if (mdata_ptr->change_count[0] > 0 &&
      (mdata_ptr->change->change_index[0] >= 0x80 ||
       mdata_ptr->change->address_index[0] >= 0x80))
    return false;
  if (BYTE_ARRAY_TO_UINT32(mdata_ptr->purpose_index) == NON_SEGWIT &&
      BYTE_ARRAY_TO_UINT32(mdata_ptr->coin_index) == ETHEREUM &&
      mdata_ptr->token_name[0] == '\0')
    return false;
  return true;
}
ui_display_node *ui_create_display_node(const char *title,
                                        const size_t title_size,
                                        const char *value,
                                        const size_t value_size) {
  ui_display_node *result = cy_malloc(sizeof(ui_display_node));
  memzero(result, sizeof(ui_display_node));

  size_t title_length = strnlen(title, title_size) + 1;
  result->title = cy_malloc(title_length);
  memzero(result->title, title_length);
  strncpy(result->title, title, title_length - 1);

  size_t value_length = strnlen(value, value_size) + 1;
  result->value = cy_malloc(value_length);
  memzero(result->value, value_length);
  strncpy(result->value, value, value_length - 1);

  result->next = NULL;
  return result;
}

void bech32_addr_encode(char *output,
                        char *hrp,
                        uint8_t *address_bytes,
                        uint8_t byte_len) {
  uint8_t data[65] = {0};
  size_t datalen = 0;
  convert_bits(data, &datalen, 5, address_bytes, byte_len, 8, 1);
  bech32_encode(output, hrp, data, datalen);
}

bool verify_xpub_derivation_path(const uint32_t *path, uint8_t depth) {
  bool status = false;

  if (depth < 2)
    return status;

  uint32_t purpose = path[0], coin = path[1];

  switch (coin) {
    case NEAR:
      status = near_verify_derivation_path(path, depth);
      break;

    case SOLANA:
      status = sol_verify_derivation_path(path, depth);
      break;

    case LITCOIN:
    case DOGE:
    case DASH:        // m/44'/5'  /i'
    case ETHEREUM:    // m/44'/60' /i'
      status = (purpose == NON_SEGWIT && depth == XPUB_DEFAULT_DEPTH &&
                is_hardened(path[2]));
      break;

    case BTC_TEST:    // m/44'/1'  /i'
    case BITCOIN:     // m/44'/0'  /i'
      status = (purpose == NON_SEGWIT || purpose == NATIVE_SEGWIT) &&
               (depth == XPUB_DEFAULT_DEPTH) && is_hardened(path[2]);
      break;

    default:
      break;
  }

  return status;
}

bool verify_receive_derivation_path(const uint32_t *path, uint8_t depth) {
  bool status = false;

  if (depth < 2)
    return status;

  uint32_t purpose = path[0], coin = path[1];

  switch (coin) {
    case NEAR:
      status = near_verify_derivation_path(path, depth);
      break;

    case SOLANA:    // m/44'/501'/i'/j'
      status = sol_verify_derivation_path(path, depth);
      break;

    case LITCOIN:
    case DOGE:
    case DASH:    // m/44'/5'  /i'/0 /j
      status = (depth == ADDR_DEFAULT_DEPTH) && (purpose == NON_SEGWIT) &&
               is_hardened(path[2]) && (path[3] == 0) &&
               is_non_hardened(path[4]);
      break;

    case ETHEREUM:    // m/44'/60' /i'/0 /0
      status = (depth == ADDR_DEFAULT_DEPTH) && (purpose == NON_SEGWIT) &&
               is_hardened(path[2]) && (path[3] == 0) && (path[4] == 0);
      break;

    case BTC_TEST:
    case BITCOIN:    // m/44'/0'  /i'/0 /j
      status = (depth == ADDR_DEFAULT_DEPTH) &&
               (purpose == NON_SEGWIT || purpose == NATIVE_SEGWIT) &&
               is_hardened(path[2]) && (path[3] == 0) &&
               is_non_hardened(path[4]);
      break;

    default:
      break;
  }

  return status;
}

FUNC_RETURN_CODES derivation_path_array_to_string(const uint32_t *path,
                                                  const size_t path_length,
                                                  const bool harden_all,
                                                  char *output,
                                                  const size_t out_len) {
  if (out_len == 0 || output == NULL || path == NULL)
    return FRC_INVALID_ARGUMENTS;

  int offset = 0;
  offset += snprintf(output + offset, out_len - offset, "m");

  for (int i = 0; i < path_length; i++) {
    const bool hardened = path[i] & 0x80000000;
    const uint32_t value = path[i] & 0x7FFFFFFF;

    if (out_len <= offset)
      return FRC_SIZE_EXCEEDED;

    offset += snprintf(output + offset, out_len - offset, "/%ld", value);

    if (out_len <= offset)
      return FRC_SIZE_EXCEEDED;

    if (harden_all || hardened)
      offset += snprintf(output + offset, out_len - offset, "'");

    // extra check needed as snprintf returns estimated size rather than actual
    // size written
    if (out_len <= offset)
      return FRC_SIZE_EXCEEDED;
  }
  return FRC_SUCCESS;
}
