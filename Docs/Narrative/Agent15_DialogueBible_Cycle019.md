# Narrative & Dialogue Agent #15 — Dialogue Bible
## Cycle PROD_CYCLE_AUTO_20260619_003

---

## TRIBE LORE: ORIGIN STORY

**Camp Location:** (-500, -800) — western ridge, sheltered by rock overhang
**Why here:** Three seasons ago the tribe fled their original camp near the river delta when a raptor pack of 7 attacked at night. They lost 4 members and all stored food. The current camp was chosen for its defensible position: one entrance, rock walls on three sides, clear sightlines to the south.

**Tribe composition (active NPCs):**
- `Tribe_Elder` — oldest survivor, keeper of route memory and seasonal patterns
- `Tribe_Lookout` — young adult, fastest runner, stationed at perimeter
- `Tribe_Hunter` — experienced hunter, teaches new members
- `Tribe_Survivor` — traumatized, guards cave entrance, rarely speaks

---

## DIALOGUE TREES

### NPC: Tribe_Elder (Dialogue_Elder_CampCenter)

**STATE: Pre-Quest (player first approach)**
> "You are new to this land. I can tell — you move like you are not afraid yet. That will change."
> [PLAYER CHOICE A] "What happened here?"
> → "The herd passed through three days ago. Brachiosaurus — forty, maybe fifty. Where they go, the predators follow. We have little time."
> [PLAYER CHOICE B] "I need supplies."
> → "Everyone needs supplies. What can you give in return? We do not trade with empty hands."

**STATE: Quest Active — The Migration**
> "The herd moves north at dawn. Follow the Brachiosaurus trail — but stay wide of the tree line. That is where the raptors wait for stragglers."
> "Come back before dark. If you are not back by dark, we will not look for you."

**STATE: Quest Complete**
> "You tracked the herd and came back alive. Good. Now you understand — this land does not reward courage. It rewards patience."

---

### NPC: Tribe_Lookout (Dialogue_Lookout_Perimeter)

**STATE: Normal (no threat)**
> "Nothing moving on the south ridge. The T-Rex went toward the wetlands — I watched it cross the river an hour ago."
> "The raptors hunt at dusk. If you are going out, go now, come back before the light drops."

**STATE: Alert (T-Rex detected)**
> "Hunters! The T-Rex has crossed the river. Take the children to the cave — now! I will hold the treeline with the others."

**STATE: Relief (threat passed)**
> "They are gone. The T-Rex moved south toward the wetlands — following the herd. We have maybe two days before it circles back. Use the time. Gather flint from the eastern ridge, dry meat from the kill site, and reinforce the camp perimeter. Two days. Do not waste them."

---

### NPC: Tribe_Hunter (Dialogue_Hunter_CraftZone)

**STATE: Pre-Hunt Tutorial**
> "You want to hunt? Good. But listen first. A raptor does not charge — it circles. It tests you. It wants to know if you will run. If you run, you die."
> "Stand your ground, make yourself large, hold the spear at eye level. Let it come to you. One strike, throat or eye. Do not miss."

**STATE: During First Hunt Quest**
> "The young raptor scouts alone — away from the pack. That is your target. Do not engage the pack. One raptor, isolated, moving east of the ridge. Go."

**STATE: Post-Hunt Success**
> "You killed it cleanly. Good. Now skin it — the hide is useful, the claws are tools. Nothing is wasted here. Nothing."

**STATE: Post-Hunt Failure**
> "You ran. I saw you run. The raptor let you go — it was testing you, not hunting you. Next time, hold your ground. There may not be a next time if you do not learn this."

---

### NPC: Tribe_Survivor (Dialogue_Survivor_CaveEntrance)

**STATE: Default (minimal speech)**
> "..." (gestures toward cave interior — safe)
> "..." (shakes head — do not go that way)

**STATE: Player approaches at night**
> "No. Not at night. Nothing moves at night that you want to meet."

**STATE: After T-Rex threat passes**
> "I have seen three camps destroyed. This one will be destroyed too. But not today. Today we are alive. That is enough."

---

## TRIBE ELDER — LORE MONOLOGUES (for ambient/cutscene use)

### Monologue 1: Survival Philosophy
> "The great lizards do not hunt us because we are weak. They hunt us because we are slow, and loud, and foolish. Watch the ground. Watch the sky. Watch the trees. The one who survives is not the strongest — it is the one who sees danger before danger sees them."
> **Audio:** `tts/1781839321447_Tribe_Elder_Lore.mp3`

### Monologue 2: Origin Story
> "I remember the old camp. Three seasons ago, before the river changed course. We had fire, shelter, good stone for tools. Then the pack found us. Seven raptors at night. We ran with nothing. That is why we do not stay in one place too long. That is why we always keep the children near the cave entrance."
> **Audio:** `tts/1781839340603_Tribe_Elder_Origin.mp3`

---

## VOICE ASSETS THIS CYCLE

| Character | Line | Duration | URL |
|-----------|------|----------|-----|
| Tribe_Elder_Lore | Survival philosophy monologue | ~18s | `tts/1781839321447_Tribe_Elder_Lore.mp3` |
| Tribe_Elder_Origin | Origin story — old camp destroyed | ~21s | `tts/1781839340603_Tribe_Elder_Origin.mp3` |
| Tribe_Hunter_Mentor | First hunt teaching — raptor behavior | ~19s | `tts/1781839349848_Tribe_Hunter_Mentor.mp3` |
| Tribe_Lookout_Relief | T-Rex gone — 2 days to prepare | ~18s | `tts/1781839352811_Tribe_Lookout_Relief.mp3` |

---

## UE5 ACTORS PLACED THIS CYCLE

| Label | Type | Location | Purpose |
|-------|------|----------|---------|
| Dialogue_Elder_CampCenter | PointLight (amber) | (-500,-800,120) | Elder dialogue trigger zone |
| Dialogue_Lookout_Perimeter | PointLight (blue-white) | (200,-600,150) | Lookout dialogue trigger zone |
| Dialogue_Hunter_CraftZone | PointLight (orange) | (-300,-1000,120) | Hunter dialogue trigger zone |
| Dialogue_Survivor_CaveEntrance | PointLight (green) | (-1200,-400,120) | Survivor dialogue trigger zone |

**Map saved:** `/Game/Maps/MinPlayableMap` — MAP_SAVED:True

---

## NARRATIVE MIRROR: QUEST 3 — FIRST HUNT

The young raptor scout that the player hunts in Quest 3 is narratively parallel to the player themselves:
- Both are alone, away from their group
- Both are learning their environment for the first time
- Both are testing their limits

The Hunter's dialogue acknowledges this implicitly: "The young raptor scouts alone — away from the pack. That is your target." The player kills something that mirrors their own situation. This creates moral weight without supernatural framing — pure survival realism.

---

## HANDOFF TO AGENT #16 — AUDIO AGENT

**Priority audio tasks:**
1. Ambient sound for camp zone (-500,-800): fire crackling, distant wind, occasional distant dinosaur calls
2. Danger audio cue: low rumble + ground vibration when T-Rex enters detection radius
3. Raptor vocalization set: idle chirp, alert bark, attack screech (3 distinct states)
4. Brachiosaurus herd ambient: slow heavy footsteps, low rumble, leaf-eating sounds
5. Cave entrance audio: dripping water, echo, wind through rock — signals safety/shelter

**Voice lines ready for integration:**
- All 4 TTS files above are production-ready for NPC dialogue system
- Tribe_Elder_Lore and Tribe_Elder_Origin suitable for ambient campfire monologue triggers
- Tribe_Hunter_Mentor suitable for quest briefing cutscene
- Tribe_Lookout_Relief suitable for post-threat resolution event
