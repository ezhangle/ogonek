// Ogonek
//
// Written in 2012 by Martinho Fernandes <martinho.fernandes@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related
// and neighboring rights to this software to the public domain worldwide. This software is
// distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication along with this software.
// If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.

// UTF-16 encoding form

#ifndef OGONEK_ENCODING_UTF16_HPP
#define OGONEK_ENCODING_UTF16_HPP

#include "iterator.h++"
#include "../types.h++"
#include "../validation.h++"
#include "../detail/partial_array.h++"

#include <boost/range/sub_range.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/empty.hpp>

#include <array>
#include <utility>

namespace ogonek {
    struct utf16 {
        using code_unit = char16_t;
        static constexpr bool is_fixed_width = false;
        static constexpr std::size_t max_width = 2;
        static constexpr bool is_self_synchronizing = true;
        struct state {};

        template <typename SinglePassRange, typename ValidationPolicy,
                  typename Iterator = typename boost::range_const_iterator<SinglePassRange>::type,
                  typename EncodingIterator = encoding_iterator<utf16, Iterator, ValidationPolicy>>
        static boost::iterator_range<EncodingIterator> encode(SinglePassRange const& r, ValidationPolicy) {
            return boost::make_iterator_range(
                    EncodingIterator { boost::begin(r), boost::end(r) },
                    EncodingIterator { boost::end(r), boost::end(r) });
        }

        template <typename SinglePassRange, typename ValidationPolicy,
                  typename Iterator = typename boost::range_const_iterator<SinglePassRange>::type,
                  typename DecodingIterator = decoding_iterator<utf16, Iterator, ValidationPolicy>>
        static boost::iterator_range<DecodingIterator> decode(SinglePassRange const& r, ValidationPolicy) {
            return boost::make_iterator_range(
                    DecodingIterator { boost::begin(r), boost::end(r) },
                    DecodingIterator { boost::end(r), boost::end(r) });
        }

        template <typename ValidationPolicy>
        static detail::coded_character<utf16> encode_one(codepoint u, state&, ValidationPolicy) {
            if(u <= 0xFFFF) {
                return { static_cast<code_unit>(u) };
            } else {
                auto normal = u - 0x10000;
                auto lead = 0xD800 + ((normal & 0xFFC00) >> 10);
                auto trail = 0xDC00 + (normal & 0x3FF);
                return {
                    static_cast<code_unit>(lead),
                    static_cast<code_unit>(trail)
                };
            }
        }

        static bool is_lead_surrogate(codepoint u) { return u >= 0xD800 && u <= 0xDBFF; };
        static bool is_trail_surrogate(codepoint u) { return u >= 0xDC00 && u <= 0xDFFF; };
        static bool is_surrogate(codepoint u) { return u >= 0xD800 && u <= 0xDFFF; };

        template <typename SinglePassRange>
        static boost::sub_range<SinglePassRange> decode_one(SinglePassRange const& r, codepoint& out, state&, decltype(skip_validation)) {
            auto first = boost::begin(r);
            auto lead = *first++;
            if(!is_surrogate(lead)) {
                out = lead;
            } else {
                auto trail = *first++;
                auto hi = lead - 0xD800;
                auto lo = trail - 0xDC00;
                out = 0x10000 + ((hi << 10) | lo);
            }
            return { first, boost::end(r) };
        }
        template <typename SinglePassRange, typename ValidationPolicy>
        static boost::sub_range<SinglePassRange> decode_one(SinglePassRange const& r, codepoint& out, state& s, ValidationPolicy) {
            auto first = boost::begin(r);
            auto lead = *first++;

            if(!is_surrogate(lead)) {
                out = lead;
                return { first, boost::end(r) };
            }
            if(!is_lead_surrogate(lead)) {
                return ValidationPolicy::template apply_decode<utf16>(r, s, out);
            }

            auto trail = *first++;
            if(!is_trail_surrogate(trail)) {
                return ValidationPolicy::template apply_decode<utf16>(r, s, out);
            }

            auto hi = lead - 0xD800;
            auto lo = trail - 0xDC00;
            out = 0x10000 + ((hi << 10) | lo);
            return { first, boost::end(r) };
        }
    };
} // namespace ogonek

#endif // OGONEK_ENCODING_UTF16_HPP


