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

// Little endian order

#ifndef OGONEK_ENCODING_LITTLE_ENDIAN_HPP
#define OGONEK_ENCODING_LITTLE_ENDIAN_HPP

#include <ogonek/types.h++>

#include <taussig/primitives.h++>

#include <cstdint>
#include <array>
#include <type_traits>

namespace ogonek {
    struct little_endian {
    public:
        static std::array<byte, 2> map(std::uint16_t u16) {
            return std::array<byte, 2> {{
                byte((u16 & 0x00FF) >> 0),
                byte((u16 & 0xFF00) >> 8),
            }};
        }
        static std::array<byte, 4> map(std::uint32_t u32) {
            return std::array<byte, 4> {{
                byte((u32 & 0x000000FF) >> 0),
                byte((u32 & 0x0000FF00) >> 8),
                byte((u32 & 0x00FF0000) >> 16),
                byte((u32 & 0xFF000000) >> 24),
            }};
        }
        template <typename DoNotConvert>
        static void map(DoNotConvert) = delete;

    private:
        template <int N>
        struct reader;

    public:
        template <typename Sequence, typename Integer>
        static Sequence unmap(Sequence s, Integer& out) {
            static_assert(std::is_integral<Integer>::value, "Unmapped type must be integral");
            static_assert(sizeof(Integer) == 2 || sizeof(Integer) == 4, "Unmapped type must be have 2 or 4 bytes");
            out = reader<sizeof(Integer)>::read(s);
            return s;
        }
    };

    template <>
    struct little_endian::reader<2> {
        using type = std::uint16_t;
        template <typename Sequence>
        static type read(Sequence& s) {
            type b0 = seq::front(s);
            seq::pop_front(s);
            type b1 = seq::front(s);
            seq::pop_front(s);
            return (b1 << 8) | b0;
        }
    };
    template <>
    struct little_endian::reader<4> {
        using type = std::uint32_t;
        template <typename Sequence>
        static type read(Sequence& s) {
            type b0 = seq::front(s);
            seq::pop_front(s);
            type b1 = seq::front(s);
            seq::pop_front(s);
            type b2 = seq::front(s);
            seq::pop_front(s);
            type b3 = seq::front(s);
            seq::pop_front(s);
            return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
        }
    };
}

#endif // OGONEK_ENCODING_LITTLE_ENDIAN_HPP

