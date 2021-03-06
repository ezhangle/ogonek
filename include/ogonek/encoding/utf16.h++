// Ogonek
//
// Written in 2012-2013 by Martinho Fernandes <martinho.fernandes@gmail.com>
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

#include <ogonek/encoding/iterator.h++>
#include <ogonek/types.h++>
#include <ogonek/error.h++>
#include <ogonek/error/unicode_error.h++>
#include <ogonek/detail/ranges.h++>
#include <ogonek/detail/constants.h++>
#include <ogonek/detail/container/partial_array.h++>

#include <taussig/primitives.h++>

#include <boost/range/sub_range.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/empty.hpp>

#include <array>
#include <utility>

namespace ogonek {
    struct utf16 {
    private:
        static constexpr code_point last_bmp_value = 0xFFFF;
        static constexpr code_point normalizing_value = 0x10000;
        static constexpr auto lead_surrogate_bitmask = 0xFFC00;
        static constexpr auto trail_surrogate_bitmask = 0x3FF;
        static constexpr auto lead_shifted_bits = 10;

    public:
        using code_unit = char16_t;
        static constexpr bool is_fixed_width = false;
        static constexpr std::size_t max_width = 2;
        static constexpr bool is_self_synchronizing = true;
        struct state {};

        template <typename ErrorHandler>
        static detail::encoded_character<utf16> encode_one(code_point u, state&, ErrorHandler) {
            if(u <= last_bmp_value) {
                return { static_cast<code_unit>(u) };
            } else {
                auto normal = u - normalizing_value;
                auto lead = detail::first_lead_surrogate + ((normal & lead_surrogate_bitmask) >> lead_shifted_bits);
                auto trail = detail::first_trail_surrogate + (normal & trail_surrogate_bitmask);
                return {
                    static_cast<code_unit>(lead),
                    static_cast<code_unit>(trail)
                };
            }
        }

        static code_point combine_surrogates(char16_t lead, char16_t trail) {
            auto hi = lead - detail::first_lead_surrogate;
            auto lo = trail - detail::first_trail_surrogate;
            return normalizing_value + ((hi << lead_shifted_bits) | lo);
        }

        template <typename Range>
        static boost::sub_range<Range> decode_one(Range const& r, code_point& out, state&, assume_valid_t) {
            auto first = boost::begin(r);
            auto lead = *first++;
            if(!detail::is_surrogate(lead)) {
                out = lead;
            } else {
                auto trail = *first++;
                out = combine_surrogates(lead, trail);
            }
            return { first, boost::end(r) };
        }
        template <typename Range, typename ErrorHandler>
        static boost::sub_range<Range> decode_one(Range const& r, code_point& out, state& s, ErrorHandler) {
            auto first = boost::begin(r);
            auto lead = *first++;

            if(!detail::is_surrogate(lead)) {
                out = lead;
                return { first, boost::end(r) };
            }
            if(!detail::is_lead_surrogate(lead)) {
                return ErrorHandler::template apply_decode<utf16>(r, s, out);
            }

            auto trail = *first++;
            if(!detail::is_trail_surrogate(trail)) {
                return ErrorHandler::template apply_decode<utf16>(r, s, out);
            }

            out = combine_surrogates(lead, trail);
            return { first, boost::end(r) };
        }
        template <typename Sequence>
        static std::pair<Sequence, code_point> decode_one_ex(Sequence s, state&, assume_valid_t) {
            auto lead = seq::front(s);
            seq::pop_front(s);

            if(!detail::is_surrogate(lead)) {
                return { s, lead };
            }
            auto trail = seq::front(s);
            seq::pop_front(s);

            return { s, combine_surrogates(lead, trail) };
        }
        template <typename Sequence, typename ErrorHandler>
        static std::pair<Sequence, code_point> decode_one_ex(Sequence s, state& state, ErrorHandler handler) {
            auto lead = seq::front(s);
            seq::pop_front(s);

            if(!detail::is_surrogate(lead)) {
                return { s, lead };
            }
            if(!detail::is_lead_surrogate(lead)) {
                decode_error<Sequence, utf16> error { s, state };
                wheels::optional<code_point> u;
                std::tie(s, state, u) = handler.handle(error);
                return { s, *u };
            }

            auto trail = seq::front(s);
            if(!detail::is_trail_surrogate(trail)) {
                decode_error<Sequence, utf16> error { s, state };
                wheels::optional<code_point> u;
                std::tie(s, state, u) = handler.handle(error);
                return { s, *u };
            }
            seq::pop_front(s);

            return { s, combine_surrogates(lead, trail) };
        }
    };
    
    namespace detail {
        template <typename ErrorHandler>
        null_terminated_utf16<ErrorHandler> as_code_point_range(char16_t const* sequence, ErrorHandler) {
            using source_iterator = null_terminated_range_iterator<char16_t const>;
            using result_iterator = decoding_iterator<utf16, source_iterator, ErrorHandler>;
            return {
                result_iterator(source_iterator(sequence), source_iterator()),
                result_iterator(source_iterator(), source_iterator())
            };
        }
    } // namespace detail
} // namespace ogonek

#endif // OGONEK_ENCODING_UTF16_HPP
