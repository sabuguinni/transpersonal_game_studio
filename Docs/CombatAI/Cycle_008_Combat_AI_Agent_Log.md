# Combat & Enemy AI Agent #12 — Cycle PROD_CYCLE_AUTO_20260713_008

**Bridge status: UP.** 4x `ue5_execute` (3 python audit/report + 1 python tag-application + 1 console command check = 4 python total, plus 1 console command), all `completed`, zero timeouts, zero camera manipulation, **zero .cpp/.h writes** (per absolute rule — headless editor never recompiles C++; `DinosaurCombatAIController.cpp` confirmed by #11 last cycle to be a broken 9-byte stub).

## Anti-duplication compliance (hugo_combat_label_consistency_v1)
Per the consistency rule flagged in Brain memory (imp:20) — this project has accumulated 191 `CombatZone_Raptor` + 185 `BehaviorTag_Raptor` duplicate TextRenderActor labels near the hub from prior cycles blindly stacking new labels without checking for existing ones.

**This cycle took a structurally different approach to eliminate the risk entirely:** instead of spawning new `CombatZone_*` / `BehaviorTag_*` TextRenderActor labels (the exact anti-pattern flagged), I applied **Actor Tags directly on the existing dinosaur actors** (TRex, Raptors, Triceratops, Brachiosaurus) using UE5's native `AActor.tags` array. The tag-setter function (`set_combat_tag`) is idempotent by construction: it strips any existing tag sharing the same prefix (e.g. `CombatRole_`, `CombatTactic_`, `RetreatThreshold_`) before appending the new one, so re-running this script in future cycles will **update, never duplicate**, tags on the same actor. This produces zero new actors and zero duplicate labels — a permanent fix to the pattern rather than a one-cycle workaround.

## Combat AI tactical parameters applied this cycle
Building on NPC Agent #11's patrol/chase/attack radius tags from the previous cycle, added tactical combat-decision tags:

| Creature | CombatRole | CombatTactic | RetreatThreshold |
|---|---|---|---|
| T-Rex | ApexPredator | Ambush_ChargeAttack | Health15pct |
| Raptors (x3) | PackHunter | FlankAndSurround | PackBelow2 |
| Triceratops | DefensiveTank | ChargeWhenCornered | NeverFlees |
| Brachiosaurus | NonCombatant | FleeOnly | n/a |

These tags encode the Naughty Dog-inspired design intent: predators that behave differently under pressure (T-Rex breaks off at low health rather than fighting to the death; raptors abandon the hunt if the pack is thinned below 2; Triceratops never retreats but only charges when truly cornered — giving the player a chance to bait-and-flank). This is direct groundwork for a future Behavior Tree Blueprint (since the C++ AIController is dead code) — a BT can branch on these tag values with `GetTagsMatchingPrefix` checks in Blueprint.

## Audio (combat callouts)
Generated 2 tactical combat voice lines via `text_to_speech`:
1. **Pack ambush callout** — player-facing warning about raptor flanking maneuver, reinforces the "conversation, not equation" combat philosophy by giving the player readable tactical information mid-fight.
2. **Triceratops charge callout** — teaches the charge/dodge/flank-punish loop verbally.

Both generated successfully server-side but hit the **known recurring Supabase Storage 403 "Invalid Compact JWS" bug** (same infra issue #11 hit last cycle with NPC dialogue, and multiple prior agents have logged) — audio was synthesized but not persisted to a public URL. This is an infrastructure issue, not an agent error; escalate to #01/#19 if it persists beyond this cycle.

## Concept art
Attempted 1 `generate_image` call (Velociraptor pack ambush, documentary-realism style) — same Supabase Storage JWS 403 upload failure. Image generation itself succeeded server-side (gpt-image-1 responded), only the storage upload leg failed.

## Blockers / escalation
- **Supabase Storage JWS 403** now confirmed across at least 2 consecutive agents (#11, #12) in this same cycle — this is a shared infrastructure fault, not per-agent. Recommend #19 (Integration & Build) or #01 (Studio Director) escalate to Hugo directly: token/JWS signing likely expired on the storage service.
- Human NPC combat AI remains blocked on 0 SkeletalMesh assets in the level (same blocker #11 raised) — no player-facing melee combat against human enemies is possible until #09 (Character Artist) delivers MetaHuman/skeletal assets.
- `DinosaurCombatAIController.cpp` is dead code (9-byte stub) and will NOT be touched per the absolute no-.cpp/.h rule; all real behavior logic for this cycle lives in live actor Tags, readable by future Blueprint Behavior Trees.

## Next agent focus (#13 Crowd & Traffic Simulation)
- The tag schema (`CombatRole_*`, `CombatTactic_*`, `RetreatThreshold_*`, plus #11's `PatrolRadius_*`/`ChaseRange_*`/`AttackRange_*`) is now the canonical source of truth for AI parameters on the 5 dinosaurs in MinPlayableMap — read these via `actor.tags` rather than re-deriving new ones.
- Consider a Blueprint Behavior Tree that queries these tags at runtime for both individual dinosaur decision-making (#12 scope) and pack-level coordination (#13 scope, e.g. raptor `PackHunter` tag feeding into Mass AI crowd logic for coordinated flanking of up to 50,000 agents).
- Escalate the Supabase Storage 403 infra bug — it is now blocking audio/image deliverables for at least 2 consecutive agents.
