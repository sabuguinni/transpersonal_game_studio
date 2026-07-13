
# Quest & Mission Designer — Agent #14
## Cycle PROD_CYCLE_AUTO_20260713_002

**Bridge status: UP.** 4x `ue5_execute` Python calls (command IDs 33064–33067), all `completed` in ~3s each, zero timeouts, zero camera manipulation, zero .cpp/.h writes. Plus 2x `text_to_speech` calls for quest NPC dialogue (audio generated successfully server-side; Supabase upload returned the known 403 "Invalid Compact JWS" storage error — same infra issue logged in prior cycles, not an agent error).

### What was done in the live UE5 world

1. **Bridge validation + audit** — confirmed live editor world, scanned all actors within 3500u of the content hub (2100,2400) for pre-existing `Herd_*`, `Quest_*`, and resource-named actors. Full dump written to `/tmp/quest_audit_20260713_002.txt` and read back into the log for traceability (per naming/dedup discipline).
2. **Reuse-first quest objective placement** — found and reused Agent #13's `Herd_HubGrazing_01`-tagged Brachiosaurus group and Agent #12's `Combat_PackCoordination_Flank`-tagged raptor pack. **No dinosaur actors were duplicated.** Three quest-only marker/pickup actors were spawned instead:
   - `QuestMarker_TrackHerd_001` — small sphere marker offset near the existing herd (tags: `Quest_TrackHerd`, `Quest_ObjectiveMarker`, `Quest_NonHostile`). Anchors the **"Track the Herd"** exploration/observation quest.
   - `QuestMarker_ScoutRaptors_001` — sphere marker offset near the existing raptor pack (tags: `Quest_ScoutRaptors`, `Quest_ObjectiveMarker`, `Quest_DangerZone`). Anchors the **"Scout the Raptor Pack"** reconnaissance/danger-contrast quest, directly building on #13's suggestion to pair safe herbivore herd behavior against a dangerous carnivore pack at the same hub.
   - Three crafting resource pickups — `Rock_Hub_001`, `Stick_Hub_001`, `Leaf_Hub_001` (tags: `Quest_Resource_Rock/Stick/Leaf`, `Quest_CraftingMaterial`, `Quest_ObjectiveMarker`) scattered within ~150u of the hub center, tying survival crafting (per Agent #14's parallel crafting directive scope) directly to a quest pickup objective: **"Gather Crafting Materials."**
3. **Verification pass** — re-queried all `Quest_*`-tagged actors, logged their labels/locations/tags, and checked for duplicate actor labels across the whole level. No duplicates found.
4. Level saved after mutation.

### Quest Design — Three Objectives This Cycle

#### 1. "Track the Herd" (Exploration / Observation)
- **Emotional arc**: Curiosity → calm competence. The player learns to read animal behavior (bunching, quieting) as an early-game survival skill, not combat.
- **Objective anchor**: `QuestMarker_TrackHerd_001`, reusing #13's `Herd_HubGrazing_01` Brachiosaurus group.
- **NPC voice line (CampElder)**: "Track that herd, stranger. If you can follow their trail without spooking them, you'll learn where the water runs and where the ground stays soft come the dry season. Watch their pace — when they bunch up tight and go quiet, something's hunting close by." *(generated via ElevenLabs TTS this cycle)*
- **Payoff**: Teaches the player that herd behavior is a readable warning system for predator proximity — mechanically useful, narratively grounded in realistic ecology, zero mysticism.

#### 2. "Scout the Raptor Pack" (Reconnaissance / Danger)
- **Emotional arc**: Caution → controlled fear → earned confidence. Deliberately paired against objective #1 at the same hub location to give the player an immediate safe-vs-dangerous contrast without traveling far.
- **Objective anchor**: `QuestMarker_ScoutRaptors_001`, reusing #12's `Combat_PackCoordination_Flank` raptor tags.
- **NPC voice line (HunterScout)**: "Those raptors run in a pack, and packs don't forgive mistakes. Scout their den from a distance first — count how many there are, mark which way the wind blows, and never let them see you before you see them. Bring back what you learn, not a fight you can't win." *(generated via ElevenLabs TTS this cycle)*
- **Payoff**: Rewards patience and observation over combat, reinforcing realistic pack-hunter danger without turning the raptors into scripted setpieces.

#### 3. "Gather Crafting Materials" (Survival / Crafting Tie-in)
- **Objective anchor**: `Rock_Hub_001`, `Stick_Hub_001`, `Leaf_Hub_001` pickups near hub.
- **Design intent**: Direct feed into the crafting system (Stone Axe = 2 rocks + 1 stick, Campfire = 3 sticks, Water Container = 1 rock + 1 leaf). This is the quest-layer hook for the crafting system's resource loop — picking these up should trigger inventory events once the crafting/inventory C++ is live.

### Decisions & justification
- No new C++ (QuestManager class, dialogue trees, etc.) was written this cycle — per `hugo_no_cpp_h_v2`, this headless editor never recompiles C++, so all quest logic is expressed as live, inspectable Actor tags + transforms + generated NPC voice lines, exactly matching the pattern #13 validated.
- Strict reuse-first: audited before spawning, reused existing `Herd_HubGrazing_01` and `Combat_PackCoordination_Flank` tagged actors rather than creating duplicate quest-specific dinosaur instances (the exact anti-pattern flagged in `hugo_naming_dedup_v2`).
- Video walkthrough tool (`heygen_create_video`) referenced in the production mandate is not present in the available toolset this session — flagging this gap rather than fabricating a call.

### Files created
- `Docs/QuestDesign/QuestObjectives_PROD_20260713_002.md` (this file)

### Dependencies / next agent (#15 Narrative & Dialogue Agent)
- Use `Quest_TrackHerd`, `Quest_ScoutRaptors`, and `Quest_Resource_*` tags as anchors for full dialogue trees and lore framing — the CampElder and HunterScout voice lines above are placeholder quest-giver lines ready for narrative expansion.
- Crafting recipe logic (Stone Axe, Campfire, Water Container) still needs actual inventory/crafting C++ implementation from a future compile-enabled cycle — flag to #01/#02 whether the build pipeline can be re-enabled for this.
- True quest state tracking (objective completion, quest log UI) requires a QuestManager subsystem — currently expressed only via Actor tags since C++ changes don't take effect in this headless instance.
