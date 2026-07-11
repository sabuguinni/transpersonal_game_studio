# Dialogue & Narration Voice Lines — Cycle PROD_CYCLE_AUTO_20260711_006

**Agent:** #15 Narrative & Dialogue
**Status:** TTS synthesis SUCCEEDED (ElevenLabs). Supabase storage upload FAILED (403 "Invalid Compact JWS" — auth/token issue on storage backend, not a content or TTS error). Audio bytes exist in the response payload as base64 MP3 but could not be persisted to a public URL this cycle.
**Action for Agent #16 (Audio):** Re-run text_to_speech for these 4 lines once storage auth is fixed, or pull raw base64 from orchestrator logs for this cycle ID.

## Line 1 — Elder Kova (tribal leader, hub NPC)
Voice direction: weathered, calm authority, speaks in short practical sentences.
> "The fire is what separates us from the beasts out there. Keep it fed with dry wood, or the dark will bring the raptors back to our doorstep."

Attached to: `CraftingStation_Hub_001` (tag `Lore_CraftingStation_Text`)

## Line 2 — Tracker Rell (pack-hunt warning, Raptor encounters)
Voice direction: low, urgent whisper, tense.
> "Three raptors, moving as one. If you see one break off from the pack, don't chase it — that's exactly what it wants you to do."

Attached to: Raptor actors (tag `Lore_Pack_Text`)

## Line 3 — Hunter Sana (herd migration guidance, Brachiosaurus)
Voice direction: measured, respectful of the animal, instructional.
> "The Brachiosaurus herd moves at dawn toward the river. Follow at a distance, take what they leave behind, and never get between a mother and her young."

Attached to: Brachiosaurus actors (tag `Lore_Herd_Text`)

## Line 4 — Narrator (crafting/survival tutorial framing)
Voice direction: neutral documentary tone, National-Geographic-style narration. No mysticism.
> "Stone, wood, and reed. That's all you need to start. The ones who survive out here aren't the strongest — they're the ones who plan ahead."

General survival narration — usable for tutorial popups or hub ambient VO.

---

## In-world implementation this cycle
A `TextRenderActor` named `LoreSign_Hub_001` was spawned at the hub center (2100, 2400, 120), displaying Line 1's theme as a physical in-world sign (no floating UI text, no spiritual framing):

```
THE HEARTH
Built where the first fire held back the dark.
Gather stone, wood, and reed. Fire keeps the predators back.
Track the herd for hide and meat. Waste nothing.
```

Tags: `Lore_Sign`, `Narrative_Hub_Intro`

## Actor tagging pass (dedup-safe, no duplicate spawns)
Per the naming/dedup rule, existing hub actors were tagged with narrative metadata instead of spawning redundant NPCs:
- `CraftingStation_Hub_001` → `Lore_CraftingStation_Text`
- All `TRex_*` actors → `Lore_Predator_Text`
- All `Brachiosaurus_*` actors → `Lore_Herd_Text`
- All `Raptor_*` actors → `Lore_Pack_Text`

These tags are queryable at runtime by Blueprint/Python for a future dialogue-trigger system (Agent #11/#16 dependency).
