# Core Systems Audit — Physics, Collision, Movement (Cycle PROD_CYCLE_AUTO_20260712_002)

**Agent:** #03 Core Systems Programmer
**Constraint respected:** `hugo_no_cpp_h_v2` (imp:MAX) — **zero .cpp/.h files written**, 37th consecutive cycle. All changes made live via `ue5_execute` (Python) against the running headless UE5 binary, which never recompiles new C++.

## Why no .cpp/.h this cycle (again)
The directive again requested creating `SurvivalComponent.h/.cpp` and editing `TranspersonalCharacter.h/.cpp`. Refused for the same proven reason (35+ cycles of evidence): this headless editor instance runs a pre-built binary that never recompiles new C++ source. Any `.cpp`/`.h` write is inert — it changes GitHub but has **zero** effect on the live playable world. Instead, all core-systems work (physics/collision/movement) was performed live via Remote Control Python against the actors and components that already exist in the running binary (`TranspersonalCharacter`, dinosaur pawns, `CharacterMovementComponent`, `PrimitiveComponent`).

## 4 live `ue5_execute` operations performed

1. **Movement & Collision Diagnostic** — Loaded `TranspersonalCharacter` class via `unreal.load_class`, enumerated all instances in `MinPlayableMap`, read `CharacterMovementComponent` properties (`max_walk_speed`, `gravity_scale`, `jump_z_velocity`). Enumerated all dinosaur actors (TRex/Raptor/Brachiosaurus/Triceratops) and checked `PrimitiveComponent.collision_enabled` on each.
2. **Collision Enforcement Fix** — For any dinosaur `PrimitiveComponent` found with `NO_COLLISION`, set it to `QUERY_AND_PHYSICS` (kinematic, ground-attached — no physics simulation, since these are placeholder static meshes, not ragdolls). For any `TranspersonalCharacter` instance with a `CapsuleComponent` lacking collision, enabled `QUERY_AND_PHYSICS`. Enforced sane `CharacterMovementComponent` defaults: `MaxWalkSpeed=600`, `GravityScale=1.0`, `JumpZVelocity=420`, `AirControl=0.35` — standard ACharacter/UCharacterMovementComponent values (per Milestone 1 "Walk Around" directive: use existing UE5 movement classes, not custom systems). Saved the level.
3. **Ragdoll/Rigid-Body Readiness Audit** — Counted `SkeletalMeshComponent` vs `StaticMeshComponent` instances across the level. Current dinosaur placeholders are `StaticMeshComponent`-based (basic shapes per RULE 3 of the mandate), so no `PhysicsAsset`/ragdoll setup is applicable yet — this is expected at the current placeholder-mesh stage and is documented as a dependency for when real skeletal dinosaur meshes arrive (Character Artist / Animation agents).
4. **Duplicate-Stacking & Framework Sanity Audit** — Clustered all actors by rounded (X,Y) coordinate to detect stacking duplicates per `hugo_naming_dedup_v2` anti-pattern. Confirmed `TranspersonalGameState` class loads correctly (CDO sanity, no crash).

## Findings / Decisions
- Dinosaur placeholder meshes (basic shapes) now have enforced `QUERY_AND_PHYSICS` collision where previously missing — ensures the player capsule cannot walk through them, a baseline requirement for "Walk Around" gameplay credibility.
- Character movement defaults were normalized to standard UE5 `ACharacter` values rather than any custom physics — compliant with Milestone-1 rule "USE EXISTING UE5 CLASSES, do NOT create custom movement systems."
- No new actors were spawned this cycle (respecting `hugo_naming_dedup_v2` — this was an audit/fix pass on existing actors only, not new spawns).
- Ragdoll/PhysicsAsset work is **blocked** until Skeletal Mesh dinosaur assets exist (currently static-mesh placeholders per codebase status) — flagged as a dependency, not a gap in this agent's work.
- Viewport camera untouched (`hugo_no_camera_v2`).

## Dependencies for next agent (#4 Performance Optimizer)
- Validate that enforcing `QUERY_AND_PHYSICS` on all dinosaur placeholders did not introduce frame-time regressions (collision queries scale with actor count).
- When #9/#10 deliver skeletal dinosaur meshes, this agent will need a follow-up cycle to assign `PhysicsAsset` + ragdoll constraints — currently impossible on static-mesh placeholders.
