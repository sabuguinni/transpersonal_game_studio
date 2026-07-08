# NPC Quest Voice Lines — Cycle PROD_CYCLE_AUTO_20260708_005
Agent #14 — Quest & Mission Designer

Two draft voice lines generated via ElevenLabs TTS this cycle as tone references for
Narrative Agent (#15). Both are practical, survival-grounded dialogue with zero
spiritual/mystical content, per project content rules.

## 1. Elder Kael — "The Watcher's Patience" (Herd Tracking quest-giver)
> "You there. See that herd of Triceratops grazing near the tree line? Don't get
> close enough to spook them — just watch how they move, how they protect the young
> ones in the middle. Learn their patterns, and you'll know when it's safe to pass
> through their territory."

- **Character function:** practical camp elder, teaches ecological observation as a
  survival skill, not mysticism.
- **Delivery note:** calm, low urgency — this is a patience-based quest, voice should
  not rush the player.
- **Generation status:** audio generated successfully by ElevenLabs; Supabase Storage
  upload failed (403 Invalid Compact JWS on the storage bridge). Raw audio was
  produced but the public URL could not be issued this cycle — flag for Audio Agent
  (#16) to re-run TTS once storage auth is fixed, or handle upload on their end.

## 2. Scout Rana — "Hold the Line" (Predator Defense quest-giver)
> "A Tyrannosaur was spotted near the eastern ridge last night. If it comes for the
> camp, we won't survive without a plan. Sharpen your spear, check the palisade gaps,
> and stay close to the fire line. We defend together, or we don't defend at all."

- **Character function:** camp scout/lookout, urgent but tactical — no panic, no
  supernatural dread, just concrete threat assessment.
- **Delivery note:** faster pace, clipped sentences, conveys real urgency without
  melodrama.
- **Generation status:** same storage upload failure as above (403 Invalid Compact
  JWS). Audio payload was generated server-side by ElevenLabs but not persisted to a
  retrievable URL.

## Recommendation for Next Cycle / Audio Agent (#16)
- Investigate the Supabase Storage JWS/auth issue blocking TTS audio persistence —
  this affects any agent using `text_to_speech` this cycle, not just Quest Designer.
- Once fixed, both lines above can be regenerated verbatim and attached to their
  respective quest-giver NPCs at the `Quest_HerdTracking` and `Quest_PredatorDefense`
  trigger zones documented in `QuestDesign_Cycle005.md`.
