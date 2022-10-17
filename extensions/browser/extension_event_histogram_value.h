// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This source code is a part of eyeo Chromium SDK.
// Use of this source code is governed by the GPLv3 that can be found in the
// components/adblock/LICENSE file.

#ifndef EXTENSIONS_BROWSER_EXTENSION_EVENT_HISTOGRAM_VALUE_H_
#define EXTENSIONS_BROWSER_EXTENSION_EVENT_HISTOGRAM_VALUE_H_

namespace extensions {
namespace events {

// Short version:
//  *Never* reorder or delete entries in the |HistogramValue| enumeration.
//  When creating a new extension event, add a new `ENUM = <value>` entry at
//  the end of the list, just prior to ENUM_BOUNDARY.
//
// Long version: See extension_function_histogram_value.h
enum HistogramValue {
  UNKNOWN = 0,

  // Tests should use this for a stub histogram value (not UNKNOWN).
  FOR_TEST = 1,

  ACCESSIBILITY_PRIVATE_ON_INTRODUCE_CHROME_VOX = 2,
  ACTIVITY_LOG_PRIVATE_ON_EXTENSION_ACTIVITY = 3,
  ALARMS_ON_ALARM = 4,
  APP_CURRENT_WINDOW_INTERNAL_ON_ALPHA_ENABLED_CHANGED = 5,
  APP_CURRENT_WINDOW_INTERNAL_ON_BOUNDS_CHANGED = 6,
  APP_CURRENT_WINDOW_INTERNAL_ON_CLOSED = 7,
  APP_CURRENT_WINDOW_INTERNAL_ON_FULLSCREENED = 8,
  APP_CURRENT_WINDOW_INTERNAL_ON_MAXIMIZED = 9,
  APP_CURRENT_WINDOW_INTERNAL_ON_MINIMIZED = 10,
  APP_CURRENT_WINDOW_INTERNAL_ON_RESTORED = 11,
  APP_CURRENT_WINDOW_INTERNAL_ON_WINDOW_SHOWN_FOR_TESTS = 12,
  APP_RUNTIME_ON_EMBED_REQUESTED = 13,
  APP_RUNTIME_ON_LAUNCHED = 14,
  APP_RUNTIME_ON_RESTARTED = 15,
  APP_WINDOW_ON_BOUNDS_CHANGED = 16,
  APP_WINDOW_ON_CLOSED = 17,
  APP_WINDOW_ON_FULLSCREENED = 18,
  APP_WINDOW_ON_MAXIMIZED = 19,
  APP_WINDOW_ON_MINIMIZED = 20,
  APP_WINDOW_ON_RESTORED = 21,
  DELETED_AUDIO_MODEM_ON_RECEIVED = 22,
  DELETED_AUDIO_MODEM_ON_TRANSMIT_FAIL = 23,
  DELETED_AUDIO_ON_DEVICE_CHANGED = 24,
  AUDIO_ON_DEVICES_CHANGED = 25,
  AUDIO_ON_LEVEL_CHANGED = 26,
  AUDIO_ON_MUTE_CHANGED = 27,
  AUTOFILL_PRIVATE_ON_ADDRESS_LIST_CHANGED_DEPRECATED = 28,
  AUTOFILL_PRIVATE_ON_CREDIT_CARD_LIST_CHANGED_DEPRECATED = 29,
  AUTOMATION_INTERNAL_ON_ACCESSIBILITY_EVENT = 30,
  AUTOMATION_INTERNAL_ON_ACCESSIBILITY_TREE_DESTROYED = 31,
  BLUETOOTH_LOW_ENERGY_ON_CHARACTERISTIC_VALUE_CHANGED = 32,
  BLUETOOTH_LOW_ENERGY_ON_DESCRIPTOR_VALUE_CHANGED = 33,
  BLUETOOTH_LOW_ENERGY_ON_SERVICE_ADDED = 34,
  BLUETOOTH_LOW_ENERGY_ON_SERVICE_CHANGED = 35,
  BLUETOOTH_LOW_ENERGY_ON_SERVICE_REMOVED = 36,
  BLUETOOTH_ON_ADAPTER_STATE_CHANGED = 37,
  BLUETOOTH_ON_DEVICE_ADDED = 38,
  BLUETOOTH_ON_DEVICE_CHANGED = 39,
  BLUETOOTH_ON_DEVICE_REMOVED = 40,
  BLUETOOTH_PRIVATE_ON_PAIRING = 41,
  BLUETOOTH_SOCKET_ON_ACCEPT = 42,
  BLUETOOTH_SOCKET_ON_ACCEPT_ERROR = 43,
  BLUETOOTH_SOCKET_ON_RECEIVE = 44,
  BLUETOOTH_SOCKET_ON_RECEIVE_ERROR = 45,
  BOOKMARK_MANAGER_PRIVATE_ON_DRAG_ENTER = 46,
  BOOKMARK_MANAGER_PRIVATE_ON_DRAG_LEAVE = 47,
  BOOKMARK_MANAGER_PRIVATE_ON_DROP = 48,
  BOOKMARK_MANAGER_PRIVATE_ON_META_INFO_CHANGED = 49,
  BOOKMARKS_ON_CHANGED = 50,
  BOOKMARKS_ON_CHILDREN_REORDERED = 51,
  BOOKMARKS_ON_CREATED = 52,
  BOOKMARKS_ON_IMPORT_BEGAN = 53,
  BOOKMARKS_ON_IMPORT_ENDED = 54,
  BOOKMARKS_ON_MOVED = 55,
  BOOKMARKS_ON_REMOVED = 56,
  BRAILLE_DISPLAY_PRIVATE_ON_DISPLAY_STATE_CHANGED = 57,
  BRAILLE_DISPLAY_PRIVATE_ON_KEY_EVENT = 58,
  BROWSER_ACTION_ON_CLICKED = 59,
  DELETED_CAST_STREAMING_RTP_STREAM_ON_ERROR = 60,
  DELETED_CAST_STREAMING_RTP_STREAM_ON_STARTED = 61,
  DELETED_CAST_STREAMING_RTP_STREAM_ON_STOPPED = 62,
  COMMANDS_ON_COMMAND = 63,
  CONTEXT_MENUS_INTERNAL_ON_CLICKED = 64,
  CONTEXT_MENUS_ON_CLICKED = 65,
  COOKIES_ON_CHANGED = 66,
  DELETED_COPRESENCE_ON_MESSAGES_RECEIVED = 67,
  DELETED_COPRESENCE_ON_STATUS_UPDATED = 68,
  DELETED_COPRESENCE_PRIVATE_ON_CONFIG_AUDIO = 69,
  DELETED_COPRESENCE_PRIVATE_ON_DECODE_SAMPLES_REQUEST = 70,
  DELETED_COPRESENCE_PRIVATE_ON_ENCODE_TOKEN_REQUEST = 71,
  DEBUGGER_ON_DETACH = 72,
  DEBUGGER_ON_EVENT = 73,
  DECLARATIVE_CONTENT_ON_PAGE_CHANGED = 74,
  DECLARATIVE_WEB_REQUEST_ON_MESSAGE = 75,
  DECLARATIVE_WEB_REQUEST_ON_REQUEST = 76,
  DEVELOPER_PRIVATE_ON_ITEM_STATE_CHANGED = 77,
  DEVELOPER_PRIVATE_ON_PROFILE_STATE_CHANGED = 78,
  DEVTOOLS_INSPECTED_WINDOW_ON_RESOURCE_ADDED = 79,
  DEVTOOLS_INSPECTED_WINDOW_ON_RESOURCE_CONTENT_COMMITTED = 80,
  DEVTOOLS_NETWORK_ON_NAVIGATED = 81,
  DEVTOOLS_NETWORK_ON_REQUEST_FINISHED = 82,
  DOWNLOADS_ON_CHANGED = 83,
  DOWNLOADS_ON_CREATED = 84,
  DOWNLOADS_ON_DETERMINING_FILENAME = 85,
  DOWNLOADS_ON_ERASED = 86,
  DELETED_EASY_UNLOCK_PRIVATE_ON_START_AUTO_PAIRING = 87,
  DELETED_EASY_UNLOCK_PRIVATE_ON_USER_INFO_UPDATED = 88,
  DELETED_EXPERIENCE_SAMPLING_PRIVATE_ON_DECISION = 89,
  DELETED_EXPERIENCE_SAMPLING_PRIVATE_ON_DISPLAYED = 90,
  EXPERIMENTAL_DEVTOOLS_CONSOLE_ON_MESSAGE_ADDED = 91,
  EXTENSION_ON_REQUEST = 92,
  EXTENSION_ON_REQUEST_EXTERNAL = 93,
  EXTENSION_OPTIONS_INTERNAL_ON_CLOSE = 94,
  EXTENSION_OPTIONS_INTERNAL_ON_LOAD = 95,
  EXTENSION_OPTIONS_INTERNAL_ON_PREFERRED_SIZE_CHANGED = 96,
  FEEDBACK_PRIVATE_ON_FEEDBACK_REQUESTED = 97,
  FILE_BROWSER_HANDLER_ON_EXECUTE = 98,
  FILE_MANAGER_PRIVATE_ON_COPY_PROGRESS = 99,
  FILE_MANAGER_PRIVATE_ON_DEVICE_CHANGED = 100,
  FILE_MANAGER_PRIVATE_ON_DIRECTORY_CHANGED = 101,
  FILE_MANAGER_PRIVATE_ON_DRIVE_CONNECTION_STATUS_CHANGED = 102,
  FILE_MANAGER_PRIVATE_ON_DRIVE_SYNC_ERROR = 103,
  FILE_MANAGER_PRIVATE_ON_FILE_TRANSFERS_UPDATED = 104,
  FILE_MANAGER_PRIVATE_ON_MOUNT_COMPLETED = 105,
  FILE_MANAGER_PRIVATE_ON_PREFERENCES_CHANGED = 106,
  FILE_SYSTEM_ON_VOLUME_LIST_CHANGED = 107,
  FILE_SYSTEM_PROVIDER_ON_ABORT_REQUESTED = 108,
  FILE_SYSTEM_PROVIDER_ON_ADD_WATCHER_REQUESTED = 109,
  FILE_SYSTEM_PROVIDER_ON_CLOSE_FILE_REQUESTED = 110,
  FILE_SYSTEM_PROVIDER_ON_CONFIGURE_REQUESTED = 111,
  FILE_SYSTEM_PROVIDER_ON_COPY_ENTRY_REQUESTED = 112,
  FILE_SYSTEM_PROVIDER_ON_CREATE_DIRECTORY_REQUESTED = 113,
  FILE_SYSTEM_PROVIDER_ON_CREATE_FILE_REQUESTED = 114,
  FILE_SYSTEM_PROVIDER_ON_DELETE_ENTRY_REQUESTED = 115,
  FILE_SYSTEM_PROVIDER_ON_GET_METADATA_REQUESTED = 116,
  FILE_SYSTEM_PROVIDER_ON_MOUNT_REQUESTED = 117,
  FILE_SYSTEM_PROVIDER_ON_MOVE_ENTRY_REQUESTED = 118,
  FILE_SYSTEM_PROVIDER_ON_OPEN_FILE_REQUESTED = 119,
  FILE_SYSTEM_PROVIDER_ON_READ_DIRECTORY_REQUESTED = 120,
  FILE_SYSTEM_PROVIDER_ON_READ_FILE_REQUESTED = 121,
  FILE_SYSTEM_PROVIDER_ON_REMOVE_WATCHER_REQUESTED = 122,
  FILE_SYSTEM_PROVIDER_ON_TRUNCATE_REQUESTED = 123,
  FILE_SYSTEM_PROVIDER_ON_UNMOUNT_REQUESTED = 124,
  FILE_SYSTEM_PROVIDER_ON_WRITE_FILE_REQUESTED = 125,
  FONT_SETTINGS_ON_DEFAULT_FIXED_FONT_SIZE_CHANGED = 126,
  FONT_SETTINGS_ON_DEFAULT_FONT_SIZE_CHANGED = 127,
  FONT_SETTINGS_ON_FONT_CHANGED = 128,
  FONT_SETTINGS_ON_MINIMUM_FONT_SIZE_CHANGED = 129,
  DELETED_GCD_PRIVATE_ON_DEVICE_REMOVED = 130,
  DELETED_GCD_PRIVATE_ON_DEVICE_STATE_CHANGED = 131,
  GCM_ON_MESSAGE = 132,
  GCM_ON_MESSAGES_DELETED = 133,
  GCM_ON_SEND_ERROR = 134,
  HANGOUTS_PRIVATE_ON_HANGOUT_REQUESTED_DEPRECATED = 135,
  HID_ON_DEVICE_ADDED = 136,
  HID_ON_DEVICE_REMOVED = 137,
  HISTORY_ON_VISITED = 138,
  HISTORY_ON_VISIT_REMOVED = 139,
  HOTWORD_PRIVATE_ON_DELETE_SPEAKER_MODEL = 140,
  HOTWORD_PRIVATE_ON_ENABLED_CHANGED = 141,
  HOTWORD_PRIVATE_ON_FINALIZE_SPEAKER_MODEL = 142,
  HOTWORD_PRIVATE_ON_HOTWORD_SESSION_REQUESTED = 143,
  HOTWORD_PRIVATE_ON_HOTWORD_SESSION_STOPPED = 144,
  HOTWORD_PRIVATE_ON_HOTWORD_TRIGGERED = 145,
  HOTWORD_PRIVATE_ON_MICROPHONE_STATE_CHANGED = 146,
  HOTWORD_PRIVATE_ON_SPEAKER_MODEL_EXISTS = 147,
  HOTWORD_PRIVATE_ON_SPEAKER_MODEL_SAVED = 148,
  IDENTITY_ON_SIGN_IN_CHANGED = 149,
  IDENTITY_PRIVATE_ON_WEB_FLOW_REQUEST = 150,
  IDLE_ON_STATE_CHANGED = 151,
  IMAGE_WRITER_PRIVATE_ON_DEVICE_INSERTED = 152,
  IMAGE_WRITER_PRIVATE_ON_DEVICE_REMOVED = 153,
  IMAGE_WRITER_PRIVATE_ON_WRITE_COMPLETE = 154,
  IMAGE_WRITER_PRIVATE_ON_WRITE_ERROR = 155,
  IMAGE_WRITER_PRIVATE_ON_WRITE_PROGRESS = 156,
  INPUT_IME_ON_ACTIVATE = 157,
  INPUT_IME_ON_BLUR = 158,
  INPUT_IME_ON_CANDIDATE_CLICKED = 159,
  INPUT_IME_ON_DEACTIVATED = 160,
  INPUT_IME_ON_FOCUS = 161,
  INPUT_IME_ON_INPUT_CONTEXT_UPDATE = 162,
  INPUT_IME_ON_KEY_EVENT = 163,
  INPUT_IME_ON_MENU_ITEM_ACTIVATED = 164,
  INPUT_IME_ON_RESET = 165,
  INPUT_IME_ON_SURROUNDING_TEXT_CHANGED = 166,
  INPUT_METHOD_PRIVATE_ON_CHANGED = 167,
  INPUT_METHOD_PRIVATE_ON_COMPOSITION_BOUNDS_CHANGED = 168,
  INPUT_METHOD_PRIVATE_ON_DICTIONARY_CHANGED = 169,
  INPUT_METHOD_PRIVATE_ON_DICTIONARY_LOADED = 170,
  INSTANCE_ID_ON_TOKEN_REFRESH = 171,
  DELETED_LOCATION_ON_LOCATION_ERROR = 172,
  DELETED_LOCATION_ON_LOCATION_UPDATE = 173,
  DELETED_LOG_PRIVATE_ON_CAPTURED_EVENTS = 174,
  MANAGEMENT_ON_DISABLED = 175,
  MANAGEMENT_ON_ENABLED = 176,
  MANAGEMENT_ON_INSTALLED = 177,
  MANAGEMENT_ON_UNINSTALLED = 178,
  MDNS_ON_SERVICE_LIST = 179,
  MEDIA_GALLERIES_ON_GALLERY_CHANGED = 180,
  MEDIA_GALLERIES_ON_SCAN_PROGRESS = 181,
  MEDIA_PLAYER_PRIVATE_ON_NEXT_TRACK = 182,
  MEDIA_PLAYER_PRIVATE_ON_PREV_TRACK = 183,
  MEDIA_PLAYER_PRIVATE_ON_TOGGLE_PLAY_STATE = 184,
  DELETED_NETWORKING_CONFIG_ON_CAPTIVE_PORTAL_DETECTED = 185,
  NETWORKING_PRIVATE_ON_DEVICE_STATE_LIST_CHANGED = 186,
  NETWORKING_PRIVATE_ON_NETWORK_LIST_CHANGED = 187,
  NETWORKING_PRIVATE_ON_NETWORKS_CHANGED = 188,
  NETWORKING_PRIVATE_ON_PORTAL_DETECTION_COMPLETED = 189,
  DELETED_NOTIFICATION_PROVIDER_ON_CLEARED = 190,
  DELETED_NOTIFICATION_PROVIDER_ON_CREATED = 191,
  DELETED_NOTIFICATION_PROVIDER_ON_UPDATED = 192,
  NOTIFICATIONS_ON_BUTTON_CLICKED = 193,
  NOTIFICATIONS_ON_CLICKED = 194,
  NOTIFICATIONS_ON_CLOSED = 195,
  NOTIFICATIONS_ON_PERMISSION_LEVEL_CHANGED = 196,
  NOTIFICATIONS_ON_SHOW_SETTINGS = 197,
  OMNIBOX_ON_INPUT_CANCELLED = 198,
  OMNIBOX_ON_INPUT_CHANGED = 199,
  OMNIBOX_ON_INPUT_ENTERED = 200,
  OMNIBOX_ON_INPUT_STARTED = 201,
  PAGE_ACTION_ON_CLICKED = 202,
  PASSWORDS_PRIVATE_ON_PASSWORD_EXCEPTIONS_LIST_CHANGED = 203,
  PASSWORDS_PRIVATE_ON_PLAINTEXT_PASSWORD_RETRIEVED = 204,
  PASSWORDS_PRIVATE_ON_SAVED_PASSWORDS_LIST_CHANGED = 205,
  PERMISSIONS_ON_ADDED = 206,
  PERMISSIONS_ON_REMOVED = 207,
  PRINTER_PROVIDER_ON_GET_CAPABILITY_REQUESTED = 208,
  PRINTER_PROVIDER_ON_GET_PRINTERS_REQUESTED = 209,
  PRINTER_PROVIDER_ON_GET_USB_PRINTER_INFO_REQUESTED = 210,
  PRINTER_PROVIDER_ON_PRINT_REQUESTED = 211,
  PROCESSES_ON_CREATED = 212,
  PROCESSES_ON_EXITED = 213,
  PROCESSES_ON_UNRESPONSIVE = 214,
  PROCESSES_ON_UPDATED = 215,
  PROCESSES_ON_UPDATED_WITH_MEMORY = 216,
  PROXY_ON_PROXY_ERROR = 217,
  RUNTIME_ON_BROWSER_UPDATE_AVAILABLE = 218,
  RUNTIME_ON_CONNECT = 219,
  RUNTIME_ON_CONNECT_EXTERNAL = 220,
  RUNTIME_ON_INSTALLED = 221,
  RUNTIME_ON_MESSAGE = 222,
  RUNTIME_ON_MESSAGE_EXTERNAL = 223,
  RUNTIME_ON_RESTART_REQUIRED = 224,
  RUNTIME_ON_STARTUP = 225,
  RUNTIME_ON_SUSPEND = 226,
  RUNTIME_ON_SUSPEND_CANCELED = 227,
  RUNTIME_ON_UPDATE_AVAILABLE = 228,
  SEARCH_ENGINES_PRIVATE_ON_SEARCH_ENGINES_CHANGED = 229,
  SERIAL_ON_RECEIVE = 230,
  SERIAL_ON_RECEIVE_ERROR = 231,
  SESSIONS_ON_CHANGED = 232,
  SETTINGS_PRIVATE_ON_PREFS_CHANGED = 233,
  DELETED_SIGNED_IN_DEVICES_ON_DEVICE_INFO_CHANGE = 234,
  SOCKETS_TCP_ON_RECEIVE = 235,
  SOCKETS_TCP_ON_RECEIVE_ERROR = 236,
  SOCKETS_TCP_SERVER_ON_ACCEPT = 237,
  SOCKETS_TCP_SERVER_ON_ACCEPT_ERROR = 238,
  SOCKETS_UDP_ON_RECEIVE = 239,
  SOCKETS_UDP_ON_RECEIVE_ERROR = 240,
  STORAGE_ON_CHANGED = 241,
  DELETED_STREAMS_PRIVATE_ON_EXECUTE_MIME_TYPE_HANDLER = 242,
  SYNC_FILE_SYSTEM_ON_FILE_STATUS_CHANGED = 243,
  SYNC_FILE_SYSTEM_ON_SERVICE_STATUS_CHANGED = 244,
  SYSTEM_DISPLAY_ON_DISPLAY_CHANGED = 245,
  SYSTEM_INDICATOR_ON_CLICKED = 246,
  SYSTEM_PRIVATE_ON_BRIGHTNESS_CHANGED = 247,
  SYSTEM_PRIVATE_ON_SCREEN_UNLOCKED = 248,
  SYSTEM_PRIVATE_ON_VOLUME_CHANGED = 249,
  SYSTEM_PRIVATE_ON_WOKE_UP = 250,
  SYSTEM_STORAGE_ON_ATTACHED = 251,
  SYSTEM_STORAGE_ON_DETACHED = 252,
  TAB_CAPTURE_ON_STATUS_CHANGED = 253,
  TABS_ON_ACTIVATED = 254,
  TABS_ON_ACTIVE_CHANGED = 255,
  TABS_ON_ATTACHED = 256,
  TABS_ON_CREATED = 257,
  TABS_ON_DETACHED = 258,
  TABS_ON_HIGHLIGHT_CHANGED = 259,
  TABS_ON_HIGHLIGHTED = 260,
  TABS_ON_MOVED = 261,
  TABS_ON_REMOVED = 262,
  TABS_ON_REPLACED = 263,
  TABS_ON_SELECTION_CHANGED = 264,
  TABS_ON_UPDATED = 265,
  TABS_ON_ZOOM_CHANGE = 266,
  TERMINAL_PRIVATE_ON_PROCESS_OUTPUT = 267,
  TEST_ON_MESSAGE = 268,
  TTS_ENGINE_ON_PAUSE = 269,
  TTS_ENGINE_ON_RESUME = 270,
  TTS_ENGINE_ON_SPEAK = 271,
  TTS_ENGINE_ON_STOP = 272,
  USB_ON_DEVICE_ADDED = 273,
  USB_ON_DEVICE_REMOVED = 274,
  VIRTUAL_KEYBOARD_PRIVATE_ON_BOUNDS_CHANGED = 275,
  VIRTUAL_KEYBOARD_PRIVATE_ON_TEXT_INPUT_BOX_FOCUSED = 276,
  VPN_PROVIDER_ON_CONFIG_CREATED = 277,
  VPN_PROVIDER_ON_CONFIG_REMOVED = 278,
  VPN_PROVIDER_ON_PACKET_RECEIVED = 279,
  VPN_PROVIDER_ON_PLATFORM_MESSAGE = 280,
  VPN_PROVIDER_ON_UI_EVENT = 281,
  DELETED_WALLPAPER_PRIVATE_ON_WALLPAPER_CHANGED_BY_3RD_PARTY = 282,
  WEB_NAVIGATION_ON_BEFORE_NAVIGATE = 283,
  WEB_NAVIGATION_ON_COMMITTED = 284,
  WEB_NAVIGATION_ON_COMPLETED = 285,
  WEB_NAVIGATION_ON_CREATED_NAVIGATION_TARGET = 286,
  WEB_NAVIGATION_ON_DOM_CONTENT_LOADED = 287,
  WEB_NAVIGATION_ON_ERROR_OCCURRED = 288,
  WEB_NAVIGATION_ON_HISTORY_STATE_UPDATED = 289,
  WEB_NAVIGATION_ON_REFERENCE_FRAGMENT_UPDATED = 290,
  WEB_NAVIGATION_ON_TAB_REPLACED = 291,
  WEB_REQUEST_ON_AUTH_REQUIRED = 292,
  WEB_REQUEST_ON_BEFORE_REDIRECT = 293,
  WEB_REQUEST_ON_BEFORE_REQUEST = 294,
  WEB_REQUEST_ON_BEFORE_SEND_HEADERS = 295,
  WEB_REQUEST_ON_COMPLETED = 296,
  WEB_REQUEST_ON_ERROR_OCCURRED = 297,
  WEB_REQUEST_ON_HEADERS_RECEIVED = 298,
  WEB_REQUEST_ON_RESPONSE_STARTED = 299,
  WEB_REQUEST_ON_SEND_HEADERS = 300,
  WEBRTC_AUDIO_PRIVATE_ON_SINKS_CHANGED = 301,
  WEBSTORE_ON_DOWNLOAD_PROGRESS = 302,
  WEBSTORE_ON_INSTALL_STAGE_CHANGED = 303,
  DELETED_WEBSTORE_WIDGET_PRIVATE_ON_SHOW_WIDGET = 304,
  WEBVIEW_TAG_CLOSE = 305,
  WEBVIEW_TAG_CONSOLEMESSAGE = 306,
  WEBVIEW_TAG_CONTENTLOAD = 307,
  WEBVIEW_TAG_DIALOG = 308,
  WEBVIEW_TAG_EXIT = 309,
  WEBVIEW_TAG_FINDUPDATE = 310,
  WEBVIEW_TAG_LOADABORT = 311,
  WEBVIEW_TAG_LOADCOMMIT = 312,
  WEBVIEW_TAG_LOADREDIRECT = 313,
  WEBVIEW_TAG_LOADSTART = 314,
  WEBVIEW_TAG_LOADSTOP = 315,
  WEBVIEW_TAG_NEWWINDOW = 316,
  WEBVIEW_TAG_PERMISSIONREQUEST = 317,
  WEBVIEW_TAG_RESPONSIVE = 318,
  WEBVIEW_TAG_SIZECHANGED = 319,
  WEBVIEW_TAG_UNRESPONSIVE = 320,
  WEBVIEW_TAG_ZOOMCHANGE = 321,
  WINDOWS_ON_CREATED = 322,
  WINDOWS_ON_FOCUS_CHANGED = 323,
  WINDOWS_ON_REMOVED = 324,
  FILE_SYSTEM_PROVIDER_ON_EXECUTE_ACTION_REQUESTED = 325,
  FILE_SYSTEM_PROVIDER_ON_GET_ACTIONS_REQUESTED = 326,
  DELETED_LAUNCHER_SEARCH_PROVIDER_ON_QUERY_STARTED = 327,
  DELETED_LAUNCHER_SEARCH_PROVIDER_ON_QUERY_ENDED = 328,
  DELETED_LAUNCHER_SEARCH_PROVIDER_ON_OPEN_RESULT = 329,
  CHROME_WEB_VIEW_INTERNAL_ON_CLICKED = 330,
  WEB_VIEW_INTERNAL_CONTEXT_MENUS = 331,
  CONTEXT_MENUS = 332,
  TTS_ON_EVENT = 333,
  LAUNCHER_PAGE_ON_TRANSITION_CHANGED_DEPRECATED = 334,
  LAUNCHER_PAGE_ON_POP_SUBPAGE_DEPRECATED = 335,
  DIAL_ON_DEVICE_LIST = 336,
  DIAL_ON_ERROR = 337,
  CAST_CHANNEL_ON_MESSAGE = 338,
  CAST_CHANNEL_ON_ERROR = 339,
  DELETED_SCREENLOCK_PRIVATE_ON_CHANGED = 340,
  DELETED_SCREENLOCK_PRIVATE_ON_AUTH_ATTEMPTED = 341,
  TYPES_CHROME_SETTING_ON_CHANGE = 342,
  DELETED_TYPES_PRIVATE_CHROME_DIRECT_SETTING_ON_CHANGE = 343,
  WEB_VIEW_INTERNAL_ON_MESSAGE = 344,
  // Obsolete: crbug.com/982858
  DELETED_EXTENSION_VIEW_INTERNAL_ON_LOAD_COMMIT = 345,
  RUNTIME_ON_REQUEST = 346,
  RUNTIME_ON_REQUEST_EXTERNAL = 347,
  CHROME_WEB_VIEW_INTERNAL_ON_CONTEXT_MENU_SHOW = 348,
  WEB_VIEW_INTERNAL_ON_BEFORE_REQUEST = 349,
  WEB_VIEW_INTERNAL_ON_BEFORE_SEND_HEADERS = 350,
  WEB_VIEW_INTERNAL_ON_CLOSE = 351,
  WEB_VIEW_INTERNAL_ON_COMPLETED = 352,
  WEB_VIEW_INTERNAL_ON_CONSOLE_MESSAGE = 353,
  WEB_VIEW_INTERNAL_ON_CONTENT_LOAD = 354,
  WEB_VIEW_INTERNAL_ON_DIALOG = 355,
  DELETED_WEB_VIEW_INTERNAL_ON_DROP_LINK = 356,
  WEB_VIEW_INTERNAL_ON_EXIT = 357,
  WEB_VIEW_INTERNAL_ON_EXIT_FULLSCREEN = 358,
  WEB_VIEW_INTERNAL_ON_FIND_REPLY = 359,
  WEB_VIEW_INTERNAL_ON_FRAME_NAME_CHANGED = 360,
  WEB_VIEW_INTERNAL_ON_HEADERS_RECEIVED = 361,
  WEB_VIEW_INTERNAL_ON_LOAD_ABORT = 362,
  WEB_VIEW_INTERNAL_ON_LOAD_COMMIT = 363,
  WEB_VIEW_INTERNAL_ON_LOAD_PROGRESS = 364,
  WEB_VIEW_INTERNAL_ON_LOAD_REDIRECT = 365,
  WEB_VIEW_INTERNAL_ON_LOAD_START = 366,
  WEB_VIEW_INTERNAL_ON_LOAD_STOP = 367,
  WEB_VIEW_INTERNAL_ON_NEW_WINDOW = 368,
  WEB_VIEW_INTERNAL_ON_PERMISSION_REQUEST = 369,
  WEB_VIEW_INTERNAL_ON_RESPONSE_STARTED = 370,
  WEB_VIEW_INTERNAL_ON_RESPONSIVE = 371,
  WEB_VIEW_INTERNAL_ON_SIZE_CHANGED = 372,
  WEB_VIEW_INTERNAL_ON_UNRESPONSIVE = 373,
  WEB_VIEW_INTERNAL_ON_ZOOM_CHANGE = 374,
  GUEST_VIEW_INTERNAL_ON_RESIZE = 375,
  LANGUAGE_SETTINGS_PRIVATE_ON_INPUT_METHOD_ADDED = 376,
  LANGUAGE_SETTINGS_PRIVATE_ON_INPUT_METHOD_REMOVED = 377,
  LANGUAGE_SETTINGS_PRIVATE_ON_SPELLCHECK_DICTIONARIES_CHANGED = 378,
  LANGUAGE_SETTINGS_PRIVATE_ON_CUSTOM_DICTIONARY_CHANGED = 379,
  CAST_DEVICES_PRIVATE_ON_UPDATE_DEVICES_REQUESTED = 380,
  CAST_DEVICES_PRIVATE_ON_START_CAST = 381,
  CAST_DEVICES_PRIVATE_ON_STOP_CAST = 382,
  CERTIFICATEPROVIDER_ON_CERTIFICATES_REQUESTED = 383,
  CERTIFICATEPROVIDER_ON_SIGN_DIGEST_REQUESTED = 384,
  WEB_VIEW_INTERNAL_ON_AUTH_REQUIRED = 385,
  WEB_VIEW_INTERNAL_ON_BEFORE_REDIRECT = 386,
  WEB_VIEW_INTERNAL_ON_ERROR_OCCURRED = 387,
  WEB_VIEW_INTERNAL_ON_SEND_HEADERS = 388,
  DELETED_EASY_UNLOCK_PRIVATE_ON_CONNECTION_STATUS_CHANGED = 389,
  DELETED_EASY_UNLOCK_PRIVATE_ON_DATA_RECEIVED = 390,
  DELETED_EASY_UNLOCK_PRIVATE_ON_SEND_COMPLETED = 391,
  DELETED_DISPLAY_SOURCE_ON_SINKS_UPDATED = 392,
  INPUT_IME_ON_COMPOSITION_BOUNDS_CHANGED = 393,
  INPUT_METHOD_PRIVATE_ON_IME_MENU_ACTIVATION_CHANGED = 394,
  INPUT_METHOD_PRIVATE_ON_IME_MENU_LIST_CHANGED = 395,
  INPUT_METHOD_PRIVATE_ON_IME_MENU_ITEMS_CHANGED = 396,
  BLUETOOTH_LOW_ENERGY_ON_CHARACTERISTIC_READ_REQUEST = 397,
  BLUETOOTH_LOW_ENERGY_ON_CHARACTERISTIC_WRITE_REQUEST = 398,
  BLUETOOTH_LOW_ENERGY_ON_DESCRIPTOR_READ_REQUEST = 399,
  BLUETOOTH_LOW_ENERGY_ON_DESCRIPTOR_WRITE_REQUEST = 400,
  ACCESSIBILITY_PRIVATE_ON_ACCESSIBILITY_GESTURE = 401,
  QUICK_UNLOCK_PRIVATE_ON_ACTIVE_MODES_CHANGED = 402,
  CLIPBOARD_ON_CLIPBOARD_DATA_CHANGED = 403,
  VIRTUAL_KEYBOARD_PRIVATE_ON_KEYBOARD_CLOSED = 404,
  FILE_MANAGER_PRIVATE_ON_APPS_UPDATED = 405,
  ACCESSIBILITY_PRIVATE_ON_TWO_FINGER_TOUCH_START = 406,
  ACCESSIBILITY_PRIVATE_ON_TWO_FINGER_TOUCH_STOP = 407,
  MEDIA_PERCEPTION_PRIVATE_ON_MEDIA_PERCEPTION = 408,
  NETWORKING_PRIVATE_ON_CERTIFICATE_LISTS_CHANGED = 409,
  LOCK_SCREEN_DATA_ON_DATA_ITEMS_AVAILABLE = 410,
  WEB_VIEW_INTERNAL_ON_AUDIO_STATE_CHANGED = 411,
  AUTOMATION_INTERNAL_ON_ACTION_RESULT = 412,
  OMNIBOX_ON_DELETE_SUGGESTION = 413,
  VIRTUAL_KEYBOARD_PRIVATE_ON_KEYBOARD_CONFIG_CHANGED = 414,
  PASSWORDS_PRIVATE_ON_PASSWORDS_FILE_EXPORT_PROGRESS = 415,
  SAFE_BROWSING_PRIVATE_ON_POLICY_SPECIFIED_PASSWORD_REUSE_DETECTED = 416,
  SAFE_BROWSING_PRIVATE_ON_POLICY_SPECIFIED_PASSWORD_CHANGED = 417,
  SAFE_BROWSING_PRIVATE_ON_DANGEROUS_DOWNLOAD_OPENED = 418,
  SAFE_BROWSING_PRIVATE_ON_SECURITY_INTERSTITIAL_SHOWN = 419,
  SAFE_BROWSING_PRIVATE_ON_SECURITY_INTERSTITIAL_PROCEEDED = 420,
  ACCESSIBILITY_PRIVATE_ON_SELECT_TO_SPEAK_STATE_CHANGE_REQUESTED = 421,
  INPUT_METHOD_PRIVATE_ON_FOCUS = 422,
  DELETED_SYSTEM_POWER_SOURCE_ONPOWERCHANGED = 423,
  WEB_REQUEST_ON_ACTION_IGNORED = 424,
  ARC_APPS_PRIVATE_ON_INSTALLED = 425,
  FILE_MANAGER_PRIVATE_ON_CROSTINI_CHANGED = 426,
  AUTOMATION_INTERNAL_ON_GET_TEXT_LOCATION_RESULT = 427,
  INPUT_METHOD_PRIVATE_ON_SETTINGS_CHANGED = 428,
  INPUT_METHOD_PRIVATE_ON_SCREEN_PROJECTION_CHANGED = 429,
  STORAGE_LOCAL_ON_CHANGE = 430,
  STORAGE_SYNC_ON_CHANGE = 431,
  STORAGE_MANAGED_ON_CHANGE = 432,
  AUTOFILL_PRIVATE_ON_LOCAL_CREDIT_CARD_LIST_CHANGED_DEPRECATED = 433,
  AUTOFILL_PRIVATE_ON_SERVER_CREDIT_CARD_LIST_CHANGED_DEPRECATED = 434,
  ACCESSIBILITY_PRIVATE_ON_ANNOUNCE_FOR_ACCESSIBILITY = 435,
  MIME_HANDLER_PRIVATE_SAVE = 436,
  RUNTIME_ON_CONNECT_NATIVE = 437,
  ACTION_ON_CLICKED = 438,
  ACCESSIBILITY_PRIVATE_ON_SWITCH_ACCESS_COMMAND = 439,
  ACCESSIBILITY_PRIVATE_FIND_SCROLLABLE_BOUNDS_FOR_POINT = 440,
  LOGIN_STATE_ON_SESSION_STATE_CHANGED = 441,
  PRINTING_METRICS_ON_PRINT_JOB_FINISHED = 442,
  AUTOTESTPRIVATE_ON_CLIPBOARD_DATA_CHANGED = 443,
  AUTOFILL_PRIVATE_ON_PERSONAL_DATA_CHANGED = 444,
  PRINTING_ON_JOB_STATUS_CHANGED = 445,
  DECLARATIVE_NET_REQUEST_ON_RULE_MATCHED_DEBUG = 446,
  TERMINAL_PRIVATE_ON_SETTINGS_CHANGED = 447,
  DELETED_AUTOFILL_ASSISTANT_PRIVATE_ON_ACTIONS_CHANGED = 448,
  DELETED_AUTOFILL_ASSISTANT_PRIVATE_ON_STATUS_MESSAGE_CHANGED = 449,
  BLUETOOTH_PRIVATE_ON_DEVICE_ADDRESS_CHANGED = 450,
  PASSWORDS_PRIVATE_ON_ACCOUNT_STORAGE_OPT_IN_STATE_CHANGED = 451,
  ACCESSIBILITY_PRIVATE_ON_CUSTOM_SPOKEN_FEEDBACK_TOGGLED = 452,
  PASSWORDS_PRIVATE_ON_COMPROMISED_CREDENTIALS_INFO_CHANGED = 453,
  TERMINAL_PRIVATE_ON_A11Y_STATUS_CHANGED = 454,
  PASSWORDS_PRIVATE_ON_PASSWORD_CHECK_STATUS_CHANGED = 455,
  INPUT_IME_ON_ASSISTIVE_WINDOW_BUTTON_CLICKED = 456,
  INPUT_IME_ON_SUGGESTIONS_CHANGED = 457,
  INPUT_IME_ON_INPUT_METHOD_OPTIONS_CHANGED = 458,
  CERTIFICATEPROVIDER_ON_CERTIFICATES_UPDATE_REQUESTED = 459,
  CERTIFICATEPROVIDER_ON_SIGNATURE_REQUESTED = 460,
  WINDOWS_ON_BOUNDS_CHANGED = 461,
  DELETED_WALLPAPER_PRIVATE_ON_CLOSE_PREVIEW_WALLPAPER = 462,
  PASSWORDS_PRIVATE_ON_WEAK_CREDENTIALS_CHANGED = 463,
  ACCESSIBILITY_PRIVATE_ON_MAGNIFIER_BOUNDS_CHANGED = 464,
  FILE_MANAGER_PRIVATE_ON_PIN_TRANSFERS_UPDATED = 465,
  ACCESSIBILITY_PRIVATE_ON_POINT_SCAN_SET = 466,
  ACCESSIBILITY_PRIVATE_ON_SELECT_TO_SPEAK_PANEL_ACTION = 467,
  FILE_MANAGER_PRIVATE_ON_TABLET_MODE_CHANGED = 468,
  VIRTUAL_KEYBOARD_PRIVATE_ON_CLIPBOARD_HISTORY_CHANGED = 469,
  VIRTUAL_KEYBOARD_PRIVATE_ON_CLIPBOARD_ITEM_UPDATED = 470,
  TAB_GROUPS_ON_CREATED = 471,
  TAB_GROUPS_ON_MOVED = 472,
  TAB_GROUPS_ON_REMOVED = 473,
  TAB_GROUPS_ON_UPDATED = 474,
  FILE_MANAGER_PRIVATE_ON_DRIVE_CONFIRM_DIALOG = 475,
  TTS_ENGINE_ON_SPEAK_WITH_AUDIO_STREAM = 476,
  ACCESSIBILITY_PRIVATE_ON_SHOW_CHROMEVOX_TUTORIAL = 477,
  STORAGE_SESSION_ON_CHANGE = 478,
  ACCESSIBILITY_PRIVATE_ON_TOGGLE_DICTATION = 479,
  WEB_AUTHENTICATION_PROXY_ON_ISUVPAA_REQUEST = 480,
  SPEECH_RECOGNITION_PRIVATE_ON_STOP = 481,
  SPEECH_RECOGNITION_PRIVATE_ON_RESULT = 482,
  SPEECH_RECOGNITION_PRIVATE_ON_ERROR = 483,
  FILE_MANAGER_PRIVATE_ON_IO_TASK_PROGRESS_STATUS = 484,
  INPUT_METHOD_PRIVATE_ON_TOUCH = 485,
  WEB_AUTHENTICATION_PROXY_ON_CREATE_REQUEST = 486,
  WEB_AUTHENTICATION_PROXY_REQUEST_CANCELLED = 487,
  WEB_AUTHENTICATION_PROXY_ON_GET_REQUEST = 488,
  DEVELOPER_PRIVATE_ON_USER_SITE_SETTINGS_CHANGED = 489,
  TERMINAL_PRIVATE_ON_PREF_CHANGED = 490,
  WEB_AUTHENTICATION_PROXY_ON_REMOTE_SESSION_STATE_CHANGE = 491,
  LOGIN_ON_REQUEST_EXTERNAL_LOGOUT = 492,
  LOGIN_ON_EXTERNAL_LOGOUT_DONE = 493,
  ACCESSIBILITY_PRIVATE_ON_PUMPKIN_INSTALLED = 494,
  ENTERPRISE_REMOTE_APPS_ON_REMOTE_APP_LAUNCHED = 495,
  INPUT_METHOD_PRIVATE_ON_CARET_BOUNDS_CHANGED = 496,
  PASSWORDS_PRIVATE_ON_PASSWORD_MANAGER_AUTH_TIMEOUT = 497,

  // Adblock's below:
  ADBLOCK_PRIVATE_AD_BLOCKED = 995,
  ADBLOCK_PRIVATE_AD_ALLOWED = 996,
  ADBLOCK_PRIVATE_SUBSCRIPTION_UPDATED = 997,
  ADBLOCK_PRIVATE_PAGE_ALLOWED = 998,
  ADBLOCK_PRIVATE_POPUP_BLOCKED = 999,
  ADBLOCK_PRIVATE_POPUP_ALLOWED = 1000,

  // Last entry: Add new entries above, then run:
  // tools/metrics/histograms/update_extension_histograms.py
  ENUM_BOUNDARY
};

}  // namespace events
}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_EXTENSION_EVENT_HISTOGRAM_VALUE_H_
