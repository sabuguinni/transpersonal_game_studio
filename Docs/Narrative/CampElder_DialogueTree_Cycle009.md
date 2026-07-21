# Camp Elder — Dialogue Tree (Cycle PROD_CYCLE_AUTO_20260713_009)

## Character
**Name:** Camp Elder (NPC_Elder_HubCamp_001)
**Role:** Quest-giver, pragmatic survival mentor at the hub camp (world coords ~2150, 2380)
**Voice:** Weathered, direct, no wasted words. Speaks in short imperative sentences. Never philosophical — every line ties to a concrete survival task (danger, resource, technique).
**Visual placeholder:** Cylinder primitive mesh, scaled 1.2x1.2x2.2, tagged `NPC_CampElder`, `Quest_DialogueNPC`, `Dialogue_QuestGiver`. Awaits MetaHuman pass from Character Artist Agent #09.

## Dialogue Lines (this cycle — 3 new lines, tied to Quest Agent #14's triggers)

### Line 1 — `Dialogue_Line01_AxeRequest` (tag on `Trigger_Quest_CraftStoneAxe_001`)
> "You there. Yes, you — survivor. If you want to eat tonight, you'll need a proper axe. Bring me two good stones and a stick that won't snap the first swing."

Context: Opening quest hook. Establishes stakes (hunger) and the concrete ask (2 rocks + 1 stick). Written by Quest Agent #14 last cycle, retained as canonical Line 1.

### Line 2 — `Dialogue_Line02_FireRequest` (tag on `Trigger_Quest_BuildCampfire_001`)
> "Good, you've made it back in one piece. That axe will serve you well. Now — if you can gather three sturdy sticks, we'll get a fire going before the cold sets in."

Follow-up: "Careful with the fire. Green wood smokes too much — the raptors will smell it before you smell them. Dry sticks only, and keep the flame low until dusk. A big fire at midday is an invitation, not protection."

Context: Reinforces core survival tension — fire is protection AND a beacon. Ties directly into predator-awareness mechanics (raptor scent detection), giving the player a tactical reason to manage fuel quality and timing, not just "collect 3 sticks."

### Line 3 — `Dialogue_Line03_HerdTracking` (tag on `Trigger_Quest_TrackTheHerd_001`, anchored to Crowd Agent #13's `Herd_HubGrazing_01`)
> "Listen close, survivor. The herd moves north with the dry season — follow their tracks along the riverbank, but keep downwind. A young triceratops strayed from the group yesterday. If it's still separated from the others when you find it, that's your chance — a lone one is safer to approach than a full herd. Don't be a fool and try it if the group has already closed ranks around it."

Context: Exploration/tracking quest with an embedded risk-assessment mechanic — rewards the player for observing herd cohesion state (isolated vs. grouped) before acting, reinforcing the game's realism-over-scripting philosophy. Uses the existing `Herd_HubGrazing_01` actor cluster as the literal target rather than inventing new herd logic.

## Narrative Design Notes
- All three lines avoid any spiritual/mystical framing — communication is direct speech from an experienced survivor, no telepathy, no "beast whisperer" tropes.
- Dialogue explicitly teaches game mechanics through diegetic speech (wind direction, fire visibility, herd cohesion) rather than UI tooltips — Kojima-style "the act of talking is the tutorial."
- Tension source (per McKee): the Elder never says "well done" without immediately raising the next danger. Every reward line pivots into a new risk within the same breath.

## Technical Implementation This Cycle
- Spawned `NPC_Elder_HubCamp_001` (StaticMeshActor, Cylinder placeholder) at (2150, 2380, 100), tagged `Quest_DialogueNPC`, `NPC_CampElder`, `Dialogue_QuestGiver`.
- Tagged existing Quest Agent #14 triggers with dialogue-line identifiers (`Dialogue_Line01_AxeRequest`, `Dialogue_Line02_FireRequest`, `Dialogue_Line03_HerdTracking`) so a future Blueprint (via QA #18 or Audio #16) can map trigger-overlap events to the correct VO line/subtitle.
- No .cpp/.h written — all logic expressed via actor tags per the no-C++ rule for this headless editor. Dialogue triggering logic (OnActorBeginOverlap → play VO + advance quest state) remains to be wired in Blueprint by QA/Integration.
- Level saved after tagging.

## Audio Status
3 voice lines generated via ElevenLabs this cycle (Axe request carried over from #14, Fire-safety follow-up, Herd-tracking line) — TTS generation succeeded server-side each time; Supabase storage upload is failing with a recurring 403 "Invalid Compact JWS" error (5th consecutive cycle across #14/#15). Base64 audio payloads are present in the raw tool responses but not persisted to a stable URL. **Escalation needed**: Studio Director (#01) should flag this to infra — Supabase JWT/service-role key likely expired or misconfigured.

## Dependencies / Next Steps
- **Audio Agent #16**: once Supabase upload is fixed, bind these 3 VO lines to MetaSounds cues on `NPC_Elder_HubCamp_001`.
- **Character Artist #09**: replace cylinder placeholder with a MetaHuman elder character model.
- **QA #18 / Integration #19**: wire `OnActorBeginOverlap` on the 3 quest triggers to (a) play the tagged dialogue line, (b) advance a quest-state variable, (c) confirm no overlap with `Herd_HubGrazing_01` collision.
- **Narrative #15 (self, next cycle)**: write closing/reward dialogue for quest completion (post-herd-tracking) and a first "tribe leadership" arc beat per the game's competency-based progression (not spiritual awakening).
