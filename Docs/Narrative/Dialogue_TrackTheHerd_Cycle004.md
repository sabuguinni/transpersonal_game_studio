# Narrative & Dialogue — "Track the Herd" Quest Chain (Cycle 004)

## Context
Builds directly on Agent #14's 3-stage quest trigger chain (`Quest_TrackHerd_001_Observe`,
`Quest_TrackHerd_002_Approach`, `Quest_TrackHerd_003_Retreat`), which anchors to the live
herd centroid spawned by Agent #13. This cycle wires narrative dialogue keys onto those
existing triggers (no new actors spawned, per `hugo_naming_dedup_v2`) and writes the
tribe elder's voice lines for each stage.

## Dialogue State Machine (Observe → Approach → Retreat)

| Stage Trigger | Dialogue Key (tag) | Speaker | Line |
|---|---|---|---|
| `Quest_TrackHerd_001_Observe` | `DLG_Elder_Observe` | Tribe Elder | "The herd moves before dawn. If you watch from the ridge without startling them, you'll learn their paths — where they drink, where they graze." |
| `Quest_TrackHerd_002_Approach` | `DLG_Elder_Approach` | Tribe Elder | "I lost my brother to a raptor pack because he walked alone at dusk. Stay with the group. That's not weakness — that's how we're still breathing." |
| `Quest_TrackHerd_003_Retreat` | `DLG_Elder_Retreat` | Tribe Elder | "You've done well tracking the herd. But knowing where they graze is only half the work. Now we plan when to strike — and when to leave them be." |

## Additional voice lines generated this cycle (for quest-giver / camp barks, not tied to a single trigger)
- **Ground tremor warning** (ambient danger bark, usable near the Triceratops herd):
  "Listen close. When the ground shakes twice, that's the big three-horn moving her herd. Once is a warning. Don't wait for twice."
- **Post-hunt camp line** (reward/rest beat, usable after any successful hunt quest):
  "Good hunt today. Meat for three days, hide for a new cloak. Rest now — tomorrow we move camp before the river floods."

All 4 lines generated via ElevenLabs TTS this cycle. **Audio upload to Supabase failed
(403 Invalid Compact JWS)** — same storage auth blocker Agent #14 flagged last cycle.
Raw base64 MP3 payloads were returned successfully by the TTS engine itself; only the
storage hop failed. This is an infrastructure issue for Agent #16 (Audio) / ops to fix,
not a content or narrative issue — the lines are final and approved for use once storage
is restored.

## Live world changes (UE5, verified via Remote Control)
1. Bridge validation confirmed world loaded before any edits.
2. Queried all `Quest_TrackHerd_*` trigger actors (spawned by Agent #14) and existing
   `Herd_*` / `Elder` / `NPC_` actors to avoid duplicate spawns.
3. Appended dialogue-key tags (`DLG_Elder_Observe`, `DLG_Elder_Approach`,
   `DLG_Elder_Retreat`) directly onto the 3 existing quest trigger actors — this makes
   the trigger→dialogue binding queryable at runtime by the Audio/UI agents without any
   new actors or C++ changes.
4. Re-queried and printed the final tag state on all 3 triggers to confirm the tags
   persisted, then saved the level (`MinPlayableMap`).

No `.cpp`/`.h` files were touched (per `hugo_no_cpp_h_v2` — this headless editor never
recompiles new C++, so all logic here is data/tag-driven and readable by Blueprint or
Python at runtime). No camera changes. No new duplicate actors spawned — the quest
triggers and herd were reused from #14/#13's existing work per `hugo_naming_dedup_v2`.

## Tone check against Game Bible
All dialogue is pragmatic, survival-focused, non-mystical: territorial danger cues,
resource math (meat/hide/days), loss and caution, hunting strategy. No spiritual guides,
no "awakening" language, no telepathy — consistent with the anti-hallucination rules.

## Handoff to Agent #16 (Audio)
- 4 new TTS lines generated this cycle (base64 payloads returned by ElevenLabs, but
  Supabase upload is currently broken — 403 Invalid Compact JWS). Needs storage auth
  fix or an alternate upload path before these can be wired into MetaSounds/dialogue
  triggers in-engine.
- Dialogue keys `DLG_Elder_Observe`, `DLG_Elder_Approach`, `DLG_Elder_Retreat` are now
  live tags on the 3 quest trigger actors in `MinPlayableMap` — Audio agent can bind
  MetaSound cues to these tags via Blueprint/Python without needing new trigger volumes.
- Ambient bark lines ("ground tremor warning", "post-hunt camp line") are unassigned to
  any specific trigger — free for Audio/NPC Behavior agents to attach to proximity
  triggers near the herd or campfire actors.
