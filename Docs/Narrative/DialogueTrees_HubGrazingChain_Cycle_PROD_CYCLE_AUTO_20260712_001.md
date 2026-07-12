# Narrative & Dialogue — Hub Grazing Chain
Agent #15 (Narrative & Dialogue) — Cycle PROD_CYCLE_AUTO_20260712_001

## Context
Builds directly on Agent #14's Quest Design (`QuestDesign_HubGrazingChain_Cycle_PROD_CYCLE_AUTO_20260712_001.md`):
- Quest 1 "Read the Herd" → `QuestMarker_ObserveHerd_HubGrazing_001`
- Quest 2 "Two Rocks, One Stick" → crafting resource pickups near herd zone
- Combat link → `CoordSignal_RaptorPack` (Agent #12), reused, not duplicated

## Live World Changes (verified via ue5_execute, command IDs 32293–32295)
1. **Audit** — Scanned hub (2100,2400) for existing `NPC_`, `Dialogue_`, `Lore_` tags. None found; new spawns authorized (no duplication).
2. **`NPCDialogue_CampElder_HubGrazing_001`** — TriggerSphere (300u), placed at (2050,2350,100), tags: `NPC_CampElder`, `Dialogue_ObserveHerd`, `Narrative_QuestGiver`. Anchors CampElder's dialogue to the observation quest.
3. **`NPCDialogue_ToolmakerNPC_HubGrazing_001`** — TriggerSphere (300u), placed at (2150,2450,100), tags: `NPC_Toolmaker`, `Dialogue_CraftingChain`, `Narrative_QuestGiver`. Anchors Toolmaker's dialogue to the crafting quest.
4. **`LoreSign_TribeOrigin_HubGrazing_001`** — TextRenderActor at (2100,2400,220), tag `Lore_TribeOrigin`, world-visible text establishing tribe backstory tied to herd migration — placed at the exact hero-screenshot centroid per content quality bar.
5. Level saved after mutation. Verification pass (32295) confirms all 3 actors exist with correct tags/locations.

## Dialogue Tree — Camp Elder (branches on player quest state)
- **Node A (pre-observation):** "You watched the herd break formation before the storm came. Most hunters never notice that. Tell me — what else did their movement teach you?"
- **Node B (post-observation, unlocks ambush warning):** "The raptors hunt in threes near the grazing hollow. One shows itself to draw your eye — the other two circle behind. Remember that, or you won't live to remember anything else."
- **Node C (quest-giver hook, ties to Quest 1):** "You've been watching the herd near the grazing hollow, haven't you? Good. Keep your distance — Triceratops don't forgive a bad approach."

## Dialogue Tree — Toolmaker NPC
- **Node A (quest-giver hook, ties to Quest 2):** "Two good rocks and a straight stick — that's all I need to bind you a proper axe. Bring them to the crafting stone."
- **Node B (crafting instruction, unlocked after pickup delivery):** "A blade this dull will snap on bone. Feel the edge — no, slower. That's it. Now you understand why my father made me do this a hundred times before he trusted me with fire."

## Intro Narration (world/tribe framing — no cutscene, delivered via ambient VO on level load)
"Before the rivers had names, our people learned to read the land as a hunter reads tracks in mud. This is the story of the last free tribe of the valley — and the price of surviving among giants."

## Tone & Content Compliance
All dialogue is pragmatic survival content: territory, danger, resource crafting, herd behavior, ambush tactics. Zero spiritual/mystical/telepathic framing — CampElder and Toolmaker are practical tribal specialists (elder hunter, craftsman), not "wisdom keepers" or "spirit guides." Communication is oral/gestural, consistent with anti-hallucination rules.

## Voice Production (this cycle)
4 lines synthesized via ElevenLabs (audio generation succeeded for all 4):
1. Narrator — intro cinematic framing (~14s)
2. Camp Elder — herd-reading dialogue branch (~11s)
3. Toolmaker — crafting instruction (~13s)
4. Camp Elder — raptor ambush warning (~13s)

**Known issue (recurring across cycles):** Supabase Storage upload fails with `403 Invalid Compact JWS` (JWT auth error) on every line this cycle and last cycle (#14's output). Audio is synthesized correctly but no public URL is produced. **Flagging for Agent #16 (Audio) and Agent #19 (Integration):** the Supabase storage credentials/JWT used by the `text_to_speech` tool need rotation/fix at the infrastructure level — this is blocking all narrative/quest VO delivery, not an agent-side error.

## Tooling Gap
No `heygen_create_video` tool was present in this session's toolset, so the mandated "narrative intro video" could not be produced. Documenting as a capability gap for Agent #01/#19 to resolve (either provision the tool or reassign video capture to a screen-capture pipeline in the editor).

## Next Agent Focus
- **Agent #16 (Audio):** Re-synthesize/re-upload the 4 VO lines once Supabase JWT is fixed; attach `NPCDialogue_*` trigger actors to MetaSound cue playback on player overlap.
- **Agent #11 (NPC Behavior):** Attach actual CampElder/Toolmaker NPC pawns to these dialogue trigger anchors (currently placeholder TriggerSpheres only).
- **Agent #19 (Integration):** Assess feasibility of an in-editor video capture workflow to replace the missing `heygen_create_video` tool.
