
# Narrative & Dialogue — Cycle 007
Agent #15 — Narrative & Dialogue Agent

## Context
Builds directly on Agent #14's Cycle 007 quest triggers (`Quest_TrackHerdA_TrackingObjective_001`,
`Quest_ObserveHerdB_StealthObjective_001`, `Quest_GatherMaterials_HubObjective_001`) placed around
the content hub (world ~2100, 2400). This cycle adds voice lines and in-world journal text for
those three quest beats, keeping tone pragmatic/survival — no spiritual or mystical framing.

## Voice Lines Generated (ElevenLabs TTS)
1. **Tracker_QuestTrackHerdA** — tracking objective flavor line:
   "This ground is soft. Herd A passed here at dawn — three, maybe four animals, moving slow.
   Follow the broken stems, stay downwind, and don't let them see you first."
2. **HunterElder_QuestObserveHerdB** — stealth objective flavor line:
   "Don't move. That second herd near the ridge spooks easy. Watch them from the treeline,
   count how many young they have, and we'll know if it's safe to pass through their ground."
3. **Crafter_QuestGatherMaterials** — gathering objective flavor line:
   "Good stone for an axe head, dry sticks for the fire, broad leaves to carry water.
   Gather what the clearing gives you before the light goes. We don't waste a single trip."
4. **HunterElder_QuestRewardGather** — reward/completion line for the gathering quest:
   "You made it back with the stone and the sticks. That's a fire tonight, and an axe by morning.
   One more trip like that and this camp starts to look like it might survive."

Note: TTS audio generated successfully (11–12s clips each) but Supabase Storage upload returned
`403 Invalid Compact JWS` on all 4 uploads — a storage-auth issue on the platform side, not a
content issue. Raw MP3 payloads were returned inline (base64) by the TTS tool; scripts above are
final and ready for re-upload once storage auth is fixed. Flagging for #01/orchestrator infra fix.

## UE5 World Changes (ue5_execute, both calls healthy, 3.0s each)
1. Bridge validation — confirmed `EditorLevelLibrary.get_editor_world()` resolves.
2. Spawned 3 `TextRenderActor` journal markers, one above each of Agent #14's quest triggers
   (offset +220 Z), each displaying short in-world objective text matching the voice line content:
   - `Narr_JournalText_TrackHerdA_TrackingObjective_001` → "TRACK: Follow Herd A's trail. Stay downwind."
   - `Narr_JournalText_ObserveHerdB_StealthObjective_001` → "OBSERVE: Watch Herd B from the treeline. Do not approach."
   - `Narr_JournalText_GatherMaterials_HubObjective_001` → "GATHER: Collect stone, sticks, and leaves before dark."
   Text color set to warm parchment tone (255,220,150), world size 28, level saved after spawn.

## Compliance
- No new C++ (.h/.cpp) written — per hard Brain rule `hugo_no_cpp_h_v2` (imp 20), all changes done
  via `ue5_execute` python (TextRenderActor is a native engine class, zero custom code needed).
- No duplicate quest/herd/dino actors created — text markers reference Agent #14's existing
  trigger actors by label lookup instead of respawning geometry.
- No camera moves.
- No spiritual/mystical content — dialogue is strictly survival-pragmatic (tracking, stealth,
  resource gathering, camp survival).

## Next Agent Focus
- **#16 Audio Agent**: wire the 4 voice lines above into MetaSounds cues triggered by the 3 quest
  TriggerBox overlaps (once storage-upload auth is fixed, re-run TTS or re-upload cached audio).
- **#3 Core Systems**: bind quest trigger overlap events to actually surface the journal text /
  play the audio cues in-game (currently static world-space text, not yet event-driven).
- **#18 QA**: verify `Narr_JournalText_*` markers are legible from typical player approach angles
  and don't clip into terrain/foliage.
