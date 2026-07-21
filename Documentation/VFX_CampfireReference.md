# VFX Campfire Reference Documentation

## Visual Description
Realistic prehistoric campfire particle effect for survival game environment.

### Fire Component
- **Base flames**: Bright orange (#FF6600) and yellow (#FFCC00) colors
- **Flame height**: 0.8-1.2 meters above wood logs
- **Movement**: Dynamic flickering with natural turbulence
- **Intensity**: Medium brightness suitable for night illumination

### Smoke Component  
- **Color**: White to light gray gradient (#FFFFFF to #CCCCCC)
- **Behavior**: Rising vertically then dispersing with wind
- **Density**: Semi-transparent, realistic volume
- **Height**: 2-4 meters before full dispersion

### Ember/Sparks Component
- **Particles**: Small glowing orange dots (#FF3300)
- **Behavior**: Occasional sparks floating upward
- **Lifespan**: 2-3 seconds before fading
- **Frequency**: 5-10 sparks per second during active burning

### Wood Logs
- **Arrangement**: 4-6 logs in circular/triangular formation  
- **Texture**: Charred wood with glowing ember spots
- **Material**: Dark brown (#2D1B14) with red-hot areas (#CC3300)

## Technical Implementation
- **Niagara System**: NS_Fire_Campfire
- **LOD Levels**: 3 (High/Medium/Low detail)
- **Performance**: Optimized for 60fps with 200-500 particles max
- **Audio Sync**: Crackling sound intensity matches visual flame size

## Usage Context
- Player camps and rest areas
- Cooking stations for food preparation  
- Light source during night gameplay
- Warmth mechanic for survival stats
- Social gathering points for NPCs