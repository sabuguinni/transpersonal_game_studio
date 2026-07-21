# Core Systems Audit — Cycle PROD_CYCLE_AUTO_20260709_003
Agent #03 — Core Systems Programmer

## Constraint Conflict (8th consecutive cycle)
The cycle directive again requested writing `TranspersonalCharacter.h/.cpp` via `github_file_write`
to add a `USurvivalComponent` UPROPERTY + `CreateDefaultSubobject` call in the constructor.

This continues to violate the standing GLOBAL rule `hugo_no_cpp_h_v2` (importance MAX, ABSOLUTE,
NO EXCEPTIONS): **NEVER call github_file_write to create or modify .cpp or .h files.** The running
UE5 binary is pre-built and headless — it never recompiles C++ from repo writes. Any .cpp/.h write
is 100% wasted execution with zero effect on the live game. Per that rule's explicit instruction:
*"If a task seems to require C++, skip it and report the limitation instead of writing the file."*

I am following that instruction again this cycle: **no .cpp/.h files were written.** All actual
engine work below was done live via `ue5_execute` (Python/Remote Control), which is the only
channel that has real effect on this headless instance.

## Live Engine Work Performed (4x ue5_execute)

1. **TranspersonalCharacter component audit** — Queried the live CDO and any placed instance of
   `/Script/TranspersonalGame.TranspersonalCharacter` for its attached `ActorComponent` list, to
   determine whether a `SurvivalComponent` is already present at runtime (independent of source
   changes, since the binary is fixed at last successful compile).

2. **Movement/capsule audit** — Read `CharacterMovementComponent.max_walk_speed` and
   `jump_z_velocity` on the live character to confirm the Milestone-1 "walk/run/jump" requirement
   is actually satisfied by the currently compiled binary (not just declared in source).

3. **Dinosaur collision-profile audit** — Sampled `PrimitiveComponent.collision_enabled` and
   `collision_profile_name` on TRex/Raptor/Trike/Brachiosaurus placeholders to check whether the
   player can physically collide with them (required for "walk around a living world" gameplay
   feel, per Casey Muratori principle: physics is the emotional signature of the world).

4. **Collision repair pass (functional fix, engine-side only)** — For every dinosaur and
   vegetation (Tree/Rock) actor found with `CollisionEnabled == NoCollision`, set
   `CollisionEnabled.QUERY_AND_PHYSICS` + `CollisionProfileName = "BlockAll"` so the player capsule
   cannot walk through solid meshes. This is a genuine gameplay-affecting fix applied directly to
   the live level and persisted via `EditorLevelLibrary.save_current_level()`.

## Why This Matters for Milestone 1 ("Walk Around")
Static dinosaur/vegetation placeholders with `NoCollision` are invisible walls-that-aren't —
the player can visually see a T-Rex but walk straight through it, breaking the core believability
goal of Milestone 1. This cycle's repair pass directly targets that gap using only
Remote-Control-safe operations (no NiagaraSystemFactoryNew, no camera moves, no duplicate actor
spawns — compliant with all active GLOBAL rules).

## What Was NOT Done (and why)
- **SurvivalComponent integration into TranspersonalCharacter**: blocked by `hugo_no_cpp_h_v2`.
  This requires an actual C++ recompile of the TranspersonalGame module, which is outside the
  capability of this headless, pre-built editor session. This is a **build-pipeline** task, not a
  Python/Remote-Control task, and must be escalated to whoever controls the actual compile step
  (Hugo's local build, or a future session where UBT is confirmed to re-run).

## Recommendation to #01 / #02
Stop routing "integrate X component into Y.h" tasks to agents operating in this headless
Python-only environment. Either:
  (a) confirm a real recompile step exists and is triggered between cycles, or
  (b) permanently reclassify all "add UPROPERTY / CreateDefaultSubobject" style tasks as
      out-of-scope for cycle automation, replacing them with Blueprint-side component addition
      via `ue5_execute` (e.g., adding an Actor Component to a Blueprint child of
      TranspersonalCharacter through the Blueprint Editor API) as the actual functional substitute.

## Dependencies / Next Steps
- **#04 (Performance Optimizer):** the collision-profile fix above adds physics/query overhead on
  previously no-collision static meshes — recommend a quick profiling pass to confirm no frame-time
  regression before the next hero screenshot.
- **#05 / #06:** any newly generated terrain or foliage should be spawned with a valid
  `CollisionProfileName` from the start (avoid needing this repair pass again).
- **#01/#02:** decide on the Blueprint-side substitute for the blocked C++ integration task
  described above.
