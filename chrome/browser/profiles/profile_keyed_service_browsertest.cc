// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This source code is a part of eyeo Chromium SDK.
// Use of this source code is governed by the GPLv3 that can be found in the
// components/adblock/LICENSE file.

#include <sstream>

#include "base/test/scoped_feature_list.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/enterprise/connectors/connectors_service.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/common/chrome_features.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/profile_waiter.h"
#include "components/breadcrumbs/core/features.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/keyed_service/core/dependency_graph.h"
#include "components/keyed_service/core/keyed_service_base_factory.h"
#include "content/public/test/browser_test.h"
#include "third_party/blink/public/common/features.h"

// Ash doesn't support System Profile.
#if !BUILDFLAG(IS_CHROMEOS_ASH)
namespace {

// Constructs the Original System Profile for testing.
Profile* GetSystemOriginalProfile() {
  ProfileManager* profile_manager = g_browser_process->profile_manager();
  ProfileWaiter profile_waiter;
  profile_manager->CreateProfileAsync(ProfileManager::GetSystemProfilePath(),
                                      {});
  Profile* system_profile = profile_waiter.WaitForProfileAdded();
  content::RunAllTasksUntilIdle();
  return system_profile;
}

// Gets all the KeyedServices from the DependencyGraph.
std::vector<KeyedServiceBaseFactory*> GetKeyedServiceBaseFactories() {
  BrowserContextDependencyManager* dependency_manager =
      BrowserContextDependencyManager::GetInstance();
  DependencyGraph& dependency_graph =
      dependency_manager->GetDependencyGraphForTesting();
  std::vector<DependencyNode*> nodes;
  bool success = dependency_graph.GetConstructionOrder(&nodes);
  DCHECK(success);

  std::vector<KeyedServiceBaseFactory*> keyedServiceFactories;
  keyedServiceFactories.reserve(nodes.size());
  std::transform(nodes.begin(), nodes.end(),
                 std::back_inserter(keyedServiceFactories),
                 [](DependencyNode* node) {
                   return static_cast<KeyedServiceBaseFactory*>(node);
                 });
  return keyedServiceFactories;
}

std::string GetDifferenceString(const std::set<std::string>& set1,
                                const std::set<std::string>& set2) {
  std::vector<std::string> differences;
  std::set_difference(set1.begin(), set1.end(), set2.begin(), set2.end(),
                      std::back_inserter(differences));

  return differences.empty() ? "None" : base::JoinString(differences, ", ");
}

// Helper function to properly display differences between expected and reached
// service names.
std::string DisplaySetDifference(
    const std::set<std::string>& expected_active_services_names,
    const std::set<std::string>& active_services_names) {
  std::stringstream error;
  error << "Differences between expected and reached services:" << std::endl;

  error << "-- Missing Expected Services:" << std::endl;
  error << GetDifferenceString(active_services_names,
                               expected_active_services_names)
        << std::endl;

  error << "-- Added Extra Services:" << std::endl;
  error << GetDifferenceString(expected_active_services_names,
                               active_services_names)
        << std::endl;

  return error.str();
}

// The test comparing expected vs reached keyed services for the given profile.
void TestKeyedProfileServicesActives(
    Profile* profile,
    const std::set<std::string>& expected_active_services_names) {
  static const std::vector<KeyedServiceBaseFactory*> keyedServiceFactories =
      GetKeyedServiceBaseFactories();

  std::set<std::string> active_services_names;
  for (KeyedServiceBaseFactory* factory : keyedServiceFactories) {
    if (factory->IsServiceCreated(profile)) {
      active_services_names.emplace(factory->name());
    }
  }

  EXPECT_EQ(active_services_names, expected_active_services_names)
      << DisplaySetDifference(expected_active_services_names,
                              active_services_names);
}

}  // namespace

// If you are adding a new keyed service and this test fails:
// - determine if your service is intended to be created for the System profile
// - if yes, add it to the list of allowed services
// - if not, update your factory class so that the service is not created for
// the system profile.
//
// Note: if your service should not be used on the system profile, but still has
// to, because other services depend on it, add a comment explaining why.
// Example:
//   // FooService is required because BarService depends on it.
//   // TODO(crbug.com/12345): Stop creating BarService for the system profile.
class ProfileKeyedServiceBrowserTest : public InProcessBrowserTest {
 public:
  ProfileKeyedServiceBrowserTest() {
    // Force features activation to make sure the test is accurate as possible.
    // Also removes differences between official and non official run of the
    // tests. If a feature is integrated in the fieldtrial_testing_config.json,
    // it might not be considered under an official build. Adding it under a
    // InitWithFeatures to activate it would neglect that difference.
    feature_list_.InitWithFeatures(
        {
#if !BUILDFLAG(IS_ANDROID)
          features::kTrustSafetySentimentSurvey,
#endif  // !BUILDFLAG(IS_ANDROID)
              breadcrumbs::kLogBreadcrumbs, blink::features::kBrowsingTopics
        },
        {});
  }

 private:
  base::test::ScopedFeatureList feature_list_;
};

IN_PROC_BROWSER_TEST_F(ProfileKeyedServiceBrowserTest,
                       SystemProfileOTR_NeededServices) {
  // clang-format off
  std::set<std::string> system_otr_active_services {
#if BUILDFLAG(IS_CHROMEOS_LACROS)
    "CleanupManagerLacros",
    "DownloadCoreService",
#endif // BUILDFLAG(IS_CHROMEOS_LACROS)
    "AlarmManager",
    "BackgroundContentsService",
    "BackgroundSyncService",
    "BluetoothApiSocketManager",
    "BluetoothSocketEventDispatcher",
    "BrowsingDataLifetimeManager",
    "CookieSettings",
    "ExtensionSystem",
    "ExtensionURLLoaderFactory::BrowserContextShutdownNotifierFactory",
    "FeedbackPrivateAPI",
    "FileSystemAccessPermissionContext",
    "GeneratedPrefs",
    "HeavyAdService",
    "HidDeviceManager",
    "HostContentSettingsMap",
    "LastTabStandingTrackerKeyedService",
    "MediaRouterUIService",
    "NotificationDisplayService",
    "OptimizationGuideKeyedService",
    "PlatformNotificationService",
    "PrefWatcher",
    "PrivacySandboxSettings",
    "ProcessManager",
    "ProfileNetworkContextService",
    "RendererUpdater",
    "ResumableTCPServerSocketManager",
    "ResumableTCPSocketManager",
    "ResumableUDPSocketManager",
    "RulesRegistryService",
    "SerialConnectionManager",
    "SettingsPrivateEventRouter",
    "SiteDataCacheFacadeFactory",
    "SiteEngagementService",
    "SocketManager",
    "StorageNotificationService",
    "TCPServerSocketEventDispatcher",
    "TCPSocketEventDispatcher",
    "TabGroupsEventRouter",
    "ToolbarActionsModel",
    "UDPSocketEventDispatcher",
    "UkmBackgroundRecorderService",
    "UsbDeviceManager",
    "UsbDeviceResourceManager",
    "sct_reporting::Factory"
  };
  // clang-format on

  Profile* system_profile = GetSystemOriginalProfile();
  ASSERT_TRUE(system_profile->HasAnyOffTheRecordProfile());
  std::vector<Profile*> otr_profiles =
      system_profile->GetAllOffTheRecordProfiles();
  ASSERT_EQ(otr_profiles.size(), 1u);

  Profile* system_profile_otr = otr_profiles[0];
  ASSERT_TRUE(system_profile_otr->IsOffTheRecord());
  ASSERT_TRUE(system_profile_otr->IsSystemProfile());
  TestKeyedProfileServicesActives(system_profile_otr,
                                  system_otr_active_services);
}

IN_PROC_BROWSER_TEST_F(ProfileKeyedServiceBrowserTest,
                       SystemProfileParent_NeededServices) {
  // clang-format off
  std::set<std::string> system_active_services {
#if BUILDFLAG(IS_CHROMEOS_LACROS)
    "ChildAccountService",
    "CleanupManagerLacros",
    "ClipboardAPI",
    "DownloadCoreService",
    "ExternalLogoutRequestEventHandler",
    "ManualTestHeartbeatEvent",
    "SessionStateChangedEventDispatcher",
    "SupervisedUserService",
#else // !BUILDFLAG(IS_CHROMEOS_LACROS)
    "DownloadCoreService",
    "SystemIndicatorManager",
#endif
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_WIN)
    "SpellcheckService",
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_WIN)
    "AboutSigninInternals",
    "AboutThisSiteServiceFactory",
    "AccessibilityLabelsService",
    "AccountInvestigator",
    "AccountPasswordStore",
    "AccountReconcilor",
    "ActivityLog",
    "ActivityLogPrivateAPI",
    "AdaptiveQuietNotificationPermissionUiEnabler",
    "AdvancedProtectionStatusManager",
    "AffiliationService",
    "AlarmManager",
    "AnnouncementNotificationService",
    "AppLifetimeMonitor",
    "AppLoadService",
    "AppRestoreService",
    "AppShortcutManager",
    "AppTerminationObserver",
    "AppWindowRegistry",
    "AudioAPI",
    "AutofillImageFetcher",
    "AutofillPrivateEventRouter",
    "AutofillStrikeDatabase",
    "BackgroundContentsService",
    "BackgroundFetchService",
    "BackgroundSyncService",
    "Blocklist",
    "BluetoothAPI",
    "BluetoothApiSocketManager",
    "BluetoothLowEnergyAPI",
    "BluetoothPrivateAPI",
    "BluetoothSocketEventDispatcher",
    "BookmarkManagerPrivateAPI",
    "BookmarkModel",
    "BookmarkSyncServiceFactory",
    "BookmarkUndoService",
    "BookmarksAPI",
    "BookmarksApiWatcher",
    "BrailleDisplayPrivateAPI",
    "BreadcrumbManagerService",
    "BrowsingDataHistoryObserverService",
    "BrowsingDataLifetimeManager",
    "BrowsingDataRemover",
    "BrowsingTopicsService",
    "ChromeSigninClient",
    "CommandService",
    "ConsentAuditor",
    "ContentIndexProvider",
    "ContentSettingsService",
    "CookieSettings",
    "CookiesAPI",
    "CredentialsCleanerRunner",
    "DeveloperPrivateAPI",
    "DeviceInfoSyncService",
    "EventRouter",
    "ExtensionActionAPI",
    "ExtensionActionManager",
    "ExtensionCommandsGlobalRegistry",
    "ExtensionGCMAppHandler",
    "ExtensionGarbageCollector",
    "ExtensionHostRegistry",
    "ExtensionManagement",
    "ExtensionPrefValueMap",
    "ExtensionPrefs",
    "ExtensionRegistry",
    "ExtensionSyncService",
    "ExtensionSystem",
    "ExtensionSystemShared",
    "ExtensionURLLoaderFactory::BrowserContextShutdownNotifierFactory",
    "ExtensionWebUIOverrideRegistrar",
    "FaviconService",
    "FeedbackPrivateAPI",
    "FileSystemAccessPermissionContext",
    "FontPrefChangeNotifier",
    "FontSettingsAPI",
    "GAIAInfoUpdateService",
    "GCMProfileService",
    "GeneratedPrefs",
    "HeavyAdService",
    "HidDeviceManager",
    "HistoryAPI",
    "HistoryService",
    "HostContentSettingsMap",
    "HttpEngagementKeyService",
    "IdentityAPI",
    "IdentityManager",
    "IdleManager",
    "InstallStageTracker",
    "InstallTracker",
    "InstallVerifier",
    "InstanceIDProfileService",
    "InvalidationService",
    "LanguageSettingsPrivateDelegate",
    "LastTabStandingTrackerKeyedService",
    "LazyBackgroundTaskQueue",
    "LiveCaptionController",
    "LoginUIServiceFactory",
    "MDnsAPI",
    "ManagedBookmarkService",
    "ManagedConfigurationAPI",
    "ManagementAPI",
    "MediaRouter",
    "MediaRouterUIService",
    "MenuManager",
    "ModelTypeStoreService",
    "NavigationPredictorKeyedService",
    "NetworkingPrivateEventRouter",
    "NotificationDisplayService",
    "NotifierStateTracker",
    "OmniboxAPI",
    "OptimizationGuideKeyedService",
    "PageContentAnnotationsService",
    "PasswordStore",
    "PasswordsPrivateEventRouter",
    "PermissionHelper",
    "PermissionsManager",
    "PermissionsUpdaterShutdownFactory",
    "PersonalDataManager",
    "PinnedTabService",
    "PlatformNotificationService",
    "PluginManager",
    "PluginPrefs",
    "PrefMetricsService",
    "PrefWatcher",
    "PreferenceAPI",
    "PrimaryAccountPolicyManager",
  #if BUILDFLAG(IS_CHROMEOS) && defined(USE_CUPS)
    "PrintingMetricsService",
  #endif // BUILDFLAG(IS_CHROMEOS) && defined(USE_CUPS)
    "PrivacyMetricsService",
    "PrivacySandboxService",
    "PrivacySandboxSettings",
    "ProcessManager",
    "ProcessMap",
    "ProcessesAPI",
    "ProfileNetworkContextService",
    "ProfileThemeUpdateServiceFactory",
    "ProtocolHandlerRegistry",
    "ReadingListModel",
    "RendererStartupHelper",
    "RendererUpdater",
    "ResumableTCPServerSocketManager",
    "ResumableTCPSocketManager",
    "ResumableUDPSocketManager",
    "RulesMonitorService",
    "RulesRegistryService",
    "RuntimeAPI",
    "SafeBrowsingMetricsCollector",
    "SafeBrowsingNetworkContextService",
    "SafeBrowsingTailoredSecurityService",
    "SecurityEventRecorder",
    "SendTabToSelfClientService",
    "SendTabToSelfSyncService",
    "SerialConnectionManager",
    "SessionDataService",
    "SessionProtoDBFactory",
    "SessionSyncService",
    "SessionsAPI",
    "SettingsOverridesAPI",
    "SettingsPrivateEventRouter",
    "SharingMessageBridge",
    "SharingService",
    "ShoppingService",
    "SidePanelService",
    "SigninErrorController",
    "SigninManager",
    "SigninProfileAttributesUpdater",
    "SiteDataCacheFacadeFactory",
    "SiteEngagementService",
    "SocketManager",
    "StorageFrontend",
    "StorageNotificationService",
    "SyncInvalidationsService",
    "SyncService",
    "SyncSessionsWebContentsRouter",
    "SystemInfoAPI",
    "TCPServerSocketEventDispatcher",
    "TCPSocketEventDispatcher",
    "TabGroupsEventRouter",
    "TabsWindowsAPI",
    "TemplateURLServiceFactory",
    "ThemeService",
    "ToolbarActionsModel",
    "TranslateRanker",
    "TriggeredProfileResetter",
    "TtsAPI",
    "UDPSocketEventDispatcher",
    "UkmBackgroundRecorderService",
    "UnifiedConsentService",
    "UsbDeviceManager",
    "UsbDeviceResourceManager",
    "UserCloudPolicyInvalidator",
    "UserEventService",
    "UserPolicySigninService",
    "WarningBadgeService",
    "WarningService",
    "WebAuthenticationProxyAPI",
    "WebDataService",
    "WebNavigationAPI",
    "WebRequestAPI",
    "WebRtcEventLogManagerKeyedService",
    "WebrtcAudioPrivateEventService",
    "feedback::FeedbackUploaderChrome",
    "sct_reporting::Factory",

    // eyeo Chromium SDK services:
    "AdblockPrivateAPI",
    "AdblockSubscriptionPersistentMetadata",
    "AdblockSubscriptionService",
    "AdblockSubscriptionUpdater",
    "AdblockTelemetryService",
    "ResourceClassificationRunner",
    "SessionStats",
    "SitekeyStorage",
  };
  // clang-format on

  Profile* system_profile = GetSystemOriginalProfile();
  ASSERT_FALSE(system_profile->IsOffTheRecord());
  ASSERT_TRUE(system_profile->IsSystemProfile());
  TestKeyedProfileServicesActives(system_profile, system_active_services);
}
<<<<<<< HEAD
#endif  // !BUILDFLAG(IS_CHROMEOS_ASH)
=======

IN_PROC_BROWSER_TEST_F(ProfileKeyedServiceBrowserTest,
                       GuestProfileOTR_NeededServices) {
  // clang-format off
  std::set<std::string> guest_otr_active_services {
#if BUILDFLAG(IS_CHROMEOS_LACROS)
    "CleanupManagerLacros",
    "DownloadCoreService",
#endif // BUILDFLAG(IS_CHROMEOS_LACROS)
    "AlarmManager",
    "BackgroundContentsService",
    "BackgroundSyncService",
    "BluetoothApiSocketManager",
    "BluetoothSocketEventDispatcher",
    "BrowsingDataLifetimeManager",
    "CookieSettings",
    "ExtensionSystem",
    "ExtensionURLLoaderFactory::BrowserContextShutdownNotifierFactory",
    "FeedbackPrivateAPI",
    "FileSystemAccessPermissionContext",
    "GeneratedPrefs",
    "HeavyAdService",
    "HidDeviceManager",
    "HostContentSettingsMap",
    "LastTabStandingTrackerKeyedService",
    "MediaRouterUIService",
    "NotificationDisplayService",
    "OptimizationGuideKeyedService",
    "PlatformNotificationService",
    "PrefWatcher",
    "PrivacySandboxSettings",
    "ProcessManager",
    "ProfileNetworkContextService",
    "RealtimeReportingClient",
    "RendererUpdater",
    "ResumableTCPServerSocketManager",
    "ResumableTCPSocketManager",
    "ResumableUDPSocketManager",
    "RulesRegistryService",
    "SafeBrowsingPrivateEventRouter",
    "SerialConnectionManager",
    "SettingsPrivateEventRouter",
    "SiteDataCacheFacadeFactory",
    "SiteEngagementService",
    "SocketManager",
    "StorageNotificationService",
    "TCPServerSocketEventDispatcher",
    "TCPSocketEventDispatcher",
    "TabGroupsEventRouter",
    "ToolbarActionsModel",
    "UDPSocketEventDispatcher",
    "UkmBackgroundRecorderService",
    "UsbDeviceManager",
    "UsbDeviceResourceManager",
    "sct_reporting::Factory"
  };
  // clang-format on

  Profile* guest_profile =
      CreateProfileAndWaitForAllTaks(ProfileManager::GetGuestProfilePath());
  ASSERT_TRUE(guest_profile->HasAnyOffTheRecordProfile());
  Profile* guest_profile_otr = guest_profile->GetPrimaryOTRProfile(false);
  ASSERT_TRUE(guest_profile_otr->IsOffTheRecord());
  ASSERT_TRUE(guest_profile_otr->IsGuestSession());
  TestKeyedProfileServicesActives(guest_profile_otr, guest_otr_active_services);
}

IN_PROC_BROWSER_TEST_F(ProfileKeyedServiceBrowserTest,
                       GuestProfileParent_NeededServices) {
  // clang-format off
  std::set<std::string> guest_active_services {
#if BUILDFLAG(IS_CHROMEOS_LACROS)
    "ChildAccountService",
    "CleanupManagerLacros",
    "ClipboardAPI",
    "ExternalLogoutRequestEventHandler",
    "ManualTestHeartbeatEvent",
    "SessionStateChangedEventDispatcher",
    "SupervisedUserService",
#else // !BUILDFLAG(IS_CHROMEOS_LACROS)
    "SystemIndicatorManager",
    "WebAppAdjustments",
    "WebAppProvider",
#endif
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_WIN)
    "SpellcheckService",
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_WIN)
    "AboutSigninInternals",
    "AboutThisSiteServiceFactory",
    "AccessibilityLabelsService",
    "AccountInvestigator",
    "AccountPasswordStore",
    "AccountReconcilor",
    "ActivityLog",
    "ActivityLogPrivateAPI",
    "AdaptiveQuietNotificationPermissionUiEnabler",
    "AdvancedProtectionStatusManager",
    "AffiliationService",
    "AlarmManager",
    "AnnouncementNotificationService",
    "AppLifetimeMonitor",
    "AppLoadService",
    "AppRestoreService",
    "AppServiceProxy",
    "AppShortcutManager",
    "AppTerminationObserver",
    "AppWindowRegistry",
    "AudioAPI",
    "AutofillImageFetcher",
    "AutofillPrivateEventRouter",
    "AutofillStrikeDatabase",
    "BackgroundContentsService",
    "BackgroundFetchService",
    "BackgroundSyncService",
    "Blocklist",
    "BluetoothAPI",
    "BluetoothApiSocketManager",
    "BluetoothLowEnergyAPI",
    "BluetoothPrivateAPI",
    "BluetoothSocketEventDispatcher",
    "BookmarkManagerPrivateAPI",
    "BookmarkModel",
    "BookmarkSyncServiceFactory",
    "BookmarkUndoService",
    "BookmarksAPI",
    "BookmarksApiWatcher",
    "BrailleDisplayPrivateAPI",
    "BreadcrumbManagerService",
    "BrowsingTopicsService",
    "ChromeSigninClient",
    "CommandService",
    "ConsentAuditor",
    "ContentIndexProvider",
    "ContentSettingsService",
    "CookieSettings",
    "CookiesAPI",
    "CredentialsCleanerRunner",
    "DeveloperPrivateAPI",
    "DeviceInfoSyncService",
    "DownloadCoreService",
    "EventRouter",
    "ExtensionActionAPI",
    "ExtensionActionManager",
    "ExtensionCommandsGlobalRegistry",
    "ExtensionGCMAppHandler",
    "ExtensionGarbageCollector",
    "ExtensionHostRegistry",
    "ExtensionManagement",
    "ExtensionPrefValueMap",
    "ExtensionPrefs",
    "ExtensionRegistry",
    "ExtensionSyncService",
    "ExtensionSystem",
    "ExtensionSystemShared",
    "ExtensionURLLoaderFactory::BrowserContextShutdownNotifierFactory",
    "ExtensionWebUIOverrideRegistrar",
    "FaviconService",
    "FeedbackPrivateAPI",
    "FileSystemAccessPermissionContext",
    "FontPrefChangeNotifier",
    "FontSettingsAPI",
    "GAIAInfoUpdateService",
    "GCMProfileService",
    "GeneratedPrefs",
    "HeavyAdService",
    "HidDeviceManager",
    "HistoryAPI",
    "HistoryService",
    "HostContentSettingsMap",
    "HttpEngagementKeyService",
    "IdentityAPI",
    "IdentityManager",
    "IdleManager",
    "InstallStageTracker",
    "InstallTracker",
    "InstallVerifier",
    "InstanceIDProfileService",
    "InvalidationService",
    "LanguageSettingsPrivateDelegate",
    "LastTabStandingTrackerKeyedService",
    "LazyBackgroundTaskQueue",
    "LiveCaptionController",
    "LoginUIServiceFactory",
    "MDnsAPI",
    "ManagedBookmarkService",
    "ManagedConfigurationAPI",
    "ManagementAPI",
    "MediaRouter",
    "MediaRouterUIService",
    "MenuManager",
    "ModelTypeStoreService",
    "NavigationPredictorKeyedService",
    "NetworkingPrivateEventRouter",
    "NotificationDisplayService",
    "NotifierStateTracker",
    "OmniboxAPI",
    "OptimizationGuideKeyedService",
    "PageContentAnnotationsService",
    "PasswordStore",
    "PasswordsPrivateEventRouter",
    "PermissionHelper",
    "PermissionsManager",
    "PermissionsUpdaterShutdownFactory",
    "PersonalDataManager",
    "PinnedTabService",
    "PlatformNotificationService",
    "PluginManager",
    "PluginPrefs",
    "PowerBookmarkService",
    "PrefMetricsService",
    "PrefWatcher",
    "PreferenceAPI",
    "PrimaryAccountPolicyManager",
  #if BUILDFLAG(IS_CHROMEOS) && defined(USE_CUPS)
    "PrintingMetricsService",
  #endif // BUILDFLAG(IS_CHROMEOS) && defined(USE_CUPS)
    "PrivacyMetricsService",
    "PrivacySandboxService",
    "PrivacySandboxSettings",
    "ProcessManager",
    "ProcessMap",
    "ProcessesAPI",
    "ProfileNetworkContextService",
    "ProfileThemeUpdateServiceFactory",
    "ProtocolHandlerRegistry",
    "ReadingListModel",
    "RealtimeReportingClient",
    "RendererStartupHelper",
    "RendererUpdater",
    "ResumableTCPServerSocketManager",
    "ResumableTCPSocketManager",
    "ResumableUDPSocketManager",
    "RulesMonitorService",
    "RulesRegistryService",
    "RuntimeAPI",
    "SafeBrowsingMetricsCollector",
    "SafeBrowsingNetworkContextService",
    "SafeBrowsingPrivateEventRouter",
    "SafeBrowsingTailoredSecurityService",
    "SecurityEventRecorder",
    "SendTabToSelfClientService",
    "SendTabToSelfSyncService",
    "SerialConnectionManager",
    "SessionDataService",
    "SessionProtoDBFactory",
    "SessionSyncService",
    "SessionsAPI",
    "SettingsOverridesAPI",
    "SettingsPrivateEventRouter",
    "SharingMessageBridge",
    "SharingService",
    "ShoppingService",
    "SidePanelService",
    "SigninErrorController",
    "SigninManager",
    "SigninProfileAttributesUpdater",
    "SiteDataCacheFacadeFactory",
    "SiteEngagementService",
    "SocketManager",
    "StorageFrontend",
    "StorageNotificationService",
    "SyncInvalidationsService",
    "SyncService",
    "SyncSessionsWebContentsRouter",
    "SystemInfoAPI",
    "TCPServerSocketEventDispatcher",
    "TCPSocketEventDispatcher",
    "TabGroupsEventRouter",
    "TabsWindowsAPI",
    "TemplateURLServiceFactory",
    "ThemeService",
    "ToolbarActionsModel",
    "TranslateRanker",
    "TriggeredProfileResetter",
    "TtsAPI",
    "UDPSocketEventDispatcher",
    "UkmBackgroundRecorderService",
    "UnifiedConsentService",
    "UsbDeviceManager",
    "UsbDeviceResourceManager",
    "UserCloudPolicyInvalidator",
    "UserEventService",
    "UserPolicySigninService",
    "WarningBadgeService",
    "WarningService",
    "WebAuthenticationProxyAPI",
    "WebDataService",
    "WebNavigationAPI",
    "WebRequestAPI",
    "WebRtcEventLogManagerKeyedService",
    "WebrtcAudioPrivateEventService",
    "feedback::FeedbackUploaderChrome",
    "sct_reporting::Factory",

     // eyeo Chromium SDK services:
    "AdblockPrivateAPI",
    "AdblockSubscriptionPersistentMetadata",
    "AdblockSubscriptionService",
    "AdblockSubscriptionUpdater",
    "AdblockTelemetryService",
    "ResourceClassificationRunner",
    "SessionStats",
    "SitekeyStorage",
  };
  // clang-format on

  Profile* guest_profile =
      CreateProfileAndWaitForAllTaks(ProfileManager::GetGuestProfilePath());
  ASSERT_FALSE(guest_profile->IsOffTheRecord());
  ASSERT_TRUE(guest_profile->IsGuestSession());
  TestKeyedProfileServicesActives(guest_profile, guest_active_services);
}
>>>>>>> 54b2a67d3c457... Squashed commits