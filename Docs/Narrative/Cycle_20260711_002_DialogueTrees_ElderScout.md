# Dialogue Trees — Tribe Elder & Tribe Scout
**Cycle:** PROD_CYCLE_AUTO_20260711_002 | **Agent:** #15 Narrative & Dialogue
**Depends on:** Agent #14 quest markers `CraftingStation_Hub_001` (crafting mentor) and `QuestObjective_TrackHerd_001` (herd tracking), both near content hub (2100, 2400).

## Design Note
Both characters speak in blunt, practical survival language — no mysticism, no "wisdom keeper" framing. The Elder teaches through consequence (a loose axe head "takes an eye"), the Scout through observation discipline (herd behavior reads as danger signal). Dialogue is gated by quest state, not free-roam chatter, per Kojima principle: dialogue exists to make the player's next action legible, not to interrupt it.

---

## NPC: Tribe Elder — Crafting Mentor
**Actor tag applied:** `Dialogue_TribeElder`, `DialogueLine_01_AxeBinding`, `NPC_Role_CraftingMentor`
**Anchor:** `CraftingStation_Hub_001`

### Node 1 — First Approach (quest state: NotStarted)
> "So you want to learn the old ways. Good. Listen close — I won't repeat myself twice."

**Player options:**
- [Ask about the axe] → Node 2
- [Ask what's needed] → Node 3 (lists 2 rocks + 1 stick, references Agent #14's pickups)

### Node 2 — Binding Instruction (quest state: InProgress, materials in inventory)
> "A stone axe starts with the right rock: heavy, flat, one sharp edge. Strike it against the stick at an angle, not straight down, or you'll split the handle. Bind it tight with the sinew, three wraps, no more."

### Node 3 — Warning / Stakes (triggers before crafting confirmation)
> "A loose axe head kills faster than any raptor — it flies off mid-swing and takes an eye with it. Swing it once against the log there. If it holds, you're ready. If it doesn't, we start again with a new stick."

### Node 4 — Completion (quest state: Completed)
> "Good. That'll split kindling or crack bone. Don't let it dull on rock — that's what the whetstone's for."

**Voice line generated (Node 1+2+3 combined take):** `TribeElder_AxeBinding_01` — see production note below (storage pending).

---

## NPC: Tribe Scout — Tracking Guide
**Actor tag applied:** `Dialogue_TribeScout`, `DialogueLine_01_HerdReport`, `NPC_Role_TrackingGuide`
**Anchor:** `QuestObjective_TrackHerd_001` (reuses `Herd_HubGrazing_01`, no duplicate herd)

### Node 1 — Debrief Prompt (quest state: PlayerReturnedFromTracking)
> "You're back faster than I expected. Did you find the herd?"

### Node 2 — Report Request
> "Tell me exactly what you saw. How many young ones were walking at the center of the group? That's where the mothers keep them, away from the tree line. Count the adults on the outer edge too — those are the ones that'll charge if they smell you upwind."

### Node 3a — Branch: Calm Herd (player reports steady grazing)
> "If the herd's grazing calm, we can trail them another day before the rains come."

### Node 3b — Branch: Skittish Herd (player reports fast/bunched movement)
> "If they're skittish, moving fast, bunching tight — something's already hunting them, and we stay clear of that ridge entirely."

**Design intent:** this branch seeds a future predator-encounter quest hook for Agent #12 (Combat & Enemy AI) without triggering combat directly — pure narrative foreshadowing tied to observable herd behavior state.

---

## Production Notes
- 2 full voice takes generated server-side via ElevenLabs this cycle (`TribeElder` and `TribeScout` character names) — audio synthesis succeeded, but Supabase Storage upload failed with `403 Invalid Compact JWS` (same infra fault reported by Agent #14 last cycle — likely an expired/misconfigured storage JWT, not a content issue). Flagging for #01/Hugo: **storage auth key needs rotation**.
- 1 concept illustration (Elder teaching axe-binding by firelight, documentary-realistic, zero fantasy elements) also generated server-side but failed the same upload step.
- Live UE5 changes (verified via Remote Control): `CraftingStation_Hub_001` and `QuestObjective_TrackHerd_001` actors tagged with dialogue/role identifiers so Agent #16 (Audio) and future UI/dialogue-widget work can bind lines to the correct actor without new spawns — complies with naming/dedup rule (no new NPC actors created, existing quest markers reused as dialogue anchors).

## Dependencies for Next Agent
**#16 Audio Agent:** Once Supabase storage auth is fixed, re-run TTS for `TribeElder_AxeBinding_01` and `TribeScout_HerdReport_01` and bind resulting audio assets to the tagged actors (`Dialogue_TribeElder`, `Dialogue_TribeScout`) via MetaSounds. Concept art re-generation also pending same fix.
