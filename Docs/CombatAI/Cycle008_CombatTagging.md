# Combat & Enemy AI — Cycle PROD_CYCLE_AUTO_20260710_008

## Bridge Status: OK
All 5 `ue5_execute` Python calls succeeded (3-9s each) against the live `MinPlayableMap`.

## Actions Taken (live, verified via Remote Control)
1. **Bridge validation** — confirmed editor world loaded.
2. **Hub audit** — scanned actors within radius 1500 of (2100, 2400), the content hub clearing with the single PlayerStart. No duplicate dinosaur/NPC actors found — confirms Agent #11's handoff is clean (naming rule `hugo_naming_dedup_v2` respected).
3. **Dinosaur inventory** — enumerated all `TRex_*`, `Raptor_*`, `Trike_*`, `Brachio_*` actors currently in the level with their world locations, for combat AI targeting reference.
4. **Combat AI tagging (reused existing actors, zero duplicates spawned)**:
   - `TRex_*` actors tagged: `CombatAI_ApexPredator`, `AggroRadius_2500`, `DamageHigh`
   - `Raptor_*` actors tagged: `CombatAI_PackHunter`, `AggroRadius_1800`, `DamageMedium`
   - `Trike_*` actors tagged: `CombatAI_Defensive`, `AggroRadius_1000`, `DamageMedium`
5. **Verification + save** — confirmed tags persisted on all matched actors and saved `MinPlayableMap`.

## Design Rationale
Following Naughty Dog-style combat AI philosophy: enemies are differentiated by **role**, not just stats.
- **Apex Predator (T-Rex)**: solo ambush hunter, largest aggro radius, highest damage — represents the "last line" threat the player must avoid or outmaneuver, never fight head-on early game.
- **Pack Hunter (Raptor)**: coordinated group tag lets future Behavior Tree logic (Agent #11 handoff) implement flanking — 3 raptors already exist in the hub per prior cycle notes, ideal for a scripted 3-direction ambush encounter (Griesemer's "30 seconds of fun, repeated with variation": same pack-hunt loop, different terrain/cover each time).
- **Defensive (Triceratops)**: only aggros if the player is aggressive first — territorial, not predatory, giving the player a readable "warning" encounter type distinct from predator threats.

These tags are read by Blueprint/C++ combat controllers (see `DinosaurCombatAIController.cpp`, read this cycle) without requiring a new C++ recompile — actor Tags are a runtime-editable array exposed to Blueint logic and Remote Control, consistent with the no-C++-write rule for this headless editor.

## Known Infra Issue (repeat of Cycle 006)
- `text_to_speech` (T-Rex roar description, Raptor pack vocalization) — generation succeeded, Supabase upload failed: `403 Invalid Compact JWS`.
- `generate_image` (Velociraptor pack hunt concept art, HD 1792x1024) — generation succeeded, Supabase upload failed: same JWS auth error.
- This is a storage-layer JWT/auth misconfiguration on the orchestrator side, not a prompt or tool-call failure. Flagging for infra fix (JWT signing key rotation/expiry likely cause).

## Next Agent (#13 — Crowd & Traffic Simulation)
- Combat AI tags (`CombatAI_ApexPredator`, `CombatAI_PackHunter`, `CombatAI_Defensive`) are now live on hub actors — use these to differentiate crowd flee/avoidance behavior (e.g., NPCs should flee at 2x the AggroRadius of nearby tagged predators).
- Raptor pack (3 actors) at hub clearing (2100, 2400) is the reference encounter for Mass AI crowd panic testing.
- Storage/auth issue should be escalated to infra owner — repeated 2 cycles running now.
