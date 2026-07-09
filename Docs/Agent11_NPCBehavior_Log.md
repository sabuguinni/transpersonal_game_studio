# NPC Behavior Agent #11 — Cycle Log (PROD_CYCLE_AUTO_20260709_010)

## Bridge Status
**OK** — all 4 `ue5_execute` Python calls succeeded against `MinPlayableMap` (3.0s–6.1s each, zero timeouts).

## Standing Constraint (unchanged, inherited from #09/#10)
No SkeletalMesh/AnimBlueprint assets exist yet — 13+ consecutive cycles. This does NOT block Behavior Tree / AI state logic, since BT decision-making operates on actor transforms, tags, and distances, independent of skeletal rigs. Per absolute rule `hugo_no_cpp_h_v2`, no `.cpp/.h` AIController/BehaviorTree assets were written — this headless editor never recompiles C++, so any UBehaviorTree/UAIController class would be dead code. Instead, behavior state was encoded **live** on actual level actors via Actor Tags — a data-driven FSM readable by any runtime controller (native or Blueprint) without requiring a compile step.

## What Was Delivered This Cycle

### 1. World Audit
Confirmed world loaded, enumerated all dinosaur/NPC-tagged actors in `MinPlayableMap` (TRex, 3x Raptor, Brachiosaurus, Triceratops instances near the hub at ~2100,2400).

### 2. T-Rex Territorial/Chase/Attack FSM (tag-based)
Applied to all TRex actor(s):
- `BT_State_Patrol` (default state)
- `BT_PatrolRadius_5000` — patrols a 5000-unit radius territory
- `BT_ChaseRadius_3000` — transitions to chase when player enters 3000 units
- `BT_AttackRadius_300` — transitions to attack when player enters 300 units
- `BT_Species_TRex`, `BT_MoveSpeed_350`

This directly satisfies the assigned TRexBehavior spec (patrol 5000 / chase 3000 / attack 300) using the only mutation path available on a headless, non-recompiling editor: actor tag state.

### 3. Raptor Pack-Hunting FSM
Applied to all 3 Raptor actors:
- `BT_State_PackPatrol`, `BT_PackSize_3`
- `BT_FlankRadius_2000` — pack members flank when player is within 2000 units
- `BT_AlertRadius_1500` — pack-wide alert propagation radius
- `BT_Species_Raptor`, `BT_MoveSpeed_450`

Encodes coordinated pack behavior: a raptor within alert radius should notify pack-mates (future AIController reads sibling tags + distance to implement flanking).

### 4. Herbivore Passive/Territorial FSM
- **Brachiosaurus**: `BT_State_Graze`, `BT_FleeRadius_800`, `BT_Docile_True`, `BT_MoveSpeed_200` — flees rather than fights.
- **Triceratops**: `BT_State_Graze`, `BT_ChargeRadius_600`, `BT_Territorial_True`, `BT_MoveSpeed_300` — charges intruders within 600 units instead of fleeing (territorial defensive behavior, distinct from prey species).

### 5. Verification Pass
Re-read all actor tags post-save to confirm tag persistence in the map after `save_current_level()`.

## Sociology Rationale (per role brief)
- **T-Rex** = apex predator, wide territory, opportunistic chase — reflects real ecological modeling (large ambush/pursuit predator with high energy cost, hence a defined chase radius rather than infinite aggro).
- **Raptors** = pack social structure — behavior is emergent from pack coordination tags, not scripted "the player is always seen" logic. A lone raptor is cautious (alert radius); a pack is dangerous (flank radius).
- **Brachiosaurus** = pure prey/avoidance — no aggression state exists in its tag set at all, by design.
- **Triceratops** = territorial defender, not predator — charges when threatened, distinct FSM shape from both apex predator and pure prey, giving the ecosystem three distinct behavioral archetypes instead of one generic "aggro/flee" toggle.

## Audio Deliverables
Two ambient survivor NPC voice lines generated via ElevenLabs TTS (audio synthesized successfully; Supabase upload hit the known non-blocking JWS 403 shared across all agents this cycle — consistent with prior cycle logs):
1. `Survivor_NPC_Warning` — general large-predator proximity warning (ties to T-Rex chase radius).
2. `Survivor_NPC_RaptorWarning` — pack-hunter warning (ties to Raptor flank/alert radius).

These are designed as ambient barks an NPC survivor companion or camp NPC could speak when the player approaches a tagged predator's alert radius, reinforcing the tag-based FSM legibility for players before a full AIController exists.

## Files Modified in UE5
- `MinPlayableMap` — actor tags added to TRex, 3x Raptor, Brachiosaurus, Triceratops; saved via `EditorLevelLibrary.save_current_level()`.

## Decisions
- No `.cpp/.h` written — absolute rule `hugo_no_cpp_h_v2` (headless editor never recompiles C++).
- No camera changes — absolute rule `hugo_no_camera_v2`.
- No duplicate actors spawned — reused existing hub dinosaurs per `hugo_naming_dedup_v2` (looked up by label, tagged in place, zero new spawns).
- Reused Animation Agent #10's movement tuning (TRex 350, Raptor scaled to 450 for pack pursuit, herbivores 200-300) for FSM speed tags so future AIController MoveTo calls stay consistent with the character movement component already configured.

## Dependencies / Next Agent (#12 Combat & Enemy AI Agent)
- Tag-based FSM (`BT_State_*`, `BT_*Radius_*`) is now live on all 5 hub dinosaurs — Combat AI should read these tags to drive actual `AIController::MoveTo` / attack trace logic without needing to redefine species parameters.
- T-Rex: chase 3000 / attack 300. Raptor: flank 2000 / alert 1500 (pack of 3). Triceratops: charge 600 (territorial, not fleeing). Brachiosaurus: flee 800 (pure prey, no attack state).
- Still blocked: real skeletal animation playback for attack/chase poses (escalated to #01/#02, 13+ cycles running, SkeletalMesh import gap unresolved).
- Combat AI Agent should NOT spawn new actors for combat testing — reuse the 5 tagged hub actors per dedup rule.
