/**
 *  @file
 *  @copyright defined in evt/LICENSE.txt
*/

#pragma once
#include <deque>
#include <functional>
#include <eosio/chain/contracts/types.hpp>

namespace rocksdb {
class DB;
} // namespace rocksdb

namespace evt { namespace chain {

using namespace eosio::chain::contracts;

using read_domain_func = std::function<void(const domain_def&)>;
using read_token_func = std::function<void(const token_def&)>;
using read_group_func = std::function<void(const group_def&)>;

enum tokendb_error {
    ok = 0,
    domain_existed = -1,
    not_found_domain = -2,
    group_existed = -3,
    not_found_group = -4,
    token_id_existed = -5,
    not_found_token_id = -6,
    rocksdb_err = -7,
    no_checkpoint = -8,
    seq_not_valid = -9
};

class tokendb {
private:
    struct dbaction {
        int     type;
        void*   data;
    };
    struct checkpoint {
        uint32                  seq;
        const void*             rb_snapshot;
        std::vector<dbaction>   actions;
    };

public:
    tokendb() : db_(nullptr) {}

public:
    int initialize(const std::string& dbpath);

public:
    int add_domain(const domain_def&);
    int exists_domain(const domain_name);
    int issue_tokens(const issuetoken&);
    int exists_token(const domain_name, const token_name name);
    int add_group(const group_def&);
    int exists_group(const group_id&);

    int read_domain(const domain_name, const read_domain_func&) const;
    int read_token(const domain_name, const token_name, const read_token_func&) const;
    int read_group(const group_id&, const read_group_func&) const;

    // specific function, use merge operator to speed up rocksdb action.
    int update_domain(const updatedomain& ud);
    int update_group(const updategroup& ug);
    int transfer_token(const transfertoken& tt);

public:
    int add_checkpoint(uint32 seq);
    int rollback_to_latest();
    int pop_checkpoints(uint32 until);

private:
    int should_record() { return !checkpoints_.empty(); }
    int record(int type, void* data);
    int free_checkpoint(checkpoint&);

private:
    rocksdb::DB*            db_;
    std::deque<checkpoint>  checkpoints_;
};

}}  // namespace evt::chain
