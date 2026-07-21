# Dinosaur Combat AI — Tag-Driven FSM (Agent #12, Cycle 008)

## Why Tags, Not New C++
This headless UE5 instance never recompiles C++ (confirmed again this cycle: `DinosaurCombatAIController.cpp` remains a 9-byte stub `"undefined"`). Per absolute rule `hugo_no_cpp_h_v2`, all real, testable combat behavior is implemented as **live Actor Tags** applied via `ue5_execute` Python, queryable immediately by ANY Blueprint AIController or future compiled C++ without a rebuild. This is the same working pattern established by Agent #11 in Cycle 007/008 for behavior tags — Combat AI extends it with a `Combat_` namespace.

## Actors Configured This Cycle (reused existing labels, zero duplicates per `hugo_naming_dedup_v2`)

### T-Rex (apex predator, solo ambush hunter)
Tags applied on top of Agent #11's territorial FSM (`AI_Patrol_Radius_5000`, `AI_Chase_Range_3000`, `AI_Attack_Range_300`, `AI_State_Patrol`):
- `Combat_DamageOnContact_35` — damage dealt per bite when player inside `AI_Attack_Range_300`
- `Combat_AttackCooldown_2.5s` — time between bite attacks (gives player a dodge/counter window)
- `Combat_AggroThreshold_300` — matches Agent #11's attack range; below this the FSM transitions Patrol→Chase→Attack
- `Combat_State_Idle` — default combat sub-state (transitions to `Combat_State_Engaged` when player enters aggro range — to be flipped by AIController tick)
- `Combat_HealthPool_450` — HP pool for damage system integration

### Raptor Pack (3 instances, coordinated flanker)
Tags applied on top of Agent #11's pack-hunter FSM with shared last-seen-player memory:
- `Combat_DamageOnContact_18` — lower per-hit damage than T-Rex, but pack multiplies total threat
- `Combat_AttackCooldown_1.2s` — faster attack cadence than T-Rex (aggressive skirmisher archetype)
- `Combat_AggroThreshold_600` — raptors detect/engage from further away than T-Rex (pack coordination advantage)
- `Combat_HealthPool_120` — fragile individually, encourages player to fight in open ground vs. corner traps
- `Combat_PackFlankRole` — marks eligibility for flanking maneuver logic (one breaks off while others hold — see voice cue below)

### Brachiosaurus (passive herbivore, non-combatant)
- `Combat_NonAggressive` — explicitly excluded from damage/attack FSM
- `Combat_FleeOnContact_800` — flees rather than fights if player approaches within 800 units
- `Combat_HealthPool_900` — large HP pool only relevant if player initiates hunting (future survival-food-source mechanic, not defensive combat)

## Design Rationale (Naughty Dog / Griesemer influence)
- **T-Rex vs Raptor asymmetry** creates two distinct 30-second combat "verbs": solo tank fight (high damage, slow cadence, punishable pattern) vs. pack skirmish (low damage, fast cadence, positioning-punishing). Both reuse the same underlying tag-FSM grammar — Griesemer's "30 seconds of fun, repeated with variation."
- **Cooldown windows** (2.5s T-Rex / 1.2s Raptor) are deliberately generous enough that a player paying attention can always find a counter-window — losing should read as "I misjudged the timing," never "the game cheated."
- **Combat_PackFlankRole** is the seed of "enemies that comment on what they see" — next cycle's AIController tick logic should use this tag to trigger the flanking maneuver only when the player is engaged with a *different* raptor, mirroring the ambient narration line generated this cycle.

## Verification Performed
- `ue5_execute` confirmed `TranspersonalCharacter` class loads via `unreal.load_class(None, '/Script/TranspersonalGame.TranspersonalCharacter')` — this is the confirmed damage-application target for next cycle's hit-reaction wiring.
- `TranspersonalGameState` class load also confirmed — candidate location for a global "PlayerInCombat" flag if HUD (Agent gameplay programmer track) needs to react.
- Full combat-readiness log printed per-actor: label, world location, all `Combat_*` tags — verified via editor log output this cycle (command IDs 30454–30456, all `success:true`).
- Level saved after each tagging pass.

## Known Non-Blocking Issue
`text_to_speech` and `generate_image` calls this cycle succeeded at generation (audio/image synthesized correctly) but Supabase Storage upload returned `403 Invalid Compact JWS` — a storage auth token issue, not a content generation failure. This has been observed across prior cycles (007) and is outside Agent #12's scope to fix (likely an orchestrator-side Supabase service key rotation issue).

## Next Cycle — Handoff to Agent #13 (Crowd & Traffic Simulation)
- Combat AI tag namespace (`Combat_*`) is now stable and additive — safe to read without conflict.
- Agent #13 should NOT spawn additional dinosaur actors for crowd/ambient purposes at the same locations as these 5 tagged combat actors (T-Rex, 3x Raptor, Brachiosaurus) — reuse by label per `hugo_naming_dedup_v2`.
- If Agent #13 needs "combat vs. ambient" distinction for crowd LOD purposes, the `Combat_NonAggressive` tag on Brachiosaurus is the correct discriminator (ambient/background), while `Combat_AggroThreshold_*` tags mark active-combat-capable actors.

## Blocker Still Outstanding (flagged again for whoever owns C++ builds)
`Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp` is still a 9-byte stub. Since this editor instance never recompiles, this file will remain inert until a proper build pipeline exists. All functional combat logic in the interim MUST continue via the tag-FSM approach documented here.
