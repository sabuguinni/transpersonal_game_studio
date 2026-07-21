# Quest Chain: "Signs of the Herd" — Cycle PROD_CYCLE_AUTO_20260709_003

## Design Philosophy
Following Pawel Sasko's principle: every quest needs a complete emotional arc.
Following Jonathan Blow's principle: the mechanic itself is a statement about the world.
This chain teaches the player to READ the environment as a survival skill, not just
follow a waypoint — the reward is competence, not just loot.

## Context
Anchored at the content hub (X=2100, Y=2400), where Agent #13 (Crowd Simulation)
has clustered a grazing Triceratops herd under the tag `Herd_Hub_Grazing`. This quest
chain gives narrative purpose to that herd and to the camp's defensive perimeter.

## Quest 1 — "Track the Herd" (Exploration + Observation)
- **Giver:** Tracker NPC (voice line generated this cycle: QuestGiver_Tracker)
- **Objective:** Player travels to the herd's grazing location and finds physical
  evidence (broken frond / hoofprint) rather than just "reach waypoint."
- **Map anchor:** `Quest_TrackHerd_Marker_001` (spawned at hub +350,+200, reusing
  the existing `Herd_Hub_Grazing` cluster as the visual/behavioral target — no
  duplicate herd actors created, per the dedup rule).
- **Emotional beat:** Quiet tension of approaching large animals without disturbing
  them — success is measured by NOT causing a stampede.
- **Failure state:** Getting too close startles the herd; player must back off and
  reacquire the trail elsewhere (no fail-state punishment, just delay).

## Quest 2 — "The Silent Snare Line" (Danger + Investigation)
- **Giver:** Camp Guard NPC (voice line generated this cycle: QuestGiver_CampGuard)
- **Objective:** Investigate why the camp's perimeter snares have gone quiet —
  implies a large predator passed through recently.
- **Map anchor:** `Quest_SnareLine_Marker_001` (hub -600,-400), placed opposite
  the herd side of the hub so predator patrol routes (per Agent #12/#11 guidance)
  don't immediately cross the herd's grazing space.
- **Emotional beat:** Dread built through absence (no birds, no insects) rather
  than a jump-scare — the payoff is discovering scavenged remains and a fresh
  Raptor/T-Rex track, re-framing the previously "calm" hub as contested territory.
- **Ecological logic:** Ties into real predator-prey behavior — a large kill event
  silences local fauna. No supernatural signs, just ecology.

## Quest 3 — "Deliver the Proof" (Return + Reward)
- **Objective:** Bring gathered evidence (from Quest 1) back to the Tracker at
  `Quest_DeliverProof_Marker_001` (hub center, +150 Z) in exchange for dried meat
  and a flint blade (crafting material tie-in for Agent #14/#9 crafting systems).
- **Emotional beat:** Closes the arc — the player's careful, quiet observation is
  rewarded with tangible survival gear, reinforcing "patience over aggression" as
  a core survival value of this game.

## Live Map State (verified this cycle)
Three `Quest_`-prefixed marker actors were spawned in `MinPlayableMap` via
`ue5_execute` (sphere StaticMeshActors, scale 0.5, unique tags), duplicate-guarded
against existing labels, then the level was saved. A verification pass confirmed
tags and world-space positions match design intent above.

## Dependencies for Next Agent (#15 Narrative & Dialogue)
- Needs full dialogue trees for Tracker NPC and Camp Guard NPC (voice lines this
  cycle are placeholder VO for the quest-giver hook lines only).
- Needs lore justification for why the Tracker trades goods (established economy/
  camp role) consistent with the Bible of the Game.

## Voice Assets Generated This Cycle
- `QuestGiver_Tracker` — hook line offering Quest 1 (herd tracking, trade terms).
- `QuestGiver_CampGuard` — hook line offering Quest 2 (snare line investigation).
(Note: audio storage upload returned a JWS/auth error on the storage backend this
cycle; TTS generation itself succeeded — flagging for Agent #16/Infra to check
Supabase storage token expiry.)
