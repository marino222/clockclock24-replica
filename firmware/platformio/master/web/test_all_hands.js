/**
 * ClockClock24 – full-board hand test.
 *
 * The ESP32 runs the test locally: it first moves every clock into the known
 * stop pose, then each selected hand makes exactly one full clockwise rotation
 * and lands back in that pose.
 *
 * Usage:
 *   node test_all_hands.js          # hour hands, then minute hands
 *   node test_all_hands.js h        # hour hands only
 *   node test_all_hands.js m        # minute hands only
 *
 * Optional env vars:
 *   HOST=192.168.1.220
 *   PORT=80
 *   STEP_DELAY_MS=400   (pause between per-clock moves, on the ESP32)
 *   PHASE_DELAY_MS=3000 (pause between hour and minute phase, on the ESP32)
 *   RETRIES=3           (retries per request)
 *   TIMEOUT_MS=12000    (single request timeout)
 */

const HOST = process.env.HOST ?? '192.168.1.220'
const PORT = process.env.PORT ?? '80'
const STEP_DELAY_MS = Number(process.env.STEP_DELAY_MS ?? 400)
const PHASE_DELAY_MS = Number(process.env.PHASE_DELAY_MS ?? 3000)
const RETRIES = Number(process.env.RETRIES ?? 3)
const TIMEOUT_MS = Number(process.env.TIMEOUT_MS ?? 12000)
const BASE = `http://${HOST}:${PORT}`

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms))
}

async function postWithTimeout(url, options, timeoutMs) {
  const controller = new AbortController()
  const timer = setTimeout(() => controller.abort(), timeoutMs)
  try {
    return await fetch(url, { ...options, signal: controller.signal })
  } finally {
    clearTimeout(timer)
  }
}

async function startTest(hand) {
  const body = new URLSearchParams({
    hand,
    step_delay_ms: STEP_DELAY_MS,
    phase_delay_ms: PHASE_DELAY_MS,
  }).toString()

  for (let attempt = 1; attempt <= RETRIES; attempt++) {
    try {
      const res = await postWithTimeout(`${BASE}/test/all`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body,
      }, TIMEOUT_MS)

      if (res.ok)
        return true

      if (attempt === RETRIES)
        console.error(`FAIL hand=${hand} http=${res.status}`)
    } catch (err) {
      if (attempt === RETRIES)
        console.error(`FAIL hand=${hand} error=${err.message}`)
    }

    await sleep(500)
  }

  return false
}

async function runTest(hand) {
  const label = hand === 'h' ? 'HOUR' : hand === 'm' ? 'MINUTE' : 'HOUR + MINUTE'
  console.log(`Starting ${label} test via ${BASE}/test/all ...`)

  const ok = await startTest(hand)
  console.log(ok ? 'Test accepted by controller.' : 'Test request failed.')
}

const arg = process.argv[2]
const hand = arg === 'h' ? 'h' : arg === 'm' ? 'm' : 'both'

console.log(`ClockClock24 hand test -> ${BASE}`)
await runTest(hand)
console.log('Done.')
