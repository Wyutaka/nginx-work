//
// Created by cerussite on 2019/10/11.
//

#pragma once

// #include <cpp17/string_view.hpp>
#include <string_view>

namespace proxy {
    class Peer {
    public:
        using id_type = uint64_t;

    private:
        // cpp17::string_view _id;
        id_type _id;

    public:
        Peer() = default;
        explicit Peer(id_type id)
            : _id(std::move(id)) {}

        Peer(const Peer &) = default;
        Peer(Peer &&) = default;

        Peer &operator=(const Peer &) = default;
        Peer &operator=(Peer &&) = default;

        ~Peer() = default;

    public:
        const id_type &id() const noexcept { return _id; }

    public:
        void id(id_type n){_id = n;}

    public:
        void printId() const noexcept {
            std::cout << "peer id = " << id() << std::endl;
        }

    public:
        bool operator==(const Peer &rhs) const { return id() == rhs.id(); }
        bool operator!=(const Peer &rhs) const { return !((*this) == rhs); }
    };

    class PeerHash {
    public:
        std::size_t operator()(const Peer &p) const { return std::hash<Peer::id_type>{}(p.id()); }
    };
} // namespace proxy
