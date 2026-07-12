# Core Systems Report — Cycle PROD_CYCLE_AUTO_20260712_004
## Agent #03 — Core Systems Programmer

### Absolute Constraint Respected
`hugo_no_cpp_h_v2` (imp:MAX) — **zero .cpp/.h files written**, 39th consecutive cycle.
This cycle's directive again requested creating `SurvivalComponent.h/.cpp` and wiring it into
`TranspersonalCharacter` via `CreateDefaultSubobject`. Declined for the same standing reason
verified across 38+ prior cycles: this headless editor runs a pre-built binary that never
recompiles new C++. Any `.h/.cpp` write is 100% wasted execution with zero effect on the live
game. All engine-facing work this cycle was done live, in-editor, via `ue5_execute` Python
against the running `MinPlayableMap`.

### Work Performed (Live, via ue5_execute)

**1. Collision/Physics Audit — Content Hub (X=2100, Y=2400, r=2500)**
- Swept all actors within the hub radius (the composition zone prioritized by
  `hugo_hub_quality_v2_fix`).
- Verified/forced `CollisionEnabled = QueryAndPhysics` on every `StaticMeshComponent` so the
  player character cannot walk through trees, rocks, or dinosaur placeholders.
- Enforced mobility policy: **Static** for terrain/props/vegetation (performance), **Movable**
  for dinosaur actors (TRex/Raptor/Brachiosaurus/Triceratops), to keep the door open for future
  AI locomotion without breaking current placeholders.
- Enabled `generate_overlap_events` broadly so future gameplay (interaction, proximity fear/AI
  triggers) has overlap data available.

**2. Character/PlayerStart Sanity Check**
- Confirmed `TranspersonalCharacter` class is loadable via `unreal.load_class` and counted live
  instances in the level (boundary contract validated by #02 this same cycle).
- Confirmed `PlayerStart` count and logged its world position to ensure it is not embedded
  inside a collider (would block spawn).
- Logged count of actors currently simulating physics (`simulate_physics=true`) as a baseline —
  these are the future ragdoll/destruction candidates; currently near-zero, as expected since no
  destruction system exists yet.

**3. NavMesh & Interaction Trigger Collision Semantics**
- Verified `NavMeshBoundsVolume` presence/extent (required before any dinosaur AI pathing work
  from #11/#12 can begin).
- Forced all actors labeled `*Trigger*` (the 3 interaction triggers from map spec) to
  `CollisionEnabled = QueryOnly` — triggers must **overlap**, never **block**, the player.
- Forced all ground/terrain-labeled `StaticMeshActor`s to `QueryAndPhysics` collision with
  `simulate_physics = False` — terrain must always block movement and never fall/ragdoll.

**4. Dinosaur Pawn Collision Channel Pass**
- Set every dinosaur placeholder's primitive components to
  `CollisionObjectType = ECC_Pawn`, so future collision responses (player vs. dinosaur,
  dinosaur vs. dinosaur) can be authored correctly by #11/#12 without channel conflicts.
- Counted static, non-dino, non-terrain props as candidates for a future lightweight
  destruction system (tree fall/knockback), without implementing said system yet — that
  requires actual C++ (Chaos Destruction components), currently blocked by the no-recompile
  constraint.

### Technical Decisions
- **Collision policy codified operationally** (not in source, since C++ can't be recompiled):
  Terrain/props = Static mobility + blocking collision + no physics simulation.
  Dinosaurs = Movable mobility + blocking collision + Pawn channel.
  Triggers = any mobility + QueryOnly (overlap-only) collision.
- This is the same physics/collision contract Casey Muratori/Mike Acton doctrine calls for:
  simple, data-driven, testable in isolation via live inspection — no hidden state, no
  custom movement code written from scratch (UE5's `UCharacterMovementComponent` remains
  untouched and authoritative).
- Ragdoll/destruction remain **out of scope** until a genuine C++ recompile path exists; faking
  it via Blueprint-only physics simulation on placeholder meshes would create inconsistent,
  untestable behavior contradicting the "physics as emotional signature" principle — better to
  report the limitation than ship a fake system.

### Level State
- `MinPlayableMap` saved after all fixes applied.

### Files Written
- `Docs/Architecture/Cycle_004_CoreSystems_PhysicsCollisionReport.md` (this report)

### Dependencies / Next Agent Input
- **#04 Performance Optimizer**: mobility/collision policy above is now consistent — safe to
  profile draw calls / collision overhead on the hub without hidden per-actor variance.
- **#11/#12 (NPC/Combat AI)**: NavMesh bounds confirmed, dinosaur collision channel set to
  `ECC_Pawn` — safe to begin behavior tree work assuming this channel contract.
- **#05/#06**: terrain/prop mobility+collision policy is now uniform; any newly spawned
  vegetation/rocks should follow the same Static+QueryAndPhysics convention.
- **Escalation to Engine Architect (#02) / Studio Director (#01)**: a genuine ragdoll/destruction
  system requires either (a) a C++ recompile pipeline being restored, or (b) an explicit decision
  to implement physics reactions via Blueprint/Control Rig only. This is a resource/technology
  decision outside Core Systems Programmer autonomy — flagging per the 3-situations escalation
  rule (irreversible architecture decision).

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Collision/mobility audit + fix pass over all actors in content hub radius (2100,2400,r=2500): forced QueryAndPhysics collision, Static/Movable mobility split (props vs dinosaurs), overlap events enabled.
- [UE5_CMD] TranspersonalCharacter class/instance validation + PlayerStart position sanity check + physics-simulation baseline count.
- [UE5_CMD] NavMeshBoundsVolume validation + interaction trigger collision fixed to QueryOnly + ground/terrain actors forced to blocking, non-simulated collision. Level saved.
- [UE5_CMD] Dinosaur pawn collision channel pass (ECC_Pawn) across all placeholder dinosaurs + destruction-candidate prop count logged.
- [FILE] Docs/Architecture/Cycle_004_CoreSystems_PhysicsCollisionReport.md
- [NEXT] #04 Performance Optimizer to profile the now-uniform collision/mobility setup; #11/#12 to build AI on top of validated NavMesh + ECC_Pawn contract; #01/#02 to decide on C++ recompile path before any real ragdoll/destruction system can be attempted.
