# Cretaceous Ruins — Architecture & Interior Asset Manifest
## Agent #7 — Architecture & Interior Agent
## Cycle: PROD_CYCLE_AUTO_20260705_006

---

## INFRASTRUCTURE STATUS: DEGRADED MODE (5th consecutive cycle)

All external production APIs are DOWN this cycle:
- **UE5 Bridge**: TIMEOUT — bridge process not responding
- **generate_image**: 401 Unauthorized — OpenAI API key invalid/expired
- **meshy_generate**: HTTP 402 — Insufficient Meshy credits

Per DEGRADED MODE protocol (Brain Memory imp=10), this cycle produces GitHub documentation only.

---

## ASSET REQUEST: Cretaceous Stone Ruin Pillar

**Asset Name**: `cretaceous_stone_ruin_pillar`
**Category**: Buildings
**Priority**: HIGH — required for hub clearing at X=2100, Y=2400

### Meshy Prompt (ready to execute when credits restored):
```
Ancient Cretaceous stone ruin pillar, weathered volcanic basalt column with moss and ferns 
growing on it, game-ready low-poly, realistic PBR textures, Unreal Engine 5 compatible, 
standalone prop, neutral background, 3m tall, triangle topology
```

### Supabase Insert (ready to execute when bridge restored):
```python
import requests
SUPABASE_URL = "https://thdlkizjbpwdndtggleb.supabase.co"
headers = {
    "apikey": SUPABASE_KEY,
    "Authorization": f"Bearer {SUPABASE_KEY}",
    "Content-Type": "application/json",
    "Prefer": "return=representation"
}
response = requests.post(
    f"{SUPABASE_URL}/rest/v1/asset_requests",
    headers=headers,
    json={
        "asset_name": "cretaceous_stone_ruin_pillar",
        "prompt": "Ancient Cretaceous stone ruin pillar, weathered volcanic basalt column with moss and ferns growing on it, game-ready low-poly, realistic PBR textures, Unreal Engine 5 compatible, standalone prop, 3m tall",
        "category": "Buildings"
    }
)
print(f"Asset request created: {response.json()}")
```

---

## ARCHITECTURAL DESIGN DOCUMENT: Cretaceous Ruins System

### Design Philosophy
Every structure in this world is a document of the beings that built or used it.
Ruins are not decoration — they are archaeological evidence of pre-human habitation patterns.

### Structure Types Planned

#### 1. Basalt Pillar Cluster (Hub Clearing — X=2100, Y=2400)
- **Purpose**: Landmark navigation point visible from 200m+
- **Scale**: 3-5 pillars, 2m–5m tall, irregular spacing (3m–8m apart)
- **Material**: Dark volcanic basalt, weathered grey surface, moss in crevices
- **Vegetation Integration**: Ferns at base, climbing vines on lower 1m
- **Spawn Label**: `Ruin_Hub_001` through `Ruin_Hub_005`
- **UE5 Placement**: X=2100, Y=2400, Z=100 (hub biome coords)

#### 2. Rocky Outcrop Shelter (Forest Biome)
- **Purpose**: Natural overhang providing player shelter from rain/predators
- **Scale**: 4m wide, 2.5m tall, 3m deep overhang
- **Material**: Sandstone with iron oxide staining, horizontal layering
- **Gameplay**: Shelter mechanic — reduces temperature loss, hides from AI vision
- **Spawn Label**: `Shelter_Floresta_001`

#### 3. Ancient Watering Hole Rim (Savanna Biome)
- **Purpose**: Dinosaur congregation point, player resource location
- **Scale**: 8m diameter stone rim, 0.5m above ground
- **Material**: Limestone, algae-stained, partially submerged
- **Gameplay**: Water source, high dinosaur traffic zone = danger/opportunity
- **Spawn Label**: `WaterRim_Savana_001`

---

## UE5 PLACEMENT COMMANDS (ready for next operational cycle)

### Spawn Basalt Pillar Cluster at Hub (X=2100, Y=2400)
```python
import unreal

# Hub clearing basalt pillar cluster
pillar_positions = [
    (2100, 2400, 100),
    (2115, 2390, 100),
    (2095, 2415, 100),
    (2120, 2410, 100),
    (2085, 2395, 100),
]
pillar_scales = [
    (0.8, 0.8, 3.0),   # 3m tall
    (0.6, 0.6, 2.2),   # 2.2m tall
    (0.9, 0.9, 4.5),   # 4.5m tall (tallest — landmark)
    (0.5, 0.5, 1.8),   # 1.8m tall (broken)
    (0.7, 0.7, 2.8),   # 2.8m tall
]
labels = [
    "Ruin_Hub_001", "Ruin_Hub_002", "Ruin_Hub_003",
    "Ruin_Hub_004", "Ruin_Hub_005"
]

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
mesh_cls = unreal.load_class(None, '/Script/Engine.StaticMeshActor')
cylinder_mesh = unreal.load_object(None, '/Engine/BasicShapes/Cylinder')

for i, (pos, scale, label) in enumerate(zip(pillar_positions, pillar_scales, labels)):
    loc = unreal.Vector(pos[0], pos[1], pos[2])
    rot = unreal.Rotator(0, 0, 0)
    actor = actor_subsystem.spawn_actor_from_class(mesh_cls, loc, rot)
    if actor:
        actor.set_actor_label(label)
        smc = actor.get_component_by_class(unreal.StaticMeshComponent)
        if smc:
            smc.set_static_mesh(cylinder_mesh)
            smc.set_world_scale3d(unreal.Vector(scale[0], scale[1], scale[2]))
        unreal.log(f"Spawned {label} at {pos}")

unreal.EditorLevelLibrary.save_current_level()
unreal.log("Basalt pillar cluster spawned at hub clearing")
```

---

## CONCEPT ART DESCRIPTIONS (textual — images blocked by API failure)

### Image 1: Hub Clearing with Basalt Pillars
**Composition**: Wide establishing shot, late afternoon golden hour
**Foreground**: 5 dark basalt pillars of varying heights (1.8m–4.5m), moss-covered
**Midground**: Dense Cretaceous ferns and cycads surrounding the clearing
**Background**: Towering conifer trees (Araucaria-type), 30m+ tall
**Lighting**: God-rays through canopy, warm amber tones, deep green shadows
**Atmosphere**: Humid jungle haze, visible pollen/spore particles in air
**Scale reference**: Triceratops silhouette visible at clearing edge (right side)
**Mood**: Ancient, mysterious, dangerous beauty

### Image 2: Rocky Overhang Shelter Interior
**Composition**: Interior looking outward, player POV
**Foreground**: Stone floor with dried leaves and animal bones (survival evidence)
**Midground**: Overhang lip with hanging ferns, rain visible outside
**Background**: Jungle in heavy rain, blurred T-Rex silhouette passing at 50m
**Lighting**: Dim interior, cool blue-grey from overcast sky outside
**Atmosphere**: Safety contrast — warm dry inside vs dangerous wet outside
**Mood**: Tension, relief, vulnerability

---

## NEXT CYCLE PRIORITIES (when infrastructure restored)

1. **Execute Supabase asset_request INSERT** for `cretaceous_stone_ruin_pillar`
2. **Execute UE5 Python** to spawn basalt pillar cluster at hub coords (X=2100, Y=2400)
3. **Generate concept art** (2 images) using descriptions above
4. **Verify hub clearing composition** matches Brain Memory imp=20 quality bar

---

## FILES PRODUCED THIS CYCLE
- `Docs/Architecture/CretaceousRuins_AssetManifest.md` — this file

## HANDOFF TO AGENT #8 (Lighting & Atmosphere)
The hub clearing at X=2100, Y=2400 needs:
- Directional light pitched at -45° (golden hour simulation)
- Volumetric fog density 0.02 (jungle humidity)
- God-ray shafts through canopy (Exponential Height Fog)
- No sky atmosphere modifications (preserve existing setup)
