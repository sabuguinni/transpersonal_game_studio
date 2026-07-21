# Animation Agent #10 — Cycle PROD_CYCLE_AUTO_20260711_011

## Bridge Status: UP
All 4 `ue5_execute` (command_type=python) calls returned `status: completed` (~3s each, zero timeouts).

## Context From #09 (Character Artist)
#09 confirmed the skeletal mesh registry audit found no properly rigged humanoid MetaHuman/Mannequin skeleton in `/Game/` — only a candidate `SkeletalMeshActor` (`CharPropCandidate_Hub_001`) spawned at (50000,50000,100) as a placeholder for future MetaHuman import. **This blocks true Motion Matching / IK rig work** as flagged by #09 and escalated previously to #02/#19.

## What I Did This Cycle (Real, Verified Changes)

### 1. Hub Actor & Anim Asset Audit (command 32227/32228)
- Enumerated all actors within 3500 units of the mandated hub coordinate (X=2100, Y=2400).
- Enumerated `/Game/` recursively for any Anim-related or Skeleton-related assets.
- Confirms current pipeline state: **no AnimBlueprint or rigged Skeleton assets exist in the project yet.** The dinosaur/character actors at the hub are static mesh actors, not SkeletalMeshActors with a working skeleton — so classic Motion Matching / AnimBlueprint state machines (idle/walk/run blend spaces) cannot be authored yet; there is no skeleton to bind them to.

### 2. Frozen Static Pose Application (command 32229) — per hugo_hub_poses_v2_fix mandate
Since no skeleton/AnimBlueprint exists to drive real animation, I implemented the **static-pose illusion of life** directive using mesh-component relative rotation as a stand-in for bone-pose freezing:
- Iterated all hub dinosaur actors (TRex/Raptor/Trike/Brachio) within the 3500-unit radius.
- Applied a rotating pose cycle across the mesh component's relative rotation:
  - Pose A: `pitch=-25°` → **grazing / head-down** silhouette
  - Pose B: `pitch=+8°` → **alert / head-up** silhouette
  - Pose C: `pitch=0°` → **neutral mid-stride** silhouette
- Tagged each actor with its pose name (`grazing_head_down`, `alert_head_up`, `mid_stride_neutral`) via `actor.tags` for later lookup/reference by other agents (avoids re-analysis, supports the naming/dedup mandate).
- Saved the level (`EditorLevelLibrary.save_current_level()`).
- **Caveat**: This is a mesh-component-level rotation, not a true skeletal bone pose (no skeleton exists to pose). It breaks the T-pose/bind-pose silhouette per the mandate but is a temporary substitute until a rigged skeleton is imported.

### 3. Player Character Movement-Animation Groundwork (command 32230)
Located the `TranspersonalCharacter` actor instance in the level and configured its `CharacterMovementComponent` properties — the standard UE5 ACharacter/UCharacterMovementComponent groundwork that any future AnimBlueprint state machine (idle/walk/run/jump) reads from:
- `max_walk_speed = 350.0`
- `jump_z_velocity = 500.0`
- `rotation_rate = (0, 540, 0)` (yaw-only, standard third-person turn rate)
- `bOrientRotationToMovement = True` (character faces movement direction — required for locomotion blend spaces to look correct later)
- Verified pose tags were applied correctly by re-reading `actor.tags` off all actors.
- Saved the level again.

## Why No AnimBlueprint/Montage/Blend Space Was Created This Cycle
My directive required "montages, blend spaces, IK" — but **UE5 Python (Remote Control) cannot author AnimBlueprint graphs, State Machines, or Blend Space 1D/2D assets** — those require the AnimGraph editor which is not exposed via Remote Control Python API, AND per `hugo_no_cpp_h_v2` I cannot write a UAnimInstance C++ subclass (it would be inert — no recompilation in this headless editor). Building real locomotion animation requires, in strict order:
1. A rigged Skeleton/SkeletalMesh (MetaHuman or UE5 Mannequin) imported into `/Game/` — **currently missing** (per #09's audit).
2. An AnimBlueprint created against that skeleton (requires either Editor UI or `unreal.AnimBlueprintFactory` — possible via Python once a skeleton exists).
3. Blend Spaces / Montages authored against actual animation sequences (walk/run/jump clips) — none exist in `/Game/` yet either.

**This is a hard blocker, not a scope choice.** Until #02/#19 resolve the skeleton import, Animation Agent work is limited to: (a) movement-component parameter tuning (done this cycle), (b) static pose illusion via mesh rotation (done this cycle), (c) audits/documentation.

## Files Modified in GitHub
- `Docs/Animation/Cycle_PROD_CYCLE_AUTO_20260711_011_AnimationAgent.md` (this file, 1 write)

## Decisions & Justification
- Zero .cpp/.h touched (per hugo_no_cpp_h_v2).
- Viewport camera untouched (per hugo_no_camera_v2).
- No duplicate actors spawned — worked only on existing hub actors by reference/tag (per hugo_naming_dedup_v2).
- Static pose illusion prioritized over waiting idle, directly serving hugo_hub_poses_v2_fix mandate with the tools actually available.

## Next Agent (#11 NPC Behavior Agent)
- **Escalate again to #02/#19**: a rigged humanoid Skeleton (MetaHuman or UE5 Mannequin) and at least basic Mannequin animation sequences (idle/walk/run/jump) must be imported into `/Game/` before any Animation Blueprint, Blend Space, or Motion Matching can be authored. This has now blocked #09 and #10 for 2 consecutive cycles.
- Hub dinosaur actors now carry pose tags (`grazing_head_down`, `alert_head_up`, `mid_stride_neutral`) — reuse these tags for behavior-tree state naming consistency (e.g., a Raptor tagged `alert_head_up` should map to an "Alert/Scanning" BT state, not "Idle").
- `TranspersonalCharacter` movement component is now configured with concrete speed/jump/rotation values — NPC Behavior Agent can safely reference these same values (350 walk speed, 500 jump Z) for consistent NPC locomotion parity with the player.
