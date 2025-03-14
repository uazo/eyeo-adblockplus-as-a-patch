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

#include <sstream>
#include <string>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_piece_forward.h"
#include "base/timer/elapsed_timer.h"
#include "components/adblock/core/classifier/resource_classifier_impl.h"
#include "components/adblock/core/common/adblock_constants.h"
#include "components/adblock/core/converter/flatbuffer_converter.h"
#include "components/adblock/core/subscription/installed_subscription_impl.h"
#include "components/adblock/core/subscription/subscription_collection_impl.h"
#include "components/adblock/core/subscription/subscription_service.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/zlib/google/compression_utils.h"

namespace adblock {

class ResourceClassifierPerfTest : public testing::Test {
 public:
  void SetUp() override {
    classifier_ = base::MakeRefCounted<ResourceClassifierImpl>();
  }
  std::unique_ptr<SubscriptionCollectionImpl> CreateSubscriptionCollection(
      std::initializer_list<std::string> filenames) {
    std::vector<scoped_refptr<InstalledSubscription>> state;
    for (const auto& cur : filenames) {
      const std::string content = ReadFromTestData(cur);
      std::stringstream input(std::move(content));
      auto converter_result =
          FlatbufferConverter::Convert(input, CustomFiltersUrl(), false);
      DCHECK(absl::holds_alternative<std::unique_ptr<FlatbufferData>>(
          converter_result));
      state.push_back(base::MakeRefCounted<InstalledSubscriptionImpl>(
          std::move(
              absl::get<std::unique_ptr<FlatbufferData>>(converter_result)),
          Subscription::InstallationState::Installed, base::Time()));
    }
    return std::make_unique<SubscriptionCollectionImpl>(state);
  }

  static std::string ReadFromTestData(const std::string& file_name) {
    base::FilePath source_file;
    EXPECT_TRUE(base::PathService::Get(base::DIR_SOURCE_ROOT, &source_file));
    source_file = source_file.AppendASCII("components")
                      .AppendASCII("test")
                      .AppendASCII("data")
                      .AppendASCII("adblock")
                      .AppendASCII(file_name);
    std::string content;
    CHECK(base::ReadFileToString(source_file, &content));
    CHECK(compression::GzipUncompress(content, &content));
    return content;
  }

  static int BenchmarkRepetitions() {
    // Android devices are much slower than Desktop computers, reduce the
    // number of reps so they don't time out. On Desktop, a higher number of
    // reps allows more reliable measurement via perf tools.
#if BUILDFLAG(IS_ANDROID)
    return 5;
#else
    return 500;
#endif
  }

  void MeasureUrlMatchingTime(
      GURL url,
      ContentType content_type,
      std::unique_ptr<SubscriptionCollectionImpl> sub_collection,
      int cycles = BenchmarkRepetitions()) {
    SubscriptionService::Snapshot snapshot;
    snapshot.push_back(std::move(sub_collection));
    ResourceClassifier::ClassificationResult classification_result;
    base::ElapsedTimer timer;
    // Call matching many times to make sure perf woke up for measurement.
    for (int i = 0; i < cycles; ++i) {
      classification_result = classifier_->ClassifyRequest(
          std::move(snapshot), url, DefautFrameHeirarchy(), content_type,
          DefaultSitekey());
    }
    LOG(INFO) << "URL matching time: " << timer.Elapsed() / cycles;
    LOG(INFO) << "Classification result: "
              << ClassificationResultToString(classification_result);
  }

  void MeasureCSPMatchingTime(
      GURL url,
      ContentType content_type,
      std::unique_ptr<SubscriptionCollectionImpl> sub_collection,
      int cycles = BenchmarkRepetitions()) {
    std::set<base::StringPiece> csp_injections;
    base::ElapsedTimer timer;
    // Call matching many times to make sure perf woke up for measurement.
    for (int i = 0; i < cycles; ++i) {
      csp_injections =
          sub_collection->GetCspInjections(url, DefautFrameHeirarchy());
    }
    VLOG(1) << "CSP filter search time: " << timer.Elapsed() / cycles;
    for (const auto& csp_i : csp_injections) {
      VLOG(1) << "CSP injection found: " << csp_i;
    }
  }

  void MeasureElemhideGeneretionTime(
      GURL url,
      std::unique_ptr<SubscriptionCollectionImpl> collection) {
    // Call generation many times to make sure perf woke up for measurement.
    for (int i = 0; i < BenchmarkRepetitions(); ++i) {
      collection->GetElementHideSelectors(url, DefautFrameHeirarchy(),
                                          DefaultSitekey());
    }
  }

  const GURL& UnknownAddress() const {
    static const GURL kUnknownAddress{
        "https://eyeo.com/themes/custom/eyeo_theme/logo.svg"};
    return kUnknownAddress;
  }

  const GURL& BlockedAddress() const {
    static const GURL kBlockedAddress{"https://0265331.com/whatever/image.png"};
    return kBlockedAddress;
  }

  const std::vector<GURL>& DefautFrameHeirarchy() const {
    static const std::vector<GURL> kFrameHierarchy{
        GURL("https://frame.com/frame1.html"),
        GURL("https://frame.com/frame2.html"),
        GURL("https://frame.com/"),
    };
    return kFrameHierarchy;
  }

  const SiteKey& DefaultSitekey() const {
    static const SiteKey kSiteKey{"abc"};
    return kSiteKey;
  }

  base::StringPiece ClassificationResultToString(
      const ResourceClassifier::ClassificationResult& result) {
    switch (result.decision) {
      case ResourceClassifier::ClassificationResult::Decision::Allowed:
        return "Allowed";
      case ResourceClassifier::ClassificationResult::Decision::Blocked:
        return "Blocked";
      case ResourceClassifier::ClassificationResult::Decision::Ignored:
        return "Ignored";
    }
    return "";
  }

  scoped_refptr<ResourceClassifier> classifier_;
};

TEST_F(ResourceClassifierPerfTest, UrlNoMatch) {
  auto sub_collection = CreateSubscriptionCollection(
      {"easylist.txt.gz", "exceptionrules.txt.gz"});
  MeasureUrlMatchingTime(UnknownAddress(), ContentType::Image,
                         std::move(sub_collection));
}

TEST_F(ResourceClassifierPerfTest, UrlBlocked) {
  auto sub_collection = CreateSubscriptionCollection(
      {"easylist.txt.gz", "exceptionrules.txt.gz"});
  MeasureUrlMatchingTime(BlockedAddress(), ContentType::Image,
                         std::move(sub_collection));
}

TEST_F(ResourceClassifierPerfTest, ElemhideNoMatch) {
  auto sub_collection = CreateSubscriptionCollection(
      {"easylist.txt.gz", "exceptionrules.txt.gz"});
  MeasureElemhideGeneretionTime(UnknownAddress(), std::move(sub_collection));
}

TEST_F(ResourceClassifierPerfTest, ElemhideMatch) {
  auto sub_collection = CreateSubscriptionCollection(
      {"easylist.txt.gz", "exceptionrules.txt.gz"});
  MeasureElemhideGeneretionTime(
      GURL{"https://www.heise.de/news/"
           "Privacy-Shield-2-0-Viele-offene-Fragen-zum-Datenverkehr-mit-den-"
           "USA-6658370.html"},
      std::move(sub_collection));
}

TEST_F(ResourceClassifierPerfTest, LongUrlMatch) {
  auto sub_collection = CreateSubscriptionCollection(
      {"easylist.txt.gz", "exceptionrules.txt.gz"});
  // "longurl.txt.gz" contains a 300K-long URL that encodes a ton of debug data.
  // This URL was recorded from a real site. It can be orders of magnitude
  // slower to match than typical URLs so we use a custom repetition count.
#if BUILDFLAG(IS_ANDROID)
  const int kRepCount = 1;
#else
  const int kRepCount = 50;
#endif
  const GURL long_url(ReadFromTestData("longurl.txt.gz"));
  MeasureUrlMatchingTime(long_url, ContentType::Subdocument,
                         std::move(sub_collection), kRepCount);
}

TEST_F(ResourceClassifierPerfTest, LongUrlFindCsp) {
  auto sub_collection = CreateSubscriptionCollection(
      {"easylist.txt.gz", "exceptionrules.txt.gz"});
#if BUILDFLAG(IS_ANDROID)
  const int kRepCount = 1;
#else
  const int kRepCount = 5;
#endif
  const GURL long_url(ReadFromTestData("longurl.txt.gz"));
  MeasureCSPMatchingTime(long_url, ContentType::Subdocument,
                         std::move(sub_collection), kRepCount);
}

}  // namespace adblock
