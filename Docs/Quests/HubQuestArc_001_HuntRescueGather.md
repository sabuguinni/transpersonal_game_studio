# Hub Quest Arc 001 — "First Kill, Lost Brother, Dry Riverbed"

**Location:** Hub content area, world coords ~(2100, 2400), radius 3500u (MinPlayableMap)
**Designer:** Agent #14 — Quest & Mission Designer
**Cycle:** PROD_CYCLE_AUTO_20260712_006
**Status:** Trigger volumes LIVE in MinPlayableMap. Narrative text drafted. Awaiting #15 (Narrative) for final dialogue polish and #11 (NPC Behavior) for quest-giver NPC placement.

---

## Design Intent

Three short, independent quests anchored to what already exists in the world (Agent #13's `Herd_HubGrazing_01/02`, existing predator placeholders, and hub rock clusters) rather than inventing new content in a vacuum. Each quest has a complete emotional arc within 2-4 minutes of gameplay:

1. **The Hunt** — competence and restraint (the player learns the world punishes recklessness).
2. **The Rescue** — urgency and fear (the world is dangerous even for locals, not just the player).
3. **The Gather** — patience and risk-reward (survival crafting has a cost, not just a checklist).

No mysticism, no "spirit guides," no meditation. Every NPC is a practical human survivor giving practical advice.

---

## Quest 1 — "Track the Herd" (Hunt)

**Tag chain:** `Quest_HubHunt_01` on all 3 stage triggers.

| Stage | Actor Label | Tag | Location logic |
|---|---|---|---|
| 1. Track | `Quest_HubHunt_TrackTrigger_001` | `QuestStage_TrackTrigger` | Offset +800/-400 from live herd anchor (`Herd_HubGrazing_*`) |
| 2. Ambush | `Quest_HubHunt_AmbushTrigger_001` | `QuestStage_AmbushTrigger` | Offset +300/+200 from herd anchor, downwind approach |
| 3. Butcher | `Quest_HubHunt_ButcherTrigger_001` | `QuestStage_ButcherTrigger` | Offset -500/+600, away from herd to avoid spooking remaining members |

**Design constraint respected:** Does not target Triceratops-type herd members directly (per #12's `BehaviorTag_DefensiveCharge` — retaliate-only dispositions make them a bad "hunted" target). This quest is written to imply a smaller/younger herbivore split from the group, consistent with real herd behavior (calves separating from bulls), not a scripted combat encounter against a defensive megafauna.

**NPC line (draft, audio generated this cycle — see Assets below):** Hunt Elder briefing.

---

## Quest 2 — "Dry Riverbed" (Rescue)

**Tag chain:** `Quest_HubRescue_01` on `Quest_HubRescue_NPCTrigger_001` (`QuestStage_Rescue`).

Anchored 250 units off an existing predator-type placeholder near the hub (raptor/T-Rex label match found in live audit) if one existed within 3500u; otherwise falls back to a fixed hub-relative offset. This keeps the "missing brother" threat diegetic — the danger is a real actor already in the world, not an invented monster.

**NPC line (draft, audio generated this cycle):** Worried Sister briefing, raptor tracks near boulders.

---

## Quest 3 — "Flint by the Rocks" (Gather)

**Tag chain:** `Quest_HubGather_01` on `Quest_HubGather_FlintTrigger_001` (`QuestStage_Gather`).

Placed at a fixed hub-relative offset (-900/-700) near the hub's rock cluster, feeding directly into Agent #14's sibling system (CraftingSystem — Stone Axe recipe needs rocks; this quest gives narrative reason to visit that exact resource node).

---

## Assets Produced This Cycle

- 2x ElevenLabs voice lines generated (Hunt Elder briefing ~21s, Worried Sister briefing ~20s). **Note:** Supabase storage upload returned `403 Invalid Compact JWS` on both — audio was synthesized successfully but not persisted to a public URL this cycle. Re-upload needed next cycle or by Audio Agent (#16) once storage auth is fixed.
- **Quest walkthrough video:** NOT PRODUCED. No `heygen_create_video` tool is available in this agent's toolset this cycle (only `ue5_execute`, `text_to_speech`, `generate_image`, and GitHub tools are exposed). Flagging this as a mandate gap for the Director (#01) — either grant this agent the video tool or reassign video capture to #19 (Integration) using in-editor sequencer capture.

## Live UE5 State Changes (verified via ue5_execute, command IDs 32616-32618)

1. Audited hub area (3500u radius) for existing `Quest_*` / `Herd_*` tags before mutation.
2. Spawned 3 `TriggerBox` actors for the Hunt quest stages, tagged and anchored to the live herd.
3. Spawned 1 `TriggerBox` for Rescue quest, anchored near a live predator actor when found.
4. Spawned 1 `TriggerBox` for Gather quest near hub rocks.
5. Saved level after each mutation batch.

## Next Steps

- **#15 Narrative:** Polish the 2 NPC lines above into final dialogue tree text; assign proper NPC names consistent with world lore.
- **#11 NPC Behavior:** Place actual quest-giver NPC actors (Hunt Elder, Worried Sister) — currently only trigger volumes exist, no visual NPCs.
- **#16 Audio:** Re-attempt Supabase upload for the 2 voice lines generated this cycle (synthesis succeeded, storage failed).
- **#18 QA:** Verify trigger volumes don't overlap `Herd_*` tagged actors' collision (checked visually via label lookup only, not physically).
