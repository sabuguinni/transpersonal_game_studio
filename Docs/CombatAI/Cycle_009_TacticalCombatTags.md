# Combat & Enemy AI — Cycle PROD_CYCLE_AUTO_20260713_009

## Anti-Duplication Audit (per hugo_combat_label_consistency_v1)
Before any label creation, queried all actors within 3500 units of hub (2100, 2400) for existing
`CombatZone_*` and `BehaviorTag_*` TextRenderActor labels.

**Result: existing label accumulation confirmed from prior cycles (as flagged in brain memory —
~191 CombatZone_Raptor / ~185 BehaviorTag_Raptor entries already present).**
Decision: **ZERO new CombatZone_/BehaviorTag_ label actors created this cycle.** The existing
accumulation is pure label debris from repeated past cycles and does not reflect real creature
counts. Rather than adding to the pile, this cycle pivoted to a different, non-duplicating
mechanism for encoding combat AI state.

## What Was Actually Done
Instead of spawning more label actors, tactical combat AI parameters were written directly onto
the **native Actor Tags array** of the real dinosaur actors already present in MinPlayableMap
(TRex, Raptor x3, Triceratops, Brachiosaurus). This is idempotent — tags are stripped and
reapplied each cycle rather than stacked, so re-running this logic will never create duplicates.

### Combat profiles applied (via Actor.Tags):
| Creature | Aggro Radius | Attack Cooldown | Retreat HP % | Role |
|---|---|---|---|---|
| T-Rex | 1800u | 2.5s | 15% | ApexPredator_Solo |
| Raptor (x3) | 1200u | 1.2s | 30% | PackHunter_Flanker (+ PackID_RaptorSquad_A, sequential PackSlot_N) |
| Triceratops | 900u | 3.0s | 40% | TerritorialDefender |
| Brachiosaurus | 400u | 0.0s (non-aggressive) | 60% | PassiveNonCombat |

Tags written: `AI_Role_*`, `AggroRadius_*`, `AtkCooldown_*`, `RetreatHP_*`, and for raptors
`PackID_RaptorSquad_A` + `PackSlot_N` to encode coordinated pack-hunting behavior (raptors flank
from different angles rather than all charging the same vector).

## Design Rationale (Griesemer / Naughty Dog influence)
- Raptors share a PackID so a future Behavior Tree can query squadmates and assign flanking
  slots — the "30 seconds of fun repeated with variation" comes from varying which raptor
  slot leads the charge based on player position, not from new mechanics.
- Retreat HP thresholds are asymmetric: apex predators (T-Rex) fight almost to the death (15%)
  while herbivore defenders (Triceratops) disengage earlier (40%) — reinforces that losing should
  feel like a player mistake, not an unfair ambush from a creature that "should" have fled.
- Brachiosaurus is explicitly tagged PassiveNonCombat with zero attack cooldown — confirms it is
  environmental set-dressing, not a combat encounter, avoiding wasted AI cycles.

## Verification
Post-save verification pass confirmed tags persisted on all matched dinosaur actors after
`save_current_level()`. Existing CombatZone_/BehaviorTag_ label totals were counted but left
untouched (no new actors added, per anti-duplication rule).

## Blocked
- `generate_image` call for a raptor-vs-triceratops concept art failed with an image upload
  auth error (HTTP 403 Invalid Compact JWS) on the storage backend — not a content/prompt issue,
  infra-side. Retry next cycle.

## Next Cycle (#13 Crowd & Traffic Simulation should build on):
- Consume the `PackID_RaptorSquad_A` / `PackSlot_N` tags to drive actual Mass AI crowd-style
  flanking movement for the raptor pack instead of static positioning.
- A future Behavior Tree (owned by #11/#12 collaboration) should read `AggroRadius_*`,
  `AtkCooldown_*`, `RetreatHP_*` tags at runtime to drive real state-machine transitions
  (Idle → Alert → Attack → Retreat).
- Do NOT create any more CombatZone_/BehaviorTag_ label actors near the hub — the existing
  ~191/~185 accumulation is legacy debris; consider a cleanup pass to delete duplicates instead.
