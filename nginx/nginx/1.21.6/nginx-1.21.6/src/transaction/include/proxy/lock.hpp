//
// Created by cerussite on 2019/10/11.
//

#pragma once

#include <atomic>
#include <mutex>
#include <unordered_map>

#include <cpp17/string_view.hpp>

#include <proxy/detail/dpdk_hash_map.hpp>
#include <proxy/detail/dpdk_hash_set.hpp>
#include <proxy/detail/hash.hpp>
#include <proxy/system/hook.hpp>
#include <proxy/system/hook_context.hpp>
#include <proxy/types/request/request.hpp>
#include <proxy/types/response/response.hpp>

namespace proxy {
    namespace detail {
        class shared_mutex {
        private:
            std::atomic<int> _shared;
            std::atomic_flag _unique;

        public:
            shared_mutex()
                : _shared(0)
                , _unique() {}
            shared_mutex(const shared_mutex &) = delete;
            shared_mutex(shared_mutex &&) = delete;
            shared_mutex &operator=(const shared_mutex &) = delete;
            shared_mutex &operator=(shared_mutex &&) = delete;

        public:
            void lock_shared() {
                while (_unique.test_and_set())
                    ;
                _shared += 1;
                _unique.clear();
            }

            void unlock_shared() { _shared -= 1; }

        public:
            void lock() {
                while (_unique.test_and_set())
                    ;
                while (_shared > 0)
                    ;
            }
            void unlock() { _unique.clear(); }
        };

        template <class MtxT> class shared_lock {
        private:
            MtxT &_mutex;

        public:
            explicit shared_lock(MtxT &mtx)lock
                : _mutex(mtx) {
                _mutex.lock_shared();
            }

            ~shared_lock() { _mutex.unlock_shared(); }
        };
    } // namespace detail

    template <class NextF> class Lock {
    private:
        using Ul = std::unique_lock<
            detail::
                shared_mutex>; //ミューテックスのlock(),unlock()処理をコンストラクタとデストラクタで確実に実行するためのクラス
        using Sl = detail::shared_lock<
            detail::
                shared_mutex>; //ミューテクスのlock_shared(),unlock_shared()処理をコンストラクタとデストラクタで確実に実行するためのクラス

    private:
        struct Key {
            char table[16];
        };

    private:
        enum class LockedState {
            Unlocked,
            LockedByOther,
            LockedMySelf,
        };

    private:
        detail::shared_mutex _mutex;
        // std::unordered_map<cpp17::string_view, Peer, proxy::string_view_hash> _locked;
        dpdk_hash_map<Key, Peer> _locked;
        dpdk_hash_set<Peer> _tx_otw; //トランザクション中のクライアントを記憶しておく
        std::unordered_map<Peer,std::vector<Key>, PeerHash> _client_to_key; //どのクライアントがどのKeyをロックしているかを記憶しておく

        NextF _next;

    public:
        explicit Lock(NextF next)
            : _next(std::move(next)) {}

    private:
        static std::string _toLockedKeyString(const std::pair<std::string, std::string> &kv) {
            return kv.first + '=' + kv.second;
        }

        LockedState _checkLockedByOtherImpl(const cpp17::string_view &key, const Peer &peer) {
            Peer p;
            Key k = {};
            memcpy(&k, key.data(), std::min(sizeof(k), key.size()));
            auto offset = _locked.get_data(k, &p);
            if (offset < 0) {
                return LockedState::Unlocked;
            }else if(peer != p){
               return LockedState::LockedByOther;
            }else{
                return LockedState::LockedMySelf;
            }
        }

        LockedState _checkLockedByOther(const cpp17::string_view &key, const Peer &peer) {
            // Sl sl(_mutex);
            return _checkLockedByOtherImpl(key, peer);
        }
        bool _checkLockedByOther(const Peer &peer) {
            // TODO ハッシュテーブルにあるデータをすべて持ってきて比較する
            // rte_hash_lookup_bulk_dataを使う?

            // Sl sl(_mutex);
            // for (const auto &kv : _locked) {
            //    if (kv.second != peer)
            //        return true;
            //}
            // return false;
        }

        bool _getLock(const cpp17::string_view &key, const Peer &peer) {
            //Ul ul(_mutex);

            auto locked_state = _checkLockedByOtherImpl(key, peer);
            if (locked_state == LockedState::LockedByOther) {
                return false;
            }

            Key k = {};
            memcpy(&k, key.data(), std::min(sizeof(k), key.size()));

            _locked.add_key_value(k, peer);
            if(_client_to_key.find(peer) != _client_to_key.end()){
                auto keys = _client_to_key.at(peer);
                keys.emplace_back(k);
                _client_to_key[peer] = keys;
            }else{
                std::vector<Key> keys{k};
                _client_to_key.emplace(peer, keys);
            }
            return true;
        }

    public:
        void operator()(Request req, Response &res) { 
            const auto &query = boost::get<request::RequestQuery>(req.query());

            switch (query.type()) {
            case request::RequestQuery::Type::Begin: {
                const auto& peer = req.peer();
                _tx_otw.set(peer);
                break;
            }
            case request::RequestQuery::Type::Commit: {
                const auto& peer = req.peer();
                // TODO 本来なら全てのクエリをCassandraで処理し終えてからremoveすべき
                _tx_otw.remove(peer);
                _client_to_key.erase(peer);
                break;
            }
            case request::RequestQuery::Type::Rollback: {
                // TODO ロックを解除する必要がある
                const auto& peer = req.peer();
                const auto& keys = _client_to_key.at(peer);
                for(const auto& key : keys){
                    _locked.del_key(key);
                    // TODO Hookに登録されているKeyも解除する必要がある
                }
                _tx_otw.remove(peer);
                _client_to_key.erase(peer);
                break;
            }
            case request::RequestQuery::Type::Lwt: {
                const auto &key = query.key();
                if (!_getLock(*key, req.peer())) {
                    Aggr aggr = req.aggr();
                    res.end(std::move(req), Status::AlreadyLocked, aggr);
                    return;
                }

                auto ft = [this,key=key->str(),peer=req.peer()](HookContext &response){
                  Key k = {};
                  memcpy(&k, key.data(), std::min(sizeof(k), key.size()));
                  _client_to_key.erase(peer);
                  _locked.del_key(k);
                };

                int16_t streamID = *(reinterpret_cast<int16_t *>(query.raw().data()+1+1));

                SingletonHook::AddHook(req.aggr().order(),SingletonHook::CreateKey(req.peer(), streamID), std::move(ft));

                break;
            }
            case request::RequestQuery::Type::Unknown:{
                break;
            }
            default: {
                const auto &key = query.key();
                if(key->data() == nullptr){
                    break;
                }
                auto tx_state = _tx_otw.has(req.peer());

                auto locked_state = _checkLockedByOther(*key, req.peer());
                if(locked_state == LockedState::LockedByOther){
                    // TODO keyがない場合を考えなければならない（ただのselectなど）
                    // TODO ロックに失敗した場合、他のスレッドに伝える必要がある(アプリケーション側で対応？)
                    Aggr aggr = req.aggr();
                    res.end(std::move(req), Status::AlreadyLocked, aggr);
                    return;
                }

                if(tx_state) { //トランザクション
                    if(locked_state ==
                        LockedState::
                            LockedMySelf) { // すでに自身でロックを獲得している(トランザクション処理中)
                        // TODO 一つのトランザクション中に、同一Keyに対する処理が複数ある場合、複数回delされてしまう
                        auto ft = [this, key = key->str()](HookContext &response) {
                            Key k = {};
                            memcpy(&k, key.data(), std::min(sizeof(k), key.size()));
                            response.abort();
                            _locked.del_key(k);
                        };

                        int16_t streamID =
                            *(reinterpret_cast<int16_t *>(query.raw().data() + 1 + 1));

                        SingletonHook::AddHook(req.aggr().order(), SingletonHook::CreateKey(req.peer(), streamID),
                                               std::move(ft));
                        break;
                    }else {
                        if(!_getLock(*key, req.peer())){
                            DebugPrint("cannot get lock");
                        }
                        // TODO 一つのトランザクション中に、同一Keyに対する処理が複数ある場合、複数回delされてしまう
                        auto ft = [this, key = key->str()](HookContext &response) {
                          Key k = {};
                          memcpy(&k, key.data(), std::min(sizeof(k), key.size()));
                          response.abort();
                          _locked.del_key(k);
                        };
                        int16_t streamID =
                            *(reinterpret_cast<int16_t *>(query.raw().data() + 1 + 1));

                        SingletonHook::AddHook(req.aggr().order(), SingletonHook::CreateKey(req.peer(), streamID),
                                               std::move(ft));

                        break;
                    }
                }else{ //非トランザクション
                    break;
                }
            }
            }
            _next(std::move(req), res);
        }
    };
} // namespace proxy
