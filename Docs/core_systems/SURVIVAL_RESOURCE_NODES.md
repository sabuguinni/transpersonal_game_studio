# Survival Resource Nodes — Core Systems #03
## Cycle: PROD_CYCLE_AUTO_20260618_009

## Overview
Physical world objects placed in MinPlayableMap that the SurvivalComponent queries for survival stat restoration.
All nodes follow label convention: `Type_Subtype_NNN`.

## Placed Nodes

### Water Sources (3)
| Label | Location | Purpose |
|-------|----------|---------|
| WaterSource_River_001 | (-800, 400, 50) | Restores thirst stat — river bank |
| WaterSource_Pond_001 | (600, -900, 30) | Restores thirst stat — standing pond |
| WaterSource_Stream_001 | (-200, 1200, 60) | Restores thirst stat — mountain stream |

### Food Sources (4)
| Label | Location | Purpose |
|-------|----------|---------|
| FoodSource_Berries_001 | (300, 500, 80) | Restores hunger stat — berry bush |
| FoodSource_Berries_002 | (-500, -300, 75) | Restores hunger stat — berry bush |
| FoodSource_Mushrooms_001 | (900, 200, 90) | Restores hunger stat — mushroom cluster |
| FoodSource_Mushrooms_002 | (-100, -800, 70) | Restores hunger stat — mushroom cluster |

### Shelter Sites (2)
| Label | Location | Purpose |
|-------|----------|---------|
| ShelterSite_Cave_001 | (-1200, 600, 100) | Temperature protection, sleep restore |
| ShelterSite_Overhang_001 | (700, -1100, 120) | Partial weather protection |

## SurvivalComponent Integration Rules

### Interaction Radius
- Water sources: 150 units (player must be close to drink)
- Food sources: 100 units (player must be adjacent to forage)
- Shelter sites: 300 units (passive temperature bonus in range)

### Stat Restoration Rates (per second while interacting)
```
WaterSource → Thirst +8.0/s (max 100)
FoodSource_Berries → Hunger +4.0/s (max 100)
FoodSource_Mushrooms → Hunger +3.0/s (max 100, slight risk of poison)
ShelterSite → Temperature stabilise at 20°C, Stamina +2.0/s
```

### Blueprint Query Pattern
```cpp
// In SurvivalComponent::TickComponent
TArray<AActor*> NearbyActors;
UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("WaterSource"), NearbyActors);
for (AActor* Node : NearbyActors)
{
    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Node->GetActorLocation());
    if (Dist < 150.f && bIsInteracting)
    {
        Thirst = FMath::Clamp(Thirst + 8.f * DeltaTime, 0.f, 100.f);
    }
}
```

## Dependency Chain
- **Agent #3 (Core Systems)** — Places resource nodes, defines interaction radii
- **Agent #6 (Environment Artist)** — Replaces placeholder meshes with foliage/rock assets
- **Agent #11 (NPC Behavior)** — Dinosaurs use water/food nodes for their own survival AI
- **Agent #14 (Quest Designer)** — "Find water before nightfall" quest uses WaterSource nodes as objectives

## Architecture Pillars Satisfied
- ✅ P3 (Character System) — SurvivalComponent has physical world targets
- ✅ P4 (Combat) — Shelter sites are safe zones from dinosaur aggression
- ✅ P5 (Quest) — Resource nodes are quest objective anchors
- ✅ P2 (Dinosaur AI) — Dinos also path to water/food nodes (shared ecology)

## Next Steps
1. Agent #6: Replace cube/sphere placeholder meshes with actual berry bush / mushroom / water surface assets
2. Agent #11: Add `WaterSource` / `FoodSource` / `ShelterSite` actor tags so AI can query them
3. Agent #14: Wire `WaterSource_River_001` as objective for tutorial quest "Survive Day One"
