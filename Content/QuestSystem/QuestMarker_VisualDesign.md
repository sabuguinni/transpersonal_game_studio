# Quest Marker Visual Design

## Concept Description
Since generate_image API failed, documenting visual design for implementation by VFX Agent #17.

### Quest Marker Appearance
- **Shape**: Glowing golden tribal symbol (stylized spear crossed with sun rays)
- **Size**: 2m diameter, visible from 500m distance
- **Animation**: Gentle pulsing glow (0.8s cycle), rotating slowly (30 degrees/sec)
- **Particle Effects**: Golden embers rising upward, tribal smoke wisps
- **Visibility**: Always visible through terrain (X-ray shader when occluded)

### Marker Types by Mission
1. **Hunt Markers**: Red-orange glow, aggressive pulse, danger symbol
2. **Escort Markers**: Blue-green glow, calm pulse, protection symbol
3. **Gather Markers**: Yellow-gold glow, steady pulse, resource symbol

### Technical Implementation (for VFX Agent)
- Use Niagara particle system for embers and smoke
- Material with emissive channel for glow
- Blueprint actor with rotating movement component
- LOD system: Full detail <100m, simplified 100-300m, billboard >300m
- Audio: Subtle ambient hum (spatial audio, 50m radius)

### UI Integration
- Minimap icon: Simplified 2D version of 3D marker
- Compass indicator when marker off-screen
- Distance text overlay when within 100m
- Quest objective text appears on hover (5m range)

## Placeholder Asset
Until VFX Agent implements, use UE5 default TargetPoint with:
- Billboard sprite scaled 2x
- Orange emissive material
- Simple rotation component
