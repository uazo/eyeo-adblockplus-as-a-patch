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

#include "components/adblock/core/subscription/pattern_matcher.h"

#include "absl/types/optional.h"
#include "base/logging.h"
#include "base/notreached.h"
#include "base/ranges/algorithm.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_piece_forward.h"
#include "base/strings/string_util.h"
#include "components/adblock/core/common/regex_filter_pattern.h"

namespace adblock {
namespace {

constexpr int kMaxRecursionDepth = 50;

bool CharacterIsValidSeparator(char c) {
  // The separator character can be anything but a letter, a digit, or one of
  // the following: _, -, ., %
  return !base::IsAsciiAlphaNumeric(c) &&
         base::StringPiece("_-.%").find(c) == base::StringPiece::npos;
}

// Returns if |candidate| (e.g. "https://sub") is a valid start of |url|'s host.
// If url is "https://sub.domain.com/path" then:
// Valid candidates:
// - https://
// - https://sub.
// - https://sub.domain.
// - https://sub.domain.com
// Invalid candidates:
// - https://s
// - https://sub
// - https://sub.domain.com/
// - https://sub.domain.com/p
bool IsValidStartOfHost(base::StringPiece candidate, const GURL& url) {
  const GURL trimmed_url = url.GetWithEmptyPath();
  if (url.has_scheme()) {
    DCHECK(base::StartsWith(candidate, url.scheme_piece()));
    candidate = candidate.substr(url.scheme_piece().size());
  }
  candidate =
      base::TrimString(candidate, ":/", base::TrimPositions::TRIM_LEADING);
  return candidate.empty() || candidate == url.host_piece() ||
         (base::EndsWith(candidate, ".") &&
          candidate.find_first_of("/") == base::StringPiece::npos);
}

class PatternTokenizer {
 public:
  explicit PatternTokenizer(base::StringPiece filter_pattern)
      : consumed_filter_pattern_(filter_pattern) {}

  base::StringPiece NextToken() {
    if (consumed_filter_pattern_.empty()) {
      return {};
    }
    // If the previous call left us on a wildcard character, return it and
    // and advance to first non-wildcard position.
    if (consumed_filter_pattern_[0] == '*') {
      consumed_filter_pattern_ =
          base::TrimString(consumed_filter_pattern_, "*", base::TRIM_LEADING);
      return "*";
    }
    // If the previous call left us on a ^ separator, return it and advance
    if (consumed_filter_pattern_[0] == '^') {
      consumed_filter_pattern_ = consumed_filter_pattern_.substr(1);
      return "^";
    }
    // If the previous call left us on a | anchor (or anchors), return it and
    // advance to first non-anchor position.
    if (consumed_filter_pattern_[0] == '|') {
      const auto token = consumed_filter_pattern_.substr(
          0, consumed_filter_pattern_.find_first_not_of("|"));
      consumed_filter_pattern_ = consumed_filter_pattern_.substr(token.size());
      return token;
    }

    // The next token is whatever characters are between current position and
    // the next separator (or EOF)
    const auto next_token = consumed_filter_pattern_.substr(
        0, consumed_filter_pattern_.find_first_of(kSeparators));
    // Advance to next token.
    consumed_filter_pattern_ =
        consumed_filter_pattern_.substr(next_token.size());
    return next_token;
  }

 private:
  constexpr static base::StringPiece kSeparators{"*^|"};
  // The tokenizer consumes |consumed_filter_pattern_| from the left as it
  // advances. This is cheap, just incrementing the begin index.
  base::StringPiece consumed_filter_pattern_;
};

absl::optional<base::StringPiece> FindNextTokenInInput(
    base::StringPiece consumed_input,
    PatternTokenizer tokenizer,
    int recursion_depth);

// Check if |consumed_input| starts with next token from |tokenizer| and
// continues matching subsequent tokens (recursively).
bool NextTokenBeginsInput(base::StringPiece consumed_input,
                          PatternTokenizer tokenizer,
                          int recursion_depth) {
  if (++recursion_depth > kMaxRecursionDepth) {
    return false;
  }
  const auto token = tokenizer.NextToken();
  if (token.empty()) {
    // Matching finished, no more tokens in the filter.
    return true;
  }
  if (token == "^") {
    // The next character must either be a valid separator, or EOF. "^" matches
    // either.
    if (!consumed_input.empty()) {
      // This is not an EOF, ^ must match a valid separator, followed by
      // subsequent matching tokens.
      return CharacterIsValidSeparator(consumed_input[0]) &&
             NextTokenBeginsInput(consumed_input.substr(1), tokenizer,
                                  recursion_depth);
    }
    // ^ is a valid match for EOF, but only if there aren't any tokens left
    // that want to match text.
    return NextTokenBeginsInput({}, tokenizer, recursion_depth);
  } else if (token == "*") {
    // The next characters can be anything, as long as subsequent tokens are
    // matched further in |consumed_input| (recursively).
    return FindNextTokenInInput(consumed_input, tokenizer, recursion_depth)
        .has_value();
  } else if (token == "|") {
    // "|" is an end-of-URL anchor, verify we indeed reached end of input.
    // TODO(mpawlowski) A literal "|"" character can occur in a URL, we should
    // probably check this as well: DPD-1755.
    return consumed_input.empty();
  } else {
    // The next characters should exactly match the token, and then subsequent
    // tokens must continue matching the input.
    if (!base::StartsWith(consumed_input, token)) {
      return false;
    }
    return NextTokenBeginsInput(consumed_input.substr(token.size()), tokenizer,
                                recursion_depth);
  }
}

// Returns characters skipped in order to reach next token from |tokenizer|, or
// nullopt if not found.
absl::optional<base::StringPiece> FindNextTokenInInput(
    base::StringPiece consumed_input,
    PatternTokenizer tokenizer,
    int recursion_depth) {
  if (++recursion_depth > kMaxRecursionDepth) {
    return absl::nullopt;
  }
  const auto token = tokenizer.NextToken();
  // We're searching for |token| anywhere inside |consumed_input|, we may skip
  // any number of characters while we try to find it.
  DCHECK(token != "*") << "PatternTokenizer failed to handle multiple "
                          "consecutive wildcards in the filter pattern";
  if (token == "^") {
    // We're looking for input that matches the ^ separator, followed by next
    // tokens (recursively).
    // It is possible that the first separator we find won't be followed by the
    // correct next token. This is ok, this algorithm cannot be greedy. Keep
    // skipping characters until we match a separator followed by subsequent
    // tokens.
    for (size_t i = 0; i < consumed_input.size(); i++) {
      if (!CharacterIsValidSeparator(consumed_input[i])) {
        continue;
      }
      if (NextTokenBeginsInput(consumed_input.substr(i + 1), tokenizer,
                               recursion_depth)) {
        return consumed_input.substr(0, i + 1);
      }
    }
    // Reached the end of the input without matching a valid separator (that was
    // followed by the right tokens, recursively).
    // It is OK as long as there are no further tokens that require matching
    // input. The "^" symbol matches EOF too.
    return NextTokenBeginsInput(base::StringPiece(), tokenizer, recursion_depth)
               ? absl::optional<base::StringPiece>{consumed_input}
               : absl::nullopt;
  } else {
    // The searched token is just ASCII text. Keep searching for occurrences of
    // it within consumed_input.
    for (auto match_pos = consumed_input.find(token);
         match_pos != base::StringPiece::npos;
         match_pos = consumed_input.find(token, match_pos + 1)) {
      if (NextTokenBeginsInput(consumed_input.substr(match_pos + token.size()),
                               tokenizer, recursion_depth)) {
        return consumed_input.substr(0, match_pos);
      }
      // If the first occurrence of token inside consumed_input isn't the right
      // one, keep looking. Subsequent tokens didn't match, but the algorithm is
      // not greedy, there might be another match.
    }

    return absl::nullopt;
  }
}

}  // namespace

bool DoesPatternMatchUrl(base::StringPiece filter_pattern, const GURL& url) {
  DCHECK(!ExtractRegexFilterFromPattern(filter_pattern))
      << "This function does not support regular expressions filters";
  const base::StringPiece input(url.spec());
  PatternTokenizer tokenizer(filter_pattern);
  const auto first_token = tokenizer.NextToken();
  if (first_token == "|") {
    return NextTokenBeginsInput(input, tokenizer, 0);
  } else if (first_token == "||") {
    {
      // If the next token is *, we discard the start-from-host anchor, behave
      // as if the filter started from *
      auto wildcard_tokenizer = tokenizer;
      const auto wildcard_token = wildcard_tokenizer.NextToken();
      if (wildcard_token == "*") {
        return FindNextTokenInInput(input, wildcard_tokenizer, 0).has_value();
      }
    }
    const auto skipped_characters = FindNextTokenInInput(input, tokenizer, 0);
    if (!skipped_characters) {
      return false;
    }
    return IsValidStartOfHost(*skipped_characters, url);

  } else if (first_token == "*") {
    return FindNextTokenInInput(input, tokenizer, 0).has_value();
  } else {
    // Behave as if the first token is a wildcard, recreate tokenizer to restart
    // from the first token.
    return FindNextTokenInInput(input, PatternTokenizer(filter_pattern), 0)
        .has_value();
  }
}

}  // namespace adblock
