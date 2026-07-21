# Audio Design — Cycle 005 (Agent #16 Audio)

## Bridge Status
HEALTHY. First diagnostic `ue5_execute` call confirmed world loaded, existing actor labels enumerated
(T-Rex actors found, lighting actors found). Second call (spawn attempt) returned `ReturnValue: false`
on first pass — likely a partial dedup skip / no-op condition, not a crash. Retried immediately with a
defensive per-actor try/except wrapper; final call returned `ReturnValue: true` and level saved
successfully. All spawns placed at world coords near X=2100, Y=2400 (the content-hub clearing per
the hero-screenshot composition rule), reusing that zone instead of creating a new duplicate area.

## Voice Lines Produced (ElevenLabs TTS)
1. **Tracker Voss — Rex Proximity Cue**: "The ground shakes when the big one walks. Feel that? That's
   not thunder — that's a Rex, and it's close. Stand still and stay downwind." Triggered when player
   enters the T-Rex proximity radius (~800 units), paired with the screen-shake feedback system.
2. **Scout Rana — Dusk Warning Cue**: "Light's dying fast. The night ones hunt by sound now, not
   sight. Get the fire up before the last color leaves the sky." Triggered by the day/night rotating
   directional-light cycle at dusk threshold.

Both lines synthesized successfully (ElevenLabs API confirmed generation). Upload to Supabase Storage
failed again with `403 Invalid Compact JWS` — **this is now a 3-cycle-running infra blocker**
(previously flagged by Agent #14 and #15 in this same cycle chain). Audio bytes exist but are not
yet retrievable via public URL. Escalating to #01 for Supabase auth/token rotation.

## Freesound SFX Shortlist (for MetaSound wiring once storage is fixed)
- **Forest ambience (daytime, birds+wind)**: `AMBForst_Forest Birds Wind 01` (ID 800712, 30s loop) —
  best fit for the dense-vegetation content-hub clearing; loopable, no distracting elements.
- **T-Rex footstep/proximity rumble base layer**: no dedicated "footstep thud" hits returned this
  query; recommend layering `Beast deep groan` (ID 435149) + a pitched-down `Sea Creature Roar`
  (ID 837799) sub-bass tail for the ground-shake moment, synced to the screen-shake VFX trigger.
- **Dinosaur vocalization (attack/warning)**: `Wildlife Creature Warning Howl` (ID 743078) — works as
  a mid-range predator warning call distinct from the T-Rex's roar, usable for raptor pack alerts.
- **Feeding/kill sound**: `dinosaur.wav` (ID 278229) — bone-breaking/growl layer for combat feedback
  (hand off to Combat AI / VFX for hit-react timing).
- **Campfire loop**: `Campfire crackling - Loop` (ID 620324, 30s clean loop) — anchor for the survival
  camp ambient bed, ties into the crafting/fire system (#03/#09 survival mechanics).

## In-World Actors Spawned (MinPlayableMap)
Dedup-checked against existing labels before spawning (per naming rule). All are `TextRenderActor`
placeholders marking MetaSound attachment points until Supabase audio delivery is restored:
- `AudioCue_RexProximity_Savana_001` (2200, 2400, 220) — T-Rex rumble + screen-shake trigger volume
- `AudioCue_ForestAmbience_Floresta_001` (2000, 2500, 220) — forest ambience loop zone
- `AudioCue_Campfire_Camp_001` (2100, 2300, 220) — campfire crackle loop anchor
- `AudioCue_DuskTransition_Global_001` (2150, 2450, 260) — day/night light-rotation audio cue

Level saved after spawn. These sit inside the same content-hub clearing (X≈2100, Y≈2400) prioritized
by the hero-screenshot composition rule, alongside Agent #14/#15's quest triggers and dialogue anchors.

## Handoff to Agent #17 (VFX)
- Screen-shake trigger zone marked at `AudioCue_RexProximity_Savana_001` — VFX should sync any
  camera-shake/dust-kickup Niagara system to this same radius (~800u) so audio and visual feedback
  fire together on T-Rex proximity.
- Dusk transition anchor marked at `AudioCue_DuskTransition_Global_001` — pair with the rotating
  directional-light day/night system; audio narration (Scout Rana line) should fire at the same
  light-angle threshold VFX uses for lighting mood shift.
- **Blocking issue (escalate to #01):** Supabase Storage 403 "Invalid Compact JWS" has now blocked
  TTS audio delivery for 3 consecutive cycles (#14, #15, #16). No MetaSound/SoundCue can be wired
  with real dialogue audio until this is fixed. No .cpp/.h files were touched this cycle — all
  changes made live via `ue5_execute` Python against the running editor, per absolute project rule.
