# T-Rex Behavior Design — Agent #11 (NPC/Dinosaur Behavior)
Cycle: PROD_CYCLE_AUTO_20260720_006

## Status Check (verified live in UE5, not assumed)

1. **DinosaurCombatAIController.cpp** — checked via `github_file_read`. File exists at
   `Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp` but content is empty/placeholder
   (9 bytes, "undefined"). No functional AI logic is present in this file.
2. **SurvivalComponent.h** — confirmed present and fully implemented at
   `Source/TranspersonalGame/Core/Survival/SurvivalComponent.h` (Health, Hunger, Thirst, Stamina, Fear,
   Temperature, biome integration, delegates, full API). Owned by Core Systems Programmer #03.

## IMPORTANT — No new .cpp/.h created this cycle

Per ABSOLUTE RULE `hugo_no_cpp_h_v2` (imp:20, no exceptions): this headless UE5 editor never
recompiles new C++ (pre-built binary, 218 UHT errors on record). Writing `TRexBehavior.cpp` would be
100% wasted effort with zero effect on the live game. The task instruction to "CREATE TRexBehavior.cpp"
is superseded by this rule. Instead, the T-Rex behavior spec below has been implemented **directly in
the live world** via `ue5_execute` (data-driven, tag-based), and documented here for whoever eventually
builds a real Behavior Tree / Blackboard around it (Combat & Enemy AI Agent #12).

## T-Rex Behavior Spec (data now live on actors, verifiable in-world)

Applied to 40 `TRex_Savana_*` actors in the playable core (hub at 2100,2400) via live UE5 Python:

| Behavior | Radius | Actor Tag |
|---|---|---|
| Patrol | 5000 units around spawn point | `Behavior_Patrol_5000` |
| Chase player | Triggers within 3000 units | `Behavior_Chase_3000` |
| Attack player | Triggers within 300 units | `Behavior_Attack_300` |

These tags are now readable by any Behavior Tree / Blackboard the Combat AI Agent (#12) builds on top —
no C++ required, purely actor tag + Blueprint/BT decorator driven.

## Live Verification (this cycle, real ue5_execute calls)

- Bridge check: `MinPlayableMap` loaded, confirmed live.
- Found **101 actors** with "TRex" in label; **40** are the tagged `TRex_Savana_*_Posed` instances that
  received the 3 behavior tags above.
- Ground-check via line trace to Landscape at each TRex's XY: **0 of 40 needed correction** — all 40 were
  already within 15 units of terrain surface (grounded, consistent with Environment/Animation agents'
  prior passes).
- `CombatZone_TRex_Hub` anchor actor confirmed present at (2000, 2500, 120) — reused, not duplicated.
- Level saved once, at end of turn, after verification (per Definition of Done rule #5).

## Handoff to Combat & Enemy AI Agent (#12)

- `DinosaurCombatAIController.cpp` is an empty placeholder — needs real Behavior Tree / EQS asset work
  (Blueprint-side, not C++, since C++ won't compile here).
- The 3 tags (`Behavior_Patrol_5000`, `Behavior_Chase_3000`, `Behavior_Attack_300`) are live on 40 TRex
  actors and ready to be read by BT decorators/services.
- `SurvivalComponent` (player-side) is fully implemented and exposes `AddFear`/`ReduceFear` — Combat AI
  should call into this when T-Rex threatens the player, to drive the Fear stat shown in HUD.
- No new actors were spawned this cycle (naming/dedup rule respected — reused existing `TRex_Savana_*`
  and `CombatZone_TRex_Hub`).
