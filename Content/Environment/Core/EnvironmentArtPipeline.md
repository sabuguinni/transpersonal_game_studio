# ENVIRONMENT ART PIPELINE
## Transpersonal Game Studio — Jurassic Survival Game
## Agent #06 — Environment Artist
## Version 1.0 — PROD_JURASSIC_001

---

## VISUAL PHILOSOPHY

**"Every detail tells a story that happened before the player arrived"**

### Core Principles
1. **Narrative Through Environment** — Each prop placement suggests history
2. **Procedural Believability** — Random generation that feels intentional
3. **Atmospheric Tension** — Beauty hiding danger
4. **Scale Hierarchy** — From macro biomes to micro details

---

## BIOME VISUAL IDENTITY

### PRIMARY BIOMES

#### 1. Dense Jungle Canopy
- **Mood:** Claustrophobic beauty, hidden threats
- **Lighting:** Dappled sunlight, deep shadows
- **Vegetation:** Layered canopy (emergent/canopy/understory/floor)
- **Props:** Fallen logs, ancient bones, territorial markings
- **Sound Cues:** Rustling leaves masking movement

#### 2. River Valley Systems  
- **Mood:** Open vulnerability, predator highways
- **Lighting:** Bright reflective water, exposed areas
- **Vegetation:** Riparian plants, muddy banks
- **Props:** Drinking spots, predator tracks, carcass remains
- **Danger Indicators:** Disturbed water, broken vegetation

#### 3. Rocky Outcrops & Caves
- **Mood:** Shelter with escape routes
- **Lighting:** Harsh shadows, cave mouth silhouettes  
- **Vegetation:** Hardy cliff plants, moss
- **Props:** Nesting sites, territorial scratches, bone accumulations
- **Defensive Value:** High ground, multiple exits

#### 4. Swamplands
- **Mood:** Primordial danger, ancient atmosphere
- **Lighting:** Misty, filtered light
- **Vegetation:** Prehistoric ferns, cypress-like trees
- **Props:** Bubbling mud, half-submerged logs, egg sites
- **Hazards:** Unstable ground, hidden predators

---

## ASSET CATEGORIZATION

### VEGETATION LIBRARY

#### Canopy Trees (50-80m height)
- **Emergent_Pine_01-05** — Towering conifers
- **Ancient_Oak_01-03** — Massive deciduous 
- **Jungle_Giant_01-04** — Tropical emergents

#### Understory Trees (10-30m height)  
- **Fern_Tree_01-08** — Tree ferns (domestication friendly)
- **Palm_Cycad_01-06** — Prehistoric palms
- **Flowering_Tree_01-04** — Fruit trees (food source)

#### Shrubs & Bushes (1-5m height)
- **Dense_Fern_01-12** — Ground cover, hiding spots
- **Berry_Bush_01-06** — Food sources, small herbivore attractors
- **Thorn_Shrub_01-04** — Natural barriers
- **Moss_Cluster_01-08** — Moisture indicators

#### Ground Cover & Details
- **Grass_Tuft_01-20** — Various grass types
- **Fallen_Log_01-15** — Decomposing wood, insect habitats
- **Rock_Moss_01-10** — Weathered stone with vegetation
- **Mushroom_Cluster_01-08** — Decomposer organisms

### PROP LIBRARY

#### Environmental Storytelling
- **Dinosaur_Skull_01-05** — Various species, weathered
- **Bone_Scatter_01-12** — Feeding site evidence
- **Claw_Marks_01-08** — Territory markers on trees/rocks
- **Nest_Remains_01-06** — Abandoned or active nesting sites
- **Footprint_Cast_01-10** — Mud impressions, various sizes

#### Geological Features
- **Boulder_Formation_01-20** — Shelter, climbing, landmarks
- **Rock_Outcrop_01-15** — Natural walls, vantage points
- **Cave_Entrance_01-08** — Shelter options, danger zones
- **Stream_Rock_01-12** — Water crossing points
- **Cliff_Face_01-06** — Vertical terrain elements

#### Interactive Elements
- **Climbable_Vine_01-04** — Escape routes
- **Hollow_Log_01-03** — Hiding spots
- **Fruit_Tree_Branch_01-06** — Food gathering points
- **Water_Source_01-04** — Springs, pools, streams

---

## MATERIAL SYSTEM

### Base Material Categories

#### Vegetation Materials
- **M_Vegetation_Canopy** — Large tree bark, leaves
- **M_Vegetation_Understory** — Ferns, shrubs, ground plants  
- **M_Vegetation_Moss** — Moisture-loving plants
- **M_Vegetation_Dead** — Fallen logs, dead branches

#### Geological Materials
- **M_Rock_Sedimentary** — Layered cliff faces
- **M_Rock_Volcanic** — Dark, sharp stones
- **M_Rock_Weathered** — Rounded, moss-covered
- **M_Soil_Rich** — Dark, fertile ground
- **M_Soil_Sandy** — River banks, dry areas
- **M_Mud_Wet** — Swamp, stream edges

#### Organic Materials
- **M_Bone_Fresh** — Recent remains
- **M_Bone_Weathered** — Ancient, sun-bleached
- **M_Wood_Living** — Healthy tree bark
- **M_Wood_Dead** — Rotting, insect-damaged

### Material Features
- **Wetness Response** — Dynamic rain/humidity effects
- **Wind Animation** — Procedural movement for vegetation
- **Damage States** — Claw marks, bite damage, weathering
- **Seasonal Variation** — Color shifts, growth stages

---

## FOLIAGE PAINTING GUIDELINES

### Density Rules
- **Canopy Layer:** 60-80% coverage, irregular gaps
- **Understory:** 40-60% coverage, clustered around water
- **Ground Cover:** 80-95% coverage, varies by moisture
- **Props:** 5-15% coverage, story-driven placement

### Ecological Relationships
- **Water Proximity:** Denser vegetation near streams
- **Soil Quality:** Larger plants on fertile ground  
- **Sunlight Access:** Varied species by light levels
- **Predator Paths:** Trampled areas, broken vegetation

### Performance Optimization
- **LOD Distances:** 
  - Hero vegetation: 100m
  - Standard vegetation: 50m  
  - Background fill: 25m
- **Culling:** Aggressive culling behind player
- **Instancing:** Heavy use of instanced static meshes

---

## PROCEDURAL GENERATION SUPPORT

### Biome Transition Zones
- **Gradient Blending:** Smooth transitions between biomes
- **Edge Species:** Plants that thrive at biome boundaries
- **Micro-Climates:** Small pockets of different conditions

### Storytelling Automation
- **Predator Territory Markers** — Automatic claw mark placement
- **Feeding Site Generation** — Bone scatter near water sources
- **Travel Path Creation** — Worn trails between key locations
- **Seasonal Storytelling** — Nests, migration signs, food caches

---

## LIGHTING INTEGRATION

### Time of Day Considerations
- **Golden Hour:** Warm, dangerous beauty
- **Midday:** Harsh shadows, clear sight lines
- **Dusk/Dawn:** Predator hunting times, atmospheric
- **Night:** Minimal visibility, maximum tension

### Weather Integration  
- **Rain Effects:** Muddy ground, dripping vegetation
- **Fog:** Reduced visibility, muffled sounds
- **Wind:** Animated vegetation, falling debris
- **Storms:** Dramatic lighting, shelter urgency

---

## NEXT AGENT HANDOFF

**For Agent #07 (Architecture & Interior):**
- Vegetation provides natural building material sources
- Clearings and rock formations offer construction sites
- Cave systems can be expanded into shelters
- Tree platforms enable elevated construction
- Natural barriers guide building placement

**Key Integration Points:**
- Building materials should match local geology
- Structures must feel integrated, not imposed
- Architecture should enhance, not compete with, natural beauty
- Defensive positions should feel organically placed

---

*Environment Art Pipeline — Agent #06*  
*Transpersonal Game Studio — March 2026*