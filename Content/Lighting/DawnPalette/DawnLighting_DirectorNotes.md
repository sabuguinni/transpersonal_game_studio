# Dawn Lighting — Director of Photography Notes
## Agent #8 | PROD_CYCLE_AUTO_20260629_011 | Palette v60

---

## The Intent

Dawn in the Cretaceous is not beautiful. It is **dangerous**.

The world does not wake gently. It snaps awake — predators that hunted through the night are still active, and the ones that hunt by day are beginning to stir. The player standing in the ruins at dawn is caught between two worlds of danger.

The light reflects this. It is not the warm, welcoming sunrise of a nature documentary. It is:
- **Cold** on the western side (night still retreating)
- **Rose-gold and threatening** on the eastern horizon (day arriving like a predator)
- **Volumetric and obscuring** in the ruin area (fog hides what might be watching)

---

## Technical Execution

### Sun Position
- Pitch: **-30°** (below horizon — no direct sunlight yet)
- Only indirect scatter illuminates the scene
- Atmosphere does all the heavy lifting via Lumen sky light

### God-Ray Light Shaft
- **Light_GodRay_RuinCenter** — SpotLight at Z=2500, pointing down at -85°
- Inner cone 8°, outer cone 22° — tight, dramatic shaft
- Cast volumetric shadow: TRUE — pillars will cast visible shadow shafts in fog
- Color: warm gold (1.0, 0.92, 0.75) — the first promise of sunlight

### Cave Entrance
- **Light_CaveEntrance_AmbientGlow** — PointLight, cool blue (0.2, 0.55, 0.9)
- Intensity 1200 — visible but not overwhelming
- Cast volumetric shadow: TRUE — creates depth inside cave mouth
- Emotional read: "Something cold lives in there"

### Ruin Shadow Deepening
- **Light_RuinShadow_AmbientDarken** — RectLight overhead, deep blue (0.15, 0.18, 0.35)
- Intensity 400 — subtle, not a primary light source
- Purpose: ruins should feel **darker** than the surrounding jungle
- The jungle is alive and lit; the ruins are dead and shadowed

### Fog
- Blue-violet (0.35, 0.42, 0.72) — cool, pre-dawn
- Density 0.018 — present but not blinding
- Volumetric fog enabled — god-ray shafts will be visible in it
- Start distance 2000 — player can see immediate surroundings clearly

---

## Color Temperature Progression

| Time | Kelvin | Primary Color | Emotional Read |
|------|--------|---------------|----------------|
| Night | 3200K | Deep blue-silver | Isolation, danger |
| **Dawn** | **4200K** | **Blue-pink** | **Tense anticipation** |
| Sunrise | 2800K | Warm gold-orange | Brief hope |
| Morning | 5500K | Natural white | Clarity, opportunity |
| Midday | 6500K | Cool white | Harsh, exposed |
| Afternoon | 4800K | Amber | Fatigue, urgency |
| Dusk | 3000K | Orange-violet | Melancholy, danger approaching |
| Night | 3200K | Deep blue-silver | Isolation, danger |

---

## Cinematography Reference

**Roger Deakins — Sicario (2015), dawn border crossing sequence**

The light arrives not as comfort but as exposure. What was hidden in darkness is now visible — including you. The warm gold of dawn is not welcoming; it is the light that makes you a target.

Apply this to the Cretaceous: the player who survived the night now faces the dawn exposed. The ruins offer shadow but not safety. The god-rays are beautiful and terrifying simultaneously.

---

## Handoff Notes for Agent #9 (Character Artist)

The dawn palette creates specific demands on character materials:

1. **Skin materials** must respond to Lumen GI — the rose-gold east fill (Light_Dawn_EastRose) should catch on skin highlights
2. **Clothing/fur materials** should have subsurface scattering enabled — dawn light is diffuse and wraps around forms
3. **Eye materials** need specular response — the god-ray shaft will create a catchlight if positioned correctly
4. **Silhouette** is critical at dawn — character should read clearly against the blue-pink sky

The player character at dawn should look like a survivor, not a hero. Tired, alert, small against the ancient ruins.

---

## Files Created This Cycle

| File | Purpose |
|------|---------|
| `LightingPalette_Dawn_v60.json` | Full technical spec for all dawn lights |
| `DawnLighting_DirectorNotes.md` | This file — creative intent and handoff |

## UE5 Actors Spawned

| Actor Label | Type | Purpose |
|-------------|------|---------|
| `Light_GodRay_RuinCenter` | SpotLight | God-ray shaft between pillars |
| `Light_CaveEntrance_AmbientGlow` | PointLight | Cave mouth cool blue glow |
| `Light_RuinShadow_AmbientDarken` | RectLight | Deepen ruin ambient shadow |
| `Light_Dawn_EastRose` | PointLight | Rose-gold eastern horizon fill |
| `Light_Dawn_WestCool` | PointLight | Cool blue western sky fill |
| `Light_Dawn_ZenithBlue` | PointLight | Deep indigo zenith fill |

Total new lights this cycle: **6**
Cumulative lighting actors: **~24** (across all Agent #8 cycles)
