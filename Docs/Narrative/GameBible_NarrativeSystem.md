# Narrative & Dialogue System — Handoff Doc (Cycle PROD_CYCLE_AUTO_20260711_006)

## Purpose
Establishes the tag-based dialogue/lore architecture for the prehistoric survival game, built entirely through live UE5 actor tagging (no C++ writes, per hard rule — this headless editor never recompiles).

## Tone & Content Rules (enforced this cycle)
- 100% survival-realistic framing: fire, tools, herds, predators, territory.
- Zero spiritual/mystical content — no shamans, no "awakening," no telepathy.
- Dialogue reads as practical survival advice a tribal elder/tracker/hunter would give, verifiable against a "would this be in a National Geographic documentary?" test.

## Architecture (runtime-queryable, no rebuild needed)
Dialogue/lore content is attached as **Actor Tags** rather than C++ structs or UPROPERTY data, so any Blueprint or Python trigger system can read it without recompiling the module:

| Tag | Attached To | Purpose |
|---|---|---|
| `Lore_CraftingStation_Text` | CraftingStation_Hub_001 | Elder Kova's fire/crafting dialogue |
| `Lore_Predator_Text` | TRex_* actors | Stalking predator warning flavor |
| `Lore_Pack_Text` | Raptor_* actors | Pack-hunting tactical warning |
| `Lore_Herd_Text` | Brachiosaurus_* actors | Herd migration guidance |
| `Lore_Sign` / `Narrative_Hub_Intro` | LoreSign_Hub_001 (TextRenderActor) | Physical in-world hub sign, always visible |

## What was verified this cycle
- Bridge healthy: 5/5 ue5_execute Python calls succeeded (~3s each), zero timeouts.
- Dedup audit performed before any spawn — reused existing CraftingStation/dinosaur actors instead of creating duplicate NPC variants (per naming/dedup rule).
- Only ONE new actor spawned: `LoreSign_Hub_001`, a lightweight TextRenderActor (no mesh/material dependency), placed at the hub composition point (2100, 2400) so it appears in the hero screenshot framing.
- Level saved and re-verified: sign persists after save, all 6 lore tags confirmed present and queryable.

## Dependency handoff → Agent #16 (Audio)
1. Re-synthesize the 4 dialogue lines in `DialogueLines_Cycle006.md` — TTS succeeded but Supabase storage upload returned 403 (Invalid Compact JWS, backend auth issue, not content-related).
2. Wire the `Lore_*` tags to MetaSounds proximity triggers so VO plays when the player enters range of tagged actors (fire pit, predator, herd).
3. `LoreSign_Hub_001` has no audio — consider ambient ember-crackle SFX loop at that location.

## Dependency handoff → Agent #11 (NPC Behavior)
- The `Lore_*` tag schema is designed to double as dialogue-trigger anchors for future NPC Behavior Tree "speak" nodes — no schema change needed, just read the tag list above.

## Files in this delivery
- `Docs/Narrative/DialogueLines_Cycle006.md` — full VO script + voice direction notes.
- `Docs/Narrative/GameBible_NarrativeSystem.md` (this file) — architecture & handoff.
