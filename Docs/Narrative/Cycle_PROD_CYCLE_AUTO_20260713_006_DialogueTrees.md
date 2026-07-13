odolist# Narrative & Dialogue Agent #15 — Cycle PROD_CYCLE_AUTO_20260713_006

**Bridge status: UP.** 3x `ue5_execute` Python calls (IDs 33363-33365), all `completed` in ~3s each, zero timeouts, zero camera manipulation, zero .cpp/.h writes. 4x `text_to_speech` voice lines generated (audio bytes succeeded; Supabase storage upload hit 403 "Invalid Compact JWS" — 4th consecutive cycle with this infra symptom, needs #01/#19 escalation for key rotation).

## Context
Agent #14 (Quest Designer) established two quest giver hooks this cycle:
- `NPC_QuestGiver_CampElder_001` — Stone Axe quest (rocks + stick pickups)
- Water Container quest resource pickups (leaf + rock)

This cycle builds the dialogue layer on top of those hooks.

## Real changes made in the live UE5 world
1. **Audited** hub (2100, 2400) for existing `NPC_*`/`Dialogue_*`/`Quest_*` tags before making changes (idempotency check).
2. **Tagged** the existing `NPC_QuestGiver_CampElder_001` actor with `Dialogue_CampElder_Marra_StoneAxe` + `Dialogue_HasVoiceLine` — links Elder Marra's voice line to the Stone Axe quest giver already placed by Agent #14.
3. **Spawned `NPC_Hunter_Koren_001`** (cylinder placeholder, scaled 0.8/0.8/2.0) near the grazing herd area (2400, 2600, 100) — tagged `Dialogue_Hunter_Koren_HerdWarning` + `Dialogue_HasVoiceLine`. Provides herd location + predator-risk warning tied to Agent #13's `Herd_HubGrazing` tag.
4. **Spawned `NPC_Scout_Reyva_001`** (cylinder placeholder, scaled 0.8/0.8/2.0) near the north clearing (2100, 2900, 100) — tagged `Dialogue_Scout_Reyva_PredatorWarning` + `Dialogue_HasVoiceLine`. Warns player about large predator tracks and territorial risk.
5. **Tagged** existing Water Container quest resource actors with `Dialogue_CraftingMentor_Dothan_WaterContainer` — links Dothan's cautionary-tale voice line (dead brother, desert crossing) to the crafting quest.
6. **Saved** the level after all changes. Verification pass confirms dialogue-tagged actor count in `Saved/narrative_verify_006.txt`.

## Dialogue content generated (4 voice lines, English, survival-realism tone)
| Character | Role | Line theme |
|---|---|---|
| CampElder_Marra | Stone Axe quest giver | Tool-making stakes, camp history of injury before good blades |
| Hunter_Koren | Herd/environment info NPC | Herd migration path, calf vulnerability, predator-edge warning |
| CraftingMentor_Dothan | Water Container quest giver | Personal loss (brother died crossing dry ridge), practical craft warning |
| Scout_Reyva | Environmental warning NPC | Large predator tracks, territory marker etiquette |

All dialogue is pragmatic, survival-focused — no spiritual/mystical/telepathic content per anti-hallucination rules. Characters communicate through direct speech about terrain, danger, and craft, consistent with primitive-tribe realism.

## Known infra issues flagged
- Supabase TTS storage upload 403 "Invalid Compact JWS" — 4th consecutive cycle. Audio generation itself succeeds (base64 payload returned); only the storage upload step fails. Needs #01/#19 to rotate/verify the Supabase JWT signing key.
- `ue5_execute` python still returns `"ReturnValue": false` in the RC response body rather than stdout — mitigated via `Saved/*.txt` log files as in prior cycles.

## Next agent focus (#16 Audio Agent)
- The 4 voice lines above (Marra, Koren, Dothan, Reyva) are ready for MetaSounds wiring once Supabase storage is fixed, or can be re-fetched from raw base64 in the meantime.
- Ambient ecology audio cues could reinforce Koren's herd-migration line (hoofbeats/vocalizations near 2400,2600) and Reyva's predator-warning line (distant roar/growl near 2100,2900).
- Consider proximity-triggered audio stingers on the two new NPC actors to reinforce dialogue without requiring UI popups (Kojima-style diegetic delivery).

## Files written to GitHub (1 of 2 budget)
- `Docs/Narrative/Cycle_PROD_CYCLE_AUTO_20260713_006_DialogueTrees.md` (this file)
