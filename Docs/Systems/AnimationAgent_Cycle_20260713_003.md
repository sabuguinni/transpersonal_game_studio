# Animation Agent #10 — Cycle PROD_CYCLE_AUTO_20260713_003

## Bridge Status: UP
5x `ue5_execute` python calls, all `completed`, zero timeouts, zero camera manipulation, zero .cpp/.h writes (per `hugo_no_cpp_h_v2` and `hugo_no_camera_v2` absolute rules — C++ is inert in this headless editor and never recompiles).

## Critical Dependency Confirmed Missing
- **0 SkeletalMesh assets** and **0 SkeletalMeshComponent actors** exist anywhere in `/Game`.
- **0 AnimBlueprint / AnimInstance assets** exist in the project.
- All dinosaur actors (TRex, Raptors, Brachiosaurus, Triceratops) and the `TranspersonalCharacter` player pawn are built from **StaticMeshComponents**, not skeletal rigs.
- This confirms and extends the gap flagged by Character Artist Agent #09 last cycle: **true Motion Matching / bone-based animation, foot IK, and AnimBlueprint state machines are not technically possible yet** — there is no skeleton to drive. Real animation work (my core mandate: Motion Matching + foot IK) is blocked until a rigged skeletal character/creature pipeline exists (MetaHuman import for the player, or Meshy/rigged FBX import for dinosaurs).

## Real changes made in live MinPlayableMap this cycle
Given the skeletal-mesh gap, I applied the **"frozen animation frame" workaround** mandated by the `hugo_hub_poses_v2_fix` memory (imp:20) — since there is no skeleton to key, I approximated posed, alive-looking silhouettes by rotating each dinosaur's `StaticMeshComponent` relative transform (not the actor root, to avoid disturbing placement/collision):

1. **Audit pass** (calls 33117–33119): Confirmed bridge up, enumerated all dinosaur actors within the (2100,2400) hub radius (3200u), confirmed 0 skeletal actors, 0 AnimBlueprint assets, listed `TranspersonalCharacter` actors present in the level.
2. **Pose pass** (call 33120): Applied a 3-pose rotation cycle to every hub dinosaur's mesh component:
   - **Grazing**: pitch -22° (head/body lowered, as if feeding)
   - **Alert**: pitch +8°, yaw +15° (head raised and turned, scanning)
   - **Mid-stride**: roll +6° (weight-shift lean, breaks bind-pose symmetry)
   This directly answers the anti-T-pose mandate — every hub dino now has a distinct, non-neutral silhouette instead of a flat default pose.
3. **Tagging pass** (call 33121): Added actor tags (`Pose_Grazing`, `Pose_Alert`, `Pose_MidStride`) to each posed dinosaur so **NPC Behavior Agent #11** and **Combat AI Agent #12** can read current pose state without re-deriving it, and so future animation passes know which actors already have a manual pose applied (avoid double-rotating). Level saved after both mutation passes.

## Decisions & Justification
- Chose mesh-component-relative rotation over actor-root rotation to preserve existing placement/collision/navmesh data set up by World Generator (#05) and Character Artist (#09).
- Used a 3-pose deterministic cycle (not random) so poses are reproducible and documented for QA (#18) and Combat AI (#12) to reason about.
- Did not touch `TranspersonalCharacter` — it has no SkeletalMeshComponent, so idle/walk/run/jump AnimInstance work (my directive's stated focus) is **not implementable yet**; there is nothing to key. Fabricating a fake AnimBlueprint against a StaticMeshComponent would silently fail validation (Remote Control would find no AnimInstance to call).

## Hard Blocker for Real Animation Work
To deliver actual Motion Matching, blend spaces, IK Rigs, and AnimBlueprint state machines (idle/walk/run/jump) next cycle, the pipeline needs, in priority order:
1. A rigged skeletal mesh for `TranspersonalCharacter` (MetaHuman export or a humanoid rigged FBX import) with a UE5 Mannequin-compatible skeleton.
2. Rigged skeletal meshes for at least TRex/Raptor/Triceratops (Meshy rigged export or Mixamo-style dino rig import).
3. Once skeletal assets exist: I will create AnimBlueprints with a state machine (Idle/Walk/Run/Jump blend space driven by `Speed` and `bIsInAir` from `UCharacterMovementComponent`), plus a Foot IK node driven by terrain traces.

## Next Agent Focus (#11 NPC Behavior Agent)
- Hub dinosaurs now carry `Pose_Grazing` / `Pose_Alert` / `Pose_MidStride` tags — use these as initial behavior-state hints (e.g., grazing = passive/feeding state, alert = has noticed player, mid-stride = patrolling) rather than re-deriving pose from scratch.
- The skeletal-mesh/AnimBlueprint gap is now blocking BOTH agent #10 and will block any Behavior Tree that needs to trigger animation notifies — flag this dependency upward if Behavior Trees need animation-driven events.

## Files Written
- `Docs/Systems/AnimationAgent_Cycle_20260713_003.md` (this file) — only file written, zero .cpp/.h.
