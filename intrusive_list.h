#pragma once
#include <type_traits>
#include <iterator>
#include <memory>

namespace intrusive {
    struct default_tag;

    template <typename Tag = default_tag>
    struct list_element {
        list_element *next = nullptr, *prev = nullptr;

        void unlink() noexcept {
            if (next != nullptr) {
                next->prev = prev;
            }
            if (prev != nullptr) {
                prev->next = next;
            }
            next = nullptr;
            prev = nullptr;
        }
    };

    template <typename T, typename Tag>
    struct list_iterator
    {
        template<typename E, typename D>
        friend class list_iterator;
        template<typename E, typename D>
        friend class list;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = std::remove_const_t<T>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        list_iterator() = default;

        template <typename NonConstIterator>
        list_iterator<T, Tag>(NonConstIterator other,
                      std::enable_if_t<
                              std::is_same_v<NonConstIterator, list_iterator<std::remove_const_t<T>, Tag>> &&
                              std::is_const_v<T>>* = nullptr) noexcept : current(other.current) {}

        T& operator*() const noexcept;
        T* operator->() const noexcept;

        list_iterator<T, Tag>& operator++() & noexcept;
        list_iterator<T, Tag>& operator--() & noexcept;

        list_iterator<T, Tag> operator++(int) & noexcept;
        list_iterator<T, Tag> operator--(int) & noexcept;

        bool operator==(list_iterator<T, Tag> const& rhs) const& noexcept;
        bool operator!=(list_iterator<T, Tag> const& rhs) const& noexcept;

    private:
        explicit list_iterator(list_element<Tag>* current) noexcept;

    private:
        list_element<Tag>* current;
    };

    template<typename T, typename Tag>
    T& list_iterator<T, Tag>::operator*() const noexcept {
        return static_cast<T&>(*current);
    }

    template<typename T, typename Tag>
    T* list_iterator<T, Tag>::operator->() const noexcept {
        return static_cast<T*>(current);
    }

    template<typename T, typename Tag>
    list_iterator<T, Tag>& list_iterator<T, Tag>::operator++()& noexcept {
        current = current->next;
        return *this;
    }

    template<typename T, typename Tag>
    list_iterator<T, Tag>& list_iterator<T, Tag>::operator--()& noexcept {
        current = current->prev;
        return *this;
    }

    template<typename T, typename Tag>
    list_iterator<T, Tag> list_iterator<T, Tag>::operator++(int)& noexcept {
        auto old = *this;
        ++(*this);
        return old;
    }

    template<typename T, typename Tag>
    list_iterator<T, Tag> list_iterator<T, Tag>::operator--(int)& noexcept {
        auto old = *this;
        --(*this);
        return old;
    }

    template<typename T, typename Tag>
    bool list_iterator<T, Tag>::operator==(const list_iterator<T, Tag>& rhs) const& noexcept {
        return current == rhs.current;
    }

    template<typename T, typename Tag>
    bool list_iterator<T, Tag>::operator!=(const list_iterator<T, Tag>& rhs) const& noexcept {
        return current != rhs.current;
    }

    template<typename T, typename Tag>
    list_iterator<T, Tag>::list_iterator(list_element<Tag>* current) noexcept : current(current) {}

    template <typename T, typename Tag = default_tag>
    struct list {
        typedef list_iterator<T, Tag> iterator;
        typedef list_iterator<T const, Tag> const_iterator;

        list() noexcept;
        list(list const&) = delete;
        list(list&&) noexcept;
        ~list();

        void swap(list&) noexcept;

        list<T, Tag>& operator=(list const&) = delete;
        list<T, Tag>& operator=(list&&) noexcept;

        void clear() noexcept;

        void push_back(T&) noexcept;
        void pop_back() noexcept;
        T& back() noexcept;
        T const& back() const noexcept;

        void push_front(T&) noexcept;
        void pop_front() noexcept;
        T& front() noexcept;
        T const& front() const noexcept;

        bool empty() const noexcept;

        iterator begin() noexcept;
        const_iterator begin() const noexcept;

        iterator end() noexcept;
        const_iterator end() const noexcept;

        iterator insert(const_iterator pos, T&) noexcept;
        iterator erase(const_iterator pos) noexcept;
        void splice(const_iterator pos, list&, const_iterator first, const_iterator last) noexcept;

    private:
        list_element<Tag> fake;
    };

    template<typename T, typename Tag>
    list<T, Tag>::list() noexcept : fake() {
        fake.next = &fake;
        fake.prev = &fake;
    }

    template<typename T, typename Tag>
    list<T, Tag>::list(list&& rhs) noexcept : list() {
        swap(rhs);
        rhs.clear();
    }

    template<typename T, typename Tag>
    list<T, Tag>::~list() {
        clear();
    }

    template<typename T, typename Tag>
    list<T, Tag>& list<T, Tag>::operator=(list&& rhs) noexcept {
        swap(rhs);
        rhs.clear();
        return *this;
    }

    template<typename T, typename Tag>
    void list<T, Tag>::clear() noexcept {
        while (fake.next != &fake) {
            fake.next->unlink();
        }
    }

    template<typename T, typename Tag>
    void list<T, Tag>::push_back(T& val) noexcept {
        static_assert(std::is_convertible_v<T&, list_element<Tag>&>,
                      "value type is not convertible to list_element");
        auto &cur = static_cast<list_element<Tag>&>(val);
        cur.next = &fake;
        cur.prev = fake.prev;
        fake.prev->next = &cur;
        fake.prev = &cur;
    }

    template<typename T, typename Tag>
    void list<T, Tag>::pop_back() noexcept {
        fake.prev->unlink();
    }

    template<typename T, typename Tag>
    T& list<T, Tag>::back() noexcept {
        return static_cast<T&>(*(fake.prev));
    }

    template<typename T, typename Tag>
    T const& list<T, Tag>::back() const noexcept {
        return static_cast<T const&>(*(fake.prev));
    }

    template<typename T, typename Tag>
    void list<T, Tag>::push_front(T& val) noexcept {
        static_assert(std::is_convertible_v<T&, list_element<Tag>&>,
                      "value type is not convertible to list_element");
        auto &cur = static_cast<list_element<Tag>&>(val);
        cur.prev = &fake;
        cur.next = fake.next;
        fake.next->prev = &cur;
        fake.next = &cur;
    }

    template<typename T, typename Tag>
    void list<T, Tag>::pop_front() noexcept {
        fake.next->unlink();
    }

    template<typename T, typename Tag>
    T& list<T, Tag>::front() noexcept {
        return static_cast<T&>(*(fake.next));
    }

    template<typename T, typename Tag>
    T const& list<T, Tag>::front() const noexcept {
        return static_cast<T const&>(*(fake.next));
    }

    template<typename T, typename Tag>
    bool list<T, Tag>::empty() const noexcept {
        return &fake == fake.next;
    }

    template<typename T, typename Tag>
    typename list<T, Tag>::iterator list<T, Tag>::begin() noexcept {
        return intrusive::list<T, Tag>::iterator(fake.next);
    }

    template<typename T, typename Tag>
    typename list<T, Tag>::iterator list<T, Tag>::end() noexcept {
        return intrusive::list<T, Tag>::iterator(&fake);
    }

    template<typename T, typename Tag>
    typename list<T, Tag>::const_iterator list<T, Tag>::begin() const noexcept {
        return intrusive::list<T, Tag>::const_iterator(fake.next);
    }

    template<typename T, typename Tag>
    typename list<T, Tag>::const_iterator list<T, Tag>::end() const noexcept {
        return intrusive::list<T, Tag>::const_iterator(const_cast<list_element<Tag>*>(&fake));
    }

    template<typename T, typename Tag>
    typename list<T, Tag>::iterator list<T, Tag>::insert(list::const_iterator pos, T& val) noexcept {
        auto &list_ent = const_cast<list_element<Tag>&>(static_cast<list_element<Tag> const&>(*pos));
        auto &new_list_ent = static_cast<list_element<Tag>&>(val);
        new_list_ent.next = &list_ent;
        new_list_ent.prev = list_ent.prev;
        list_ent.prev->next = &new_list_ent;
        list_ent.prev = &new_list_ent;
        return list_iterator<T, Tag>(&new_list_ent);
    }

    template<typename T, typename Tag>
    typename list<T, Tag>::iterator list<T, Tag>::erase(list::const_iterator pos) noexcept {
        auto &list_ent = const_cast<list_element<Tag>&>(static_cast<list_element<Tag> const&>(*pos));
        auto *ret = list_ent.next;
        list_ent.unlink();
        return list_iterator<T, Tag>(ret);
    }

    template<typename T, typename Tag>
    void list<T, Tag>::splice(list::const_iterator pos, list& other, list::const_iterator first,
                              list::const_iterator last) noexcept {
        if (first == last) {
            return;
        }
        auto &to_right = const_cast<list_element<Tag>&>(static_cast<list_element<Tag> const&>(*pos));
        auto &to_left = *(to_right.prev);
        auto &from_right = const_cast<list_element<Tag>&>(static_cast<list_element<Tag> const&>(*std::prev(last)));
        auto &from_left = const_cast<list_element<Tag>&>(static_cast<list_element<Tag> const&>(*first));

        to_left.next = &from_left;
        to_right.prev = &from_right;

        from_left.prev->next = from_right.next;
        from_right.next->prev = from_left.prev;

        from_left.prev = &to_left;
        from_right.next = &to_right;
    }

    template<typename T, typename Tag>
    void list<T, Tag>::swap(list &rhs) noexcept {
        auto &my_next = *(fake.next);
        auto &my_prev = *(fake.prev);
        my_next.prev = &(rhs.fake);
        my_prev.next = &(rhs.fake);

        auto &rhs_next = *(rhs.fake.next);
        auto &rhs_prev = *(rhs.fake.prev);
        rhs_next.prev = &fake;
        rhs_prev.next = &fake;

        std::swap(fake, rhs.fake);
    }
}
