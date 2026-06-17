# Narrative Bible — Agent #15 — Cycle PROD_CYCLE_AUTO_20260617_007

## RECURRING CHARACTERS ESTABLISHED THIS CYCLE

### TribalElder (Unnamed — referred to as "Elder")
- **Role**: Mentor figure, quest giver, survival knowledge repository
- **Voice**: Gruff, weathered, pragmatic — no sentimentality, only hard-won wisdom
- **Motivation**: Tribe survival above all. Respects competence, not bravado.
- **First appearance**: Camp center (`NPC_TribalElder_001` at -50, -100, 100)
- **Dialogue style**: Short sentences. Pauses. Lets silence do work. Never explains twice.

### Pira (Juvenile Parasaurolophus)
- **Role**: Recurring companion animal — NOT a pet, NOT mystical. A scared juvenile separated from herd.
- **Name origin**: Tribal word for "small flame" — she has a distinctive orange crest marking
- **Scarred flank**: Right side, three parallel claw marks from a previous raptor encounter
- **Narrative function**: Empathy anchor. Player protects her across multiple quests. If she dies, it's permanent.
- **Spawn marker**: `NPC_Pira_Parasaur_001` at (600, -400, 100)
- **Behavioral notes**: Skittish. Freezes when T-Rex is near. Calls out (low honk) when raptors are within 200m.
- **Recurring arc**: Safe Passage → Migration → Northern Water → (potential) Herd Reunion

### Scarback (Raptor Alpha — Named Antagonist)
- **Role**: Recurring predator antagonist — intelligent, territorial, remembers the player
- **Name origin**: Deep scar across his back from a previous territorial fight — won that fight
- **Pack**: Leads a pack of 4 raptors. Two flankers, one distractor, Scarback closes.
- **Behavioral signature**: Circles before charging. Tests prey. Never wastes energy.
- **Spawn marker**: `NPC_Scarback_Alpha_001` at (-900, 1100, 100)
- **Narrative function**: Escalating threat. First encounter = observation. Second = near-miss. Third = direct confrontation.
- **Player relationship**: Scarback learns. If player escapes twice, he changes tactics on third encounter.

### TribalWarrior (Named: Koru)
- **Role**: Combat advisor, field partner on dangerous missions
- **Voice**: Younger than Elder, more aggressive, less patient — but not reckless
- **Motivation**: Prove himself to the tribe. Respects the player if player survives.
- **Spawn marker**: `NPC_TribalWarrior_001` at (100, -200, 100)

---

## DIALOGUE TREES — THIS CYCLE

### Quest: "The Migration" — TribalElder Dialogue Tree

#### Node 1 — INTRO (triggers when player approaches Elder at camp)
> **Elder**: "The herd moves east at dawn. You will follow."
> [PLAYER CHOICE A] "How many days?"
>   → **Elder**: "Three. Maybe four if the raptors slow you. They will try."
> [PLAYER CHOICE B] "Is it dangerous?"
>   → **Elder**: "Everything is dangerous. The question is whether you are more dangerous."
> [PLAYER CHOICE C] "I am ready."
>   → **Elder**: "No. But you will be, or you will not come back. Both are acceptable."
> **[QUEST ACCEPTED — Migration trail begins]**

#### Node 2 — MID-QUEST (triggers at Stage 3 marker — T-Rex territory)
> **Elder** (via signal fire — pre-recorded message delivered by scout):
> "If you are reading this fire, you reached the T-Rex ground. Do not light fires here. Do not make noise. Move at dusk — the T-Rex hunts at dawn and midday. At dusk it rests. You have one hour."

#### Node 3 — COMPLETION (triggers when player returns to camp)
> **Elder**: "You made it. The herd is safe, the little one is safe, and you — you are still breathing."
> **Elder**: "Not every warrior who faces the migration trail can say the same."
> **Elder**: "The elders will hear of this. Tonight, you eat with us."
> **[QUEST COMPLETE — Tribe Trust +25, Unlock: Elder's Cache]**

---

### Quest: "Raptor Observation" — Internal Monologue Lines

#### Phase 1 — Approaching the observation post
- *"The scout said stay low. I understand why now — the raptors are closer than I expected."*
- *"Three of them. No — four. One is hiding in the tall grass to the east. Smart."*

#### Phase 2 — Observing Scarback's behavior
- *"That one with the scar — he's not hunting. He's watching. Watching me."*
- *"He knows I'm here. He's deciding if I'm worth the effort. Don't move. Don't breathe."*

#### Phase 3 — Withdrawing safely
- *"Slow. One step at a time. If I run, they run faster."*
- *"Made it. But Scarback — he watched me leave. He'll remember this position."*

---

### Environmental Storytelling — Stampede Failure State

**What the camp looks like after the herd passes through (player failed to redirect stampede):**

- Three shelters collapsed — wooden frames snapped, hides scattered
- Fire pit extinguished and scattered — ash trail leads east
- Supply cache crushed — food stores destroyed (hunger penalty: -40%)
- Two tribal NPCs injured — one with broken leg (Koru), one unconscious
- Pira's pen destroyed — Pira missing (triggers "Find Pira" side quest)
- Hoof prints 60cm deep across entire camp area
- **Ambient audio**: Distant thunder of retreating herd, injured NPC groans, Koru calling for help

**Narrative text (shown on screen):**
> *"You were too slow. The herd came through like a river of bone and muscle, and the camp paid the price. Koru is alive — barely. Pira is gone. You have to find her before the raptors do."*

---

## VOICE LINES GENERATED THIS CYCLE

| Character | Line | Duration | URL |
|-----------|------|----------|-----|
| TribalElder_TRexLesson | "Three winters ago, I watched the T-Rex take my brother..." | ~20s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781702696892_TribalElder_TRexLesson.mp3 |
| TribalElder_PiraIntro | "Pira. That is what we call her — the little one with the scarred flank..." | ~17s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781702719854_TribalElder_PiraIntro.mp3 |
| TribalWarrior_ScarbackEncounter | "It is watching us. Do not move. Scarback does not charge unless you run..." | ~17s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781702733348_TribalWarrior_ScarbackEncounte.mp3 |
| TribalElder_MigrationComplete | "You made it. The herd is safe, the little one is safe..." | ~15s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781702736618_TribalElder_MigrationComplete.mp3 |

---

## UE5 ACTORS PLACED THIS CYCLE

| Label | Location | Purpose |
|-------|----------|---------|
| NPC_TribalElder_001 | (-50, -100, 100) | Elder NPC position marker (gold light) |
| NPC_Pira_Parasaur_001 | (600, -400, 100) | Pira juvenile Parasaur position (green light) |
| NPC_Scarback_Alpha_001 | (-900, 1100, 100) | Scarback Raptor Alpha territory (red light) |
| NPC_TribalWarrior_001 | (100, -200, 100) | Koru warrior NPC position (amber light) |
| Dialogue_MigrationBriefing_001 | (-100, -150, 120) | Dialogue trigger zone (blue light) |

---

## HANDOFF TO AGENT #16 — AUDIO AGENT

### Audio Assets Needed (Priority Order):

1. **Pira ambient sounds** — juvenile Parasaur calls: low honk (calm), high-pitched alarm call (raptor nearby), distress call (injured). Suggest ElevenLabs animal SFX or Freesound queries: "parasaurolophus call", "hadrosaur juvenile"

2. **Scarback Raptor signature sound** — distinctive click-growl before a charge. Should be recognizable as "Scarback specifically" vs generic raptor. Player learns to fear this specific sound.

3. **TribalElder ambient camp sounds** — fire crackling, low murmuring, occasional cough. Establishes camp as safe zone via audio.

4. **Stampede aftermath ambience** — distant rumble fading, debris settling, wind through broken shelters, Koru's pained breathing.

5. **Internal monologue delivery** — the 6 raptor observation lines need a specific voice: hushed, controlled fear, NOT dramatic. Breathing audible between sentences.

### Voice Line Integration Notes:
- All TTS files above are hosted on Supabase — direct URL access confirmed
- TribalElder voice: gruff male, ~50-60 years, slow cadence
- TribalWarrior (Koru) voice: younger male, ~30 years, more urgent
- Internal monologue: same voice as player character — hushed, first-person

---

## NARRATIVE CONTINUITY NOTES

### Established Canon (DO NOT CONTRADICT):
- Pira has a scarred right flank from a raptor encounter BEFORE the game starts
- Scarback's scar is on his BACK (not face) from a territorial fight he WON
- The Elder lost a brother to a T-Rex three winters ago — this is personal, not abstract
- Koru (TribalWarrior) is younger, wants to prove himself — NOT reckless, but eager
- The tribe uses signal fires for long-distance communication
- Tribe word "Pira" = "small flame" (internal lore — not spoken in dialogue)

### Open Narrative Threads (for future cycles):
- Who gave Pira her first scar? (Could be Scarback — creates deeper antagonist connection)
- What happened to the Elder's brother's body? (Tribe ritual around T-Rex kills?)
- Is there a rival tribe that also follows the migration? (Conflict beyond dinosaurs)
- Koru's motivation: what does he need to prove, and to whom?
