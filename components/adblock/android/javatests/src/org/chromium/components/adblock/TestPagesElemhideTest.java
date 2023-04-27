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
import org.chromium.chrome.test.ChromeJUnit4ClassRunner;
import org.chromium.chrome.test.ChromeTabbedActivityTestRule;

import java.util.concurrent.TimeoutException;

@RunWith(ChromeJUnit4ClassRunner.class)
@CommandLineFlags.Add({ChromeSwitches.DISABLE_FIRST_RUN_EXPERIENCE})
public class TestPagesElemhideTest {
    @Rule
    public ChromeTabbedActivityTestRule mActivityTestRule = new ChromeTabbedActivityTestRule();
    private TestPagesTestsHelper mHelper = new TestPagesTestsHelper();

    public static final String ELEMENT_HIDING_TESTPAGES_URL =
            TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "element-hiding";
    public static final String ELEMENT_HIDING_EXCEPTIONS_TESTPAGES_URL =
            TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "elemhide";

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
    public void testElemHideFiltersIdSelector() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s###eh-id", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1, "div[id='eh-id']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideFiltersIdSelectorDoubleCurlyBraces()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s##div[id='{{eh-id}}']", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1, "div[id='{{eh-id}}']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideFiltersClassSelector() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s##.eh-class", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1, "div[class='eh-class']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideFiltersDescendantSelector()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s##.testcase-area > .eh-descendant", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1, "div[class='eh-descendant']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideFiltersSiblingSelector() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s##.testcase-examplecontent + .eh-sibling",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1, "div[class='eh-sibling']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideFiltersAttributeSelector1()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s##div[height=\"100\"][width=\"100\"]", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='attribute-selector-1-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideFiltersAttributeSelector2()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s##div[href=\"http://testcase-attribute.com/\"]",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='attribute-selector-2-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideFiltersAttributeSelector3()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s##div[style=\"width: 200px;\"]", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='attribute-selector-3-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideFiltersStartsWithSelector1()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s##div[href^=\"http://testcase-startswith.com/\"]",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='starts-with-selector-1-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideFiltersStartsWithSelector2()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s##div[style^=\"width: 201px;\"]", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='starts-with-selector-2-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideFiltersEndsWithSelector1()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s##div[style$=\"width: 202px;\"]", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='ends-with-selector-1-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideFiltersContains() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s##div[style*=\"width: 203px;\"]", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1, "div[id='contains-fail-1']");
    }

    // Exceptions:
    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideFiltersBasicException() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s##.ex-elemhide", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.addCustomFilter(String.format(
                "||%s/testfiles/elemhide/basic/*", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EXCEPTIONS_TESTPAGES_URL);
        // No exceptions added yet, both objects should be blocked.
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "img[id='basic-usage-fail-1']");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='basic-usage-pass-1']");
        // Add exception filter and reload.
        mHelper.addCustomFilter(String.format(
                "@@%s/en/exceptions/elemhide$elemhide", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EXCEPTIONS_TESTPAGES_URL);
        // Image should remain blocked, div should be unblocked.
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "img[id='basic-usage-fail-1']");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "div[id='basic-usage-area'] > div[id='basic-usage-pass-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideFiltersIframeException() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s##.targ-elemhide", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.addCustomFilter(String.format(
                "||%s/testfiles/elemhide/iframe/*.png", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EXCEPTIONS_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1, "img[id='iframe-fail-1']");
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1, "div[id='iframe-pass-1']");

        // Add exception filter and reload.
        mHelper.addCustomFilter(String.format(
                "@@%s/en/exceptions/elemhide$elemhide", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EXCEPTIONS_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1, "img[id='iframe-fail-1']");
        TestVerificationUtils.verifyDisplayedCount(mActivityTestRule, 1, "div[id='iframe-pass-1']");
    }
}
