# Cretaceous Period Atmospheric Lighting Reference

## Visual Style Guide for Transpersonal Game Lighting System

### Primary Lighting Characteristics
- **Sun Color**: Warm amber-gold (RGB: 255, 240, 200) - mimics Cretaceous atmosphere with higher CO2 content
- **Sun Intensity**: 5.0 lux - bright but not harsh, creating natural prehistoric ambiance
- **Atmospheric Scattering**: Enhanced blue-orange gradient during dawn/dusk transitions
- **Volumetric Fog**: Subtle density (0.02) with warm inscattering (RGB: 180, 160, 120)

### Time of Day Progression
1. **Dawn (05:00-07:00)**: Deep orange horizon, long shadows, cool blue sky dome
2. **Morning (07:00-10:00)**: Golden hour lighting, warm directional shadows
3. **Midday (10:00-14:00)**: Bright overhead sun, minimal atmospheric scattering
4. **Afternoon (14:00-17:00)**: Warm side-lighting, enhanced color saturation
5. **Dusk (17:00-19:00)**: Deep amber sun, purple-orange sky gradient
6. **Night (19:00-05:00)**: Cool moonlight, star field, bioluminescent accents

### Environmental Lighting Zones
- **Forest Canopy**: Dappled light through prehistoric ferns and cycads
- **Open Plains**: Direct sunlight with atmospheric haze
- **River Valleys**: Reflected light from water surfaces, misty fog layers
- **Rocky Outcrops**: Sharp shadow contrasts, warm stone reflections
- **Cave Entrances**: Dramatic light-to-dark transitions

### Technical Implementation Notes
- Uses UE5 Lumen for real-time global illumination
- SkyAtmosphere component for realistic sky dome
- ExponentialHeightFog for atmospheric depth
- Volumetric lighting for god rays through vegetation
- Dynamic time-of-day system with smooth transitions

### Mood and Emotional Impact
The lighting system emphasizes the ancient, untamed nature of the Cretaceous world while maintaining visual clarity for gameplay. Warm tones suggest a hospitable but dangerous environment where survival depends on adaptation to natural cycles.