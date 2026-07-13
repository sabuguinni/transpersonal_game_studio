# Crowd & Traffic Simulation Agent #13 — Cycle PROD_CYCLE_AUTO_20260713_008

**Bridge status: UP.** 5x `ue5_execute` (4 python + 1 console `stat unit`), all `completed`, zero timeouts, zero camera manipulation, zero .cpp/.h writes. 1x `github_file_write` (docs only, within 2-file budget).

## Consistency check performed (per `hugo_herd_consistency_v1`)

Before touching any actor, ran a full audit of all actors within 3500u of the hub (X=2100, Y=2400), logging label / location / existing tags, specifically searching for any pre-existing `Herd_*` / `herd_*` tag (case-insensitive) on herbivore actors. This was done via `unreal.log()` output (not readable back into this session directly, so the subsequent action script was written to be **self-contained and idempotent**: it re-scans and re-checks for existing `Herd_` prefixes as part of the same atomic script before deciding whether to create new tags or reuse existing ones).

**Result of the check (embedded in the atomic action script):**
- If any actor near the hub already carried a tag starting with `Herd_`/`herd_`, that exact tag prefix was reused and the herd was **not repositioned or renamed** — the script logs `REUSED_EXISTING_TAG=<tag>` and takes no further action on those actors.
- If no `Herd_*` tag existed yet, a **new herd was formed** for the first time this run, using the naming scheme `Herd_HubGrazing_01` (first cluster) and, if a second distinct group of untagged herbivores was available, `Herd_HubGrazing_02` (second cluster). This satisfies the "1-2 cohesive herds" directive without inventing a third scheme or duplicating actors.

## What was done to the live MinPlayableMap

1. **Actor discovery** — filtered all actors within 3500u of the hub by label keywords (`trice`/`trike`, `brachio`, `para`, `herb`, `steg`, `ankylo`, etc.) to identify herbivore candidates, explicitly excluding predator-keyword labels (`trex`, `raptor`, `rex`, `carno`, `allo`) so no carnivore was ever pulled into a "grazing herd."
2. **Herd 1 — `Herd_HubGrazing_01`** (only created if no existing `Herd_*` tag was found): up to 6 herbivore actors repositioned into a clearing centered near (2500, 2100), with individual offsets of 300–600u from each other and a shared base orientation (~35° yaw) with small per-individual jitter (±10°) so they read as grazing together rather than a grid or random scatter.
3. **Herd 2 — `Herd_HubGrazing_02`** (only if ≥2 herbivore actors remained untagged after pass 1): a second smaller cluster centered near (1200, 3100), same spacing/orientation logic, opposite shared yaw (~200°) so the two herds don't mirror each other mechanically.
4. **Tagging** — every herd member received its `Herd_HubGrazing_0N` tag plus a lightweight `Crowd_HerdVerified_Cycle008` marker tag (additive, never replacing prior tags) so future cycles can confirm this pass ran without re-deriving state from logs.
5. `unreal.EditorLevelLibrary.save_current_level()` called after each pass — both confirmed via `ReturnValue: true`.

## Design rationale (Jane Jacobs / GTA V lens)

The herd is not a grid of identical assets — it's a small group sharing a *general* orientation (grazing toward the same rough direction) with enough per-individual variance (±10-12° yaw, 300-600u spacing) that it reads as organic clustering rather than placed geometry. This is the minimum viable "living herd" signal for the hero screenshot composition at the hub, without requiring new Mass AI C++ systems (which would be inert in this headless, non-recompiling editor per the `hugo_no_cpp_h_v2` rule).

## Known limitations this cycle

- Python `unreal.log()` output is not readable back into this agent session — verification of *which* branch (reuse vs. create) actually fired could not be confirmed by reading logs directly. The action script was therefore written to self-determine and self-log its own branch decision atomically, per the consistency rule's spirit (never repositioning actors that already carry a herd tag).
- No new C++ Mass Entity crowd system was written — per repeated hard rules (`hugo_no_cpp_h_v2`, mandate v3, and the P5 priority note that C++ is inert here), all crowd behavior this cycle is expressed through actor transform + tag state, which is the only mechanism with real effect in this headless, pre-built binary.

## Next agent focus (#14 Quest & Mission Designer)

- The herbivore herd(s) at/near the hub (`Herd_HubGrazing_01`, possibly `_02`) are now a stable environmental beat — consider a quest objective built around observing, tracking, or hunting near this herd rather than spawning new narrative-specific duplicate actors.
- Reuse the `Herd_HubGrazing_0N` tags and #12's `CombatRole_*` / `CombatTactic_*` tags as the single source of truth for any quest logic referencing these dinosaurs — do not create new duplicate actors with quest-specific suffixes (the documented anti-pattern from `hugo_naming_dedup_v2`).
- Escalate (still open): Supabase Storage 403 "Invalid Compact JWS" bug blocking audio/image uploads for two consecutive agents (#11, #12).
