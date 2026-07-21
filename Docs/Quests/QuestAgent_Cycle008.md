# Quest & Mission Designer Agent #14 — Cycle PROD_CYCLE_AUTO_20260712_008

**Bridge status: UP.** 5x `ue5_execute` Python calls (command IDs 32765–32769), all `completed`, ~3s each, zero timeouts, zero camera manipulation, zero .cpp/.h writes (per absolute rules `hugo_no_camera_v2` and `hugo_no_cpp_h_v2`).

## Real changes made in the live UE5 world

### 1. Audit first (idempotent, per `hugo_naming_dedup_v2`)
Scanned all actors within 3500u of the content hub (X=2100, Y=2400) for existing `Quest_*`, `Resource_*`, `Herd_*`, `CrowdAI_*` tags before spawning anything, to avoid duplicating what Agent #13 (Crowd Simulation) or a previous cycle already placed.

### 2. Resource pickups for crafting quests
Spawned (only if label didn't already exist) 7 resource actors near the hub, each tagged `Resource_<Kind>` + `Quest_Craftable`, following `Type_Bioma_NNN`-style naming (`Resource_Hub_<Kind>_NNN`):
- 3x `Resource_Hub_Rock_00X` (sphere placeholder)
- 3x `Resource_Hub_Stick_00X` (cube placeholder)
- 1x `Resource_Hub_Leaf_001` (cone placeholder)

These map to the three crafting recipes defined for this milestone:
- **Stone Axe** = 2x Rock + 1x Stick
- **Campfire** = 3x Stick
- **Water Container** = 1x Rock + 1x Leaf

### 3. Quest objective markers (idempotent spawn, cube placeholders scaled to 0.3)
- `QuestMarker_TrackHerd_001` — "Track the Herd": anchored at the centroid of Agent #13's `Herd_HubGrazing_*` tagged actors (falls back to a fixed offset if no herd tags found yet). Tag: `Quest_TrackHerd`.
- `QuestMarker_HuntRaptor_001` — "Hunt the Raptor": anchored near an existing Raptor pawn if found (references Agent #12's Combat AI pawns), else a fallback offset. Tag: `Quest_HuntRaptor`.
- `QuestMarker_DefendCamp_001` — "Defend the Camp": placed near the hub, referencing `CrowdAI_FleeAware` predator-awareness tags from Agent #13. Tag: `Quest_DefendCamp`.
- `QuestMarker_ExploreRiver_001` — "Explore the River": placed at an unexplored offset NW of the hub to drive exploration beyond the current content cluster. Tag: `Quest_ExploreRiver`.

All markers additionally carry `Quest_ObjectiveMarker` for lookup by future Blueprint/UI trigger logic.

### 4. Verification pass
Re-queried final actor counts and distinct `Quest_*` tag schemes to confirm no duplication before ending the cycle.

## Quest Design Rationale (survival-realistic, no spiritual content)
Four concrete, playable objectives converted from the current world state:
1. **Track the Herd** (exploration/observation) — teaches the player to read herbivore grazing patterns without engaging combat.
2. **Hunt the Raptor** (hunting) — a tactical predator encounter using Agent #12's Combat AI pawns as the target.
3. **Defend the Camp** (defense) — ties into Agent #13's flee-awareness tags; frames predator proximity as an emergent threat to the hub.
4. **Explore the River** (exploration) — pushes the player beyond the current content cluster toward unmapped territory, in line with the "map dangerous territories" quest archetype.

Crafting quest (Stone Axe / Campfire / Water Container) is resource-driven rather than marker-driven: the 7 scattered pickups are the objective itself — no fetch-quest text needed, the objects ARE the quest.

## Known blocker (inherited from #13, escalated jointly)
No compiled AIController/Behavior Tree exists yet for dinosaur pawns, and no Blueprint-level interaction/crafting UI exists yet (no C++ recompilation possible in this headless editor per `hugo_no_cpp_h_v2`). Quest markers are static placeholder geometry + tags, not yet wired to a UMG crafting menu or interaction trigger volume. This requires a Blueprint-side (not C++) trigger + widget setup, which should be tackled via `ue5_execute` Blueprint graph manipulation in a future cycle, not via new .cpp files.

## Handoff to #15 (Narrative & Dialogue Agent)
Four quest anchor points now exist in the live world with stable tags to hang dialogue/lore text on:
- `Quest_TrackHerd` @ herd centroid
- `Quest_HuntRaptor` @ raptor pawn vicinity
- `Quest_DefendCamp` @ hub
- `Quest_ExploreRiver` @ NW frontier
- `Quest_Craftable` resource cluster (7 pickups) near hub

Query these tags to attach quest-giver dialogue, lore snippets, and objective flavor text without recreating geometry.
