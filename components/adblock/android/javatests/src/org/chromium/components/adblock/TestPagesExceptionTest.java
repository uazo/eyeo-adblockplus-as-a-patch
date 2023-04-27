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
import org.chromium.base.test.util.Feature;
import org.chromium.chrome.browser.flags.ChromeSwitches;
import org.chromium.chrome.test.ChromeBrowserTestRule;
import org.chromium.chrome.test.ChromeJUnit4ClassRunner;
import org.chromium.chrome.test.ChromeTabbedActivityTestRule;
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
public class TestPagesExceptionTest {
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
    public void testVerifyImageExceptions() throws TimeoutException, InterruptedException {
        final String subdomainImage = String.format(
                "https://allowed.subdomain.%s/testfiles/image_exception/subdomain.png",
                TestPagesTestsHelper.TESTPAGES_DOMAIN);
        mHelper.loadUrl(TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "image");
        Assert.assertEquals(2, mHelper.numAllowed());
        Assert.assertEquals(2, mHelper.numAllowedByType(AdblockContentType.CONTENT_TYPE_IMAGE));
        Assert.assertTrue(mHelper.isAllowed(subdomainImage));
        Assert.assertTrue(mHelper.isAllowed(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "image_exception/image.png"));
        TestVerificationUtils.verifyDisplayedCount(mActivityTestRule, 2, "img");
        String numImages = JavaScriptUtils.executeJavaScriptAndWaitForResult(
                mHelper.getWebContents(), "document.getElementsByTagName(\"img\").length;");
        Assert.assertEquals("2", numImages);
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyPopupException() throws TimeoutException, InterruptedException {
        final String POPUP_TESTACE_URL =
                TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "popup";
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
        Assert.assertEquals(4, mHelper.getTabCount());
        Assert.assertEquals(3, mHelper.numAllowedPopups());
        Assert.assertTrue(mHelper.isPopupAllowed(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "popup_exception/link.html"));
        Assert.assertTrue(mHelper.isPopupAllowed(TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT
                + "popup_exception/script-window.html"));
        Assert.assertTrue(mHelper.isPopupAllowed(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "popup_exception/script-tab.html"));
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifySubdocumentException() throws TimeoutException, InterruptedException {
        mHelper.loadUrl(TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "subdocument");
        Assert.assertEquals(1, mHelper.numAllowed());
        Assert.assertTrue(mHelper.isAllowed(TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT
                + "subdocument_exception/subdocument.html"));
        TestVerificationUtils.verifyGreenBackground(mActivityTestRule, "exception-target");
        String numFrames = JavaScriptUtils.executeJavaScriptAndWaitForResult(
                mHelper.getWebContents(), "window.frames.length;");
        Assert.assertEquals("1", numFrames);
        TestVerificationUtils.verifyDisplayedCount(mActivityTestRule, 1, "iframe");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyScriptException() throws TimeoutException, InterruptedException {
        mHelper.loadUrl(TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "script");
        Assert.assertEquals(1, mHelper.numAllowed());
        Assert.assertTrue(mHelper.isAllowed(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "script_exception/script.js"));
        TestVerificationUtils.verifyGreenBackground(mActivityTestRule, "script-target");
        Assert.assertEquals(1, mHelper.numBlocked());
        Assert.assertTrue(mHelper.isBlocked(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "script_exception/image.png"));
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "img[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyStylesheetException() throws TimeoutException, InterruptedException {
        final String allowedUrl = TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT
                + "stylesheet_exception/stylesheet.cs";
        final CountDownLatch countDownLatch =
                mHelper.setOnAdMatchedExpectations(null, new HashSet<>(Arrays.asList(allowedUrl)));
        mHelper.loadUrl(TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "stylesheet");
        // Wait with 10 seconds max timeout
        countDownLatch.await(10, TimeUnit.SECONDS);
        Assert.assertEquals(1, mHelper.numAllowed());
        Assert.assertTrue(mHelper.isAllowed(allowedUrl));
        Assert.assertEquals(1, mHelper.numBlocked());
        Assert.assertTrue(mHelper.isBlocked(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "stylesheet_exception/image.png"));
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1, "img");
        TestVerificationUtils.verifyGreenBackground(mActivityTestRule, "exception-target");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyXHRException() throws TimeoutException, InterruptedException {
        final String allowedUrl =
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "xmlhttprequest_exception/text.txt";
        final CountDownLatch countDownLatch =
                mHelper.setOnAdMatchedExpectations(null, new HashSet<>(Arrays.asList(allowedUrl)));
        mHelper.loadUrl(TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "xmlhttprequest");
        // Wait with 10 seconds max timeout
        countDownLatch.await(10, TimeUnit.SECONDS);
        Assert.assertEquals(1, mHelper.numAllowed());
        Assert.assertTrue(mHelper.isAllowed(allowedUrl));
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyGenericBlockException() throws TimeoutException, InterruptedException {
        mHelper.loadUrl(TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "genericblock");
        Assert.assertEquals(1, mHelper.numBlocked());
        Assert.assertTrue(mHelper.isBlocked(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "genericblock/specific.png"));
        Assert.assertEquals(1, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_IMAGE));
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "img[data-expectedresult='pass']");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "img[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyGenericHideException() throws TimeoutException, InterruptedException {
        mHelper.loadUrl(TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "generichide");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "div[data-expectedresult='pass']");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyDocumentException() throws TimeoutException, InterruptedException {
        mHelper.loadUrl(TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "document");
        Assert.assertTrue(mHelper.isAllowed(
                TestPagesTestsHelper.TESTPAGES_RESOURCES_ROOT + "document/image.png"));
        Assert.assertTrue(mHelper.isPageAllowed(
                TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "document"));
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "div[data-expectedresult='pass']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testVerifyWebSocketException() throws TimeoutException, InterruptedException {
        final String wssUrl =
                String.format("wss://%s/websocket", TestPagesTestsHelper.TESTPAGES_DOMAIN);
        final CountDownLatch countDownLatch =
                mHelper.setOnAdMatchedExpectations(null, new HashSet<>(Arrays.asList(wssUrl)));
        mHelper.loadUrl(TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "websocket");
        // Wait with 10 seconds max timeout
        countDownLatch.await(10, TimeUnit.SECONDS);
        Assert.assertEquals(1, mHelper.numAllowedByType(AdblockContentType.CONTENT_TYPE_WEBSOCKET));
        Assert.assertTrue(mHelper.isAllowed(wssUrl));
    }
}
