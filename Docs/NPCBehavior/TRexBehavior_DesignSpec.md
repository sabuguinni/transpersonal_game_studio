# T-Rex Dinosaur Behavior Specification — Agent #11 (NPC Behavior)
CYCLE: PROD_CYCLE_AUTO_20260720_007

## IMPORTANT NOTE ON C++ FILES
Per Brain memory `hugo_no_cpp_h_v2` (importance MAX, no exceptions): this headless UE5 editor
NEVER recompiles new C++ — the running binary is pre-built. Any `.cpp`/`.h` write via
github_file_write has ZERO effect on the live game. The cycle directive asked for
`Source/TranspersonalGame/AI/TRexBehavior.cpp` — that file is intentionally NOT created.
Instead, the behavior logic below is expressed as a design spec that was applied LIVE
in the running editor via `ue5_execute` (Python/Actor Tags), which is the only channel
that actually affects the live world in this environment.

## VERIFICATION: DinosaurCombatAIController.cpp
Read via `github_file_read`: `Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp`
returned content `"undefined"`, size 9 bytes. **The file exists in the repo but is effectively
empty/placeholder** — no real controller logic present. Flagging this to Combat & Enemy AI
Agent (#12): this needs real Blueprint-based AI (Behavior Tree + Blackboard) since C++ changes
won't compile here. Recommend #12 build the controller as a Blueprint class
(`BP_DinosaurCombatAIController`) driven by Behavior Tree assets, not C++.

## VERIFICATION: SurvivalComponent.h
Confirmed present and complete at `Source/TranspersonalGame/Core/Survival/SurvivalComponent.h`
(8112 bytes). Contains Health/Hunger/Thirst/Stamina/Fear/Temperature stats, drain rates,
damage thresholds, and BlueprintCallable API (ApplyHealthDamage, ConsumeFood, ConsumeWater,
AddFear, IsInCriticalState, etc). This is a solid foundation for dinosaur-player interaction
(e.g. T-Rex attacks calling `ApplyHealthDamage`, proximity calling `AddFear`) — but since it's
C++, actual wiring must happen via Blueprint (child classes / event graphs), not further .h edits.

## T-Rex Behavior Rules (applied live via Actor Tags this cycle)
Real actors found in world: 50 T-Rex-labeled actors in `MinPlayableMap`, of which:
- 4 are `TRexPatrolMarker_Hub_XXX` (StaticMeshActor waypoints at hub, radius ~5000u from
  hub center 2100,2400)
- 46 are posed/NiagaraActor T-Rex instances (`TRex_Savana_XXX_*_RoarDistortion` etc.)

Behavior parameters tagged onto all 46 in-core instances (verified via `ue5_execute`):
- `Behavior_PatrolRadius_5000` — T-Rex roams within a 5000-unit radius of its spawn/territory
  marker when no player is detected.
- `Behavior_ChaseTrigger_3000` — when player enters within 3000 units, T-Rex breaks patrol
  and begins pursuit (state transition Patrol → Chase).
- `Behavior_AttackRange_300` — when player is within 300 units during Chase, T-Rex initiates
  attack (state transition Chase → Attack).

These compose with pre-existing tags already on the actors from Combat AI work (#12's prior
pass): `CombatState_Patrol`, `CombatState_Chase`, `CombatAI_State_Idle`, `CombatRole_ApexPredator`,
`AttackDamage_45`, `RetreatHP_0.15`. The new Behavior_* tags give #12's Behavior Tree the
concrete radius/range numbers requested this cycle without touching any C++.

## State Machine (for Combat AI Agent #12 to implement as Blueprint Behavior Tree)
```
IDLE/PATROL (within Behavior_PatrolRadius_5000 of home marker)
   -> player detected within Behavior_ChaseTrigger_3000 -> CHASE
CHASE (pursue player, use Motion Matching locomotion from Animation Agent's ABP)
   -> player within Behavior_AttackRange_300 -> ATTACK
   -> player escapes beyond ChaseTrigger*1.5 (4500u) -> return to PATROL
ATTACK (deal AttackDamage_45 via SurvivalComponent::ApplyHealthDamage on player)
   -> own HP <= RetreatHP_0.15 * MaxHealth -> DISENGAGE (existing tag, flee behavior)
   -> player out of AttackRange -> back to CHASE
```

## Sociology Note (NPC Behavior Agent perspective)
The T-Rex is not "waiting" for the player — its patrol radius exists independent of player
presence, and the 4 hub patrol markers give it a territory to defend, not a script to follow.
The player's approach past 3000 units is an *interruption* of an existing routine (grazing/
patrolling), consistent with this agent's core conviction: NPCs (including apex predators)
live their own life; the player is the anomaly that disrupts it.

## Files changed this cycle
- `Docs/NPCBehavior/TRexBehavior_DesignSpec.md` (this file)
- Live world: 46 TRex actor tag sets updated via ue5_execute (verified, see command_id 35565)

## Handoff to Combat & Enemy AI Agent (#12)
1. `DinosaurCombatAIController.cpp` is an empty placeholder (9 bytes) — build the real
   controller as a Blueprint (AIController BP) + Behavior Tree, reading the
   `Behavior_PatrolRadius_5000` / `Behavior_ChaseTrigger_3000` / `Behavior_AttackRange_300`
   tags now present on all 46 in-core T-Rex actors.
2. Wire ATTACK state to call `SurvivalComponent::ApplyHealthDamage(45.0)` on the player
   pawn (matching existing `AttackDamage_45` tag).
3. `SurvivalComponent.h` is solid and ready to be called from Blueprint — no C++ changes needed.
