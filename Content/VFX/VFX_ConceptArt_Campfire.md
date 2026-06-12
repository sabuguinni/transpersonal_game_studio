# VFX Concept Art - Prehistoric Campfire Particle Effect

## Visual Description
**Realistic prehistoric campfire particle effect reference for Niagara system creation**

### Flame Characteristics
- **Primary Flames**: Bright orange and red flames dancing upward in natural, organic patterns
- **Height**: 0.5-1.5 meters tall, varying dynamically
- **Color Gradient**: Deep red at base transitioning to bright orange in middle, yellow-white at tips
- **Movement**: Flickering, swaying motion affected by wind simulation

### Smoke Properties
- **Color**: White and light gray smoke billowing upward
- **Density**: Medium density, becoming lighter as it rises
- **Behavior**: Realistic physics-based movement, affected by wind direction
- **Dissipation**: Gradual fade-out over 3-5 meters height

### Ember System
- **Particles**: Glowing orange embers of varying sizes (2-8mm)
- **Behavior**: Float upward with slight random drift
- **Lifetime**: 2-4 seconds before fading
- **Glow**: Bright orange core with soft falloff

### Spark Effects
- **Frequency**: Occasional bursts of sparks (every 3-7 seconds)
- **Direction**: Upward trajectory with slight spread
- **Size**: Small bright points (1-3mm)
- **Color**: Bright yellow-white to orange

### Environmental Integration
- **Ground Interaction**: Heat distortion effect near base
- **Lighting**: Dynamic light casting on surrounding objects
- **Shadows**: Flickering shadows from flame movement
- **Heat Shimmer**: Subtle air distortion above flames

### Technical Specifications for Niagara
- **Particle Count**: 200-500 particles for flames, 100-200 for smoke
- **LOD Levels**: 3 levels (High/Medium/Low) for performance scaling
- **Collision**: Ground collision for embers and sparks
- **Wind Response**: All particles affected by wind vector
- **Performance Target**: 60fps on medium hardware

### Audio Integration
- **Primary Sound**: Crackling and popping of burning wood
- **Secondary**: Low rumbling of flames
- **Ambient**: Subtle hissing of escaping gases
- **Dynamics**: Volume and intensity tied to flame size

This concept serves as the primary reference for creating the NS_Fire_Campfire Niagara system in Unreal Engine 5.