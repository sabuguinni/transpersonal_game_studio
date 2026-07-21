# Narrative & Dialogue Agent #15 — Cycle 006 Voice Lines & Dialogue System

## Voice Lines Generated This Cycle

### 1. TribalElder — Stealth Warning
**Character:** Tribal Elder (camp NPC, NPC_TribalElder_001)
**Trigger:** Player approaches T-Rex territory for first time
**Audio:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781698230225_TribalElder_Stealth.mp3
**Duration:** ~7s
**Line:** *"Quiet. Listen. The T-Rex hunts by sound. Move slow, stay downwind, and do not — whatever you do — run."*
**Quest Hook:** Unlocks stealth tutorial objective

---

### 2. Scout — Nest Discovery
**Character:** Scout (outpost NPC, NPC_Scout_001)
**Trigger:** Player reaches eastern ridge scout outpost
**Audio:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781698241124_Scout_NestDiscovery.mp3
**Duration:** ~8s
**Line:** *"We found the nest. Forty eggs, maybe more. If they hatch before we move — this valley belongs to them, not us."*
**Quest Hook:** Triggers Quest: "Destroy the Nest" or "Relocate the Tribe"

---

### 3. Hunter — River Briefing
**Character:** Hunter (river NPC, NPC_Hunter_001)
**Trigger:** Player approaches river hunting ground
**Audio:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781698254038_Hunter_RiverBriefing.mp3
**Duration:** ~10s
**Line:** *"The river bends north past the dead forest. That is where the herd waters at dusk. Bring your spear — and bring someone you trust with your back."*
**Quest Hook:** Unlocks "First Hunt" quest objective at river bend

---

## Dialogue Trigger Volumes Placed in MinPlayableMap

| Label | Location | Linked NPC | Dialogue |
|---|---|---|---|
| Dialogue_TribalElder_Camp | (200, 100, 50) | NPC_TribalElder_001 | Stealth Warning |
| Dialogue_Scout_Outpost | (800, -400, 80) | NPC_Scout_001 | Nest Discovery |
| Dialogue_Hunter_River | (-600, 900, 60) | NPC_Hunter_001 | River Briefing |
| Dialogue_Elder_NestWarning | (1200, 1500, 70) | NPC_TribalElder_001 | Nest Danger |

## NPC Placeholder Actors Placed in MinPlayableMap

| Label | Location | Role |
|---|---|---|
| NPC_TribalElder_001 | (180, 80, 50) | Tribal Elder — main story NPC |
| NPC_Scout_001 | (820, -380, 80) | Scout — intel/quest giver |
| NPC_Hunter_001 | (-580, 920, 60) | Hunter — combat/hunt quests |
| NPC_Gatherer_001 | (300, -200, 50) | Gatherer — resource/crafting quests |

## Cumulative Voice Line Library (All Cycles)

| Cycle | Character | URL |
|---|---|---|
| 002 | TribalElder_HuntBriefing | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/... |
| 005 | TribalElder_HuntBriefing2 | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781693798880_TribalElder_HuntBriefing2.mp3 |
| 005 | Scout_MigrationBriefing | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/... |
| 006 | TribalElder_Stealth | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781698230225_TribalElder_Stealth.mp3 |
| 006 | Scout_NestDiscovery | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781698241124_Scout_NestDiscovery.mp3 |
| 006 | Hunter_RiverBriefing | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781698254038_Hunter_RiverBriefing.mp3 |

## For Agent #16 (Audio Agent)

These voice lines need:
1. **Ambient reverb** — cave/forest/open-air variants per trigger location
2. **Crowd murmur** — low tribal camp background under Elder lines
3. **Tension sting** — short percussion hit when nest discovery line plays
4. **Distance attenuation** — lines should fade if player walks away mid-dialogue

Audio trigger volumes are placed in map — connect MetaSounds cues to `Dialogue_*` TriggerBox actors.
