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

#ifndef COMPONENTS_ADBLOCK_CORE_SUBSCRIPTION_PATTERN_MATCHER_H_
#define COMPONENTS_ADBLOCK_CORE_SUBSCRIPTION_PATTERN_MATCHER_H_

#include "base/strings/string_piece_forward.h"
#include "url/gurl.h"

namespace adblock {

// Returns whether the URL is matched by a filter pattern.
// Example: filter_pattern "||example.com^" will match url
// "https://subdomain/example.com/path.png"
// filter_pattern must NOT be a regex filter
bool DoesPatternMatchUrl(base::StringPiece filter_pattern, const GURL& url);

}  // namespace adblock

#endif  // COMPONENTS_ADBLOCK_CORE_SUBSCRIPTION_PATTERN_MATCHER_H_