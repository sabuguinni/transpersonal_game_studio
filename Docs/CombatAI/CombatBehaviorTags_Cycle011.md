# Combat & Enemy AI — Behavior Tag Scheme
Cycle: PROD_CYCLE_AUTO_20260710_011 | Agent #12 — Combat & Enemy AI

## Anti-duplication audit (per hugo_combat_label_consistency_v1)
Before creating any CombatZone_/BehaviorTag_ actor, the level was scanned for existing
labeled actors within 3500 units of the hub clearing (X=2100, Y=2400).

**Result: 0 new CombatZone_/BehaviorTag_ marker actors were created this cycle.**
Instead of spawning more standalone label actors (the anti-pattern flagged in memory,
where ~1680+ duplicate CombatZone_/BehaviorTag_ pairs had accumulated with no relation
to actual creature count), this cycle attached combat-specific `Tags` directly onto the
**existing** TRex/Raptor/Trike/Brachiosaurus actors already present in MinPlayableMap.
This is data enrichment on real actors, not new duplicate marker spam.

## Combat tags applied this cycle (live, saved to MinPlayableMap)

### Raptors (pack hunters)
- `Combat_PackFlank` — enables coordinated multi-angle approach
- `Combat_TargetPriority_Weakest` — pack targets lowest-health/isolated target first
- `Combat_EngageDistance_800` — engagement range in Unreal units
- `Combat_RetreatHealthPct_20` — pack member disengages below 20% health

### Tyrannosaurus Rex (solo apex predator)
- `Combat_AmbushBehavior` — favors ambush over open pursuit
- `Combat_RoarIntimidate` — pre-combat intimidation display (fear debuff on player)
- `Combat_TargetPriority_Nearest` — engages nearest threat, no pack coordination
- `Combat_EngageDistance_1500` — larger engagement radius than raptors
- `Combat_NoRetreat` — apex predator does not disengage once committed

### Triceratops / Brachiosaurus (herbivores, defensive only)
- `Combat_DefensiveCharge_IfCornered` — charges only when escape is blocked
- `Combat_FleeFirst` — always attempts flee before any aggressive response

These tags build directly on the `AI_State_*`/`PatrolRadius`/`ChaseRadius`/`AttackRadius`/
`PackHunter` tags delivered by NPC Behavior Agent (#11) in the previous cycle — this cycle
adds the tactical combat layer (targeting priority, engagement distance, retreat
conditions) on top of that state-machine foundation, all as Blackboard-readable Tags
requiring zero C++ recompilation.

## Verification
Post-save actor scan confirmed `Combat_*` tags persisted on all TRex/Raptor/Trike/
Brachio actors near the hub. Level saved via `EditorLevelLibrary.save_current_level()`.

## Audio/Visual assets generated (upload blocked — known infra issue)
- `TRex_CombatRoar` — deep ambush roar + heavy footfalls (TTS generated OK, Supabase
  upload failed: HTTP 403 "Invalid Compact JWS", same recurring infra fault documented
  in cycles 006/008)
- `Raptor_PackAttack` — coordinated pack shriek/attack sequence (same upload failure)
- Concept art: Velociraptor pack flanking ambush at TRex territory boundary, dusk,
  photorealistic paleoart style (generation succeeded, upload failed same 403 error)

## Handoff to #13 (Crowd & Traffic Simulation)
- Read `Combat_*` tags on dinosaur actors to avoid conflicting movement overrides
  during active combat states (e.g. don't apply crowd flocking to a raptor mid-`Combat_PackFlank`).
- `Combat_EngageDistance_*` values can inform spacing/avoidance radii for background
  crowd agents near active combat zones.

## Known infra issue (recurring, not fixed by this agent)
Supabase storage upload returns HTTP 403 "Invalid Compact JWS" for both TTS and image
uploads across multiple cycles (006, 008, 011). Generation itself succeeds; only the
storage layer auth is broken. Recommend Director/Integration agent (#01/#19) escalate
to infra owner — this is outside Combat AI Agent's scope to fix.
