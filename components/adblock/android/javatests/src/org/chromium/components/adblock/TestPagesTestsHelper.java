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

import org.chromium.base.Log;
import org.chromium.base.test.util.CallbackHelper;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.test.ChromeTabbedActivityTestRule;
import org.chromium.content_public.browser.WebContents;
import org.chromium.content_public.browser.test.util.TestThreadUtils;

import java.lang.Thread;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

public class TestPagesTestsHelper {
    public static final String TESTPAGES_DOMAIN = "abptestpages.org";
    public static final String TESTPAGES_BASE_URL = "https://" + TESTPAGES_DOMAIN;
    public static final String TESTPAGES_TESTCASES_ROOT = TESTPAGES_BASE_URL + "/en/";
    public static final String FILTER_TESTPAGES_TESTCASES_ROOT =
            TESTPAGES_TESTCASES_ROOT + "filters/";
    public static final String EXCEPTION_TESTPAGES_TESTCASES_ROOT =
            TESTPAGES_TESTCASES_ROOT + "exceptions/";
    public static final String CIRCUMVENTION_TESTPAGES_TESTCASES_ROOT =
            TESTPAGES_TESTCASES_ROOT + "circumvention/";
    public static final String SITEKEY_TESTPAGES_TESTCASES_ROOT =
            EXCEPTION_TESTPAGES_TESTCASES_ROOT + "sitekey";
    public static final String SNIPPETS_TESTPAGES_TESTCASES_ROOT =
            TESTPAGES_TESTCASES_ROOT + "snippets/";
    public static final String TESTPAGES_RESOURCES_ROOT = TESTPAGES_BASE_URL + "/testfiles/";
    public static final String TESTPAGES_SUBSCRIPTION =
            TESTPAGES_TESTCASES_ROOT + "/abp-testcase-subscription.txt";

    private static final int TEST_TIMEOUT_SEC = 30;

    private URL mTestSubscriptionUrl;
    private CallbackHelper mHelper = new CallbackHelper();
    private TestAdBlockedObserver mObserver = new TestAdBlockedObserver();
    private TestSubscriptionUpdatedObserver mSubscriptionUpdateObserver =
            new TestSubscriptionUpdatedObserver();
    private ChromeTabbedActivityTestRule mActivityTestRule;

    private class TestSubscriptionUpdatedObserver
            implements AdblockController.SubscriptionUpdateObserver {
        @Override
        public void onSubscriptionDownloaded(final URL url) {
            if (mTestSubscriptionUrl == null) return;
            if (url.toString().contains(mTestSubscriptionUrl.toString())) {
                Log.d("TestSubscriptionUpdatedObserver",
                        "Notify subscription updated: " + url.toString());
                mHelper.notifyCalled();
            }
        }
    }

    public void setUp(final ChromeTabbedActivityTestRule activityRule) {
        mActivityTestRule = activityRule;
        mActivityTestRule.startMainActivityOnBlankPage();
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            AdblockController.getInstance().addOnAdBlockedObserver(mObserver);
            AdblockController.getInstance().addSubscriptionUpdateObserver(
                    mSubscriptionUpdateObserver);
        });
    }

    public void addFilterList(final String filterListUrl) {
        try {
            mTestSubscriptionUrl = new URL(filterListUrl);
            mObserver.setExpectedSubscriptionUrl(mTestSubscriptionUrl);
        } catch (MalformedURLException e) {
        }
        Assert.assertNotNull("Test subscription url", mTestSubscriptionUrl);
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            AdblockController.getInstance().installSubscription(mTestSubscriptionUrl);
        });
        try {
            mHelper.waitForCallback(0, 1, TEST_TIMEOUT_SEC, TimeUnit.SECONDS);
        } catch (TimeoutException e) {
            Assert.assertEquals(
                    "Test subscription was properly added", "Failed to add test subscription");
        }
    }

    public void addCustomFilter(final String filter) {
        TestThreadUtils.runOnUiThreadBlocking(
                () -> { AdblockController.getInstance().addCustomFilter(filter); });
    }

    public void tearDown() {
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            AdblockController.getInstance().removeSubscriptionUpdateObserver(
                    mSubscriptionUpdateObserver);
            AdblockController.getInstance().removeOnAdBlockedObserver(mObserver);
        });
    }

    public WebContents getWebContents() {
        return mActivityTestRule.getActivity().getCurrentWebContents();
    }

    public int getTabCount() {
        return mActivityTestRule.getActivity().getTabModelSelector().getTotalTabCount();
    }

    // Note: Use either setOnAdMatchedLatch XOR setOnAdMatchedExpectations
    public void setOnAdMatchedLatch(final CountDownLatch countDownLatch) {
        Assert.assertTrue(
                mObserver.countDownLatch == null || mObserver.countDownLatch.getCount() == 0);
        mObserver.countDownLatch = countDownLatch;
    }

    // Note: Use either setOnAdMatchedLatch XOR setOnAdMatchedExpectations
    public CountDownLatch setOnAdMatchedExpectations(
            final Set<String> onBlocked, final Set<String> onAllowed) {
        Assert.assertTrue(
                mObserver.countDownLatch == null || mObserver.countDownLatch.getCount() == 0);
        mObserver.countDownLatch = new CountDownLatch(1);
        mObserver.expectedBlocked = onBlocked;
        mObserver.expectedAllowed = onAllowed;
        return mObserver.countDownLatch;
    }

    public boolean isBlocked(final String url) {
        return mObserver.isBlocked(url);
    }

    public boolean isPopupBlocked(final String url) {
        return mObserver.isPopupBlocked(url);
    }

    public int numBlockedByType(final AdblockContentType type) {
        return mObserver.numBlockedByType(type);
    }

    public int numBlockedPopups() {
        return mObserver.numBlockedPopups();
    }

    public int numAllowedByType(final AdblockContentType type) {
        return mObserver.numAllowedByType(type);
    }

    public int numAllowedPopups() {
        return mObserver.numAllowedPopups();
    }

    public boolean isAllowed(final String url) {
        return mObserver.isAllowed(url);
    }

    public boolean isPageAllowed(final String url) {
        return mObserver.isPageAllowed(url);
    }

    public boolean isPopupAllowed(final String url) {
        return mObserver.isPopupAllowed(url);
    }

    public void loadUrl(final String url) throws InterruptedException, TimeoutException {
        mActivityTestRule.loadUrl(url, TEST_TIMEOUT_SEC);
    }

    public void loadUrlWaitForContent(final String url)
            throws InterruptedException, TimeoutException {
        loadUrl(url);
        TestVerificationUtils.verifyCondition(mActivityTestRule,
                "document.getElementsByClassName('testcase-waiting-content').length == 0");
    }

    public int numBlocked() {
        return mObserver.blockedInfos.size();
    }

    public int numAllowed() {
        return mObserver.allowedInfos.size();
    }
}
