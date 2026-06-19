# NPC Voice Lines — Cycle 007
## Agent #15 — Narrative & Dialogue Agent
## PROD_CYCLE_AUTO_20260619_007

---

## Voice Lines Produced

### Elder_Kael — Survival Wisdom
**Text:** "The valley speaks to those who listen. Not with words — with tracks in the mud, with broken branches, with the silence before the great lizard charges. Learn to read the land, or the land will swallow you."
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781858544375_Elder_Kael.mp3
**Duration:** ~14s
**Context:** Triggered when player first enters the valley biome. Elder_Kael is the tribe's most experienced tracker — pragmatic, weathered, no-nonsense.

---

### Hunter_Voss — TRex Encounter Warning
**Text:** "Stay low. Move slow. The TRex sees movement first, then shape. If you freeze against the rock, you are the rock. If you run, you are prey."
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781858546232_Hunter_Voss.mp3
**Duration:** ~10s
**Context:** Triggered when TRex_Savana_001 enters detection radius (200m). Hunter_Voss is the tribe's lead predator-hunter — calm under pressure, tactical.

---

### Survivor_Mara — Dehydration Threshold
**Text:** "Three days without water and the mind plays tricks. I saw my mother in the river reflection. I kept walking. You must keep walking."
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781858553207_Survivor_Mara.mp3
**Duration:** ~9s
**Context:** Triggered when player Thirst stat drops below 20%. Survivor_Mara is a lone wanderer who joined the tribe — resilient, emotionally scarred, honest.

---

### Scout_Dren — Raptor Flanking Warning
**Text:** "The raptors hunt in threes. One drives you forward, two wait at the sides. Do not run straight. Run into the trees — they cannot flank in dense growth."
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781858554978_Scout_Dren.mp3
**Duration:** ~11s
**Context:** Triggered when 2+ Raptor actors are within 150m of player. Scout_Dren is young, fast, observant — the tribe's eyes.

---

## Trigger Mapping (for Audio Agent #16)

| Voice Line | Trigger Condition | Priority | Cooldown |
|---|---|---|---|
| Elder_Kael_Valley | OnBiomeEnter(Valley) | Low | 1x only |
| Hunter_Voss_TRex | TRex within 200m | High | 120s |
| Survivor_Mara_Thirst | Thirst < 20% | Medium | 300s |
| Scout_Dren_Raptors | 2+ Raptors within 150m | High | 90s |

---

## Character Bible Entries (Summary)

### Elder_Kael
- **Role:** Tribe elder, master tracker
- **Age:** ~55 (ancient by prehistoric standards)
- **Motivation:** Preserve tribal knowledge before he dies
- **Speech Pattern:** Short declarative sentences. Never questions. States facts.
- **Relationship to Player:** Mentor — gives knowledge freely, expects action in return

### Hunter_Voss
- **Role:** Lead predator-hunter
- **Age:** ~30
- **Motivation:** Protect the tribe's hunting grounds
- **Speech Pattern:** Military brevity. Tactical language. "Move", "Stay", "Now".
- **Relationship to Player:** Peer — respects competence, dismisses weakness

### Survivor_Mara
- **Role:** Lone survivor who joined the tribe
- **Age:** ~25
- **Motivation:** Find belonging after losing her original tribe
- **Speech Pattern:** Personal anecdotes. Emotional but controlled. Past tense.
- **Relationship to Player:** Ally — shares survival stories as warnings

### Scout_Dren
- **Role:** Young scout, fast runner
- **Age:** ~18
- **Motivation:** Prove worth to the tribe
- **Speech Pattern:** Urgent, observational, specific details. "Three", "Left side", "Now".
- **Relationship to Player:** Informant — reports what he sees, player decides action

---

## Next Cycle Priorities (for Agent #16 — Audio Agent)

1. Load these 4 MP3 URLs into MetaSounds audio assets
2. Create trigger volumes at valley entrance, near TRex spawn, river zones
3. Implement cooldown system so lines don't repeat within cooldown window
4. Spatial audio: voice lines should feel like they come from NPC position, not UI
