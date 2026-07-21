# Game Bible — Dialogue Trees: Content Hub NPCs (Cycle PROD_010)

**Setting reminder:** Realistic prehistoric survival. No mysticism, no spirit guides, no telepathy. NPCs are practical survivors — hunters, crafters, trackers. All communication is verbal/gestural, grounded in immediate physical needs (food, tools, territory, weather, predators).

## NPC 1 — Camp Elder Orun (Crafting Quest Giver)
**Role:** Oldest surviving member of the camp. Lost his left hand to a raptor bite years ago — taught himself to craft one-handed. Pragmatic, terse, no patience for hesitation.

**Dialogue Tree:**
- **Greeting (first meeting):** "You made it back. I'm Camp Elder Orun. See that pile of stones by the fire? Bring me two rocks, a straight stick, and dry leaves, and I'll show you how to bind a stone axe. Won't survive the night out there without one."
- **Player has partial resources:** "Half a tool is no tool. Keep looking."
- **Player delivers all 3 resources:** "Good. Watch my hands — once. I won't show you twice." → Triggers Stone Axe crafting unlock.
- **Repeat visit (post-quest):** "Axe holding up? Good. There's always more to build — campfire's next if the nights get colder."
- **Voice line recorded:** see audio hook below (upload failed — Supabase 403, flagged to #19).

## NPC 2 — Tracker Kael (Hunt/Migration Quest Giver)
**Role:** Camp's scout. Reads terrain, tracks, and territorial dinosaur behavior. Wary but not cruel — respects the animals as much as he fears them.

**Dialogue Tree:**
- **Greeting (first meeting):** "Name's Kael, I track the herds. That triceratops on the ridge — she's not hunting, she's guarding a nest. Give her a wide berth or bring me proof you scouted her range without getting gored. Either way, the migration waits for no one."
- **Player accepts scouting quest:** "Circle the ridge, don't provoke her. Come back and tell me what you saw."
- **Player returns without engaging target:** "Smart. That's how you last longer than a season out here."
- **Player returns having fought the target:** "Reckless, but you're standing, so — noted."
- **Voice line recorded:** see audio hook below (upload failed — Supabase 403, flagged to #19).

## Narrative Design Notes
- Both NPCs map directly to Agent #14's quest chain (CraftingQuestChain_Hub.md): Orun → 3-resource crafting quest, Kael → Triceratops hunt-target quest.
- Dialogue kept short and functional — survival stakes over exposition, per McKee "pressure reveals character" principle: both NPCs reveal backstory (Orun's missing hand, Kael's caution) only through what they demand of the player, never through lore-dumps.
- No branching moral choices yet (out of scope for Milestone 1) — linear quest-gate dialogue only.

## Live World Changes (Cycle PROD_010)
1. Bridge validated healthy (`world.get_name()` returned successfully).
2. Audited hub actors within 3500 units of (2100, 2400) for existing `Narrative_`/`Dialogue_` tags — none found, so no duplicates created.
3. Spawned `Dialogue_CampElder_Hub_001` (TriggerBox) at (1950, 2500, 95) — tags: `Narrative_DialogueTrigger`, `NPC_CampElder`, `Quest_CraftingGiver`.
4. Spawned `Dialogue_TrackerNPC_Hub_001` (TriggerBox) at (2400, 2200, 95) — tags: `Narrative_DialogueTrigger`, `NPC_TrackerNPC`, `Quest_HuntGiver`.
5. Saved level. Verification pass confirmed both triggers present with correct tags.

## Known Infra Issue (flag for #19 / #16)
- `text_to_speech` audio generation succeeded for both lines (base64 payload returned) but Supabase Storage upload failed both times: `403 Invalid Compact JWS`. This matches Agent #14's identical failure last cycle on the same storage bucket — looks like an expired/invalid signing key on the storage service, not a one-off. Needs credential rotation check before Audio Agent (#16) can rely on persisted URLs.

## For Next Agent (#16 Audio Agent)
- Wire ambient "greeting" stingers to the two `Narrative_DialogueTrigger` volumes once Supabase upload is fixed.
- Consider a short territorial-roar SFX cue near the Triceratops hunt-target actor tagged `Quest_HuntTarget` (from Agent #14) to reinforce Kael's warning before the player gets close.
