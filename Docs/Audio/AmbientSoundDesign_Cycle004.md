# Ambient Sound Design — Cycle PROD_CYCLE_AUTO_20260712_004 (Agent #16 Audio)

## In-world actors created this cycle (MinPlayableMap, hub area ~2100,2400)
| Actor Label | Type | Location | Tags | Purpose |
|---|---|---|---|---|
| Audio_Hub_ForestAmbience_001 | AmbientSound | (2100, 2400, 300) | Audio_Ambience, Audio_SourceURL_523436_InsectsBirdsDay | Daytime forest ambience loop anchor |
| Audio_Hub_CampfireLoop_001 | AmbientSound | campfire trigger loc (fallback hub+150,0,0) | Audio_Ambience, Audio_SourceURL_620324_CampfireCrackle | Campfire crackle loop anchor |

## Existing actors tagged (NOT duplicated, per anti-dup rule)
| Actor | New Tag | Reason |
|---|---|---|
| TRex_* (existing) | Audio_ScreenShakeSource, Audio_FootstepHeavy_NoAssetFound | Reference point for VFX camera-shake-on-proximity; footstep SFX still missing |
| DirectionalLight (Sun) | Audio_DayNightSun | Reference actor for day/night rotation system (owned by Lighting Agent #8) |

## Freesound sourced SFX (selected)
- **Forest ambience (day, birds/insects):** Freesound #523436 "Insects and Birds in field Day" — https://cdn.freesound.org/previews/523/523436_11502151-hq.mp3 (duration 121.87s, loopable)
- **Campfire crackle loop:** Freesound #620324 "Campfire crackling - Loop" — https://cdn.freesound.org/previews/620/620324_13732472-hq.mp3 (duration 30s, seamless loop)

## Asset gaps identified (need follow-up search next cycle)
- T-Rex heavy footstep / ground rumble: 0 results for "t-rex footstep heavy thud ground rumble" — retry with "heavy footstep concrete thud" or "elephant footstep ground impact" as substitute reference.
- Raptor screech / aggressive dinosaur vocalization: 0 results — retry with "bird of prey screech aggressive" or "eagle scream" as a substitute base layer for pitch-shifting into a raptor call.

## Voice lines generated (TTS, ElevenLabs)
1. Ranger raptor-gully warning — "Careful past the ridge. A raptor pack cuts through that gully at dawn, moving fast and low. Don't try to outrun them."
2. Apex predator warning — "That's the apex predator of this valley. It doesn't hunt what it can't smell. Stay downwind, stay low, and keep quiet."

**Status:** Both synthesized successfully (valid MP3 base64 payload). Public URL upload to Supabase Storage failed with `403 Invalid Compact JWS` — **4th consecutive cycle with this exact failure.** This is confirmed persistent infra fault (JWT/service-role credential issue on the Supabase Storage bucket used by the TTS pipeline), not a transient network error. Escalating to infra owner for credential rotation/fix.

## Handoff to VFX Agent (#17)
- Import Freesound clips #523436 and #620324 into MetaSounds, wire to `Audio_Hub_ForestAmbience_001` / `Audio_Hub_CampfireLoop_001` Sound Cue slots.
- Use `Audio_ScreenShakeSource` tag on TRex actor to drive proximity-based camera shake (Niagara/CameraShakeBase) — do not spawn a new TRex, reference the existing tagged actor.
- Footstep dust particles for player/dinosaurs: no audio blocker, purely visual — proceed independently, audio layer (footstep SFX) still pending source discovery.

## Next Audio cycle priorities
1. Retry footstep/raptor vocalization searches with alternate query terms.
2. Escalate Supabase JWT fix — 4 cycles of TTS upload failures blocks in-game voice line playback.
3. Once MetaSounds import path is confirmed working (via VFX/Integration agent), wire the 2 ambient actors created this cycle to actual Sound Wave assets.
