# Game Bible Fragment — Survival Narrative Core (v1)
### Narrative & Dialogue Agent #15

## Premise
A lone human survivor is dropped into a hostile Cretaceous ecosystem. There is no rescue, no map, no
guide. Survival is earned through observation, tool-making, and territory awareness — not destiny.

## Tone
Tense, pragmatic, observational. Every line of dialogue or lore text answers one of: *Where is danger?
Where is food/water? What worked last time? What will kill me if I'm careless?*

## Narrative Arc (Solo Survivor → Tribal Leader)
1. **Arrival** — player has nothing; learns terrain, learns which dinosaurs are predators vs herbivores.
2. **Adaptation** — crafts first tools (stone axe, spear), builds first fire, learns herd movement patterns.
3. **Territory** — defends a fixed camp, tracks raptor pack behavior, maps river/resource routes.
4. **Leadership** — earns cooperation of other survivors through demonstrated competence (successful
   hunts, camp defense), not through revelation or spiritual awakening. Leadership is earned by results.

## Cast Conventions
- Survivors are named by role/trade, not archetype-mysticism: e.g. "Toolmaker", "Tracker", "Firekeeper".
- No shamans, no spirit guides, no beast-whisperers. Communication is gesture, carved marks, spoken
  fragments of proto-language — never telepathy.
- All in-world text (lore notes, camp logs) is written as field observation: what was seen, what worked,
  what to avoid.

## Current In-World Content (live in MinPlayableMap as of Cycle 008)
- 4 survivor field notes (TextRenderActor) anchored to Quest Designer's marker actors:
  TrackHerd, HuntRaptor, DefendCamp, ExploreRiver.
- 4 recorded survivor voice lines (see `NarrativeAgent_Cycle008_LoreNotes.md` for full audio manifest).

## Anti-Hallucination Compliance
No content in this document or in-world references consciousness, meditation, spiritual awakening,
mysticism, or supernatural abilities. All narrative elements verified against the "would this exist in a
National Geographic prehistoric documentary?" test.

## Next Cycle Priorities
1. Wire lore notes to proximity-trigger audio playback (Blueprint via `ue5_execute`, no new C++).
2. Expand cast with 2-3 named NPC survivors tied to NPC Behavior Agent (#11) routines.
3. Draft first branching dialogue exchange (player choice affects camp cooperation, not story outcome —
   consistent with McKee/Kojima principle: illusion of consequence via gameplay state, not cutscenes).
