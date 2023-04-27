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

import androidx.test.filters.LargeTest;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.base.test.util.CommandLineFlags;
import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.chromium.chrome.browser.flags.ChromeSwitches;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.test.ChromeBrowserTestRule;
import org.chromium.chrome.test.ChromeJUnit4ClassRunner;
import org.chromium.chrome.test.ChromeTabbedActivityTestRule;
import org.chromium.components.adblock.TestVerificationUtils.IncludeSubframes;
import org.chromium.content_public.browser.test.util.DOMUtils;
import org.chromium.content_public.browser.test.util.JavaScriptUtils;

import java.lang.Thread;
import java.util.Arrays;
import java.util.HashSet;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

@RunWith(ChromeJUnit4ClassRunner.class)
@CommandLineFlags.Add({ChromeSwitches.DISABLE_FIRST_RUN_EXPERIENCE})
public class TestPagesFilterTest {
    @Rule
    public final ChromeBrowserTestRule mBrowserTestRule = new ChromeBrowserTestRule();
    @Rule
    public ChromeTabbedActivityTestRule mActivityTestRule = new ChromeTabbedActivityTestRule();
    private TestPagesTestsHelper mHelper = new TestPagesTestsHelper();

    @Before
    public void setUp() {
        mHelper.setUp(mActivityTestRule);
        mHelper.addFilterList(TestPagesTestsHelper.TESTPAGES_SUBSCRIPTION);
    }

    @After
    public void tearDown() {
        mHelper.tearDown();
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testBlockingFilters() throws TimeoutException, InterruptedException {
        final CountDownLatch countDownLatch = mHelper.setOnAdMatchedExpectations(
                new HashSet<>(Arrays.asList(
                        TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "blocking/full-path.png",
                        TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT
                                + "blocking/partial-path/partial-path.png",
                        TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT
                                + "blocking/wildcard/1/wildcard.png",
                        TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT
                                + "blocking/wildcard/2/wildcard.png",
                        TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "blocking/dynamic.png",
                        TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "blocking/subdomain.png")),
                null);
        mHelper.loadUrl(TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "blocking");
        // Wait with 10 seconds max timeout
        countDownLatch.await(10, TimeUnit.SECONDS);
        Assert.assertEquals(6, mHelper.numBlocked());
        Assert.assertEquals(6, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_IMAGE));
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 6, "img[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyScriptFilters() throws TimeoutException, InterruptedException {
        mHelper.loadUrl(TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "script");
        Assert.assertEquals(1, mHelper.numBlocked());
        Assert.assertEquals(1, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_SCRIPT));
        Assert.assertTrue(mHelper.isBlocked(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "script/script.js"));

        String childCount =
                JavaScriptUtils.executeJavaScriptAndWaitForResult(mHelper.getWebContents(),
                        "document.getElementById(\"script-target\").childElementCount");
        Assert.assertEquals("1", childCount);
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyImageFilters() throws TimeoutException, InterruptedException {
        final CountDownLatch countDownLatch = mHelper.setOnAdMatchedExpectations(
                new HashSet<>(Arrays.asList(
                        TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "image/static/static.png",
                        TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT
                                + "image/dynamic/dynamic.png")),
                null);
        mHelper.loadUrl(TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "image");
        countDownLatch.await(10, TimeUnit.SECONDS);
        Assert.assertEquals(2, mHelper.numBlocked());
        Assert.assertEquals(2, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_IMAGE));
        Assert.assertTrue(mHelper.isBlocked(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "image/static/static.png"));
        Assert.assertTrue(mHelper.isBlocked(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "image/dynamic/dynamic.png"));
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 2, "img[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyStylesheetFilters() throws TimeoutException, InterruptedException {
        final String blockedUrl =
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "stylesheet/stylesheet.cs";
        final CountDownLatch countDownLatch =
                mHelper.setOnAdMatchedExpectations(new HashSet<>(Arrays.asList(blockedUrl)), null);
        mHelper.loadUrl(TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "stylesheet");
        // Wait with 10 seconds max timeout
        countDownLatch.await(10, TimeUnit.SECONDS);
        Assert.assertEquals(1, mHelper.numBlocked());
        Assert.assertEquals(
                1, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_STYLESHEET));
        Assert.assertTrue(mHelper.isBlocked(blockedUrl));
        String value = DOMUtils.getNodeContents(mHelper.getWebContents(), "stylesheet-target");
        Assert.assertEquals("Passed. Stylesheet was blocked.", value);
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyPopupFilters() throws TimeoutException, InterruptedException {
        final String POPUP_TESTACE_URL =
                TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "popup";
        mHelper.loadUrl(POPUP_TESTACE_URL);
        Assert.assertEquals(1, mHelper.getTabCount());
        String numElements =
                JavaScriptUtils.executeJavaScriptAndWaitForResult(mHelper.getWebContents(),
                        "var elements = document.getElementsByClassName(\"testcase-trigger\");"
                                + "for (let i = 0; i < elements.length; ++i) {"
                                + "    elements[i].click();"
                                + "}"
                                + "elements.length;");
        Assert.assertEquals("3", numElements);
        Assert.assertEquals(3, mHelper.numBlockedPopups());
        Assert.assertTrue(mHelper.isPopupBlocked(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "popup/link.html"));
        Assert.assertTrue(mHelper.isPopupBlocked(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "popup/script-window.html"));
        Assert.assertTrue(mHelper.isPopupBlocked(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "popup/script-tab.html"));
        Assert.assertEquals(1, mHelper.getTabCount());
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyXHRFilters() throws TimeoutException, InterruptedException {
        final String blockedUrl =
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "xmlhttprequest/text.txt";
        final CountDownLatch countDownLatch =
                mHelper.setOnAdMatchedExpectations(new HashSet<>(Arrays.asList(blockedUrl)), null);
        mHelper.loadUrl(TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "xmlhttprequest");
        // Wait with 10 seconds max timeout
        countDownLatch.await(10, TimeUnit.SECONDS);
        Assert.assertEquals(1, mHelper.numBlocked());
        Assert.assertEquals(
                1, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_XMLHTTPREQUEST));
        Assert.assertTrue(mHelper.isBlocked(blockedUrl));
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifySubdocumentFilters() throws TimeoutException, InterruptedException {
        mHelper.loadUrl(TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "subdocument");
        Assert.assertEquals(1, mHelper.numBlocked());
        Assert.assertEquals(
                1, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_SUBDOCUMENT));
        Assert.assertTrue(mHelper.isBlocked(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "subdocument/subdocument.html"));
        // Do not search for iframe within the site's iframes.
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "iframe[data-expectedresult='fail']", IncludeSubframes.NO);
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    @DisabledTest(message = "Please enable again when rewrite filters will be supported")
    public void testVerifyRewrite() throws TimeoutException, InterruptedException {
        mHelper.loadUrl(TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "rewrite");
        Assert.assertEquals(3, mHelper.numBlocked());
        Assert.assertEquals(1, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_SCRIPT));
        Assert.assertEquals(2, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_MEDIA));
        Assert.assertTrue(mHelper.isBlocked(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "rewrite/audio.mp3"));
        Assert.assertTrue(mHelper.isBlocked(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "rewrite/video.mp4"));
        Assert.assertTrue(mHelper.isBlocked(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "rewrite/script.js"));
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyMatchCaseFilter() throws TimeoutException, InterruptedException {
        mHelper.loadUrl(TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "match-case");
        Assert.assertEquals(2, mHelper.numBlocked());
        Assert.assertEquals(2, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_IMAGE));
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 2, "img[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyThirdPartyFilter() throws TimeoutException, InterruptedException {
        mHelper.loadUrl(TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "third-party");
        Assert.assertEquals(2, mHelper.numBlocked());
        Assert.assertEquals(2, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_IMAGE));
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 2, "img[data-expectedresult='fail']");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 2, "img[data-expectedresult='pass']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyOtherFilter() throws TimeoutException, InterruptedException {
        mHelper.loadUrl(TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "other");
        Assert.assertEquals(1, mHelper.numBlocked());
        Assert.assertEquals(1, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_OTHER));
        Assert.assertTrue(mHelper.isBlocked(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "other/image.png"));
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyDomainFilter() throws TimeoutException, InterruptedException {
        mHelper.loadUrl(TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "domain");
        Assert.assertEquals(2, mHelper.numBlocked());
        Assert.assertEquals(2, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_IMAGE));
        Assert.assertTrue(mHelper.isBlocked(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "domain/static/target/image.png"));
        Assert.assertTrue(mHelper.isBlocked(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "domain/dynamic/image.png"));
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 2, "img[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyPingFilter() throws TimeoutException, InterruptedException {
        mHelper.loadUrl(TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "ping");
        // Ping action not yet triggered
        Assert.assertEquals(0, mHelper.numBlocked());
        final CountDownLatch countDownLatch = new CountDownLatch(1);
        mHelper.setOnAdMatchedLatch(countDownLatch);
        // Trigger ping action
        final Tab tab = mActivityTestRule.getActivity().getActivityTab();
        JavaScriptUtils.executeJavaScriptAndWaitForResult(
                tab.getWebContents(), "document.getElementById(\"script-ping-trigger\").click()");
        // Wait with 10 seconds max timeout
        countDownLatch.await(10, TimeUnit.SECONDS);
        Assert.assertEquals(1, mHelper.numBlocked());
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyPingFilterException() throws TimeoutException, InterruptedException {
        mHelper.loadUrl(TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "ping");
        // Ping action not yet triggered
        Assert.assertEquals(0, mHelper.numAllowed());
        final CountDownLatch countDownLatch = new CountDownLatch(1);
        mHelper.setOnAdMatchedLatch(countDownLatch);
        // Trigger ping action
        final Tab tab = mActivityTestRule.getActivity().getActivityTab();
        JavaScriptUtils.executeJavaScriptAndWaitForResult(tab.getWebContents(),
                "document.getElementsByClassName(\"testcase-trigger\")[0].click()");
        // Wait with 10 seconds max timeout
        countDownLatch.await(10, TimeUnit.SECONDS);
        Assert.assertEquals(1, mHelper.numAllowed());
        Assert.assertEquals(0, mHelper.numBlocked());
    }
}
