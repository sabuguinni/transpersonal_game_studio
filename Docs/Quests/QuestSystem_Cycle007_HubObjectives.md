# Quest & Mission Design — Cycle PROD_CYCLE_AUTO_20260709_007

**Agent:** #14 Quest & Mission Designer
**Bridge status:** HEALTHY — 5/5 `ue5_execute` Python calls completed cleanly (3.0–3.1s each, zero timeouts).

## Design Philosophy
Per Pawel Sasko / Jonathan Blow doctrine: every quest here is anchored to a **real, physical
world feature** already present in the level — not an abstract marker floating in empty space.
This cycle specifically builds on Agent #13's herd formation work from the same cycle, so the
quest objectives and the crowd simulation are mechanically and spatially coupled.

## What Was Built This Cycle

### 1. Audit pass (naming-dedup compliance)
Scanned all actors within 4000 units of the hero hub (X=2100, Y=2400) for existing `Quest_*`
triggers and herbivore/dinosaur actors, to avoid stacking duplicate objectives on top of
existing content (per `hugo_naming_dedup_v2`).

### 2. Three quest trigger volumes (TriggerBox, real gameplay volumes — not decorative)

| Label | Location | Anchor | Design Intent |
|---|---|---|---|
| `Quest_TrackHerdA_TrackingObjective_001` | (1850, 2150, 120) | Herd A center (Agent #13, heading ≈25°) | Tracking quest: player follows footprints/trail to locate the grazing herd. Emotional beat: first contact with a living, moving group of herbivores — awe, not combat. |
| `Quest_ObserveHerdB_StealthObjective_001` | (2500, 2750, 120), yaw 205° | Herd B center (Agent #13, heading ≈205°, facing Herd A) | Observation/stealth quest: player must approach from downwind cover and document herd behavior without spooking it. Rotated to face the herd's own facing direction, reinforcing that both herds are aware of each other across the clearing. |
| `Quest_GatherMaterials_HubObjective_001` | (2100, 2400, 120) | Hero hub clearing itself | Resource-gathering quest: introduces crafting loop by directing the player to collect the 6 resource pickups scattered in the immediate clearing (see below). |

All three use `/Script/Engine.TriggerBox` — a native UE5 collision volume, no custom C++
gameplay class required. Overlap events will be wired to Blueprint/GameMode logic by the
Narrative Agent (#15) and QA (#18) once dialogue text exists for these beats.

### 3. Six crafting resource pickups (visible, placed props — supports the P9 Survival system)
Scattered in the hub clearing to support the crafting recipes the codebase already anticipates
(Stone Axe: 2 rocks + 1 stick; Campfire: 3 sticks; Water Container: 1 rock + 1 leaf):

- `Resource_Rock_Hub_001` @ (2000, 2300, 30) — Cube mesh, scaled 0.3 (rock-sized boulder chunk)
- `Resource_Rock_Hub_002` @ (2220, 2320, 30) — Cube mesh, scaled 0.3
- `Resource_Stick_Hub_001` @ (2050, 2450, 20) — Cube mesh, scaled (0.08, 0.6, 0.08) (stick proportions)
- `Resource_Stick_Hub_002` @ (2180, 2480, 20) — Cube mesh, scaled (0.08, 0.6, 0.08)
- `Resource_Stick_Hub_003` @ (2120, 2350, 20) — Cube mesh, scaled (0.08, 0.6, 0.08)
- `Resource_Leaf_Hub_001` @ (2260, 2250, 25) — Sphere mesh, flattened scale (0.25, 0.25, 0.1)

These are placeholder primitives (per the same convention as the existing TRex/Raptor/Brachiosaurus
placeholders already in `MinPlayableMap`). They are real `StaticMeshActor` instances with assigned
meshes — not decorative-only, they can be given `UStaticMeshComponent` overlap pickup logic by
the Core Systems Programmer without any respawn work needed.

### 4. Verification + save
Re-scanned the level for all `Quest_*` and `Resource_*` labeled actors post-spawn, then called
`EditorLevelLibrary.save_current_level()`. All spawn calls returned `success: true` with no
exceptions in the bridge log.

## Known Limitation (carried over from Agent #13's note, confirmed again this cycle)
`ue5_execute`'s `result` field only surfaces the **last statement's return value** — in this
bridge, that resolves to a generic `{"ReturnValue": true}` regardless of what a Python variable
named `result` is set to. This means exact runtime-captured actor lists/coordinates could not be
echoed verbatim in this report. Recommendation for next cycle: write findings to a `/tmp/*.txt`
file (as done here — `/tmp/quest_audit_014.txt`) and treat the RC `result` field purely as a
pass/fail signal, not a data channel.

## Compliance Notes
- **No .cpp/.h files written** — per absolute rule `hugo_no_cpp_h_v2` (imp 20, no exceptions).
  This cycle's directive text requested a `CraftingSystem.h/.cpp`, but per the hard override in
  the Brain memory, C++ is inert in this headless editor build and any such write would be
  0% functional. Crafting is instead represented as real placed pickup actors + this design doc,
  ready for a Blueprint-side implementation via Remote Control in a future cycle.
- **No camera moves** — did not touch viewport camera per `hugo_no_camera_v2`.
- **No duplicate actors** — reused Agent #13's herd positions as anchors rather than spawning new
  dinosaur actors; only new content is the 3 trigger volumes + 6 resource pickups, which did not
  previously exist under these labels.
- **Naming convention followed** — `Quest_<Verb+Target>_<Type>_NNN` and `Resource_<Type>_<Area>_NNN`.

## Dependencies / Next Agent Focus
- **#15 Narrative & Dialogue Agent**: write the actual dialogue/journal text for the 3 quest
  beats above (tracking, stealth observation, gathering) — the trigger volumes exist and are
  positioned, but have no narrative payload yet.
- **#3 Core Systems / Crafting follow-up**: implement pickup-on-overlap logic (add/remove from
  inventory) for the 6 `Resource_*` actors via Blueprint or Remote Control property/function
  calls — no new C++ required, `UStaticMeshComponent` overlap events are sufficient.
- **#18 QA**: verify trigger volumes actually fire overlap events in PIE once GameMode wiring
  exists; verify resource pickups are not floating/clipping into terrain (Z values were estimated,
  not raycast-snapped to ground height).
- Future cycle: expand from 3 quests to a short 5-quest arc once Narrative Agent's Bible content
  for this hub area is available, per the "side quest has a complete emotional arc" principle.
