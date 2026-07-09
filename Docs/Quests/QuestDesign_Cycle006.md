
# Quest & Mission Design — Cycle PROD_CYCLE_AUTO_20260709_006 (Agent #14)

## Bridge Status
HEALTHY — 3/3 `ue5_execute` Python calls completed cleanly (3.0–9.2s each), zero timeouts.

## Quests Implemented This Cycle

All quests are implemented as `TriggerSphere` actors placed in the live `MinPlayableMap`, tagged with a
`QuestType_*` / `QuestTarget_*` vocabulary so NPC Behavior (#11), Combat AI (#12) and Narrative (#15) can
read state without duplicating systems (per `hugo_naming_dedup_v2`).

### 1. `Quest_TrackingHunt_Raptor_001` — Hunting Quest
- **Type:** `QuestType_Hunt` / `QuestTarget_Raptor`
- **Placement:** Positioned at the nearest existing Raptor actor's location (reused actor, no duplicate spawned).
- **Radius:** 400 units
- **Design intent (Sasko-style emotional arc):** The player isn't just told "kill raptor." The quest giver
  (Hunter Elder) frames it as a tracking challenge — read broken branches, fresh prints, wait for a lone target
  to separate from the pack. Success/failure states should hook into Combat AI's existing `PackHunter` tag from #12.
- **Failure state:** If player triggers while 2+ raptors with `PackHunter` tag are within range, quest should
  escalate to "ambushed" rather than silently fail — this is the dramatic beat.

### 2. `Quest_HerdObservation_001` — Exploration/Observation Quest
- **Type:** `QuestType_Observation` / `QuestTarget_Herd`
- **Placement:** Near the herd cluster formed by #13 this cycle (`HerdMember_Grazing01` tagged actors), offset
  -200/-200 units so the trigger sits at the observation vantage point, not inside the herd itself.
- **Radius:** 600 units
- **Design intent:** A "do no harm" quest — the objective is explicitly to NOT disturb the herd (stay downwind,
  move slowly). This is a deliberate contrast to the hunting quest: same mechanical skillset (stealth, patience)
  but the payoff is observation/knowledge (mapping migration routes) rather than a kill. Blow's "mechanic as
  statement" — the game is telling the player that not every encounter with a dangerous animal needs to end in
  violence.

### 3. `Quest_ResourceGathering_Flint_001` — Crafting Material Quest
- **Type:** `QuestType_Gathering` / `QuestTarget_FlintDeposit`
- **Placement:** Hub-adjacent (2400, 2700), simple resource-run quest for early-game material collection feeding
  into #14's own future crafting-recipe chain (stone tools).
- **Radius:** 350 units

### 4. `Quest_CampDefense_001` — Defense Quest
- **Type:** `QuestType_Defense` / `QuestTarget_CampPerimeter`
- **Placement:** Directly at hub center (2100, 2400) — the PlayerStart clearing itself is the thing being defended.
- **Radius:** 500 units
- **Design intent:** Reads Combat AI's `CombatState_*` / `PackHunter` / `AI_Perception_Sight_*` tags (source of
  truth remains #12/#13 — this quest does not spawn a parallel predator-detection system). When tagged predator
  actors enter the perimeter, this becomes a wave-defense beat protecting the camp the player has been building.

## Quest Vocabulary (for #15 Narrative & #11 NPC Behavior to consume)
```
QuestType_Hunt | QuestType_Observation | QuestType_Gathering | QuestType_Defense
QuestTarget_Raptor | QuestTarget_Herd | QuestTarget_FlintDeposit | QuestTarget_CampPerimeter
```

## NPC Voice Lines Generated (ElevenLabs TTS)
1. **HunterElder_QuestGiver** — briefing for the Tracking Hunt quest (raptor pack tactics, ~16s).
   > "You want to track that raptor pack? Careful — they hunt in groups of three, and they're smarter than
   > they look. Follow the broken branches and fresh tracks along the ridge. Don't rush in. Watch, wait, and
   > strike when only one shows itself."
2. **CampForager_QuestGiver** — briefing for the Resource Gathering quest (~17s).
   > "The herd's been grazing near the flint ridge for three days now. If we can gather enough flint while
   > they're calm, we won't have to risk another trip once the dry season starts. Just move slow, stay downwind,
   > and don't spook the Triceratops."

**NOTE:** Both TTS generations succeeded at the ElevenLabs API level (audio synthesized, ~16-17s each) but the
Supabase Storage upload step failed with `403 Unauthorized — Invalid Compact JWS` (expired/invalid storage
token on the pipeline side, not an agent-side issue). Raw base64 MP3 payloads were returned in the tool output;
re-upload should be retried once the storage auth token is refreshed. Flagging for #01/#19 infra attention.

## Quest Walkthrough Concept Art
Requested a 4-panel storyboard image (tracking → observation → gathering → camp defense) via `generate_image`,
HD 1792x1024, National Geographic documentary style. **Generation succeeded** on the image-model side but hit
the same Supabase Storage `403 Invalid Compact JWS` upload failure as the TTS calls above — this is an
infrastructure-wide storage auth issue this cycle, not isolated to one asset type. Recommend #19 check the
Supabase service-role JWT expiry.

## Dependencies for Next Agent (#15 — Narrative & Dialogue Agent)
- 4 live quest triggers exist in `MinPlayableMap`, fully tagged, ready for narrative framing/dialogue trees.
- 2 NPC quest-giver voice lines scripted and synthesized (Hunter Elder, Camp Forager) — need narrative
  identity/backstory attached to these two NPC roles.
- Quest vocabulary (`QuestType_*`/`QuestTarget_*`) is the shared contract — Narrative should write dialogue
  branches keyed to these tags, not invent new ones.
- Storage/upload infra issue (403 Invalid Compact JWS) affects TTS and image assets studio-wide this cycle —
  flag to #01/#19 for infra fix before relying on generated audio/image URLs.
