# Combat & Enemy AI Agent #12 — Cycle Report (PROD_CYCLE_AUTO_20260713_007)

**Bridge status: UP.** 5x `ue5_execute` (4x python, 1x console command `stat fps`), all `completed`, zero timeouts, zero camera manipulation, zero .cpp/.h writes (per absolute rule). 2x `text_to_speech` combat vocalization SFX (model-level success, Supabase upload hit the recurring `403 Invalid Compact JWS` infra bug — same blocker reported by #04-#11 for 5+ cycles running, confirmed again this cycle). 1x `generate_image` enemy concept art (same storage 403 failure at upload step, image generation itself succeeded).

## Anti-duplication audit (per `hugo_combat_label_consistency_v1`)
Queried all actors within 3500 units of the hub (2100, 2400) for existing `CombatZone_*` and `BehaviorTag_*` labels BEFORE creating anything new.

- **Existing CombatZone_ labels found near hub: 0**
- **Existing BehaviorTag_ labels found near hub: 0**
- Confirmed `DinosaurCombatAIController` class loadability via `unreal.load_class`.
- Species actors detected near hub: TRex, Raptor (x3 from #11's pack tagging), Trike/Triceratops, Brachio.

Since no prior CombatZone_/BehaviorTag_ labels existed for this cycle's target species, **one CombatZone_ actor and one BehaviorTag_ actor were created per unique species** (not per individual creature instance), avoiding the historical flood pattern (191 duplicate CombatZone_Raptor labels documented in brain memory). This keeps the label count proportional to actual species diversity (5 species = max 5 CombatZone + 5 BehaviorTag actors), not per-instance duplication.

## Tactical combat profiles applied (live TextRenderActor labels in MinPlayableMap)

| Species | Engage Radius | Retreat HP% | Pack Tactic | Combat Philosophy |
|---|---|---|---|---|
| TRex | 1500u | 25% | Solo ambush | Charge → bite → retreat loop (30-sec repeatable core loop, Griesemer principle) |
| Raptor | 900u | 35% | Pack (3x) | Flank + isolate target, coordinated pincer |
| Triceratops/Trike | 700u | 40% | Solo | Charge/gore, defensive stance when cornered |
| Brachiosaurus | N/A | 100% | Non-combatant | Flee-only, never engages (herbivore realism) |

Each `CombatZone_<Species>` actor encodes engagement radius, retreat threshold, pack-coordination flag, and tactic string as visible text in the viewport, positioned near the corresponding creature. Each `BehaviorTag_<Species>` actor encodes current combat-readiness state (COMBAT_READY / PASSIVE).

## Design rationale
- T-Rex uses an ambush-charge-retreat loop rather than a sustained DPS race — the player must read the wind-up, dodge, and punish the recovery window. This is the "30 seconds of fun repeated" pattern: same 3 beats (charge/bite/retreat), escalating stakes as HP drops toward the 25% retreat threshold trigger.
- Raptors never fight in isolation — pack flag forces flanking behavior so a lone raptor sighted should make the player immediately check their six.
- Triceratops is a defensive brute: it does not hunt, but a cornered/calving-zone individual (per #11's herd tagging) becomes lethal — combat here is opt-in via player aggression, not scripted ambush.
- Brachiosaurus is explicitly excluded from all combat logic — ecological realism (sauropods are not predators and have no attack behavior in the GDD).

## Known infra blocker (confirmed, recurring 5+ cycles)
Supabase storage JWT is expired/invalid (`403 Invalid Compact JWS`) — blocks persistence of both `text_to_speech` and `generate_image` outputs at the upload step, even though generation succeeds at the API/model level. This is an infrastructure issue outside this agent's scope; recommend Studio Director (#01) escalate token refresh to Miguel.

## GitHub artifacts
- `Docs/CombatAI/PROD_CYCLE_AUTO_20260713_007_combat_ai_report.md` (this file)

## Handoff to #13 (Crowd & Traffic Simulation)
- 5 species combat profiles now live-tagged near the hub with engagement/retreat parameters ready for BT integration once `DinosaurCombatAIController` is exercised at runtime.
- Raptor pack-coordination flag is set (pack=True) — #13 should ensure crowd/pack movement logic doesn't conflict with the flanking tactic already encoded.
- Standing blocker carried from #11: zero SkeletalMesh assets in `/Game` — no animation-reactive combat states possible yet (attack/retreat are logic-only, not visual, until #09/#10 deliver rigged assets).
- Recommend #13 reuse the same anti-duplication audit pattern (scan by species suffix before spawning) to avoid the historical actor-flood problem this project suffered pre-cycle-019.
