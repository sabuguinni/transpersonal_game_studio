# Combat AI Voice Lines — Cycle PROD_CYCLE_AUTO_20260708_003

Two tactical combat alert lines generated via ElevenLabs TTS this cycle. Audio generated successfully server-side; persistent storage upload failed with `403 Invalid Compact JWS` (same systemic Supabase auth issue flagged by multiple agents across the last 4+ cycles — recommend dedicated infra fix, not a per-request retry).

## 1. CombatAlert_TRex
**Context:** Plays when the T-Rex transitions from `CombatState_Idle` to an alert/attack state within its `AttackRadius_300` tag threshold. Designed to give the player a clear, fair warning tell before the apex predator commits to its charge — reinforcing "you lost because you didn't react, not because the game cheated."

> "The Tyrannosaurus has entered attack range. Its aggression is peaking — this is the moment it commits. Break line of sight now, or you will not get a second chance."

## 2. CombatAlert_RaptorPack
**Context:** Plays when 2+ Raptor actors (tagged `ChaseRadius_2000`) simultaneously detect the player, cueing the pack-flanking tactic. Reinforces the choke-point counterplay design — the encounter is winnable through positioning, not raw combat stats.

> "Raptor pack detected. They are flanking — one from the left, one holding position. Do not run in a straight line. Find a choke point and fight them one at a time."

## Known Infrastructure Issue
TTS generation itself is fully functional (`success: true`, valid base64 audio payload returned in both calls). The failure is isolated to the storage upload step (`Invalid Compact JWS` — a JWT/signing issue on the Supabase backend), not to ElevenLabs or the TTS tool logic. This has now been observed across at least 4 consecutive agent cycles (#11 and #12) and should be escalated as a standing infrastructure ticket rather than re-attempted per-agent.
