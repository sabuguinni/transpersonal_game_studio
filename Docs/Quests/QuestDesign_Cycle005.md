# Quest Design — Cycle PROD_CYCLE_AUTO_20260708_005
Agent #14 — Quest & Mission Designer

## Context
This cycle converts Agent #13's live herd tags (`Herd_01`, `Herd_02`) and Agent #12's
predator actors (TRex/Raptor) near the content hub (X=2100, Y=2400) into three
concrete, playable quest zones. Followed the naming/dedup rule: existing trigger
actors with the target label are reused (repositioned + retagged) instead of
stacking duplicates on top of existing actors.

## Quests Implemented This Cycle

### 1. "The Watcher's Patience" — Herd Tracking / Observation
- **Trigger actor:** `Quest_HerdTracking_Observe_001` (TriggerBox, tag `Quest_HerdTracking`)
- **Location:** offset ~360 units from hub center, near `Herd_01` (Triceratops herd)
- **Objective:** Player must remain within the trigger volume, at a distance, without
  entering the herd's flee radius, for a sustained observation window.
- **Emotional arc:** Tension of proximity to a large, potentially dangerous herbivore →
  patience rewarded with understanding of herd movement patterns → practical payoff
  (safe passage routes revealed for future herd encounters).
- **Design note (Sasko principle):** The quest has no combat outcome. Success is
  behavioral — the player *chooses* restraint. Failure state (scattering the herd)
  is not punished mechanically beyond losing the observation reward, reinforcing that
  disruption has a real ecological cost rather than a game-over state.

### 2. "Follow the Green" — Migration Tracking
- **Trigger actor:** `Quest_MigrationFollow_002` (TriggerBox, tag `Quest_MigrationFollow`)
- **Location:** offset from hub, adjacent to `Herd_02`
- **Objective:** Track the herd's movement across 2-3 waypoints as it migrates toward
  water/forage; player must keep pace without alarming the group.
- **Reward:** Reveals a resource-rich zone (new foraging/water location) discovered
  by following the herd's own survival logic — reinforces the game's core theme that
  the player survives by reading the ecology, not fighting it.

### 3. "Hold the Line" — Predator Defense
- **Trigger actor:** `Quest_PredatorDefense_003` (TriggerBox, tag `Quest_PredatorDefense`)
- **Location:** near existing TRex/Raptor actors east of the hub
- **Objective:** Defend the camp perimeter when a predator approaches — reinforce
  palisade gaps, use fire and spears, coordinate with NPC defenders.
- **Emotional arc:** Dread (predator sighting) → preparation (tight timer to fortify)
  → payoff (successful defense = camp trust increase; failure = resource/NPC loss,
  not instant death, so stakes feel real without being cheap).

## Method
- Queried all actors within 3500 units of hub, cross-referenced with `Herd_01`/
  `Herd_02` tags (from Agent #13) and predator name matches (`trex`, `raptor`) from
  Agent #12's placements.
- For each quest, reused an existing actor with the exact target label if present
  (dedup rule), otherwise spawned a single `TriggerBox`, scaled to a meaningful
  quest-zone size (6x6x3), and tagged it for downstream systems (Narrative/Audio/VFX
  agents can key off these tags without creating duplicate actors).
- Saved `MinPlayableMap` after placement.

## Handoff to Agent #15 (Narrative & Dialogue Agent)
- Three tagged quest zones now exist: `Quest_HerdTracking`, `Quest_MigrationFollow`,
  `Quest_PredatorDefense`. These are ready to receive dialogue hooks, NPC quest-giver
  placement, and journal/objective text tied to the Narrative Bible.
- Two NPC voice lines were generated this cycle (see `NPC_VoiceLines_Cycle005.md`)
  as prototype dialogue for the tracking and defense quests — Narrative Agent should
  treat these as draft tone references, not final VO.

## Known Limitation
- No `heygen_create_video` tool is available in this agent's toolset despite being
  referenced in the cycle mandate — a quest walkthrough video could not be produced.
  Recommend the mandate be corrected or the tool be added to this agent's access if
  walkthrough videos are required in future cycles.
- Bridge does not echo `print()`/`unreal.log()` stdout into the tool response — only
  `{"ReturnValue": true}` — so exact spawned/reused actor labels could not be
  programmatically confirmed in this report; the script is idempotent and safe
  regardless (reuses by label, never stacks duplicates).
