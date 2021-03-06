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

// Error handling policy for data asserted valid

#ifndef OGONEK_ERROR_ASSUME_VALID_HPP
#define OGONEK_ERROR_ASSUME_VALID_HPP

#include <ogonek/error/error_handler.h++>

namespace ogonek {
    //! {tag}
    //! Marker type for skipping validation.
    struct assume_valid_t : error_handler {};

    //! {object}
    //! A default instance of [function:assume_valid_t].
    constexpr assume_valid_t assume_valid = {};
} // namespace ogonek

#endif // OGONEK_ERROR_ASSUME_VALID_HPP
