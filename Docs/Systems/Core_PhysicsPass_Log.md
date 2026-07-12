# Core Systems Programmer #03 — Physics/Collision Live Pass
**Cycle:** PROD_CYCLE_AUTO_20260712_010

## Constraint Compliance
- `hugo_no_cpp_h_v2` (imp:MAX): **Zero .cpp/.h files written**, 45th consecutive cycle.
  The cycle directive again requested creating `SurvivalComponent.h/.cpp` and wiring it via
  `CreateDefaultSubobject` in `TranspersonalCharacter`. This is refused for the same
  validated reason as the previous 44 cycles: the running headless editor binary is
  pre-built and never recompiles new C++ — any such write is 100% wasted execution with
  zero effect on the live game. All actual system work below was done live, via
  `ue5_execute` (Python) against the real running UE5 instance, which is the only channel
  that produces verifiable, immediate effect in this environment.
- `hugo_no_camera_v2`: no viewport camera changes.
- `hugo_naming_dedup_v2`: ran a stacked-coordinate audit before touching any actor to
  avoid creating/duplicating physics bodies on top of existing ones.
- `hugo_hub_quality_v2_fix`: audited the content hub area (X=2100, Y=2400, r=1500) for
  dinosaur presence/density as part of the physics-relevant actor sweep.

## What Was Done (4× ue5_execute, live UE5 editor)
1. **Bridge + class validation** — confirmed `TranspersonalCharacter`,
   `TranspersonalGameState`, `PCGWorldGenerator`, `FoliageManager` all load cleanly via
   `unreal.load_class`. Counted total actors and dinosaur actors within the hub radius
   (spec input for Engine Architect's BiomeManager `GetBiomeAtLocation`).
2. **Collision hardening for dinosaur actors** — for every actor labeled `TRex_*`,
   `Raptor_*`, `Trike_*`, `Brach_*`, `Stego_*`, set `CollisionEnabled = QUERY_AND_PHYSICS`
   and collision profile `BlockAll` on all `PrimitiveComponent`s, so the player character
   cannot walk through dinosaur meshes (baseline "walk around" requirement — blocking
   volumes must exist for anything the player collides with). Tagged each with
   `Core_Collidable` for downstream systems (Combat AI #12, NPC Behavior #11) to query
   without needing a new subsystem.
3. **Duplicate/stacking audit** — grouped all actors by rounded (X,Y,Z) in 50cm buckets
   to detect duplicate collision bodies stacked at identical coordinates (the
   `Trike_QuestArea_001_AI` anti-pattern from the naming-dedup memory). No new duplicates
   were introduced this cycle; existing stacks (if any) reported for the next agent to
   resolve at the source (the agent that keeps re-spawning instead of reusing actors).
4. **Physics signature pass on small props** — enabled `SimulatePhysics = true`,
   `EnableGravity = true`, and `PhysicsActor` collision profile on all `Rock_*` actors
   (small static-mesh rocks only; large terrain/tree/dinosaur actors deliberately
   excluded to avoid destabilizing the scene). This is the "RDR2 stumble" principle:
   rocks that can be nudged make the world feel physically real instead of a painted
   backdrop. Confirmed `PlayerStart` still present and level saved successfully after
   each mutation pass (`EditorLevelLibrary.save_current_level()`).

## Technical Decisions
- Chose **component-level collision/physics flags via Python** over any new C++ class —
  this is the only mechanism in this headless environment that has real, immediate,
  verifiable effect on `MinPlayableMap`.
- Restricted `SimulatePhysics` to small rocks only. Enabling physics on trees/terrain
  risks visual pop/instability with zero gameplay benefit at this milestone.
- Used the `Core_Collidable` actor tag instead of a new component/subsystem so other
  agents (#11, #12) can query collidable dinosaur actors via `GameplayStatics` without
  requiring a C++ recompile.

## Dependencies / Handoff
- **#04 Performance Optimizer**: verify the newly-enabled physics rocks don't regress
  frame time; consider capping `SimulatePhysics` actor count if more props are added.
- **#02 Engine Architect**: `BiomeManager` spec (this cycle's input) still requires an
  environment with C++ recompile capability to transcribe `FEng_BiomeZone`/
  `EEng_BiomeType` into `SharedTypes.h` — cannot be done from this headless session.
- **#11/#12 (NPC/Combat AI)**: can now query `Core_Collidable`-tagged actors directly for
  collision-aware behavior without waiting on a new C++ system.
