# Engine Architect (#02) — Architecture Finding & Ruling
## Cycle: PROD_CYCLE_AUTO_20260720_004

### Context
Studio Director (#01) escalated a critical finding this cycle: the actor label
`PLAYER0`, referenced by name in the PLAYABLE-FIRST HANDS-OFF rule and in Brain
memory `hugo_mobility_rule_v1`, **does not exist** in `MinPlayableMap`. Instead,
35 actors of class `TranspersonalCharacter` exist in the level, several of them
clearly duplicates/prototypes stacked at the same coordinates (origin, hub).

### Investigation performed this cycle (verified via live ue5_execute, MinPlayableMap)
1. **PlayerController / GameMode check**: 0 `PlayerController` actors and 0
   `GameModeBase` actors exist in the level actor list (expected — this is an
   editor-only session, not a running PIE/game instance, so no controller has
   possessed a pawn yet).
2. **World Settings GameMode override — CONFIRMED CORRECT**:
   `WorldSettings.GameModeOverride = /Script/TranspersonalGame.TranspersonalGameMode`.
   This resolves item #2 of the original directive ("ensure TranspersonalGameMode
   sets DefaultPawnClass correctly") at the level of what is actually
   verifiable in this headless editor — the GameMode class is wired at the
   world level. `DefaultPawnClass` itself lives inside the C++ class default
   object and cannot be inspected or changed via safe Python reflection without
   recompiling C++, which is explicitly forbidden (`hugo_no_cpp_h_v2` — this
   binary never recompiles new C++; any `.cpp/.h` write this cycle would have
   zero effect on the live game). No `.cpp`/`.h` files were created, per that
   absolute rule.
3. **Mobility audit (safety check)**: all 35 `TranspersonalCharacter` actors
   currently report `CapsuleComponent.mobility = MOVABLE`. **No STATIC-mobility
   regression exists right now** — the historic 13–18/07 bug
   (`hugo_mobility_rule_v1`) is not currently reproduced. Good sign, but the
   underlying duplication problem (many candidate "player" pawns) remains.
4. **Player-candidate shortlist**: filtered all 35 actors for labels containing
   "player" (case-insensitive) or located exactly at the sole `PlayerStart`
   (`PlayerStart_Hub_001`, 2100, 2400, z=188). Result: **9 candidates**:

   | Label | Location (x,y,z) | Note |
   |---|---|---|
   | `PlayerChar_Preview_Hub_001` | 2100, 2400, 102 | At PlayerStart XY, z≈ground(100) — **most plausible canonical candidate** |
   | `PrehistoricPlayer_Char09` | 2100, 2400, 240 | At PlayerStart XY, z high (may be non-grounded prototype) |
   | `PlayerCharacter_Prototype` | 2300, 2400, 240 | Near hub, offset 200 units |
   | `MainPlayerCharacter` | 0, 0, 50 | At origin, below ground reference (234) — likely stale/broken prototype |
   | `Char_Player_001` | 0, 0, 100 | At origin |
   | `Player_Character_Visual` | 200, 0, 200 | Near origin |
   | `PrehistoricSurvivor_Player` | -600, 300, 273 | Off-hub |
   | `PrehistoricSurvivor_Player` (2nd) | 50000, 50000, 100 | Far outside playable core — orphaned |
   | `MetaHuman_PlayerCharacter` | 6000, 6000, 200 | Far outside playable core — orphaned |

### Action taken this cycle (non-destructive, verified)
Tagged all 9 candidates with `ARCH_PendingPlayerCanonDecision` (additive tag
only — **zero changes to position, rotation, mobility, components, or input**
on any of them, per HANDS OFF rule §4). This makes the ambiguity visible
in-world to every future agent via `actor.tags`, without requiring anyone to
re-read this document or Brain memory to rediscover it.

### Ruling (Engine Architect law for this project, effective immediately)
1. **No agent may spawn a new actor of class `TranspersonalCharacter` with a
   label containing "Player"** until this canonical-pawn decision is resolved
   by #01/Hugo. This is now a naming law under `hugo_naming_dedup_v2`.
2. **No agent may modify, delete, or reposition any of the 9 tagged
   `ARCH_PendingPlayerCanonDecision` actors** until the decision is made — this
   extends the spirit of the HANDS-OFF rule to the full candidate set, not just
   the literal (missing) `PLAYER0` label, since we cannot safely assume which
   one is actually driving gameplay.
3. **Recommended resolution** (for #01/Hugo, not executed — this is an
   irreversible-adjacent decision about which pawn the game actually
   controls, correctly escalated rather than decided unilaterally):
   `PlayerChar_Preview_Hub_001` (2100, 2400, z=102) is the strongest candidate:
   it sits exactly at the single `PlayerStart` XY and at a Z consistent with
   standing on hub ground (documented hub surface z≈100 in
   `hugo_terrain_savana_v1`). `PrehistoricPlayer_Char09` at the same XY but
   z=240 is the second candidate but its higher Z is inconsistent with resting
   on the ground plane without a capsule-height explanation we could not verify
   without touching the actor.
4. Two orphaned duplicates (`MetaHuman_PlayerCharacter` at 6000,6000 and
   `PrehistoricSurvivor_Player` at 50000,50000) sit far outside the playable
   core (x -3000..5000, y -1000..5500) and are candidates for future
   maintenance-script cleanup — **not deleted this cycle**, per the
   "no mass-delete, cleanup belongs to maintenance scripts" rule.

### Files changed
- `Docs/Architecture/PROD_CYCLE_AUTO_20260720_004_PlayerPawn_Architecture_Finding.md` (this file)

### Live UE5 changes made (verified via ue5_execute)
- Read-only confirmation: `WorldSettings.GameModeOverride` = `TranspersonalGameMode` (correct, no action needed).
- Read-only confirmation: all 35 `TranspersonalCharacter` actors are `MOVABLE` (no static-mobility regression found).
- Additive tag `ARCH_PendingPlayerCanonDecision` applied to exactly 9 actors (listed above) — no other property touched.

### Handoff
- **#01 Studio Director**: needs to confirm with Hugo which of the 9 tagged
  candidates is the real controlled pawn (or confirm none is, and a proper
  `TranspersonalCharacter` needs to be spawned fresh at the PlayerStart).
- **#03 Core Systems Programmer**: blocked from any physics/collision work on
  Characters until the canonical pawn is confirmed — do not touch any of the 9
  tagged actors.
- **#18 QA**: recommend keeping the build-block flagged by #01 until this is
  resolved; this is a legitimate architecture gate, not a formality.
