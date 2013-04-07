// Ogonek
//
// Written in 2013 by Martinho Fernandes <martinho.fernandes@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related
// and neighboring rights to this software to the public domain worldwide. This software is
// distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication along with this software.
// If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.

// Smart sequences

#ifndef OGONEK_DETAIL_SEQUENCE_TRAITS_HPP
#define OGONEK_DETAIL_SEQUENCE_TRAITS_HPP

#include <wheels/meta.h++>

#include <type_traits>
#include <cstddef>

namespace ogonek {
    namespace detail {
        //! {concept}
        //! A source of values for a sequence.
        // concept SequenceSource<S> {
            //! {function}
            //! *Returns*: a [concept:Sequence] with elements from `s`.
            // nonmember Sequence<Seq> forward_as_sequence(SequenceSource<S> const& s);
        // };

        //! {concept}
        //! A sequence with a smart interface based on Andrei Alexandrescu's ideas.
        // concept Sequence<S> requires Copyable<S> {
            //! {trait}
            //! The type of values in this sequence.
            // nonmember typename sequence::Value<Sequence<S>>;

            //! {trait}
            //! The type of references to this sequence.
            //! *Note*: this type may not be a reference if the sequence is not persistent.
            // nonmember typename sequence::Reference<Sequence<S>>;

            //! {function}
            //! *Returns*: `true` if the sequence `s` has no elements.
            // nonmember bool sequence::empty(Sequence<S> const& s);

            //! {function}
            //! *Requires*: `!empty()`.
            //! *Returns*: the first element of the sequence `s`.
            // nonmember reference sequence::front(Sequence<S> const &);

            //! {function}
            //! *Requires*: `!empty()`.
            //! *Effects*: skips the first element in the sequence `s`.
            // nonmember void sequence::pop_front(Sequence<S>& s);

            //! {function}
            //! *Returns*: a sequence in the current state of the sequence `s`.
            // nonmemberSequence<S> sequence::save(Sequence<S> const& s);
        // };

        //! {tag}
        //! *Effects*: marks a derived type as a [concept:Sequence] with native operations.
        struct sequence_tag {};

        //! {trait}
        //! *Returns*: `true` if `S` is a [concept:Sequence] with native operations;
        //             `false` otherwise.
        template <typename S>
        struct is_simple_sequence : std::is_base_of<sequence_tag, wheels::Unqualified<S>> {};

        //! {traits}
        //! *Note*: implementation backend for [traits:sequence_ops].
        template <typename S,
                  bool = is_simple_sequence<S>()>
        struct sequence_ops_impl {};

        //! {specialization:1}
        template <typename S>
        struct sequence_ops_impl<S, true> {
            //! {trait}
            //! The type of values in the sequence `S`.
            using value_type = typename S::value_type;
            //! {trait}
            //! The type of references to the sequence `S`.
            //! *Note*: this type may not be a reference if the sequence `S` is not persistent.
            using reference = typename S::reference;

            //! {function}
            //! *Returns*: `true` if the sequence `s` has no elements.
            static bool empty(S const& s) { return s.empty(); }
            //! {function}
            //! *Requires*: `!empty(s)`.
            //! *Returns*: the first element of the sequence `s`.
            static reference front(S const& s) { return s.front(); }
            //! {function}
            //! *Requires*: `!empty(s)`.
            //! *Effects*: skips the first element in the sequence `s`.
            static void pop_front(S& s) { s.pop_front(); }
            //! {function}
            //! *Returns*: a sequence in the current state of `s`.
            static S save(S const& s) { return s.save(); }
        };

        //! {specialization}
        template <typename Iterator>
        struct sequence_ops_impl<std::pair<Iterator, Iterator>, false> {
            using value_type = typename std::iterator_traits<Iterator>::value_type;
            using reference = typename std::iterator_traits<Iterator>::reference;

            static bool empty(std::pair<Iterator, Iterator> const& its) { return its.first == its.second; }
            static reference front(std::pair<Iterator, Iterator> const& its) { return *its.first; }
            static void pop_front(std::pair<Iterator, Iterator>& its) { ++its.first; }
            static std::pair<Iterator, Iterator> save(std::pair<Iterator, Iterator> const& its) { return its; }
        };

        //! {specialization}
        template <typename Char>
        struct sequence_ops_impl<Char const*, false> {
            using value_type = Char;
            using reference = Char const&;

            static bool empty(Char const* p) { return *p == 0; }
            static reference front(Char const* p) { return *p; }
            static void pop_front(Char const*& p) { ++p; }
            static Char const* save(Char const* p) { return p; }
        };

        //! {traits}
        //! Provides a unified interface for using models of [concept:SequenceSource].
        //! *Requires*: `S` is a type returned from [function:forward_as_sequence] [soft].
        template <typename S>
        struct sequence_ops : sequence_ops_impl<wheels::Unqualified<S>> {};

        namespace sequence {
            //! {metafunction}
            //! *Requires*: `S` is a type returned from [function:forward_as_sequence] [soft].
            //! *Returns*: the type of values in the sequence`S`.
            template <typename S>
            using Value = typename sequence_ops<S>::value_type;

            //! {trait}
            //! *Requires*: `S` is a type returned from [function:forward_as_sequence] [soft].
            //! *Returns*: The type of references to the sequence `S`.
            //! *Note*: this type may not be a reference if the sequence `S` is not persistent.
            template <typename S>
            using Reference = typename sequence_ops<S>::reference;

            //! {function}
            //! *Requires*: `S` is a type returned from [function:forward_as_sequence] [soft].
            //! *Returns*: `true` if the sequence `s` has no elements.
            template <typename S>
            bool empty(S const& s) { return sequence_ops<S>::empty(s); }

            //! {function}
            //! *Requires*: `S` is a type returned from [function:forward_as_sequence] [soft]; and
            //!             `!empty(s)`.
            //! *Returns*: the first element of the sequence `s`.
            template <typename S>
            Reference<S> front(S const& s) { return sequence_ops<S>::front(s); }

            //! {function}
            //! *Requires*: `S` is a type returned from [function:forward_as_sequence] [soft]; and
            //!             `!empty(s)`.
            //! *Effects*: skips the first element in the sequence `s`.
            template <typename S>
            void pop_front(S& s) { return sequence_ops<S>::pop_front(s); }

            //! {function}
            //! *Requires*: `S` is a type returned from [function:forward_as_sequence] [soft].
            //! *Returns*: a sequence in the current state of `s`.
            template <typename S>
            S save(S const& s) { return sequence_ops<S>::save(s); }
        } // namespace sequence

        struct is_sequence_test {
            template <typename T>
            wheels::Bool<true, sequence::Value<T>> static test(int);
            template <typename...>
            std::false_type static test(...);
        };
        template <typename S>
        struct is_sequence : wheels::TraitOf<is_sequence_test, S> {};

        struct is_sequence_of_test {
            template <typename T, typename U>
            std::is_same<U, sequence::Value<T>> static test(int);
            template <typename...>
            std::false_type static test(...);
        };
        template <typename S, typename V>
        struct is_sequence_of : wheels::TraitOf<is_sequence_of_test, S, V> {};
    } // namespace detail
} // namespace ogonek

#endif // OGONEK_DETAIL_SEQUENCE_TRAITS_HPP
