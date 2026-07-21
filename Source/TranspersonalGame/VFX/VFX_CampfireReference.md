# Campfire VFX Reference Documentation

## Visual Description
Realistic prehistoric campfire particle effect with the following characteristics:

### Fire Core
- Bright orange and yellow flames dancing upward (300-500 units height)
- Base temperature: 1200°C orange core fading to 800°C yellow tips
- Flame shape: Irregular, organic movement with realistic physics
- Turbulence: Wind-affected flickering and swaying

### Particle Elements
- **Embers**: Glowing orange sparks (5-15 units) floating upward
- **Sparks**: Small crackling particles with brief lifespan (0.5-2 seconds)
- **Smoke**: Gray-white wisps rising and dispersing (opacity 0.3-0.8)
- **Ash**: Fine particles settling around fire base

### Lighting Properties
- Warm orange glow (color temperature 2000K)
- Dynamic light radius: 400-600 units
- Volumetric lighting on smoke for atmospheric depth
- Flickering intensity variation (0.7-1.0 multiplier)

### Audio Integration
- Crackling wood sounds (40-60 dB ambient)
- Occasional pop/spark audio cues
- Low-frequency fire rumble (sub-bass)

## Technical Implementation
- Niagara system with 3 emitters: Fire, Embers, Smoke
- LOD levels: High (close), Medium (50m), Low (100m+)
- Performance target: <2ms GPU time at 1080p