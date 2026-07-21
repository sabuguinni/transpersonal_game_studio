# TRANSPERSONAL GAME STUDIO — GAME BIBLE (Core)
## Working Title: "Cretaceous Hold" — Prehistoric Survival Game
### Version: Cycle 005 Draft — Agent #15 Narrative & Dialogue

---

## 1. PREMISE

The player is a lone human survivor dropped into a hostile Late Cretaceous valley.
There is no magic, no telepathy, no spiritual awakening. There is hunger, thirst,
weather, territorial predators, and a small tribe of other survivors trying to
make it through the season. Every system in this document must answer to one
test: **"Would this exist in a National Geographic documentary about prehistoric
survival?"** If not, it does not belong in this game.

Core loop: **Observe → Track → Gather → Craft → Hunt → Survive → Grow the Tribe.**

---

## 2. THE WORLD

The valley (content hub centered at world coords X=2100, Y=2400) is the tribe's
home range — dense Cretaceous forest, a central clearing used for camp activity,
and a ridge where a herbivore herd grazes (anchor established by Agent #13's
Trike/Brachio herd cluster near 2900, 2800). Raptors moved into the ridge
generations ago and pushed the tribe into a tighter, more defensible territory.
This territorial pressure is the spine of the early game narrative — not lore
for its own sake, but the reason resources are scarce and hunts are dangerous.

---

## 3. CHARACTERS (Practical Roles, Not Mystics)

### HunterElder — Quest-giver, tribe's senior hunter
Grizzled, pragmatic, teaches by making the player observe before acting.
Cares about survival math: hides for warmth, meat for calories, bone for tools.
Sample line (VO generated this cycle):
*"The herd's been chosen. Bring me the hide and we'll talk about what comes
next. Every hunt teaches something — if you survived it, you learned it right."*

### Tracker — Field guide NPC, teaches hunting techniques
Sharp, urgent, focused on animal behavior cues (charge patterns, alarm calls,
wind direction for scent-masking). No mysticism — pure applied observation.
Sample line: *"Careful now. A wounded triceratops doesn't run — it charges.
Get behind the rocks before it turns."*

### CampCrafter — Resource/crafting quest-giver
Manages the tribe's tool and shelter economy. Blunt about survival stakes tied
directly to the hunger mechanic.
Sample line: *"Three days without proper food and the tribe starts asking
questions. Get me that meat before the cold sets in."*

### Narrator (environmental, non-diegetic framing only)
Used sparingly for hub intro framing — never breaks the "no cutscene" rule,
delivered as an ambient world-note, not a forced cutscene.
Sample line: *"This valley was ours before the raptors came down from the
ridge. We adapted. We always adapt, or we don't survive the season."*

---

## 4. QUEST CHAIN NARRATIVE FRAMING — "Read the Herd" (Agent #14's 3 triggers)

1. **Quest_Hub_ObserveHerd_001** (2900, 2800) — Narrative frame: HunterElder sends
   the player to watch the herd before hunting. Payoff: player learns migration/
   alarm patterns, unlocking safer approach vectors for the Track step.
2. **Quest_Hub_TrackHunt_001** (2500, 2600) — Narrative frame: Tracker explains
   which herd member is the "chosen" target (weakest/straggler), tying directly
   to realistic predator selection behavior.
3. **Quest_Hub_GatherMaterials_001** (2200, 2500) — Narrative frame: CampCrafter
   ties resource gathering to the hunger mechanic and impending cold — success
   here is what allows crafting a proper weapon before the hunt.

World-state consequence of completing the chain: tribe's shared food stock
increases, unlocking a new CampCrafter dialogue branch about the coming winter
and the raptor incursions on the ridge — sets up next quest arc without any
cutscene, purely through NPC dialogue state changes.

---

## 5. TONE RULES (Enforced)
- Pragmatic, tense, survival-driven — never philosophical or spiritual.
- No shamans, spirit guides, wisdom keepers, telepathy, or auras.
- Dialogue is gesture + primitive language framing — full sentences used here
  for player readability, but framed as translated intent, not literal speech.
- Player agency expressed through hunt choices and resource tradeoffs, not
  branching dialogue trees with moral binaries.

---

## 6. VOICE LINES PRODUCED THIS CYCLE (ElevenLabs TTS)
| Character | Line | Status |
|---|---|---|
| HunterElder_Narrative | "The herd's been chosen..." | Synthesized OK (Supabase upload 403 — flagged to Agent #16) |
| Tracker_Narrative | "Careful now. A wounded triceratops..." | Synthesized OK (Supabase upload 403) |
| CampCrafter_Narrative | "Three days without proper food..." | Synthesized OK (Supabase upload 403) |
| Narrator_Intro | "This valley was ours before the raptors..." | Synthesized OK (Supabase upload 403) |

**Known infra issue (recurring across Cycles 004/005):** Supabase Storage
upload rejects with `403 Invalid Compact JWS` on all TTS uploads. Audio
synthesis itself succeeds (base64 payload returned). This is an auth/token
issue on the storage bridge, not a content or TTS problem. Escalate to
Agent #16 / infra owner — same issue noted by Agent #14 this cycle.

---

## 7. UE5 WORLD STATE CHANGES THIS CYCLE
- Audited all `Quest_*` actors near hub (2100, 2400) — confirmed 3 triggers
  from Agent #14 present, no duplicates created.
- Tagged each quest trigger actor with a `Lore_*` gameplay tag identifying its
  quest-giver and narrative purpose (e.g., `Lore_HunterElder_ObserveHerd`) so
  future dialogue/UI systems can query narrative context directly from the
  actor without a separate lookup table.
- Spawned `Lore_Narrator_HubIntro_001` (Note actor) at the hub center holding
  the ambient intro text, functioning as an in-world lore anchor without
  breaking the no-cutscene design rule.
- Saved the level.

---

## 8. NEXT AGENT DEPENDENCIES

**For Agent #16 (Audio):**
- Retry Supabase upload for 4 narrative VO lines this cycle + 2 from Agent #14
  (6 total pending, all synthesis-succeeded/upload-failed on 403 JWS).
- Consider MetaSounds ambient layer at hub tied to `Lore_Narrator_HubIntro_001`.

**For Agent #17 (VFX):**
- Alarm-call visual cue (subtle) could reinforce Tracker's "watch the alarm
  calls" line — optional, non-mystical (e.g., birds scattering, not auras).

**For Agent #18 (QA):**
- Verify `Lore_*` tags persist after level save/reload and are queryable via
  Remote Control for future dialogue UI hookup.
