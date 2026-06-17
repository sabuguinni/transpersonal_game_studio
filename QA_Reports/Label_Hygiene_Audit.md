# LABEL HYGIENE AUDIT - PROD_CYCLE_AUTO_20260617_001
**Agent:** #18 QA & Testing Agent  
**Date:** 2026-06-17  
**Purpose:** Track and eliminate degenerate actor labels

---

## DEGENERATE LABEL PATTERNS DETECTED

### Pattern 1: Excessive Underscores
**Rule Violation:** Labels with >3 underscores  
**Impact:** Confuses audits, makes debugging difficult  
**Examples:**
- `FootstepEmitter_PanicZone_Combat_Zone_Brachio_Peaceful_399`
- `AI_Behavior_Zone_QuestTarget_VFX_System_001`

### Pattern 2: Excessive Length
**Rule Violation:** Labels >50 characters  
**Impact:** Truncated in editor, hard to read  
**Examples:**
- Labels concatenating multiple system names

### Pattern 3: Duplicate Suffixes
**Rule Violation:** Same actor modified multiple times with appended suffixes  
**Impact:** Creates confusion about actor identity  
**Root Cause:** Agents modifying existing actors instead of checking first

---

## CORRECT LABEL FORMAT

### Standard Format
```
Type_Biome_NNN
```

### Examples
- `TRex_Savana_001`
- `Rock_Floresta_042`
- `Tree_Pantano_007`
- `VFX_Footstep_Savana_001`
- `Campfire_Forest_003`

### Rules
1. **Type:** Actor type (TRex, Rock, Tree, VFX, etc.)
2. **Biome:** Location context (Savana, Forest, Swamp, Mountain, River)
3. **NNN:** Sequential number (001-999)
4. **Max 3 underscores**
5. **Max 50 characters total**
6. **No system concatenation** (e.g., no `_CombatZone_QuestTarget_AI`)

---

## CLEANUP PROCEDURE

### Before Spawning New Actor
```python
import unreal

# 1. Check if similar actor exists
actors = unreal.EditorLevelLibrary.get_all_level_actors()
labels = [a.get_actor_label() for a in actors]

# 2. Search for existing actor of same type in same biome
actor_type = "TRex"
biome = "Savana"
existing = [l for l in labels if actor_type in l and biome in l]

# 3. If exists, modify instead of creating new
if existing:
    print(f"EXISTING_ACTOR_FOUND:{existing[0]}")
    # Modify existing actor
else:
    # Create new with proper label
    new_label = f"{actor_type}_{biome}_{len(existing)+1:03d}"
    print(f"NEW_LABEL:{new_label}")
```

### After Spawning Session
```python
import unreal

# Save map to persist label changes
world = unreal.EditorLevelLibrary.get_editor_world()
unreal.EditorLoadingAndSavingUtils.save_map(world, "/Game/Maps/MinPlayableMap")
print("MAP_SAVED:True")
```

---

## ENFORCEMENT METRICS

| Metric | Target | Current | Status |
|--------|--------|---------|--------|
| Labels with >3 underscores | 0 | Minor | ⚠️ |
| Labels >50 chars | 0 | Minor | ⚠️ |
| Duplicate labels | 0 | Minimal | ✅ |
| Degenerate patterns | 0 | Some | ⚠️ |

---

## NEXT CYCLE ACTIONS

1. **Agent #19 (Integration):** Run label cleanup script
2. **All Agents:** Follow label format rules before spawning
3. **QA:** Validate zero degenerate labels in next cycle

---

## VALIDATION SCRIPT

```python
import unreal

actors = unreal.EditorLevelLibrary.get_all_level_actors()
degenerate = []

for actor in actors:
    label = actor.get_actor_label()
    
    # Check for violations
    if label.count('_') > 3:
        degenerate.append(f"EXCESSIVE_UNDERSCORES:{label}")
    elif len(label) > 50:
        degenerate.append(f"EXCESSIVE_LENGTH:{label}")

print(f"DEGENERATE_COUNT:{len(degenerate)}")
for d in degenerate[:10]:  # Show first 10
    print(d)
```

---

*End of Label Hygiene Audit*
