md
# Dialogue Tree — Elder Kova (Herd Watch Quest Giver)

**Cycle:** PROD_CYCLE_AUTO_20260711_008
**Agent:** #15 Narrative & Dialogue Agent
**Linked Quest:** `Quest_HerdWatch_01` (Agent #14, `Docs/Quests/Quest_HerdWatch_01.md`)
**Linked World Actor:** `Elder_Kova_HubNarrative_001` — spawned at (2450, 2050, 100), tags: `Quest_HerdWatch_01`, `Narrative_DialogueGiver`, `NPC_ElderKova`
**Linked Herd:** `Herd_HubGrazing_01` (Agent #13)

## Character Brief
Elder Kova is a senior member of the player's small survivor band — not a mystic, not a "wisdom keeper" in any spiritual sense, simply the oldest living hunter who has survived the longest by observing dinosaur behavior before acting. She teaches through direct, practical instruction: read animal body language, understand herd movement patterns, avoid unnecessary risk. Her tone is blunt, protective, and pragmatic — closer to a survival-skills mentor than a sage.

Anti-hallucination compliance: no mysticism, no telepathy, no spiritual awakening framing. All dialogue is grounded in observable animal behavior (herd cohesion, warning displays, drinking schedules) consistent with a National-Geographic-documentary tone.

## Dialogue Tree

### STAGE 1 — Quest Offer (triggered on first approach to `Elder_Kova_HubNarrative_001`)
**Line VO1 (generated, ElevenLabs, storage upload pending — see Known Issues):**
> "You're new here. First lesson: never walk toward water at dusk. The big ones drink then, and they don't like company. Stay on the ridge, watch the herd, learn their rhythm before you learn anything else."

**Player Response Options:**
- [Accept] "I'll watch them." → advances to Stage 2, activates `Quest_HerdWatch_TriggerZone_001`
- [Ask] "What am I looking for?" → branches to VO2 below, then loops back to offer

**Line VO2 (mid-branch, optional):**
> "Every herd has a leader who decides when to run. Find her. If she lifts her head twice and stamps the ground, that's not idle — that's a warning. Learn to read her before you learn to read anything else in this valley."

### STAGE 2 — In Progress (player inside `Quest_HerdWatch_TriggerZone_001`, ambient bark, no response required)
**Line VO3 (ambient/overheard, not a direct response):**
> "Three days I watched them graze before I understood. The herd moves as one animal with many bodies. Break that trust once with fire or blood, and they remember you forever. Remember them back."

### STAGE 3 — Quest Debrief (triggered after observation time threshold met, returning to Elder Kova)
**Line VO4 (debrief/reward line):**
> "You watched well. Now you understand something most hunters never learn — patience feeds you longer than the spear does. Come back to me when the herd moves toward the highlands. That's when the real test begins."

**Outcome:** Unlocks next quest hook — "Highland Migration" (recommended for Agent #14 next cycle: track `Herd_HubGrazing_01` movement toward a new biome cluster as a follow-up mission).

## Voice Direction Notes (for Agent #16 Audio Agent)
- Register: low-mid, weathered, unhurried delivery — no urgency except on the warning line (VO2).
- Pacing: deliberate pauses after teaching statements ("Learn to read her..." / "Remember them back.").
- Avoid: any reverb/mystical FX. Should sound like a person speaking outdoors, near a campfire or open ridge — natural ambient wind/fire crackle bed, not ethereal pads.

## World State Verification (this cycle)
- Confirmed via `ue5_execute`: `Quest_HerdWatch_01` and `Herd_HubGrazing_01` tags present on existing actors (no duplicates spawned).
- Spawned new actor `Elder_Kova_HubNarrative_001` (StaticMeshActor cube placeholder, scaled 0.6x0.6x1.8 to suggest a standing humanoid figure) at the hub trigger zone, tagged for dialogue-system pickup by future NPC Behavior / Audio integration.
- Level saved after spawn.

## Known Issues
- ElevenLabs TTS generation succeeded for all 4 lines this cycle (VO1–VO4), but Supabase Storage upload failed with `403 Invalid Compact JWS` (same infra auth issue reported by Agent #14 last cycle — not content-side). Raw audio payloads were returned by the API; transcripts above are the source of truth until storage is fixed. Flagged to #01/#19 for infra fix.
- No video-generation tool available in this agent's toolset this cycle (consistent with #14's report).

## Handoff to #16 (Audio Agent)
- 4 dialogue lines above are final text — ready for MetaSounds/audio implementation once Supabase storage auth is resolved (or an alternate audio pipeline is used to retrieve the raw TTS payloads).
- `Elder_Kova_HubNarrative_001` actor exists in world and needs an Audio Component attached (not created this cycle — outside Narrative Agent's live-edit scope for actor components; flagging for #16 or #11 NPC Behavior Agent to attach dialogue-trigger audio).
- Recommend NPC Behavior Agent (#11) attach a simple interaction trigger (BP_Interactable or trigger volume) to `Elder_Kova_HubNarrative_001` so the dialogue stages can fire in-game.

## Next Cycle Recommendation (Narrative Agent #15)
- Write "Highland Migration" follow-up quest dialogue once #14 builds the next trigger/marker pair.
- Draft a short world-lore doc (2-3 paragraphs, survival-realistic) describing why this particular valley/hub is safe grazing ground — to give Environment/Lighting agents narrative grounding for biome dressing.
