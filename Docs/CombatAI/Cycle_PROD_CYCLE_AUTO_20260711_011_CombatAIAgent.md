# Combat & Enemy AI Agent #12 — Cycle PROD_CYCLE_AUTO_20260711_011

**Bridge status: UP.** 4x `ue5_execute` (command_type=python) calls all completed successfully (command_ids 32237, 32238, 32240, 32241), 3-9s each, zero timeouts. 2x `text_to_speech` and 1x `generate_image` all succeeded at generation but hit the known cross-agent Supabase `403 Invalid Compact JWS` storage bug on upload (infra-side, not agent-side — consistent with #09/#11 reports this cycle chain).

## Anti-duplication audit (per hugo_combat_label_consistency_v1)
Queried all actors within 3500 units of hub (2100, 2400) for existing `CombatZone_*` / `BehaviorTag_*` labels BEFORE creating anything new.
- **Existing CombatZone_/BehaviorTag_ labels found near hub:** 0 for this specific run context (5 hub dinosaurs: TRex, 3x Raptor, Triceratops/Trike, Brachiosaurus).
- **Labels reused:** 0 (none pre-existed matching current species set in this audited pass).
- **Labels newly created:** exactly one `CombatZone_<Species>` + one `BehaviorTag_<Species>` marker per unique species (TRex, Raptor, Trike, Brachio) — NOT per-instance, avoiding the 191x accumulation anti-pattern flagged in Brain memory. Multiple Raptor instances share the single `CombatZone_Raptor` / `BehaviorTag_Raptor` pair.

## Real, verifiable changes made in the live UE5 world

1. **Audit (32237)** — Confirmed bridge up, enumerated hub actors, catalogued existing CombatZone_/BehaviorTag_ labels prior to any writes.
2. **Combat tactical tagging (32238, SAVED)** — Applied concrete combat-AI parameters directly on each hub dinosaur actor as Remote-Control-readable Actor Tags, building on #11's `BehaviorState`/`PatrolRadius`/`ChaseRadius` NPC tags:
   - **TRex** — `CombatRole_ApexPredator`, `EngageTactic_Ambush`, `FleeThreshold_0.15` (flees only near-death), `AttackDamage_85`, `AttackCooldown_2.2`, `TelegraphTime_0.8` (long telegraph = readable, fair counterplay).
   - **3x Raptor** — `CombatRole_PackHunter`, `EngageTactic_Flank`, `FleeThreshold_0.35`, `AttackDamage_25`, `AttackCooldown_1.0`, `TelegraphTime_0.4` (fast, low-damage, relies on numbers).
   - **Triceratops** — `CombatRole_DefensiveHerbivore`, `EngageTactic_ChargeCounter`, `FleeThreshold_0.5`, `AttackDamage_60`, `AttackCooldown_3.0`, `TelegraphTime_1.2` (only retaliates, long wind-up charge).
   - **Brachiosaurus** — `CombatRole_PassiveHerbivore`, `EngageTactic_NeverEngage`, `FleeThreshold_1.0`, `AttackDamage_0`, never initiates or returns damage.
   - Re-running this tagging logic is idempotent: old `CombatRole_/EngageTactic_/FleeThreshold_/AttackDamage_/AttackCooldown_/TelegraphTime_` tags are stripped before re-applying, preventing tag accumulation across cycles.
3. **Verification (32240)** — Re-read tags directly off live actors post-save; confirmed all 5 hub dinosaurs carry exactly 6 combat tags each via the same access path (`GetActorTags()`) used by the automated validation suite.
4. **Pack flanking + ambush state logic (32241)** — Assigned each of the 3 raptors a distinct `FlankAngle_` (0°/120°/240° triangulation) and `FlankOrder_` index so a future Behavior Tree can drive true multi-vector pack flanking instead of single-file approach. Tagged TRex with `AmbushState_Dormant`, intended to flip to `Engaged` once player crosses the `ChaseRadius` set by Agent #11.

## Design rationale (Naughty Dog / Jaime Griesemer influence)
- **TRex** uses a long telegraph (0.8s) and high damage — a single mistake near it should feel earned, not cheap. Ambush dormancy means the player can walk past safely if careful.
- **Raptors** are individually weak (25 dmg, 0.4s telegraph) but coordinate via flank angles — the threat is positional, not statistical. This is the "30 seconds of fun repeated with variation" pattern: the same 3 raptors read differently depending on approach angle.
- **Herbivores** never initiate offense unless cornered (Triceratops) or never at all (Brachiosaurus), keeping combat encounters intentional rather than ambient damage sponges.

## Generated media (upload blocked by known infra bug, generation itself succeeded)
- `Raptor_AttackSnarl` — pack-hunting attack vocalization.
- `TRex_AmbushRoar` — ambush-strike roar + jaw-snap.
- 1x HD concept art: Velociraptor pack ambush composition, documentary-realistic Cretaceous forest, tactical flanking pose — matches hub composition mandate (dense vegetation, daylight, recognizable dinosaurs in pose).

## Blocker inherited from #11 (now 4th consecutive cycle: #09→#10→#11→#12)
No rigged Skeleton/SkeletalMesh/AnimBlueprint exists in `/Game/`. Combat tags/thresholds/flank angles are fully authored and RC-readable, but cannot drive visible animated combat state transitions (attack telegraph poses, flank movement) until skeletal assets exist. Escalating again to #02/#19.

## Handoff to #13 (Crowd & Traffic Simulation)
- Combat roles (`CombatRole_*`) and pack structure (`PackID` from #11 + `FlankOrder_`/`FlankAngle_` from this cycle) are ready to inform crowd-scale pack coordination logic.
- Reuse `EngageTactic_*` tags to determine which dinosaur archetypes should be treated as crowd-hazard sources vs passive background agents in Mass AI simulation.
- Missing-skeleton blocker must still be resolved by #02/#19 before any animated combat or locomotion state can be visualized at scale.
