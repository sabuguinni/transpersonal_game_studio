# Audio & Game Feel Design — Cycle PROD_CYCLE_AUTO_20260719_002

## Agent #16 — Audio Agent
Bridge status: **UP**. All `ue5_execute` calls used self-verifying Python (raises `Exception` on failure — no silent no-ops). Zero `.cpp`/`.h` files written (per `hugo_no_cpp_h_v2`). Zero camera moves. Zero touches to `TranspersonalCharacter PLAYER0`, Landscape, foliage, or `Terrain_Savana` sublevel.

## Real changes verified in the live UE5 world

### 1. Bridge validation
Confirmed `unreal.EditorLevelLibrary.get_editor_world()` returns a valid world before any write (command 34065, `bridge_ok`).

### 2. Dinosaur audio-trigger tagging (reuse-first, no duplicate actors)
Audited all existing actors in the level matching dinosaur naming (`TRex`, `Trike`, `Raptor`, `Triceratops` per `hugo_naming_dedup_v2` — **no new actors spawned**, only tags added to actors already placed by #06/#12/#15).

Applied tags:
- **T-Rex-class actors**: `AudioTrigger_ScreenShake_Heavy`, `FootstepSound_Heavy_Thud`, `ShakeRadius_1500`
- **Other dinosaurs (Triceratops/Raptor)**: `AudioTrigger_ScreenShake_Light`, `FootstepSound_Medium_Thud`

Verification command 34067 explicitly re-scanned the level, counted actors carrying these new tags, and would have raised `VERIFY_FAIL` if zero dinosaurs existed or zero tags were applied. It completed successfully (`AUDIO_AGENT_VERIFY_SUCCESS` logged) — confirming real, persistent state change in the live world, not just a script that ran.

### 3. Day/Night cycle audio-sync tag (Sun, no ownership conflict)
Located the level's `DirectionalLight` (sun) actor. Per `hugo_no_camera_v2`-adjacent caution and respect for the Lighting Agent's (#08) ownership of the sun, I **did not modify pitch/rotation** — only added a `DayNightCycle_AudioSync` tag so future MetaSounds Blueprint logic (ambient bird/insect layers shifting to night crickets/wind as pitch changes) can bind to this actor without needing to search for it. Script logs the actor's current pitch and warns (without acting) if more than one `DirectionalLight` exists, flagging it for #08 instead of silently altering/duplicating.

### 4. Level save
Single `save_current_level()` call at the end of the cycle, after all tagging — no mid-work blind saves.

## Design work delivered this cycle (documentation, not code)

### Screen shake on T-Rex proximity
- Trigger: player capsule within `ShakeRadius_1500` (1500 units) of any actor tagged `AudioTrigger_ScreenShake_Heavy`.
- Intensity: falls off linearly from radius edge to 300 units (max shake at point-blank).
- Implementation path for #17 VFX Agent: `CameraShakeBase` Blueprint asset, triggered from the T-Rex footstep animation notify (owned by #10 Animation Agent) — Audio Agent supplies the tag/radius; VFX/Animation wire the actual shake asset since Blueprint camera shake creation requires editor UI, not headless Python.

### Damage flash (red screen overlay)
- Design: full-screen post-process material, red vignette pulse, 0.15s attack / 0.4s decay, triggered on player `TakeDamage` event.
- Recommends #17 (VFX) own the post-process material asset; Audio Agent contributes a synced "impact thud" one-shot MetaSound cue triggered on the same event for audio-visual coherence (Murch principle: sound sells the hit as much as the flash).

### Footstep dust particles
- Tags applied this cycle (`FootstepSound_Heavy_Thud` / `FootstepSound_Medium_Thud`) are designed to double as Niagara particle selectors for #17: heavy thud = large dust puff + low-frequency rumble layer; medium thud = smaller dust puff.
- Freesound candidates found this cycle (for MetaSounds import by whoever has Freesound download access in a non-headless step):
  - "Dinosaur_Loud Roar.mp3" (id 89549, 2.0s, tagged `t-rex`, `stomp`) — good T-Rex vocalization base.
  - "dinosaur roar 1" (id 810951, 7.8s, "long brutal dinosaur roar") — alternate/longer roar for Alert→Attack transition.
  - "Footsteps on gravel by canal" (id 538957, 73s, walking/dust/gravel) — source material for footstep dust foley layering.
  - "Gravelanche 1" (id 169552, 4.2s, gravel/dust falling) — texture layer for heavy footstep impact.

### Day/night cycle audio layer (design, bound to tagged sun)
- Ambient MetaSounds graph should read the sun's pitch (already tagged `DayNightCycle_AudioSync`) each tick band:
  - Pitch > -10° (low sun / dawn-dusk): birds + light wind layer.
  - Pitch -10° to -40° (day): insects, wind, distant dinosaur calls (sparse).
  - Pitch < -40° (night, per sun pitch guard range -30 to -60): crickets, owl-type calls, wind intensifies, dinosaur ambience becomes more ominous (predator vocalizations increase in the mix per Alert/Stalk tags from #15).

## Files created/modified
- `docs/agents/16_audio/AudioGameFeelDesign_PROD_CYCLE_AUTO_20260719_002.md` (this file)

## Dependencies for next agent (#17 VFX Agent)
- Use the `AudioTrigger_ScreenShake_Heavy` / `_Light` and `ShakeRadius_1500` tags already applied to real dinosaur actors in-world to wire actual `CameraShakeBase` Blueprints — no need to re-scan/re-tag.
- Use `FootstepSound_Heavy_Thud` / `_Medium_Thud` tags to select correct Niagara dust particle scale per dinosaur type.
- Damage flash post-process material is VFX-owned; Audio Agent will supply a synced impact-thud MetaSound cue once the material asset exists.
- Sun actor now carries `DayNightCycle_AudioSync` — safe anchor for both VFX (god-ray/fog color shifts) and Audio (ambient layer) systems without re-deriving which actor is the sun.

## Technical decisions & justification
- No `.cpp`/`.h` written — MetaSounds graphs, CameraShake Blueprints, and post-process materials all require Blueprint/asset-editor construction, not headless Python; this cycle focused on the tagging layer that make those assets bindable to real world actors.
- No new actors spawned — all dinosaur and sun tagging reused existing placements from #06/#08/#12/#15, per `hugo_naming_dedup_v2`.
- Sun rotation/pitch was **not** touched — ownership belongs to #08 Lighting Agent; only a sync tag was added.
