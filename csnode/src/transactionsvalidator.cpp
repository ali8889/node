#ifdef _MSC_VER
#include <intrin.h>
#endif

#include <csdb/amount.hpp>
#include <csdb/amount_commission.hpp>
#include <client/params.hpp>
#include <lib/system/logger.hpp>
#include <csnode/transactionsvalidator.hpp>
#include <smartcontracts.hpp>

namespace cs {
TransactionsValidator::TransactionsValidator(WalletsState& walletsState, const Config& config)
: config_(config)
, walletsState_(walletsState)
, cntRemovedTrxs_(0) {
  negativeNodes_.reserve(config_.initialNegNodesNum_);
}

void TransactionsValidator::reset(size_t transactionsNum) {
  trxList_.clear();
  trxList_.resize(transactionsNum, WalletsState::noInd_);

  negativeNodes_.clear();
  cntRemovedTrxs_ = 0;
}

bool TransactionsValidator::validateTransaction(const csdb::Transaction& trx, size_t trxInd, uint8_t& del1, bool newState) {
  if (!validateTransactionAsSource(trx, trxInd, del1, newState)) {
    return false;
  }
  return validateTransactionAsTarget(trx);
}

bool TransactionsValidator::validateTransactionAsSource(const csdb::Transaction& trx, size_t trxInd, uint8_t& del1, bool newState) {
  WalletsState::WalletId walletId{};
  WalletsState::WalletData& wallState = walletsState_.getData(trx.source(), walletId);

  WalletsState::WalletId walletIdNewState{};
  WalletsState::WalletData& wallStateIfNewState = walletsState_.getData(trx.target(), walletIdNewState);
  csdb::Amount feeForExecution(0);
  if (newState) {
    feeForExecution = trx.user_field(trx_uf::new_state::Fee).value<csdb::Amount>();
  }

#ifndef WITHOUT_DELTA
  csdb::Amount newBalance;
  if (!newState && !SmartContracts::is_executable(trx)) {
    newBalance = wallState.balance_ - trx.amount() - csdb::Amount(trx.counted_fee().to_double());
  } else if (!newState && SmartContracts::is_executable(trx)) {
    newBalance = wallState.balance_ - trx.amount() - csdb::Amount(trx.max_fee().to_double());
  } else {
    newBalance = wallState.balance_ - trx.amount() - feeForExecution;
  }
#ifdef _MSC_VER
  int8_t bitcnt = static_cast<decltype(bitcnt)>(__popcnt(newBalance.integral()) + __popcnt64(newBalance.fraction()));
#else
  int8_t bitcnt = __builtin_popcount(newBalance.integral()) + __builtin_popcountl(newBalance.fraction());
#endif

#ifndef SPAMMER
  if (newState && !wallStateIfNewState.trxTail_.isAllowed(trx.innerID())) {
    del1 = -bitcnt;
    return false;
  }
  else if (!newState && !wallState.trxTail_.isAllowed(trx.innerID())) {
    del1 = -bitcnt;
    return false;
  }
#endif

  del1 = bitcnt;
  wallState.balance_ = newBalance;
#else
#ifndef SPAMMER
  if (newState && !wallStateIfNewState.trxTail_.isAllowed(trx.innerID()))
    return false;
  else if (!newState && !wallState.trxTail_.isAllowed(trx.innerID()))
    return false;
#endif
  if (!newState && !SmartContracts::is_executable(trx)) {
    wallState.balance_ = wallState.balance_ - trx.amount() - csdb::Amount(trx.counted_fee().to_double());
  } else if (!newState && SmartContracts::is_executable(trx)) {
    wallState.balance_ = wallState.balance_ - trx.amount() - csdb::Amount(trx.max_fee().to_double());
  } else {
    wallState.balance_ = wallState.balance_ - trx.amount() - feeForExecution;
  }

#endif
  if (!newState) {
    wallState.trxTail_.push(trx.innerID());
    trxList_[trxInd] = wallState.lastTrxInd_;
    wallState.lastTrxInd_ = static_cast<decltype(wallState.lastTrxInd_)>(trxInd);
  } else {
    wallStateIfNewState.trxTail_.push(trx.innerID());
    trxList_[trxInd] = wallStateIfNewState.lastTrxInd_;
    wallStateIfNewState.lastTrxInd_ = static_cast<decltype(wallStateIfNewState.lastTrxInd_)>(trxInd);
    walletsState_.setModified(walletIdNewState);
  }

  walletsState_.setModified(walletId);


  if (wallState.balance_ < zeroBalance_) {
    negativeNodes_.push_back(&wallState);
  }

  return true;
}

bool TransactionsValidator::validateTransactionAsTarget(const csdb::Transaction& trx) {
  //TODO: test WalletId exists
  WalletsState::WalletId walletId{};
  WalletsState::WalletData& wallState = walletsState_.getData(trx.target(), walletId);

  wallState.balance_ = wallState.balance_ + trx.amount();

  walletsState_.setModified(walletId);
  return true;
}

void TransactionsValidator::validateByGraph(CharacteristicMask& maskIncluded, const Transactions& trxs,
                                            csdb::Pool& trxsExcluded) {
  while (!negativeNodes_.empty()) {
    Node& currNode = *negativeNodes_.back();
    negativeNodes_.pop_back();

    if (currNode.balance_ >= zeroBalance_) {
      continue;
    }

    removeTransactions(currNode, trxs, maskIncluded, trxsExcluded);
  }
}

void TransactionsValidator::removeTransactions(Node& node, const Transactions& trxs, CharacteristicMask& maskIncluded,
                                               csdb::Pool& trxsExcluded) {
  if (removeTransactions_PositiveOne(node, trxs, maskIncluded, trxsExcluded)) {
    return;
  }
  if (removeTransactions_PositiveAll(node, trxs, maskIncluded, trxsExcluded)) {
    return;
  }
  if (removeTransactions_NegativeOne(node, trxs, maskIncluded, trxsExcluded)) {
    return;
  }
  if (removeTransactions_NegativeAll(node, trxs, maskIncluded, trxsExcluded)) {
    return;
  }

  csdebug() << "removeTransactions: Failed to make balance non-negative ";
}

bool TransactionsValidator::removeTransactions_PositiveOne(Node& node, const Transactions& trxs,
                                                           CharacteristicMask& maskIncluded, csdb::Pool& trxsExcluded) {
  if (node.balance_ >= zeroBalance_)
    return true;

  const csdb::Amount absBalance = -node.balance_;
  TransactionIndex* prevNext = &node.lastTrxInd_;

  for (TransactionIndex trxInd = *prevNext; trxInd != WalletsState::noInd_; trxInd = *prevNext) {
    const csdb::Transaction& trx = trxs[trxInd];
    const csdb::Amount& trxCost = trx.amount().to_double() + trx.counted_fee().to_double();

    if (trxCost < absBalance) {
      prevNext = &trxList_[trxInd];
      continue;
    }

    WalletsState::WalletId walletId{};
    Node& destNode = walletsState_.getData(trx.target(), walletId);

    const bool isTrxPositive = (trx.amount() <= destNode.balance_);

    if (!isTrxPositive) {
      prevNext = &trxList_[trxInd];
      continue;
    }

    maskIncluded[trxInd] = 0;

#ifndef WITHOUT_BAD_BLOCK
    trxsExcluded.add_transaction(trx);
#endif

    node.balance_ = node.balance_ + trx.amount() + csdb::Amount(trx.counted_fee().to_double());
    destNode.balance_ = destNode.balance_ - trx.amount();

    *prevNext = trxList_[trxInd];
    trxList_[trxInd] = WalletsState::noInd_;

    ++cntRemovedTrxs_;

    return true;
  }

  return false;
}

bool TransactionsValidator::removeTransactions_PositiveAll(Node& node, const Transactions& trxs,
                                                           CharacteristicMask& maskIncluded, csdb::Pool& trxsExcluded) {
  if (node.balance_ >= zeroBalance_)
    return true;

  TransactionIndex* prevNext = &node.lastTrxInd_;

  for (TransactionIndex trxInd = *prevNext; trxInd != WalletsState::noInd_; trxInd = *prevNext) {
    const csdb::Transaction& trx = trxs[trxInd];

    WalletsState::WalletId walletId{};
    Node& destNode = walletsState_.getData(trx.target(), walletId);
    const bool isTrxPositive = (trx.amount() <= destNode.balance_);

    if (!isTrxPositive) {
      prevNext = &trxList_[trxInd];
      continue;
    }

    maskIncluded[trxInd] = 0;

#ifndef WITHOUT_BAD_BLOCK
    trxsExcluded.add_transaction(trx);
#endif

    node.balance_ = node.balance_ + trx.amount() + csdb::Amount(trx.counted_fee().to_double());
    destNode.balance_ = destNode.balance_ - trx.amount();

    *prevNext = trxList_[trxInd];
    trxList_[trxInd] = WalletsState::noInd_;

    ++cntRemovedTrxs_;

    if (node.balance_ >= zeroBalance_) {
      return true;
    }
  }

  return false;
}

bool TransactionsValidator::removeTransactions_NegativeOne(Node& node, const Transactions& trxs,
                                                           CharacteristicMask& maskIncluded, csdb::Pool& trxsExcluded) {
  if (node.balance_ >= zeroBalance_)
    return true;

  const csdb::Amount absBalance = -node.balance_;
  TransactionIndex* prevNext = &node.lastTrxInd_;

  for (TransactionIndex trxInd = *prevNext; trxInd != WalletsState::noInd_; trxInd = *prevNext) {
    const csdb::Transaction& trx = trxs[trxInd];
    const csdb::Amount& trxCost = trx.amount().to_double() + trx.counted_fee().to_double();

    if (trxCost < absBalance) {
      prevNext = &trxList_[trxInd];
      continue;
    }

    WalletsState::WalletId walletId{};
    Node& destNode = walletsState_.getData(trx.target(), walletId);

    maskIncluded[trxInd] = 0;

#ifndef WITHOUT_BAD_BLOCK
    trxsExcluded.add_transaction(trx);
#endif

    node.balance_ = node.balance_ + trx.amount() + csdb::Amount(trx.counted_fee().to_double());
    destNode.balance_ = destNode.balance_ - trx.amount();

    *prevNext = trxList_[trxInd];
    trxList_[trxInd] = WalletsState::noInd_;

    ++cntRemovedTrxs_;

    if (destNode.balance_ < zeroBalance_)
      negativeNodes_.push_back(&destNode);

    return true;
  }
  return false;
}

bool TransactionsValidator::removeTransactions_NegativeAll(Node& node, const Transactions& trxs,
                                                           CharacteristicMask& maskIncluded, csdb::Pool& trxsExcluded) {
  if (node.balance_ >= zeroBalance_)
    return true;

  TransactionIndex* prevNext = &node.lastTrxInd_;

  for (TransactionIndex trxInd = *prevNext; trxInd != WalletsState::noInd_; trxInd = *prevNext) {
    const csdb::Transaction& trx = trxs[trxInd];

    WalletsState::WalletId walletId{};
    Node& destNode = walletsState_.getData(trx.target(), walletId);

    maskIncluded[trxInd] = 0;

#ifndef WITHOUT_BAD_BLOCK
    trxsExcluded.add_transaction(trx);
#endif

    node.balance_ = node.balance_ + trx.amount() + csdb::Amount(trx.counted_fee().to_double());
    destNode.balance_ = destNode.balance_ - trx.amount();

    *prevNext = trxList_[trxInd];
    trxList_[trxInd] = WalletsState::noInd_;

    ++cntRemovedTrxs_;

    if (destNode.balance_ < zeroBalance_)
      negativeNodes_.push_back(&destNode);

    if (node.balance_ >= zeroBalance_)
      return true;
  }

  return false;
}
}  // namespace cs
