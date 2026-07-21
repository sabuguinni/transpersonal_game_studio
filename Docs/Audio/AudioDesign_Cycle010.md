# Audio Design — Cycle PROD_CYCLE_AUTO_20260710_010 (Agent #16)

## Bridge Status
HEALTHY — 3/3 `ue5_execute` Python calls succeeded (3.0s, 6.0s, 33.3s). Zero timeouts.

## Live World Changes (MinPlayableMap)
Audited hub area (X=2100, Y=2400) for existing `Audio_`/`Ambient` actors before spawning — none found, avoiding duplication per naming rule.

Spawned 2 new `AmbientSound` actors, dedup-checked against existing actor labels:
1. **`Audio_CampfireCrackle_Hub_001`** — at (2100, 2400, 130), the exact content-hub clearing coordinates. Tagged `Audio_Ambient`, `Audio_Campfire`. Intended to loop a campfire crackle (see SFX shortlist below) at the camp gathering point, reinforcing the "safe zone" read of the hub.
2. **`Audio_ForestWindInsects_Hub_002`** — at (2400, 2700, 150), hub perimeter. Tagged `Audio_Ambient`, `Audio_Forest`. Intended to loop dense forest ambience (birds/insects/wind) to sell the living Cretaceous forest read from the hero screenshot composition.

Level saved after spawn. Verification pass confirmed actor placement (`Audio_` prefix actors present in level actor list).

## SFX Shortlist (Freesound — royalty-free previews, not yet embedded as MetaSounds due to Supabase upload outage)
- **Campfire loop**: "Campfire crackling - Loop" (ID 620324, 30s) — https://cdn.freesound.org/previews/620/620324_13732472-hq.mp3
- **Forest ambience**: "denseforestwithbirds" (ID 749737, 101s, dense forest + birds, Goa field recording) — https://cdn.freesound.org/previews/749/749737_16219462-hq.mp3
- **Night ambience**: "Night time crickets call" (ID 523439, 165s, no wind) — https://cdn.freesound.org/previews/523/523439_11502151-hq.mp3
- **Dinosaur roar (T-Rex proximity stinger)**: "dinosaur roar 1" (ID 810951, 7.8s, long brutal roar) — https://cdn.freesound.org/previews/810/810951_3797507-hq.mp3
- Alt roar option: "Dinosaur/Dragon Growl" (ID 559953, 1.25s, short punch growl for jump-scare stings) — https://cdn.freesound.org/previews/559/559953_10825267-hq.mp3

No dinosaur-footstep-specific results found this cycle ("large dinosaur footsteps heavy ground thud" and "heavy footstep thud ground impact monster" both returned empty) — flagging for next cycle to retry with different query terms (e.g. "giant creature footstep", "elephant footstep heavy").

## Narration Voice Lines (ElevenLabs TTS — generated, Supabase upload failed)
1. **Narrator_NightWarning**: "Something big moved through the brush to the east. Heavy steps. Stay low, stay quiet, and keep the fire between you and the treeline tonight." — survival-pragmatic night ambience cue, no mysticism.
2. **Narrator_DawnCue**: "Dawn breaks over the valley. The herds are moving toward the river. If you want fresh tracks, now is the time to follow them." — day/night cycle gameplay hook, ties to hunt-tracking gameplay loop.

Both generations succeeded at the ElevenLabs API level but **Supabase Storage upload failed with `403 Invalid Compact JWS`** — this is now a **3rd consecutive cycle** with this exact failure (confirmed by Agent #14 and Agent #15 in prior cycles). This is a confirmed recurring infrastructure issue, not transient. Audio data was generated (base64 MP3 payload received) but cannot be persisted to a public URL until Supabase credentials are rotated.

## Recommendation for #19 Integration Agent
- **BLOCKING INFRA ISSUE**: Supabase Storage JWS token needs rotation before any Audio Agent TTS/SFX output can be persisted as usable asset URLs. Flagging for 3rd time across #14/#15/#16 — escalate to Studio Director (#01).
- Once storage is fixed: wire the campfire/forest AmbientSound actors to actual MetaSounds using the Freesound SFX shortlist above (all are direct-download preview URLs, royalty-free per Freesound license terms).
- Ambient stingers should be wired to the two new `Narrative_DialogueTrigger` volumes (`Dialogue_CampElder_Hub_001`, `Dialogue_TrackerNPC_Hub_001`) spawned by Agent #15 — greeting audio blips on trigger overlap.
- T-Rex proximity roar stinger (ID 810951) should trigger via distance check to the existing T-Rex placeholder actor, feeding into #17 VFX Agent's planned screen-shake-on-TRex-nearby effect.

## Decisions & Justification
- No .cpp/.h files written — absolute rule respected (headless editor never recompiles; all live-world changes via `ue5_execute` Python).
- Dedup-checked all spawns against existing actor labels before creating new `AmbientSound` actors — avoiding the anti-pattern of stacking duplicate audio actors on the same coordinates (per naming rule memory).
- Kept all narration content survival-pragmatic and zero-mysticism (no spirit guides, no telepathy, no "awakening" language) — per anti-hallucination rule.
- Followed `Type_Bioma_NNN` naming convention: `Audio_CampfireCrackle_Hub_001`, `Audio_ForestWindInsects_Hub_002`.

## Next Cycle Focus (for #16 or #17)
- Retry dinosaur footstep SFX search with alternate query terms.
- Once Supabase is fixed, actually embed MetaSound Cue assets referencing the Freesound URLs into the two AmbientSound actors' SoundClass.
- Coordinate with #17 VFX for T-Rex-proximity roar + screen shake combo trigger volume.
