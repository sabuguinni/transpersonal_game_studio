# Crowd & Traffic Simulation Agent (#13) — Cycle PROD_CYCLE_AUTO_20260711_002

## Bridge Status
Fully operational — 5/5 `ue5_execute` Python calls succeeded with no timeouts (command IDs 31568–31572).

## Anti-duplication / consistency compliance
Per the standing MAX-importance memory (`hugo_herd_consistency_v1`), the FIRST action performed was an audit
of all actors within ~3500 units of the hub (X=2100, Y=2400) for any existing tag starting with `Herd_`
(case-insensitive). The audit query executed cleanly (ReturnValue=true, IDs 31569/31570), but this UE5 Python
bridge does not surface captured stdout back into the agent's tool-result channel (only a boolean
`ReturnValue` is returned) — so the exact list of pre-existing tags could not be read back into this report
directly. To remain safe under that constraint, the herd-formation script (ID 31571) was written to be
**self-auditing and idempotent within its own execution**:
- It re-queries tags itself and splits herbivore actors near the hub into `already_tagged` (any existing
  `Herd_*` tag, case-insensitive) vs `untagged`.
- Actors already carrying a `Herd_*` tag were **left untouched** — no repositioning, no renaming, no
  new tag added. Their existing tag scheme was logged for reuse (`log_lines` "REUSE_EXISTING" entries via
  `unreal.log`, visible in the live UE5 Output Log).
- Only actors with **no** `Herd_*` tag were assigned the new tag `Herd_HubGrazing_01` and repositioned into
  a grazing cluster.
- If any already-tagged herd existed AND untagged herbivores also existed, the script would extend using
  `<existing_tag>_ext` rather than inventing a fresh unrelated scheme — this branch only fires if that
  specific condition is met.

## Herd formation logic (script ID 31571)
- Candidate set: all non-predator herbivore actors (label contains Trike/Triceratops/Brachio/Para/Stego/
  Edmonto, excluding TRex/Raptor) within 3500 units of (2100, 2400).
- Untagged candidates were arranged in a grid-like grazing cluster centered near (1700, 2600), 3 per row,
  450-unit spacing (within the 300–600 spec), each individual given a shared base orientation (yaw 35°)
  with a small per-individual variance (0–8°) to avoid robotic uniformity while still reading as "grazing
  together facing roughly the same way."
- Tag applied: `Herd_HubGrazing_01` (only to actors that had zero prior `Herd_*` tag).
- Level saved via `unreal.EditorLevelLibrary.save_current_level()` after changes.

## Verification pass (script ID 31572)
Re-queried all actors near the hub carrying any `Herd_*` tag and logged the full member list (label, tags,
rounded X/Y) to the UE5 Output Log under `=== HERD VERIFICATION ===` for the next agent (#14 Quest & Mission
Designer) to consume directly from the live editor state rather than from this file, since this bridge
does not return stdout content to the calling agent.

## Known tooling limitation (flag for #01/Hugo)
The `ue5_execute` Python bridge in this environment returns only `{"ReturnValue": true/false}` — it does
not capture and return `print()`/`unreal.log()` output to the calling agent. This means agents cannot
verify exact prior-cycle tag names/state through this channel alone and must rely on defensive,
self-auditing scripts (query-then-branch inside a single execution) rather than reading back results
across turns. Recommend exposing captured log/stdout in the `result` field of the command response so
future herd/crowd cycles can positively confirm reuse instead of relying on idempotent script design alone.

## Dependency for next agent
**#14 Quest & Mission Designer** can reference the `Herd_HubGrazing_01` tag (or any pre-existing `Herd_*`
tag preserved this cycle) to anchor herd-observation objectives near the hub. **#01/Hugo**: consider adding
stdout/log capture to the UE5 Python bridge response so agents can positively verify state instead of
inferring from `ReturnValue` alone.
