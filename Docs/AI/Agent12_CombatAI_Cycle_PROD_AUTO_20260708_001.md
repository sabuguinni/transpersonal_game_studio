# Combat & Enemy AI Agent #12 — Cycle PROD_CYCLE_AUTO_20260708_001

## Bridge Status
HEALTHY — 4/4 `ue5_execute` calls succeeded this cycle (bridge validation, tagging, marker spawn, verification). Following the atomic validate→act→verify→save workflow per standing operational guidance.

## Standing Infra Constraint (Honored)
Per `hugo_no_cpp_h_v2` (importance MAX): C++ is inert in this headless editor (218 UHT errors on record, binary never recompiles). Did **not** write to `DinosaurCombatAIController.cpp` or create new `.h/.cpp` files. #11 confirmed this file is a dead 9-byte stub (`undefined`). All combat AI logic this cycle was implemented as **live, queryable actor state** (tags) directly on the running dinosaur actors in `MinPlayableMap`, plus spawned encounter-trigger markers — both inspectable and buildable-upon by Blueprint Behavior Trees without any C++ recompilation.

## What Was Built

### 1. Combat Behavior Contracts (tags on 5 dinosaur actors)
Extended #11's `Behavior_<State>` tagging system with a full **combat layer** (`Combat_*` tags), giving each species a distinct tactical identity:

| Species | combat_state | detect_radius | attack_radius | retreat_hp_pct | flank_style | commit_style | bite_damage | cooldown |
|---|---|---|---|---|---|---|---|---|
| T-Rex | Apex_Ambush | 3000u | 350u | 0% (never retreats) | None | FullCommit | 85 | 2.5s |
| Raptor x3 | Pack_Flank | 2200u | 220u | 25% | Coordinated | WaitForPack | 25 | 1.2s |
| Triceratops | Defensive_Charge | 1200u | 400u | 0% | None | ChargeOnly | 60 | 3.0s |
| Brachiosaurus | NonCombatant | 800u (flee trigger only) | 0 | 100% | None | FleeOnly | 0 | — |

**Design intent (Naughty Dog / Griesemer influence):**
- T-Rex never retreats — a fight with it is binary: win or lose. High damage, long cooldown = punishes greedy attacks, rewards spacing.
- Raptors use `WaitForPack` commit style — a lone raptor will NOT engage until at least one other pack member is in range, replicating pack-hunting caution. This is the "conversation, not equation" design: player who isolates one raptor gets a much easier fight than one who triggers the whole pack.
- Triceratops is a pure telegraphed threat: long detection range but only charges (no pursuit), teaching players that avoidance beats confrontation for non-predators.
- Brachiosaurus is confirmed non-combatant — no encounter should ever force a fight with it end of chain check for #13 crowd/traffic (should path around it, not through combat logic).

### 2. Encounter Zone Markers (3 spawned Note actors, saved to level)
- `AmbushZone_Raptor_001` @ (2600, 2200, 120) — pack convergence trigger, ties directly to Combat_state=Pack_Flank.
- `AmbushZone_TRex_001` @ (1500, 3000, 120) — solo apex encounter, no retreat clause.
- `DefenseZone_Trike_001` @ (2000, 1800, 120) — proximity-triggered charge only.

These are placed in-world (not off in the void) and queryable by label for the next agent building actual Blueprint Behavior Tree triggers.

## Verification
Ran a post-write query across all level actors confirming: (a) tag application persisted after `save_current_level()`, (b) all 3 marker actors exist with correct labels. Both prior `ue5_execute` save calls returned `ReturnValue: true`.

## Blockers / Escalation to #01
Same blocker #11 raised: `DinosaurCombatAIController.cpp` cannot be made functional in this headless setup since C++ never recompiles. Real chase/attack state-machine logic needs to be authored as a **Blueprint Behavior Tree + Blackboard** (via `unreal.AssetToolsHelpers` / `unreal.BehaviorTreeFactory` in Python) reading the `Combat_*` tags set this cycle. Requesting confirmation this is in scope for #12/#13, since it's a deviation from the original C++-first architecture mandate.

## Asset Generation Notes
- Concept art generation (raptor pack ambush scene) succeeded on the model side but **image upload to Supabase failed** (`403 Invalid Compact JWS` — same infra issue #11 reported for audio). Prompt is preserved below for regeneration once storage auth is fixed:
  > "Realistic documentary-style concept art of a Velociraptor pack ambushing a lone hunter in a dense Cretaceous fern forest, dynamic action pose, one raptor lunging mid-air with jaws open, muscular textured skin, natural earth-tone feathers, dappled sunlight through canopy, National Geographic wildlife photography style, no fantasy elements, no magic, photorealistic, high detail"
- 2x TTS combat-alert voice lines generated successfully (raw audio returned as base64, Supabase upload failed same auth issue). Lines: raptor-pack-flanking alert, T-Rex-disengage-and-freeze guidance. Both reinforce the "read the encounter, don't just react" design philosophy.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Bridge health validation
- [UE5_CMD] Applied `Combat_*` tag contracts (detect/attack radius, damage, cooldown, flank/commit/retreat style) to 5 dinosaur actors in MinPlayableMap
- [UE5_CMD] Spawned 3 combat encounter zone markers (AmbushZone_Raptor_001, AmbushZone_TRex_001, DefenseZone_Trike_001), level saved
- [UE5_CMD] Verification query confirming tags + markers persisted after save
- [TTS] Raptor pack flanking combat alert (audio generated, storage upload pending infra fix)
- [TTS] T-Rex disengage/freeze guidance line (audio generated, storage upload pending infra fix)
- [IMG] Raptor pack ambush concept art prompt (generation succeeded, storage upload pending infra fix — prompt preserved in this doc for retry)
- [FILE] Docs/AI/Agent12_CombatAI_Cycle_PROD_AUTO_20260708_001.md
- [NEXT] #13 Crowd & Traffic Simulation: Brachiosaurus is confirmed `NonCombatant`/`FleeOnly` — pathing systems should route crowd agents around it without invoking combat logic. Raptor pack `WaitForPack` commit style means crowd/NPC flee routines should check pack proximity before triggering panic state. Next agent building real BT logic should read `Combat_*` tags via `actor.tags` lookup rather than hardcoding values.
