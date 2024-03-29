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

#include <string>

#include "base/strings/string_piece.h"
#include "base/strings/string_piece_forward.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace adblock {

TEST(AdblockPatternMatcherTest, EmptyPatternMatchesInputs) {
  // Filters that have no pattern will match any URL. An example of such filter:
  // $third_party,domain=abc.com
  // (Nothing before the '$' character that denotes start of options)
  // This means "block *all* third party URLs while on abc.com".
  EXPECT_TRUE(DoesPatternMatchUrl("", GURL("https://ads.com")));
}

TEST(AdblockPatternMatcherTest, NonAnchoredPatternMatchesAnywhere) {
  // Filter patterns that have no anchoring characters will match anywhere URL.
  const auto pattern = base::StringPiece("ad-970");

  // Inside the host:
  EXPECT_TRUE(DoesPatternMatchUrl(pattern, GURL("https://x-ad-970-x.com")));
  // Start of host:
  EXPECT_TRUE(DoesPatternMatchUrl(pattern, GURL("https://ad-970-x.com/")));
  // End of host:
  EXPECT_TRUE(DoesPatternMatchUrl(pattern, GURL("https://x-ad-970.com/")));
  // Somewhere in path:
  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("https://ad.com/path/ad-970/x")));
  // The very end:
  EXPECT_TRUE(DoesPatternMatchUrl(pattern, GURL("https://ad.com/path/ad-970")));

  // Does not mach partial appearance.
  EXPECT_FALSE(DoesPatternMatchUrl(pattern, GURL("https://ad.com/ad/970")));
}

TEST(AdblockPatternMatcherTest,
     HostAnchoredPatternMatchesDomainsAndSubdomains) {
  // Filter pattern that starts from || will match if the prefix is a subdomain
  // or is empty.

  const auto pattern = base::StringPiece("||example.com/banner.gif");

  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com/banner.gif")));
  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("http://example.com/banner.gif")));
  EXPECT_TRUE(DoesPatternMatchUrl(
      pattern, GURL("https://subdomain.example.com/banner.gif")));
  EXPECT_TRUE(DoesPatternMatchUrl(
      pattern, GURL("https://deep.subdomain.example.com/banner.gif")));

  // Partial match is not a match:
  EXPECT_FALSE(DoesPatternMatchUrl(pattern, GURL("https://example.com/")));
  // nonexample.com is not a subdomain of example.com
  EXPECT_FALSE(
      DoesPatternMatchUrl(pattern, GURL("http://nonexample.com/banner.gif")));
}

TEST(AdblockPatternMatcherTest, HostAnchoredPatternWithImmediateWildcard) {
  // This is a weird kind of filter but it appears in the wild. It is equivalent
  // to one that has no host anchor, since the wildcard matches anything anyway.
  const auto pattern = base::StringPiece("||*/banner.gif");

  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com/banner.gif")));
  EXPECT_TRUE(DoesPatternMatchUrl(
      pattern, GURL("http://example.com/foobar/banner.gif")));
}

TEST(AdblockPatternMatcherTest, StartAnchoredPatternMatchesOnlyStartOfUrl) {
  const auto pattern = base::StringPiece("|https");

  // Pattern appears at the start of the URL, this is matched:
  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com/banner.gif")));

  // Partial match = not a match
  EXPECT_FALSE(
      DoesPatternMatchUrl(pattern, GURL("http://example.com/banner.gif")));
  // Pattern appears in the middle of the URL, not a match.
  EXPECT_FALSE(DoesPatternMatchUrl(
      pattern, GURL("http://example.com/https/banner.gif")));
}

TEST(AdblockPatternMatcherTest, EndAnchoredPatternMatchesOnlyEndOfUrl) {
  const auto pattern = base::StringPiece("/popup/log|");

  // Pattern appears at the end of the URL, this is matched:
  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com/popup/log")));

  // Partial match = not a match
  EXPECT_FALSE(DoesPatternMatchUrl(pattern, GURL("http://example.com/log")));
  // Pattern appears in the middle of the URL, not a match.
  EXPECT_FALSE(DoesPatternMatchUrl(
      pattern, GURL("http://example.com/popup/log/banner.gif")));
}

TEST(AdblockPatternMatcherTest, SeparatorCharacterBetweenHostAndPath) {
  // This is a *very* common filter pattern:
  const auto pattern = base::StringPiece("||example.com^");

  EXPECT_TRUE(DoesPatternMatchUrl(pattern, GURL("https://example.com/ad.gif")));
  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com:8000/ad.gif")));

  // '.' is not a character matched by ^. This treats example.com.ar as a
  // different domain
  EXPECT_FALSE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com.ar/ad.gif")));
}

TEST(AdblockPatternMatcherTest, SeparatorCharacterAtTheEndOfUrl) {
  const auto pattern = base::StringPiece("||example.com^");

  EXPECT_TRUE(DoesPatternMatchUrl(pattern, GURL("https://example.com")));
}

TEST(AdblockPatternMatcherTest,
     SeparatorCharacterContinuesMatchingAfterEndOfUrl) {
  // The ^ separator matches the end of URL, but in this filter we still expect
  // more tokens.
  const auto pattern = base::StringPiece("file^more");

  EXPECT_FALSE(DoesPatternMatchUrl(pattern, GURL("https://start.com/file")));
}

TEST(AdblockPatternMatcherTest, SeparatorCharacterInsideUrl) {
  const auto pattern = base::StringPiece("foo^bar");

  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("http://example.com/foo/bar?a=12")));
  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("http://example.com/foo?bar=12")));
}

TEST(AdblockPatternMatcherTest, MultipleSeparatorCharacters) {
  const auto pattern = base::StringPiece("^foo.bar^");

  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("http://example.com/foo.bar?a=12")));
}

TEST(AdblockPatternMatcherTest, SeparatorCharactersNotMatchedTooEagerly) {
  const auto pattern = base::StringPiece("^foo^");

  // The first "foo" is not a valid match for the filter, since it's surrounded
  // by dots which are not considered separators. But the filter should match
  // the second "foo".
  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("http://example.foo.com/foo?a=12")));
}

TEST(AdblockPatternMatcherTest, SeparatorCharacterDoesNotMatchWords) {
  const auto pattern = base::StringPiece("^foo.bar^");

  EXPECT_FALSE(DoesPatternMatchUrl(pattern, GURL("https://nonfoo.bar.com/")));
  EXPECT_FALSE(DoesPatternMatchUrl(pattern, GURL("https://foo.barbara.com/")));
}

TEST(AdblockPatternMatcherTest, SeparatorCharacterExceptions) {
  // The separator character can be anything but a letter, a digit, or one of
  // the following: _, -, ., %.
  const auto pattern = base::StringPiece("foo^bar");

  EXPECT_FALSE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com/foo-bar")));
  EXPECT_FALSE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com/foo_bar")));
  EXPECT_FALSE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com/foo.bar")));
  EXPECT_FALSE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com/foo%bar")));
  EXPECT_FALSE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com/fooXbar")));
  EXPECT_FALSE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com/foo5bar")));
}

TEST(AdblockPatternMatcherTest, SeparatorCharacterMatchesOnlySingleCharacter) {
  const auto pattern = base::StringPiece("http^value");

  // More than one character between "http" and "value"
  EXPECT_FALSE(DoesPatternMatchUrl(pattern, GURL("http://value.com/")));
  EXPECT_FALSE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com/http/?value")));
}

TEST(AdblockPatternMatcherTest, WildcardInPattern) {
  const auto pattern = base::StringPiece("&gerf=*&guro=");

  // Wildcard matches zero characters
  EXPECT_TRUE(DoesPatternMatchUrl(
      pattern, GURL("https://example.com/data?x&gerf=&guro=")));

  // Matches one digit
  EXPECT_TRUE(DoesPatternMatchUrl(
      pattern, GURL("https://example.com/data?x&gerf=1&guro=")));

  // Matches a long string with non-alphanumerical characters
  EXPECT_TRUE(DoesPatternMatchUrl(
      pattern, GURL("https://example.com/data?x&gerf=abcd&yyy=zzzz&guro=asd")));
}

TEST(AdblockPatternMatcherTest, HostWildcard) {
  const auto pattern = base::StringPiece("||ad.*.example.net^");

  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("https://ad.host.example.net/data")));
  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("https://ad.server.example.net/data")));
  EXPECT_TRUE(DoesPatternMatchUrl(
      pattern, GURL("https://subdomain.ad.server.example.net/data")));

  // Does not match because there need to be at least two dots between "ad" and
  // "example", and possibly something between the dots.
  EXPECT_FALSE(
      DoesPatternMatchUrl(pattern, GURL("https://ad.example.net/data")));
  // Does not match the host anchor, even though it matches the wildcard.
  EXPECT_FALSE(DoesPatternMatchUrl(
      pattern, GURL("https://domain.com/ad.x.example.net/")));
}

TEST(AdblockPatternMatcherTest, SeveralWildcardsInPattern) {
  const auto pattern = base::StringPiece("||example.com^*_*.php");

  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com/data_file.php")));
  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com/_file.php")));
  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com/data_.php")));
  EXPECT_TRUE(DoesPatternMatchUrl(pattern, GURL("https://example.com/_.php")));

  EXPECT_FALSE(
      DoesPatternMatchUrl(pattern, GURL("https://example.com/datafile.php")));
}

TEST(AdblockPatternMatcherTest, NonGreedyWildcardMatch) {
  const auto pattern = base::StringPiece("start*foobar^");

  // The first match of "foobar" isn't followed by a separator (but instead by
  // "bad"). But the algorithm doesn't stop searching and finds the next match
  // that is followed by a separator.
  EXPECT_TRUE(DoesPatternMatchUrl(
      pattern, GURL("https://start.com/foobarbad/foobar/file.png")));
}

TEST(AdblockPatternMatcherTest, MultipleConsecutiveWildcards) {
  const auto pattern = base::StringPiece("start***foobar");

  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("https://start.com/foobar/file.png")));
}

TEST(AdblockPatternMatcherTest, UrlEndsWithSeparator) {
  const auto pattern = base::StringPiece("file^|");

  // URL ends with "file" followed by a separator character, good match.
  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("https://start.com/foobar/file/")));

  // Ends without a separator, but ^ matches EOF too, good match.
  EXPECT_TRUE(DoesPatternMatchUrl(pattern, GURL("https://start.com/file")));

  // Has file/ but doesn't end with it.
  EXPECT_FALSE(
      DoesPatternMatchUrl(pattern, GURL("https://start.com/file/foobar")));
}

TEST(AdblockPatternMatcherTest, MatchAfterPartialMatch) {
  const auto pattern = base::StringPiece("barbar^");

  // This checks that we don't skip too far forward when the first match
  // position fails. The first position of "barbar" is not followed by a
  // separator, the second position is but it also overlaps with the first
  // match.
  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("https://start.com/barbarbar/")));
}

TEST(AdblockPatternMatcherTest, UrlEndsWithWildcardAndSeparator) {
  const auto pattern = base::StringPiece("file*^|");

  // URL ends with "file" followed by a separator character, good match.
  EXPECT_TRUE(
      DoesPatternMatchUrl(pattern, GURL("https://start.com/foobar/file/")));

  // Ends without a separator, but ^ matches EOF too, good match.
  EXPECT_TRUE(DoesPatternMatchUrl(pattern, GURL("https://start.com/file")));
}

TEST(AdblockPatternMatcherTest, FirstTokenIsWildcard) {
  // This is redundant and equivalent to not having the starting wildcard, but
  // some filters do this.
  const auto pattern = base::StringPiece("*file");

  EXPECT_TRUE(DoesPatternMatchUrl(pattern, GURL("https://start.com/file")));
}

TEST(AdblockPatternMatcherTest, DeepRecursion) {
  auto recursive_input_maker = [](int depth) -> std::pair<std::string, GURL> {
    std::string pattern;
    GURL url("https://example.com");
    for (int i = 0; i < depth; i++) {
      pattern += "a^";
      url = url.Resolve("a/");
    }
    return std::make_pair(pattern, url);
  };
  // For patterns that are simple enough, match normally.
  const auto [shallow_pattern, short_url] = recursive_input_maker(5);
  EXPECT_TRUE(DoesPatternMatchUrl(shallow_pattern, short_url));

  // Malicious, long filters are never matched.
  const auto [deep_pattern, long_url] = recursive_input_maker(100);
  EXPECT_FALSE(DoesPatternMatchUrl(deep_pattern, long_url));
}

}  // namespace adblock
