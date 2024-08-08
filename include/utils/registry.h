#ifndef UTILS_REGISTRY_H
#define UTILS_REGISTRY_H

#include <cassert>
#include <map>

struct Handle {

    enum : unsigned { invalid = 0, start = 1 };

    unsigned regid = 0;
    unsigned id = 0;
};

template <>
struct std::hash<Handle> {
    std::size_t operator()(Handle const &h) const {
        return ((std::size_t)h.regid << 32) & h.id;
    }
};

template <>
struct std::less<Handle> {
    bool operator()(Handle const &lhs, Handle const &rhs) const {
        assert(lhs.regid == rhs.regid && "handles from different registries in the same container?");
        return lhs.id < rhs.id;
    }
};

struct RegistryIDGen {
    static unsigned id_gen;
};

template <
    typename Key,
    typename T,
    typename Compare = std::less<Key>
>
struct Registry {

    using key_type = Key;
    using mapped_type = T;

    unsigned id;
    unsigned curr_handle_id = Handle::start;

    std::map<Key, Handle, Compare> registry;
    std::map<Handle, T> entries;

    Registry() : id(RegistryIDGen::id_gen++) { }

    Handle put(Key k, T t) {
        auto &handle = registry[k];
        assert(handle.id == Handle::invalid && "re-registering?");
        handle = Handle{ id, curr_handle_id++ };
        entries[handle] = t;
        return handle;
    }

    T &operator[](Handle h) {
        assert(h.regid == id && "using handle from different registry?");
        auto it = entries.find(h);
        assert(it != entries.end() && "attemping to get non-registered entry?");
        return entries[h];
    }

};

#endif
