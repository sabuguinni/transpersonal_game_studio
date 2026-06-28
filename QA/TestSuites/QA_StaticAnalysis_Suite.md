# QA Static Analysis Test Suite
**Agent:** #18 — QA & Testing Agent  
**Version:** 1.0  
**Purpose:** Tests that run WITHOUT UE5 bridge (fallback mode)

---

## Suite 1 — File Pair Completeness
Every .h MUST have a matching .cpp. Run this check when bridge is offline.

### Expected Pairs
```
TranspersonalGame.h          → TranspersonalGame.cpp          ✅
TranspersonalGameState.h     → TranspersonalGameState.cpp     ✅
TranspersonalCharacter.h     → TranspersonalCharacter.cpp     ✅
PCGWorldGenerator.h          → PCGWorldGenerator.cpp          ✅
FoliageManager.h             → FoliageManager.cpp             ✅
CrowdSimulationManager.h     → CrowdSimulationManager.cpp     ✅
ProceduralWorldManager.h     → ProceduralWorldManager.cpp     ✅
BuildIntegrationManager.h    → BuildIntegrationManager.cpp    ✅
```

### Orphan Headers (no .cpp) — FAIL if found
- Any .h with UCLASS/USTRUCT/UENUM that has no matching .cpp = COMPILATION FAILURE

---

## Suite 2 — SharedTypes.h Compliance
All cross-agent types MUST be in SharedTypes.h. Check before creating any struct/enum.

### Known Types in SharedTypes.h (22 types as of cycle 003)
- Verify no duplicate definitions exist in other headers
- Verify all agents use SharedTypes.h for shared enums/structs

---

## Suite 3 — UE5 Compilation Rules Checklist
Run mentally before any github_file_write:

| Rule | Check |
|------|-------|
| USTRUCT/UENUM at global scope only | ✅ |
| Unique type names with QA_ prefix | ✅ |
| No engine type redefinitions | ✅ |
| .generated.h is LAST include | ✅ |
| No escaped quotes in macros | ✅ |
| No spaces in identifiers | ✅ |
| CallInEditor is bare flag | ✅ |
| BlueprintReadOnly private = AllowPrivateAccess | ✅ |
| Script size < 8000 chars | ✅ |
| One GENERATED_BODY per UCLASS | ✅ |

---

## Suite 4 — Gameplay Milestone 1 Validation
Tests to run when bridge comes back online:

```python
import unreal

# Test 1: TranspersonalCharacter loadable
cls = unreal.load_class(None, '/Script/TranspersonalGame.TranspersonalCharacter')
assert cls is not None, "FAIL: TranspersonalCharacter not found"
unreal.log("PASS: TranspersonalCharacter loadable")

# Test 2: TranspersonalGameState loadable
gs = unreal.load_class(None, '/Script/TranspersonalGame.TranspersonalGameState')
assert gs is not None, "FAIL: TranspersonalGameState not found"
unreal.log("PASS: TranspersonalGameState loadable")

# Test 3: PCGWorldGenerator loadable
pcg = unreal.load_class(None, '/Script/TranspersonalGame.PCGWorldGenerator')
assert pcg is not None, "FAIL: PCGWorldGenerator not found"
unreal.log("PASS: PCGWorldGenerator loadable")

# Test 4: MinPlayableMap exists
map_exists = unreal.EditorAssetLibrary.does_asset_exist('/Game/Maps/MinPlayableMap')
assert map_exists, "FAIL: MinPlayableMap not found"
unreal.log("PASS: MinPlayableMap exists")

# Test 5: Actor count in MinPlayableMap >= 20
actors = unreal.EditorLevelLibrary.get_all_level_actors()
assert len(actors) >= 20, f"FAIL: Only {len(actors)} actors (expected >= 20)"
unreal.log(f"PASS: {len(actors)} actors in level")

# Test 6: Directional light exists
dl_class = unreal.load_class(None, '/Script/Engine.DirectionalLight')
dls = [a for a in actors if isinstance(a, dl_class)]
assert len(dls) >= 1, "FAIL: No DirectionalLight in level"
unreal.log(f"PASS: {len(dls)} DirectionalLight(s) found")

# Test 7: PlayerStart exists
ps_class = unreal.load_class(None, '/Script/Engine.PlayerStart')
pss = [a for a in actors if isinstance(a, ps_class)]
assert len(pss) >= 1, "FAIL: No PlayerStart in level"
unreal.log(f"PASS: {len(pss)} PlayerStart(s) found")
```

---

## Suite 5 — Anti-Pattern Detection
Flags code that violates project rules:

### Forbidden Content (Anti-Hallucination)
- [ ] No "consciousness", "meditation", "spiritual" in class names
- [ ] No "chakra", "aura", "mystical" in property names
- [ ] No "awakening", "transcendence" in function names
- [ ] All content passes "National Geographic prehistoric documentary" test

### Forbidden Code Patterns
- [ ] No `#if 0` blocks disabling entire classes
- [ ] No empty stub implementations (must have real logic)
- [ ] No HTTP calls from UE5 Python scripts
- [ ] No `unreal.find_class()` (use `unreal.load_class()`)
- [ ] No `unreal.SystemLibrary.get_project_name()`

---

## Bridge Recovery Procedure
When bridge is offline, Integration Agent #19 must:

1. Verify UE5 Editor is running (not crashed)
2. Check Remote Control plugin: Edit → Plugins → "Remote Control API" → Enabled
3. Verify port 30010 is not blocked by firewall
4. Restart UE5 Editor if needed
5. Re-run bridge validation: `import unreal; print('bridge_ok')`

---

## QA Sign-off Criteria
**PASS:** All Suite 1-3 checks green + bridge online + Suite 4 all pass  
**CONDITIONAL PASS:** Suite 1-3 green + bridge offline (infrastructure issue)  
**FAIL/BLOCK:** Any Suite 1-3 violation OR Suite 4 failure when bridge is online
