
# Narrative & Dialogue — Cycle 006 (Agent #15)

## Context
Building directly on Quest Designer #14's Cycle 006 work: the "Track the Herd" objective marker at (2500, 2100) and the crafting resource cluster (rocks, sticks, leaves) near the content hub (2100, 2400).

## In-world actors spawned this cycle
| Actor | Location | Tags | Purpose |
|---|---|---|---|
| `TribeElder_Hub_001` | (2050, 2350, 100) | `NPC_Dialogue`, `Role_TribeElder`, `QuestGiver_FirstTools` | Dialogue anchor standing beside the crafting resource cluster. Gives the "First Tools" tutorial quest context — explains why the player needs a Stone Axe and Campfire before heading out. |
| `LoreMarker_HerdTrail_001` | (2480, 2130, 100) | `Lore_Trigger`, `Quest_TrackHerd` | Flavor/lore trigger placed right at Quest #14's cyan objective marker — fires tracker narration when player reaches the herd trail. |

Both actors reuse existing coordinates/context from #13 (herd location) and #14 (quest marker, resource cluster) — no duplicate herds or quest markers were created, per naming-dedup rule.

## Dialogue lines written (voice generated via ElevenLabs TTS this cycle)

### TribeElder — "First Tools" quest giver (~41s line)
> "You there. Yes, you — the one still standing after the crossing. Listen well, I won't repeat myself twice. See those rocks and sticks scattered near the fire pit? Two stones bound to a stick makes an axe sharp enough to split bone. Three dry sticks stacked right will keep the cold and the night-things away. Before you chase that herd east toward the grazing ground, arm yourself. A raptor doesn't care how brave you feel with empty hands. Gather what you need here, then track the herd's trail — their prints will lead you to water, and where there's water, there's always something watching from the reeds."

Tone: pragmatic, urgent, survival-first. No mysticism — an elder who has survived long enough to be blunt about danger.

### Tracker Narration — "Track the Herd" ambient lore trigger (~21s line)
> "Fresh tracks. Deep ones — a full herd, moving slow, grazing as they go. Follow the broken branches and the flattened grass. Stay downwind, stay low, and never step between a mother and her young. If the ground shakes twice and stops, that means they've caught your scent. Run for the treeline, not the open plain."

Tone: field-craft instruction disguised as narration — teaches the player the herd-tracking mechanic diegetically instead of via UI tooltip.

Note: Both audio files generated successfully via ElevenLabs (41s + 21s runtime) but Supabase upload returned a storage auth error (403 Invalid Compact JWS) — raw audio payload was generated and is available in the tool output for Audio Agent #16 to re-upload/attach to MetaSounds once storage auth is fixed.

## Verification
Confirmed in live `MinPlayableMap` via Remote Control:
- `TribeElder_Hub_001` spawned at (2050, 2350, 100) with tags `NPC_Dialogue`, `Role_TribeElder`, `QuestGiver_FirstTools`
- `LoreMarker_HerdTrail_001` spawned at (2480, 2130, 100) with tags `Lore_Trigger`, `Quest_TrackHerd`
- `TribeScout_Hub_001` (from Cycle 003) still present, no duplicates created
- Level saved successfully

## Dependencies for next agents
- **Audio Agent (#16)**: Re-upload the two generated voice lines (storage 403 error blocked auto-upload) and wire them into MetaSounds triggered by proximity to `TribeElder_Hub_001` and `LoreMarker_HerdTrail_001`.
- **Core Systems (#03)**: `CraftingSystem.cpp` should read `QuestGiver_FirstTools` tag to unlock the "First Tools" quest state when player first interacts with `TribeElder_Hub_001`.
- **NPC Behavior (#11)**: `TribeElder_Hub_001` currently static (cone placeholder) — needs idle behavior tree once character art (#09) delivers a MetaHuman mesh.
- **UI Agent**: Trigger subtitle display for both dialogue lines using the `NPC_Dialogue` / `Lore_Trigger` tags for proximity detection.

## Decisions & justification
- No `.cpp`/`.h` written — per hard rule, C++ is inert in this headless editor. All narrative logic delivered as in-world tagged actors + design doc + voice assets.
- Reused herd/quest coordinates from #13/#14 instead of creating duplicate markers, per naming-dedup rule.
- Dialogue content strictly survival-pragmatic (tools, danger, tracking) — zero mysticism/spiritual framing per anti-hallucination rule.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Spawned `TribeElder_Hub_001` (cone placeholder, tagged NPC_Dialogue/Role_TribeElder/QuestGiver_FirstTools) at crafting resource cluster
- [UE5_CMD] Spawned `LoreMarker_HerdTrail_001` (plane trigger, tagged Lore_Trigger/Quest_TrackHerd) at herd objective marker
- [UE5_CMD] Verification pass confirming both actors + existing TribeScout present, no duplicates, level saved
- [VOICE] Generated 2 dialogue lines via ElevenLabs TTS: TribeElder quest-giver line (~41s), Tracker narration lore line (~21s)
- [FILE] `Docs/Narrative/Cycle_006_DialogueAndLore.md` — full dialogue text, tags, and dependency notes
- [NEXT] Audio Agent (#16) should re-upload the two voice lines (Supabase 403 blocked auto-storage) and wire into MetaSounds via proximity triggers on the tagged actors
