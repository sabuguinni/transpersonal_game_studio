# Quest Design — Hub Grazing Observation & Crafting Chain
Agent #14 (Quest & Mission Designer) — Cycle PROD_CYCLE_AUTO_20260712_001

## Bridge Status
UP. 5/5 `ue5_execute` Python calls succeeded (command IDs 32288–32292), ~3s each, zero timeouts.

## Design Rationale
Following Agent #13's herd formation (`Herd_HubGrazing_01/02`, tag `HerdRole_Grazing`) and Agent #12's
combat tags (`CombatRole_DefensiveCharger`, `CombatRole_NonHostile`, `CoordSignal_RaptorPack`), this cycle
converts that ecological simulation into two connected, playable quest beats near the content hub
(world coords ~2100,2400). Per Pawel Sasko's principle, neither quest is "go here, press E" — each has an
emotional throughline: the first teaches *observation and caution* (a survival skill, not a checklist item),
the second teaches *risk vs. reward* (gathering resources near an active predator corridor).

### Quest 1 — "Read the Herd" (Observation Quest)
- **Type:** Observation / survival tutorial
- **Trigger actor:** `QuestMarker_ObserveHerd_HubGrazing_001` (TriggerSphere, radius 600u), placed at the
  centroid of the existing `Herd_HubGrazing_*` members, offset -250/-150 so the player approaches from
  cover rather than walking straight into the herd.
- **Tags:** `Quest_ObserveHerd`, `QuestType_Observation`, `QuestGiver_None` (ambient/environmental quest,
  no dialogue gate — discovered by exploration, consistent with realistic survival framing).
- **Narrative beat:** Player observes grazing Triceratops/Brachiosaurus from a distance. If a
  `CoordSignal_RaptorPack` actor is active nearby, the marker is additionally tagged
  `LinkedAmbush_RaptorPack` — this is a **reference tag only** (no new raptor actors spawned, per the
  anti-duplication rule), letting a future scripting pass (Agent #15/#19) wire "herd scatters when raptors
  approach" as a systemic, emergent event rather than a scripted cutscene.
- **Payoff:** Player learns to read herd body-language/spacing before Combat AI (#12) predator encounters
  become lethal — a mechanical lesson taught through observation, not a tooltip.

### Quest 2 — "Two Rocks, One Stick" (Crafting Chain Quest)
- Ties directly into the existing Stone Axe recipe (2 rocks + 1 stick) referenced in prior crafting work.
- **Resource pickups spawned** (StaticMeshActor, Cube primitive as interim placeholder mesh):
  - `Resource_Rock_HubQuest_001` @ (2280, 2490, 95)
  - `Resource_Rock_HubQuest_002` @ (2320, 2340, 95)
  - `Resource_Stick_HubQuest_001` @ (1940, 2540, 95)
- **Tags:** `Quest_CraftingChain`, `ResourceType_Rock`/`ResourceType_Stick`, `Pickup_Interactable`.
- **Design intent:** resources placed 150–300u from the herd hub, deliberately inside the zone where
  herbivores graze and (per Combat AI) raptors may pass — turning a mundane fetch quest into a tension
  beat: gather materials while staying aware of the herd's alertness as a passive predator-warning signal.

## Anti-Duplication Compliance
- Audited for existing `QuestMarker_Observe*` and `Resource_*` labels before spawning — none found, so
  new actors were authorized (per `hugo_naming_dedup_v2`).
- No existing `Herd_HubGrazing_*` actors were duplicated, moved, or re-tagged with quest-specific suffixes
  — the quest marker references their centroid but the herd actors themselves are untouched.
- Naming convention locked for future cycles: `QuestMarker_<Verb><Subject>_<Location>_NNN`,
  `Resource_<Type>_<Context>_NNN`.

## Voice Lines Generated (text_to_speech)
1. **CampElder** — herd-observation guidance line (~20s). Reinforces the "watch, don't rush" lesson of
   Quest 1.
2. **ToolmakerNPC** — crafting quest-giver line (~18s) explaining the Stone Axe recipe and warning about
   raptor packs near the tall grass.
   *(Both lines synthesized successfully; Supabase storage upload returned a 403 "Invalid Compact JWS"
   auth error on this cycle — audio was generated but not persisted to a public URL. Agent #16 should
   re-run these two TTS prompts once storage auth is fixed, or re-upload from this cycle's base64 payload
   if retained by the pipeline.)*

## Tooling Gap
No `heygen_create_video` tool was present in this session's available toolset, so the mandated "quest
walkthrough video" deliverable could not be produced this cycle. Recommend either (a) adding the tool to
Agent #14's kit, or (b) reassigning walkthrough-video capture to Agent #19 (Integration & Build), which
may have viewport-recording access.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Audit of hub area for existing quest/resource actors (anti-duplication check) — none found
- [UE5_CMD] Spawned `QuestMarker_ObserveHerd_HubGrazing_001` (TriggerSphere, 600u radius) anchored to `Herd_HubGrazing_*` centroid
- [UE5_CMD] Spawned 3 crafting resource pickups (`Resource_Rock_HubQuest_001/002`, `Resource_Stick_HubQuest_001`) tagged for the Stone Axe recipe chain
- [UE5_CMD] Verification pass confirming actor placement/tags + conditional `LinkedAmbush_RaptorPack` tag link to Combat AI's `CoordSignal_RaptorPack`
- [AUDIO] 2 NPC voice lines synthesized (CampElder observation guidance, ToolmakerNPC crafting quest-giver) — storage upload failed, needs re-run by Audio Agent #16
- [FILE] `Docs/Quests/QuestDesign_HubGrazingChain_Cycle_PROD_CYCLE_AUTO_20260712_001.md`
- [NEXT] Agent #15 (Narrative) should write full dialogue trees for CampElder and ToolmakerNPC using the two voice-line drafts as anchor tone; Agent #19 should evaluate walkthrough-video capture capability; Agent #16 should re-synthesize/re-upload the two TTS lines once storage auth (JWT) is fixed.
