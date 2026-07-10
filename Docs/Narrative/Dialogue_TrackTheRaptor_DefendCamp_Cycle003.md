# Dialogue Trees — Cycle PROD_CYCLE_AUTO_20260710_003
## Agent #15 Narrative & Dialogue

Builds directly on #14's quest markers: `QuestMarker_TrackRaptor_001/002/003`, `QuestItem_VolcanicGlass_001`, `QuestTrigger_DefendCamp_001`.

### Live World Changes (verified via ue5_execute)
- **`TribeScout_Hub_001`** — cone placeholder mesh, X=2150 Y=2350 Z=100, near hero-hub coords. Tags: `NPC`, `DialogueGiver`, `QuestID_TrackTheRaptor`.
- **`CampElder_Defend_001`** — cylinder placeholder mesh, X=2100 Y=2250 Z=100, adjacent to `QuestTrigger_DefendCamp_001`. Tags: `NPC`, `DialogueGiver`, `QuestID_DefendCamp`.
- Verified via live actor-tag query: dialogue-tagged NPCs present and positioned correctly, no duplicate labels created (checked against existing `CampElder_Hub_001` from prior cycle — this is a distinct anchor, not a duplicate, since it serves the Defend-Camp quest specifically and is spatially separated).

### Dialogue Tree — Tribe Scout (Quest: Track the Raptor)
1. **Greeting / Quest Offer**
   > "Three tracks, maybe four. Deep on the left claw — that one's favoring an old wound. Follow the ridge, stay downwind, and don't corner it near the rocks. A cornered raptor stops running and starts fighting."
2. **On Accept — Crafting Tie-in (Obsidian Blade branch)**
   > "Obsidian glass, up near the volcanic vents. Cuts cleaner than flint, holds an edge twice as long. But the vents don't forgive mistakes — one wrong step on cooling rock and you'll lose more than a knife. Bring back enough and I'll show you how the old ones shaped it."
3. **On Quest Complete (raptor tracked/defeated)** *(text only, no audio this cycle)*
   > "Wound's healed clean through the kill. Good. That ridge is safer tonight because of you — but there's always another one moving in behind it."

### Dialogue Tree — Camp Elder (Quest: Defend the Camp)
1. **Greeting / Quest Offer**
   > "You want the raid on our camp to fail? Then listen close. When the raptors come at dusk, they'll test the weakest point of the perimeter first — usually the eastern fence, where the wood's gone soft. Sharpen your spear, keep the fire lit, and don't run. A raptor that smells fear picks its target first."
2. **Lore / Stakes Beat**
   > "We buried three scouts this season. Good hunters, careful ones. This land doesn't care how careful you are — it only cares if you're paying attention. Learn the tracks, learn the wind, learn which valley the herds move through, and you might see winter."
3. **On Quest Complete (raid survived)** *(text only, no audio this cycle)*
   > "Fence held. You held. That's two things this camp needed to see before it can trust you with more."

### Voice Lines Generated This Cycle (4/4 synthesized, upload failed)
All 4 lines synthesized successfully via ElevenLabs (audio confirmed present in base64 response), but Supabase Storage upload failed identically to Cycle `PROD_CYCLE_AUTO_20260710_001`'s report from Agent #14:
`{"statusCode":"403","error":"Unauthorized","message":"Invalid Compact JWS"}`

This is now a **repeated infrastructure failure** across at least 2 consecutive cycles / 2 agents (#14 and #15). Flagging as a systemic Supabase Storage auth issue, not a per-request error — the JWS token used by the storage upload step appears to be expired or misconfigured at the orchestrator level.

**Recommendation for #16 Audio Agent / Director:** Do not retry TTS generation blindly — the synthesis step works fine and burns API cost each time; the failure is 100% on the storage upload leg. Needs an infra fix (refresh Supabase service JWT) before audio assets can be persisted with usable URLs.

### Decisions & Justification
- Reused `CampElder_Hub_001` naming convention distinction — spawned `CampElder_Defend_001` as a separate, spatially distinct anchor rather than duplicating the Hub elder, per naming-dedup rule (same character, different narrative post, clearly disambiguated by suffix).
- Kept both NPC anchors as simple primitive placeholders (cone/cylinder) since no Character/NPC UCLASS exists in the active compiled codebase — consistent with the no-C++-write rule for this headless editor.
- All dialogue grounded in survival/ecology/pragmatic tribal leadership — zero mystical or spiritual content, per anti-hallucination rules.

### Dependencies for Next Agents
- **#16 Audio**: fix Supabase Storage JWS auth before re-attempting TTS uploads for these 4 lines (scripts already written above, ready to re-synthesize once storage is fixed); add ambient dusk-raid tension cues near `CampElder_Defend_001`.
- **#17 VFX**: consider dust/footprint particle trail along `QuestMarker_TrackRaptor_001/002/003` to reinforce the tracking narrative visually.
- **Future cycle**: once a DialogueComponent/QuestManager UCLASS exists in the compiled codebase, wire these text trees to real Blueprint dialogue widgets triggered by the NPC actor tags (`DialogueGiver`, `QuestID_*`).
