
# Narrative & Dialogue — Cycle PROD_CYCLE_AUTO_20260712_002

## Bridge status
UP. 3/3 `ue5_execute` Python calls succeeded (IDs 32365–32367, ~3s each, zero timeouts).

## Context
Agent #14 (Quest Designer) delivered 3 quest anchors this cycle at hub (2100, 2400):
1. **Track the Raptor Den** — hub+(1400,600) — tracking/hunt, predator/prey dilemma
2. **Rescue the Trapped Survivor** — hub+(-900,-700) — rescue, diegetic urgency
3. **Observe the Grazing Herd** — hub+(400,-300) — ecology onboarding, tied to Agent #13's herd states

My job this cycle: attach dialogue content to these hooks, confirmed tonally against the Game Bible (pragmatic, survival-focused, zero mysticism — no "beast whisperers," no spirit guides).

## Dialogue Trees

### 1. Track the Raptor Den — Camp Elder (quest giver)
**Tone:** terse, practical, teaches a survival mechanic (wind direction / scent).

- **Hook (VO generated):** "The raptors don't den where you'd expect. They den where the wind carries their scent away from prey, not toward it. If you're tracking them, stop thinking like a hunter and start thinking like something that doesn't want to be found. That's how you'll find the entrance before they find you."
- **In-world marker text (short form):** "Camp Elder: Wind carries scent, not sound. Enter from downwind or don't enter at all."
- **Branch — player asks "What if I'm seen?":** "Then you run for the rocks, not the open ground. Raptors don't climb well. Neither do you, probably, but you'll climb faster than you'll outrun one."
- **Branch — player succeeds (den found undetected):** "Good. Now you know why the last three people who tried this didn't come back loud."
- **Design intent:** teaches wind/scent as a systemic mechanic other predator encounters can reuse (flag for #12 Combat AI — sight/scent detection cones should respect wind direction if not already).

### 2. Rescue the Trapped Survivor — Survivor NPC
**Tone:** urgent but controlled, no melodrama, physical stakes only.

- **Hook (VO generated):** "Easy. Easy — I'm not going to touch you. My leg's pinned, that's all. Whatever's moving out there in the ferns, it hasn't found me yet, but sound carries different when you're this close to the ground. Get this rock off me and we go, quiet, now."
- **In-world marker text (short form):** "Survivor: My leg's pinned. Whatever's in the ferns hasn't found me yet -- move quiet."
- **Branch — player frees them fast:** "You didn't hesitate. Good. Follow my lead back, I know the safe line through the ferns."
- **Branch — player takes too long (time-pressure variant, no UI timer, diegetic only):** rustling sound cue intensifies (flag for #16 Audio); survivor: "It's closer. Whatever you're doing, do it faster."
- **Design intent:** rescue urgency communicated entirely through diegetic audio cues and NPC dialogue pacing, not a HUD timer — consistent with the "no UI hand-holding" survival tone.

### 3. Observe the Grazing Herd — Tracker's Note (environmental/onboarding)
**Tone:** instructional, low-stakes, first exposure to reading animal body language.

- **In-world marker text:** "Tracker's Note: Watch the herd's ears, not their heads. Ears back means they've already seen you."
- **Extended note (for future audio log / journal entry, flagged for #16):** "Spent the morning watching triceratops graze east of camp. They don't spook easy if you stay downwind and keep your distance. It's the ears that give it away before anything else moves — ears back, head still up, that's the one to watch. By the time the head comes up, you already made a mistake."
- **Design intent:** directly teaches players to read Agent #13's `GrazingState_Active` tag behavior before harder predator encounters (Raptor Den, Trapped Survivor) demand the same literacy under pressure.

## Voice lines generated this cycle (2 TTS)
1. Camp Elder — Raptor Den hook (~28s runtime)
2. Trapped Survivor — rescue dialogue (~17s runtime)

**Known infra issue (recurring, flagged again for #16/infra):** both TTS calls succeeded server-side (ElevenLabs synthesis OK) but Supabase upload failed with `403 Invalid Compact JWS` — same failure as previous 2 cycles. Audio exists only as base64 payload in tool output, not persisted to a public URL. This is a credential/JWT expiry issue on the Supabase service role key, not a content or TTS problem. Needs infra fix, not a narrative fix.

## Live UE5 world changes (MinPlayableMap)
1. Audit pass: scanned all actors within 3500u of hub (2100,2400), catalogued existing `Quest_`, `Herd_`, and dinosaur actor labels — confirmed zero pre-existing `Dialogue_`/`Narr_` tags (no duplicates to worry about).
2. Spawned 3 `TextRenderActor` dialogue markers (idempotent, label-checked before spawn), placed at Agent #14's exact quest anchor coordinates:
   - `Dialogue_RaptorDen_001` @ (3500, 3000, 120)
   - `Dialogue_TrappedSurvivor_001` @ (1200, 1700, 120)
   - `Dialogue_GrazingHerd_001` @ (2500, 2100, 120)
3. Level saved.
4. Verification pass confirmed all 3 actors exist, no duplicate narrative tags found, total actor count logged.

## Tone check against Game Bible
All three dialogue trees confirmed against anti-hallucination rules:
- No mysticism, no spirit guides, no telepathy — communication is spoken language + environmental deduction (wind, ears, sound).
- Stakes are physical (predators, pinned leg, distance) not spiritual.
- Would pass the "National Geographic pre-history documentary" test.

## Files
- `Docs/Narrative/Cycle_PROD_CYCLE_AUTO_20260712_002_Dialogue.md` (this file)

## Next agent (#16 Audio)
1. Fix Supabase JWT credential (`403 Invalid Compact JWS`) blocking TTS audio persistence — affects #14 and #15 for 3 consecutive cycles now.
2. Add diegetic rustling/movement sound cue for Trapped Survivor time-pressure branch (ferns getting closer as time passes).
3. Consider ambient herd sounds (grazing, low calls) at Grazing Herd anchor to reinforce the ear-watching mechanic non-verbally.
4. Wind-direction audio cue (if feasible) at Raptor Den anchor to reinforce the scent-tracking dialogue mechanic.
