#ifndef BLOCKHASHES_HPP
#define BLOCKHASHES_HPP

#include <csdb/pool.hpp>
#include <vector>

namespace cs {
class BlockHashes {
public:
  struct DbStructure {
    csdb::Pool::sequence_t first_;
    csdb::Pool::sequence_t last_;
  };

public:
  BlockHashes();
  const DbStructure& getDbStructure() const {
    return db_;
  }
  bool empty() const {
    return hashes_.empty();
  }
  void initStart();
  bool initFromPrevBlock(csdb::Pool prevBlock);
  void initFinish();
  bool loadNextBlock(csdb::Pool nextBlock);

  bool find(csdb::Pool::sequence_t seq, csdb::PoolHash& res) const;

  csdb::PoolHash removeLast();

private:
  std::vector<csdb::PoolHash> hashes_;

  DbStructure db_;
  bool isDbInited_;
};
}  // namespace cs

#endif  //  BLOCKHASHES_HPP
