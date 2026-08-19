# Field Notes — "The Cold Trail Above" (QuestChain_006_ColdTrailAbove)

Solo first-person field notes, written by the protagonist (a paleontologist) as he
follows physical evidence uphill from the Savana/Floresta biome toward the ridgeline
that eventually leads to Snowy Rockside. No NPCs, no dialogue, no quest-givers — the
trail is read entirely from terrain and dinosaur sign. Tags are already applied in
the live world (verified via ue5_execute this cycle) on real, pre-existing resource
actors — no new actors were spawned.

## Chain order (in-world, verified)

| Order | Actor (label) | Location | Step type | Tag |
|---|---|---|---|---|
| 01 | Stick_Savana_101 | Savana | Find | `Narrative_FieldNote_ColdAnomaly` |
| 02 | Rock_Floresta_403 | Floresta | Find | `Narrative_FieldNote_ColdTrailRockFace` |
| 03 | Rock_Floresta_401 | Floresta (2600,2250,765) | Climb | `Narrative_FieldNote_ClimbToRidgeline` |
| 04 | Rock_Floresta_402 | Floresta (2400,2050,930) | Confirm | `Narrative_FieldNote_FirstSnowConfirmed` |

All four carry `Narrative_SoloPlayerVoice`. Steps 03/04 additionally carry
`Narrative_Theme_WayHome`, since this is the first physical evidence in the game
that a colder biome — and therefore Snowy Rockside — exists above the tree line.

## Field Note text (in-world log entries, first person)

**01 — Stick_Savana_101 (Narrative_FieldNote_ColdAnomaly)**
> "Found a broken branch here, snapped clean, not chewed. The bark on the north
> face is bleached — frost damage, and we're nowhere near the pole. There's a
> temperature gradient on this hill that doesn't match the valley floor. Worth
> following up."

**02 — Rock_Floresta_403 (Narrative_FieldNote_ColdTrailRockFace)**
> "This rock face is fractured in bands — freeze-thaw cycling, repeated over
> years. That doesn't happen at this elevation unless something changes further
> up. I'm going to climb and see how far the cold reaches."

**03 — Rock_Floresta_401 (Narrative_FieldNote_ClimbToRidgeline)**
> "Air's dropped at least ten degrees in the last two hundred meters of climb.
> My breath is showing. Whatever is up there, it's not more of the same forest —
> the vegetation is already thinning out around me."

**04 — Rock_Floresta_402 (Narrative_FieldNote_FirstSnowConfirmed / Quest_Reward_ColdTrailRevealed)**
> "Snow. Actual snow, packed into the shaded side of this outcrop, this far
> south, in this heat. Which means somewhere past this ridge there's a highland
> cold enough to hold it year-round. If the second gem is where I think it is,
> this is the road there."

## Design notes for #14 / #19
- No new actors spawned. All four tags sit on pre-existing `Rock_Floresta_*` /
  `Stick_Savana_*` static mesh actors with real (non-BasicShapes) meshes.
- I inspected `Rock_Highlands_013` and `Rock_Highlands_015` as alternate
  candidates for steps 03/04, found them untagged and geographically plausible,
  but reverted the attempt once I discovered `Rock_Floresta_401`/`_402` already
  carried the canonical `QuestOrder_03`/`QuestOrder_04` + `QuestStep_Climb` /
  `QuestStep_Confirm` tags from the Survival Objectives Designer (#14). Keeping
  a single canonical actor per step avoids a duplicate/ambiguous quest chain.
- Dino_ actor count confirmed unchanged at 40, zero moved, per the absolute
  movement prohibition on creature actors.
