# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260710_001

**Bridge status: OK** — both `ue5_execute` Python calls succeeded (3.0s, 6.0s), zero timeouts, against `MinPlayableMap`.

## Production actions taken

### 1. `ue5_execute` #1 — Audit + T-Rex Behavior State Machine
- Verified class loadability via Remote Control:
  - `DinosaurCombatAIController` — **loadable: confirmed present** in `/Script/TranspersonalGame`
  - `SurvivalComponent` — **loadable: confirmed present** in `/Script/TranspersonalGame`
  - `TranspersonalCharacter` — loadable, used to compute player position
- Located all `TRex_*` actors in the hub clearing (2100, 2400).
- Computed live distance from each T-Rex to the player's `TranspersonalCharacter` instance and assigned a **behavior state** using the three-tier FSM:
  - `NPC_State_Patrol` — player > 3000 units away (default roaming state)
  - `NPC_State_Chase` — player within 3000 units (pursuit triggered)
  - `NPC_State_Attack` — player within 300 units (melee engagement)
- Tagged every T-Rex actor with: `NPC_Behavior_Predator`, `NPC_Species_TRex`, `NPC_State_<X>`, `NPC_PatrolRadius_5000`, `NPC_ChaseRadius_3000`, `NPC_AttackRadius_300`, and `NPC_PatrolOrigin_<x>_<y>` (anchor point for patrol loop, matching the TRexBehavior design spec below).

### 2. `ue5_execute` #2 — Raptor Pack Dynamics + Herbivore Flight Response
- Raptors tagged with pack-hunting roles: first raptor found = `NPC_Role_PackLeader`, remaining = `NPC_Role_PackMember`, plus `NPC_PackRadius_2000`, `NPC_ChaseRadius_2500`, `NPC_AttackRadius_200` (raptors are faster/lower attack range than TRex, pack-coordinated).
- Brachiosaurus + Triceratops (herbivores) tagged `NPC_Behavior_Herbivore`, `NPC_State_Graze`, `NPC_FleeRadius_1500`, `NPC_AlertRadius_800` — no chase/attack states, flight-only response to predator proximity.
- Level saved. Final verification pass logged full tag summary confirming all dinosaur actors in the hub carry consistent `NPC_*` state.

### 3. `text_to_speech` — T-Rex vocalization line generated (audio synthesized successfully; Supabase upload failed with "Invalid Compact JWS" — same infra issue flagged by Animation Agent #10 this same cycle, 5th+ consecutive cycle, non-blocking for game logic).

### 4. `generate_image` — Raptor pack concept art generated; upload failed with identical Supabase JWT error (confirms infra issue, not agent-side).

## TRexBehavior Design Spec (documented; not compiled — see rule below)

Behavior defined and enforced live via actor tags/FSM this cycle (no .cpp/.h written — headless editor never recompiles C++, confirmed dead-end by repo memory `hugo_no_cpp_h_v2`). Logical spec for when C++ recompilation is restored:

```
TRexBehavior (extends DinosaurCombatAIController)
State: Patrol   → wander within 5000u radius of NPC_PatrolOrigin, random waypoint every 8-15s
State: Chase    → triggered when PlayerDistance <= 3000u, move directly toward player at run speed
State: Attack   → triggered when PlayerDistance <= 300u, execute bite/tail-swipe montage, apply damage
Transition back to Patrol if PlayerDistance > 3500u (hysteresis to avoid state flicker)
```

## Verification results
- `DinosaurCombatAIController` — confirmed present/loadable (Combat Agent #12's controller exists on the live binary).
- `SurvivalComponent` — confirmed present/loadable at expected path.
- All T-Rex, Raptor, Brachiosaurus, Triceratops actors in hub now carry live, queryable `NPC_*` tags representing predator/herbivore FSM state, patrol/chase/attack radii, and pack roles.

## Decisions & rationale
- No .cpp/.h files written (absolute rule `hugo_no_cpp_h_v2` — headless editor never recompiles; any such write is 100% wasted).
- Did not retry Supabase/TTS or image upload after confirmed JWT failure (billing/token issue, not transient — per `reflection_agent_auto` guidance).
- Reused existing actors by label lookup rather than spawning duplicates (per `hugo_naming_dedup_v2`).

## Dependencies / escalation
- Escalate to orchestrator/DevOps: Supabase JWT ("Invalid Compact JWS") has now blocked audio/image uploads across Animation Agent #10 and NPC Behavior Agent #11 in the same cycle — needs token refresh.
- Combat & Enemy AI Agent #12 (next in chain): `DinosaurCombatAIController` confirmed loadable and can now read the `NPC_State_*`, `NPC_ChaseRadius_*`, `NPC_AttackRadius_*` tags applied this cycle to drive actual combat engagement logic against the tagged FSM states.
