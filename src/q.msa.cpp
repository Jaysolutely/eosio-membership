#include <eosio/eosio.hpp>

class [[eosio::contract("msa")]] msa : public eosio::contract {
  public:
    msa(eosio::name receiver, eosio::name code, eosio::datastream<const char *> ds)
        : eosio::contract(receiver, code, ds) {}

    [[eosio::action]] void clubme(eosio::name user, std::string name) {
        eosio::require_auth(user);
        clubs_index clubs(eosio::contract::get_self(), eosio::contract::get_first_receiver().value);
        auto itr = clubs.find(user.value);
        if (itr == clubs.end())
            clubs.emplace(user, [&](auto &row) {
                row.key = user;
                row.name = name;
            });
    }

    [[eosio::action]] void join(eosio::name member, eosio::name club) {
        eosio::require_auth(member);
        clubs_index clubs(eosio::contract::get_self(), eosio::contract::get_first_receiver().value);
        auto member_itr = clubs.find(member.value);
        auto club_itr = clubs.find(club.value);
        eosio::check(member_itr == clubs.end(), "A club can not be the subject of this action");
        eosio::check(club_itr != clubs.end(), "Target is not a club");
        memberships_index memberships(eosio::contract::get_self(),
                                      eosio::contract::get_first_receiver().value);
        auto member_index = memberships.get_index<"bymember"_n>();
        auto itr = member_index.find(member.value);
        bool isMembership = false;
        while (itr != member_index.end()) {
            if (itr->club == club)
                isMembership = true;
            itr++;
        }
        eosio::check(!isMembership, "Membership already registered");
        memberships.emplace(member, [&](auto &row) {
            row.key = memberships.available_primary_key();
            row.club = club;
            row.member = member;
            row.refDetails = "";
        });
    }

  private:
    struct [[eosio::table]] club {
        eosio::name key;
        std::string name;
        uint64_t primary_key() const { return key.value; }
    };
    struct [[eosio::table]] membership {
        uint64_t key;
        eosio::name club;
        eosio::name member;
        std::string refDetails;
        uint64_t primary_key() const { return key; }
        uint64_t get_club() const { return club.value; }
        uint64_t get_member() const { return member.value; }
    };
    typedef eosio::multi_index<"clubs"_n, club> clubs_index;
    typedef eosio::multi_index<
        "memberships"_n, membership,
        eosio::indexed_by<"byclub"_n,
                          eosio::const_mem_fun<membership, uint64_t, &membership::get_club>>,
        eosio::indexed_by<"bymember"_n,
                          eosio::const_mem_fun<membership, uint64_t, &membership::get_member>>>
        memberships_index;
};
