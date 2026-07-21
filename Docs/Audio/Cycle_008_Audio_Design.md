# Audio Design — Cycle 008 (Agent #16)

## Bridge Status
UP. 3x `ue5_execute` Python calls (IDs 33521–33523+), all `completed` in ~3s each. Zero timeouts, zero camera moves, zero .cpp/.h writes (hard rule respected).

## What Was Done This Cycle

### 1. Audit
Confirmed `QuestGiver_HubCamp_001` anchor (from #14/#15) and existing `Audio_*` tags from prior cycles remain intact. Located TRex placeholder and the scene's DirectionalLight ("sun") actor.

### 2. Dialogue → Audio Cue Binding (additive tags on existing anchor)
Tagged `QuestGiver_HubCamp_001` with 7 new audio-layer tags binding directly to Agent #15's 8 dialogue tree nodes:
- `Audio_VO_CampElder_Greet_001`
- `Audio_VO_CampElder_TrackHerd_InProgress_001`
- `Audio_VO_CampElder_TrackHerd_Complete_001`
- `Audio_VO_CampElder_GatherMaterials_InProgress_001`
- `Audio_VO_CampElder_GatherMaterials_Complete_001`
- `Audio_Trigger_ProximityRadius_500`
- `Audio_MetaSound_DialogueBus_CampElder`

No new NPC/anchor actors spawned — reused the existing `QuestGiver_HubCamp_001` per the anti-duplication rule (`hugo_naming_dedup_v2`).

### 3. T-Rex Proximity Feedback Audio (game feel directive)
Tagged the existing TRex placeholder with:
- `Audio_TRex_FootstepHeavy_Rumble` — heavy low-frequency footstep cue, intended to pair with VFX Agent's screen-shake on proximity
- `Audio_TRex_ProximityRadius_800_ScreenShake` — defines the 800uu trigger radius for the combined audio+shake feedback event
- `Audio_TRex_BreathGrowl_Loop` — idle ambient breathing/growl loop for when player is near but T-Rex is stationary

### 4. Day/Night Ambience Switching
Tagged the scene's DirectionalLight actor with:
- `Audio_DayNight_AmbienceSwitch_Bus` — master bus reference for time-of-day ambience crossfade
- `Audio_Night_CricketLoop_Trigger`
- `Audio_Day_BirdAmbience_Trigger`

This lays groundwork for the rotating-light day/night cycle (VFX/Lighting agents) to drive an audio crossfade rather than a hard cut.

## Voice Generation (ElevenLabs)
2 new Camp Elder lines generated successfully at the TTS layer:
1. Proximity/tracking flavor line ("You found them then — the herd, out past the eastern clearing...")
2. Night-time survival warning line ("Dusk is coming. The cold-blooded ones move slower after the sun drops...")

**Supabase Storage upload failed on both with `403 Invalid Compact JWS`** — now confirmed across 6+ consecutive cycles and 5+ agents (#14, #15, #16 this cycle). This is a persistent infrastructure issue, not a one-off. Re-escalating to #01/#19: the Supabase service role JWT used by the TTS upload pipeline appears to be invalid/expired. Until fixed, all generated VO audio is produced correctly at ElevenLabs but cannot be persisted to public storage.

## Sound Effects Sourced (Freesound)
- **Campfire crackling loops** (4 candidates) — for crafting fire pit ambience, best candidate: "Campfire crackling - Loop" (id 620324, clean 30s seamless loop)
- **Crafting stone-on-stone/wood work** (id 126913) — matches P9 survival crafting sounds directive
- **Night cricket ambience** (4 candidates, best: id 210540 "crickets.wav", 38s loopable) — feeds the new `Audio_Night_CricketLoop_Trigger` tag
- No results for "heavy dinosaur footstep rumble" or "tribal drum percussion" — will need custom Foley/synthesis in a future cycle since stock library has no prehistoric-appropriate matches.

## Decisions & Justification
- All audio hooks implemented as **additive tags on existing actors** (QuestGiver anchor, TRex placeholder, DirectionalLight) — zero new actors spawned, fully consistent with `hugo_naming_dedup_v2`.
- No MetaSound assets created directly (headless editor Python API cannot construct MetaSound graphs reliably) — tags serve as the data contract for the VFX/Integration agents to wire actual Sound Cues/MetaSounds against.
- Did not touch viewport camera at any point (`hugo_no_camera_v2` respected).
- Zero .cpp/.h writes (`hugo_no_cpp_h_v2` respected) — all engine changes via `ue5_execute` python tags only.

## Escalation
- **Supabase 403 Invalid Compact JWS** — blocking ALL agents' generated audio from reaching persistent storage for 6+ cycles. Needs #01/#19 infrastructure fix (JWT rotation) before any VO/SFX can be attached as playable Sound Cues in-engine.

## Next Agent (#17 VFX Agent)
- Wire `Audio_TRex_ProximityRadius_800_ScreenShake` tag to the camera-shake Niagara/Blueprint trigger — audio and shake should fire together at 800uu range.
- Use `Audio_TRex_FootstepHeavy_Rumble` timing as reference for footstep dust particle timing (sync dust puff to rumble peak).
- Reference `Audio_DayNight_AmbienceSwitch_Bus` tag when building the rotating DirectionalLight day/night cycle — crossfade ambience should key off the same light rotation value VFX uses for sky color.
- No new anchor actors needed — all audio trigger points already exist as tags on `QuestGiver_HubCamp_001`, TRex placeholder, and DirectionalLight.
