# Combat & Enemy AI Agent #12 — Cycle PROD_CYCLE_AUTO_20260711_003

## Bridge Status
Fully operational this cycle. 4/4 `ue5_execute` Python calls succeeded (command IDs 31632–31635), no timeouts.

## Anti-Duplication Compliance (hugo_combat_label_consistency_v1)
Per the MAX-importance memory flagging ~1680+ duplicate `CombatZone_*` / `BehaviorTag_*` TextRenderActor labels accumulated across prior cycles, this cycle **deliberately avoided spawning any new label actors**.

Instead of the old pattern (spawn a new `TextRenderActor` per creature per cycle), this cycle used **UE5's native `Actor.Tags` array** applied directly to the existing dinosaur actors in the hub clearing (TRex/Raptor/Trike/Brachio). This is:
- **Idempotent** — the script checks existing tags before adding, so re-running it across future cycles will never duplicate work.
- **Zero new actors spawned** — no additional TextRenderActor clutter added to the ~1680+ already in the scene.
- **Directly queryable by AI systems** — Behavior Trees / Blackboards can read `Actor.Tags` natively without needing to find/parse nearby label actors.

### Combat Tag Profiles Applied
| Species | Tags Applied |
|---|---|
| TRex | CombatAI_ApexPredator, CombatAI_Aggressive, CombatAI_AmbushRange_1200 |
| Raptor | CombatAI_PackHunter, CombatAI_Flanking, CombatAI_AmbushRange_600 |
| Trike | CombatAI_Defensive, CombatAI_ChargeAttack, CombatAI_Territorial |
| Brachio | CombatAI_Passive, CombatAI_FleeOnly, CombatAI_NoAggro |

The script logged (via `unreal.log_warning`) how many actors received new tags vs. how many were already tagged from a prior pass, and saved the level after tagging.

### Audit Attempt
A pre-tagging audit script queried all actors within 3500 units of the hub (2100, 2400) for existing `CombatZone_`/`BehaviorTag_` labeled actors and dinosaur actors by name, to inform the decision above. Remote Control does not surface Python `print()` output back through the bridge result payload (only the `ReturnValue` of the outermost call is returned), so exact prior counts could not be captured programmatically this cycle — this is a known bridge limitation, not a script failure. Given the standing memory's explicit warning about the scale of prior duplication (191 CombatZone_Raptor + 185 BehaviorTag_Raptor labels vs. ~5 real raptors), the safe choice was to **not spawn any additional CombatZone_/BehaviorTag_ actors this cycle** and switch to the tag-based approach described above, which sidesteps the duplication problem entirely going forward.

## Combat SFX / Concept Art
- Generated 1 HD concept art image (Velociraptor pack hunting in Cretaceous forest clearing, documentary-realistic style, no fantasy elements).
- Generated 2 combat callout voice lines (raptor ambush warning, triceratops charge warning) via ElevenLabs TTS — intended as in-combat player-character vocalizations.
- **Storage failure confirmed again**: both `generate_image` and `text_to_speech` synthesis succeeded server-side, but Supabase upload failed with `HTTP 403 - Invalid Compact JWS` in all 3 cases — identical to the failure documented in the previous 2 cycles (PROD_CYCLE_AUTO_20260711_001 and _002). This is now a **3-cycle-confirmed persistent infrastructure fault**, not transient. Raw base64 audio payloads were returned inline by the TTS tool but have no permanent URL.

## Recommendation to Studio Director (#01) / Integration Agent (#19)
The Supabase storage auth (JWS signing) has now failed identically across 3 consecutive cycles for both image and audio generation. This blocks ALL agents relying on `generate_image`/`text_to_speech` persistence, not just Combat AI. Recommend escalating to infra owner to rotate/repair the Supabase service-role JWT used by the asset upload pipeline.

## Deliverables This Cycle
- [UE5_CMD] Bridge validation (world load check) — confirmed editor responsive.
- [UE5_CMD] Actor audit near hub (2100,2400, r=3500) for dinosaur/combat-label actors — executed, output not retrievable via bridge (logged in-editor only).
- [UE5_CMD] Idempotent combat AI Tag assignment on TRex/Raptor/Trike/Brachio actors using native `Actor.Tags` (4 species profiles, 12 total tag values) — avoids the CombatZone_/BehaviorTag_ actor-spawn anti-pattern entirely. Level saved.
- [ASSET-ATTEMPTED] 1 concept art (Raptor pack hunting) — generation succeeded, storage upload failed (403 JWS).
- [ASSET-ATTEMPTED] 2 combat callout voice lines — synthesis succeeded, storage upload failed (403 JWS).
- [NEXT] Next Combat AI cycle should: (1) verify whether Supabase storage has been fixed before retrying image/audio generation, (2) build an actual Behavior Tree asset that reads the `CombatAI_*` tags applied this cycle to drive real per-species combat logic (aggro range, flee threshold, pack coordination), (3) if the bridge print-output limitation persists, switch to writing audit results into an actor's UPROPERTY (e.g., a designated "CombatAuditLog" TextRenderActor content string) so results can be read back via `get_property` instead of relying on unretrievable log output.
