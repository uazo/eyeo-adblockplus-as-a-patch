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

package org.chromium.components.adblock;

import org.junit.Assert;
import org.junit.Rule;

import java.net.URL;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeoutException;

public class TestAdBlockedObserver implements ResourceClassificationNotifier.AdBlockedObserver {
    @Override
    public void onAdAllowed(AdblockCounters.ResourceInfo info) {
        allowedInfos.add(info);
        Assert.assertTrue(info.getSubscription().equals(getExpectedSubscriptionUrl()));
        CheckAndCountDownLatch(Decision.ALLOWED, info.getRequestUrl().split("\\?")[0]);
    }

    @Override
    public void onAdBlocked(AdblockCounters.ResourceInfo info) {
        blockedInfos.add(info);
        Assert.assertTrue(info.getSubscription().equals(getExpectedSubscriptionUrl()));
        CheckAndCountDownLatch(Decision.BLOCKED, info.getRequestUrl().split("\\?")[0]);
    }

    @Override
    public void onPageAllowed(AdblockCounters.ResourceInfo info) {
        allowedPageInfos.add(info);
        Assert.assertTrue(info.getSubscription().equals(getExpectedSubscriptionUrl()));
    }

    @Override
    public void onPopupAllowed(AdblockCounters.ResourceInfo info) {
        allowedPopupsInfos.add(info);
        Assert.assertTrue(info.getSubscription().equals(getExpectedSubscriptionUrl()));
    }

    @Override
    public void onPopupBlocked(AdblockCounters.ResourceInfo info) {
        blockedPopupsInfos.add(info);
        Assert.assertTrue(info.getSubscription().equals(getExpectedSubscriptionUrl()));
    }

    public boolean isBlocked(String url) {
        for (AdblockCounters.ResourceInfo info : blockedInfos) {
            if (info.getRequestUrl().contains(url)) return true;
        }

        return false;
    }

    public boolean isPopupBlocked(String url) {
        for (AdblockCounters.ResourceInfo info : blockedPopupsInfos) {
            if (info.getRequestUrl().contains(url)) return true;
        }

        return false;
    }

    public int numBlockedByType(AdblockContentType type) {
        int result = 0;
        for (AdblockCounters.ResourceInfo info : blockedInfos) {
            if (info.getAdblockContentType() == type) ++result;
        }
        return result;
    }

    public int numBlockedPopups() {
        int result = 0;
        for (AdblockCounters.ResourceInfo info : blockedPopupsInfos) {
            ++result;
        }
        return result;
    }

    public boolean isAllowed(String url) {
        for (AdblockCounters.ResourceInfo info : allowedInfos) {
            if (info.getRequestUrl().contains(url)) return true;
        }

        return false;
    }

    public boolean isPageAllowed(String url) {
        for (AdblockCounters.ResourceInfo info : allowedPageInfos) {
            if (info.getRequestUrl().contains(url)) return true;
        }

        return false;
    }

    public boolean isPopupAllowed(String url) {
        for (AdblockCounters.ResourceInfo info : allowedPopupsInfos) {
            if (info.getRequestUrl().contains(url)) return true;
        }

        return false;
    }

    public int numAllowedByType(AdblockContentType type) {
        int result = 0;
        for (AdblockCounters.ResourceInfo info : allowedInfos) {
            if (info.getAdblockContentType() == type) ++result;
        }
        return result;
    }

    public int numAllowedPopups() {
        int result = 0;
        for (AdblockCounters.ResourceInfo info : allowedPopupsInfos) {
            ++result;
        }
        return result;
    }

    public void setExpectedSubscriptionUrl(URL url) {
        mTestSubscriptionUrl = url;
    }

    private String getExpectedSubscriptionUrl() {
        if (mTestSubscriptionUrl != null) return mTestSubscriptionUrl.toString();
        return "adblock:custom";
    }

    private enum Decision { ALLOWED, BLOCKED }

    // We either countDown() our latch for every filtered resource if there are no
    // specific expectations set (expectedAllowed == null && expectedBlocked == null),
    // or we countDown() only when all expectations have been met so when:
    // (expectedAllowed.isNullOrEmpty() && expectedBlocked.isNullOrEmpty()).
    private void CheckAndCountDownLatch(final Decision decision, final String url) {
        if (countDownLatch != null) {
            if (expectedBlocked == null && expectedAllowed == null) {
                countDownLatch.countDown();
            } else {
                if (decision == Decision.BLOCKED) {
                    if (expectedBlocked != null) {
                        expectedBlocked.remove(url);
                    }
                } else {
                    if (expectedAllowed != null) {
                        expectedAllowed.remove(url);
                    }
                }
                boolean expectationsMet = (expectedAllowed == null || expectedAllowed.isEmpty())
                        && (expectedBlocked == null || expectedBlocked.isEmpty());
                if (expectationsMet) {
                    countDownLatch.countDown();
                }
            }
        }
    }

    private URL mTestSubscriptionUrl;
    List<AdblockCounters.ResourceInfo> blockedInfos = new CopyOnWriteArrayList<>();
    List<AdblockCounters.ResourceInfo> allowedInfos = new CopyOnWriteArrayList<>();
    List<AdblockCounters.ResourceInfo> allowedPageInfos = new CopyOnWriteArrayList<>();
    List<AdblockCounters.ResourceInfo> blockedPopupsInfos = new CopyOnWriteArrayList<>();
    List<AdblockCounters.ResourceInfo> allowedPopupsInfos = new CopyOnWriteArrayList<>();
    CountDownLatch countDownLatch;
    Set<String> expectedAllowed;
    Set<String> expectedBlocked;
};
