# Audio Design — Cycle 006 (Agent #16)

**Bridge status:** HEALTHY — 3/3 `ue5_execute` Python calls completed cleanly (3.0s audit, 9.1s ambient setup, 6.1s tagging/damage-flash), zero timeouts.

## What was produced this cycle

### 1. Narration VO (ElevenLabs TTS — 2 lines)
- **T-Rex Proximity Warning**: "A low rumble shakes the ground. Somewhere close, something massive is moving through the trees. Stay still. Stay quiet. Let it pass." — tied to the `Audio_ProximityShake_Radius_1500` tag now on all TRex actors.
- **Damage Warning**: "You're hurt. Bleeding slows you down out here — find shelter, bind the wound, and don't let the smell of blood carry on the wind." — descriptive, survival-realistic, zero mystical language.
- Both generated successfully at the synthesis layer; **Supabase Storage upload failed again** with `403 Invalid Compact JWS` (same infra issue flagged by #14 and #15 the last 2 cycles — now a 3-cycle-confirmed studio-wide storage token problem, not agent-specific). Scripts are preserved here for re-run once storage is fixed.

### 2. SFX sourced from Freesound (4 searches)
- **Heavy footstep/dust impact** — no strong matches this query; will retry with narrower terms next cycle (e.g. "dinosaur footstep boom").
- **Forest wind ambient loop** — "Forest Rainstorm 01/02" (Freesound #648474/#648475, field-recorded cabin ambience, wind+rain, no thunder).
- **T-Rex roar/growl** — "T-Rex Calls" (#529462, layered alligator/lion/elk composite) + 4 clean vocal-performance roar/growl takes (#607937–607940). Recommended primary: #529462 for variety, #607939 for a clean hero roar.
- **Campfire crackle loop** — "Campfire crackling - Loop" (#620324, clean 30s loop) — best candidate for the camp hub ambient bed.

### 3. Live UE5 world changes (via Python, no C++ touched)
- Tagged all `TRex_*` actors with `Audio_ProximityShake_Radius_1500` and `Audio_RoarCue_TRexCalls` — ready for #17 VFX to wire actual `CameraShakeBase` trigger logic within this radius, and for future MetaSound roar cue binding.
- Tagged all `Raptor_*` / `Brachio_*` / `Trike_*` actors with footstep-dust cues: `Audio_FootstepDust_Light` (bipedal) vs `Audio_FootstepDust_Heavy` (quadruped/large) — ready for #17 to bind Niagara dust emitters per movement tick.
- Spawned **2 AmbientSound actors** at the content hub (X=2100,Y=2400 per `hugo_naming_dedup_v2` composition priority): `Audio_ForestWind_Hub_001` and `Audio_CampfireCrackle_Hub_001` — placeholders ready for the SoundCue/MetaSound assets above once storage/import is confirmed.
- Set the single `DirectionalLight` to a golden-hour base pose (pitch -45°, guarded within the -30/-60 safe range) and tagged it `Audio_DayNightCycle_GoldenHourBase` — this is the static baseline; a true rotating day/night cycle requires either a Sequencer track or a `RotatingMovementComponent`-driven Blueprint actor, which is a Blueprint-graph task (flagged for #17/#19, not achievable via one-shot Python without a dedicated BP asset).
- Spawned `Audio_DamageFlashVolume_Hub_001` (PostProcessVolume, vignette override initialized to 0.0) as the hookup point for a red damage-flash effect — actual on-hit trigger logic (vignette intensity ramp + color grading pulse) needs a UMG widget or Blueprint event graph, handed off to #17 VFX / UI.
- All changes saved to `MinPlayableMap`. No duplicate actors created — reused existing TRex/Raptor/Trike/Brachio actors per the anti-duplication rule instead of stacking new audio-prefixed clones on top of them.

## Decisions & rationale
- Followed `hugo_no_cpp_h_v2` absolute rule: zero `.cpp`/`.h` files touched. All audio hookups exist as live actor tags and spawned actors in the running headless editor, queryable via `get_all_level_actors()`.
- Followed `hugo_naming_dedup_v2`: no new `_Audio_001_AI` clones on top of existing dinosaur actors — tags were added directly to the existing TRex/Raptor/Trike/Brachio actors instead.
- Followed `hugo_no_camera_v2`: viewport camera untouched; only the DirectionalLight actor rotation was modified (a lighting actor, not the editor camera).
- Reused #15's dialogue infra-failure diagnosis rather than re-diagnosing — this is now a **confirmed 3-cycle recurring blocker**, escalating to #01/#19 for a storage token refresh.

## Dependencies / handoff to #17 VFX Agent
1. Wire `CameraShakeBase` (or a simple `PlayerCameraManager` shake call) triggered when player is within `1500` units of any actor tagged `Audio_ProximityShake_Radius_1500`.
2. Bind Niagara footstep-dust emitters to `Audio_FootstepDust_Light` / `Audio_FootstepDust_Heavy` tagged actors, keyed to their movement/animation notify events.
3. Build the red damage-flash effect on `Audio_DamageFlashVolume_Hub_001` — ramp `VignetteIntensity` + a red color-grade pulse on player-hit events, then decay back to 0.
4. Build a proper day/night cycle: either a Sequencer level sequence or a Blueprint actor with `RotatingMovementComponent` driving the tagged `DirectionalLight` (`Audio_DayNightCycle_GoldenHourBase`) through a full rotation — this needs the Blueprint editor, not raw Python.
5. Once #01/#19 confirm the Supabase storage token is fixed, re-run the 2 TTS scripts above (T-Rex proximity + damage warning) plus #15's 4 dialogue lines to get persistent URLs for MetaSound Wave asset import.

## Infra flag (escalating — 3 consecutive cycles)
`text_to_speech` Supabase Storage upload fails studio-wide with `403 Invalid Compact JWS` since at least cycle 004. Synthesis itself always succeeds; only URL persistence is broken. Recommend #01/#19 treat this as a blocking infra ticket for next cycle — it's now affecting Narrative (#15), Audio (#16), and likely VFX/Quest agents relying on generated asset URLs.
