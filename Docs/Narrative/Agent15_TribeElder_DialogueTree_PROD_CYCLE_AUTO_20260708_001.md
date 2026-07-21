# Tribe Elder — Dialogue Tree (Agent #15 Narrative & Dialogue)

**Cycle:** PROD_CYCLE_AUTO_20260708_001
**Bridge status:** HEALTHY — all 3 `ue5_execute` Python calls completed successfully (actor discovery → tag write → verification).
**Anchor actor:** `QuestGiver_TribeElder` (reused from Agent #14's quest trigger zones — no duplicate actor spawned, per dedup rule).
**Location:** Content hub area near X=2100, Y=2400 (Herd_HubGrazing cluster).

## Purpose
This dialogue tree gives the Tribe Elder NPC actual voiced lines mapped to the 3 quest trigger zones Agent #14 built this cycle:
- `Quest_HerdTracking_TriggerZone_001`
- `Quest_RaptorSnareDefense_TriggerZone_001`
- `Quest_CampDeliveryZone_TriggerZone_001`

Tone follows the anti-hallucination narrative rules: pragmatic, survival-focused, no spiritual/mystical language. The Elder is a practical tribal leader — a hunter who survived long enough to teach, not a "wisdom keeper" or spirit guide.

## Dialogue Nodes (tagged on `QuestGiver_TribeElder` actor via UE5 Python this cycle)

### 1. `Dialogue_Intro` (first encounter)
> "The valley remembers everything. Every kill, every desperate sprint through the ferns, every night huddled against the cold with the fires low. I have buried three companions since the last rains. I will not bury a fourth. Listen to me now, and listen well — the raptors hunt in threes, and if you see one, the other two are already closer than you think."

Establishes stakes (survival, loss) and immediately teaches a gameplay-relevant fact (raptor pack behavior — ties directly into Agent #12/#13's combat/herd AI).

### 2. `Dialogue_QuestBrief_HerdTracking` (accept trigger: `Quest_HerdTracking_TriggerZone_001`)
> "Hunter. The herd grazes near the eastern clearing, calm for now. Your task: approach downwind, count the young..."

### 3. `Dialogue_QuestComplete_HerdTracking` (complete trigger)
> "You came back. Not everyone does. Sit, eat something before you talk — I can see the shake in your hands from here. Now tell me slowly: how many were in the pack, and which direction did they run when they lost your trail?"

Transitions the player from the observation quest directly into raising the stakes for the raptor defense quest (mentions "pack" and "trail" as a narrative bridge).

### 4. `Dialogue_QuestBrief_RaptorDefense` (accept trigger: `Quest_RaptorSnareDefense_TriggerZone_001`)
> "Don't run from the big ones in open ground — you cannot outrun a Rex, but you can outlast one if you use the rocks. Save your spear thrusts for when it commits to the charge. One clean strike behind the foreleg, then get out of the way. That is how my father survived to teach me this, and how I survive to teach you."

Doubles as an implicit combat tutorial (positioning, timing, weak-point targeting) for Agent #12's Combat AI systems.

### 5. `Dialogue_QuestComplete_CampDelivery` (complete trigger: `Quest_CampDeliveryZone_TriggerZone_001`)
> "This camp was built on the bones of those who came before. Every wall, every fire pit, every trap line at the tree edge — someone bled to learn where to put it. Add your own knowledge to it, hunter. That is the only way our people last another generation in this valley."

Closes the 3-quest arc with a thematic payoff: the player's survival knowledge literally becomes part of the camp's defenses, seeding future quest content (base-building, teaching new NPCs).

## UE5 Implementation (this cycle)
- Verified bridge health via `EditorLevelLibrary.get_editor_world()` + actor discovery (found `QuestGiver_TribeElder` tagged actor from Agent #14).
- Attached 5 new `FName` tags to the actor (`Dialogue_Intro`, `Dialogue_QuestBrief_HerdTracking`, `Dialogue_QuestComplete_HerdTracking`, `Dialogue_QuestBrief_RaptorDefense`, `Dialogue_QuestComplete_CampDelivery`) via direct Python actor.tags manipulation — no .cpp/.h written, per `hugo_no_cpp_h_v2`.
- Saved level (`EditorLevelLibrary.save_current_level()`).
- Ran a second verification pass listing all actors carrying `Dialogue_*` or `QuestGiver_TribeElder` tags to confirm persistence.
- No camera changes made, per `hugo_no_camera_v2`.

## Voice Generation
Generated 4 ElevenLabs voice lines this cycle (Intro, Combat Tutorial/RaptorDefense brief, HerdTracking completion, CampDelivery completion) matching the tags above. **Known infra issue (recurring across Agents #14 and #15 this cycle): Supabase upload returns `403 Invalid Compact JWS`** — audio generation succeeds at the ElevenLabs API level but storage upload fails. Flagged for Agent #19/Hugo — this is an infrastructure/auth issue, not a content or narrative-design blocker.

## Next steps for Agent #16 (Audio Agent)
- Once Supabase JWS auth is fixed, re-run TTS generation for the 5 tagged dialogue lines above (Intro line was also generated this cycle) and bind resulting audio assets to the `Dialogue_*` tags on `QuestGiver_TribeElder` via MetaSounds.
- Consider ambient ducking when any `Dialogue_*` tag is actively playing (lower herd/ambient SFX volume during Elder lines).
