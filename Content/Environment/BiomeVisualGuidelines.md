# Biome Visual Guidelines - Environment Artist Reference

## Forest Biome (Dense Canopy)
**Location:** (5000, 5000) center, ~15000 unit radius
**Lighting Characteristics:**
- Dappled light through canopy (use volumetric god rays)
- Cooler color temperature in shade (blue-green ambient)
- High contrast between sunlit patches and shadow
- Dust particles visible in light shafts

**Atmospheric Elements:**
- Particle system: Forest_Dust_001, Forest_Pollen_002
- Sound zone: Forest_Ambience_Zone
- Fog: Moderate density, green-tinted in shadows

**Visual Storytelling:**
- Ancient, untouched wilderness
- Sense of being watched (dense vegetation)
- Discovery potential (hidden clearings)

## Swamp Biome (Murky Wetlands)
**Location:** (-5000, 15000) center, ~8000 unit radius
**Lighting Characteristics:**
- Diffused light through mist and fog
- Cool blue-green color palette
- Low visibility (atmospheric perspective)
- Potential for bioluminescent accents (fungi, insects)

**Atmospheric Elements:**
- Particle system: Swamp_Mist_003
- Sound zone: Swamp_Ambience_Zone
- Fog: High density near water surface

**Visual Storytelling:**
- Mysterious and slightly unsettling
- Danger lurking beneath murky water
- Decay and renewal cycle (dead logs, new growth)

## Savanna Biome (Open Plains)
**Location:** (25000, -5000) center, ~12000 unit radius
**Lighting Characteristics:**
- Bright, direct sunlight (high exposure)
- Warm golden tones (especially at golden hour)
- Long shadows from scattered trees
- Heat haze effect near ground (future enhancement)

**Atmospheric Elements:**
- Particle system: Savanna_Dust_004
- Sound zone: Savanna_Ambience_Zone
- Fog: Low density, warm color

**Visual Storytelling:**
- Vast, exposed, vulnerable feeling
- Long sightlines (predator awareness)
- Harsh but beautiful environment

## River Biome (Water Corridors)
**Location:** (15000, 25000) center, meandering path
**Lighting Characteristics:**
- Reflective water surface (specular highlights)
- Cooler temperature near water
- Transition zone between biomes
- Dynamic light from water caustics (future enhancement)

**Atmospheric Elements:**
- Sound zone: River_Ambience_Zone
- Fog: Variable density (higher near water)

**Visual Storytelling:**
- Lifeline through dangerous territory
- Congregation point for wildlife
- Natural navigation landmark

## Cross-Biome Transition Zones
**Design Principle:** Gradual blending, not hard boundaries
- Particle systems fade in/out over 500-1000 units
- Sound zones overlap for smooth audio transitions
- Fog density interpolates between biome settings
- Vegetation density gradients (handled by Agent #5)

## Color Palette Reference
**Golden Hour (Current Time):**
- Sun: Warm amber (255, 217, 153)
- Sky: Soft orange to purple gradient
- Shadows: Cool blue-purple (complementary to sun)
- Fog: Warm haze (230, 200, 128)

**Future Time-of-Day Variations:**
- Dawn: Cool pink-orange, low saturation
- Midday: Neutral white sun, blue sky, harsh shadows
- Dusk: Deep orange-red, long shadows, purple sky
- Night: Blue moonlight, deep shadows, bioluminescence

## Performance Budget (Per Biome)
- Particle emitters: Max 2 per biome zone
- Sound triggers: 1 primary zone per biome
- Dynamic lights: Avoid (use baked lighting where possible)
- Fog: Single global system with biome-specific tinting (future)

## Narrative Through Environment
Every visual element should answer: "What happened here before the player arrived?"

**Examples:**
- Fallen log with claw marks → territorial dispute
- Cluster of bones near water → ambush site
- Trampled vegetation path → migration route
- Broken branches at height → large creature passage

## Tools for Future Cycles
- Decal system for ground detail (mud, blood, footprints)
- Foliage wind animation (grass sway, tree movement)
- Water system (rivers, ponds, puddles)
- Weather effects (rain, storms, wind)
- Time-of-day cycle (dynamic sun rotation)

## Integration with Other Systems
- **Agent #5 (World Gen):** Biome boundaries, terrain height
- **Agent #7 (Architecture):** Building placement respects lighting
- **Agent #8 (Lighting):** Will expand on this foundation
- **Agent #12 (Combat AI):** Visibility affects AI behavior
- **Agent #16 (Audio):** Sound zones ready for audio attachment
