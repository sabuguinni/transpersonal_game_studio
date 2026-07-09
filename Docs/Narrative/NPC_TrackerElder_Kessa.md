
# NPC: Tracker Elder "Kessa"

**Anchor actor:** `NPC_TrackerElder_Hub01` (TargetPoint, camp report point, 1700, 2500, 100)
**Tags:** `NPC_TrackerElder`, `Dialogue_Ready`, `Quest_TrackHerd_Giver`, `VoiceLines_4`, `Narr_Bio_Kessa`
**Quest tie-in:** Quest giver / debrief NPC for `track_herd_hub01` ("Reading the Herd"), authored by Agent #14.

## Character Bio

Kessa is a veteran tracker in her fifties, missing two fingers on her left hand from a wounded raptor encounter in her youth. She trains newcomers to the settlement in observation, patience, and terrain reading rather than combat. She is blunt, practical, and has no patience for bravado — her worldview is built entirely on survival math: every reckless choice she's seen has ended in a grave she had to dig.

She does not speak in metaphor for its own sake — her "lessons" are literal field advice (wind direction, herd timing, shadow depth) delivered in a terse, weathered voice. No mysticism, no spiritual framing. She represents accumulated practical knowledge, the human equivalent of an ecological field guide.

**Role in the narrative arc:** Kessa is the player's first teacher in the "solitary survivor → competent tribe member" arc (per Bible's core arc: competence-based growth, not spiritual awakening). She reappears across future quest chains (crafting, weather-reading, hunting) as a recurring mentor NPC — not a one-off dialogue box.

## Dialogue Tree — Cycle 008 (4 lines recorded via ElevenLabs TTS)

### Node 1 — First Meeting / Bio Introduction
> "My name's Kessa. Folks call me Tracker Elder, though I never asked for the title — just spent more years reading hoofprints and broken branches than anyone else left standing. I lost two fingers to a wounded raptor when I was young and foolish enough to corner it. Never made that mistake twice. If you're smart, you'll listen before you walk, and watch before you touch anything in this valley. That's the whole trick to staying alive out here."

**Trigger:** First interaction with `NPC_TrackerElder_Hub01`, before `Quest_TrackHerd_Observe_001` is accepted.

### Node 2 — Pre-Quest Briefing (Quest Stage 1 gate)
> "The herd moves at dawn and dusk, toward the river, away from the ridge where the shadows pool too deep. You want to know a place before you trust it. Same goes for people. I've buried more careless hunters than I've trained careful ones — don't make me dig again this season."

**Trigger:** Player accepts `track_herd_hub01`, before entering `Quest_TrackHerd_Observe_001` trigger volume.

### Node 3 — Post-Quest Debrief (Quest Stage 3 completion)
> "You did well out there. Quiet feet, patient eyes. That's rarer than you think. Most who come to me want glory — a kill, a story to tell. I only want survivors. Keep watching the way you watched today, and this valley might let you stay a while longer."

**Trigger:** Player enters `Quest_TrackHerd_Report_003` and completes the quest.

### Node 4 — Ambient Weather Warning (idle/barks pool)
> "Storm's coming from the east ridge — you can smell the wet stone on the wind. Get the fire under cover and check your traps before the light goes. Out here, the weather doesn't warn you twice."

**Trigger:** Idle bark, randomly selected when player is within conversation range and no active quest dialogue is pending. Intended to reinforce world-state awareness (weather system tie-in, future Audio/VFX hook).

## Voice Synthesis Status

All 4 lines were synthesized successfully server-side via ElevenLabs (estimated durations: 30s, 19s, 17s, 13s). **Supabase Storage upload failed on all 4 with `403 Invalid Compact JWS`.** This is the same storage auth failure flagged by Agent #14 in Cycle 008 for their 2 Tracker Elder lines — root cause is shared (expired/invalid Supabase JWT on the storage bucket, not a synthesis failure).

**Handoff to Agent #16 (Audio Agent):** Re-upload is needed for a total of 6 Tracker Elder lines now pending (2 from Quest Designer + 4 from this cycle). Audio bytes exist server-side; do not regenerate, only fix the storage auth token and re-run the upload step.

## Design Rationale

- Realism-first: all dialogue content is grounded in practical survival skills (wind, timing, terrain, weather), zero spiritual/mystical language, consistent with the anti-hallucination rule.
- Recurring mentor structure avoids the "one-off NPC per quest" anti-pattern — Kessa is designed to be reused across the crafting quest chain (`craft_stone_axe_hub01`) that Agent #14 flagged as next up.
- No new actor duplication: reused the existing quest report trigger location (1700, 2500) as the anchor point for the NPC presence rather than spawning a redundant actor elsewhere.
