// Copyright (c) 2016, Tom Honermann
//
// This file is distributed under the MIT License. See the accompanying file
// LICENSE.txt or http://www.opensource.org/licenses/mit-license.php for terms
// and conditions.

#ifndef TEXT_VIEW_ITEXT_ITERATOR_HPP // {
#define TEXT_VIEW_ITEXT_ITERATOR_HPP


#include <iterator>
#include <range/v3/range_traits.hpp>
#include <range/v3/utility/iterator_traits.hpp>
#include <text_view_detail/adl_customization.hpp>
#include <text_view_detail/concepts.hpp>
#include <text_view_detail/iterator_preserve.hpp>
#include <text_view_detail/subobject.hpp>


namespace std {
namespace experimental {
inline namespace text {


namespace text_detail {

template<typename ET, typename VT,
CONCEPT_REQUIRES_(
    TextEncoding<ET>(),
    ranges::View<VT>())>
class itext_cursor_base
    : private subobject<typename ET::state_type>
{
    using base_type = subobject<typename ET::state_type>;
    using encoding_type = ET;
    using view_type = VT;
    using iterator_type =
        ranges::range_iterator_t<
            typename std::add_const<view_type>::type>;
    using state_type = typename encoding_type::state_type;

public:
    itext_cursor_base() = default;

    itext_cursor_base(
        state_type state,
        const view_type *view)
    :
        base_type{std::move(state)},
        view(view)
    {}

    const state_type& state() const noexcept {
        return base_type::get();
    }
    state_type& state() noexcept {
        return base_type::get();
    }

    const view_type* underlying_view() const noexcept {
        return view;
    }

private:
    const view_type *view;
};


// Primary class template is incomplete.
template<typename ET, typename VT, typename Enable = void,
CONCEPT_REQUIRES_(
    TextEncoding<ET>(),
    ranges::View<VT>(),
    TextDecoder<
        ET,
        ranges::range_iterator_t<
            typename std::add_const<VT>::type>>())>
class itext_cursor_data;

// Specialization for input views.
template<typename ET, typename VT>
class itext_cursor_data<
          ET,
          VT,
          typename std::enable_if<
              ! (bool)TextForwardDecoder<
                          ET,
                          ranges::range_iterator_t<
                              typename std::add_const<VT>::type>>()>::type>
: public itext_cursor_base<ET, VT>
{
    using encoding_type = typename itext_cursor_data::encoding_type;
    using view_type = typename itext_cursor_data::view_type;
    using state_type = typename itext_cursor_data::state_type;
    using iterator_type = typename itext_cursor_data::iterator_type;

public:
    itext_cursor_data() = default;

    itext_cursor_data(
        state_type state,
        const view_type *view,
        iterator_type current)
    :
        itext_cursor_base<ET, VT>{std::move(state), view},
        current(std::move(current))
    {}

    const iterator_type& base() const noexcept {
        return current;
    }
    iterator_type& base() noexcept {
        return current;
    }

private:
    iterator_type current;
};

// Specialization for forward views.
template<typename ET, typename VT>
class itext_cursor_data<
          ET,
          VT,
          typename std::enable_if<
              (bool)TextForwardDecoder<
                  ET,
                  ranges::range_iterator_t<
                      typename std::add_const<VT>::type>>()>::type>
: public itext_cursor_base<ET, VT>
{
    using encoding_type = typename itext_cursor_data::encoding_type;
    using view_type = typename itext_cursor_data::view_type;
    using state_type = typename itext_cursor_data::state_type;
    using iterator_type = typename itext_cursor_data::iterator_type;

    struct current_view_type {
        current_view_type()
            : first{}, last{} {}
        current_view_type(iterator_type first, iterator_type last)
            : first{first}, last{last} {}

        iterator_type begin() const { return first; }
        iterator_type end() const { return last; }

        iterator_type first;
        iterator_type last;
    };

public:
    itext_cursor_data() = default;

    itext_cursor_data(
        state_type state,
        const view_type *view,
        iterator_type first)
    :
        itext_cursor_base<ET, VT>{std::move(state), view},
        current_view{first, first}
    {}

    const iterator_type& base() const noexcept {
        return current_view.first;
    }
    iterator_type& base() noexcept {
        return current_view.first;
    }

    const current_view_type& base_range() const noexcept {
        return current_view;
    }
    current_view_type& base_range() noexcept {
        return current_view;
    }

private:
    current_view_type current_view;
};


template<typename ET, typename VT,
CONCEPT_REQUIRES_(
    TextEncoding<ET>(),
    ranges::View<VT>())>
class itext_cursor
    : public itext_cursor_data<ET, VT>
{
    using encoding_type = typename itext_cursor::encoding_type;
    using view_type = typename itext_cursor::view_type;
    using state_type = typename itext_cursor::state_type;
    using iterator_type = typename itext_cursor::iterator_type;
    using value_type = character_type_t<encoding_type>;
    using reference = value_type;
    using pointer = const value_type*;
    using difference_type = ranges::iterator_difference_t<iterator_type>;

    class post_increment_proxy {
        friend class itext_cursor;
    public:
        post_increment_proxy(itext_cursor& self) noexcept
            : self(self), value(self.value)
        {}

        reference operator*() const noexcept {
            return value;
        }

    private:
        itext_cursor& self;
        value_type value;
    };

public:
    using single_pass =
        std::integral_constant<bool, !ranges::ForwardIterator<iterator_type>()>;

    class mixin
        : protected ranges::basic_mixin<itext_cursor>
    {
        using base_type = ranges::basic_mixin<itext_cursor>;
    public:
        using encoding_type = typename itext_cursor::encoding_type;
        using view_type = typename itext_cursor::view_type;
        using state_type = typename itext_cursor::state_type;

        mixin() = default;

        mixin(
            state_type state,
            const view_type *view,
            iterator_type first)
        :
            base_type{itext_cursor{std::move(state), view, std::move(first)}}
        {}

        mixin(
            const post_increment_proxy &p)
        :
            base_type{p.self}
        {
            this->value = p.value;
        }

        using base_type::base_type;

        const state_type& state() const noexcept {
            return this->get().state();
        }

        const iterator_type& base() const noexcept {
            return this->get().base();
        }

        template<typename This = const mixin*,
        CONCEPT_REQUIRES_(
            TextForwardDecoder<encoding_type, iterator_type>())>
        decltype(auto) base_range() const noexcept {
            return this->get().base_range();
        }

        // Iterators that are not ok include:
        // - Singular iterators.
        // - Past the end iterators.
        // - Iterators for which a decoding error occurred during increment or
        //   decrement operations.
        bool is_ok() const noexcept {
            return this->get().is_ok();
        }
    };

    itext_cursor() = default;

    itext_cursor(
        state_type state,
        const view_type *view,
        iterator_type first)
    :
        itext_cursor_data<ET, VT>{std::move(state), view, std::move(first)}
    {
        next();
    }

    bool is_ok() const noexcept {
        return ok;
    }

    reference read() const noexcept {
        return value;
    }

    pointer arrow() const noexcept {
        return &value;
    }

    CONCEPT_REQUIRES(
        ! TextForwardDecoder<encoding_type, iterator_type>())
    void next() {
        ok = false;
        auto preserved_base = make_iterator_preserve(this->base());
        auto end(text_detail::adl_end(*this->underlying_view()));
        while (preserved_base.get() != end) {
            value_type tmp_value;
            int decoded_code_units = 0;
            bool decoded_code_point = encoding_type::decode(
                this->state(),
                preserved_base.get(),
                end,
                tmp_value,
                decoded_code_units);
            preserved_base.update();
            if (decoded_code_point) {
                value = tmp_value;
                ok = true;
                break;
            }
        }
    }

    CONCEPT_REQUIRES(
        TextForwardDecoder<encoding_type, iterator_type>())
    void next() {
        ok = false;
        this->base_range().first = this->base_range().last;
        iterator_type tmp_iterator{this->base_range().first};
        auto end(text_detail::adl_end(*this->underlying_view()));
        while (tmp_iterator != end) {
            value_type tmp_value;
            int decoded_code_units = 0;
            bool decoded_code_point = encoding_type::decode(
                this->state(),
                tmp_iterator,
                end,
                tmp_value,
                decoded_code_units);
            this->base_range().last = tmp_iterator;
            if (decoded_code_point) {
                value = tmp_value;
                ok = true;
                break;
            }
            this->base_range().first = this->base_range().last;
        }
    }

    // For input iterators, a proxy is returned for post increment operations
    // that limits further operations to dereferences and explicit conversions
    // to a copy of the iterator.  This is done to prevent possibly unintended
    // usage of a copy of the stored input iterator that would otherwise
    // invalidate the stored iterator.
    CONCEPT_REQUIRES(
        ! TextForwardDecoder<encoding_type, iterator_type>())
    auto post_increment() {
        post_increment_proxy proxy{*this};
        next();
        return proxy;
    }

    CONCEPT_REQUIRES(
        TextBidirectionalDecoder<encoding_type, iterator_type>())
    void prev() {
        ok = false;
        this->base_range().last = this->base_range().first;
        std::reverse_iterator<iterator_type> rcurrent{this->base_range().last};
        std::reverse_iterator<iterator_type> rend{
            text_detail::adl_begin(*this->underlying_view())};
        while (rcurrent != rend) {
            value_type tmp_value;
            int decoded_code_units = 0;
            bool decoded_code_point = encoding_type::rdecode(
                this->state(),
                rcurrent,
                rend,
                tmp_value,
                decoded_code_units);
            this->base_range().first = rcurrent.base();
            if (decoded_code_point) {
                value = tmp_value;
                ok = true;
                break;
            }
            this->base_range().last = this->base_range().first;
        }
    }

    CONCEPT_REQUIRES(
        TextRandomAccessDecoder<encoding_type, iterator_type>())
    void advance(difference_type n) {
        if (n < 0) {
            this->base_range().first +=
                ((n+1) * encoding_type::max_code_units);
            prev();
        } else if (n > 0) {
            this->base_range().last +=
                ((n-1) * encoding_type::max_code_units);
            next();
        }
    }

    CONCEPT_REQUIRES(
        TextRandomAccessDecoder<encoding_type, iterator_type>())
    difference_type distance_to(const itext_cursor &other) const {
        return (other.base_range().first - this->base_range().first) /
               encoding_type::max_code_units;
    }

    CONCEPT_REQUIRES(
        ! ranges::ForwardIterator<iterator_type>())
    bool equal(const itext_cursor &other) const {
        // For input iterators, the base iterator corresponds to the next input
        // to be decoded.  Naively checking for base comparison only therefore
        // results in premature matches when the last code point in the input
        // is consumed.  For this reason, base equality is only considered a
        // match if the iterator is not ok.  It is assumed that the reason the
        // base iterator is not ok is that an attempt was made to decode a
        // code point after the last one in the input.  This heuristic can be
        // defeated when an iterator is not ok for other reasons.
        return ok == other.ok
            && (!ok || this->base() == other.base());
    }
    CONCEPT_REQUIRES(
        ranges::ForwardIterator<iterator_type>())
    bool equal(const itext_cursor &other) const {
        return this->base() == other.base();
    }

private:
    value_type value = {};
    bool ok = false;
};

} // namespace text_detail


/*
 * itext_iterator
 */
template<typename ET, typename VT,
CONCEPT_REQUIRES_(
    TextEncoding<ET>(),
    ranges::View<VT>(),
    TextDecoder<
        ET,
        ranges::range_iterator_t<
            typename std::add_const<VT>::type>>())>
using itext_iterator =
    ranges::basic_iterator<text_detail::itext_cursor<ET, VT>>;


} // inline namespace text
} // namespace experimental
} // namespace std


#endif // } TEXT_VIEW_ITEXT_ITERATOR_HPP
