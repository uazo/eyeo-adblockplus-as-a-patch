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

#ifndef COMPONENTS_ADBLOCK_CORE_CONVERTER_PARSER_DOMAIN_OPTION_H_
#define COMPONENTS_ADBLOCK_CORE_CONVERTER_PARSER_DOMAIN_OPTION_H_

#include <string>
#include <vector>

#include "base/strings/string_piece.h"

namespace adblock {

class DomainOption {
 public:
  DomainOption();

  static DomainOption FromString(base::StringPiece domains_list,
                                 base::StringPiece separator);

  DomainOption(const DomainOption& other);
  DomainOption(DomainOption&& other);
  DomainOption& operator=(const DomainOption& other);
  DomainOption& operator=(DomainOption&& other);
  ~DomainOption();

  const std::vector<std::string>& GetExcludeDomains() const;
  const std::vector<std::string>& GetIncludeDomains() const;

  void RemoveDomainsWithNoSubdomain();
  bool UnrestrictedByDomain() const;

 private:
  DomainOption(std::vector<std::string> exclude_domains,
               std::vector<std::string> include_domains);

  static bool HasSubdomainOrLocalhost(base::StringPiece domain);

  std::vector<std::string> exclude_domains_;
  std::vector<std::string> include_domains_;
};

}  // namespace adblock

#endif  // COMPONENTS_ADBLOCK_CORE_CONVERTER_PARSER_DOMAIN_OPTION_H_
