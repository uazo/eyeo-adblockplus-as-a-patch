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

import org.chromium.chrome.test.ChromeTabbedActivityTestRule;
import org.chromium.content_public.browser.test.util.JavaScriptUtils;

import java.util.concurrent.TimeoutException;

public class TestVerificationUtils {
    public enum IncludeSubframes {
        YES,
        NO,
    }

    private static final String STATUS_OK = "\"OK\"";

    private static final String MATCHES_HIDDEN_FUNCTION = "let matches = function(element) {"
            + "  return window.getComputedStyle(element).display == \"none\";"
            + "}";

    private static final String MATCHES_DISPLAYED_FUNCTION = "let matches = function(element) {"
            + "  return window.getComputedStyle(element).display != \"none\";"
            + "}";

    private static final String COUNT_ELEMENT_FUNCTION =
            "let countElements = function(selector, includeSubframes) {"
            + "  let count = 0;"
            + "  for (let element of document.querySelectorAll(selector)) {"
            + "    if (matches(element))"
            + "      ++count;"
            + "  }"
            + "  if (includeSubframes) {"
            + "    for (let frame of document.querySelectorAll(\"iframe\")) {"
            + "      for (let element of frame.contentWindow.document.body.querySelectorAll(selector)) {"
            + "        if (matches(element))"
            + "          ++count;"
            + "      }"
            + "    }"
            + "  }"
            + "  return count;"
            + "}";

    private static final String WAIT_FOR_COUNT_FUNCTION_WRAPPER = "(function () {"
            + "%s\n" // matches() defintion placeholder
            + "%s\n" // countElements() defintion placeholder
            + "%s\n" // WAIT_FUNCTION placeholder which calls countElements() as a predicate
            + "}());";

    // Poll every 100 ms until condition is met or 10 seconds timeout occurs
    private static final String WAIT_FUNCTION = "let repeat = 100;"
            + "const id = setInterval(() => {"
            + "  --repeat;"
            + "  if (%s) {" // predicate placeholder
            + "    clearInterval(id);"
            + "    domAutomationController.send('OK');"
            + "  } else if (repeat == 0) {"
            + "    clearInterval(id);"
            + "    domAutomationController.send('Timeout');"
            + "  }"
            + "}, 100);";

    private static void verifyMatchesCount(final ChromeTabbedActivityTestRule mActivityTestRule,
            final int num, final String matchesFunction, final String selector,
            IncludeSubframes includeSubframes) throws TimeoutException {
        final String boolIncludeSubframes =
                includeSubframes == IncludeSubframes.YES ? "true" : "false";
        final String predicate = String.format(
                "countElements(\"%s\", %s) == %d", selector, boolIncludeSubframes, num);
        final String waitFunction = String.format(WAIT_FUNCTION, predicate);
        final String js = String.format(WAIT_FOR_COUNT_FUNCTION_WRAPPER, matchesFunction,
                COUNT_ELEMENT_FUNCTION, waitFunction);
        final String result = JavaScriptUtils.runJavascriptWithAsyncResult(
                mActivityTestRule.getActivity().getActivityTab().getWebContents(), js);
        Assert.assertEquals(STATUS_OK, result);
    }

    public static void verifyHiddenCount(final ChromeTabbedActivityTestRule mActivityTestRule,
            final int num, final String selector) throws TimeoutException {
        verifyHiddenCount(mActivityTestRule, num, selector, IncludeSubframes.YES);
    }

    public static void verifyHiddenCount(final ChromeTabbedActivityTestRule mActivityTestRule,
            final int num, final String selector, final IncludeSubframes includeSubframes)
            throws TimeoutException {
        verifyMatchesCount(
                mActivityTestRule, num, MATCHES_HIDDEN_FUNCTION, selector, includeSubframes);
    }

    public static void verifyDisplayedCount(final ChromeTabbedActivityTestRule mActivityTestRule,
            final int num, final String selector) throws TimeoutException {
        verifyDisplayedCount(mActivityTestRule, num, selector, IncludeSubframes.YES);
    }

    public static void verifyDisplayedCount(final ChromeTabbedActivityTestRule mActivityTestRule,
            final int num, final String selector, final IncludeSubframes includeSubframes)
            throws TimeoutException {
        verifyMatchesCount(
                mActivityTestRule, num, MATCHES_DISPLAYED_FUNCTION, selector, includeSubframes);
    }

    public static void verifyCondition(final ChromeTabbedActivityTestRule mActivityTestRule,
            final String predicate) throws TimeoutException {
        final String waitFunction = String.format(WAIT_FUNCTION, predicate);
        Assert.assertEquals(STATUS_OK,
                JavaScriptUtils.runJavascriptWithAsyncResult(
                        mActivityTestRule.getActivity().getActivityTab().getWebContents(),
                        waitFunction));
    }

    public static void verifyGreenBackground(final ChromeTabbedActivityTestRule mActivityTestRule,
            final String elemId) throws TimeoutException {
        verifyCondition(mActivityTestRule,
                "window.getComputedStyle(document.getElementById('" + elemId
                        + "')).backgroundColor == 'rgb(13, 199, 75)'");
    }

    // For some cases it is better to rely on page script testing element
    // rather than invent a specific script to check condition. For example
    // checks for rewrite filters replaces content proper way.
    public static void verifySelfTestPass(final ChromeTabbedActivityTestRule mActivityTestRule,
            final String elemId) throws TimeoutException {
        verifyCondition(mActivityTestRule,
                "document.getElementById('" + elemId
                        + "').getAttribute('data-expectedresult') == 'pass'");
    }

    public static void expectResourceBlocked(final ChromeTabbedActivityTestRule mActivityTestRule,
            final String elemId) throws TimeoutException {
        verifyCondition(mActivityTestRule,
                "window.getComputedStyle(document.getElementById('" + elemId
                        + "')).display == 'none'");
    }

    public static void expectResourceShown(final ChromeTabbedActivityTestRule mActivityTestRule,
            final String elemId) throws TimeoutException {
        verifyCondition(mActivityTestRule,
                "window.getComputedStyle(document.getElementById('" + elemId
                        + "')).display == 'inline'");
    }
}
