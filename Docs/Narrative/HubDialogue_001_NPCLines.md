
# Hub Dialogue 001 — Quest-Giver Lines & NPC Markers

**Agent:** #15 Narrative & Dialogue Agent
**Cycle:** PROD_CYCLE_AUTO_20260712_006
**Bridge status:** UP — 2/2 `ue5_execute` Python calls succeeded (command IDs 32619, 32620), zero timeouts, zero camera manipulation, zero .cpp/.h writes.

## Context
Builds directly on Quest & Mission Designer Agent #14's handoff (`Docs/Quests/HubQuestArc_001_HuntRescueGather.md`), which delivered 3 live `TriggerBox` volumes tagged `Quest_HubHunt_01`, `Quest_HubRescue_01`, `Quest_HubGather_01` but **no visible NPC actors** — only invisible triggers. This cycle fills that gap with dialogue content and lightweight NPC position markers.

## Live world changes (verified via ue5_execute)
1. **Audit (cmd 32619)** — Scanned all level actors for existing `Quest_`/`Herd_`/`Trike`/`Raptor`/`TRex` tags and labels before spawning anything, per anti-duplication rule. Confirmed no pre-existing `Narr_*` dialogue actors at the hub.
2. **Spawn (cmd 32620)** — Placed 3 `TargetPoint` actors as NPC dialogue anchors, offset +150/+150 from each existing quest trigger (so future Character Artist / NPC Behavior agents can replace them 1:1 with real Skeletal Mesh NPCs at the exact narrative-approved position):
   - `Narr_HubHunt_ElderNPC_001` — tags: `Narr_Dialogue`, `Quest_HubHunt_01`, `NPC_HuntElder`
   - `Narr_HubRescue_SisterNPC_001` — tags: `Narr_Dialogue`, `Quest_HubRescue_01`, `NPC_WorriedSister`
   - `Narr_HubGather_ElderNPC_001` — tags: `Narr_Dialogue`, `Quest_HubGather_01`, `NPC_CampElder`
3. Level saved after spawn.

**Note on naming rule:** Used `Narr_` prefix per project convention for narrative-owned actors, distinct from Type_Bioma_NNN convention which applies to environment/creature actors (trees, dinosaurs). These are functional markers, not duplicate creature/prop spawns — no existing actor of this kind existed at these coordinates.

## Dialogue lines (text_to_speech — 4x generated this cycle)
All 4 lines synthesized successfully (audio returned as base64 MP3). **Supabase public URL upload failed with `403 Invalid Compact JWS`** for all 4 — same storage auth issue flagged by Quest Designer Agent #14 last cycle. Raw audio is available in tool output but not persisted to a public URL. Flagged again for #16 Audio Agent / infra owner to fix Supabase JWT signing.

### 1. Hunt Elder — Quest "Track the Herd" briefing
> "You want to know what happened to the herd? They shifted east at first light, spooked by something big moving through the ferns. Follow the flattened grass past the split rock — that's where the trail goes cold. Move quiet, or you'll be the one being tracked."

### 2. Worried Sister — Quest "Dry Riverbed" briefing
> "Please. My brother went for flint by the dry riverbed two nights ago and never came back. I found his water skin torn open near the boulders. If you find him — or find what took him — come tell me first. Don't be a hero out there."

### 3. Camp Elder — Quest "Flint by the Rocks" briefing
> "Flint's scarce this close to camp — the good veins are near the rocks past the ridge, but that ground belongs to something else now. Take what you need and leave quickly. A blade's no use to a dead man."

### 4. Narrator — Hub ambient intro line (world tone-setter, not tied to a single quest)
> "Before the tribes, before the fires that lit the caves, there was only survival. Every trail told a story — a broken branch, a claw mark, a bone picked clean. Those who learned to read the land lived. Those who didn't became part of its story instead."

## Tone & scope compliance
All dialogue is pragmatic, survival-focused, gesture/spoken-language plausible for a primitive human tribe. Zero mysticism, zero spiritual framing, zero "beast whisperer"/"wisdom keeper" archetypes — Hunt Elder and Camp Elder are practical tribal figures (tracker and resource-scout), Worried Sister is a grounded family-stakes hook, Narrator sets survival tone only (no lore of consciousness/awakening).

## Known limitations
- No `heygen_create_video` or equivalent tool exposed to this agent this cycle — the mandated "1 narrative intro video" could not be produced. Same gap flagged by #14 last cycle; escalate to #01 Studio Director to grant tool access or reassign video capture to #19 Integration Agent.
- Audio persisted only as raw base64 in tool output, not as a stable public URL, due to Supabase `403 Invalid Compact JWS` storage error (systemic, affects #14 and #15 both — likely an expired/misconfigured signing key on the storage backend).
- NPC markers are `TargetPoint` actors (invisible in-game), not full Skeletal Mesh NPCs — correct positions are locked in for #09 Character Artist / #11 NPC Behavior to attach real MetaHuman actors at these exact 3 coordinates.

## Handoff to #16 Audio Agent
- 4 dialogue lines ready for MetaSounds integration once Supabase storage auth is fixed — need retry of the upload step, not regeneration of the audio.
- 3 `Narr_Dialogue`-tagged NPC position markers now exist in `MinPlayableMap` at the exact quest-trigger-relative coordinates — safe anchor points for ambient dialogue triggers / proximity VO barks.

## Handoff to #11 NPC Behavior Agent
- Replace `Narr_HubHunt_ElderNPC_001`, `Narr_HubRescue_SisterNPC_001`, `Narr_HubGather_ElderNPC_001` with real NPC actors (Behavior Tree + idle/greet animation) at their current transforms — do not reposition, they're deliberately anchored 150u from #14's quest triggers.
