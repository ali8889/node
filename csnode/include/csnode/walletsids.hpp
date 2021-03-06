#ifndef WALLET_IDS_HPP
#define WALLET_IDS_HPP

#include <memory>
#include <type_traits>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>

#include <csdb/address.hpp>
#include <csdb/internal/types.hpp>

using namespace boost::multi_index;

namespace cs {

class WalletsIds {
public:
    using WalletId = csdb::internal::WalletId;
    using WalletAddress = csdb::Address;

public:
    class Special {
    public:
        static bool isSpecial(WalletId id);
        static WalletId makeSpecial(WalletId id);
        static WalletId makeNormal(WalletId id);

    public:
        Special(WalletsIds& norm);
        // returns true if new id was inserted
        bool insertNormal(const WalletAddress& address, WalletId id, WalletId& idSpecial);
        bool findAnyOrInsertSpecial(const WalletAddress& address, WalletId& id);

    private:
        WalletsIds& norm_;
        WalletId nextIdSpecial_;
        static constexpr uint32_t maskSpecial_ = (1u << 31);
        static constexpr WalletId noSpecial_ = 0;

        static_assert(std::is_integral<WalletId>::value, "WalletId is expected to be integer");
        static_assert(sizeof(WalletId) == sizeof(maskSpecial_), "sizeof(WalletId) == sizeof(maskSpecial_)");
    };

public:
    class Normal {
    public:
        Normal(WalletsIds& norm);

        // returns true if new id was inserted
        bool insert(const WalletAddress& address, WalletId id);
        bool find(const WalletAddress& address, WalletId& id) const;
        bool findaddr(const WalletId& id, WalletAddress& address) const;
        // returns true if new id was inserted
        bool get(const WalletAddress& address, WalletId& id);
        bool remove(const WalletAddress& address);

    private:
        WalletsIds& norm_;
    };

public:
    WalletsIds();
    WalletsIds(const WalletsIds&) = delete;
    WalletsIds& operator=(const WalletsIds&) = delete;
    WalletsIds(const WalletsIds&&) = delete;
    WalletsIds& operator=(const WalletsIds&&) = delete;

    Special& special() {
        return *special_;
    }
    const Special& special() const {
        return *special_;
    }

    Normal& normal() {
        return *norm_;
    }
    const Normal& normal() const {
        return *norm_;
    }

private:
    struct Wallet {
        WalletAddress address; struct byAddress {};
        WalletId id;  struct byId {};
    };

    using Data = multi_index_container<
        Wallet,
        indexed_by<
            hashed_unique<
                tag<Wallet::byAddress>, member<
                    Wallet, WalletAddress, &Wallet::address
                >
            >,
            hashed_unique<
                tag<Wallet::byId>, member<
                    Wallet, WalletId, &Wallet::id
                >
            >
        >
    >;

    Data data_;
    WalletId nextId_;
    std::unique_ptr<Special> special_;
    std::unique_ptr<Normal> norm_;
};

}  // namespace cs

#endif
