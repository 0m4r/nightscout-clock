// Define setting options, value conversions, and validation independently of DOM rendering.
// Normalize incoming configuration for the form and prepare its outgoing save payload.

/** @typedef {'mgdl' | 'mmol' | ''} GlucoseUnits Empty before units have been selected. */
/** @typedef {'display' | 'glucose' | 'alarms' | 'system'} SettingsTab */
/** @typedef {[string | number, string]} SelectOption A stored value and its visible label. */
/** @typedef {Record<string, string>} ValidationErrors Error messages indexed by setting key. */

/**
 * One daily change of clock face and brightness, using the clock's configured timezone.
 * @typedef {Object} FaceScheduleEntry
 * @property {string} time Start time as HH:MM; may be empty while editing.
 * @property {string} face Registered clock-face ID.
 * @property {number} brightness Manual level 1–10, or automatic mode 100/101.
 */
/**
 * A recurring alarm window; an end earlier than its start crosses midnight.
 * @typedef {Object} AlertWindow
 * @property {string} days Start weekdays as digits, Sunday=0 through Saturday=6.
 * @property {string} from Start time as HH:MM; may be empty while editing.
 * @property {string} to End time as HH:MM; may be empty while editing.
 */
/**
 * Settings loaded from or saved to the clock; numeric fields may hold unfinished input text in the draft.
 * Additional firmware settings pass through the page using their existing JSON keys.
 * @typedef {Record<string, JsonValue> & {
 *   units: GlucoseUnits,
 *   face_schedule_enabled?: boolean,
 *   face_schedule?: FaceScheduleEntry[],
 *   face_cycle_enabled: boolean,
 *   inactive_faces: string[],
 *   brightness_level: number,
 *   default_face: string,
 *   tz: string,
 *   tz_libc: string,
 *   alarm_high_alert_windows: AlertWindow[],
 *   alarm_low_alert_windows: AlertWindow[],
 *   alarm_urgent_low_alert_windows: AlertWindow[]
 * }} ClockConfig
 */
/**
 * Components edited separately in the Nightscout address controls.
 * @typedef {Object} NightscoutAddress
 * @property {string} protocol URL scheme, normally http or https.
 * @property {string} host Hostname or IP address without scheme or port.
 * @property {string} port Port text, or empty to use the scheme's default.
 */
/**
 * One choice in the timezone picker, as supplied by tzdata.json.
 * @typedef {Object} TimezoneEntry
 * @property {string} name Display name, e.g. Europe/Amsterdam.
 * @property {string} value POSIX/libc timezone rule used by the firmware.
 */
/**
 * Shared metadata used to build a high, low, or urgent-low alarm card.
 * @typedef {Object} AlarmDescriptor
 * @property {'high' | 'low' | 'urgent_low'} t Alarm key suffix.
 * @property {string} name Display name.
 * @property {string} compare Label describing the threshold comparison.
 * @property {string} defaultMelody Default RTTTL sound.
 */

// Stable IDs must match BGDisplayManager::setup(); array order controls UI display order.
const FACES = [
  { id: "simple", name: "Simple" },
  { id: "graph", name: "Full glucose graph" },
  { id: "graph_and_bg", name: "Glucose graph and value" },
  { id: "big_text", name: "Big text" },
  { id: "value_and_diff", name: "Value and delta" },
  { id: "clock", name: "Current time and BG value" },
  { id: "unicorn", name: "Unicorn" },
  { id: "time_only", name: "Time only" },
  { id: "diagnostics", name: "Diagnostics" },
  { id: "battery_uptime", name: "Battery and uptime" },
  { id: "big_text_rainbow", name: "Rainbow big text" },
  { id: "smiley", name: "Smiley" },
  { id: "simple_dark", name: "Simple (dark)" },
  { id: "race_car", name: "Race car" },
  { id: "dragon", name: "Dragon" },
  { id: "big_text_dark", name: "Big text (dark)" },
  { id: "rainbow_clock", name: "Rainbow clock" },
];

// The config stores the faces switched off, so a face added later starts active.
const activeFaceIds = (inactive) =>
  FACES.map((f) => f.id).filter((id) => !(inactive || []).includes(id));

// "carelink" is not a clock source: choosing it explains the xDrip+ and Nightscout bridge instead.
const SOURCES = [
  ["dexcom", "Dexcom"],
  ["nightscout", "Nightscout"],
  ["librelinkup", "LibreLinkUp"],
  ["carelink", "Medtronic CareLink"],
  ["medtrum", "Medtrum Easy Follow"],
  ["api", "API (see project documentation)"],
];
const DEXCOM_SERVERS = [
  ["us", "US"],
  ["ous", "Non-US"],
  ["jp", "Japan"],
];
const LLU_REGIONS = [
  ["AE", "United Arab Emirates"],
  ["AP", "Asia Pacific"],
  ["AU", "Australia"],
  ["CA", "Canada"],
  ["DE", "Germany"],
  ["EU", "Europe"],
  ["EU2", "Europe 2"],
  ["FR", "France"],
  ["JP", "Japan"],
  ["US", "United States"],
  ["LA", "Latin America"],
  ["RU", "Russia"],
];
const UNITS = [
  ["mgdl", "mg/dl"],
  ["mmol", "mmol/l"],
];
// Lowest range first. `color` is the key of the range's color; `limit` is the key that ends (or starts) the range.
const BANDS = [
  {
    name: "Urgent low",
    color: "bg_color_urgent_low",
    limit: "low_urgent_mgdl",
    label: "Up to",
  },
  { name: "Low", color: "bg_color_low", limit: "low_mgdl", label: "Up to" },
  { name: "In range", color: "bg_color_normal", limit: null, label: "" },
  { name: "High", color: "bg_color_high", limit: "high_mgdl", label: "From" },
  {
    name: "Urgent high",
    color: "bg_color_urgent_high",
    limit: "high_urgent_mgdl",
    label: "From",
  },
];
// No black, which never lights, and no gray, which is dark at the lowest brightness.
const BAND_COLORS = [
  ["green", "Green"],
  ["yellow", "Yellow"],
  ["red", "Red"],
  ["cyan", "Cyan"],
  ["blue", "Blue"],
  ["magenta", "Magenta"],
  ["white", "White"],
];
// Ranges that must differ, so an out-of-range reading never looks in range and an urgent one never looks milder.
const BAND_COLORS_APART = [
  ["bg_color_normal", "bg_color_urgent_low"],
  ["bg_color_normal", "bg_color_low"],
  ["bg_color_normal", "bg_color_high"],
  ["bg_color_normal", "bg_color_urgent_high"],
  ["bg_color_urgent_low", "bg_color_low"],
  ["bg_color_urgent_high", "bg_color_high"],
];
const LIMIT_KEYS = [
  "low_urgent_mgdl",
  "low_mgdl",
  "high_mgdl",
  "high_urgent_mgdl",
];
const OLD_DATA_COLORS = [
  ["gray", "Gray"],
  ["cyan", "Cyan"],
  ["magenta", "Magenta"],
  ["blue", "Blue"],
];
// No red, yellow or green: those are glucose colors.
const EARLY_STALE_COLORS = [
  ["off", "Off"],
  ["cyan", "Cyan"],
  ["blue", "Blue"],
  ["magenta", "Magenta"],
];
const EARLY_STALE_MINUTES = [
  [6, "6 min"],
  [10, "10 min"],
  [15, "15 min"],
];
const MANE_MODES = [
  ["still", "Still"],
  ["moving", "Moving"],
];
const MANE_FLOWS = [
  ["down", "Top to bottom"],
  ["back", "Colors scroll back"],
  ["run", "Light runs along the bands"],
];
const ANIMATION_SPEEDS = [
  ["calm", "Calm"],
  ["normal", "Normal"],
  ["lively", "Lively"],
];
// Every color the firmware can draw except black, which is invisible on the black panel. The glucose
// colors stay on the trend arrow, so the number keeps whichever of these is chosen.
const DARK_VALUE_COLORS = [
  ["white", "White"],
  ["cyan", "Cyan"],
  ["blue", "Blue"],
  ["magenta", "Magenta"],
  ["gray", "Gray"],
  ["green", "Green"],
  ["yellow", "Yellow"],
  ["red", "Red"],
];
const CYCLE_INTERVALS = [
  [10, "10 s"],
  [30, "30 s"],
  [60, "1 min"],
  [120, "2 min"],
  [180, "3 min"],
  [300, "5 min"],
];
const TIME_FORMATS = [
  ["24", "24h"],
  ["12", "AM/PM"],
];
const SNOOZES = [
  [5, "5 minutes"],
  [10, "10 minutes"],
  [15, "15 minutes"],
  [30, "30 minutes"],
  [60, "1 hour"],
  [120, "2 hours"],
  [0, "Until next trigger"],
];
const REPEATS = [
  [60, "1 min"],
  [120, "2 min"],
  [300, "5 min"],
];
const WIFI_TYPES = [
  ["wpa_psk", "WPA-PSK"],
  ["wpa_eap", "WPA-EAP"],
];
const BRIGHTNESS_MODES = [
  ["auto_linear", "Auto: balanced", 100],
  ["auto_dimmed", "Auto: for darker rooms", 101],
  ["manual", "Manual", null],
];
// Mon..Sun as the firmware numbers them (Sunday = 0).
const DAYS = [
  [1, "Mon"],
  [2, "Tue"],
  [3, "Wed"],
  [4, "Thu"],
  [5, "Fri"],
  [6, "Sat"],
  [0, "Sun"],
];

const ALARMS = [
  {
    t: "high",
    name: "High",
    compare: "Alert when above",
    defaultMelody: "high:d=4,o=5,b=125:4e7,p,4e7",
  },
  {
    t: "low",
    name: "Low",
    compare: "Alert when below",
... (+814 lines) [see remaining: rtk proxy git show ':2:web/src/js/schema.js' | tail -n +264]
