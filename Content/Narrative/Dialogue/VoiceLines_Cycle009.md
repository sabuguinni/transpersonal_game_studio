# Narrative & Dialogue — Voice Lines Cycle 009
**Agent #15 | PROD_CYCLE_AUTO_20260618_009**

---

## Voice Lines Produced

### 1. Elder Hunter — TRex Survival Warning
- **Character:** Elder Hunter (experienced tribe member, 40s, scarred)
- **Context:** Triggered when player first enters TRex territory zone
- **Trigger Actor:** `Dialogue_ElderHunter_001` at (800, -400, 100)
- **Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781788876258_Elder_Hunter.mp3
- **Duration:** ~15s
- **Line:** *"Stay low. The T-Rex tracks movement, not smell. If you freeze, it will pass. If you run — you are dead. Watch the tail. When it stops swinging, the animal is locked on a target. That is your warning. Do not waste it."*
- **Gameplay Function:** Tutorial/survival tip — teaches player TRex detection mechanic

---

### 2. Tribe Leader — River Migration Urgency
- **Character:** Tribe Leader (pragmatic, authoritative, female)
- **Context:** Triggered at river crossing zone before flood event
- **Trigger Actor:** `Dialogue_TribeLeader_001` at (-600, 900, 100)
- **Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781788879240_Tribe_Leader.mp3
- **Duration:** ~16s
- **Line:** *"The river is rising. Two days, maybe three, before this crossing floods completely. We take what we can carry and we move tonight. Anyone who is not ready at dusk gets left behind. That is the rule. That has always been the rule."*
- **Gameplay Function:** Time-pressure quest trigger — initiates migration sequence

---

### 3. Survivor Cache — Hidden Supply Discovery
- **Character:** Survivor Cache (recorded message via carved symbols, interpreted by narrator)
- **Context:** Triggered when player discovers hidden supply cache
- **Trigger Actor:** `Dialogue_SurvivorCache_001` at (1400, 1200, 100)
- **Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781788887657_Survivor_Cache.mp3
- **Duration:** ~14s
- **Line:** *"You found the cache. Good. My father hid it here during the last migration — flint, sinew, dried meat. Enough for a week if you are careful. Take only what you need. Others may come after you who need it more."*
- **Gameplay Function:** Lore delivery + resource discovery reward

---

### 4. Scout Tracker — Herd Intelligence Report
- **Character:** Scout Tracker (young, observant, specialist in animal behaviour)
- **Context:** Triggered at eastern ridge observation point
- **Trigger Actor:** `Dialogue_ScoutTracker_001` at (-1200, -800, 100)
- **Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781788889792_Scout_Tracker.mp3
- **Duration:** ~13s
- **Line:** *"I have been tracking this herd for four days. Twelve adults, three juveniles. They bed down at the eastern ridge when the sun drops. That is when they are slowest. That is when we hunt."*
- **Gameplay Function:** Hunt quest setup — provides tactical information for herbivore hunt mission

---

## Dialogue System Architecture

### Trigger Zone Pattern
All dialogue triggers use `TriggerSphere` actors with scale 2.0 (radius ~200 units).
Player overlap → play audio URL → display subtitle text → mark as heard (no repeat).

### Character Voice Profiles
| Character | Tone | Age | Role |
|---|---|---|---|
| Elder Hunter | Calm, authoritative, measured | ~45 | Survival mentor |
| Tribe Leader | Urgent, decisive, no-nonsense | ~35 | Group commander |
| Survivor Cache | Reflective, generous, past-tense | ~50 | Lore deliverer |
| Scout Tracker | Precise, observational, focused | ~25 | Intel provider |

### Narrative Themes This Cycle
1. **Predator awareness** — TRex behaviour tutorial via character voice
2. **Migration pressure** — time-critical survival decision
3. **Community legacy** — hidden caches as inter-generational survival tool
4. **Tactical hunting** — observation before action

---

## Cumulative Dialogue Assets (Cycles 001–009)

| Cycle | Voice Lines | Trigger Actors | Characters |
|---|---|---|---|
| 001-005 | 8 | 8 | Narrator, Elder, Scout |
| 006 | 2 | 2 | Tribe Elder, Survivor |
| 007 | 2 | 2 | Herd Observer, Combat Scout |
| 008 | 2 | 2 | Herd Watcher, Veteran Hunter |
| **009** | **4** | **4** | Elder Hunter, Tribe Leader, Survivor Cache, Scout Tracker |
| **TOTAL** | **18** | **18** | **9 distinct characters** |

---

## For Agent #16 (Audio Agent)

These 4 audio URLs are ready for integration into MetaSounds:
1. Elder Hunter TRex warning: `1781788876258_Elder_Hunter.mp3`
2. Tribe Leader migration: `1781788879240_Tribe_Leader.mp3`
3. Survivor Cache discovery: `1781788887657_Survivor_Cache.mp3`
4. Scout Tracker hunt intel: `1781788889792_Scout_Tracker.mp3`

**Recommended audio treatment:**
- Apply light reverb for cave/forest environments
- Duck ambient sound -6dB during dialogue playback
- Fade in/out 0.3s to avoid hard cuts
- All lines are English, no subtitles needed for English build
