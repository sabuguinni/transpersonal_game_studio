# Structure Placement Guidelines - Architecture Agent #7

## Biome-Specific Placement Rules

### Savanna Biome (X=50000, Y=50000)
**Characteristics:** Open grassland, scattered trees, high visibility  
**Appropriate Structures:**
- Stone shelters (wind protection)
- Fire pits (cooking, warmth, predator deterrent)
- Observation platforms (elevated lookout points)
- Water collection basins

**Placement Strategy:**
- Near water sources but elevated above flood level
- Clear sightlines for predator detection
- Proximity to tree cover for material gathering

---

### Forest Biome (X=-30000, Y=40000)
**Characteristics:** Dense vegetation, natural caves, limited visibility  
**Appropriate Structures:**
- Cave dwellings (natural shelter)
- Tree platforms (elevated safety)
- Hidden caches (food storage)
- Camouflaged blinds (hunting positions)

**Placement Strategy:**
- Utilize natural rock formations
- Integrate with existing vegetation
- Multiple escape routes from each structure

---

### Plains Biome (X=20000, Y=-20000)
**Characteristics:** Flat terrain, seasonal flooding, migration routes  
**Appropriate Structures:**
- Stone circles (gathering/ritual)
- Marker stones (navigation)
- Raised platforms (flood protection)
- Windbreaks (weather protection)

**Placement Strategy:**
- High ground to avoid seasonal flooding
- Visible from distance for navigation
- Aligned with celestial markers (if ritual purpose)

---

### Swamp Biome (X=-40000, Y=-30000)
**Characteristics:** Waterlogged ground, unstable terrain, dense reeds  
**Appropriate Structures:**
- Raised platforms on stilts
- Reed huts (lightweight, replaceable)
- Floating rafts (mobile shelter)
- Elevated walkways

**Placement Strategy:**
- Always elevated above water level
- Anchored to stable ground or large trees
- Near solid ground for emergency evacuation

---

## Construction Material Logic

### Stone Structures
**When to Use:** Permanent settlements, defensive positions, ritual sites  
**Biomes:** Savanna, Plains, Mountain (where stone naturally occurs)  
**Scale:** Large (2-5m components), heavy, immovable  

### Wood Structures
**When to Use:** Temporary camps, hunting blinds, elevated platforms  
**Biomes:** Forest, Swamp (where wood is abundant)  
**Scale:** Medium (1-3m components), lighter, semi-permanent  

### Reed/Organic Structures
**When to Use:** Seasonal shelters, camouflage, quick construction  
**Biomes:** Swamp, Riverbank  
**Scale:** Small (0.5-2m components), lightweight, disposable  

---

## Structural Integrity Rules

### Load-Bearing Principles
1. Vertical supports must be thicker than horizontal spans
2. Roof angle minimum 15° for water runoff
3. Foundation depth proportional to structure height
4. Wind resistance: lower profile in open biomes

### Human Ergonomics
- Doorway height: 2.0-2.2m (prehistoric humans slightly shorter)
- Interior ceiling height: 2.5-3.0m (allows standing + smoke clearance)
- Fire pit distance from walls: minimum 1.5m (safety)
- Sleeping platform height: 0.5-0.8m (above ground moisture/insects)

---

## Label Naming Convention

**Format:** `StructureType_BiomeName_NNN`

**Examples:**
- `StoneShelter_Savanna_001`
- `CaveEntrance_Forest_002`
- `ReedHut_Swamp_015`
- `StoneCircle_Plains_Center`

**Rules:**
- No spaces in labels
- No concatenation of system names
- Sequential numbering within biome
- Descriptive type prefix

---

## Integration with Other Systems

### Lighting Agent (#8)
- Fire pits require point light actors (warm orange, flickering)
- Cave interiors require darkness volumes
- Stone structures cast long shadows (configure for dramatic lighting)

### Audio Agent (#16)
- Structures should have reverb zones (stone = high reverb, organic = low)
- Wind interaction sounds (whistling through gaps)
- Fire crackle audio at fire pit locations

### Quest Agent (#14)
- Structures can serve as quest objectives (discover ancient site)
- Interior spaces can contain narrative items (tools, art)
- Ritual sites can trigger story events

### NPC Agent (#11)
- Structures define NPC patrol routes (guards circle perimeter)
- Shelters serve as NPC home locations (return at night)
- Gathering sites trigger social behaviors (NPCs congregate)

---

## Performance Considerations

### Actor Count Management
- Each structure = 3-10 actors (walls, roof, props)
- Use instanced static meshes for repeated elements (stone circle pillars)
- Combine small props into single actor where possible

### LOD Strategy
- Distant structures (>5000cm): single merged mesh
- Medium range (1000-5000cm): simplified geometry
- Close range (<1000cm): full detail + interior props

### Collision Optimization
- Use simple box collision for walls/floors
- No collision on decorative elements (moss, cracks)
- NavMesh should path around structures, not through
