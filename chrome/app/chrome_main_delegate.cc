// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This source code is a part of eyeo Chromium SDK.
// Use of this source code is governed by the GPLv3 that can be found in the
// components/adblock/LICENSE file.

#include "chrome/app/chrome_main_delegate.h"

#include <stddef.h>

#include <string>

#include "base/base_paths.h"
#include "base/bind.h"
#include "base/check.h"
#include "base/command_line.h"
#include "base/cpu.h"
#include "base/cpu_reduction_experiment.h"
#include "base/dcheck_is_on.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/i18n/rtl.h"
#include "base/immediate_crash.h"
#include "base/lazy_instance.h"
#include "base/metrics/histogram_macros.h"
#include "base/notreached.h"
#include "base/path_service.h"
#include "base/process/memory.h"
#include "base/process/process.h"
#include "base/process/process_handle.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/sequence_manager/sequence_manager_impl.h"
#include "base/task/sequence_manager/thread_controller_power_monitor.h"
#include "base/task/thread_pool/thread_pool_instance.h"
#include "base/threading/hang_watcher.h"
#include "base/threading/platform_thread.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "base/trace_event/trace_event_impl.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "chrome/browser/adblock/adblock_content_browser_client.h"
#include "chrome/browser/chrome_content_browser_client.h"
#include "chrome/browser/chrome_resource_bundle_helper.h"
#include "chrome/browser/defaults.h"
#include "chrome/browser/metrics/chrome_feature_list_creator.h"
#include "chrome/browser/startup_data.h"
#include "chrome/common/buildflags.h"
#include "chrome/common/channel_info.h"
#include "chrome/common/chrome_content_client.h"
#include "chrome/common/chrome_features.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/common/chrome_paths_internal.h"
#include "chrome/common/chrome_result_codes.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/crash_keys.h"
#include "chrome/common/logging_chrome.h"
#include "chrome/common/profiler/process_type.h"
#include "chrome/common/profiler/unwind_util.h"
#include "chrome/common/url_constants.h"
#include "chrome/gpu/chrome_content_gpu_client.h"
#include "chrome/renderer/chrome_content_renderer_client.h"
#include "chrome/utility/chrome_content_utility_client.h"
#include "components/component_updater/component_updater_paths.h"
#include "components/content_settings/core/common/content_settings_pattern.h"
#include "components/crash/core/app/crash_reporter_client.h"
#include "components/crash/core/common/crash_key.h"
#include "components/crash/core/common/crash_keys.h"
#include "components/gwp_asan/buildflags/buildflags.h"
#include "components/heap_profiling/in_process/heap_profiler_controller.h"
#include "components/nacl/common/buildflags.h"
#include "components/services/heap_profiling/public/cpp/profiling_client.h"
#include "components/startup_metric_utils/browser/startup_metric_utils.h"
#include "components/version_info/version_info.h"
#include "content/public/app/initialize_mojo_core.h"
#include "content/public/common/content_client.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_paths.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/public/common/profiling.h"
#include "content/public/common/url_constants.h"
#include "extensions/buildflags/buildflags.h"
#include "net/http/http_cache.h"
#include "net/url_request/url_request.h"
#include "pdf/buildflags.h"
#include "ppapi/buildflags/buildflags.h"
#include "printing/buildflags/buildflags.h"
#include "services/tracing/public/cpp/stack_sampling/tracing_sampler_profiler.h"
#include "third_party/abseil-cpp/absl/types/variant.h"
#include "third_party/blink/public/common/features.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_switches.h"

#if BUILDFLAG(IS_WIN)
#include <malloc.h>

#include <algorithm>

#include "base/files/important_file_writer_cleaner.h"
#include "base/threading/platform_thread_win.h"
#include "base/win/atl.h"
#include "chrome/child/v8_crashpad_support_win.h"
#include "chrome/chrome_elf/chrome_elf_main.h"
#include "chrome/common/child_process_logging.h"
#include "chrome/common/win/delay_load_failure_hook.h"
#include "chrome/install_static/install_util.h"
#include "components/browser_watcher/extended_crash_reporting.h"
#include "sandbox/win/src/sandbox.h"
#include "ui/base/resource/resource_bundle_win.h"
#endif

#if BUILDFLAG(IS_MAC)
#include "base/mac/foundation_util.h"
#include "base/message_loop/message_pump_mac.h"
#include "chrome/app/chrome_main_mac.h"
#include "chrome/browser/chrome_browser_application_mac.h"
#include "chrome/browser/mac/relauncher.h"
#include "chrome/browser/shell_integration.h"
#include "components/crash/core/common/objc_zombie.h"
#include "ui/base/l10n/l10n_util_mac.h"
#endif

#if BUILDFLAG(IS_POSIX)
#include <locale.h>
#include <signal.h>

#include "chrome/app/chrome_crash_reporter_client.h"
#include "components/about_ui/credit_utils.h"
#endif

#if BUILDFLAG(ENABLE_NACL) && (BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS))
#include "components/nacl/common/nacl_paths.h"
#include "components/nacl/zygote/nacl_fork_delegate_linux.h"
#endif

#if BUILDFLAG(IS_CHROMEOS)
#include "chromeos/dbus/constants/dbus_paths.h"
#endif

#if BUILDFLAG(IS_CHROMEOS_ASH)
#include "ash/constants/ash_paths.h"
#include "ash/constants/ash_switches.h"
#include "base/system/sys_info.h"
#include "chrome/browser/ash/boot_times_recorder.h"
#include "chrome/browser/ash/dbus/ash_dbus_helper.h"
#include "chrome/browser/ash/startup_settings_cache.h"
#include "chromeos/ash/components/memory/kstaled.h"
#include "chromeos/ash/components/memory/memory.h"
#include "chromeos/ash/components/memory/swap_configuration.h"
#include "chromeos/hugepage_text/hugepage_text.h"
#include "ui/lottie/resource.h"  // nogncheck
#endif

#if BUILDFLAG(IS_ANDROID)
#include "base/android/java_exception_reporter.h"
#include "base/android/library_loader/library_loader_hooks.h"
#include "chrome/browser/android/metrics/uma_session_stats.h"
#include "chrome/browser/flags/android/chrome_feature_list.h"
#include "chrome/common/chrome_descriptors.h"
#include "components/crash/android/pure_java_exception_handler.h"
#include "net/android/network_change_notifier_factory_android.h"
#else  // BUILDFLAG(IS_ANDROID)
// Diagnostics is only available on non-android platforms.
#include "chrome/browser/diagnostics/diagnostics_controller.h"
#include "chrome/browser/diagnostics/diagnostics_writer.h"
#endif

#if BUILDFLAG(IS_POSIX) && !BUILDFLAG(IS_MAC) && !BUILDFLAG(IS_ANDROID)
#include "components/crash/core/app/breakpad_linux.h"
#include "v8/include/v8-wasm-trap-handler-posix.h"
#include "v8/include/v8.h"
#endif

#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
#include "base/environment.h"
#endif

#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_ANDROID)
#include "base/message_loop/message_pump_libevent.h"
#endif

#if BUILDFLAG(IS_MAC) || BUILDFLAG(IS_WIN) || BUILDFLAG(IS_ANDROID) || \
    BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
#include "chrome/browser/policy/policy_path_parser.h"
#include "components/crash/core/app/crashpad.h"
#endif

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "chrome/browser/extensions/startup_helper.h"
#include "extensions/common/constants.h"
#endif

#if BUILDFLAG(ENABLE_NACL)
#include "components/nacl/common/nacl_switches.h"
#include "components/nacl/renderer/plugin/ppapi_entrypoints.h"
#endif

#if BUILDFLAG(ENABLE_PDF)
#include "chrome/child/pdf_child_init.h"
#endif

#if !BUILDFLAG(IS_ANDROID)
#include "chrome/browser/chrome_process_singleton.h"
#include "chrome/browser/process_singleton.h"
#endif  // !BUILDFLAG(IS_ANDROID)

#if BUILDFLAG(ENABLE_GWP_ASAN)
#include "components/gwp_asan/client/gwp_asan.h"  // nogncheck
#endif

#if BUILDFLAG(IS_CHROMEOS_LACROS)
#include "chrome/common/chrome_paths_lacros.h"
#include "chromeos/crosapi/cpp/crosapi_constants.h"  // nogncheck
#include "chromeos/crosapi/mojom/crosapi.mojom.h"    // nogncheck
#include "chromeos/lacros/dbus/lacros_dbus_helper.h"
#include "chromeos/lacros/lacros_paths.h"
#include "chromeos/lacros/lacros_service.h"
#include "chromeos/startup/browser_params_proxy.h"  // nogncheck
#include "media/base/media_switches.h"
#include "ui/base/resource/data_pack_with_resource_sharing_lacros.h"
#endif

base::LazyInstance<ChromeContentGpuClient>::DestructorAtExit
    g_chrome_content_gpu_client = LAZY_INSTANCE_INITIALIZER;
base::LazyInstance<ChromeContentRendererClient>::DestructorAtExit
    g_chrome_content_renderer_client = LAZY_INSTANCE_INITIALIZER;

extern int NaClMain(content::MainFunctionParams);

const char* const ChromeMainDelegate::kNonWildcardDomainNonPortSchemes[] = {
#if BUILDFLAG(ENABLE_EXTENSIONS)
    extensions::kExtensionScheme,
#endif
    chrome::kChromeSearchScheme, content::kChromeDevToolsScheme,
    content::kChromeUIScheme, content::kChromeUIUntrustedScheme};
const size_t ChromeMainDelegate::kNonWildcardDomainNonPortSchemesSize =
    std::size(kNonWildcardDomainNonPortSchemes);

namespace {

#if BUILDFLAG(IS_CHROMEOS_LACROS)
const base::FilePath::CharType kUserHomeDirPrefix[] =
    FILE_PATH_LITERAL("/home/user");
#endif  // BUILDFLAG(IS_CHROMEOS_LACROS)

#if BUILDFLAG(IS_WIN)
// Early versions of Chrome incorrectly registered a chromehtml: URL handler,
// which gives us nothing but trouble. Avoid launching chrome this way since
// some apps fail to properly escape arguments.
bool HasDeprecatedArguments(const std::wstring& command_line) {
  const wchar_t kChromeHtml[] = L"chromehtml:";
  std::wstring command_line_lower = base::ToLowerASCII(command_line);
  // We are only searching for ASCII characters so this is OK.
  return (command_line_lower.find(kChromeHtml) != std::wstring::npos);
}

// If we try to access a path that is not currently available, we want the call
// to fail rather than show an error dialog.
void SuppressWindowsErrorDialogs() {
  UINT new_flags = SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX;

  // Preserve existing error mode.
  UINT existing_flags = SetErrorMode(new_flags);
  SetErrorMode(existing_flags | new_flags);
}

bool IsSandboxedProcess() {
  typedef bool (*IsSandboxedProcessFunc)();
  IsSandboxedProcessFunc is_sandboxed_process_func =
      reinterpret_cast<IsSandboxedProcessFunc>(
          GetProcAddress(GetModuleHandle(NULL), "IsSandboxedProcess"));
  return is_sandboxed_process_func && is_sandboxed_process_func();
}

void SetUpExtendedCrashReporting(bool is_browser_process) {
  browser_watcher::ExtendedCrashReporting* extended_crash_reporting =
      browser_watcher::ExtendedCrashReporting::SetUpIfEnabled(
          is_browser_process
              ? browser_watcher::ExtendedCrashReporting::kBrowserProcess
              : browser_watcher::ExtendedCrashReporting::kOther);

  if (!extended_crash_reporting)
    return;

  // Record product, version, channel and special build strings.
  wchar_t exe_file[MAX_PATH] = {};
  CHECK(::GetModuleFileName(nullptr, exe_file, std::size(exe_file)));

  std::wstring product_name, version_number, channel_name, special_build;
  install_static::GetExecutableVersionDetails(
      exe_file, &product_name, &version_number, &special_build, &channel_name);

  extended_crash_reporting->SetProductStrings(
      base::WideToUTF16(product_name), base::WideToUTF16(version_number),
      base::WideToUTF16(channel_name), base::WideToUTF16(special_build));
}

#endif  // BUILDFLAG(IS_WIN)

#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
void AdjustLinuxOOMScore(const std::string& process_type) {
  int score = -1;

  if (process_type == switches::kPpapiPluginProcess) {
    score = content::kPluginOomScore;
  } else if (process_type == switches::kUtilityProcess ||
             process_type == switches::kGpuProcess) {
    score = content::kMiscOomScore;
#if BUILDFLAG(ENABLE_NACL)
  } else if (process_type == switches::kNaClLoaderProcess) {
    score = content::kPluginOomScore;
#endif
  } else if (process_type == switches::kZygoteProcess || process_type.empty()) {
    // For zygotes and unlabeled process types, we want to still make
    // them killable by the OOM killer.
    score = content::kZygoteOomScore;
  } else if (process_type == switches::kRendererProcess) {
    LOG(WARNING) << "process type 'renderer' "
                 << "should be created through the zygote.";
    // When debugging, this process type can end up being run directly, but
    // this isn't the typical path for assigning the OOM score for it.  Still,
    // we want to assign a score that is somewhat representative for debugging.
    score = content::kLowestRendererOomScore;
  } else {
    NOTREACHED() << "Unknown process type";
  }
  // In the case of a 0 score, still try to adjust it. Most likely the score is
  // 0 already, but it may not be if this process inherited a higher score from
  // its parent process.
  if (score > -1)
    base::AdjustOOMScore(base::GetCurrentProcId(), score);
}
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)

// Returns true if this subprocess type needs the ResourceBundle initialized
// and resources loaded.
bool SubprocessNeedsResourceBundle(const std::string& process_type) {
  return
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
      // The zygote process opens the resources for the renderers.
      process_type == switches::kZygoteProcess ||
#endif
#if BUILDFLAG(IS_MAC)
  // Mac needs them too for scrollbar related images and for sandbox
  // profiles.
#if BUILDFLAG(ENABLE_NACL)
      process_type == switches::kNaClLoaderProcess ||
#endif
      process_type == switches::kGpuProcess ||
#endif
      process_type == switches::kPpapiPluginProcess ||
      process_type == switches::kRendererProcess ||
      process_type == switches::kUtilityProcess;
}

#if BUILDFLAG(IS_POSIX)
bool HandleCreditsSwitch(const base::CommandLine& command_line) {
  if (!command_line.HasSwitch(switches::kCredits))
    return false;

  // Load resources: about_credits.html is in component_resources.pak that is
  // re-packed into resources.pak.
  base::FilePath resource_dir;
  bool result = base::PathService::Get(base::DIR_ASSETS, &resource_dir);
  DCHECK(result);

  const std::string locale =
      command_line.GetSwitchValueASCII(::switches::kLang);
  ui::ResourceBundle::InitSharedInstanceWithLocale(
      locale, /**parameter_name=*/nullptr,
      ui::ResourceBundle::DO_NOT_LOAD_COMMON_RESOURCES);
  base::FilePath resources_pak =
      resource_dir.Append(FILE_PATH_LITERAL("resources.pak"));

#if BUILDFLAG(IS_MAC) && !defined(COMPONENT_BUILD)
  // In non-component builds, check if a fallback in Resources/ folder is
  // available.
  if (!base::PathExists(resources_pak)) {
    resources_pak =
        resource_dir.Append(FILE_PATH_LITERAL("Resources/resources.pak"));
  }
#endif

  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      resources_pak, ui::kScaleFactorNone);

  auto credits = about_ui::GetCredits(/**include_scripts=*/false);
  // If resources failed to load, about_ui::GetCredits returns
  // a malformed HTML doc containing `</body>\n</html>`.
  // When the resources loaded successfully, we get a huge document
  // (~8 MiB) instead.
  // We use a threshold of 100 characters to see if the resources
  // were loaded successfully.
  size_t resource_loading_threshold = 100;
  if (credits.size() < resource_loading_threshold)
    printf("%s\n", "Failed to load credits.");
  else
    printf("%s\n", credits.c_str());

  return true;
}

// Check for --version and --product-version; return true if we encountered
// one of these switches and should exit now.
bool HandleVersionSwitches(const base::CommandLine& command_line) {
#if !BUILDFLAG(IS_MAC)
  if (command_line.HasSwitch(switches::kProductVersion)) {
    printf("%s\n", version_info::GetVersionNumber().c_str());
    return true;
  }
#endif

  if (command_line.HasSwitch(switches::kVersion)) {
    printf("%s %s %s\n", version_info::GetProductName().c_str(),
           version_info::GetVersionNumber().c_str(),
           chrome::GetChannelName(chrome::WithExtendedStable(true)).c_str());
    return true;
  }

  return false;
}

// TODO(crbug.com/1052397): Revisit the macro expression once build flag switch
// of lacros-chrome is complete.
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS_LACROS)
// Show the man page if --help or -h is on the command line.
void HandleHelpSwitches(const base::CommandLine& command_line) {
  if (command_line.HasSwitch(switches::kHelp) ||
      command_line.HasSwitch(switches::kHelpShort)) {
    base::FilePath binary(command_line.argv()[0]);
    execlp("man", "man", binary.BaseName().value().c_str(), NULL);
    PLOG(FATAL) << "execlp failed";
  }
}
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS_LACROS)

#if !BUILDFLAG(IS_MAC) && !BUILDFLAG(IS_ANDROID)
void SIGTERMProfilingShutdown(int signal) {
  content::Profiling::Stop();
  struct sigaction sigact;
  memset(&sigact, 0, sizeof(sigact));
  sigact.sa_handler = SIG_DFL;
  CHECK_EQ(sigaction(SIGTERM, &sigact, nullptr), 0);
  raise(signal);
}

void SetUpProfilingShutdownHandler() {
  struct sigaction sigact;
  sigact.sa_handler = SIGTERMProfilingShutdown;
  sigact.sa_flags = SA_RESETHAND;
  sigemptyset(&sigact.sa_mask);
  CHECK_EQ(sigaction(SIGTERM, &sigact, nullptr), 0);
}
#endif  // !BUILDFLAG(IS_MAC) && !BUILDFLAG(IS_ANDROID)

#endif  // BUILDFLAG(IS_POSIX)

#if BUILDFLAG(ENABLE_EXTENSIONS)
absl::optional<int> HandlePackExtensionSwitches(
    const base::CommandLine& command_line) {
  // If the command line specifies --pack-extension, attempt the pack extension
  // startup action and exit.
  if (!command_line.HasSwitch(switches::kPackExtension))
    return absl::nullopt;

  const std::string locale =
      command_line.GetSwitchValueASCII(::switches::kLang);
  ui::ResourceBundle::InitSharedInstanceWithLocale(
      locale, /*delegate=*/nullptr,
      ui::ResourceBundle::DO_NOT_LOAD_COMMON_RESOURCES);

  extensions::StartupHelper extension_startup_helper;
  std::string error_message;
  if (!extension_startup_helper.PackExtension(command_line, &error_message)) {
    if (!error_message.empty()) {
      LOG(ERROR) << error_message.c_str();
    }
    return chrome::RESULT_CODE_PACK_EXTENSION_ERROR;
  }

  return chrome::RESULT_CODE_NORMAL_EXIT_PACK_EXTENSION_SUCCESS;
}
#endif  // !BUILDFLAG(ENABLE_EXTENSIONS)

struct MainFunction {
  const char* name;
  int (*function)(content::MainFunctionParams);
};

// Initializes the user data dir. Must be called before InitializeLocalState().
void InitializeUserDataDir(base::CommandLine* command_line) {
#if BUILDFLAG(IS_WIN)
  // Reach out to chrome_elf for the truth on the user data directory.
  // Note that in tests, this links to chrome_elf_test_stubs.
  wchar_t user_data_dir_buf[MAX_PATH], invalid_user_data_dir_buf[MAX_PATH];

  // In tests this may return false, implying the user data dir should be unset.
  if (GetUserDataDirectoryThunk(user_data_dir_buf, std::size(user_data_dir_buf),
                                invalid_user_data_dir_buf,
                                std::size(invalid_user_data_dir_buf))) {
    base::FilePath user_data_dir(user_data_dir_buf);
    if (invalid_user_data_dir_buf[0] != 0) {
      chrome::SetInvalidSpecifiedUserDataDir(
          base::FilePath(invalid_user_data_dir_buf));
      command_line->AppendSwitchPath(switches::kUserDataDir, user_data_dir);
    }
    CHECK(base::PathService::OverrideAndCreateIfNeeded(
        chrome::DIR_USER_DATA, user_data_dir, false, true));
  }
#else  // BUILDFLAG(IS_WIN)
  base::FilePath user_data_dir =
      command_line->GetSwitchValuePath(switches::kUserDataDir);
  std::string process_type =
      command_line->GetSwitchValueASCII(switches::kProcessType);

#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
  // On Linux, Chrome does not support running multiple copies under different
  // DISPLAYs, so the profile directory can be specified in the environment to
  // support the virtual desktop use-case.
  if (user_data_dir.empty()) {
    std::string user_data_dir_string;
    std::unique_ptr<base::Environment> environment(base::Environment::Create());
    if (environment->GetVar("CHROME_USER_DATA_DIR", &user_data_dir_string) &&
        base::IsStringUTF8(user_data_dir_string)) {
      user_data_dir = base::FilePath::FromUTF8Unsafe(user_data_dir_string);
    }
  }
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
#if BUILDFLAG(IS_MAC)
  policy::path_parser::CheckUserDataDirPolicy(&user_data_dir);
#endif  // BUILDFLAG(IS_MAC)

  const bool specified_directory_was_invalid =
      !user_data_dir.empty() &&
      !base::PathService::OverrideAndCreateIfNeeded(chrome::DIR_USER_DATA,
                                                    user_data_dir, false, true);
  // Save inaccessible or invalid paths so the user may be prompted later.
  if (specified_directory_was_invalid)
    chrome::SetInvalidSpecifiedUserDataDir(user_data_dir);

  // Warn and fail early if the process fails to get a user data directory.
  if (!base::PathService::Get(chrome::DIR_USER_DATA, &user_data_dir)) {
    // If an invalid command-line or policy override was specified, the user
    // will be given an error with that value. Otherwise, use the directory
    // returned by PathService (or the fallback default directory) in the error.
    if (!specified_directory_was_invalid) {
      // base::PathService::Get() returns false and yields an empty path if it
      // fails to create DIR_USER_DATA. Retrieve the default value manually to
      // display a more meaningful error to the user in that case.
      if (user_data_dir.empty())
        chrome::GetDefaultUserDataDirectory(&user_data_dir);
      chrome::SetInvalidSpecifiedUserDataDir(user_data_dir);
    }

    // The browser process (which is identified by an empty |process_type|) will
    // handle the error later; other processes that need the dir crash here.
    CHECK(process_type.empty()) << "Unable to get the user data directory "
                                << "for process type: " << process_type;
  }

  // Append the fallback user data directory to the commandline. Otherwise,
  // child or service processes will attempt to use the invalid directory.
  if (specified_directory_was_invalid)
    command_line->AppendSwitchPath(switches::kUserDataDir, user_data_dir);
#endif  // BUILDFLAG(IS_WIN)
}

#if !BUILDFLAG(IS_ANDROID)
void InitLogging(const std::string& process_type) {
  logging::OldFileDeletionState file_state = logging::APPEND_TO_OLD_LOG_FILE;
  if (process_type.empty()) {
    file_state = logging::DELETE_OLD_LOG_FILE;
  }
  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();
  logging::InitChromeLogging(command_line, file_state);
}
#endif

void RecordMainStartupMetrics(base::TimeTicks application_start_time) {
  const base::TimeTicks now = base::TimeTicks::Now();

#if BUILDFLAG(IS_WIN)
  DCHECK(!application_start_time.is_null());
  startup_metric_utils::RecordApplicationStartTime(application_start_time);
#elif BUILDFLAG(IS_ANDROID)
  // On Android the main entry point time is the time when the Java code starts.
  // This happens before the shared library containing this code is even loaded.
  // The Java startup code has recorded that time, but the C++ code can't fetch
  // it from the Java side until it has initialized the JNI. See
  // ChromeMainDelegateAndroid.
#else
  // On other platforms, |application_start_time| == |now| since the application
  // starts with ChromeMain().
  startup_metric_utils::RecordApplicationStartTime(now);
#endif

#if BUILDFLAG(IS_MAC) || BUILDFLAG(IS_WIN) || BUILDFLAG(IS_LINUX) || \
    BUILDFLAG(IS_CHROMEOS)
  // Record the startup process creation time on supported platforms. On Android
  // this is recorded in ChromeMainDelegateAndroid.
  startup_metric_utils::RecordStartupProcessCreationTime(
      base::Process::Current().CreationTime());
#endif

  startup_metric_utils::RecordChromeMainEntryTime(now);
}

}  // namespace

ChromeMainDelegate::ChromeMainDelegate()
    : ChromeMainDelegate(base::TimeTicks()) {}

ChromeMainDelegate::ChromeMainDelegate(base::TimeTicks exe_entry_point_ticks) {
  // Record startup metrics in the browser process. For component builds, there
  // is no way to know the type of process (process command line is not yet
  // initialized), so the function below will also be called in renderers.
  // This doesn't matter as it simply sets global variables.
  RecordMainStartupMetrics(exe_entry_point_ticks);
}

ChromeMainDelegate::~ChromeMainDelegate() = default;

absl::optional<int> ChromeMainDelegate::PostEarlyInitialization(
    InvokedIn invoked_in) {
  DCHECK(base::ThreadPoolInstance::Get());
  const auto* invoked_in_browser =
      absl::get_if<InvokedInBrowserProcess>(&invoked_in);
  if (!invoked_in_browser) {
    CommonEarlyInitialization();
    return absl::nullopt;
  }

#if !BUILDFLAG(IS_ANDROID)
  // Configure the early process singleton experiment.
  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();
  ChromeProcessSingleton::SetupEarlySingletonFeature(command_line);

  if (ChromeProcessSingleton::IsEarlySingletonFeatureEnabled()) {
    // Take the Chrome process singleton lock. The process can become the
    // Browser process if it succeed to take the lock. Otherwise, the
    // command-line is sent to the actual Browser process and the current
    // process can be exited.
    base::FilePath user_data_dir;
    if (!base::PathService::Get(chrome::DIR_USER_DATA, &user_data_dir)) {
      return chrome::RESULT_CODE_MISSING_DATA;
    }

    ChromeProcessSingleton::CreateInstance(user_data_dir);

    ProcessSingleton::NotifyResult notify_result =
        ChromeProcessSingleton::GetInstance()->NotifyOtherProcessOrCreate();
    UMA_HISTOGRAM_ENUMERATION("Chrome.ProcessSingleton.NotifyResult",
                              notify_result,
                              ProcessSingleton::kNumNotifyResults);
    switch (notify_result) {
      case ProcessSingleton::PROCESS_NONE:
        // No process already running, continue on to starting a new one.
        break;

      case ProcessSingleton::PROCESS_NOTIFIED:
        printf("%s\n", "Opening in existing browser session.");
        return chrome::RESULT_CODE_NORMAL_EXIT_PROCESS_NOTIFIED;

      case ProcessSingleton::PROFILE_IN_USE:
        return chrome::RESULT_CODE_PROFILE_IN_USE;

      case ProcessSingleton::LOCK_ERROR:
        LOG(ERROR) << "Failed to create a ProcessSingleton for your profile "
                      "directory. This means that running multiple instances "
                      "would start multiple browser processes rather than "
                      "opening a new window in the existing process. Aborting "
                      "now to avoid profile corruption.";
        return chrome::RESULT_CODE_PROFILE_IN_USE;
    }
  }
#endif

#if BUILDFLAG(IS_WIN)
  // Initialize the cleaner of left-behind tmp files now that the main thread
  // has its SequencedTaskRunner; see https://crbug.com/1075917.
  base::ImportantFileWriterCleaner::GetInstance().Initialize();

  // For now, do not enable delay load failure hooks for browser process except
  // in tests, where failures really shouldn't happen.
  if (invoked_in_browser->is_running_test)
    chrome::DisableDelayLoadFailureHooksForCurrentModule();
#endif

  // Chrome disallows cookies by default. All code paths that want to use
  // cookies need to go through one of Chrome's URLRequestContexts which have
  // a ChromeNetworkDelegate attached that selectively allows cookies again.
  net::URLRequest::SetDefaultCookiePolicyToBlock();

  // On Chrome OS, IPC (D-Bus, Crosapi) is required to create the FeatureList,
  // which depends on policy from an OS service. So, initialize it at this
  // timing.
#if BUILDFLAG(IS_CHROMEOS_ASH)
  // The feature list depends on BrowserPolicyConnectorAsh which depends
  // on DBus, so initialize it here. Some D-Bus clients may depend on feature
  // list, so initialize them separately later at the end of this function.
  ash::InitializeDBus();
#elif BUILDFLAG(IS_CHROMEOS_LACROS)
  // Initialize D-Bus for Lacros.
  chromeos::LacrosInitializeDBus();
#endif

  // The DBus initialization above is needed for FeatureList creation here;
  // features are needed for Mojo initialization; and Mojo initialization is
  // needed for LacrosService initialization below.
  ChromeFeatureListCreator* chrome_feature_list_creator =
      chrome_content_browser_client_->startup_data()
          ->chrome_feature_list_creator();
  chrome_feature_list_creator->CreateFeatureList();

  content::InitializeMojoCore();

#if BUILDFLAG(IS_CHROMEOS_LACROS)
  // LacrosService instance needs the sequence of the main thread,
  // and needs to be created earlier than incoming Mojo invitation handling.
  // This also needs ThreadPool sequences to post some tasks internally.
  // However, the tasks can be suspended until actual start of the ThreadPool
  // sequences later.
  lacros_service_ = std::make_unique<chromeos::LacrosService>();
  {
    const chromeos::BrowserParamsProxy* init_params =
        chromeos::BrowserParamsProxy::Get();
    // This lives here rather than in ChromeBrowserMainExtraPartsLacros due to
    // timing constraints. If we relocate it, then the flags aren't propagated
    // to the GPU process.
    if (init_params->BuildFlags().has_value()) {
      for (auto flag : init_params->BuildFlags().value()) {
        switch (flag) {
          case crosapi::mojom::BuildFlag::kUnknown:
            break;
          case crosapi::mojom::BuildFlag::kEnablePlatformEncryptedHevc:
            // This was deprecated.
            break;
          case crosapi::mojom::BuildFlag::kEnablePlatformHevc:
            base::CommandLine::ForCurrentProcess()->AppendSwitch(
                switches::kLacrosEnablePlatformHevc);
            break;
          case crosapi::mojom::BuildFlag::kUseChromeosProtectedMedia:
            base::CommandLine::ForCurrentProcess()->AppendSwitch(
                switches::kLacrosUseChromeosProtectedMedia);
            break;
          case crosapi::mojom::BuildFlag::kUseChromeosProtectedAv1:
            base::CommandLine::ForCurrentProcess()->AppendSwitch(
                switches::kLacrosUseChromeosProtectedAv1);
            break;
        }
      }
    }
  }
#endif  // BUILDFLAG(IS_CHROMEOS_LACROS)

  CommonEarlyInitialization();

  // Initializes the resource bundle and determines the locale.
  std::string actual_locale = LoadLocalState(
      chrome_feature_list_creator, invoked_in_browser->is_running_test);
  chrome_feature_list_creator->SetApplicationLocale(actual_locale);
  chrome_feature_list_creator->OverrideCachedUIStrings();

  // On Chrome OS, initialize D-Bus clients that depend on feature list.
#if BUILDFLAG(IS_CHROMEOS_ASH)
  ash::InitializeFeatureListDependentDBus();
#elif BUILDFLAG(IS_CHROMEOS_LACROS)
  chromeos::LacrosInitializeFeatureListDependentDBus();
#endif

#if BUILDFLAG(IS_ANDROID)
  chrome_content_browser_client_->startup_data()->CreateProfilePrefService();
  net::NetworkChangeNotifier::SetFactory(
      new net::NetworkChangeNotifierFactoryAndroid());
#endif

  if (base::FeatureList::IsEnabled(
          features::kWriteBasicSystemProfileToPersistentHistogramsFile)) {
    bool record = true;
#if BUILDFLAG(IS_ANDROID)
    record =
        base::FeatureList::IsEnabled(chrome::android::kUmaBackgroundSessions);
#endif
    if (record)
      chrome_content_browser_client_->startup_data()->RecordCoreSystemProfile();
  }

#if BUILDFLAG(IS_ANDROID)
  UmaSessionStats::OnStartup();
#endif

#if BUILDFLAG(IS_MAC)
  chrome::CacheChannelInfo();
#endif

  return absl::nullopt;
}

bool ChromeMainDelegate::ShouldCreateFeatureList(InvokedIn invoked_in) {
  // In the browser process Chrome creates the FeatureList, so content should
  // not.
  return absl::holds_alternative<InvokedInChildProcess>(invoked_in);
}

bool ChromeMainDelegate::ShouldInitializeMojo(InvokedIn invoked_in) {
  return ShouldCreateFeatureList(invoked_in);
}

void ChromeMainDelegate::CommonEarlyInitialization() {
  std::string process_type =
      base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
          switches::kProcessType);
  bool is_browser_process = process_type.empty();

  // Enable Split cache by default here and not in content/ so as to not
  // impact non-Chrome embedders like WebView, Cronet etc. This only enables
  // it if not already overridden by command line, field trial etc.
  net::HttpCache::SplitCacheFeatureEnableByDefault();

#if BUILDFLAG(IS_CHROMEOS)
  // Threading features.
  base::PlatformThread::InitThreadPostFieldTrial();
#endif

  version_info::Channel channel = chrome::GetChannel();
  [[maybe_unused]] bool is_canary_dev =
      (channel == version_info::Channel::CANARY ||
       channel == version_info::Channel::DEV);
  // GWP-ASAN requires crashpad to gather alloc/dealloc stack traces, which is
  // not always enabled on Linux/ChromeOS.
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
  bool enable_gwp_asan = crash_reporter::IsCrashpadEnabled();
#else
  bool enable_gwp_asan = true;
#endif

  if (enable_gwp_asan) {
#if BUILDFLAG(ENABLE_GWP_ASAN_MALLOC)
    gwp_asan::EnableForMalloc(is_canary_dev || is_browser_process,
                              process_type.c_str());
#endif
#if BUILDFLAG(ENABLE_GWP_ASAN_PARTITIONALLOC)
    gwp_asan::EnableForPartitionAlloc(is_canary_dev || is_browser_process,
                                      process_type.c_str());
#endif
  }

  // Start heap profiling as early as possible so it can start recording
  // memory allocations.
  heap_profiler_controller_ =
      std::make_unique<heap_profiling::HeapProfilerController>(
          channel,
          GetProfileParamsProcess(*base::CommandLine::ForCurrentProcess()));
  heap_profiler_controller_->StartIfEnabled();

  if (is_browser_process) {
#if BUILDFLAG(IS_CHROMEOS_ASH)
    ash::ConfigureSwap();
    ash::InitializeKstaled();

    // If we're in an experimental group that locks the browser text we will do
    // that now.
    ash::LockMainProgramText();
#endif
  }

#if BUILDFLAG(IS_WIN)
  SetUpExtendedCrashReporting(is_browser_process);
  base::sequence_manager::internal::ThreadControllerPowerMonitor::
      InitializeOnMainThread();
  base::InitializePlatformThreadFeatures();
#endif

  // Initialize the HangWatcher.
  base::HangWatcher::ProcessType hang_watcher_process_type;
  if (process_type.empty()) {
    hang_watcher_process_type = base::HangWatcher::ProcessType::kBrowserProcess;
  } else if (process_type == switches::kGpuProcess) {
    hang_watcher_process_type = base::HangWatcher::ProcessType::kGPUProcess;
  } else if (process_type == switches::kRendererProcess) {
    hang_watcher_process_type =
        base::HangWatcher::ProcessType::kRendererProcess;
  } else if (process_type == switches::kUtilityProcess) {
    hang_watcher_process_type = base::HangWatcher::ProcessType::kUtilityProcess;
  } else {
    hang_watcher_process_type = base::HangWatcher::ProcessType::kUnknownProcess;
  }
  base::HangWatcher::InitializeOnMainThread(hang_watcher_process_type);

  base::internal::TimerBase::InitializeFeatures();
  base::InitializeCpuReductionExperiment();
  base::sequence_manager::internal::SequenceManagerImpl::InitializeFeatures();
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_ANDROID)
  base::MessagePumpLibevent::InitializeFeatures();
#elif BUILDFLAG(IS_MAC)
  base::PlatformThread::InitializeOptimizedRealtimeThreadingFeature();
  base::MessagePumpCFRunLoopBase::InitializeFeatures();
#endif
}

#if BUILDFLAG(IS_WIN)
bool ChromeMainDelegate::ShouldHandleConsoleControlEvents() {
  // Handle console control events so that orderly shutdown can be performed by
  // ChromeContentBrowserClient's override of SessionEnding.
  return true;
}
#endif

absl::optional<int> ChromeMainDelegate::BasicStartupComplete() {
#if BUILDFLAG(IS_CHROMEOS_ASH)
  ash::BootTimesRecorder::Get()->SaveChromeMainStats();
#endif

  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();

  // Only allow disabling web security via the command-line flag if the user has
  // specified a distinct profile directory. This still enables tests to disable
  // web security by setting the kWebKitWebSecurityEnabled pref directly.
  //
  // Note that this is done in ChromeMainDelegate::BasicStartupComplete()
  // because this is the earliest callback. Many places in Chromium gate
  // security features around kDisableWebSecurity, and it is unreasonable to
  // expect them all to properly also check for kUserDataDir.
  if (command_line.HasSwitch(switches::kDisableWebSecurity)) {
    base::FilePath default_user_data_dir;
    chrome::GetDefaultUserDataDirectory(&default_user_data_dir);
    const base::FilePath specified_user_data_dir =
        command_line.GetSwitchValuePath(switches::kUserDataDir)
            .StripTrailingSeparators();
    if (specified_user_data_dir.empty() ||
        specified_user_data_dir == default_user_data_dir) {
      LOG(ERROR) << "Web security may only be disabled if '--user-data-dir' is "
                    "also specified with a non-default value.";
      base::CommandLine::ForCurrentProcess()->RemoveSwitch(
          switches::kDisableWebSecurity);
    }
  }

#if BUILDFLAG(IS_WIN)
  // Browser should not be sandboxed.
  const bool is_browser = !command_line.HasSwitch(switches::kProcessType);
  if (is_browser && IsSandboxedProcess())
    return chrome::RESULT_CODE_INVALID_SANDBOX_STATE;
#endif

#if BUILDFLAG(IS_MAC)
  // Give the browser process a longer treadmill, since crashes
  // there have more impact.
  const bool is_browser = !command_line.HasSwitch(switches::kProcessType);
  ObjcEvilDoers::ZombieEnable(true, is_browser ? 10000 : 1000);
#endif

  content::Profiling::ProcessStarted();

  // Setup tracing sampler profiler as early as possible at startup if needed.
  // We pass in CreateCoreUnwindersFactory here since it lives in the chrome/
  // layer while TracingSamplerProfiler is outside of chrome/.
  tracing_sampler_profiler_ =
      tracing::TracingSamplerProfiler::CreateOnMainThread(
          base::BindRepeating(&CreateCoreUnwindersFactory));

#if BUILDFLAG(IS_WIN)
  v8_crashpad_support::SetUp();
#endif

#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
  if (!crash_reporter::IsCrashpadEnabled()) {
    breakpad::SetFirstChanceExceptionHandler(v8::TryHandleWebAssemblyTrapPosix);
  }
#endif

#if BUILDFLAG(IS_POSIX)
  if (HandleVersionSwitches(command_line)) {
    return 0;  // Got a --version switch; exit with a success error code.
  }
  if (HandleCreditsSwitch(command_line)) {
    return 0;  // Got a --credits switch; exit with a success error code.
  }

  // TODO(crbug.com/1052397): Revisit the macro expression once build flag
  // switch of lacros-chrome is complete.
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS_LACROS)
  // This will directly exit if the user asked for help.
  HandleHelpSwitches(command_line);
#endif
#endif  // BUILDFLAG(IS_POSIX)

#if BUILDFLAG(IS_WIN)
  // Must do this before any other usage of command line!
  if (HasDeprecatedArguments(command_line.GetCommandLineString())) {
    return 1;
  }

  // HandleVerifier detects and reports incorrect handle manipulations. It
  // tracks handle operations on builds that support DCHECK only.
#if !DCHECK_IS_ON()
  base::win::DisableHandleVerifier();
#endif

#endif  // BUILDFLAG(IS_WIN)

  chrome::RegisterPathProvider();
#if BUILDFLAG(IS_CHROMEOS_ASH)
  ash::RegisterPathProvider();
#endif
#if BUILDFLAG(IS_CHROMEOS_LACROS)
  chromeos::lacros_paths::RegisterPathProvider();
#endif
#if BUILDFLAG(IS_CHROMEOS)
  chromeos::dbus_paths::RegisterPathProvider();
#endif
#if BUILDFLAG(ENABLE_NACL) && (BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS))
  nacl::RegisterPathProvider();
#endif

  ContentSettingsPattern::SetNonWildcardDomainNonPortSchemes(
      kNonWildcardDomainNonPortSchemes, kNonWildcardDomainNonPortSchemesSize);

// No support for ANDROID yet as DiagnosticsController needs wchar support.
// TODO(gspencer): That's not true anymore, or at least there are no w-string
// references anymore. Not sure if that means this can be enabled on Android or
// not though.  As there is no easily accessible command line on Android, I'm
// not sure this is a big deal.
#if !BUILDFLAG(IS_ANDROID)
  // If we are in diagnostics mode this is the end of the line: after the
  // diagnostics are run the process will invariably exit.
  if (command_line.HasSwitch(switches::kDiagnostics)) {
    diagnostics::DiagnosticsWriter::FormatType format =
        diagnostics::DiagnosticsWriter::HUMAN;
    if (command_line.HasSwitch(switches::kDiagnosticsFormat)) {
      std::string format_str =
          command_line.GetSwitchValueASCII(switches::kDiagnosticsFormat);
      if (format_str == "machine") {
        format = diagnostics::DiagnosticsWriter::MACHINE;
      } else if (format_str == "log") {
        format = diagnostics::DiagnosticsWriter::LOG;
      } else {
        DCHECK_EQ("human", format_str);
      }
    }

    diagnostics::DiagnosticsWriter writer(format);
    int exit_code = diagnostics::DiagnosticsController::GetInstance()->Run(
        command_line, &writer);
    diagnostics::DiagnosticsController::GetInstance()->ClearResults();
    return exit_code;
  }
#endif

#if BUILDFLAG(IS_CHROMEOS_ASH)
  // Initialize primary user homedir (in multi-profile session) as it may be
  // passed as a command line switch.
  base::FilePath homedir;
  if (command_line.HasSwitch(ash::switches::kHomedir)) {
    homedir = base::FilePath(
        command_line.GetSwitchValueASCII(ash::switches::kHomedir));
    base::PathService::OverrideAndCreateIfNeeded(base::DIR_HOME, homedir, true,
                                                 false);
  }

  // If we are recovering from a crash on a ChromeOS device, then we will do
  // some recovery using the diagnostics module, and then continue on. We fake
  // up a command line to tell it that we want it to recover, and to preserve
  // the original command line. Note: logging at this point is to /var/log/ui.
  if ((base::SysInfo::IsRunningOnChromeOS() &&
       command_line.HasSwitch(ash::switches::kLoginUser)) ||
      command_line.HasSwitch(switches::kDiagnosticsRecovery)) {
    base::CommandLine interim_command_line(command_line.GetProgram());
    const char* const kSwitchNames[] = {
        switches::kUserDataDir,
    };
    interim_command_line.CopySwitchesFrom(command_line, kSwitchNames,
                                          std::size(kSwitchNames));
    interim_command_line.AppendSwitch(switches::kDiagnostics);
    interim_command_line.AppendSwitch(switches::kDiagnosticsRecovery);

    diagnostics::DiagnosticsWriter::FormatType format =
        diagnostics::DiagnosticsWriter::LOG;
    if (command_line.HasSwitch(switches::kDiagnosticsFormat)) {
      std::string format_str =
          command_line.GetSwitchValueASCII(switches::kDiagnosticsFormat);
      if (format_str == "machine") {
        format = diagnostics::DiagnosticsWriter::MACHINE;
      } else if (format_str == "human") {
        format = diagnostics::DiagnosticsWriter::HUMAN;
      } else {
        DCHECK_EQ("log", format_str);
      }
    }

    diagnostics::DiagnosticsWriter writer(format);
    int diagnostics_exit_code =
        diagnostics::DiagnosticsController::GetInstance()->Run(command_line,
                                                               &writer);
    if (diagnostics_exit_code) {
      // Diagnostics has failed somehow, so we exit.
      return diagnostics_exit_code;
    }

    // Now we run the actual recovery tasks.
    int recovery_exit_code =
        diagnostics::DiagnosticsController::GetInstance()->RunRecovery(
            command_line, &writer);

    if (recovery_exit_code) {
      // Recovery has failed somehow, so we exit.
      return recovery_exit_code;
    }
  } else {  // Not running diagnostics or recovery.
    diagnostics::DiagnosticsController::GetInstance()->RecordRegularStartup();
  }
#endif

  // The TLS slot used by the memlog allocator shim needs to be initialized
  // early to ensure that it gets assigned a low slot number. If it gets
  // initialized too late, the glibc TLS system will require a malloc call in
  // order to allocate storage for a higher slot number. Since malloc is hooked,
  // this causes re-entrancy into the allocator shim, while the TLS object is
  // partially-initialized, which the TLS object is supposed to protect again.
  heap_profiling::InitTLSSlot();

  return absl::nullopt;
}

#if BUILDFLAG(IS_MAC)
void ChromeMainDelegate::InitMacCrashReporter(
    const base::CommandLine& command_line,
    const std::string& process_type) {
  // TODO(mark): Right now, InitializeCrashpad() needs to be called after
  // CommandLine::Init() and chrome::RegisterPathProvider().  Ideally, Crashpad
  // initialization could occur sooner, preferably even before the framework
  // dylib is even loaded, to catch potential early crashes.

  const bool browser_process = process_type.empty();
  const bool install_from_dmg_relauncher_process =
      process_type == switches::kRelauncherProcess &&
      command_line.HasSwitch(switches::kRelauncherProcessDMGDevice);

  const bool initial_client =
      browser_process || install_from_dmg_relauncher_process;

  crash_reporter::InitializeCrashpad(initial_client, process_type);

  if (!browser_process) {
    std::string metrics_client_id =
        command_line.GetSwitchValueASCII(switches::kMetricsClientID);
    crash_keys::SetMetricsClientIdFromGUID(metrics_client_id);
  }

  // Mac Chrome is packaged with a main app bundle and a helper app bundle.
  // The main app bundle should only be used for the browser process, so it
  // should never see a --type switch (switches::kProcessType).  Likewise,
  // the helper should always have a --type switch.
  //
  // This check is done this late so there is already a call to
  // base::mac::IsBackgroundOnlyProcess(), so there is no change in
  // startup/initialization order.

  // The helper's Info.plist marks it as a background only app.
  if (base::mac::IsBackgroundOnlyProcess()) {
    CHECK(command_line.HasSwitch(switches::kProcessType) &&
          !process_type.empty())
        << "Helper application requires --type.";
  } else if (base::mac::AmIBundled()) {
    CHECK(!command_line.HasSwitch(switches::kProcessType) &&
          process_type.empty())
        << "Main application forbids --type, saw " << process_type;
  }
}

void ChromeMainDelegate::SetUpInstallerPreferences(
    const base::CommandLine& command_line) {
  const bool uma_setting = command_line.HasSwitch(switches::kEnableUserMetrics);
  const bool default_browser_setting =
      command_line.HasSwitch(switches::kMakeChromeDefault);

  if (uma_setting)
    crash_reporter::SetUploadConsent(uma_setting);
  if (default_browser_setting)
    shell_integration::SetAsDefaultBrowser();
}
#endif  // BUILDFLAG(IS_MAC)

void ChromeMainDelegate::PreSandboxStartup() {
  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();
  std::string process_type =
      command_line.GetSwitchValueASCII(switches::kProcessType);

  crash_reporter::InitializeCrashKeys();

#if BUILDFLAG(IS_POSIX)
  ChromeCrashReporterClient::Create();
#endif

#if BUILDFLAG(IS_MAC)
  InitMacCrashReporter(command_line, process_type);
  SetUpInstallerPreferences(command_line);
#endif

#if BUILDFLAG(IS_WIN)
  child_process_logging::Init();
#endif
#if defined(ARCH_CPU_ARM_FAMILY) && \
    (BUILDFLAG(IS_ANDROID) || BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS))
  // Create an instance of the CPU class to parse /proc/cpuinfo and cache
  // cpu_brand info.
  base::CPU cpu_info;
#endif

  // Initialize the user data dir for any process type that needs it.
  if (chrome::ProcessNeedsProfileDir(process_type))
    InitializeUserDataDir(base::CommandLine::ForCurrentProcess());

#if BUILDFLAG(IS_CHROMEOS_LACROS)
  if (process_type.empty() || process_type == switches::kZygoteProcess ||
      process_type == switches::kUtilityProcess) {
    // Initialize BrowserInitParams before generating and loading shared
    // resource file since the path required for the feature is set by
    // BrowserInitParams initialization.
    const chromeos::BrowserParamsProxy* init_params =
        chromeos::BrowserParamsProxy::Get();
    chrome::SetLacrosDefaultPathsFromInitParams(
        init_params->DefaultPaths().get());

    // Override the login user DIR_HOME path for the Lacros browser process. The
    // primary user id hash is expected to be already set, because Lacros should
    // only run inside the user session.
    if (init_params->CrosUserIdHash().has_value()) {
      base::FilePath homedir(kUserHomeDirPrefix);
      homedir = homedir.Append(init_params->CrosUserIdHash().value());
      base::PathService::OverrideAndCreateIfNeeded(
          base::DIR_HOME, homedir, /*is_absolute=*/true, /*create=*/false);
    }
  }

  // Generate shared resource file only on browser process. This is to avoid
  // generating a file in different processes again.
  // Also generate only when resource file sharing feature is enabled.
  if (command_line.HasSwitch(switches::kEnableResourcesFileSharing) &&
      process_type.empty()) {
    base::FilePath ash_resources_dir;
    base::FilePath lacros_resources_dir;
    base::FilePath user_data_dir;
    if (base::PathService::Get(chromeos::lacros_paths::ASH_RESOURCES_DIR,
                               &ash_resources_dir) &&
        base::PathService::Get(base::DIR_ASSETS, &lacros_resources_dir) &&
        base::PathService::Get(chromeos::lacros_paths::USER_DATA_DIR,
                               &user_data_dir)) {
      ui::DataPackWithResourceSharing::MaybeGenerateFallbackAndMapping(
          ash_resources_dir.Append(FILE_PATH_LITERAL("resources.pak")),
          lacros_resources_dir.Append(FILE_PATH_LITERAL("resources.pak")),
          user_data_dir.Append(crosapi::kSharedResourcesPackName),
          ui::kScaleFactorNone);
      ui::DataPackWithResourceSharing::MaybeGenerateFallbackAndMapping(
          ash_resources_dir.Append(FILE_PATH_LITERAL("chrome_100_percent.pak")),
          lacros_resources_dir.Append(
              FILE_PATH_LITERAL("chrome_100_percent.pak")),
          user_data_dir.Append(crosapi::kSharedChrome100PercentPackName),
          ui::k100Percent);
      ui::DataPackWithResourceSharing::MaybeGenerateFallbackAndMapping(
          ash_resources_dir.Append(FILE_PATH_LITERAL("chrome_200_percent.pak")),
          lacros_resources_dir.Append(
              FILE_PATH_LITERAL("chrome_200_percent.pak")),
          user_data_dir.Append(crosapi::kSharedChrome200PercentPackName),
          ui::k200Percent);
    }
  }
#endif  // BUILDFLAG(IS_CHROMEOS_LACROS)

  // Register component_updater PathProvider after DIR_USER_DATA overridden by
  // command line flags. Maybe move the chrome PathProvider down here also?
  int alt_preinstalled_components_dir =
#if BUILDFLAG(IS_CHROMEOS_ASH)
      ash::DIR_PREINSTALLED_COMPONENTS;
#else
      chrome::DIR_INTERNAL_PLUGINS;
#endif
  component_updater::RegisterPathProvider(chrome::DIR_COMPONENTS,
                                          alt_preinstalled_components_dir,
                                          chrome::DIR_USER_DATA);

#if !BUILDFLAG(IS_ANDROID) && !BUILDFLAG(IS_WIN)
  // Android does InitLogging when library is loaded. Skip here.
  // For windows we call InitLogging when the sandbox is initialized.
  InitLogging(process_type);
#endif

#if BUILDFLAG(IS_WIN)
  // TODO(zturner): Throbber icons and cursors are still stored in chrome.dll,
  // this can be killed once those are merged into resources.pak. See
  // GlassBrowserFrameView::InitThrobberIcons(), https://crbug.com/368327 and
  // https://crbug.com/1178117.
  ui::SetResourcesDataDLL(_AtlBaseModule.GetResourceInstance());
#endif

  if (SubprocessNeedsResourceBundle(process_type)) {
    // Initialize ResourceBundle which handles files loaded from external
    // sources.  The language should have been passed in to us from the
    // browser process as a command line flag.
#if !BUILDFLAG(ENABLE_NACL)
    DCHECK(command_line.HasSwitch(switches::kLang) ||
           process_type == switches::kZygoteProcess ||
           process_type == switches::kGpuProcess ||
           process_type == switches::kPpapiPluginProcess);
#else
    DCHECK(command_line.HasSwitch(switches::kLang) ||
           process_type == switches::kZygoteProcess ||
           process_type == switches::kGpuProcess ||
           process_type == switches::kNaClLoaderProcess ||
           process_type == switches::kPpapiPluginProcess);
#endif

    // TODO(markusheintz): The command line flag --lang is actually processed
    // by the CommandLinePrefStore, and made available through the PrefService
    // via the preference prefs::kApplicationLocale. The browser process uses
    // the --lang flag to pass the value of the PrefService in here. Maybe
    // this value could be passed in a different way.
    std::string locale = command_line.GetSwitchValueASCII(switches::kLang);
#if BUILDFLAG(IS_CHROMEOS_ASH)
    if (process_type == switches::kZygoteProcess) {
      DCHECK(locale.empty());
      // See comment at ReadAppLocale() for why we do this.
      locale = ash::startup_settings_cache::ReadAppLocale();
    }

    ui::ResourceBundle::SetLottieParsingFunctions(
        &lottie::ParseLottieAsStillImage,
        &lottie::ParseLottieAsThemedStillImage);
#endif
#if BUILDFLAG(IS_ANDROID)
    // The renderer sandbox prevents us from accessing our .pak files directly.
    // Therefore file descriptors to the .pak files that we need are passed in
    // at process creation time.
    auto* global_descriptors = base::GlobalDescriptors::GetInstance();
    int pak_fd = global_descriptors->Get(kAndroidLocalePakDescriptor);
    base::MemoryMappedFile::Region pak_region =
        global_descriptors->GetRegion(kAndroidLocalePakDescriptor);
    ui::ResourceBundle::InitSharedInstanceWithPakFileRegion(base::File(pak_fd),
                                                            pak_region);

    // Load secondary locale .pak file if it exists.
    pak_fd = global_descriptors->MaybeGet(kAndroidSecondaryLocalePakDescriptor);
    if (pak_fd != -1) {
      pak_region =
          global_descriptors->GetRegion(kAndroidSecondaryLocalePakDescriptor);
      ui::ResourceBundle::GetSharedInstance()
          .LoadSecondaryLocaleDataWithPakFileRegion(base::File(pak_fd),
                                                    pak_region);
    }

    int extra_pak_keys[] = {
        kAndroidChrome100PercentPakDescriptor,
        kAndroidUIResourcesPakDescriptor,
    };
    for (int extra_pak_key : extra_pak_keys) {
      pak_fd = global_descriptors->Get(extra_pak_key);
      pak_region = global_descriptors->GetRegion(extra_pak_key);
      ui::ResourceBundle::GetSharedInstance().AddDataPackFromFileRegion(
          base::File(pak_fd), pak_region, ui::k100Percent);
    }

    // For Android: Native resources for DFMs should only be used by the browser
    // process. Their file descriptors and memory mapped file region are not
    // passed to child processes, and are therefore not loaded here.

    base::i18n::SetICUDefaultLocale(locale);
    const std::string loaded_locale = locale;
#else
    const std::string loaded_locale =
        ui::ResourceBundle::InitSharedInstanceWithLocale(
            locale, nullptr, ui::ResourceBundle::LOAD_COMMON_RESOURCES);

    base::FilePath resources_pack_path;
    base::PathService::Get(chrome::FILE_RESOURCES_PACK, &resources_pack_path);
#if BUILDFLAG(IS_CHROMEOS_LACROS)
    if (command_line.HasSwitch(switches::kEnableResourcesFileSharing)) {
      // If LacrosResourcesFileSharing feature is enabled, Lacros refers to ash
      // resources pak file.
      base::FilePath ash_resources_pack_path;
      base::PathService::Get(chrome::FILE_ASH_RESOURCES_PACK,
                             &ash_resources_pack_path);
      base::FilePath shared_resources_pack_path;
      base::PathService::Get(chrome::FILE_RESOURCES_FOR_SHARING_PACK,
                             &shared_resources_pack_path);
      ui::ResourceBundle::GetSharedInstance()
          .AddDataPackFromPathWithAshResources(
              shared_resources_pack_path, ash_resources_pack_path,
              resources_pack_path, ui::kScaleFactorNone);
    } else {
      ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
          resources_pack_path, ui::kScaleFactorNone);
    }
#else
    ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
        resources_pack_path, ui::kScaleFactorNone);
#endif  // BUILDFLAG(IS_CHROMEOS_LACROS)
#endif  // BUILDFLAG(IS_ANDROID)
    CHECK(!loaded_locale.empty()) << "Locale could not be found for " << locale;
  }

#if BUILDFLAG(IS_POSIX) && !BUILDFLAG(IS_MAC)
  // Zygote needs to call InitCrashReporter() in RunZygote().
  if (process_type != switches::kZygoteProcess) {
    if (command_line.HasSwitch(switches::kPreCrashpadCrashTest)) {
      // Crash for the purposes of testing the handling of crashes that happen
      // before crashpad is initialized. Please leave this check immediately
      // before the crashpad initialization; the amount of memory used at this
      // point is important to the test.
      IMMEDIATE_CRASH();
    }
#if BUILDFLAG(IS_ANDROID)
    crash_reporter::InitializeCrashpad(process_type.empty(), process_type);
    if (process_type.empty()) {
      base::android::InitJavaExceptionReporter();
      UninstallPureJavaExceptionHandler();
    } else {
      base::android::InitJavaExceptionReporterForChildProcess();
    }
#else   // !BUILDFLAG(IS_ANDROID)
    if (crash_reporter::IsCrashpadEnabled()) {
      crash_reporter::InitializeCrashpad(process_type.empty(), process_type);
      crash_reporter::SetFirstChanceExceptionHandler(
          v8::TryHandleWebAssemblyTrapPosix);
    } else {
      breakpad::InitCrashReporter(process_type);
    }
#endif  // BUILDFLAG(IS_ANDROID)
  }
#endif  // BUILDFLAG(IS_POSIX) && !BUILDFLAG(IS_MAC)

#if BUILDFLAG(IS_ANDROID)
  CHECK_EQ(base::android::GetLibraryProcessType(),
           process_type.empty() ? base::android::PROCESS_BROWSER
                                : base::android::PROCESS_CHILD);
#endif  // BUILDFLAG(IS_ANDROID)

  // After all the platform Breakpads have been initialized, store the command
  // line for crash reporting.
  crash_keys::SetCrashKeysFromCommandLine(command_line);

#if BUILDFLAG(ENABLE_PDF)
  MaybePatchGdiGetFontData();
#endif
}

void ChromeMainDelegate::SandboxInitialized(const std::string& process_type) {
  // Note: If you are adding a new process type below, be sure to adjust the
  // AdjustLinuxOOMScore function too.
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
  AdjustLinuxOOMScore(process_type);
#endif
#if BUILDFLAG(IS_WIN)
  InitLogging(process_type);
  SuppressWindowsErrorDialogs();
#endif

#if BUILDFLAG(ENABLE_NACL)
  ChromeContentClient::SetNaClEntryFunctions(nacl_plugin::PPP_GetInterface,
                                             nacl_plugin::PPP_InitializeModule,
                                             nacl_plugin::PPP_ShutdownModule);
#endif
}

absl::variant<int, content::MainFunctionParams> ChromeMainDelegate::RunProcess(
    const std::string& process_type,
    content::MainFunctionParams main_function_params) {
#if BUILDFLAG(IS_ANDROID)
  NOTREACHED();  // Android provides a subclass and shares no code here.
#else
  static const MainFunction kMainFunctions[] = {
#if BUILDFLAG(IS_MAC)
    {switches::kRelauncherProcess, mac_relauncher::internal::RelauncherMain},
#endif

  // This entry is not needed on Linux, where the NaCl loader
  // process is launched via nacl_helper instead.
#if BUILDFLAG(ENABLE_NACL) && !BUILDFLAG(IS_LINUX) && !BUILDFLAG(IS_CHROMEOS)
    {switches::kNaClLoaderProcess, NaClMain},
#else
    {"<invalid>", nullptr},  // To avoid constant array of size 0
                             // when NaCl disabled
#endif
  };

  for (size_t i = 0; i < std::size(kMainFunctions); ++i) {
    if (process_type == kMainFunctions[i].name)
      return kMainFunctions[i].function(std::move(main_function_params));
  }
#endif  // !BUILDFLAG(IS_ANDROID)

  return std::move(main_function_params);
}

void ChromeMainDelegate::ProcessExiting(const std::string& process_type) {
#if !BUILDFLAG(IS_ANDROID)
  if (ChromeProcessSingleton::IsEarlySingletonFeatureEnabled())
    ChromeProcessSingleton::DeleteInstance();
#endif  // !BUILDFLAG(IS_ANDROID)

  if (SubprocessNeedsResourceBundle(process_type))
    ui::ResourceBundle::CleanupSharedInstance();
#if !BUILDFLAG(IS_ANDROID)
  logging::CleanupChromeLogging();
#else
  // Android doesn't use InitChromeLogging, so we close the log file manually.
  logging::CloseLogFile();
#endif  // !BUILDFLAG(IS_ANDROID)
}

#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
void ChromeMainDelegate::ZygoteStarting(
    std::vector<std::unique_ptr<content::ZygoteForkDelegate>>* delegates) {
#if BUILDFLAG(IS_CHROMEOS_ASH)
  chromeos::InitHugepagesAndMlockSelf();
#endif

#if BUILDFLAG(ENABLE_NACL)
  nacl::AddNaClZygoteForkDelegates(delegates);
#endif
}

void ChromeMainDelegate::ZygoteForked() {
  content::Profiling::ProcessStarted();
  if (content::Profiling::BeingProfiled()) {
    base::debug::RestartProfilingAfterFork();
    SetUpProfilingShutdownHandler();
  }

  // Needs to be called after we have chrome::DIR_USER_DATA.  BrowserMain sets
  // this up for the browser process in a different manner.
  const base::CommandLine* command_line =
      base::CommandLine::ForCurrentProcess();
  std::string process_type =
      command_line->GetSwitchValueASCII(switches::kProcessType);
  if (crash_reporter::IsCrashpadEnabled()) {
    crash_reporter::InitializeCrashpad(false, process_type);
    crash_reporter::SetFirstChanceExceptionHandler(
        v8::TryHandleWebAssemblyTrapPosix);
  } else {
    breakpad::InitCrashReporter(process_type);
  }

  // Reset the command line for the newly spawned process.
  crash_keys::SetCrashKeysFromCommandLine(*command_line);
}

#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)

content::ContentClient* ChromeMainDelegate::CreateContentClient() {
  return &chrome_content_client_;
}

content::ContentBrowserClient*
ChromeMainDelegate::CreateContentBrowserClient() {
  chrome_content_browser_client_ =
      std::make_unique<AdblockContentBrowserClient>();
  return chrome_content_browser_client_.get();
}

content::ContentGpuClient* ChromeMainDelegate::CreateContentGpuClient() {
  return g_chrome_content_gpu_client.Pointer();
}

content::ContentRendererClient*
ChromeMainDelegate::CreateContentRendererClient() {
  return g_chrome_content_renderer_client.Pointer();
}

content::ContentUtilityClient*
ChromeMainDelegate::CreateContentUtilityClient() {
  chrome_content_utility_client_ =
      std::make_unique<ChromeContentUtilityClient>();
  return chrome_content_utility_client_.get();
}

absl::optional<int> ChromeMainDelegate::PreBrowserMain() {
  absl::optional<int> exit_code =
      content::ContentMainDelegate::PreBrowserMain();
  if (exit_code.has_value())
    return exit_code;

#if BUILDFLAG(ENABLE_EXTENSIONS)
  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();
  absl::optional<int> pack_extension_exit_code =
      HandlePackExtensionSwitches(command_line);
  if (pack_extension_exit_code.has_value())
    return pack_extension_exit_code;  // Got a --pack-extension switch; exit.
#endif

#if BUILDFLAG(IS_MAC)
  // Tell Cocoa to finish its initialization, which we want to do manually
  // instead of calling NSApplicationMain(). The primary reason is that NSAM()
  // never returns, which would leave all the objects currently on the stack
  // in scoped_ptrs hanging and never cleaned up. We then load the main nib
  // directly. The main event loop is run from common code using the
  // MessageLoop API, which works out ok for us because it's a wrapper around
  // CFRunLoop.

  // Initialize NSApplication using the custom subclass.
  chrome_browser_application_mac::RegisterBrowserCrApp();

  if (l10n_util::GetLocaleOverride().empty()) {
    // The browser process only wants to support the language Cocoa will use,
    // so force the app locale to be overridden with that value. This must
    // happen before the ResourceBundle is loaded, which happens in
    // ChromeBrowserMainParts::PreEarlyInitialization().
    // Don't do this if the locale is already set, which is done by integration
    // tests to ensure tests always run with the same locale.
    l10n_util::OverrideLocaleWithCocoaLocale();
  }
#endif

  // Do not interrupt startup.
  return absl::nullopt;
}
