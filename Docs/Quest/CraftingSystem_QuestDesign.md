# Crafting System — Quest Design Document
**Agent:** #14 Quest & Mission Designer  
**Cycle:** PROD_CYCLE_AUTO_20260619_001  
**Status:** Active — In-World Actors Placed

---

## Overview

The crafting system is the first survival loop in the game. Players must gather raw materials from the environment and combine them at a crafting station to produce tools essential for survival.

---

## Quest: "First Tools" (Tutorial Quest)

### Objective Chain
1. **Gather 2 Rocks** — Find `Resource_Rock_001` through `Resource_Rock_004` near PlayerStart
2. **Gather 1 Stick** — Find `Resource_Stick_001` through `Resource_Stick_003` in the area
3. **Craft Stone Axe** — Interact with `CraftingStation_Campfire_001` at origin (0,0,50)
4. **Optional:** Gather 1 Leaf → Craft Water Container

### Quest Markers (In-World Lights)
| Label | Location | Color | Purpose |
|-------|----------|-------|---------|
| `QuestMarker_GatherRocks` | (300, 150, 200) | Gold | Highlights rock pickup zone |
| `QuestMarker_GatherSticks` | (200, 400, 200) | Green | Highlights stick pickup zone |
| `QuestMarker_CraftingZone` | (0, 0, 200) | Blue | Marks crafting station |
| `QuestMarker_HuntingZone` | (2000, 1500, 300) | Red | Danger zone — future hunt quest |

---

## Crafting Recipes

### Recipe 1: Stone Axe
- **Ingredients:** 2x Rock + 1x Stick
- **Output:** Stone Axe (melee weapon, +15 damage vs small prey)
- **Crafting Time:** 3 seconds
- **Purpose:** Enables wood chopping and basic combat

### Recipe 2: Campfire
- **Ingredients:** 3x Stick
- **Output:** Campfire (placed structure)
- **Crafting Time:** 5 seconds
- **Purpose:** Warmth, cooking food, deters small predators at night

### Recipe 3: Water Container
- **Ingredients:** 1x Rock + 1x Leaf
- **Output:** Crude Water Container (holds 1 unit of water)
- **Crafting Time:** 2 seconds
- **Purpose:** Reduces thirst depletion rate by 30%

---

## Resource Actors (In-World Pickups)

### Rocks (4 actors)
| Label | Location |
|-------|----------|
| `Resource_Rock_001` | (300, 150, 50) |
| `Resource_Rock_002` | (-250, 300, 50) |
| `Resource_Rock_003` | (500, -200, 50) |
| `Resource_Rock_004` | (-400, -150, 50) |

### Sticks (3 actors)
| Label | Location |
|-------|----------|
| `Resource_Stick_001` | (200, 400, 50) |
| `Resource_Stick_002` | (-300, 500, 50) |
| `Resource_Stick_003` | (600, 300, 50) |

### Leaves (3 actors)
| Label | Location |
|-------|----------|
| `Resource_Leaf_001` | (150, -400, 50) |
| `Resource_Leaf_002` | (-500, 200, 50) |
| `Resource_Leaf_003` | (400, -500, 50) |

---

## Quest NPC Voice Line
**Character:** Elder NPC (QuestGiver_Elder)  
**Audio URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781830355461_QuestGiver_Elder.mp3`  
**Line:** *"Survivor! You must gather materials to survive. Find rocks and sticks near the riverbank. Craft a stone axe before nightfall — the predators grow bold in darkness."*

---

## Next Steps for Agent #15 (Narrative & Dialogue)

1. **Write full dialogue tree** for the Elder NPC who gives the "First Tools" quest
2. **Create lore context** — why does the tribe need stone axes? What threat is approaching?
3. **Define quest failure state** — what happens if the player doesn't craft before nightfall?
4. **Write 3 additional voice lines** for quest progress updates:
   - On first rock pickup: acknowledgment line
   - On campfire craft: celebration/relief line
   - On hunting zone approach: warning line

---

## Technical Notes
- All resource actors are `StaticMeshActor` at scale 0.25 (placeholder cubes)
- Quest markers are `PointLight` actors — visible from distance as colored beacons
- Crafting station is `StaticMeshActor` at scale 0.5 at world origin
- Map saved to `/Game/Maps/MinPlayableMap` after all spawns
