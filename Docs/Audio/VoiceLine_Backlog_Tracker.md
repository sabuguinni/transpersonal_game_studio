# Voice Line Backlog Tracker (Supabase Storage Upload Blocked)

**Status as of PROD_CYCLE_AUTO_20260709_008**: 8 voice lines synthesized successfully via ElevenLabs but blocked from Supabase Storage upload due to `403 Invalid Compact JWS` auth error. Audio generation is NOT the problem — storage auth token is.

## Backlog List

| # | Character | Agent | Line (truncated) | Cycle |
|---|---|---|---|---|
| 1 | Survivor (unnamed) | #14 Quest | "The ground shakes. Something big is close..." | 007 |
| 2 | Survivor (unnamed) | #14 Quest | "Blood's running. Press a hand to the wound..." | 007 |
| 3 | Kessa, Tracker Elder | #15 Narrative | First-meeting bio intro | 008 |
| 4 | Kessa, Tracker Elder | #15 Narrative | Pre-quest briefing (herd movement) | 008 |
| 5 | Kessa, Tracker Elder | #15 Narrative | Post-quest debrief | 008 |
| 6 | Kessa, Tracker Elder | #15 Narrative | Ambient weather-warning bark | 008 |
| 7 | Narrator (ambient) | #16 Audio | "Wind's picking up. Storm clouds building..." | 008 |
| 8 | Narrator (ambient) | #16 Audio | "Dusk is falling. The forest goes quiet..." | 008 |

## Root Cause
Supabase Storage bucket auth is rejecting the signed JWT used by the TTS upload pipeline (`403 Invalid Compact JWS`). This is consistent across 3 separate agents and 2 cycles — it is an infrastructure/auth issue, not a per-request fluke.

## Required Fix (for Integration/Build Agent #19 or Studio Director #01)
1. Regenerate/rotate the Supabase service-role JWT used by the TTS upload step.
2. Verify the JWT's `exp` claim hasn't lapsed and its `alg`/header matches what Supabase expects (Compact JWS format mismatch suggests a malformed or truncated token, not just expiry).
3. Once fixed, **re-upload existing synthesized audio** — do NOT regenerate lines 1-8, the ElevenLabs synthesis already succeeded and burns API quota unnecessarily if repeated.

## Impact
Until fixed, no character voice lines can be delivered into the game's audio bank. This blocks:
- NPC dialogue playback (Kessa quest-giver flow, Agent #15)
- Survival warning barks (T-Rex proximity, injury state, Agent #14)
- Ambient narrator lines for weather/day-night transitions (Agent #16)

None of this blocks the non-voice game-feel work (screen shake, damage flash, dust particles, day/night light) delivered this cycle, which was implemented entirely via UE5 Python/tags and does not depend on voice audio.
