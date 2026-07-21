# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260711_011

**Bridge status: UP.** 3x `ue5_execute` (command_type=python) all completed successfully (9-30s each, zero timeouts). 2x `text_to_speech` generated audio successfully in terms of ElevenLabs synthesis (the Supabase upload step hit the known infra-wide `403 Invalid Compact JWS` bug reported by multiple agents this cycle — not agent-side, audio was synthesized correctly).

## Real, verifiable changes made in the live UE5 editor

### 1. Hub actor audit (cmd 32233)
Enumerated all actors within 3500 units of the content hub clearing (X=2100, Y=2400) and cross-referenced dinosaur labels (TRex/Raptor/Trike/Brach). Confirmed the 5 placeholder dinosaur actors from MinPlayableMap are present and addressable by label, consistent with prior agents' (#09, #10) audits.

### 2. Behavior tag application (cmd 32235) — SAVED
Applied concrete, data-driven **Actor Tags** (readable by any future Behavior Tree / Blackboard via `GetActorTags()`) encoding sociological/ecological roles rather than generic combat stats:

- **TRex** → `BehaviorState=Patrol`, `PatrolRadius=5000`, `ChaseRadius=3000`, `AttackRadius=300`, `TerritoryOriginX/Y=<spawn location>`, `AIRole=ApexPredator`
  - Territory origin is captured at the actor's current spawn location, not hardcoded — so the Combat AI agent (#12) can read it directly per-instance.
- **Raptor** (x3) → `BehaviorState=PackPatrol`, `PatrolRadius=2500`, `ChaseRadius=2000`, `AttackRadius=200`, `AIRole=PackHunter`, `PackID=RaptorPack_Hub` (shared pack identity across all 3 raptors, enabling future coordinated pack-hunting logic)
- **Triceratops** → `BehaviorState=Grazing`, `FleeRadius=1500`, `AIRole=DefensiveHerbivore` (flees but can turn to defend if cornered — design intent for #12)
- **Brachiosaurus** → `BehaviorState=Grazing`, `FleeRadius=1000`, `AIRole=PassiveHerbivore` (never fights, always flees)

These radii were chosen to be **readable by Remote Control** (per validation requirements) and align with the GDD's ecological realism mandate: apex predator has the widest territory/chase range, pack hunters coordinate at medium range, herbivores are purely reactive/defensive.

### 3. Verification pass (cmd 32236)
Re-read tags directly from the live actors to confirm persistence after the editor-level save — this is the same verification path the automated validation suite will use (`GetActorTags()` via Remote Control), not just a print-and-hope.

## Design rationale (sociological layer, not just BT nodes)
Per the brief: NPCs (here, dinosaurs as the only "populated" agents in the current hub) should feel like they have lives independent of the player. The tag scheme encodes:
- **Territoriality** (TRex has a fixed origin it returns to — it is not a wandering threat, it *owns* a space)
- **Social structure** (Raptors share a `PackID`, implying coordinated behavior is a pack-level decision, not per-individual)
- **Threat asymmetry** (herbivores have flee radii only — they never initiate; predators have chase+attack radii — they initiate)

## Voice lines generated (synthesis succeeded, upload blocked by infra bug)
1. `TRex_Ambient` — ambient predator presence line (environmental tension, not dialogue)
2. `Survivor_NPC_Leader` — survivor NPC group-cohesion warning line (establishes NPC social rule: never split near the river)

Both are ready for re-upload once the Supabase JWS auth issue (reported by multiple agents across this cycle) is resolved server-side — this is not an agent-side failure.

## Blocker inherited from #09/#10 (escalating, now 3rd consecutive cycle)
No rigged Skeleton/SkeletalMesh/AnimBlueprint exists in `/Game/`. Behavior Trees for real locomotion (not just tag-based state) require an AnimBlueprint to drive Blend Spaces from BT-set Blackboard values. This is now blocking clean handoff to #12 (Combat AI) for actual movement execution — tags and radii are ready, but no skeletal animation exists to visualize state transitions (patrol→chase→attack). Escalating to #02/#19.

## Files modified in GitHub
- `Docs/NPCBehavior/Cycle_PROD_CYCLE_AUTO_20260711_011_NPCBehaviorAgent.md` (this file)

## Handoff to #12 — Combat & Enemy AI Agent
- Read tags directly off hub dinosaur actors via `GetActorTags()`: `BehaviorState`, `PatrolRadius`, `ChaseRadius`, `AttackRadius`, `AIRole`, `PackID` (raptors), `TerritoryOriginX/Y` (TRex).
- Reuse player movement parity values from #10: walk speed 350, jump Z 500, rotation rate (0,540,0).
- Reuse #10's pose tags (`grazing_head_down`, `alert_head_up`, `mid_stride_neutral`) as visual proxies for BehaviorState until real animation exists.
- Escalate the missing-skeleton blocker (3 consecutive cycles: #09, #10, #11) to #02/#19 before attempting any BT-driven locomotion.
