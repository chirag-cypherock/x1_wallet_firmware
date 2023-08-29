/**
 * @file    evm_txn_helpers.h
 * @author  Cypherock X1 Team
 * @brief   Helper apis for interpreting and signing EVM transactions
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */
#ifndef EVM_TXN_HELPERS_H
#define EVM_TXN_HELPERS_H

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include "coin_utils.h"
#include "evm/sign_txn.pb.h"
#include "evm_contracts.h"

/*****************************************************************************
 * MACROS AND DEFINES
 *****************************************************************************/

#define ETHEREUM_PURPOSE_INDEX 0x8000002C
#define ETHEREUM_COIN_INDEX 0x8000003C

#define ETH_VALUE_SIZE_BYTES (32U)

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/**
 * @brief Enum used to represent the status of payload field in a transaction.
 *
 */
typedef enum {
  PAYLOAD_ABSENT = 0x0,                 // No payload present in the transaction
  PAYLOAD_SIGNATURE_NOT_WHITELISTED,    // Payload function signature is not
                                        // recognized [Blind Signing]
  PAYLOAD_CONTRACT_NOT_WHITELISTED,     // [OBSOLETE] Payload function signature
                                        // is whitelisted but contract is not
                                        // (for Transfer function) [Unverified
                                        // Contract] [OBSOLETE]
  PAYLOAD_CONTRACT_INVALID,    // Payload function signature and contract both
                               // are whitelisted but doesn't match [Invalid
                               // Transaction]
  PAYLOAD_WHITELISTED,    // Payload is completely recognized [Clear Signing]
} PAYLOAD_STATUS;

/**
 * @brief Struct to store Unsigned Ethereum Transaction details.
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
#pragma pack(push, 1)
typedef struct {
  uint8_t nonce_size[1];
  uint8_t nonce[32];

  uint8_t gas_price_size[1];
  uint8_t gas_price[32];

  uint8_t gas_limit_size[1];
  uint8_t gas_limit[32];

  uint8_t to_address[20];

  uint8_t value_size[1];
  uint8_t value[ETH_VALUE_SIZE_BYTES];

  uint64_t payload_size;
  const uint8_t *payload;

  uint8_t chain_id_size[1];
  uint8_t chain_id[8];

  uint8_t dummy_r[1];
  uint8_t dummy_s[1];

  PAYLOAD_STATUS payload_status;
} evm_unsigned_txn;
#pragma pack(pop)

typedef struct {
  /**
   * The structure holds the wallet information of the transaction.
   * @note Populated by handle_initiate_query()
   */
  evm_sign_txn_initiate_request_t init_info;

  /// remembers the allocated buffer for holding complete unsigned transaction
  uint8_t *transaction;
  /// store for decoded unsigned transaction info
  evm_unsigned_txn transaction_info;
  /// whitelisted contract in the transaction
  const erc20_contracts_t *contract;
} evm_txn_context_t;

/*****************************************************************************
 * EXPORTED VARIABLES
 *****************************************************************************/

/*****************************************************************************
 * GLOBAL FUNCTION PROTOTYPES
 *****************************************************************************/

/**
 * @brief Convert byte array representation of unsigned transaction to
 * evm_unsigned_txn.
 * @details
 *
 * @param [in] evm_utxn_byte_array  Byte array of unsigned transaction.
 * @param [in] byte_array_len               Length of byte array.
 * @param [out] unsigned_txn_ptr            Pointer to the evm_unsigned_txn
 * instance to store the transaction details.
 *
 * @return Status of conversion
 * @retval 0 Success
 * @retval -1 Failure
 */
int evm_byte_array_to_unsigned_txn(const uint8_t *evm_utxn_byte_array,
                                   size_t byte_array_len,
                                   evm_txn_context_t *txn_context);

/**
 * @brief Verifies the unsigned transaction.
 * @details
 *
 * @param [in] txn_context     Pointer to the evm_unsigned_txn instance.
 *
 * @return true, false
 * @retval true   If all the checks pass for the given instance
 * @retval false  If any of the checks pass for the given instance
 */
bool evm_validate_unsigned_txn(const evm_txn_context_t *txn_context);

/**
 * @brief Get the receivers address from evm_unsigned_txn instance.
 * @details
 *
 * @param [in] utxn_ptr     Pointer to Unsigned transaction
 * instance.
 * @param [in] address                  Byte array of receiver's address.
 * @param [in] metadata_ptr             Pointer to metadata instance
 *
 * @return
 * @retval
 */
void eth_get_to_address(const evm_unsigned_txn *utxn_ptr,
                        const uint8_t **address);

/**
 * @brief Get amount to be sent set in the evm_unsigned_txn instance
 * @details
 *
 * @param [in] utxn_ptr     Pointer to Unsigned transaction
 * instance.
 * @param [in] value                    char array to store value.
 *
 * @return
 * @retval
 */
uint32_t eth_get_value(const evm_unsigned_txn *utxn_ptr, char *value);

/**
 * @brief Return the string representation of decimal value of transaction fee
 * in ETH.
 *
 * @param utxn_ptr  The unsigned transaction containing gas_limit
 * and gas_price
 * @param fee_decimal_string    Output decimal string of at least 30 character
 * long
 */
void eth_get_fee_string(const evm_unsigned_txn *utxn_ptr,
                        char *fee_decimal_string,
                        uint8_t size,
                        uint8_t decimal);

/**
 * @brief Returns the decimal value of ethereum asset from metadata
 *
 * @param txn_context Pointer to transaction context
 * @return uint8_t Decimal value of current asset
 */
uint8_t evm_get_decimal(const evm_txn_context_t *txn_context);

/**
 * @brief Returns the asset symbol which is currently being used in the flow
 *
 * @param txn_context Pointer to transaction context
 * @return const char*
 */
const char *evm_get_asset_symbol(const evm_txn_context_t *txn_context);

/**
 * @brief Returns the title for address verification in ethereum send flow
 *        Contract address is verified when sending data with payload except for
 * whitelisted tokens
 * @param utxn_ptr Pointer to the unsigned transaction for ethereum
 * @return const char*
 */
const char *eth_get_address_title(const evm_unsigned_txn *utxn_ptr);

#endif
