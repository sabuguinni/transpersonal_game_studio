
# Game Bible — Core Narrative Framework (v3, Cycle 008 update)

## Premise
A realistic Cretaceous-era survival game. The player is a solitary human survivor who must learn to read terrain, weather, and dinosaur behavior to stay alive, eventually earning a place within a settlement/tribe through demonstrated competence — not through any spiritual or mystical "awakening."

## Core Arc
**Solitary Survivor → Competent Tracker → Trusted Tribe Member → Settlement Leader**

Each stage is unlocked by observable, practical skill demonstrations (tracking, crafting, hunting, weather-reading), not narrative exposition or lore dumps. Mentor NPCs (like Kessa, the Tracker Elder) exist to teach and gate progression through dialogue tied directly to quest triggers already placed in the world.

## Tone
Tense, pragmatic, unsentimental. Characters speak in short, practical sentences. Wisdom is field-tested experience, not philosophy. No character refers to "energy," "spirits," "awakening," or any mystical concept — all guidance is grounded in observable survival mechanics (wind direction, herd timing, shadow/light reading, structural integrity of shelter, weapon maintenance).

## Recurring Mentor NPCs (established so far)
- **Kessa ("Tracker Elder")** — observation, herd behavior, terrain reading. Anchor: `NPC_TrackerElder_Hub01` (1700, 2500). Full bio in `NPC_TrackerElder_Kessa.md`.

## Quest Chain Status (cross-referenced with Agent #14)
1. **`track_herd_hub01`** ("Reading the Herd") — COMPLETE (3-stage TriggerBox chain, Cycle 008). Dialogue fully scripted (4 lines, this cycle).
2. **`craft_stone_axe_hub01`** — NEXT (flagged by Agent #14 for a future cycle). Narrative hook: Kessa should reappear as the quest giver to maintain mentor continuity rather than introducing a new "Crafting NPC" — avoids actor/character duplication anti-pattern.

## Open Narrative Debt (for future cycles)
- Kessa needs 2 additional lines for the crafting quest chain once Agent #14 stages it.
- A second mentor archetype (a hunting-focused NPC, tone: more aggressive/tactical) should be introduced only once the crafting chain is stable — avoid stacking new named characters before existing ones have full dialogue trees.
- 6 total voice lines (2 from Quest Designer + 4 from this cycle) are pending Supabase re-upload by Agent #16 — audio exists, only storage auth needs fixing.

## Hard Constraints (unchanged, restated for continuity)
- No meditation, consciousness, spirit guides, mysticism, or "awakening" narratives.
- No telepathic/telekinetic animal communication — Kessa reads physical sign (tracks, droppings, wind, grazing patterns), not minds.
- All lore must pass the test: "Would this appear in a National Geographic prehistoric documentary?"
