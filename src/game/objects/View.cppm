
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <span>

export module beatuy.game.objects:view;

import beatuy.game.memory;

export namespace game {
template <class E> struct ArrayLayout {
    static constexpr uint32_t stride = E::byteSize;
    static E elementAt(char* at) {
        return E(at);
    }
};

template <class E> struct ArrayLayout<E*> {
    static constexpr uint32_t stride = sizeof(void*);
    static E elementAt(char* at) {
        return E(*reinterpret_cast<char**>(at));
    }
};

template <class E> class Array : public std::ranges::view_base {
    using Layout = ArrayLayout<E>;

public:
    using Element = decltype(Layout::elementAt(nullptr));

    class Iterator {
    public:
        using iterator_concept = std::random_access_iterator_tag;
        using value_type = Element;
        using difference_type = std::ptrdiff_t;

        Iterator() = default;
        explicit Iterator(char* at) : current(at) {}

        Element operator*() const {
            return Layout::elementAt(current);
        }
        Element operator[](difference_type offset) const {
            return *(*this + offset);
        }
        Iterator& operator++() {
            current += Layout::stride;
            return *this;
        }
        Iterator operator++(int) {
            Iterator was = *this;
            ++*this;
            return was;
        }
        Iterator& operator--() {
            current -= Layout::stride;
            return *this;
        }
        Iterator operator--(int) {
            Iterator was = *this;
            --*this;
            return was;
        }
        Iterator& operator+=(difference_type offset) {
            current += offset * difference_type(Layout::stride);
            return *this;
        }
        Iterator& operator-=(difference_type offset) {
            return *this += -offset;
        }
        friend Iterator operator+(Iterator iterator, difference_type offset) {
            return iterator += offset;
        }
        friend Iterator operator+(difference_type offset, Iterator iterator) {
            return iterator += offset;
        }
        friend Iterator operator-(Iterator iterator, difference_type offset) {
            return iterator -= offset;
        }
        friend difference_type operator-(const Iterator& left, const Iterator& right) {
            return (left.current - right.current) / difference_type(Layout::stride);
        }
        friend bool operator==(const Iterator&, const Iterator&) = default;
        friend auto operator<=>(const Iterator&, const Iterator&) = default;

    private:
        char* current = nullptr;
    };

    Array() = default;
    Array(char* begin, char* end) : storage(begin), storageEnd(end >= begin ? end : begin) {}

    Iterator begin() const {
        return Iterator(storage);
    }
    Iterator end() const {
        return Iterator(storage + size() * Layout::stride);
    }
    size_t size() const {
        return storage ? size_t(storageEnd - storage) / Layout::stride : 0;
    }
    bool empty() const {
        return size() == 0;
    }
    Element operator[](size_t index) const {
        return begin()[std::ptrdiff_t(index)];
    }
    char* data() const {
        return storage;
    }
    char* dataEnd() const {
        return storageEnd;
    }

private:
    char* storage = nullptr;
    char* storageEnd = nullptr;
};

class View {
public:
    View() = default;
    View(const void* pointer) : object(static_cast<char*>(const_cast<void*>(pointer))) {}

    explicit operator bool() const {
        return object != nullptr;
    }
    char* raw() const {
        return object;
    }
    friend bool operator==(const View&, const View&) = default;
    bool readable(size_t size) const {
        return game::readable(object, size);
    }

protected:
    template <class T> T& at(uint32_t offset) const {
        return field<T>(object, offset);
    }
    template <std::derived_from<View> V> V part(uint32_t offset) const {
        return V(object ? object + offset : nullptr);
    }
    template <std::derived_from<View> V> V reference(uint32_t offset) const {
        return V(object ? at<char*>(offset) : nullptr);
    }
    template <class R, class... A> R call(uint32_t slot, A... arguments) const {
        return callVirtual<R>(object, slot, arguments...);
    }
    template <class E> Array<E> arrayAt(uint32_t offset) const {
        return object ? Array<E>(at<char*>(offset), at<char*>(offset + 4)) : Array<E>();
    }
    template <class T> std::span<T> valuesAt(uint32_t offset) const {
        T* begin = object ? at<T*>(offset) : nullptr;
        T* end = object ? at<T*>(offset + 4) : nullptr;
        return std::span<T>(begin, begin && end > begin ? size_t(end - begin) : 0);
    }

    char* object = nullptr;
};

template <class V>
concept ObjectView = std::derived_from<V, View>;
}

template <class E> inline constexpr bool std::ranges::enable_borrowed_range<game::Array<E>> = true;

static_assert(std::ranges::random_access_range<game::Array<game::View*>>);
static_assert(std::ranges::sized_range<game::Array<game::View*>>);
static_assert(std::ranges::view<game::Array<game::View*>>);

