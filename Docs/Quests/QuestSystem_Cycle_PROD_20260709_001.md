# Quest & Mission Design — Cycle PROD_CYCLE_AUTO_20260709_001

**Agent:** #14 Quest & Mission Designer
**Bridge status:** HEALTHY — 3/3 ue5_execute Python calls completed cleanly this cycle.

> NOTE: Per absolute studio rule `hugo_no_cpp_h_v2`, this agent does NOT write .cpp/.h files.
> This headless UE5 instance never recompiles C++ (pre-built binary, 218 UHT errors on record),
> so all engine-side changes are made via `ue5_execute` (Python/Remote Control) against the
> already-running `MinPlayableMap`. This document records the concrete, in-editor changes made
> this cycle plus the design rationale.

## What was placed in MinPlayableMap this cycle

Three concrete, playable quest trigger actors (Engine `TriggerBox`, scale 3x, tag
`QuestTrigger`), positioned near the mandated content hub (X=2100, Y=2400) so they read
against the "living Cretaceous forest" hero composition rather than floating in empty space:

| Actor Label | Location (X,Y,Z) | Quest Type | Design Intent |
|---|---|---|---|
| `Quest_HuntTarget_TrikeArea` | 2300, 2200, 100 | Hunt | Track & ambush a Triceratops in the eastern clearing. Emotional beat: patience and respect for a dangerous herbivore, not just a damage-sponge kill. |
| `Quest_GatherMaterials_ForestEdge` | 1900, 2600, 100 | Gather | Collect hardwood + flint at the forest edge for advanced tool crafting. Ties survival crafting progression to spatial exploration. |
| `Quest_DefendCamp_NightWatch` | 2100, 2100, 100 | Defend | Predators stalk the camp perimeter at dusk; player reinforces palisade and stands watch. Emotional beat: dread building toward a controlled confrontation, not a random ambush. |

Verification pass confirmed all 3 actors exist in-level with correct tags before save.
Level was saved via `unreal.EditorLevelLibrary.save_current_level()`.

## Quest Design Rationale (Sasko/Blow lens)

- **Hunt quest** is deliberately NOT "walk up and kill". It requires downwind approach and
  leg-targeting — the mechanic *is* the statement: this animal is not prey to be trivialized,
  it is a territorial adult that punishes carelessness.
- **Gather quest** exists to make crafting (Agent #14's parallel P9 responsibility) feel
  earned rather than menu-driven — the player must physically go to a location with its own
  environmental risk (forest edge, closer to predator territory).
- **Defend quest** is a time-pressure survival beat: claw marks found in the morning is
  evidence-based tension (a documentary-realistic clue), escalating to an active night defense.

## NPC Quest-Giver Voice Lines (generated this cycle via ElevenLabs)

Two voice lines recorded for the quest-giver NPCs referenced above. Audio was generated
successfully by the TTS service; the Supabase storage upload step returned a JWT/auth error
on the platform side (`403 Invalid Compact JWS`) — this is an infrastructure issue on the
storage bridge, not a content issue. Scripts are preserved below for regeneration once storage
auth is fixed.

1. **QuestGiver_Hunter** (triggers `Quest_HuntTarget_TrikeArea`):
   > "The Triceratops has been grazing near the eastern clearing for three days now. If we're
   > going to hunt it, we need to move before it joins the larger herd. Bring your spear, and
   > stay downwind — one wrong step and it'll charge."

2. **QuestGiver_CampElder** (triggers `Quest_DefendCamp_NightWatch`):
   > "Something's been circling the camp after dark — I found claw marks on the palisade this
   > morning. We need more hands on watch tonight, and someone who isn't afraid to use fire
   > against whatever's out there."

No mystical/spiritual framing used — both lines are grounded, practical, evidence-based
dialogue consistent with the anti-hallucination rule for this project.

## Quest Walkthrough Video

`heygen_create_video` is listed in this agent's mandate but is NOT present in this agent's
available tool set for this session — cannot be executed. Flagging as a blocked deliverable
for the next cycle or for whichever agent has HeyGen access.

## Next Agent (#15 Narrative & Dialogue Agent) — Handoff

- Quest trigger actors are live in `MinPlayableMap` at the hub, tagged `QuestTrigger`, ready
  for narrative dialogue trees to hook into (via Blueprint/Remote Control, not new C++).
- Two quest-giver voice lines are scripted and voiced — Narrative agent should formalize
  these NPCs' names/backstory and expand branching dialogue around the Hunt/Gather/Defend
  quests above.
- Recommend Narrative agent avoid introducing new named locations that don't yet exist in the
  level; anchor dialogue to the 3 existing trigger coordinates first.
