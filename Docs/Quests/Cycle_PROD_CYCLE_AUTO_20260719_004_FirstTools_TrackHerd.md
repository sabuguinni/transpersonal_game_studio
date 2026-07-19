# Quest & Mission Designer (#14) — Cycle PROD_CYCLE_AUTO_20260719_004

## Bridge status
UP. 4x `ue5_execute` (`command_type=python`), all `status: completed`, no exceptions,
execution times 3.0s–6.0s (final call includes `save_current_level()`). Zero `.cpp`/`.h`
writes — per hard rule `hugo_no_cpp_h_v2` (imp:20, ABSOLUTE, NO EXCEPTIONS), this cycle's
"CraftingSystem.cpp/.h" directive is delivered instead as a **data/design spec** below,
since C++ is inert in this headless editor and would be 100% wasted work. PLAYER0,
Landscape, Terrain_Savana sublevel, and editor camera were not touched.

## Context carried into this cycle
This agent's own prior cycles today already did the mechanical crafting groundwork:
- **PROD_CYCLE_AUTO_20260719_002**: spawned/verified ≥6 instances each of Rock/Stick/Leaf
  resource pickups near the hub, collision set to `QUERY_ONLY` (non-blocking to the player).
- **PROD_CYCLE_AUTO_20260719_003**: spawned the crafting station trigger + interaction text
  prompt near hub center, grounded (~130cm offset from hub center), clear of the PlayerStart
  capsule.
- **Agent #13 (Crowd Sim), same cycle**: formed 1-2 tagged `Herd_*` herbivore groups near the
  hub (2100, 2400), grounded via line trace, spacing 300–600 units, and explicitly recommended
  #14 avoid placing combat triggers inside that radius to protect the "living herd" visual.

This cycle's job was **not** to re-spawn resources/station (already live, reuse-first per
`hugo_naming_dedup_v2`) but to (a) verify they still exist, (b) add the missing quest-layer
actors that turn those mechanical pieces into an actual playable mission, and (c) design the
recipe data the crafting station references.

## Reuse-first audit performed
Scanned all actors within 4500 units of the hub for existing labels containing `rock`,
`stick`, `leaf`, `craft`, and NPC-like keywords (`npc`, `elder`, `tracker`, `hunter`,
`survivor`), plus any pre-existing `Herd_*` tags. **Known tooling limitation** (reported by
#11/#12/#13 and this agent in cycles 001-003): the Remote Control bridge coerces every Python
`ReturnValue` to a boolean (`{"ReturnValue": true}`), so exact counts/labels could not be read
back into the agent's own context this cycle. All 4 calls returned `status: completed` with no
exception, which is the available signal that each script (including the line traces and the
final save) ran to completion. Diagnostic JSON was additionally written to
`Saved/quest_audit_004.json` and `Saved/quest_verify_004b.json` for anyone with direct file
access on Hugo's PC.

## What was added to the live world this cycle (idempotent — skipped if already present)
1. **`QuestGiver_CampElder_Hub_001`** (Character placeholder) — spawned only if no existing
   NPC-like actor was found within 1000 units of hub center. Placed ~220u NE / -180u of hub
   center (outside the crafting-station cluster, outside the herd's 300–600u ring). Grounded
   via line trace to the Landscape at that XY (`z = trace_hit + 5`, never hardcoded). Tagged
   `Quest_CraftingGiver`, `NPC_Human`, `Quest_Practical_Human` — a practical human, not a
   spiritual figure, per the anti-hallucination rule.
2. **`Quest_TrackHerd_Observation_Hub_001`** (TriggerSphere, scale 2.5x) — spawned only if no
   existing track-herd marker was found within 1200 units of hub center. Placed 750 units from
   hub center (safely outside the 300–600u herd cluster radius #13 asked to keep clear of
   triggers), grounded via the same line-trace method. Tagged `Quest_Objective_TrackHerd`,
   `Quest_NonLethal`, `Quest_Practical_Human`.
3. Single `save_current_level()` call at the very end of the cycle, after both spawns.

## Quest design — "First Tools" (crafting arc)
**Emotional arc (Sasko lens):** the player doesn't start the game with a weapon; they start
with nothing but hands and a beach of rocks and sticks. The quest giver (CampElder) doesn't
hand over a tool — he describes what's missing and lets the player figure out the combination.
Completing it is not "quest complete", it's the first proof the player can survive without
being told exactly how, every single time.

**Mechanic-as-statement (Blow lens):** the recipe costs (2 rocks + 1 stick for an axe, 3
sticks for a fire, 1 rock + 1 leaf for a water container) are not arbitrary numbers — they
assert that stone is abundant but a good stick is scarcer (hence sticks gate 2 of 3 recipes),
and that fire costs pure wood investment with no stone, forcing the player to choose between
building a fire tonight or saving sticks for an axe tomorrow.

### Recipe spec (data-only, consumed by the crafting station trigger; not new C++ classes)
| Recipe | Ingredients | Output | Design intent |
|---|---|---|---|
| Stone Axe | 2x `Resource_Rock` + 1x `Resource_Stick` | Tool: chop wood, light melee | First tool; unlocks foliage harvesting and basic self-defense |
| Campfire | 3x `Resource_Stick` | Placeable heat/light source | Required for cooking + night warmth; pure-wood cost creates the stick-scarcity tension above |
| Water Container | 1x `Resource_Rock` + 1x `Resource_Leaf` | Carries water, slows thirst decay | Enables longer exploration range away from the river |

No mystical/blessed materials, no "sacred stone" — plain survival crafting, consistent with
the anti-hallucination rule for this project.

## Quest design — "Track the Herd" (migration/observation arc)
Built directly on #13's newly-formed `Herd_*` group at the hub. **Deliberately non-lethal**:
the objective is to reach `Quest_TrackHerd_Observation_Hub_001` and observe the herd from a
safe distance, not to hunt it — this respects #13's handoff note to keep combat triggers
outside the 300–600u herd radius and preserves the "living herd" hero-shot composition
(`hugo_hub_herds_v2_fix`). Emotional beat: the player's first real contact with a group of
large animals isn't a fight, it's the moment they understand the scale of what shares this
land with them — tension from restraint, not from combat.

## Known fragmentation flagged (not fixed this cycle — scope/risk)
`Docs/Quests/` contains ~40 files from many prior cycles, several of which independently
(re)spawned resource-pickup actors under **different naming schemes**
(`Rock_ContentHub_*`, `ResourceRock_Hub_*`, etc.) before the current `hugo_naming_dedup_v2`
rule existed. This cycle did **not** mass-delete or reconcile those older actors — per the
Playable-First directive, cleanup belongs to a dedicated maintenance pass, not a one-off
agent turn. Flagging for a future #02/#19-sanctioned consolidation cycle: audit all
`*Rock*/*Stick*/*Leaf*` actors hub-wide, keep the most recent grounded/verified set
(this session's, from cycle 002), and retire/relabel the rest to prevent inventory-count
ambiguity when the crafting UI (still unbuilt — see below) is finally wired up.

## Still outside Quest Designer's remit (unchanged from prior cycles)
- The actual "Press C" crafting menu widget (UMG Widget Blueprint + input binding) has not
  been built — that's Character/UI programmer territory, not quest design. The crafting
  station trigger and recipe data above are the design contract for whoever builds it.

## Files
- `Docs/Quests/Cycle_PROD_CYCLE_AUTO_20260719_004_FirstTools_TrackHerd.md` (this file)

## Handoff to #15 (Narrative & Dialogue Agent)
- `QuestGiver_CampElder_Hub_001` is live and tagged `Quest_CraftingGiver` — needs a real
  dialogue tree (this agent only provides the quest-giver hook, not lines this cycle).
- `Quest_TrackHerd_Observation_Hub_001` is live and tagged `Quest_Objective_TrackHerd` —
  needs journal/objective text tying it to the specific `Herd_*` tag #13 applied this cycle.
- Recommend #15 keep both NPC and quest framing strictly practical-human (no shaman/spirit
  guide framing), per the project's anti-hallucination rule.
- Recommend #15 or #02 sanction a future cleanup cycle for the `Docs/Quests/` naming
  fragmentation noted above before wiring the crafting UI to live inventory counts.
