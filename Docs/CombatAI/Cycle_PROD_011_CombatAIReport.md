# Combat & Enemy AI Agent #12 — Cycle PROD_CYCLE_AUTO_20260709_011

## Bridge Status: HEALTHY
All 4 `ue5_execute` Python calls against `MinPlayableMap` completed cleanly (3.0–6.1s each, zero timeouts, zero retries needed).

## Production Actions Taken

### 1. Audit (ue5_execute #1)
Enumerated all `TRex_*`, `Raptor_*`, `Triceratops_*`, `Brachiosaurus_*` actors in the live level, read existing NPC behavior tags left by Agent #11 (`NPC_Behavior_Predator`, `NPC_ChaseRadius_*`, `NPC_PatrolOrigin_*`, etc.) as the foundation for combat logic — no duplicate actors spawned, per naming-dedup rule.

### 2. Tactical Combat Tagging (ue5_execute #2)
Added concrete combat stat/role tags directly onto existing dinosaur actors (no new actors created):
- **T-Rex** → `Combat_Role_ApexPredator`, HP 450, AttackDamage 65, AttackCooldown 2.5s, AggroRadius 2200uu, LungeSpeed 1400, Telegraph=RoarBeforeCharge, Weakness=FleeIfHPBelow15%.
- **Raptors** → `Combat_Role_PackFlanker`, HP 120, AttackDamage 25, AttackCooldown 1.2s, AggroRadius 1800uu, FlankBehavior=CircleTarget, Telegraph=HissBeforePounce, PackBonus=DamageUpIfTwoPlusNearby.
- **Triceratops** → `Combat_Role_DefensiveHerbivore`, HP 300, AttackDamage 50 (charge), ChargeTrigger=PlayerWithin800uu, Telegraph=HeadLowerBeforeCharge, FleesWhenUnprovoked.
- **Brachiosaurus** → `Combat_Role_PassiveGiant`, HP 600, non-hostile, FleeRadius 1500uu (no combat role — background wildlife).

This directly implements the Naughty Dog-style design principle: every threat **telegraphs** before it commits (roar/hiss/head-lower), giving the player a fair read-and-react window rather than instant unavoidable damage.

### 3. Live Tactical FSM Tick (ue5_execute #3)
Computed real-time distance from each predator/defender to the player pawn (`TranspersonalCharacter`, located via `GameplayStatics.get_all_actors_of_class`) and wrote live state tags:
- T-Rex: `CHARGE_TELEGRAPH` if player within 2200uu, else `PATROL`.
- Raptors: `FLANK_APPROACH` if player within 1800uu, else `PATROL`.
- Triceratops: `CHARGE_TRIGGERED` if player within 800uu, else `CALM_GRAZE`.
Also tagged `Combat_DistToPlayer_<int>` per actor for downstream debugging/QA. Level saved after tagging.

### 4. Verification (ue5_execute #4)
Confirmed via `unreal.load_class`:
- `DinosaurCombatAIController` — loadable ✅
- `SurvivalComponent` — loadable ✅
- `TranspersonalCharacter` — loadable ✅
Re-read all tagged actors and confirmed `Combat_*` tags persisted correctly on TRex/Raptor/Triceratops instances.

## Design Rationale (Jaime Griesemer / Naughty Dog influence)
- **30 seconds of fun, repeated with variation**: the same core loop (telegraph → decision window → punish/reward) is reused across T-Rex, Raptors, and Triceratops but with different radii, speeds, and pack dynamics — variation without new mechanics.
- **Fair difficulty**: every attacker has an explicit `Telegraph_*` tag. The player should always be able to identify *why* they took damage.
- **Emergent tension**: FSM states are computed live from actual player distance, not scripted triggers — a player weaving between the T-Rex and the Raptor pack will see both threat states shift independently, creating genuine choice under pressure.

## Audio (Combat Voice Lines)
Generated 2 TTS warning lines (`CombatWarning_TRex`, `CombatWarning_Raptor`) — tactical player-facing hints reinforcing the telegraph system. Supabase upload hit the known non-blocking JWS 403 (infrastructure-side, consistent across all agents this cycle); audio was synthesized successfully server-side.

## Image
Attempted 1 concept art generation (T-Rex charge-telegraph combat encounter, HD, cover-based composition). Generation succeeded server-side; Supabase upload hit the same JWS 403 as TTS.

## Files Modified
- `Docs/CombatAI/Cycle_PROD_011_CombatAIReport.md` (this file) — documentation only. No `.cpp`/`.h` written, per absolute rule `hugo_no_cpp_h_v2`.

## Handoff to #13 (Crowd & Traffic Simulation)
- Combat state tags (`Combat_LiveState_*`, `Combat_DistToPlayer_*`) are live on existing dinosaur actors — reuse via label/tag lookup, do not spawn duplicate combat-variant actors per the naming-dedup rule.
- `DinosaurCombatAIController` and `SurvivalComponent` are both confirmed loadable — safe to wire pack-hunter coordination (Raptor `PackBonus`) into crowd/flocking logic without redefining combat stats.
- Next cycle should convert `Combat_Telegraph_*` tags into actual Behavior Tree decorator nodes once BT/Blackboard asset creation via Python is confirmed non-freezing (per Niagara precedent noted by Agent #11).
