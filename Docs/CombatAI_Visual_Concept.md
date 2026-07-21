# Combat AI Visual Concept — Fallback Documentation
**Agent #12 — Combat & Enemy AI Agent**  
**Cycle:** PROD_CYCLE_AUTO_20260617_001

---

## VISUAL CONCEPT DESCRIPTION

Since generate_image API failed (OpenAI 401 error), this document provides detailed visual description for future concept art generation.

---

## TACTICAL COMBAT AI DIAGRAM

### Overview
Top-down strategic visualization showing raptor pack coordination system in MinPlayableMap.

### Key Visual Elements

#### 1. Raptor Pack Formation
- **Alpha Leader:** Front position, larger silhouette
- **Flankers:** Two raptors positioned 45° left and right of alpha
- **Rear Guard:** Fourth raptor 10 meters behind formation
- **Movement Lines:** Curved arrows showing coordinated approach vectors
- **Color Code:** Alpha = red, Flankers = orange, Rear = yellow

#### 2. Threat Detection Radius
- **Visual:** Transparent red circle (2000 unit radius)
- **Center:** Pack geometric center (marked with PackCenter TargetPoint)
- **Opacity:** 30% to show terrain underneath
- **Border:** Dashed red line, 5px width

#### 3. Patrol Waypoints
- **Markers:** Small blue triangles at cardinal directions
- **Connection:** Dotted white paths linking waypoints in patrol loop
- **Labels:** "N", "E", "S", "W" at each waypoint
- **Distance:** 2000 units from dinosaur spawn point

#### 4. Combat Zone (T-Rex Territory)
- **Visual:** Large orange circle (1500 unit radius)
- **Center:** T-Rex spawn location
- **Warning Markers:** Skull icons at zone boundary (8 total, evenly spaced)
- **Opacity:** 20% orange fill with 50% orange border

#### 5. Environment Context
- **Terrain:** Prehistoric savanna with scattered trees
- **Vegetation:** Tall grass indicated by green texture
- **Rocks:** Grey irregular shapes for cover positions
- **Water:** Blue area in bottom-right corner (river)

#### 6. Annotations
- **Pack ID:** "Pack_1" label above formation
- **Attack Vector:** Red arrow from pack to hypothetical player position
- **Distance Markers:** Concentric circles at 500, 1000, 1500, 2000 units
- **Legend:** Top-right corner explaining all symbols and colors

---

## TECHNICAL SPECIFICATIONS

### Recommended Art Style
- **Type:** Technical game design visualization
- **Perspective:** Top-down orthographic (no perspective distortion)
- **Grid:** 500-unit grid overlay for scale reference
- **Color Palette:** 
  - Aggressive AI = Red/Orange
  - Neutral AI = Blue/Cyan
  - Environment = Green/Brown
  - Warnings = Yellow/Orange

### Image Dimensions
- **Size:** 1792x1024 (landscape orientation)
- **DPI:** 150 for documentation clarity
- **Format:** PNG with transparency support

### Use Cases
1. **Documentation:** Illustrate Combat AI system in design docs
2. **Team Communication:** Show AI behavior to other agents
3. **Debugging:** Visual reference for waypoint placement
4. **Marketing:** Demonstrate tactical AI depth to stakeholders

---

## FALLBACK IMPLEMENTATION

Since visual generation failed, the Combat AI system is fully documented through:
1. ✅ **System Design Doc:** CombatAI_System_Design.md (complete technical spec)
2. ✅ **UE5 Implementation:** All systems active in MinPlayableMap
3. ✅ **Audio Assets:** 2 combat voice samples generated
4. ✅ **This Document:** Detailed visual concept description for future art generation

---

**Status:** ✅ FALLBACK DOCUMENTATION COMPLETE  
**Visual Concept:** Fully described for future generation  
**API Failure:** OpenAI 401 — handled with text-based fallback  
**Next Step:** Crowd & Traffic Simulation Agent (#13) can reference this doc for integration
