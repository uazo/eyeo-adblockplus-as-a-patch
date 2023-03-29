/*
 * This file is part of eyeo Chromium SDK,
 * Copyright (C) 2006-present eyeo GmbH
 *
 * eyeo Chromium SDK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * eyeo Chromium SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with eyeo Chromium SDK.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMPONENTS_ADBLOCK_CORE_COMMON_REGEX_FILTER_PATTERN_H_
#define COMPONENTS_ADBLOCK_CORE_COMMON_REGEX_FILTER_PATTERN_H_

#include "absl/types/optional.h"
#include "base/strings/string_piece_forward.h"

namespace adblock {

// For a regex filter "/{expression}/" returns "{expression}".
// For non-regex filters, returns nullopt.
// Cheap, may be used to identify regex filter patterns.
absl::optional<base::StringPiece> ExtractRegexFilterFromPattern(
    base::StringPiece filter_pattern);
}  // namespace adblock

#endif  // COMPONENTS_ADBLOCK_CORE_COMMON_REGEX_FILTER_PATTERN_H_