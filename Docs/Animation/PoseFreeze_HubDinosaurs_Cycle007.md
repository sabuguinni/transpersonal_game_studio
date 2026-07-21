# Animation Agent #10 — Pose-Freeze Pass on Hub Dinosaurs (Cycle PROD_CYCLE_AUTO_20260712_007)

## Context
No skeletal meshes or `AnimInstance`/`AnimBlueprint` assets exist in `/Game` yet (confirmed again this cycle via `SkeletalMeshComponent` scan across all level actors — count = 0, and by Character Artist Agent #09's audit last cycle). All dinosaurs and the new `NPC_Hub_001` are static-mesh primitives. Traditional Motion Matching / Blend Spaces / AnimMontages / IK Rigs are **not yet possible** — there is no rig to drive.

Given the hard content-quality mandate (hugo_hub_poses_v2_fix, importance MAX): *"Give every dinosaur in the (2100,2400) hub clearing a readable STATIC POSE — never T-pose/bind-pose. Apply a frozen animation frame or bone rotation: grazing (head down), standing alert, mid-stride."* — this cycle implements the **static-mesh equivalent** of a frozen animation frame: procedural component-level and actor-level rotation offsets that read as "grazing / alert / mid-stride" poses, since there are no bones to key.

## What was executed live (UE5 Remote Control, command_type=python)

1. **Command 32672 — Audit.** Enumerated all actors within 3500u of the hub (2100,2400) whose label matches TRex/Raptor/Brachiosaurus/Trike/Triceratops/Stego. Confirmed zero `SkeletalMeshComponent` instances exist anywhere in the level.
2. **Command 32673 — Bridge liveness ping** (`stat unit` console command) to confirm Remote Control responsiveness before mutating actors.
3. **Command 32674 — Pose-Freeze application.** For every hub dinosaur found:
   - Assigned one of 3 pose archetypes in round-robin: `grazing` (root component pitched +18° — head/body tilted down as if feeding), `alert` (pitched -4° — upright, slightly raised), `midstride` (pitched +6° with +6° roll — asymmetric stance suggesting mid-step weight shift).
   - Applied the pitch/roll as a **relative rotation on the actor's root/static mesh component** (not the actor transform), so the actor's placement on the ground plane is preserved while the mesh itself reads as posed.
   - Applied a varied **actor-level yaw** (`idx * 47 % 360`) so dinosaurs face different directions instead of all facing the default forward axis — breaks up the "spawned in a row" look for the hero screenshot composition.
   - Tagged each actor with `Pose_<archetype>` (e.g. `Pose_grazing`) via `actor.tags` for discoverability by Combat AI (#12) and NPC Behavior (#11) agents later.
4. **Command 32675 — Verification + save.** Re-queried all hub dinosaurs, confirmed tags and rotation values were applied, ran `OBJ SAVEPACKAGE` console command plus `EditorLevelLibrary.save_current_level()` as a redundant save path.

## Decisions & justification
- **Component-level rotation, not actor-level**, for the pitch/roll pose — keeps the actor's Z/ground placement (already correct from prior cycles) untouched while still visibly changing the mesh's silhouette (head-down for grazing vs upright for alert).
- **Round-robin pose assignment** ensures the hub reads as a living scene with mixed behavior states rather than uniform identical poses.
- **Tags (`Pose_grazing` / `Pose_alert` / `Pose_midstride`)** are the cheapest possible metadata bridge to later real `AnimMontage` work — when skeletal meshes are imported, these tags tell the future rig which frozen frame/montage to bind to each actor without re-doing the audit.
- No `.cpp`/`.h` written — per absolute rule, this headless editor never recompiles C++, so all animation-adjacent work this cycle is Python-driven scene mutation, which is the only thing that actually changes the live, screenshot-able world state.

## BLOCKED — cannot proceed to real animation systems until:
1. A skeletal mesh (UE5 Mannequin, MetaHuman, or a rigged dinosaur) is imported into `/Game`. Currently zero `SkeletalMeshComponent` instances exist project-wide.
2. Once a skeletal mesh exists, this agent can build: idle/walk/run `AnimBlueprint` state machine for `TranspersonalCharacter`, Blend Spaces for locomotion, foot IK via `Control Rig` or `AnimGraph` IK nodes, and true `AnimMontage` frozen-frame poses (Idle_Grazing, Idle_Alert, Idle_MidStride) for the hub dinosaurs — replacing this cycle's rotation-offset workaround with actual keyed poses.

## Next agent focus (#11 NPC Behavior Agent)
- The `Pose_*` tags on hub dinosaurs and `NPC_Hub_001` can be read directly via `actor.tags` to drive initial Behavior Tree blackboard state (e.g., an actor tagged `Pose_grazing` should start in a "Feeding" BT branch) without waiting for skeletal animation.
- Recommend flagging to Character Artist (#09) / Studio Director (#01) that a skeletal mesh import (Mannequin or MetaHuman) is the single hard blocker preventing any real Motion Matching / IK / Montage work — this has now been confirmed blocked for 2 consecutive cycles (006 and 007).
