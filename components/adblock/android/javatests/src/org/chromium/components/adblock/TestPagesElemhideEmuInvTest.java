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
public class TestPagesElemhideEmuInvTest {
    @Rule
    public ChromeTabbedActivityTestRule mActivityTestRule = new ChromeTabbedActivityTestRule();
    private TestPagesTestsHelper mHelper = new TestPagesTestsHelper();

    public static final String ELEMENT_HIDING_EMULATION_TESTPAGES_URL =
            TestPagesTestsHelper.FILTER_TESTPAGES_TESTCASES_ROOT
            + "element-hiding-emulation-inversion";

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
    public void testElemHideEmuNotAbpProperties() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s#?#.ehei-properties:not(:-abp-properties(width: 238px))",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='basic-not-abp-properties-usage-fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuNotAbpHas() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s#?#.ehei-has:not(:-abp-has(span.ehei-has-not-hide))",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='basic-not-abp-has-usage-fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuNotAbpContains() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s#?#.ehei-contains:not(span:-abp-contains(example-content))",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "span[id='basic-not-abp-contains-usage-fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuNotChained() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#?#.ehei-chained-parent:not(:-abp-has(> div:-abp-properties(width: 198px)))",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1,
                "div[id='chained-extended-selectors-with-not-selector-fail-1']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuNotCaseIsensitive() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#?#.ehei-case:not(:-abp-properties(WiDtH: 209px))",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1,
                "div[id='case-insensitive-extended-selectors-with-not-selector-fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuNotWildcard() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#?#.ehei-wildcard:not(:-abp-properties(cursor:*))",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1,
                "div[id='wildcard-in-extended-selector-combined-with-not-selector-fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuNotRegexAbpProperties()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s#?#.ehei-regex:not(:-abp-properties(/width: 11[1-5]px;/))",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='regular-expression-in-not-abp-properties-fail-1']");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[id='regular-expression-in-not-abp-properties-fail-2']");
        TestVerificationUtils.verifyDisplayedCount(mActivityTestRule, 1, ".ehei-regex3");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testElemHideEmuNotRegexAbpContains() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#?#.ehei-contains-regex:not(span:-abp-contains(/example-contentregex\\d/))",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrl(ELEMENT_HIDING_EMULATION_TESTPAGES_URL);
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "span[id='regular-expression-in-not-abp-contains-fail-1']");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "span[id='regular-expression-in-not-abp-contains-fail-2']");
        TestVerificationUtils.verifyDisplayedCount(mActivityTestRule, 2, ".ehei-contains-regex");
    }
}
