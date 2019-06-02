#ifndef _ATOMICBOX_HPP_
#define _ATOMICBOX_HPP_

#include <atomic>
#include <memory>
#include <optional>

namespace DrLD {

/// An atomic nullable Box
template<typename T>
class AtomicBox
{
public:
    AtomicBox() : ptr(nullptr) {}
    ~AtomicBox() {}

    inline bool is_created() { return ptr >= (T*)BOX_CREATED; }
    inline const std::optional<T*> get() const {
        T *cur_ptr = ptr;
        return cur_ptr >= (T*)BOX_CREATED ? std::make_optional(cur_ptr) : std::nullopt;
    }

    T *get_or_create() {
        // TODO: Need a test for this locking schema
        T *cur_ptr = ptr;
        if (cur_ptr >= (T*)BOX_CREATED)
            return cur_ptr;

        if (cur_ptr == (T*)BOX_NULL) {
            // Change from BOX_NULL to BOX_CREATING successfully
            if (ptr.compare_exchange_weak(cur_ptr, (T*)BOX_CREATING,
                                          std::memory_order_release,
                                          std::memory_order_relaxed))
            {
                cur_ptr = new T;
                ptr = cur_ptr;
                return cur_ptr;
            }
        }

        // local spin. maybe not efficient on NUMA, but should have little cost
        // since wlock will be immediately acquired after setting LOADING
        while (ptr == (T*)BOX_CREATING) {
            // TODO: bench performance
            // works only on x86
            asm volatile("pause" ::: "memory");
        }

        return ptr;
    }

private:
    enum BoxState { BOX_NULL = 0, BOX_CREATING = 1, BOX_CREATED = 2};
    mutable std::atomic<T*> ptr;
};

}

#endif /* _ATOMICBOX_HPP_ */
