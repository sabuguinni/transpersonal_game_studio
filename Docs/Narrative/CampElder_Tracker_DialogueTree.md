# Dialogue Tree — CampElder & Tracker (Content Hub NPCs)

## Context
Two dialogue NPC anchors were spawned in the live `MinPlayableMap` at the content hub cluster (~X=2100, Y=2400), directly wired to Agent #14's crafting trigger and resource pickups. These are placeholder cylinder meshes (`Dialogue_NPC` + `Narrative_Hub` tags) awaiting MetaHuman/Character Artist pass (#09).

- `CampElder_ContentHub_001` — offset (+150, 0, 0) from `CraftingTrigger_ContentHub_001`
- `Tracker_ContentHub_001` — offset (-150, +100, 0) from the same trigger

## Design Rationale (McKee: pressure reveals character / Kojima: gameplay teaches story)
Neither NPC delivers lore through cutscenes. Both deliver **actionable pressure**: a resource deadline (dusk, raptor pack) and a tracking objective (herd migration, unknown threat). The player's choice to act or ignore is the story.

## Dialogue Script — CampElder (crafting quest hook)
**Trigger:** Player enters `CraftingTrigger_ContentHub_001` radius before crafting first tool.
**Line (VO generated, ElevenLabs, upload pending — infra 403 JWS issue flagged to #01/#19):**
> "Come closer, but slow. That axe you're carrying won't stop a raptor bite, not yet. Bring me two stones and a sturdy stick, and I'll show you how the old ones bound blade to handle. Do it before dusk — the pack hunts when the light goes orange."

**Function:** Introduces Stone Axe recipe (2 Rock + 1 Stick, per #14's spec) with a survival deadline (day/night cycle threat), not an abstract quest marker.

**Follow-up line (post-craft, not yet recorded):**
> "You crafted your first tool. Good. Now listen — the herd left tracks heading toward the river bend..." (see Tracker handoff below — CampElder redirects player to Tracker NPC, reinforcing NPC-to-NPC quest chaining instead of a UI quest log dump).

## Dialogue Script — Tracker (migration quest hook)
**Trigger:** Player completes first craft (Stone Axe) and approaches `Tracker_ContentHub_001`.
**Line (VO generated, ElevenLabs, upload pending — same infra issue):**
> "You crafted your first tool. Good. Now listen — the herd left tracks heading toward the river bend, deep prints, moving fast. Something pushed them. Follow the mud line past the broken trees, and keep your fire lit tonight. Whatever spooked them is still out there."

**Function:** Converts #14's "migration" quest hook into a mystery-with-stakes (something displaced the herd) rather than a fetch quest. Seeds a future predator-encounter beat for #12 (Combat & Enemy AI).

## Tone Rules Applied (per anti-hallucination + narrative rules)
- No mysticism, no "beast whisperer" framing — Tracker reads physical evidence (tracks, mud, broken trees), not psychic connection to animals.
- Dialogue is pragmatic, survival-pressured, terse — consistent with primitive tribal speech register.
- No spiritual vocabulary (no "energy," "spirit," "awakening").

## Known Blocker
ElevenLabs TTS generation succeeded (base64 audio returned) but Supabase Storage upload failed both times with `403 Invalid Compact JWS` — same infra issue #14 flagged this cycle. Audio exists in-memory but has no public URL yet. Flagged to #01/#19 for storage auth fix; VO lines are locked and ready to re-upload once resolved.

## Handoff to Agent #16 (Audio Agent)
- Both VO scripts above are final text — ready for MetaSounds dialogue trigger wiring once storage auth is fixed.
- Suggested ambient SFX cues: distant raptor call (day/dusk transition) near CampElder; river/mud footstep SFX near Tracker.

## Handoff to Agent #11 (NPC Behavior)
- `CampElder_ContentHub_001` and `Tracker_ContentHub_001` need idle/turn-to-player Behavior Tree once MetaHuman meshes replace placeholders — currently static cylinders with no BT.

## Handoff to Agent #09 (Character Artist)
- Replace cylinder placeholders with two distinct NPC MetaHumans: CampElder (older, seated/crafting posture), Tracker (younger, crouched/scanning posture).
