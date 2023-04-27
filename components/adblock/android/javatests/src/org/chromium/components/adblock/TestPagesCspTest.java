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
public class TestPagesCspTest {
    @Rule
    public final ChromeBrowserTestRule mBrowserTestRule = new ChromeBrowserTestRule();
    @Rule
    public ChromeTabbedActivityTestRule mActivityTestRule = new ChromeTabbedActivityTestRule();
    private TestPagesTestsHelper mHelper = new TestPagesTestsHelper();

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
    public void testCspAllSites() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter("*$csp=script-src 'none'");
        mHelper.loadUrl(TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "csp_all");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "img[id='all-sites-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testCspSpecificSite() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("||%s^$csp=script-src https://%s/lib/utils.js",
                TestPagesTestsHelper.TESTPAGES_DOMAIN, TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "csp_specific");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "img[id='specific-site-fail-1']", IncludeSubframes.NO);
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testCspSpecificSiteFrameSrc() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("||%s^$csp=frame-src 'self'", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "csp_specific");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div[id='sub-frame-error']", IncludeSubframes.NO);
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div[id='sub-frame-error-details']", IncludeSubframes.NO);
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testCspException() throws TimeoutException, InterruptedException {
        // Blocking filter:
        mHelper.addCustomFilter(String.format("||%s^$csp=script-src https://%s/lib/utils.js",
                TestPagesTestsHelper.TESTPAGES_DOMAIN, TestPagesTestsHelper.TESTPAGES_DOMAIN));
        // Resource loaded by JS was blocked
        mHelper.loadUrl(TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "csp");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div[id='unblock-javascript'] > img");

        // Allowing filter:
        mHelper.addCustomFilter(
                String.format("@@||%s^$csp", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        // Resource loaded by JS was allowed
        mHelper.loadUrl(TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "csp");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "div[id='unblock-javascript'] > img");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testCspGenericBlockException() throws TimeoutException, InterruptedException {
        // Blocking filter:
        mHelper.addCustomFilter(String.format("||%s^$csp=script-src https://%s/lib/utils.js",
                TestPagesTestsHelper.TESTPAGES_DOMAIN, TestPagesTestsHelper.TESTPAGES_DOMAIN));
        // Resource loaded by JS was blocked
        mHelper.loadUrl(
                TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "csp_genericblock");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div[id='genericblock-javascript'] > img");

        // Allowing filter:
        mHelper.addCustomFilter(
                String.format("@@||%s^$genericblock", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        // Resource loaded by JS was allowed
        mHelper.loadUrl(
                TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "csp_genericblock");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "div[id='genericblock-javascript'] > img");
    }
}
