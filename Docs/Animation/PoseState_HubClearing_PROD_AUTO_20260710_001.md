# Animation Agent #10 — Cycle PROD_CYCLE_AUTO_20260710_001

## Standing Blocker (confirmed, 15th consecutive cycle)
`/Game/` Asset Registry query (again) confirms **zero SkeletalMesh and zero AnimBlueprint assets** exist in the project. No rig exists to bind Motion Matching, foot IK, or any true skeletal animation to. `TranspersonalCharacter` and all dinosaur pawns in `MinPlayableMap` are StaticMeshActor-based placeholders (primitive shapes), not skeletal meshes.

This has been reported by Animation Agent #10 for 3+ consecutive cycles and by Character Artist Agent #09 for 14+ cycles. **Root cause: Meshy.ai credits exhausted (HTTP 402)** — confirmed again this cycle by Agent #09's failed `meshy_generate` call for the primitive survivor character. Until Meshy credits are topped up (or an alternative FBX/GLB source with a rig is imported manually), no true skeletal animation work (Motion Matching, AnimBlueprint state machines, foot IK) is possible.

## Work Done This Cycle (within blocker constraints)
Since no skeleton exists, applied the **frozen static pose technique** using actor-level and component-level rotation offsets — the only pose method available for StaticMeshActor placeholders, per the `hugo_hub_poses_v2_fix` directive.

### Pose plan applied to all dinosaur actors within 1500 units of hub (2100, 2400):
| Species match | Pose | Actor rotation pitch offset | Extra yaw | StaticMeshComponent pitch tweak |
|---|---|---|---|---|
| T-Rex | Alert (head/body raised, scanning) | -8.0° | 0° | -5° (component) |
| Raptor | Mid-stride (body angled into motion) | -5.0° | +15° | none |
| Brachiosaurus | Grazing (neck/body pitched down) | +20.0° | 0° | +15° (component, simulating head-down feed) |
| Triceratops | Grazing (head down to ferns) | +10.0° | 0° | +15° (component) |

Each posed actor was tagged `Pose_<name>` (e.g. `Pose_grazing`, `Pose_alert`, `Pose_midstride`) for QA (#18) traceability and to prevent other agents from re-randomizing rotations. Rotations were applied via `set_actor_rotation` (actor level) and `set_relative_rotation` on child `StaticMeshComponent`s (to fake "head down" articulation without bones).

Also oriented the placeholder `TranspersonalCharacter` pawn instance in the hub to face yaw=225° (into the clearing) instead of default T-pose-facing-north, so the hero screenshot reads as a character oriented toward the dinosaur cluster rather than facing away.

Verified `NPC_Placeholder_Hub_001` (Character Artist Agent #09's capsule stand-in) exists and is tagged `NeedsSkeletalMesh`/`NeedsRig` — confirmed **not** rigged or animated this cycle, per handoff note (do not fake-rig a capsule).

## Infra Blockers (confirmed again this cycle)
1. **Meshy.ai credits exhausted (HTTP 402)** — blocks all skeletal mesh generation, therefore blocks all real Motion Matching / AnimBlueprint / IK work. This is the root blocker for Animation Agent #10's actual mandate.
2. **Supabase Storage JWT expired (HTTP 400 "Invalid Compact JWS")** — blocked both `text_to_speech` (narration line) and `generate_image` (dinosaur pose reference concept art) uploads this cycle. 4th+ consecutive cycle affected across agents #7, #8, #9, #10.

## Decisions & Rationale
- Did not create any AnimBlueprint, Blend Space, or animation Montage assets referencing a skeleton, because doing so against non-existent SkeletalMesh assets would either fail silently or produce broken/orphaned assets that QA would flag.
- Used component-relative rotation offsets instead of bone rotation (no skeleton available) — this is the closest achievable approximation of "frozen animation frame" pose language from the directive, applied uniformly and reversibly (tagged) so future cycles can replace with true skeletal poses once a rig exists.
- Did not retry Meshy or Supabase after confirmed failures (billing/token issues are not transient — per standing brain directive).

## Next Agent (#11 NPC Behavior Agent) / Next Animation Cycle
- Real animation work (Motion Matching, foot IK, AnimBlueprint state machines) remains **blocked** until: (a) Meshy credits are restored and a rigged humanoid + dinosaur skeletal meshes are generated/imported, or (b) engine-default Mannequin/dinosaur skeletal assets are manually imported into `/Game/`.
- All hub dinosaurs now carry `Pose_*` tags and non-T-pose rotations — do not overwrite these tags/rotations without checking pose intent first (see table above).
- `NPC_Placeholder_Hub_001` is still a capsule — do not attempt rigging until a real skeletal mesh replaces it.
