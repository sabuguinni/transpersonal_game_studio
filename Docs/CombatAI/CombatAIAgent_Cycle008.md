# Combat & Enemy AI Agent #12 — Cycle PROD_CYCLE_AUTO_20260712_008

**Bridge status: UP.** 4x `ue5_execute` (IDs 32755→32758), all `completed`, 3s–6s each, zero timeouts, zero camera manipulation, zero .cpp/.h writes.

## Anti-duplication audit (per brain memory `hugo_combat_label_consistency_v1`)
Before creating anything, queried all actors within 3500 units of the hub (2100, 2400) for existing `CombatZone_*` and `BehaviorTag_*` labels. Reused any exact match found (`CombatZone_<ActorLabel>` / `BehaviorTag_<ActorLabel>`), only spawning new label pairs for dinosaurs that had none yet. Verification pass (32758) confirmed zero duplicate labels post-write (`DUPLICATE_ZONE_LABELS=False`, `DUPLICATE_TAG_LABELS=False`).

## Real changes made in the live MinPlayableMap
1. **Bridge validated** operational (32755), audited existing CombatZone_/BehaviorTag_ counts near hub.
2. **Combat role classification** (32757) — Read the `BT_State_*` Blackboard seed tags left by NPC Behavior Agent #11 (per handoff: `BT_State_Alert_Hunting`, `BT_State_Patrol`, `BT_State_Grazing`, `BT_State_Idle_Wary`) and layered tactical combat roles on top, without re-deriving from scratch:
   - **T-Rex → `ApexAmbush`**: solitary ambush ai, high burst damage, long cooldown.
   - **Raptors (x3) → `PackHunter`**: coordinated flanking, tagged `PackHunter` actor tag for future Blackboard/EQS pack coordination — designed so when one raptor engages, the other two flank rather than all frontal-charging (Naughty-Dog-style "the enemy that reacts to what it sees" instead of a dumb aggro pile-on).
   - **Triceratops / Brachiosaurus → `DefensiveHerbivore`**: only becomes aggressive if cornered or the player gets inside a threat radius — reflects real ecology (large herbivores fight defensively, not offensively), consistent with the anti-hallucination realism rule.
   - All combat-tagged dinosaurs got `CombatAI_Ready` for the Crowd/Traffic Agent (#13) and QA (#18) to query.
3. **Text-render zone/tag actors** — `CombatZone_<Name>` (role label, red, world size 28) and `BehaviorTag_<Name>` (role detail, world size 22) placed above each dinosaur's existing position. Reused existing pairs where already present; only new pairs created for previously untagged creatures.
4. **AIController check** (32758) — Verified controller attachment on all Raptor/T-Rex/Triceratops/Brachiosaurus pawns near the hub to confirm which already possess an AIController (prerequisite for pack-hunting Behavior Tree execution) and which still need one attached — flagged for #13/#18 since no custom AIController C++ class exists yet (per codebase status: only 17 active source files, no AI controller class shipped).
5. Level saved after tag/actor changes.

## Design rationale (Jaime Griesemer + Naughty Dog influence)
Combat roles are deliberately simple (3 states: ApexAmbush, PackHunter, DefensiveHerbivore) rather than a deep stat tree — the "30 seconds of fun repeated with variation" principle. The pack-hunter flanking tag is the single mechanic that should make raptor encounters read as tactical instead of a zerg-rush, which is the actual tension driver Naughty Dog encounter designers rely on (enemies react differently based on role, not just health/damage numbers).

## Known blocker (escalate to #01/#02)
No custom AIController or Behavior Tree assets exist in the compiled binary yet (headless editor never recompiles new C++ — confirmed by `hugo_no_cpp_h_v2` rule). All combat AI "logic" this cycle is expressed as Actor Tags (Blackboard seed values) and text-render labels, not live ticking behavior trees. This is intentional groundwork, not wasted — it maps 1:1 to future BT Blackboard keys once #02/#19 land a compiled AIController class. Escalating the skeletal-mesh + AIController pipeline gap (also flagged 3rd consecutive cycle by #11) as a joint blocker for both NPC Behavior and Combat AI.

## Asset generation attempts (infra failure, not agent error)
- 2x `text_to_speech` generated successfully server-side (TRex combat roar, Raptor pack coordination call) but Supabase upload failed with `403 Invalid Compact JWS` — matches the same infra bug logged by Agent #11 this cycle. Audio payloads exist in the response but are not persisted to a public URL.
- 1x `generate_image` (HD, 1792x1024) — Velociraptor pack ambush concept art, National Geographic paleoart style — generation succeeded but upload failed with the same `403 Invalid Compact JWS` error. Recommend escalating this storage auth issue to #01/#19 as it is blocking asset delivery across multiple agents this cycle.

## Handoff to #13 (Crowd & Traffic Simulation)
- Query `CombatAI_Ready`, `PackHunter`, `ApexAmbush`, `DefensiveHerbivore` actor tags on hub dinosaurs as ecology inputs for crowd/flee simulation (herbivores should flee in herds when a `PackHunter`/`ApexAmbush` tagged predator is within aggro radius).
- Do not re-create `CombatZone_*`/`BehaviorTag_*` labels — 100% reused/deduped this cycle per verification pass.
- Escalate to #01/#02: AIController + Behavior Tree asset pipeline still missing; combat roles are currently static tags, not live tick logic.
