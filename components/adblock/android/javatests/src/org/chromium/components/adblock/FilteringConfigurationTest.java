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

import android.support.test.InstrumentationRegistry;

import androidx.test.filters.LargeTest;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.base.test.util.CallbackHelper;
import org.chromium.base.test.util.CommandLineFlags;
import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.chromium.base.test.util.IntegrationTest;
import org.chromium.chrome.browser.flags.ChromeSwitches;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.test.ChromeBrowserTestRule;
import org.chromium.chrome.test.ChromeJUnit4ClassRunner;
import org.chromium.chrome.test.ChromeTabbedActivityTestRule;
import org.chromium.content_public.browser.test.util.JavaScriptUtils;
import org.chromium.content_public.browser.test.util.TestThreadUtils;
import org.chromium.content_public.common.ContentSwitches;
import org.chromium.net.test.EmbeddedTestServer;

import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

@RunWith(ChromeJUnit4ClassRunner.class)
@CommandLineFlags.Add({ChromeSwitches.DISABLE_FIRST_RUN_EXPERIENCE,
        ContentSwitches.HOST_RESOLVER_RULES + "=MAP * 127.0.0.1"})
public class FilteringConfigurationTest {
    @Rule
    public final ChromeBrowserTestRule mBrowserTestRule = new ChromeBrowserTestRule();
    @Rule
    public ChromeTabbedActivityTestRule mActivityTestRule = new ChromeTabbedActivityTestRule();
    private CallbackHelper mHelper = new CallbackHelper();
    public FilteringConfiguration mConfigurationA;
    public FilteringConfiguration mConfigurationB;
    private EmbeddedTestServer mTestServer;
    private String mTestUrl;

    private class TestConfigurationChangeObserver
            implements FilteringConfiguration.ConfigurationChangeObserver {
        public volatile boolean mOnEnabledStateChangedCalled;
        public volatile boolean mOnFilterListsChanged;
        public volatile boolean mOnAllowedDomainsChanged;
        public volatile boolean mOnCustomFiltersChanged;

        public TestConfigurationChangeObserver() {
            mOnEnabledStateChangedCalled = false;
            mOnFilterListsChanged = false;
            mOnAllowedDomainsChanged = false;
            mOnCustomFiltersChanged = false;
        }

        @Override
        public void onEnabledStateChanged() {
            mOnEnabledStateChangedCalled = true;
        }
        @Override
        public void onFilterListsChanged() {
            mOnFilterListsChanged = true;
        }

        @Override
        public void onAllowedDomainsChanged() {
            mOnAllowedDomainsChanged = true;
        }

        @Override
        public void onCustomFiltersChanged() {
            mOnCustomFiltersChanged = true;
        }
    }

    public void loadTestUrl() throws InterruptedException {
        mActivityTestRule.loadUrl(mTestUrl, 5);
    }

    public String getResourcesComputedStyle() throws TimeoutException {
        final Tab tab = mActivityTestRule.getActivity().getActivityTab();
        final String javascript =
                "window.getComputedStyle(document.getElementById('subresource')).display";
        return JavaScriptUtils.executeJavaScriptAndWaitForResult(tab.getWebContents(), javascript);
    }

    public void expectResourceBlocked() throws TimeoutException {
        Assert.assertEquals("\"none\"", getResourcesComputedStyle());
    }

    public void expectResourceShown() throws TimeoutException {
        Assert.assertEquals("\"inline\"", getResourcesComputedStyle());
    }

    @Before
    public void setUp() throws TimeoutException {
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            mConfigurationA = new FilteringConfiguration("a");
            mConfigurationB = new FilteringConfiguration("b");

            mHelper.notifyCalled();
        });
        mHelper.waitForCallback(0, 1, 10, TimeUnit.SECONDS);
        mActivityTestRule.startMainActivityOnBlankPage();
        mTestServer = EmbeddedTestServer.createAndStartServer(InstrumentationRegistry.getContext());
        mTestUrl = mTestServer.getURLWithHostName(
                "test.org", "/chrome/test/data/adblock/innermost_frame.html");
    }

    @After
    public void tearDown() {
        mTestServer.stopAndDestroyServer();
    }

    @Test
    @IntegrationTest
    @LargeTest
    @Feature({"adblock"})
    public void addingAllowedDomains() throws Exception {
        final List<String> allowedDomainsA = new ArrayList<>();
        final List<String> allowedDomainsB = new ArrayList<>();
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            mConfigurationA.addAllowedDomain("foobar.com");
            mConfigurationA.addAllowedDomain("domain.com/path/to/page.html");
            mConfigurationA.addAllowedDomain("domain.com/duplicate.html");
            allowedDomainsA.addAll(mConfigurationA.getAllowedDomains());

            mConfigurationB.addAllowedDomain("https://scheme.com/path.html");
            mConfigurationB.addAllowedDomain("https://second.com");
            mConfigurationB.removeAllowedDomain("https://second.com");
            mConfigurationB.addAllowedDomain("gibberish");
            allowedDomainsB.addAll(mConfigurationB.getAllowedDomains());

            mHelper.notifyCalled();
        });
        mHelper.waitForCallback(0, 1, 10, TimeUnit.SECONDS);
        // We expect to see a sorted collection of domains (not URLs) without duplicates.
        ArrayList<String> expectedAllowedDomainsA = new ArrayList<String>();
        expectedAllowedDomainsA.add("domain.com");
        expectedAllowedDomainsA.add("foobar.com");
        Assert.assertEquals(expectedAllowedDomainsA, allowedDomainsA);

        // We expect not to see second.com because it was removed after being added.
        ArrayList<String> expectedAllowedDomainsB = new ArrayList<String>();
        expectedAllowedDomainsB.add("scheme.com");
        expectedAllowedDomainsB.add("www.gibberish.com");
        Assert.assertEquals(expectedAllowedDomainsB, allowedDomainsB);
    }

    @Test
    @IntegrationTest
    @LargeTest
    @Feature({"adblock"})
    public void addingCustomFilters() throws Exception {
        final List<String> customFiltersA = new ArrayList<>();
        final List<String> customFiltersB = new ArrayList<>();
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            mConfigurationA.addCustomFilter("foobar.com");
            mConfigurationA.addCustomFilter("foobar.com");
            mConfigurationA.addCustomFilter("abc");
            customFiltersA.addAll(mConfigurationA.getCustomFilters());

            mConfigurationB.addCustomFilter("https://scheme.com/path.html");
            mConfigurationB.addCustomFilter("https://second.com");
            mConfigurationB.removeCustomFilter("https://second.com");
            customFiltersB.addAll(mConfigurationB.getCustomFilters());

            mHelper.notifyCalled();
        });
        mHelper.waitForCallback(0, 1, 10, TimeUnit.SECONDS);
        // We expect to see a collection of custom filters without duplicates.
        // The order represents order of addition.
        ArrayList<String> expectedCustomFiltersA = new ArrayList<String>();
        expectedCustomFiltersA.add("foobar.com");
        expectedCustomFiltersA.add("abc");
        Assert.assertEquals(expectedCustomFiltersA, customFiltersA);

        // We expect not to see https://second.com because it was removed after being added.
        ArrayList<String> expectedCustomFiltersB = new ArrayList<String>();
        expectedCustomFiltersB.add("https://scheme.com/path.html");
        Assert.assertEquals(expectedCustomFiltersB, customFiltersB);
    }

    @Test
    @IntegrationTest
    @LargeTest
    @Feature({"adblock"})
    public void addingFilterLists() throws Exception {
        final URL filterList1 = new URL("http://filters.com/list1.txt");
        final URL filterList2 = new URL("http://filters.com/list2.txt");
        final List<URL> filterListsA = new ArrayList<URL>();
        final List<URL> filterListsB = new ArrayList<URL>();
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            mConfigurationA.addFilterList(filterList1);
            mConfigurationA.addFilterList(filterList2);
            mConfigurationA.addFilterList(filterList1);
            filterListsA.addAll(mConfigurationA.getFilterLists());

            mConfigurationB.addFilterList(filterList1);
            mConfigurationB.addFilterList(filterList2);
            mConfigurationB.removeFilterList(filterList1);
            filterListsB.addAll(mConfigurationB.getFilterLists());

            mHelper.notifyCalled();
        });
        mHelper.waitForCallback(0, 1, 10, TimeUnit.SECONDS);
        ArrayList<URL> expectedFilterListsA = new ArrayList<URL>();
        expectedFilterListsA.add(filterList1);
        expectedFilterListsA.add(filterList2);
        Assert.assertEquals(expectedFilterListsA, filterListsA);

        ArrayList<URL> expectedFilterListsB = new ArrayList<URL>();
        expectedFilterListsB.add(filterList2);
        Assert.assertEquals(expectedFilterListsB, filterListsB);
    }

    @Test
    @IntegrationTest
    @LargeTest
    @Feature({"adblock"})
    public void aliasedConfigurations() throws Exception {
        final URL filterList1 = new URL("http://filters.com/list1.txt");
        final URL filterList2 = new URL("http://filters.com/list2.txt");
        final List<URL> filterListsA = new ArrayList<URL>();
        final List<URL> filterListsB = new ArrayList<URL>();
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            // Create a new FilteringConfiguration with a name of one that
            // already exist.
            FilteringConfiguration aliasedConfiguration = new FilteringConfiguration("a");

            // We add filter lists only to the original configuration instance.
            mConfigurationA.addFilterList(filterList1);
            mConfigurationA.addFilterList(filterList2);

            // We check what filter lists are present in the original and in the aliased instance.
            filterListsA.addAll(mConfigurationA.getFilterLists());
            filterListsB.addAll(aliasedConfiguration.getFilterLists());

            mHelper.notifyCalled();
        });
        mHelper.waitForCallback(0, 1, 10, TimeUnit.SECONDS);
        ArrayList<URL> expectedFilterLists = new ArrayList<URL>();
        expectedFilterLists.add(filterList1);
        expectedFilterLists.add(filterList2);
        Assert.assertEquals(expectedFilterLists, filterListsA);
        Assert.assertEquals(expectedFilterLists, filterListsB);
    }

    @Test
    @IntegrationTest
    @LargeTest
    @Feature({"adblock"})
    public void configurationChangeObserverNotified() throws Exception {
        final URL filterList1 = new URL("http://filters.com/list1.txt");
        final TestConfigurationChangeObserver observer = new TestConfigurationChangeObserver();
        Assert.assertFalse(observer.mOnEnabledStateChangedCalled);
        Assert.assertFalse(observer.mOnAllowedDomainsChanged);
        Assert.assertFalse(observer.mOnCustomFiltersChanged);
        Assert.assertFalse(observer.mOnFilterListsChanged);

        TestThreadUtils.runOnUiThreadBlocking(() -> {
            // We'll create an aliased instance which will receive notifications triggered by
            // changes made to the original instance.
            FilteringConfiguration aliasedConfiguration = new FilteringConfiguration("a");
            aliasedConfiguration.addObserver(observer);

            mConfigurationA.addFilterList(filterList1);
            mConfigurationA.addAllowedDomain("test.com");
            mConfigurationA.addCustomFilter("test.com");
            mConfigurationA.setEnabled(false);

            mHelper.notifyCalled();
        });
        mHelper.waitForCallback(0, 1, 10, TimeUnit.SECONDS);
        Assert.assertTrue(observer.mOnEnabledStateChangedCalled);
        Assert.assertTrue(observer.mOnAllowedDomainsChanged);
        Assert.assertTrue(observer.mOnCustomFiltersChanged);
        Assert.assertTrue(observer.mOnFilterListsChanged);
    }

    @Test
    @IntegrationTest
    @LargeTest
    @Feature({"adblock"})
    public void resourceBlockedByFilter() throws Exception {
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            mConfigurationA.addCustomFilter("resource.png");
            mHelper.notifyCalled();
        });
        mHelper.waitForCallback(0, 1, 10, TimeUnit.SECONDS);
        loadTestUrl();
        expectResourceBlocked();
    }

    @Test
    @IntegrationTest
    @LargeTest
    @Feature({"adblock"})
    public void resourceAllowedByFilter() throws Exception {
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            mConfigurationA.addCustomFilter("resource.png");
            // Allowing filter for the mocked test.org domain that mTestServer hosts.
            mConfigurationA.addCustomFilter("@@test.org$document");
            mHelper.notifyCalled();
        });
        mHelper.waitForCallback(0, 1, 10, TimeUnit.SECONDS);
        loadTestUrl();
        expectResourceShown();
    }

    @Test
    @IntegrationTest
    @LargeTest
    @Feature({"adblock"})
    public void resourceAllowedByAllowedDomain() throws Exception {
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            mConfigurationA.addCustomFilter("resource.png");
            mConfigurationA.addAllowedDomain("test.org");
            mHelper.notifyCalled();
        });
        mHelper.waitForCallback(0, 1, 10, TimeUnit.SECONDS);
        loadTestUrl();
        expectResourceShown();
    }

    @Test
    @IntegrationTest
    @LargeTest
    @Feature({"adblock"})
    public void resourceBlockedBySecondConfiguration() throws Exception {
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            // ConfigurationA allows the resource.
            // Allowing rules take precedence within a FilteringConfiguration.
            mConfigurationA.addCustomFilter("resource.png");
            mConfigurationA.addAllowedDomain("test.org");
            // But ConfigurationB blocks the resource.
            // Blocking takes precedence across FilteringConfigurations.
            mConfigurationB.addCustomFilter("resource.png");
            mHelper.notifyCalled();
        });
        mHelper.waitForCallback(0, 1, 10, TimeUnit.SECONDS);
        loadTestUrl();
        expectResourceBlocked();
    }

    @Test
    @IntegrationTest
    @LargeTest
    @Feature({"adblock"})
    public void noBlockingWithoutFilters() throws Exception {
        loadTestUrl();
        expectResourceShown();
    }
}
