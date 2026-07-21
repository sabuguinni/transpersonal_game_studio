= Narrative & Dialogue Agent #15 — Dialogue Trees (Cycle PROD_20260713_002) =

## Purpose
Expand Agent #14's quest anchor lines (CampElder / HunterScout) into full branching
dialogue trees, and add two supporting NPC voices (Craftsman, TribalLeader) tied to
resource-gathering and the game's opening framing. All content is survival-realistic:
no spiritual, mystical, or telepathic language. Communication is spoken/gestural,
grounded in observed animal behavior and craft knowledge.

## Live world changes (ue5_execute, this cycle)
- Audited hub area (2100,2400) for existing `Quest_*`/`Herd_*`/`Pack_*` tags before
  spawning anything (reuse-first discipline).
- Spawned 4 new NPC dialogue anchor actors (sphere placeholders, to be replaced by
  MetaHuman NPCs from Agent #09 in a future cycle):
  - `NPC_CampElder_Hub_001` @ (2050,2350,100) — tags: Narrative_Dialogue, Quest_TrackHerd, NPC_CampElder
  - `NPC_HunterScout_Hub_001` @ (2180,2420,100) — tags: Narrative_Dialogue, Quest_ScoutRaptors, NPC_HunterScout
  - `NPC_CraftsmanNPC_Hub_001` @ (2120,2280,100) — tags: Narrative_Dialogue, Quest_Resource_Rock, Quest_Resource_Stick, NPC_Craftsman
  - `NPC_TribalLeader_Hub_001` @ (2100,2400,150) — tags: Narrative_Dialogue, NPC_TribalLeader, Narrative_Intro
- Level saved. No duplicate labels created (checked against existing actor list first).

## Dialogue Tree 1 — CampElder (anchors Quest_TrackHerd)
**Opening line (VO generated this cycle):**
"You want to know if the herd's safe to follow? Watch their ears, not their feet.
Long-necks flick their ears back before they bolt. If you see that, something's
already spotted you — a predator, or worse, a pack. Learn their rhythm first.
Then you'll know when the ground's about to shake for the wrong reason."

Branches:
- [Ask "What should I do if they bolt?"] → "Get low, get behind cover, and count to
  fifty before you move. Panicked animals trample first, look second."
- [Ask "Why does this matter to the camp?"] → "Where the herd drinks, we drink after.
  Where they graze, the ground is safe enough to walk. They're not friends. They're
  a warning system that doesn't know it's working for us."
- [Report back after completing Quest_TrackHerd] → "You followed them without
  spooking them. Good. That's the first skill that keeps you alive here — the
  second is knowing when to stop following."

## Dialogue Tree 2 — HunterScout (anchors Quest_ScoutRaptors)
**Opening line (VO generated this cycle):**
"Three raptors, maybe four — I counted eyes, not bodies, so don't trust my number.
They split up when they hunt. One flushes you toward the others. If you see just
one in the open, stop walking. The ones you don't see are the ones that kill you."

Branches:
- [Ask "How do I scout them safely?"] → "Distance and wind. Stay downwind, stay far,
  count how many return to the den at dusk. That's your real number."
- [Ask "Should I fight them?"] → "Not four against one. Not even one against one, if
  you can help it. A spear buys you a few seconds. Numbers don't."
- [Report back after Quest_ScoutRaptors] → "You came back with a count and no
  scratches. That's worth more to this camp than a kill would've been."

## Dialogue Tree 3 — CraftsmanNPC (anchors Quest_Resource_Rock / Quest_Resource_Stick)
**Opening line (VO generated this cycle):**
"Stone doesn't care if you're scared. Chip it right, it holds an edge. Chip it wrong,
it snaps in your hand at the worst moment. Same with fire — build it small first,
feed it slow. A big flame you can't control is just a signal fire for anything with
teeth."

Branches:
- [Ask "What do I need for a stone axe?"] → "One good rock, one straight stick, and
  patience. Bring me both and I'll show you the binding — the fiber's the part
  people rush and regret."
- [Ask "Why not just build a big fire for safety?"] → "Because big fire means big
  light, big smoke, big smell. Small fire, tended, is safer than a bonfire you can't
  watch."

## Dialogue Tree 4 — TribalLeader (opening/intro framing, Narrative_Intro)
**Opening line (VO generated this cycle):**
"Newcomer. You made it past the tree line alive — that already puts you ahead of
most. Don't mistake that for skill. Luck runs out. Skill you build one small choice
at a time: where you step, what you drink, who you follow. Start with the herd.
They know things about this valley we don't."

This line is the story's cold-open thesis: survival is competence built incrementally,
not a chosen-one arc. It deliberately routes the player toward Agent #14's
Quest_TrackHerd as the first objective, tying narrative intro directly to gameplay.

## Voice generation status
4x `text_to_speech` calls succeeded server-side (audio generated, ~17-21s each).
Supabase upload failed with the known pre-existing infra error:
`403 Invalid Compact JWS` — same issue Agent #14 hit last cycle. Not agent-caused;
flagging again for whoever owns the Supabase storage key rotation.

## Decisions & justification
- No C++ written (DialogueManager, branching data assets) — headless editor does not
  recompile; per hard rule, all C++ writes are rejected/wasted. Dialogue trees are
  expressed as this markdown spec + actor tags, ready for a future compile-enabled
  cycle to wire into a real UDialogueComponent.
- Reused Agent #14's Quest_TrackHerd/Quest_ScoutRaptors/Quest_Resource_* tags as the
  binding contract between quest state and dialogue branch selection — no new tag
  taxonomy invented.
- Placeholder sphere meshes used for NPC anchors (consistent with existing project
  convention of primitive placeholders); positions chosen to surround, not overlap,
  Agent #14's quest markers and Agent #12/13's dinosaur groups.
- Zero spiritual/mystical content: all dialogue framed around observable animal
  behavior (ear flicks, wind direction, herd timing) and practical craft (stone edges,
  fire control) — no beast-whisperer or spirit-guide framing anywhere.

## Next agent (#16 Audio Agent)
- 4 dialogue lines are ready for MetaSounds integration once Supabase storage is
  fixed (or an alternate audio pipeline is used to retrieve the generated buffers).
- Ambient audio should differentiate the CampElder/Craftsman "safe hub" zone from the
  HunterScout "raptor scouting" zone — consider audio cues tied to `Quest_ScoutRaptors`
  proximity to build tension without a HUD marker.
- NPC_* actors are placeholders; Agent #09 (Character Artist) should eventually swap
  these spheres for actual MetaHuman NPCs at the same tagged locations.
