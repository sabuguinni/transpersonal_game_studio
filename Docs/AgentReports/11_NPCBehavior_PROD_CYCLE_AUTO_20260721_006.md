# NPC Behavior Agent #11 — Report PROD_CYCLE_AUTO_20260721_006

## Bridge status: UP
6x real `ue5_execute` calls (`command_type='python'`), all completed in ~3.0s each, zero timeouts.

## Directive conflict (resolved per absolute rule)
The cycle task asked for creation of `Source/TranspersonalGame/AI/TRexBehavior.cpp`.
This directly violates the standing global rule `hugo_no_cpp_h_v2` (importance MAX, NO EXCEPTIONS):
*"NEVER call github_file_write to create or modify .cpp or .h files. C++ is inert in this headless
editor... Any .cpp/.h write is 100% wasted execution time with zero effect on the live game."*
Per the rule's own instruction, this task step was **skipped** and replaced with real, verifiable
work in the live UE5 world (see below), which is what actually affects the running game.

## Verification results
1. **`Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp`** — file read returned
   `content: "undefined"`, size 9 bytes. This is effectively an empty/placeholder stub, NOT a real
   controller implementation. Flagging for Combat & Enemy AI Agent (#12): this file needs real
   content or should be removed if superseded by another system — but per the no-.cpp/.h rule, any
   fix must happen through in-editor Blueprint/Python configuration, not through a new C++ write.
2. **`Source/TranspersonalGame/Core/Survival/SurvivalComponent.h`** — CONFIRMED present and well
   formed (8112 bytes). Exposes Health/Hunger/Thirst/Stamina/Fear/Temperature stats, drain rates,
   damage thresholds, biome condition hooks (`UpdateBiomeConditions`), and Blueprint-callable API
   (`ApplyHealthDamage`, `ConsumeFood`, `ConsumeWater`, `AddFear`, etc.). This exists as source but,
   per engine rules, has zero live effect unless it was compiled into the pre-built binary before
   this headless session started — no action needed here, just confirming presence for #12/#03.

## Real work performed in the live UE5 world (verifiable via command_id)
Audited all "TRex"-labeled actors inside the playable core (X -3000..5000, Y -1000..5500) — 108
actors matched, spanning VFX emitters, audio proximity/roar sources, dust bursts, and
`RoarDistortion` camera-shake source anchors tied to TRex mesh actors (e.g. `TRex_Savana_005` through
`TRex_Savana_074`).

- Ground-truth method: line trace from `z+5000` to `z-5000` at each actor's XY against
  `TraceTypeQuery1` (WorldStatic/Landscape channel), reading `HitResult.to_tuple()` index 0
  (blocking hit) and index 4 (impact location) — worked around the fact that `HitResult` exposes no
  direct Python property getters (`get_editor_property` fails on snake_case aliases; had to use
  `to_tuple()` positional access instead).
- Result: **50 actors already correctly grounded** (within 50 units of trace-derived terrain height).
- **58 actors were floating or offset** (mostly `RoarDistortion` shake sources hardcoded at
  `z=400` or `z=104`, and several audio/VFX anchors sitting well below or above the real landscape
  height) — all **repositioned via `set_actor_location`** to `ground_z + 5.0`, preserving X/Y.
- **0 actors returned no hit** — all 108 successfully resolved against the landscape.
- No actors were duplicated or deleted (respecting `REUSE FIRST` / actor cap rule). No mesh, material,
  or collision properties were touched — only position.

### Example fixes (label, old_z → new ground_z)
- `TRex_Savana_005_grazing_Posed_RoarDistortion`: 400.0 → 1189.2
- `TRex_Savana_006_alert_Posed_RoarDistortion`: 400.0 → 1615.3
- `TRex_Savana_041_midstride_Posed_RoarDistortion`: 104.0 → 1266.2
- `Audio_TRexProximity_001`: 130.0 → 1555.7
- `VFX_DustBurst_TRex_Savana_001_alert_Posed`: 74.2 → 1631.3

No `EditorLevelLibrary.save_current_level()` was called this cycle since remaining follow-up
(auditing the actual dinosaur *mesh/skeletal* actors distinct from these effect-anchor actors) is
still pending for a future cycle — per the "save only when change is fully complete" rule, this
partial pass (effect anchors only, not yet the core mesh actors) is left unsaved to avoid a
premature/blind save.

## Decisions & justification
- Skipped `TRexBehavior.cpp` creation — inert file, zero effect, contradicts MAX-importance rule.
- Did NOT touch `TranspersonalCharacter PLAYER0`, Landscape, foliage, Terrain_Savana sublevel,
  camera, or sun — all HANDS OFF per PLAYABLE-FIRST v4.
- Did NOT call `NiagaraSystemFactoryNew()` — respected the freeze-risk rule.
- Chose in-editor grounding fixes as the actual "dinosaur behavior" contribution this cycle because
  correct spatial grounding of TRex-associated actors (audio/VFX/shake anchors) is a prerequisite
  for any believable behavior tree work — an NPC/dinosaur that floats or clips through terrain
  breaks immersion regardless of its AI logic.

## Next agent (#12 — Combat & Enemy AI Agent) should focus on
1. `DinosaurCombatAIController.cpp` is effectively empty (9 bytes, "undefined" content) — needs a
   real implementation strategy that works within the headless-editor constraint (Blueprint-based
   AI or Python-driven Behavior Tree assets, NOT relying on new C++ compilation).
2. Distinguish between the 108 **effect-anchor actors** (audio/VFX/shake sources, now grounded) and
   the actual **TRex skeletal mesh actors** driving movement/animation — the mesh actors still need
   a dedicated grounding + patrol-radius audit (5000-unit patrol / 3000-unit chase / 300-unit attack
   ranges per this cycle's directive) in a follow-up pass.
3. `SurvivalComponent.h` (fear stat, `AddFear`/`ReduceFear`) is ready to wire into dinosaur
   proximity/chase logic for player fear feedback — confirm with #03 whether it's already compiled
   into the running binary before assuming Blueprint access to it works.
