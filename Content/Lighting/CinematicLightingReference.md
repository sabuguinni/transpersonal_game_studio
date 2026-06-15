# Cinematic Lighting System - Reference Guide

## Golden Hour Lighting Setup (Implemented)

### Primary Directional Light (Sun)
- **Position**: Low angle (-15° pitch, 45° yaw)
- **Intensity**: 8.5 lux
- **Color**: Warm golden (RGB: 1.0, 0.85, 0.6)
- **Temperature**: 3200K (warm)
- **Volumetric Scattering**: 2.5 intensity
- **Shadows**: Cast volumetric shadows enabled

### Rim Lighting System
- **Secondary Directional Light**: Cool contrast lighting
- **Position**: Back-lit angle (25° pitch, -135° yaw)
- **Intensity**: 3.5 lux
- **Color**: Cool blue-white (RGB: 0.8, 0.9, 1.0)
- **Temperature**: 6500K (cool)
- **Purpose**: Creates silhouette definition and depth

### Atmospheric Fog Enhancement
- **Density**: 0.045 (increased for cinematic atmosphere)
- **Height Falloff**: 0.15 (softer gradient)
- **Inscattering Color**: Warm amber (RGB: 0.9, 0.7, 0.4)
- **Directional Inscattering**: 12.0 exponent for strong god rays
- **Volumetric Properties**:
  - Scattering Distribution: 0.6 (forward scattering)
  - Albedo: Warm tone (RGB: 0.95, 0.85, 0.65)
  - Scattering Intensity: 2.8
  - Extinction Scale: 3.2

### Sky Atmosphere Configuration
- **Rayleigh Scattering**: 0.8 scale (reduced for warmth)
- **Mie Scattering**: 0.015 scale (increased for atmospheric haze)
- **Mie Anisotropy**: 0.85 (forward scattering for sun disk)
- **Sky Luminance**: 2.2 factor (brighter sky dome)
- **Ground Albedo**: Earthy brown (RGB: 0.25, 0.2, 0.15)

### Cinematic Light Shafts
- **Count**: 3 volumetric spot lights
- **Intensity**: 15.0 lux each
- **Color**: Warm golden (RGB: 1.0, 0.9, 0.7)
- **Temperature**: 3400K
- **Cone Angles**: 8° inner, 25° outer
- **Attenuation**: 3000 unit radius
- **Volumetric Scattering**: 4.0 intensity for visible beams

## Mood and Atmosphere Goals

### Visual Targets
- **Time of Day**: Late afternoon golden hour
- **Weather**: Clear with atmospheric haze
- **Mood**: Warm, cinematic, slightly nostalgic
- **Contrast**: High dynamic range with deep shadows and bright highlights
- **Color Palette**: Warm oranges/ambers contrasted with cool blues

### Technical Implementation
- **Lumen Global Illumination**: Enabled for realistic bounce lighting
- **Volumetric Fog**: Essential for light shaft visibility
- **Shadow Quality**: High resolution for sharp cast shadows
- **Temporal Upsampling**: Enabled for smooth volumetric effects

### Performance Considerations
- **Light Count**: 5 total (1 sun, 1 rim, 3 shafts)
- **Volumetric Quality**: Medium-High (balanced for 60fps)
- **Shadow Distance**: 5000 units maximum
- **Fog Density**: Optimized for visibility vs performance

## Integration with Gameplay

### Dynamic Elements
- **Time of Day**: System ready for day/night cycle integration
- **Weather Transitions**: Fog density can be animated for weather changes
- **Seasonal Variation**: Color temperature can shift for different seasons
- **Dramatic Moments**: Light shaft intensity can be boosted for key scenes

### Dinosaur Lighting
- **Rim Lighting**: Enhances dinosaur silhouettes against sky
- **Volumetric Shadows**: Creates realistic shadow casting on terrain
- **Subsurface Scattering**: Ready for skin shader integration
- **Eye Reflection**: Proper lighting for realistic eye rendering

This lighting system provides the cinematic foundation for the prehistoric world, emphasizing the dramatic beauty and danger of the Cretaceous period through masterful use of light and atmosphere.