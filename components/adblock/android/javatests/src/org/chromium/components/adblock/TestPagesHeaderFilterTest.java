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
import org.chromium.chrome.test.ChromeBrowserTestRule;
import org.chromium.chrome.test.ChromeJUnit4ClassRunner;
import org.chromium.chrome.test.ChromeTabbedActivityTestRule;
import org.chromium.components.adblock.TestVerificationUtils.IncludeSubframes;

import java.util.concurrent.TimeoutException;

@RunWith(ChromeJUnit4ClassRunner.class)
@CommandLineFlags.Add({ChromeSwitches.DISABLE_FIRST_RUN_EXPERIENCE})
public class TestPagesHeaderFilterTest {
    @Rule
    public final ChromeBrowserTestRule mBrowserTestRule = new ChromeBrowserTestRule();
    @Rule
    public ChromeTabbedActivityTestRule mActivityTestRule = new ChromeTabbedActivityTestRule();
    private TestPagesTestsHelper mHelper = new TestPagesTestsHelper();

    public static final String HEADER_TESTPAGES_URL =
            TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "header";
    public static final String HEADER_EXCEPTIONS_TESTPAGES_URL =
            TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "header";

    @Before
    public void setUp() {
        mHelper.setUp(mActivityTestRule);
    }

    @After
    public void tearDown() {
        mHelper.tearDown();
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHeaderFilterScript() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("||%s/testfiles/header/$header=content-type=application/javascript",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(HEADER_TESTPAGES_URL);
        Assert.assertEquals(1, mHelper.numBlocked());
        Assert.assertEquals(1, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_SCRIPT));
        Assert.assertTrue(mHelper.isBlocked(String.format(
                "https://%s/testfiles/header/script.js", TestPagesTestsHelper.TESTPAGES_DOMAIN)));
        TestVerificationUtils.verifyDisplayedCount(mActivityTestRule, 0,
                "div#functionproperty-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHeaderFilterImage() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("||%s/testfiles/header/image.png$header=content-type=image/png",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(HEADER_TESTPAGES_URL);
        Assert.assertEquals(1, mHelper.numBlocked());
        Assert.assertEquals(1, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_IMAGE));
        Assert.assertTrue(mHelper.isBlocked(String.format(
                "https://%s/testfiles/header/image.png", TestPagesTestsHelper.TESTPAGES_DOMAIN)));
        TestVerificationUtils.verifyDisplayedCount(mActivityTestRule, 0, "img[id='image-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHeaderFilterImageAndComma() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("||%s/testfiles/header/image2.png$header=date=\\x2c",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(HEADER_TESTPAGES_URL);
        Assert.assertEquals(1, mHelper.numBlocked());
        Assert.assertEquals(1, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_IMAGE));
        Assert.assertTrue(mHelper.isBlocked(String.format(
                "https://%s/testfiles/header/image2.png", TestPagesTestsHelper.TESTPAGES_DOMAIN)));
        TestVerificationUtils.verifyDisplayedCount(mActivityTestRule, 0, "img[id='comma-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHeaderFilterStylesheet() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("||%s/testfiles/header/$header=content-type=text/css",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(HEADER_TESTPAGES_URL);
        Assert.assertEquals(1, mHelper.numBlocked());
        Assert.assertEquals(
                1, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_STYLESHEET));
        Assert.assertTrue(
                mHelper.isBlocked(String.format("https://%s/testfiles/header/stylesheet.css",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN)));
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHeaderFilterException() throws TimeoutException, InterruptedException {
        // Add blocking filter, expect blocked image
        mHelper.addCustomFilter(
                String.format("||%s/testfiles/header_exception/$header=content-type=image/png",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(HEADER_EXCEPTIONS_TESTPAGES_URL);
        Assert.assertEquals(1, mHelper.numBlocked());
        Assert.assertEquals(1, mHelper.numBlockedByType(AdblockContentType.CONTENT_TYPE_IMAGE));
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "img[id='image-header-exception-pass-1']");

        // Add exception filter, expect image allowed
        mHelper.addCustomFilter(String.format(
                "@@%s/testfiles/header_exception/$header", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(HEADER_EXCEPTIONS_TESTPAGES_URL);
        Assert.assertEquals(1, mHelper.numAllowed());
        Assert.assertEquals(1, mHelper.numAllowedByType(AdblockContentType.CONTENT_TYPE_IMAGE));
        Assert.assertTrue(
                mHelper.isAllowed(String.format("https://%s/testfiles/header_exception/image.png",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN)));
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "img[id='image-header-exception-pass-1']");
    }
}
