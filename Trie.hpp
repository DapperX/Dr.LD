#ifndef _TRIE_HPP_
#define _TRIE_HPP_

#include "AtomicBox.hpp"
#include "base.h"

#include <cstddef>
#include <iostream>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

const size_t TRIE_BITS = 4;
const size_t TRIE_WIDTH = 1 << TRIE_BITS;
const size_t TRIE_MASK = TRIE_WIDTH - 1;

namespace DrLD {

template<typename V>
class Trie
{
public:
    Trie() {}
    ~Trie() {}

    // V *get(const u8 *key) { return (V*)((const Trie*)this)->get(key); }

    const V *get(const u8 *key) const {
        const Node *p = &_root;

        while (size_t c = key[0]) {
            auto child = p->_children[c & TRIE_MASK].get();
            if (!child.has_value()) return {};
            p = child.value();
            child = p->_children[c >> TRIE_BITS].get();
            if (!child.has_value()) return {};
            p = child.value();
            ++key;
        }
        return p->_value ? &p->_value.value() : nullptr;
    }

    template<typename Func>
    std::pair<std::optional<V>, bool> put(const u8 *key, V &&v, Func &&replace) {
        return _root.put(key, std::move(v), std::move(replace), false);
    }

/*    template<typename Func>
    void serialize(Func &&f) const {
        ;
    } */

    template<typename Func>
    void serialize_par(Func &&f) const {
        std::vector<u8> keybuf;
        _root.serialize_par(f, keybuf, 0, 0, false);
    }

/*    /// Locked version
    template<typename Func>
    void serialize_par(Func &&f) */

private:
    struct Node {
        Node() : _value(std::nullopt) {}

        std::atomic<size_t> _size; //, _byte_size;
        AtomicBox<Node> _children[TRIE_WIDTH];
        std::optional<V> _value;
        std::mutex _value_lock;

        template<typename Func>
        std::pair<std::optional<V>, bool> put(const u8 *key, V &&v, Func &&replace, bool is_highbits) {
            if (*key == '\0' && !is_highbits) {
                std::lock_guard<std::mutex> guard(_value_lock);
                if (_value.has_value() && !replace(_value.value(), v))
                    return std::make_pair(std::nullopt, false);

                /* if (_value.has_value())
                    _byte_size -= _value.value().raw_size();
                else
                    ++_size;
                _byte_size += v.raw_size(); */
                if (!_value.has_value())
                    ++_size;

                std::optional<V> ret = v;
                std::swap(_value, ret);
                return std::make_pair(std::move(ret), true);
            }

            size_t idx = key[0];
            if (is_highbits)
                idx >>= TRIE_BITS;
            else
                idx &= TRIE_MASK;

            auto child = _children[idx].get_or_create();

            // size_t value_raw_size = v.raw_size();
            std::pair<std::optional<V>, bool> ret;
            if (is_highbits)
                ret = child->put(key + 1, std::move(v), std::move(replace), !is_highbits);
            else
                ret = child->put(key, std::move(v), std::move(replace), !is_highbits);

            if (ret.second) {
                /* if (ret.first.has_value())
                    _byte_size -= ret.first.value().raw_size();
                else
                    ++_size;
                _byte_size += value_raw_size; */
                if (!ret.first.has_value())
                    ++_size;
            }

            return ret;
        }

        template<typename Func>
        void serialize_par(Func &f, std::vector<u8> &key, size_t rank, size_t /*offset*/, bool is_highbits) const {
            if (_value.has_value())
                f(key, _value.value(), rank); //, offset);
            if (!is_highbits)
                key.push_back(0);
            size_t rank_diff = 0, offset_diff = 0;
            for (int i = 0; i < TRIE_WIDTH; ++i) {
                auto child_ptr = _children[i].get();
                if (!child_ptr.has_value())
                    continue;
                auto &child = *child_ptr.value();
                u8 &c = key.back();
                if (is_highbits) {
                    c &= TRIE_MASK;     // Clear the modified high bits
                    c |= (i << TRIE_BITS);
                } else {
                    c = i;
                }
                child.serialize_par(f, key, rank + rank_diff, 0 /*offset + offset_diff*/, !is_highbits);
                rank_diff += child._size;
                // offset_diff += child._byte_size;
            }
            if (!is_highbits)
                key.pop_back();
        }
    };

    Node _root;
};

}

#endif /* _TRIE_HPP_ */
