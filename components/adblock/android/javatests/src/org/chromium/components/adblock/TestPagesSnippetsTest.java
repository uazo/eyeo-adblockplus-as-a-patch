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

import java.util.concurrent.TimeoutException;

@RunWith(ChromeJUnit4ClassRunner.class)
@CommandLineFlags.Add({ChromeSwitches.DISABLE_FIRST_RUN_EXPERIENCE})
public class TestPagesSnippetsTest {
    @Rule
    public final ChromeBrowserTestRule mBrowserTestRule = new ChromeBrowserTestRule();
    @Rule
    public ChromeTabbedActivityTestRule mActivityTestRule = new ChromeTabbedActivityTestRule();
    private TestPagesTestsHelper mHelper = new TestPagesTestsHelper();

    public static final String SNIPPETS_TESTPAGES_TESTCASES_ROOT =
            TestPagesTestsHelper.TESTPAGES_TESTCASES_ROOT + "snippets/";

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
    public void testAbortCurrentInlineScriptBasic() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#abort-current-inline-script console.group",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "abort-current-inline-script");
        // All "Abort" snippets cancel creation of the target div, so it won't be hidden - it will
        // not exist in DOM. Therefore we verify it's not displayed instead of verifying it's
        // hidden.
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div#basic-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testAbortCurrentInlineScriptSearch() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s#$#abort-current-inline-script console.info acis-search",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "abort-current-inline-script");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div#search-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testAbortCurrentInlineScriptRegex() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s#$#abort-current-inline-script console.warn '/acis-regex[1-2]/'",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "abort-current-inline-script");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div#regex-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testAbortOnPropertyReadBasic() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#$#abort-on-property-read aoprb", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(SNIPPETS_TESTPAGES_TESTCASES_ROOT + "abort-on-property-read");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div#basic-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testAbortOnPropertyReadSubProperty() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#$#abort-on-property-read aopr.sp", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(SNIPPETS_TESTPAGES_TESTCASES_ROOT + "abort-on-property-read");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div#subproperty-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testAbortOnPropertyReadFunctionProperty()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#$#abort-on-property-read aoprf.fp", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(SNIPPETS_TESTPAGES_TESTCASES_ROOT + "abort-on-property-read");
        TestVerificationUtils.verifyDisplayedCount(mActivityTestRule, 0,
                "div#functionproperty-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testAbortOnPropertyWriteBasic() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#abort-on-property-write window.aopwb",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "abort-on-property-write");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div#basic-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testAbortOnPropertyWriteSubProperty()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#abort-on-property-write window.aopwsp",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "abort-on-property-write");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div#subproperty-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testAbortOnPropertyWriteFunctionProperty()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#$#abort-on-property-write aopwf.fp", TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "abort-on-property-write");
        TestVerificationUtils.verifyDisplayedCount(mActivityTestRule, 0,
                "div#functionproperty-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testAbortOnIframePropertyReadBasic() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#abort-on-iframe-property-read aoiprb",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "abort-on-iframe-property-read");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div#basic-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testAbortOnIframePropertyReadSubProperty()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#abort-on-iframe-property-read aoipr.sp",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "abort-on-iframe-property-read");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div#subproperty-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testAbortOnIframePropertyReadMultipleProperties()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#abort-on-iframe-property-read aoipr1 aoipr2",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "abort-on-iframe-property-read");
        TestVerificationUtils.verifyDisplayedCount(mActivityTestRule, 0,
                "div#multipleproperties-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testAbortOnIframePropertyWriteBasic()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#abort-on-iframe-property-write aoipwb",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "abort-on-iframe-property-write");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div#basic-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testAbortOnIframePropertyWriteSubProperty()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#abort-on-iframe-property-write aoipw.sp",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "abort-on-iframe-property-write");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div#subproperty-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testAbortOnIframePropertyWriteMultipleProperties()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#abort-on-iframe-property-write aoipw1 aoipw2",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "abort-on-iframe-property-write");
        TestVerificationUtils.verifyDisplayedCount(mActivityTestRule, 0,
                "div#multipleproperties-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfContainsStatic() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#hide-if-contains 'hic-basic-static' p[id]",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-contains");
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1, "p#hic-static-id");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfContainsDynamic() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#hide-if-contains 'hic-basic-dynamic' p[id]",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-contains");
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1, "p#hic-dynamic-id");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfContainsSearch() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#hide-if-contains 'hic-search' p[id] .target",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-contains");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div#search2-target > p.target");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "div#search1-target > p[data-expectedresult='pass']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfContainsRegex() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#hide-if-contains /hic-regex-[2-3]/ p[id]",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-contains");
        // "hic-regex-1" does not match regex, should remain displayed.
        TestVerificationUtils.verifyDisplayedCount(mActivityTestRule, 1, "p#hic-regex-1");

        // "hic-regex-2" and "hic-regex-2" do match regex, should be hidden.
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1, "p#hic-regex-2");
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1, "p#hic-regex-3");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfContainsFrame() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#hide-if-contains hidden span#frame-target",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-contains");
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1, "span#frame-target");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfContainsAndMatchesStyleStatic()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#$#hide-if-contains-and-matches-style hicamss div[id] span.label /./ 'display: inline;'",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-contains-and-matches-style");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div#static-usage-area > div[data-expectedresult='fail']");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "div#static-usage-area > div[data-expectedresult='pass']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfContainsAndMatchesStyleDynamic()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#$#hide-if-contains-and-matches-style hicamsd div[id] span.label /./ 'display: inline;'",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-contains-and-matches-style");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div#dynamic-target > div[data-expectedresult='fail']");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "div#dynamic-target > div[data-expectedresult='pass']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfContainsImage() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#$#hide-if-contains-image /^89504e470d0a1a0a0000000d4948445200000064000000640802/ div[shouldhide] div",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-contains-image");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 2, "div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfContainsVisibleTextBasicUsage()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#$#hide-if-contains-visible-text Sponsored-hicvt-basic '#parent-basic > .article' '#parent-basic > .article .label'",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-contains-visible-text");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div#parent-basic > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfContainsVisibleTextContentUsage()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#$#hide-if-contains-visible-text Sponsored-hicvt-content '#parent-content > .article' '#parent-content > .article .label'",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-contains-visible-text");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div#parent-content > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfHasAndMatchesStyleBasicUsage()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#$#hide-if-has-and-matches-style a[href=\"#basic-target-ad\"] div[id] span.label /./ 'display: inline;'",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-has-and-matches-style");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div#basic-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfHasAndMatchesStyleLegitElements()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#$#hide-if-has-and-matches-style a[href=\"#comments-target-ad\"] div[id] span.label ';' /\\\\bdisplay:\\ inline\\;/",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-has-and-matches-style");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "div#comments-target > div[data-expectedresult='pass']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfLabeledBy() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format(
                "%s#$#hide-if-labelled-by 'Label' '#hilb-target [aria-labelledby]' '#hilb-target'",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-labelled-by");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div[data-expectedresult='fail']");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "div[data-expectedresult='pass']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfMatchesXPathBasicStaticUsage()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#hide-if-matches-xpath //*[@id=\"isnfnv\"]",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-matches-xpath");
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1,
                "div#basic-static-usage-area > div[data-expectedresult='fail']");
        TestVerificationUtils.verifyDisplayedCount(mActivityTestRule, 1,
                "div#basic-static-usage-area > div[data-expectedresult='pass']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfMatchesXPathClassUsage() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s#$#hide-if-matches-xpath //*[@class=\"to-be-hidden\"]",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-matches-xpath");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div#class-usage-area > div[data-expectedresult='fail']");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "div#class-usage-area > div[data-expectedresult='pass']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfMatchesXPathIdStartsWith() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s#$#hide-if-matches-xpath //div[starts-with(@id,\"fail\")]",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-matches-xpath");
        TestVerificationUtils.verifyHiddenCount(mActivityTestRule, 1,
                "div#hide-if-id-starts-with-area > div[data-expectedresult='fail']");
        TestVerificationUtils.verifyDisplayedCount(mActivityTestRule, 1,
                "div#hide-if-id-starts-with-area > div[data-expectedresult='pass']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfShadowContainsBasicUsage() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#hide-if-shadow-contains 'hisc-basic' p",
                TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-shadow-contains");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div#basic-target > p[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testHideIfShadowContainsRegexUsage() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s#$#hide-if-shadow-contains '/hisc-regex[1-2]/' div",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "hide-if-shadow-contains");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 2, "div#regex-target > div[data-expectedresult='fail']");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "div#regex-target > div[data-expectedresult='pass']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testJsonPrune() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s#$#json-prune 'data-expectedresult jsonprune aria-label'",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(SNIPPETS_TESTPAGES_TESTCASES_ROOT + "json-prune?delay=100");
        // The object does not get hidden, it no longer exists in the DOM.
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div#testcase-area > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testOverridePropertyRead() throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s#$#override-property-read overridePropertyRead.fp false",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(SNIPPETS_TESTPAGES_TESTCASES_ROOT + "override-property-read");
        TestVerificationUtils.verifyHiddenCount(
                mActivityTestRule, 1, "div#basic-target > div[data-expectedresult='fail']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testStripFetchQueryParameterBasicUsage()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(String.format("%s#$#strip-fetch-query-parameter basicBlocked %s",
                TestPagesTestsHelper.TESTPAGES_DOMAIN, TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "strip-fetch-query-parameter");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 0, "div#basic-target > div[data-expectedresult='fail']");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 1, "div#basic-target > div[data-expectedresult='pass']");
    }

    @Test
    @LargeTest
    @Feature({"adblock"})
    public void testStripFetchQueryParameterOtherUsage()
            throws TimeoutException, InterruptedException {
        mHelper.addCustomFilter(
                String.format("%s#$#strip-fetch-query-parameter otherAllowed2 other-domain",
                        TestPagesTestsHelper.TESTPAGES_DOMAIN));
        mHelper.loadUrlWaitForContent(
                SNIPPETS_TESTPAGES_TESTCASES_ROOT + "strip-fetch-query-parameter");
        TestVerificationUtils.verifyDisplayedCount(
                mActivityTestRule, 2, "div#other-target > div[data-expectedresult='pass']");
    }
}
