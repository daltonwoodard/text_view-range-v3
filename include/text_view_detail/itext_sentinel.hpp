// Copyright (c) 2016, Tom Honermann
//
// This file is distributed under the MIT License. See the accompanying file
// LICENSE.txt or http://www.opensource.org/licenses/mit-license.php for terms
// and conditions.

#ifndef TEXT_VIEW_ITEXT_SENTINEL_HPP // {
#define TEXT_VIEW_ITEXT_SENTINEL_HPP


#include <iterator>
#include <range/v3/range_traits.hpp>
#include <text_view_detail/concepts.hpp>
#include <text_view_detail/itext_iterator.hpp>


namespace std {
namespace experimental {
inline namespace text {


template<typename ET, typename VT,
CONCEPT_REQUIRES_(
    TextEncoding<ET>(),
    ranges::View<VT>())>
class itext_sentinel {
public:
    using view_type = VT;
    using sentinel =
        ranges::range_sentinel_t<typename std::add_const<VT>::type>;

    itext_sentinel() = default;

    itext_sentinel(sentinel s)
        : s{s} {}

    friend bool operator==(
        const itext_sentinel &l,
        const itext_sentinel &r) noexcept
    {
        // Sentinels always compare equal regardless of any internal state.
        // See N4128, 10.1 "Sentinel Equality".
        return true;
    }
    friend bool operator!=(
        const itext_sentinel &l,
        const itext_sentinel &r) noexcept
    {
        return !(l == r);
    }

    friend bool operator==(
        const itext_iterator<ET, VT> &ti,
        const itext_sentinel &ts)
    {
        return ts.equal(ti);
    }
    friend bool operator!=(
        const itext_iterator<ET, VT> &ti,
        const itext_sentinel &ts)
    {
        return !(ti == ts);
    }
    friend bool operator==(
        const itext_sentinel &ts,
        const itext_iterator<ET, VT> &ti)
    {
        return ti == ts;
    }
    friend bool operator!=(
        const itext_sentinel &ts,
        const itext_iterator<ET, VT> &ti)
    {
        return !(ts == ti);
    }

    const sentinel& base() const noexcept {
        return s;
    }

private:
    CONCEPT_REQUIRES(
        ! ranges::ForwardIterator<
              decltype(std::declval<const itext_iterator<ET, VT>&>().base())>())
    bool equal(const itext_iterator<ET, VT> &ti) const {
        // For input iterators, the base iterator corresponds to the next input
        // to be decoded.  Naively checking for base comparison only therefore
        // results in premature matches when the last code point in the input
        // is consumed.  For this reason, base equality is only considered a
        // sentinel match if the iterator is not ok.  It is assumed that the
        // reason the base iterator is not ok is that an attempt was made to
        // decode a code point after the last one in the input.  This heuristic
        // can be defeated when an iterator is not ok for other reasons.
        return ti.base() == base()
            && ! ti.is_ok();
    }
    CONCEPT_REQUIRES(
        ranges::ForwardIterator<
              decltype(std::declval<const itext_iterator<ET, VT>&>().base())>())
    bool equal(const itext_iterator<ET, VT> &ti) const {
        return ti.base() == base();
    }

private:
    sentinel s;
};


} // inline namespace text
} // namespace experimental
} // namespace std


#endif // } TEXT_VIEW_ITEXT_SENTINEL_HPP
