// ─── Constants ────────────────────────────────────────────────
const CLOCK_INTERVAL_MS = 500;   // setTime polling cadence
const SLEEP_DEBOUNCE_MS = 1500;  // aggregation delay before sending sleep changes
const RELOAD_DELAY_MS = 2000;  // time to reload page after saving connection settings

// Wave animation timing
const WAVE_PHASE1_MS = 7000;  // wait for hands to settle vertical before spin phase
const WAVE_PHASE2_MS = 9000;  // duration of spin phase
const WAVE_STEP_DELAY_MS = 400;   // per-column ripple step delay

// Hand travel direction for setHalfDigit(). The base type selects which arc to travel;
// the variant suffix (0 / 1 / 2) adds 0 / 1 / 2 extra full 360° rotations for a spin effect.
const DIR_CW = 0;   // clockwise
const DIR_CW_SPIN1 = 1;   // clockwise + 1 full spin
const DIR_CW_SPIN2 = 2;   // clockwise + 2 full spins
const DIR_CCW = 3;   // counter-clockwise
const DIR_CCW_SPIN1 = 4;   // counter-clockwise + 1 full spin
const DIR_CCW_SPIN2 = 5;   // counter-clockwise + 2 full spins
const DIR_MIN = 6;   // shortest arc
const DIR_MIN_SPIN1 = 7;   // shortest arc + 1 full spin
const DIR_MIN_SPIN2 = 8;   // shortest arc + 2 full spins
const DIR_MAX = 9;   // longest arc
const DIR_MAX_SPIN1 = 10;  // longest arc + 1 full spin
const DIR_MAX_SPIN2 = 11;  // longest arc + 2 full spins

// ─── State ────────────────────────────────────────────────────
// Sleep schedule: [7 days][24 hours], 1 = clock sleeps that hour
let sleep = [Array(24).fill(0), Array(24).fill(0), Array(24).fill(0), Array(24).fill(0),
Array(24).fill(0), Array(24).fill(0), Array(24).fill(0)]

let selectedMode = 0          // 0=OFF, 1=LAZY, 2=ANIMATED
let selectedAnimation = 0          // 0=WAVE, 1=CHAOS, 2=CIRCLE, 3=SPIRAL, 4=ATTRACT, 5=CYCLE
let selectedCycleType = 0          // 0=SEQUENTIAL, 1=RANDOM
let currentCycleIndex = 0          // cycle position, mirrors firmware state
let selectedClock = undefined  // index of the clock with visible overlay buttons
let selectedDay = undefined  // index of the active day in the sleep schedule
let selectedConnection = undefined  // 0=HOTSPOT, 1=EXTERNAL
let ssid = ""
let password = ""

// ─── DOM Generation ──────────────────────────────────────────
/** Returns the HTML for a single clock: an SVG face with four adjustment buttons. */
function clock(index) {
  return (
    `<div id="clock-${index}" class="clock-box hidden">` +
    `<svg class="clock clock--${index}" width="100" height="100" viewBox="0 0 100 100" onclick="selectClock(${index})">` +
    `<path class="clock-smallHand" d="M50,47 C48.3431458,47 47,48.3431458 47,50 C47,51.6568542 48.3431458,53 50,53 L95,53 L95,47 L50,47 Z" stroke="none" fill="#FFF" fill-rule="evenodd"></path>` +
    `<path class="clock-largeHand" d="M50,47 C48.3431458,47 47,48.3431458 47,50 C47,51.6568542 48.3431458,53 50,53 L100,53 L100,47 L50,47 Z" stroke="none" fill="#FFF" fill-rule="evenodd"></path>` +
    `</svg>` +
    `<div class="btn-clock btn-clock-tl" onclick="adjustHand(${index}, 0, 1)">H+</div>` +
    `<div class="btn-clock btn-clock-tr" onclick="adjustHand(${index}, 1, 0)">M+</div>` +
    `<div class="btn-clock btn-clock-bl" onclick="adjustHand(${index}, 0, -1)">H-</div>` +
    `<div class="btn-clock btn-clock-br" onclick="adjustHand(${index}, -1, 0)">M-</div>` +
    `</div>`
  )
}

/** Renders the mode buttons (OFF / LAZY / ANIMATED) into #modes. */
function genModes() {
  let modes = ["OFF", "LAZY", "ANIMATED"]
  let html = ""
  let i = 0
  for (let m of modes)
    html += `<div id="mode-${i}" class="btn ${i === selectedMode ? "active" : ""}" onclick="selectMode(${i++})">${m}</div>`
  document.getElementById("modes").innerHTML = html
}

/** Renders the animation buttons (WAVE / CHAOS / CIRCLE / SPIRAL / ATTRACT / CYCLE) into #animations. */
function genAnimations() {
  let anims = ["WAVE", "CHAOS", "CIRCLE", "SPIRAL", "ATTRACT", "LOOM", "CYCLE"]
  let html = ""
  let i = 0
  for (let a of anims)
    html += `<div id="anim-${i}" class="btn ${i === selectedAnimation ? "active" : ""}" onclick="selectAnimation(${i++})">${a}</div>`
  document.getElementById("animations").innerHTML = html
  genCycleOptions()
}

/** Renders the cycle-type buttons (SEQ / RAND) into #cycle-options; shows them only when CYCLE is active. */
function genCycleOptions() {
  let types = ["SEQ", "RAND"]
  let html = ""
  let i = 0
  for (let t of types)
    html += `<div id="cycle-${i}" class="btn ${i === selectedCycleType ? "active" : ""}" onclick="selectCycleType(${i++})">${t}</div>`
  document.getElementById("cycle-options").innerHTML = html
  if (selectedAnimation === 6)
    document.getElementById("cycle-options").classList.remove("hidden")
  else
    document.getElementById("cycle-options").classList.add("hidden")
}

/** Renders all 24 clocks grouped by digit into #art. */
function genClocks() {
  let html = `<div class="half-digit">`
  for (let i = 0; i < 24; i++) {
    if (i % 3 == 0 && i != 0) {
      html += "</div>"
      html += `<div class="half-digit">`
    }
    html += clock(i)
  }
  html += "</div>"
  document.getElementById("art").innerHTML = html
}

/** Renders the day buttons (MON–SUN) into #days. */
function genDays() {
  let days = ["MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"]
  let html = ""
  let i = 0
  for (let day of days)
    html += `<div id="day-${i}" class="btn" onclick="selectDay(${i++})">${day}</div>`
  document.getElementById("days").innerHTML = html
}

/** Renders the 24-hour checkbox grid for the given day into #hours. */
function genHours(day) {
  let html = `<div class="hours-box"><div>`
  for (let i = 0; i <= 12; i++)
    html += `<div class="hour-text">${i}</div>`
  html += `</div><div>`
  for (let i = 0; i < 12; i++)
    html += `<div id="hour-${i}" class="checkbox ${sleep[day][i] === 1 ? "selected" : ""}" onclick="selectHour(${i})"></div>`
  html += "</div><div>"
  for (let i = 12; i <= 24; i++)
    html += `<div class="hour-text">${i % 24}</div>`
  html += `</div><div>`
  for (let i = 12; i < 24; i++)
    html += `<div id="hour-${i}" class="checkbox ${sleep[day][i] === 1 ? "selected" : ""}" onclick="selectHour(${i})"></div>`
  html += "</div></div>"
  document.getElementById("hours").innerHTML = html
}

// ─── Event Handlers ──────────────────────────────────────────
/** Switches the active mode; starts or stops the clock loop and shows/hides the animation section. */
function selectMode(mode) {
  mode === 0 ? stopClock() : startClock()
  if (selectedMode !== undefined)
    document.getElementById("mode-" + selectedMode).classList.remove("active")
  selectedMode = mode
  document.getElementById("mode-" + selectedMode).classList.add("active")
  if (mode === 2)
    document.getElementById("animation-section").classList.remove("hidden")
  else
    document.getElementById("animation-section").classList.add("hidden")
  saveMode(selectedMode)
}

/** Switches the active animation and resets the cycle index to 0. */
function selectAnimation(anim) {
  if (selectedAnimation !== undefined)
    document.getElementById("anim-" + selectedAnimation).classList.remove("active")
  selectedAnimation = anim
  document.getElementById("anim-" + selectedAnimation).classList.add("active")
  currentCycleIndex = 0
  genCycleOptions()
  saveAnimation(selectedAnimation, selectedCycleType)
}

/** Switches between SEQUENTIAL and RANDOM cycle types. */
function selectCycleType(type) {
  if (selectedCycleType !== undefined)
    document.getElementById("cycle-" + selectedCycleType).classList.remove("active")
  selectedCycleType = type
  document.getElementById("cycle-" + selectedCycleType).classList.add("active")
  saveAnimation(selectedAnimation, selectedCycleType)
}

/** Selects a day and renders its sleep schedule; clicking the same day again deselects it. */
function selectDay(day) {
  deselectDay()
  if (selectedDay !== day) {
    selectedDay = day
    document.getElementById("day-" + selectedDay).classList.add("day-active")
    genHours(day)
  }
}

/** Collapses the hours grid and removes the day button highlight. */
function deselectDay() {
  if (selectedDay !== undefined)
    document.getElementById("day-" + selectedDay).classList.remove("day-active")
  selectedDay = undefined
  document.getElementById("hours").innerHTML = ""
}

/** Toggles one hour in the sleep schedule and debounces saving to the server. */
function selectHour(index) {
  let state = sleep[parseInt(selectedDay)][index]
  sleep[parseInt(selectedDay)][index] = state === 1 ? 0 : 1
  if (state === 0)
    document.getElementById("hour-" + index).classList.add("selected")
  else
    document.getElementById("hour-" + index).classList.remove("selected")
  saveSleepTime(selectedDay, sleep[parseInt(selectedDay)])
}

/** Hides the adjustment buttons overlay for the previously selected clock. */
function deselectClock() {
  if (selectedClock !== undefined)
    document.getElementById("clock-" + selectedClock).classList.add("hidden")
  selectedClock = undefined
}

/** Shows the adjustment buttons overlay for the clicked clock. */
function selectClock(index) {
  deselectClock()
  selectedClock = index
  document.getElementById("clock-" + selectedClock).classList.remove("hidden")
}

/** Switches WiFi mode; shows the SSID/password form for EXTERNAL, saves immediately for HOTSPOT. */
function selectConnection(mode) {
  // Remove last selection
  if (selectedConnection !== undefined) {
    document.getElementById("con-" + selectedConnection).classList.remove("active")
    document.getElementById("external").classList.add("hidden")
  }
  // Hotspot mode: save immediately when switching away from external
  if (mode === 0 && selectedConnection !== mode && selectedConnection !== undefined) {
    selectedConnection = mode
    saveConnection()
  } else {
    selectedConnection = mode
  }
  if (mode === 1) // Show SSID and password fields
    document.getElementById("external").classList.remove("hidden")
  document.getElementById("con-" + selectedConnection).classList.add("active")
}

// Clicking the page background deselects any active clock or day
document.addEventListener("click", function (e) {
  if (e.target.id === "body") {
    deselectClock()
    deselectDay()
  }
})

// Build initial DOM
genClocks()
genModes()
genAnimations()
genDays()

// ─── API Layer ────────────────────────────────────────────────
/** POSTs the current browser date and time to /time so the firmware can sync its RTC. */
function sendDate() {
  const d = new Date()
  let formData = new FormData()
  formData.append("h", d.getHours().toString())
  formData.append("m", d.getMinutes().toString())
  formData.append("s", d.getSeconds().toString())
  formData.append("D", d.getDate().toString())
  formData.append("M", (d.getMonth() + 1).toString())
  formData.append("Y", d.getFullYear().toString())
  formData.append("timezone", (d.getTimezoneOffset() / -60).toString())
  fetch("/time", {
    method: "post",
    body: formData,
  })
}

/** GETs /config and syncs the entire UI to the current firmware state. */
async function updateConfig() {
  let response = await fetch("/config", {
    method: "get",
  })
  let res = await response.json()
  sleep = res.sleep_time
  if (selectedDay !== undefined)
    genHours(selectedDay)
  if (res.clock_animation !== undefined) {
    selectedAnimation = res.clock_animation
    selectedCycleType = res.cycle_type || 0
    genAnimations()
  }
  selectMode(res.clock_mode)
  selectConnection(res.wireless_mode)
  document.getElementById("ssid").value = ssid = res.ssid
  document.getElementById("password").value = password = res.password
}

/** POSTs a single-step hand adjustment (±1) for the specified clock index. */
function adjustHand(index, m_amount, h_amount) {
  let f = new FormData()
  f.append("index", index.toString())
  f.append("m_amount", m_amount.toString())
  f.append("h_amount", h_amount.toString())
  fetch("/adjust", {
    method: "post",
    body: f,
  })
}

/** POSTs the selected clock mode (0=OFF, 1=LAZY, 2=ANIMATED) to /mode. */
function saveMode(mode) {
  let f = new FormData()
  f.append("mode", mode.toString())
  fetch("/mode", {
    method: "post",
    body: f,
  })
}

/** POSTs the selected animation index and cycle type to /animation. */
function saveAnimation(anim, cycleType) {
  let f = new FormData()
  f.append("animation", anim.toString())
  f.append("cycle_type", cycleType.toString())
  fetch("/animation", {
    method: "post",
    body: f,
  })
}

// Used to aggregate rapid sleep toggles: only the last change per day within the debounce
// window is sent, avoiding firmware overload when the user quickly clicks multiple hours.
let lastSleepDay = undefined
let sleepTimeout = undefined

/** Debounces /sleep POST requests to avoid overwhelming the firmware with rapid checkbox toggles. */
function saveSleepTime(day, hours) {
  if (lastSleepDay === day)
    clearTimeout(sleepTimeout);
  lastSleepDay = day
  sleepTimeout = setTimeout(() => {
    let f = new FormData()
    f.append("day", day.toString())
    for (let i = 0; i < 24; i++)
      f.append("h" + i, hours[i].toString())
    fetch("/sleep", {
      method: "post",
      body: f
    })
  }, SLEEP_DEBOUNCE_MS)
}

/** POSTs the WiFi credentials and mode to /connection, then reloads the page to reconnect. */
async function saveConnection() {
  ssid = document.getElementById("ssid").value;
  password = document.getElementById("password").value;
  let f = new FormData()
  f.append("mode", selectedConnection.toString())
  f.append("ssid", ssid)
  f.append("password", password)
  await fetch("/connection", {
    method: "post",
    body: f,
  })
  setTimeout(() => location.reload(), RELOAD_DELAY_MS)
}

// Sync time and load configuration on page load
sendDate()
updateConfig()

// ─── Digit Data ───────────────────────────────────────────────
// Each digit is six [hourAngle, minuteAngle] pairs, one per clock, in reading order
// (top-left to bottom-right within each half-digit column).

// Both hands pointing down: used for OFF/sleep state
const digit_stop = [
  [270, 270], [270, 270], [270, 270],
  [270, 270], [270, 270], [270, 270]
]

// Both hands vertical (up + down): forms two parallel vertical lines
const digit_II = [
  [270, 90], [270, 90], [270, 90],
  [270, 90], [270, 90], [270, 90]
]

/*
Starting position for the "Loom" animation.
Top and bottom rows point both hands exactly horizontal in opposite directions
Middle row points both hands in the same horizontal direction (to the right)
*/
const digit_loom = [
  [0, 180], [0, 0], [0, 180],
  [0, 180], [0, 0], [0, 180]
]

// Digit shapes 0–9; each entry is six [hourAngle, minuteAngle] pairs in reading order
const digits = [
  [ /* 0 */[270, 0], [270, 90], [0, 90], [270, 180], [270, 90], [180, 90]],
  [ /* 1 */[225, 225], [225, 225], [225, 225], [270, 270], [270, 90], [90, 90]],
  [ /* 2 */[0, 0], [270, 0], [90, 0], [180, 270], [90, 180], [180, 180]],
  [ /* 3 */[0, 0], [0, 0], [0, 0], [180, 270], [180, 90], [180, 90]],
  [ /* 4 */[270, 270], [90, 0], [225, 225], [270, 270], [270, 90], [90, 90]],
  [ /* 5 */[270, 0], [90, 0], [0, 0], [180, 180], [270, 180], [90, 180]],
  [ /* 6 */[270, 0], [270, 90], [90, 0], [180, 180], [270, 180], [90, 180]],
  [ /* 7 */[0, 0], [225, 225], [225, 225], [270, 180], [270, 90], [90, 90]],
  [ /* 8 */[270, 0], [90, 0], [90, 0], [270, 180], [90, 180], [90, 180]],
  [ /* 9 */[270, 0], [0, 90], [0, 0], [270, 180], [270, 90], [90, 180]]
]

// ─── Animation Engine ─────────────────────────────────────────
// current_state: logical target angle of each hand, wrapped to [0, 360°).
//   Used as the reference for computing the next angular delta.
// anim_state: cumulative unwrapped CSS animation angle for each hand.
//   CSS transitions animate the difference between consecutive values, so this must
//   never be snapped — only incrementally updated.
// Both are shaped [4 digits][6 clocks per digit][2 hands: hour, minute].
let anim_state = Array(4).fill().map(() => Array(6).fill().map(() => [90, 90]))
let current_state = Array(4).fill().map(() => Array(6).fill().map(() => [270, 270]))

/** Sets the CSS animation target for one clock SVG. */
function setHands(id, h, m, sec) {
  const clock = document.querySelector(`.clock--${id}`)
  clock.style.setProperty(`--small-hand`, `${h}deg`)
  clock.style.setProperty(`--large-hand`, `${m}deg`)
  clock.style.setProperty(`--animation-time`, `${sec}s`)
}

/** Returns the angular delta (degrees) traveling counter-clockwise from currentAngle to targetAngle. */
function calcAngleCCW(currentAngle, targetAngle) {
  let delta = (targetAngle - currentAngle) % 360
  return Math.abs((delta <= 0) ? -delta : 360 - delta)
}

/** Returns the angular delta (degrees) traveling clockwise from currentAngle to targetAngle. */
function calcAngleCW(currentAngle, targetAngle) {
  let delta = (targetAngle - currentAngle) % 360
  return Math.abs((delta < 0) ? 360 + delta : delta)
}

/**
 * Animates one half-digit (3 clocks) to a new set of hand angles.
 *
 * Updates both current_state (logical angle, 0–360°) and anim_state (cumulative CSS angle)
 * for each hand. The direction constant controls which arc to travel and how many extra full
 * 360° spins to add on top (see DIR_* constants). The extra spins come from `direction % 3`
 * (0, 1, or 2), which is multiplied by 360° and added to the CSS delta.
 *
 * @param {number}     index     Half-digit index 0–7 (index/2 = digit, index%2 = top/bottom half)
 * @param {number[][]} values    3 × [hourAngle, minuteAngle] target pairs for the 3 clocks
 * @param {number}     direction One of the DIR_* constants
 * @param {number}     secs      CSS transition duration in seconds
 */
function setHalfDigit(index, values, direction, secs) {
  for (let x = 0; x < 3; x++) {
    const curr = current_state[Math.floor(index / 2)][x + (index % 2) * 3]
    const anim = anim_state[Math.floor(index / 2)][x + (index % 2) * 3]

    const d1 = calcAngleCW(curr[0], values[x][0])
    const d2 = calcAngleCCW(curr[0], values[x][0])
    const d3 = calcAngleCW(curr[1], values[x][1])
    const d4 = calcAngleCCW(curr[1], values[x][1])
    const multiplier = (360 * (direction % 3))  // extra full rotations from the direction variant

    if (direction <= 2) { // CLOCKWISE
      curr[0] = (curr[0] - d2) % 360
      curr[0] = curr[0] < 0 ? curr[0] += 360 : curr[0]
      anim[0] = anim[0] + d2 + multiplier
      curr[1] = (curr[1] - d4) % 360
      curr[1] = curr[1] < 0 ? curr[1] += 360 : curr[1]
      anim[1] = anim[1] + d4 + multiplier
    } else if (direction <= 5) { // COUNTERCLOCKWISE
      curr[0] = (curr[0] + d1) % 360
      anim[0] = anim[0] - d1 - multiplier
      curr[1] = (curr[1] + d3) % 360
      anim[1] = anim[1] - d3 - multiplier
    } else if (direction <= 8) { // MIN_DISTANCE
      if (d1 <= d2) { // COUNTERCLOCKWISE
        curr[0] = (curr[0] + d1) % 360
        anim[0] = anim[0] - d1 - multiplier
      } else { // CLOCKWISE
        curr[0] = (curr[0] - d2) % 360
        curr[0] = curr[0] < 0 ? curr[0] += 360 : curr[0]
        anim[0] = anim[0] + d2 + multiplier
      }
      if (d3 <= d4) { // COUNTERCLOCKWISE
        curr[1] = (curr[1] + d3) % 360
        anim[1] = anim[1] - d3 - multiplier
      } else { // CLOCKWISE
        curr[1] = (curr[1] - d4) % 360
        curr[1] = curr[1] < 0 ? curr[1] += 360 : curr[1]
        anim[1] = anim[1] + d4 + multiplier
      }
    } else if (direction <= 11) { // MAX_DISTANCE
      if (d1 >= d2) { // COUNTERCLOCKWISE
        curr[0] = (curr[0] + d1) % 360
        anim[0] = anim[0] - d1 - multiplier
      } else { // CLOCKWISE
        curr[0] = (curr[0] - d2) % 360
        curr[0] = curr[0] < 0 ? curr[0] += 360 : curr[0]
        anim[0] = anim[0] + d2 + multiplier
      }
      if (d3 >= d4) { // COUNTERCLOCKWISE
        curr[1] = (curr[1] + d3) % 360
        anim[1] = anim[1] - d3 - multiplier
      } else { // CLOCKWISE
        curr[1] = (curr[1] - d4) % 360
        curr[1] = curr[1] < 0 ? curr[1] += 360 : curr[1]
        anim[1] = anim[1] + d4 + multiplier
      }
    }
    setHands(index * 3 + x, anim[0], anim[1], secs)
  }
}

/**
 * Animates one full digit (6 clocks) to the given hand angle layout.
 * @param {number}     index     Digit index 0–3 (left to right)
 * @param {number[][]} digit     6 × [hourAngle, minuteAngle] pairs
 * @param {number}     direction One of the DIR_* constants
 * @param {number}     secs      CSS transition duration in seconds
 */
function setDigit(index, digit, direction, secs) {
  setHalfDigit(index * 2, digit.slice(0, 3), direction, secs)
  setHalfDigit(index * 2 + 1, digit.slice(3, 6), direction, secs)
}

// ─── Animation Sequences ─────────────────────────────────────
/** Moves all hands to the resting/off position (both pointing down). */
function setStop() {
  for (let i = 0; i < 4; i++)
    setDigit(i, digit_stop, DIR_MIN, 15)
}

/** Displays the time with a minimal shortest-arc transition. */
function setLazy(time) {
  for (let i = 0; i < 4; i++)
    setDigit(i, digits[time.charAt(i)], DIR_MIN, 15)
}

/** Displays the time with a clockwise spinning transition. */
function setChaos(time) {
  for (let i = 0; i < 4; i++)
    setDigit(i, digits[time.charAt(i)], DIR_CW_SPIN1, 16)
}

/** Displays the time with a 3-phase wave: collapse to vertical → ripple spin → settle to digits. */
function setWaves(time) {
  for (let i = 0; i < 4; i++)
    setDigit(i, digit_II, DIR_MIN, 8)
  for (let i = 0; i < 8; i++)
    setTimeout(() => {
      setHalfDigit(i, digit_II, DIR_CW_SPIN1, 18)
    }, WAVE_PHASE1_MS + WAVE_STEP_DELAY_MS * (i + 1))
  for (let i = 0; i < 4; i++)
    setTimeout(() => {
      setDigit(i, digits[time.charAt(i)], DIR_CW, 15)
    }, WAVE_PHASE1_MS + WAVE_PHASE2_MS + WAVE_STEP_DELAY_MS * (i * 2 + 1))
}

/**
 * Returns the outward-ripple delay (ms) for a clock at grid position (x, y).
 * x: 1–8 (column), y: 1–3 (row). Clocks closest to the center start at T=0.
 */
function calculateStartDelay(x, y, speedMultiplier = 100) {
  const distance = Math.sqrt(Math.pow(x - 4.5, 2) + Math.pow(y - 2.0, 2))
  return Math.max(0, speedMultiplier * (distance - 0.5))
}

/**
 * Compute the absolute angle (unit-circle convention, degrees) of one
 * vector at grid position (x, y).
 *
 * @param {number} x           — Column index (1 … 8)
 * @param {number} y           — Row index    (1 … 3)
 * @param {number} vectorIndex — Which vector of the pair:
 *                                 0  →  θ_inward + offset  (counter-clockwise arm)
 *                                 1  →  θ_inward − offset  (clockwise arm)
 * @param {number} [innerAngle=90] — Offset at the center, in degrees
 * @param {number} [outerAngle=90] — Offset at the field edge, in degrees
 * @returns {number} Angle in degrees, measured counter-clockwise from the
 *                   positive x-axis (standard unit-circle convention).
 */
function computeVectorAngle(x, y, vectorIndex, innerAngle = 90, outerAngle = 90) {
  // --- Field geometry (self-contained) ---
  const centerX = 4.5;
  const centerY = 2.0;
  const maxDist = Math.sqrt(3.5 * 3.5 + 1.0 * 1.0); // ≈ 3.640
  // --- Step A: base inward angle (toward center) ---
  const thetaInward = Math.atan2(centerY - y, centerX - x) * 180 / Math.PI;
  // --- Step B: normalised distance from center (0 … 1) ---
  const dx = centerX - x;
  const dy = centerY - y;
  const r = Math.sqrt(dx * dx + dy * dy);
  const rNorm = Math.min(r / maxDist, 1.0);
  // --- Step C: interpolated offset (all in degrees) ---
  const offset = innerAngle + (outerAngle - innerAngle) * rNorm;
  // --- Step D: apply mirrored offset ---
  if (vectorIndex === 0) {
    return thetaInward + offset;
  }
  return 360 + thetaInward - offset;
}

/**
 * Returns { hourAngle, minuteAngle } for a clock at grid position (x, y) in CSS degrees.
 * innerAngle / outerAngle control each hand's spread from the inward direction:
 *   90° (default) = hands form a tangential diameter (concentric rings look)
 *   45°           = V-shape opening outward toward the edges
 *    0°           = both hands point inward (spike look)
 */
function calculateHandAngles(x, y, innerAngle = 90, outerAngle = 90) {
  return {
    hourAngle: Math.round(computeVectorAngle(x, y, 0, innerAngle, outerAngle)),
    minuteAngle: Math.round(computeVectorAngle(x, y, 1, innerAngle, outerAngle))
  }
}

/** Displays the time with a 3-phase circle: align to vector field → counter-rotating spin ripple → settle to digits. */
function setCircle(time) {
  const INNER_ANGLE = 70        // hand spread at center (degrees from inward direction)
  const OUTER_ANGLE = 80        // hand spread at edge — equal = uniform tangential diameter
  const SETUP_SEC = 3         // s: duration to reach start positions
  const SPIN_SEC = 24        // s: duration of counter-rotating spin phase
  const FINAL_SEC = 10        // s: duration to settle into final digits
  const DELAY_SPEED = 800       // ms per unit of grid distance (controls ripple speed)
  const SPINS = 2         // number of full counter-rotations per hand

  const SETUP_WAIT = SETUP_SEC * 1500

  // Phase 1: Move all clocks to their geometric start positions (shortest path).
  // Directly updates current_state/anim_state per clock and calls setHands().
  // computeVectorAngle returns CSS-convention angles, but current_state uses an
  // inverted convention (anim = 360 - curr). Convert with (360 - angle) % 360.
  for (let c = 0; c < 8; c++) {
    for (let r = 0; r < 3; r++) {
      const { hourAngle, minuteAngle } = calculateHandAngles(c + 1, r + 1, INNER_ANGLE, OUTER_ANGLE)
      const targetH = (360 - ((hourAngle % 360) + 360) % 360) % 360
      const targetM = (360 - ((minuteAngle % 360) + 360) % 360) % 360
      const clockId = c * 3 + r
      const curr = current_state[Math.floor(c / 2)][r + (c % 2) * 3]
      const anim = anim_state[Math.floor(c / 2)][r + (c % 2) * 3]

      // Small hand (hour) — MIN_DISTANCE
      const d1 = calcAngleCW(curr[0], targetH)
      const d2 = calcAngleCCW(curr[0], targetH)
      if (d1 <= d2) {
        curr[0] = (curr[0] + d1) % 360
        anim[0] = anim[0] - d1
      } else {
        curr[0] = (curr[0] - d2) % 360
        curr[0] = curr[0] < 0 ? curr[0] += 360 : curr[0]
        anim[0] = anim[0] + d2
      }

      // Large hand (minute) — MIN_DISTANCE
      const d3 = calcAngleCW(curr[1], targetM)
      const d4 = calcAngleCCW(curr[1], targetM)
      if (d3 <= d4) {
        curr[1] = (curr[1] + d3) % 360
        anim[1] = anim[1] - d3
      } else {
        curr[1] = (curr[1] - d4) % 360
        curr[1] = curr[1] < 0 ? curr[1] += 360 : curr[1]
        anim[1] = anim[1] + d4
      }

      setHands(clockId, anim[0], anim[1], SETUP_SEC)
    }
  }

  // Phase 2: Hands spin in opposite directions, rippling outward from center.
  // Direct anim_state mutation is required: setHalfDigit cannot apply opposite spin
  // directions to the two hands simultaneously. Net rotation = 0°, so current_state
  // remains valid for Phase 3's shortest-path calculation.
  let maxDelay = 0
  for (let c = 0; c < 8; c++) {
    for (let r = 0; r < 3; r++) {
      const delay = calculateStartDelay(c + 1, r + 1, DELAY_SPEED)
      if (delay > maxDelay) maxDelay = delay
      setTimeout(() => {
        const clockId = c * 3 + r
        const anim = anim_state[Math.floor(c / 2)][r + (c % 2) * 3]
        const rot = 360 * SPINS
        if (c < 4) { anim[0] -= rot; anim[1] += rot }  // cols 1-4: CW
        else { anim[0] -= rot; anim[1] += rot }  // cols 5-8: CCW
        setHands(clockId, anim[0], anim[1], SPIN_SEC)
      }, SETUP_WAIT + delay)
    }
  }

  // Phase 3: Settle to final time digits.
  // Fires FINAL_SEC seconds before Phase 2's CSS transition ends so the spin is
  // still decelerating when the settle begins — CSS re-targets mid-flight for a
  // seamless blend instead of a stop-then-start.
  setTimeout(() => {
    for (let i = 0; i < 4; i++)
      setDigit(i, digits[time.charAt(i)], DIR_MIN, FINAL_SEC)
  }, SETUP_WAIT + maxDelay + (SPIN_SEC - FINAL_SEC) * 1000)
}

/**
 * Returns the angle (degrees, 0° = right, CW+) for a hand at grid
 * position (x, y) pointing toward the attractor at (cx, cy).
 */
function getAngleToAttractor(x, y, cx, cy) {
  return Math.atan2(cy - y, cx - x) * 180 / Math.PI
}

/** Displays the time with a spiral: attractor-field alignment → column-split spin ripple → settle to digits. */
function setSpiral(time) {
  const SETUP_SEC = 3     // s: reach attractor field positions
  const WAIT_MS = 1500  // ms: pause after attractor field forms
  const SPIN_SEC = 24    // s: spin phase duration
  const FINAL_SEC = 11    // s: settle into digits
  const DELAY_SPEED = 300   // ms per unit grid distance (ripple speed)
  const SPINS = 3     // full rotations before settling

  const SETUP_WAIT = SETUP_SEC * 1000  // buffer beyond SETUP_SEC for CSS to settle

  // Phase 1: All hands (hour = minute, both overlap) move to attractor field.
  // Each clock's hands point toward center (4.5, 2.0) via the shortest arc.
  for (let c = 0; c < 8; c++) {
    for (let r = 0; r < 3; r++) {
      const angle = getAngleToAttractor(c + 1, r + 1, 4.5, 2.0)
      const target = (360 - ((angle % 360) + 360) % 360) % 360  // CSS → current_state
      const clockId = c * 3 + r
      const curr = current_state[Math.floor(c / 2)][r + (c % 2) * 3]
      const anim = anim_state[Math.floor(c / 2)][r + (c % 2) * 3]

      for (let hand = 0; hand < 2; hand++) {
        const d1 = calcAngleCW(curr[hand], target)
        const d2 = calcAngleCCW(curr[hand], target)
        if (d1 <= d2) {
          curr[hand] = (curr[hand] + d1) % 360
          anim[hand] = anim[hand] - d1
        } else {
          curr[hand] = (curr[hand] - d2) % 360
          curr[hand] = curr[hand] < 0 ? curr[hand] += 360 : curr[hand]
          anim[hand] = anim[hand] + d2
        }
      }
      setHands(clockId, anim[0], anim[1], SETUP_SEC)
    }
  }

  // Phase 2: Ripple spin outward from center. All columns CW.
  // Both hands move identically so they stay overlapping throughout.
  // Net rotation = ±SPINS * 360°, so current_state is unchanged — Phase 3 works seamlessly.
  let maxDelay = 0
  for (let c = 0; c < 8; c++) {
    for (let r = 0; r < 3; r++) {
      const delay = calculateStartDelay(c + 1, r + 1, DELAY_SPEED)
      if (delay > maxDelay) maxDelay = delay
      setTimeout(() => {
        const clockId = c * 3 + r
        const anim = anim_state[Math.floor(c / 2)][r + (c % 2) * 3]
        const rot = 360 * SPINS
        anim[0] += rot; anim[1] += rot  // all cols: CW
        setHands(clockId, anim[0], anim[1], SPIN_SEC)
      }, SETUP_WAIT + WAIT_MS + delay)
    }
  }

  // Phase 3: Fires FINAL_SEC before last spin ends → CSS re-targets mid-deceleration for seamless blend.
  setTimeout(() => {
    for (let i = 0; i < 4; i++)
      setDigit(i, digits[time.charAt(i)], DIR_MIN, FINAL_SEC)
  }, SETUP_WAIT + WAIT_MS + maxDelay + (SPIN_SEC - FINAL_SEC) * 1000)
}

//this animation looks shit and needs to be redone
function setAttract(time) {
  const SETUP_SEC = 3        // s: hands reach initial position
  const SETUP_WAIT = SETUP_SEC * 1000 + 500
  const TRAVEL_SPD = 1      // grid units per second
  const UPDATE_MS = 80       // ms between position updates
  const STEP_SEC = 0.15     // s: CSS transition per step (> UPDATE_MS for smooth trail)
  const FINAL_SEC = 4        // s: settle into digit positions

  // Outer perimeter: (0,0)→(9,0)→(9,4)→(0,4)→(0,0), just outside the clock grid (x:1–8, y:1–3)
  const waypoints = [[0, 0], [9, 0], [9, 4], [0, 4], [0, 0]]

  // Move all clocks to point at (cx, cy), both hands overlapping, shortest path
  function updateHandsToAttractor(cx, cy, sec) {
    for (let c = 0; c < 8; c++) {
      for (let r = 0; r < 3; r++) {
        const angle = getAngleToAttractor(c + 1, r + 1, cx, cy)
        const target = (360 - ((angle % 360) + 360) % 360) % 360
        const clockId = c * 3 + r
        const curr = current_state[Math.floor(c / 2)][r + (c % 2) * 3]
        const anim = anim_state[Math.floor(c / 2)][r + (c % 2) * 3]
        for (let hand = 0; hand < 2; hand++) {
          const d1 = calcAngleCW(curr[hand], target)
          const d2 = calcAngleCCW(curr[hand], target)
          if (d1 <= d2) {
            curr[hand] = (curr[hand] + d1) % 360
            anim[hand] -= d1
          } else {
            curr[hand] = (curr[hand] - d2 + 360) % 360
            anim[hand] += d2
          }
        }
        setHands(clockId, anim[0], anim[1], sec)
      }
    }
  }

  // Phase 1: align to initial attractor position
  updateHandsToAttractor(0, 0, SETUP_SEC)

  // Build position sequence for the rectangle path
  const positions = []
  for (let i = 0; i < waypoints.length - 1; i++) {
    const [x0, y0] = waypoints[i], [x1, y1] = waypoints[i + 1]
    const dx = x1 - x0, dy = y1 - y0
    const dist = Math.sqrt(dx * dx + dy * dy)
    const steps = Math.ceil(dist / TRAVEL_SPD * 1000 / UPDATE_MS)
    for (let s = 0; s < steps; s++) {
      const t = s / steps
      positions.push([x0 + dx * t, y0 + dy * t])
    }
  }

  // Phase 2: schedule all tracking updates
  for (let s = 0; s < positions.length; s++) {
    setTimeout(() => {
      updateHandsToAttractor(positions[s][0], positions[s][1], STEP_SEC)
    }, SETUP_WAIT + s * UPDATE_MS)
  }

  // Phase 3: transition to time display — current_state is exact so shortest-path is seamless
  setTimeout(() => {
    for (let i = 0; i < 4; i++)
      setDigit(i, digits[time.charAt(i)], DIR_MIN, FINAL_SEC)
  }, SETUP_WAIT + positions.length * UPDATE_MS)
}

/*
The animation begins with all clock hands aligned to form a solid horizontal block. 
The top and bottom rows then fold outward in opposite directions while the center row rotates synchronously, smoothly transitioning the entire display into vertical columns. 
This fluid, mechanical movement creates a mesmerizing visual effect that mimics rhythmic breathing or the steady flapping of wings.
*/
function setLoom(time) {
  const SETUP_SEC = 3;
  const SPIN_SEC = 16;
  const FINAL_SEC = 6;
  const SPINS = 2;
  const SETUP_WAIT = SETUP_SEC * 1000 + 500;

  for (let i = 0; i < 4; i++)
    setDigit(i, digit_loom, DIR_MIN, SETUP_SEC);

  setTimeout(() => {
    for (let c = 0; c < 8; c++) {
      for (let r = 0; r < 3; r++) {
        const clockId = c * 3 + r;
        const anim = anim_state[Math.floor(c / 2)][r + (c % 2) * 3];
        const rot = 360 * SPINS;

        if (r === 2) { // Bottom row
          anim[0] += rot;
          anim[1] -= rot;
        } else { // Top and middle rows
          anim[0] -= rot;
          anim[1] += rot;
        }

        setHands(clockId, anim[0], anim[1], SPIN_SEC);
      }
    }
  }, SETUP_WAIT);

  setTimeout(() => {
    for (let i = 0; i < 4; i++)
      setDigit(i, digits[time.charAt(i)], DIR_MIN, FINAL_SEC);
  }, SETUP_WAIT + (SPIN_SEC - FINAL_SEC) * 1000);
}

// ─── Clock Loop ───────────────────────────────────────────────
/** Routes to the active animation function; advances the cycle index when CYCLE is active. */
function dispatchAnimation(time) {
  let anim = selectedAnimation
  if (anim === 6) anim = advanceCycle()
  switch (anim) {
    case 0: setWaves(time); break
    case 1: setChaos(time); break
    case 2: setCircle(time); break
    case 3: setSpiral(time); break
    case 4: setAttract(time); break
    case 5: setLoom(time); break
  }
}

/** Forces the current animation to run immediately regardless of whether the time has changed. */
function testAnimation() {
  let d = new Date()
  let time = d.toTimeString().substring(0, 5).replace(':', '')
  dispatchAnimation(time)
}

/** Returns the next animation index for CYCLE mode (sequential or random). */
function advanceCycle() {
  let anim
  if (selectedCycleType === 1) {
    const d = new Date()
    const seed = d.getHours() * 60 + d.getMinutes()
    anim = ((seed * 2654435761) >>> 0) % 6
  } else {
    anim = currentCycleIndex
    currentCycleIndex = (currentCycleIndex + 1) % 6
  }
  return anim
}

let lastTime  // last time string rendered, used to detect minute changes

/** Checks if the current minute has changed and dispatches the appropriate display update. */
function setTime() {
  const d = new Date();
  const time = d.toTimeString().substring(0, 5).replace(':', '')
  const day = (d.getDay() + 6) % 7
  const hours = d.getHours()
  if (time !== lastTime && sleep[day][hours] === 0) {
    lastTime = time
    if (selectedMode === 1) {
      setLazy(lastTime)
    } else if (selectedMode === 2) {
      dispatchAnimation(lastTime)
    }
  } else if (sleep[day][hours] === 1) {
    setStop()
  }
}

let clockInterval = undefined

/** Starts the clock update interval if not already running. */
function startClock() {
  if (clockInterval === undefined) {
    lastTime = undefined
    clockInterval = setInterval(setTime, CLOCK_INTERVAL_MS)
  }
}

/** Stops the clock update interval and moves all hands to the rest position. */
function stopClock() {
  clearInterval(clockInterval)
  clockInterval = undefined
  setTimeout(setStop, 100)
}

let lastClockInterval = undefined

// Pause the clock loop while the tab is in the background to avoid stale setTimeout chains
// accumulating when the browser throttles inactive tabs.
document.addEventListener("visibilitychange", (event) => {
  if (document.visibilityState == "visible") {
    if (lastClockInterval !== undefined)
      clockInterval = setInterval(setTime, CLOCK_INTERVAL_MS)
  } else {
    lastClockInterval = clockInterval
    clearInterval(clockInterval)
  }
});

// Start the clock
startClock()
