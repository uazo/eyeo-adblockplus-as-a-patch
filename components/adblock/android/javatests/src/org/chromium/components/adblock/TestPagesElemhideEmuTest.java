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
public class TestPagesElemhideEmuTest {
    @Rule
    public ChromeTabbedActivityTestRule mActivityTestRule = new ChromeTabbedActivityTestRule();
    private TestPagesTestsHelper mHelper = new TestPagesTestsHelper();

    public static final String ELEMENT_HIDING_EMULATION_TESTPAGES_URL =
            TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT + "element-hiding-emulation";
    public static final String ELEMENT_HIDING_EMULATION_EXCEPTIONS_TESTPAGES_URL =
            TestPagesTestsHelper.EXCEPTION_TESTPAGES_TESTCASES_ROOT + "element-hiding";

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
    public void testElemHideEmuFiltersBasicAbpProperties()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#?#div:-abp-properties(width: 213px)", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='basic-abp-properties-usage-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuFiltersBasicAbpHas() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#?#div:-abp-has(>div>span.ehe-abp-has)", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='basic-abp-has-usage-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuFiltersBasicHas() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#?#div:has(>div>span.ehe-has)", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        // "Basic :has() usage" are duplicated on testpage.
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 2, "div[id='basic-has-usage-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuFiltersBasicAbpContains()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#?#span:-abp-contains(ehe-contains-target)",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "span[id='basic-abp-contains-usage-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuFiltersBasicXpath() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s#?#span:xpath(//*[@id=\"basic-xpath-usage-fail\"])",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "span[id='basic-xpath-usage-fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuFiltersBasicHasText() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#?#span:has-text(ehe-has-text)", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "span[id='basic-has-text-usage-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuFiltersChainedExtendedSelectors()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s#?#div:-abp-has(> div:-abp-properties(width: 214px))",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='chained-extended-selectors-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuFiltersCaseInsensitiveExtendedSelectors()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#?#div:-abp-properties(WiDtH: 215px)", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='case-insensitive-extended-selectors-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuFiltersWildcardInExtendedSelector()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#?#div:-abp-properties(cursor:*)", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='wildcard-in-extended-selector-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuFiltersRegularExpressionInAbpProperties()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#?#div:-abp-properties(/width: 12[1-5]px;/)",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='regular-expression-in-abp-properties-fail-1']");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='regular-expression-in-abp-properties-fail-2']");
        // "Not a target" div is not hidden, does not match regular expression.
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "div[class='testcase-examplecontent ehe-regex3']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuFiltersRegularExpressionInAbpContains()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s#?#div > div:-abp-contains(/ehe-containsregex\\d/)",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='regular-expression-in-abp-contains-fail-1']");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='regular-expression-in-abp-contains-fail-2']");
    }

    // Exceptions:
    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuFiltersException() throws TimeoutException, InterruptedException {
        // Add a blocking filter, verify element hidden.
        mHelper.addCustomFilter(
                String.format("%s##.testcase-ehe", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_EXCEPTIONS_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='exception-usage-pass-1']");

        // Add exception filter, verify element no longer hidden.
        mHelper.addCustomFilter(
                String.format("%s#@#.testcase-ehe", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_EXCEPTIONS_TESTPAGES_URL);
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "div[id='exception-usage-pass-1']");
    }
}
