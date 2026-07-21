# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260710_004

## Bridge Status: OK
All 4 `ue5_execute` Python calls succeeded against the live `MinPlayableMap` (54s cold start, then 3x ~3s, zero timeouts).

## Actions Taken This Cycle

1. **Bridge validation** — Confirmed `EditorLevelLibrary.get_editor_world()` returns a valid world object.
2. **T-Rex behavior tagging** — Iterated all level actors, found actors labeled with "TRex"/"Rex", and tagged them with runtime behavior-state markers:
   - `BehaviorState_Patrol`
   - `PatrolRadius_5000` (5000 unit patrol area, per directive)
   - `ChaseRadius_3000` (chase trigger distance)
   - `AttackRadius_300` (melee attack range)
3. **Class verification** — Confirmed via `unreal.load_class`:
   - `/Script/TranspersonalGame.DinosaurCombatAIController` — loadable
   - `/Script/TranspersonalGame.SurvivalComponent` — loadable
   These classes exist in the compiled binary and are usable by Behavior Tree tasks/services.
4. **Raptor pack-hunting tags** — Tagged all "Raptor" actors with pack coordination markers:
   - `BehaviorState_PackHunt`
   - `PackRadius_1500` (raptors within this radius coordinate as a pack)
   - `AlertRadius_2000` (detection range)
   - `AttackRadius_250` (melee range, smaller than T-Rex due to size)
5. **Hub clearing audit** — Scanned actors near world coords (2100, 2400) — the primary content hub / PlayerStart clearing — for dinosaur presence, logging labels and exact positions for the Lighting/Environment agents to reference when composing the hero screenshot.

## Design Rationale (Sociology of the World)

- **T-Rex**: Solitary apex predator. Patrols a large fixed territory (5000u) because real theropods of this size maintain large hunting ranges and do not pack-hunt. Chase only triggers within 3000u — it should feel like an ambush predator that doesn't waste energy chasing distant prey, consistent with megafauna metabolic constraints.
- **Raptors**: Pack hunters. Smaller individual attack/alert radii but pack coordination radius (1500u) reflects real velociraptor-family social hunting behavior — appearing in the game world as agents that alert each other and converge, not independent aggressors. This creates emergent threat: the player who sees one raptor should learn to fear the pack, not the individual.

These tags are runtime-readable by Blueprint Behavior Trees via `Actor.Tags` and can drive Blackboard key selection without requiring new C++ compilation (per the absolute rule against .cpp/.h writes in this headless environment).

## Voice Lines Generated (TTS)
1. **TribeElder_Warning** — warns the player about T-Rex tracks near camp.
2. **Scout_RaptorWarning** — warns about raptor pack-hunting behavior in the open.
Both are grounded survival-realism dialogue (no spiritual/mystical content), reinforcing the ecological threat model above.

## Files Verified (Not Modified — No C++ Writes Per Absolute Rule)
- `Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp` — exists, class loads in live binary.
- `Source/TranspersonalGame/Core/Survival/SurvivalComponent.h` — exists, class loads in live binary.

## Deliverables This Cycle
- [UE5_CMD] Tagged all TRex actors with Patrol/Chase/Attack radius behavior tags (5000/3000/300 units).
- [UE5_CMD] Tagged all Raptor actors with pack-hunt behavior tags (1500/2000/250 units).
- [UE5_CMD] Verified DinosaurCombatAIController and SurvivalComponent classes loadable via Remote Control.
- [UE5_CMD] Audited hub clearing (2100,2400) for dinosaur presence to inform hero screenshot composition.
- [TTS] 2 NPC voice lines (TribeElder_Warning, Scout_RaptorWarning) reinforcing dinosaur threat sociology.
- [NEXT] Combat & Enemy AI Agent (#12) should build actual Blueprint Behavior Tree logic reading these actor tags (BehaviorState_Patrol, BehaviorState_PackHunt) to drive Blackboard state transitions — no new C++ required, use existing DinosaurCombatAIController + Tag-based Blackboard decorators.
