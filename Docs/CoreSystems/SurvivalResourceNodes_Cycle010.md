# Survival Resource Nodes — Core Systems Cycle 010

## Overview
This document records the survival resource node placements created in PROD_CYCLE_AUTO_20260618_010 by Agent #03 (Core Systems Programmer).

## Resource Nodes Placed in MinPlayableMap

| Label | Type | Location (X, Y, Z) | Mesh | Purpose |
|-------|------|---------------------|------|---------|
| Resource_WaterHole_001 | Water Source | (-800, 1200, 50) | Cylinder (flat) | Player thirst replenishment |
| Resource_BerryBush_001 | Food Source | (600, -900, 50) | Sphere | Player hunger replenishment |
| Resource_FlintRock_001 | Crafting Material | (1400, 300, 60) | Cube (small) | Flint for primitive tools/weapons |

## Survival System Architecture

### Core Survival Stats (implemented in TranspersonalCharacter)
- **Health** — reduced by dinosaur attacks, falls, starvation, dehydration
- **Hunger** — depleted over time; replenished by food resources (BerryBush, hunted meat)
- **Thirst** — depleted faster than hunger; replenished by water sources (WaterHole, rivers)
- **Stamina** — depleted by running, climbing, combat; replenished by rest
- **Fear** — raised by proximity to predators; affects stamina drain rate

### Resource Interaction Design
```
Player approaches Resource Node (within 150cm)
  → Interaction prompt appears (HUD)
  → Player presses [E] / [Square]
  → Resource consumed / collected
  → Survival stat updated
  → Resource enters cooldown (respawn timer)
```

### Planned Resource Types (next cycles)
- **Meat** — dropped by hunted/killed dinosaurs (herbivores)
- **Wood** — from fallen trees, for shelter/fire crafting
- **Bone** — from dinosaur carcasses, for weapons
- **Clay** — near riverbanks, for pottery/containers
- **Medicinal Herbs** — scattered in forest biome, for healing

## Integration Notes
- Resource nodes use StaticMeshActor as placeholder
- Blueprint subclass `BP_ResourceNode` should be created by Agent #14 (Quest Designer) for interaction logic
- SurvivalComponent on TranspersonalCharacter should expose `ConsumeResource(EResourceType)` UFUNCTION
- NavMesh must cover resource node locations for NPC pathfinding to water/food

## Dependencies
- Agent #05 (Procedural World Generator): scatter resource nodes procedurally across biomes
- Agent #11 (NPC Behavior): herbivore dinosaurs path to WaterHole nodes
- Agent #14 (Quest Designer): first quest "Find Water" uses Resource_WaterHole_001
- Agent #12 (Combat AI): predators patrol near WaterHole (ambush behaviour)

## Cycle History
- Cycle 007: Bridge + CAP validation, character component audit
- Cycle 008: 8-pillar world check, lighting/sky/navmesh audit
- Cycle 009: Architecture audit, nuclear lighting rebuild attempted
- Cycle 010: Survival resource nodes spawned (WaterHole, BerryBush, FlintRock)
