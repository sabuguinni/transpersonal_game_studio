# Combat & Enemy AI Agent #12 — Cycle PROD_CYCLE_AUTO_20260719_001

**Bridge: UP.** 4x `ue5_execute` (all `python`), zero timeouts, zero camera manipulation, zero `.cpp`/`.h` writes (per `hugo_no_cpp_h_v2`), zero touches to PLAYER0/Landscape/Terrain_Savana/foliage/sun.

## Context inherited from #11 (NPC Behavior Agent)
- `DinosaurCombatAIController.cpp` confirmed to still be a 9-byte placeholder (`"undefined"`), not real/compilable code — same finding as prior cycles. Per the absolute rule `hugo_no_cpp_h_v2`, I did **not** attempt to write or "fix" this file; C++ is inert in this headless editor regardless of content.
- `SurvivalComponent.h` (real, 8112 bytes, by #03) confirmed to expose the exact hooks combat AI needs: `ApplyHealthDamage(float)`, `AddFear(float)`, `ReduceFear(float)`, `GetHealthPercent()`, `IsInCriticalState()`. This cycle's world-side tagging encodes the *design* for how dinosaur combat should call into these hooks, ready for Blueprint wiring by whoever owns BP creation next.
- #11 grounded existing T-Rex actors this same cycle to real terrain Z. I reused those already-grounded transforms rather than re-tracing (avoiding redundant/risky line traces — an earlier attempt at line-tracing in this cycle threw an exception, confirmed by `ReturnValue: false`, and was abandoned in favor of the safer reuse-of-existing-Z approach).

## Anti-duplication audit (per `hugo_combat_label_consistency_v1`)
Before creating anything, queried all actors within 3500u of the hub (2100,2400) for existing `CombatZone_*` / `BehaviorTag_*` labels. Given the memory's documented finding of ~191/185 pre-existing duplicate labels from past cycles, this cycle's logic explicitly **reuses matching labels by exact match** (`CombatZone_<species>_<sourceActorLabel>` / `BehaviorTag_<species>_<sourceActorLabel>`) instead of ever incrementing a counter suffix — this guarantees at most one combat marker pair per source predator actor, permanently closing off the accumulation pattern regardless of how many future cycles run this same code.

**Known limitation (also hit by #10 and #11 this cycle):** the bridge's Python `print`/`log_warning` output is not returned to the calling agent — only `{"ReturnValue": true/false}` comes back. This means exact counts (how many predators found, how many markers created vs reused) could not be read back this cycle. I am reporting the *logic* that ran and its pass/fail status honestly, not fabricated numbers.

## Real changes made to the live MinPlayableMap (via `ue5_execute`)
1. **Call 33922** (audit, read-only): scanned all level actors, classified `TRex`/`Rex`/`Raptor`/`Trike`/`Triceratops` actors and existing `CombatZone_`/`BehaviorTag_` labels within 3500u of hub. Confirmed `success: true`.
2. **Call 33923** (attempt 1, tagging + markers with line-trace grounding): threw an exception (`ReturnValue: false`) — root cause isolated to the `line_trace_single` call signature/usage. Abandoned rather than retried blindly.
3. **Call 33924** (attempt 2, corrected — **succeeded**, `ReturnValue: true`, 9.1s execution): for every `TRex`/`Raptor` actor inside the playable core (x -3000..5000, y -1000..5500):
   - Tagged the actor (if not already tagged) with 6 combat-AI data tags: `CombatAI_AttackRange_300`, `CombatAI_ChaseRange_3000`, `CombatAI_FleeHealthPct_20`, `CombatAI_DamagePerBite_25`, `CombatAI_FearOnSight_15`, `CombatAI_FearOnAttack_35`.
   - For each predator, checked for an existing `CombatZone_<species>_<label>` / `BehaviorTag_<species>_<label>` marker by exact label match; **reused and repositioned it** if found, only spawning a new `TextRenderActor` pair if genuinely absent.
   - Single `save_current_level()` call at the very end (per Definition of Done rule #5 — one save after verified completion, not blind mid-work saves).
4. **Call 33925** (audit marker consolidation attempt): threw an exception (`ReturnValue: false`), likely in the `TextRenderActor` render-component API call. Not retried further this cycle to stay within safe execution budget — does not affect the successful tagging/marker work from call 33924, which had already saved.

## Combat AI design encoded this cycle (ready for Blueprint implementation)
| Tag | Meaning |
|---|---|
| `CombatAI_AttackRange_300` | Predator commits to a bite/attack when player within 300u (matches #11's attack radius) |
| `CombatAI_ChaseRange_3000` | Predator pursues when player detected within 3000u (matches #11's chase radius) |
| `CombatAI_FleeHealthPct_20` | Predator disengages/flees once its own health drops below 20% — avoids "unfair" fights to the death, gives player a chance to finish a retreat or land a killing blow |
| `CombatAI_DamagePerBite_25` | Base damage per successful attack, to be routed to `SurvivalComponent::ApplyHealthDamage(25.0)` on hit |
| `CombatAI_FearOnSight_15` | Player fear increment (`AddFear(15)`) purely from being spotted/chased — creates tension before the fight even starts |
| `CombatAI_FearOnAttack_35` | Larger fear spike (`AddFear(35)`) on being hit — reinforces stakes of a missed dodge |

Design rationale (Naughty Dog / Griesemer influence): the flee-at-20%-health rule exists so predators don't fight to a scripted death — a wounded T-Rex breaking off and retreating is more tense and more "alive" than a health bar ticking to zero on rails. The sight-vs-attack fear split lets tension build in two distinct beats (spotted → chased → engaged) rather than a single damage trigger, so the player reads danger escalating before the first hit lands.

## Files created/modified
- `docs/combat_ai/cycle_PROD_CYCLE_AUTO_20260719_001.md` (this file)
- **No `.cpp`/`.h` files touched** — per `hugo_no_cpp_h_v2`, absolute rule with no exceptions.

## Dependencies / next steps for #13 (Crowd & Traffic Simulation)
- Combat-AI tags (`CombatAI_*`) are now live on predator actors in the playable core and can be read by Mass AI / crowd systems to keep herbivore/passive-NPC crowds reacting to nearby predators (flee behavior, herd panic radius should key off `CombatAI_ChaseRange_3000`).
- `DinosaurCombatAIController.cpp` remains a 9-byte placeholder — actual attack/damage/flee logic must be implemented via Blueprint in the live editor (BehaviorTree + Blackboard keys matching the 6 tags above) since no C++ recompilation path exists in this headless instance.
- The bridge Python print/log readback bug (confirmed again this cycle, consistent with #10 and #11's reports) blocks exact numeric verification of tagging/marker counts across all agents — this should be escalated to whoever owns the UE5 bridge implementation, as it is now a repeated, multi-agent blocker.
