# Combat & Enemy AI Agent #12 — Cycle PROD_CYCLE_AUTO_20260711_006

## Bridge Status
UP. 4/4 `ue5_execute` (command_type=python) calls succeeded (IDs 31847–31850), no timeouts. 1x `generate_image` attempted but failed at the storage layer (see Blockers).

## Anti-Duplication Audit (per hugo_combat_label_consistency_v1, importance MAX)
Queried all actors within 3500 units of hub (2100, 2400) for existing `CombatZone_` and `BehaviorTag_` labels **before** creating anything:
- **CombatZone_ labels found: 0** (contrary to the ~191 accumulated in earlier flood pattern — appears a prior cleanup cycle removed them, or this hub instance never had them)
- **BehaviorTag_ labels found: 0**
- **Dinosaur actors found near hub:** T-Rex, 3x Raptor, Triceratops/Trike, Brachiosaurus — all already carrying `NPC_*` tags from Agent #11 (Cycle 006): `NPC_State_Patrol/Graze`, `NPC_AggroRange_*`, `NPC_PackHunt`, etc.

Because zero `CombatZone_`/`BehaviorTag_` labels existed, I created **exactly ONE** new marker (`CombatZone_Hub_Tactical`, a TextRenderActor) instead of the old flood anti-pattern of one-per-creature. This satisfies both the consistency rule (reuse-if-exists) and the naming/dedup rule (no duplicate stacking).

## Tactical Combat States Applied (Actor Tags, mesh/Blueprint-agnostic)
Built directly on top of #11's `NPC_*` tags. Added `Combat_*` gameplay tags to each hub dinosaur's existing tag array (idempotent — skips if already present, confirmed via verification pass):

| Creature | Tags Added |
|---|---|
| T-Rex | `Combat_WindupTell_1.2s`, `Combat_HitReaction_Stagger`, `Combat_Death_Collapse`, `Combat_ChargeAttack_300u`, `Combat_Flank_Solo` |
| Raptor (x3) | `Combat_WindupTell_0.6s`, `Combat_HitReaction_Yelp`, `Combat_Death_Collapse`, `Combat_PackFlank_Left`, `Combat_PackFlank_Right`, `Combat_Lunge_150u` |
| Triceratops/Trike | `Combat_WindupTell_0.9s`, `Combat_HitReaction_Stagger`, `Combat_Death_Collapse`, `Combat_DefensiveCharge_400u` |
| Brachiosaurus | `Combat_Passive_NoAttack`, `Combat_FleeOnly_TailSwipe_500u` |

Design intent (Naughty Dog / Griesemer influence):
- **Windup tells** give the player a readable ~0.6–1.2s telegraph before every attack — losses should feel earned, not cheap.
- **Raptor pack flanking** (Left/Right split + center Lunge) creates the "30-second core loop" of dodge-left, dodge-right, punish-center, repeatable with variation as more raptors join.
- **Herbivores are non-combatants by default** (Trike defensive-charge only if provoked, Brachio pure flee) — combat density stays low so encounters remain readable, not chaotic.

## Verification Pass (command 31850)
Re-queried all hub dinosaurs and confirmed `Combat_*` tags persisted after save. Confirmed **zero duplicate `CombatZone_`/`BehaviorTag_` labels** created — count remains 1 total new marker.

## DinosaurCombatAIController Check
Queried `/Script/TranspersonalGame.DinosaurCombatAIController` via `unreal.load_class` — **not found** in the loaded module (consistent with the 17-active-file codebase state; no combat AI controller C++ class currently compiled in). Per `hugo_no_cpp_h_v2` (importance MAX), no `.cpp`/`.h` was written this cycle — headless editor never recompiles, so a new C++ controller would be dead code. Combat logic is instead fully expressed via the tag system above, consumable immediately by Behavior Tree Blueprint nodes (`Tag Query` / `Has Tag`) without requiring engine recompilation.

## Blockers
- **generate_image failed**: Supabase storage returned `HTTP 400 / 403 Invalid Compact JWS` — same infra bug independently confirmed this cycle by Agent #11 (TTS) and Agent #10 (Animation). This is a project-wide storage auth issue, not agent-specific. Intended asset: raptor-pack-flanking-Triceratops concept art for combat encounter reference.
- `search_sounds`/`text_to_speech` combat SFX not available in this agent's toolset this cycle — flagging for Audio Agent (#16) to source impact/roar/death SFX matching the `Combat_HitReaction_*` and `Combat_Death_Collapse` tags above.

## Next Agent (#13 Crowd & Traffic Simulation)
- The `Combat_*` tag vocabulary above is stable and ready to consume for Mass AI crowd/pack logic — reuse `Combat_PackFlank_Left/Right` semantics when scaling raptor packs to larger agent counts.
- `CombatZone_Hub_Tactical` marker exists at (2100,2400,250) — reference it as the canonical combat encounter anchor rather than spawning new zone markers.
