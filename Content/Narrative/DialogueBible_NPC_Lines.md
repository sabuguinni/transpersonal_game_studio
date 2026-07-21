# Transpersonal Game — NPC Dialogue Bible
## Agent #15 — Narrative & Dialogue Agent
## Cycle: PROD_CYCLE_AUTO_20260622_008

---

## TRIBAL ELDER (TribalElder_NPC)
**Position:** Camp center, near main fire
**Role:** Quest giver, lore keeper, survival mentor

### Greeting (First Encounter)
> "Stranger. You carry the smell of the eastern marshes. The great lizards have been restless there — three of our scouts did not return. If you want shelter in our camp, prove you are worth the fire you eat."

### Quest Assignment — Missing Scouts
> "Three suns have passed. Our hunters went east toward the river bend where the tall-necks drink. They carry red-dyed bone markers — you will know them. Bring back the markers if the men are gone. We must know where the beasts roam now."

### Quest Complete — Scouts Found
> "You returned. I did not expect that. Sit. Eat. The meat is from yesterday's kill — a young horn-face that wandered too close to camp. Tomorrow we talk about what you saw out there."

### Warning — Danger Zone
> "Do not go north of the ridge at night. The pack hunters move in darkness. They are not like the great ones — they are small, fast, and they think together. We lost two strong men last season. Strong men."

### Lore — Dinosaur Behavior
> "The great lizards follow the rains. When the sky turns yellow before a storm, the tall-necks move south. That is when the ground shakers follow them. That is when you stay very still and very small."

---

## HUNTER SCOUT (HunterScout_NPC)
**Position:** Camp perimeter, eastern watch post
**Role:** Patrol info, threat reports, combat tips

### Greeting
> "You move quietly for an outsider. Good. Noise kills faster than any beast here. What do you need?"

### Threat Report — Active
> "Two pack hunters were spotted near the dry riverbed this morning. Moving west. If you are heading that direction, go high — they do not look up often. Use the ridgeline."

### Combat Advice — Raptors
> "Do not run from the small ones. They are faster than you. Find a narrow space — a rock crack, a fallen tree. They cannot flank you there. Make them come one at a time."

### Combat Advice — T-Rex
> "If you see the ground shake before you hear it — freeze. The great jaw-beast hunts by movement. I have stood still for half a day while one walked ten steps from me. Half a day. Do not move."

### Resource Tip
> "The eastern slope has flint near the surface — good for blades. But the horn-faces graze there at midday. Go at dawn, take what you need, leave before they wake up."

---

## FORAGER (Forager_NPC)
**Position:** Camp edge, near food storage
**Role:** Resource quests, crafting materials, survival tips

### Greeting
> "You look hungry. We all are. The berry bushes near the south marsh are still producing — but the mud there is deep. Lost a good basket last week. Sank right in."

### Quest — Gather Roots
> "I need the thick yellow roots from the river bank. They grow where the water slows. Boiled, they keep hunger away for two days. Bring back ten handfuls and I will show you how to make the paste that keeps wounds from rotting."

### Quest Complete — Roots Delivered
> "Good. Sit here while I prepare this. Watch carefully — this knowledge is worth more than any stone blade. The paste goes on the wound, then you bind it with the inner bark of the grey tree. Not the outer bark. The inner."

### Crafting Tip — Fire
> "Wet wood makes smoke. Smoke brings attention. Dry the wood first — two days in the sun minimum. A small clean fire is invisible from fifty steps. A smoky fire brings everything that hunts by smell."

### Warning — Plant Hazard
> "The red berries near the tall rocks — do not touch them. Not even the skin. One of our children touched them last season. Three days of shaking and burning. She survived. Barely. Leave them alone."

---

## AMBIENT CAMP DIALOGUE (Overheard Lines)
*These play as environmental audio when player is near camp*

- "The river is lower than last year. The dry season comes early."
- "I heard the ground shakers calling last night. Far away, but calling."
- "My spear broke on the last hunt. I need better flint."
- "The children cannot sleep when the pack hunters howl. Neither can I."
- "Three more families arrived from the northern valley. Something drove them out."
- "The tall-necks are moving in groups of twenty now. Something is pushing them."

---

## DIALOGUE SYSTEM NOTES
- All dialogue triggers on proximity (3m radius) or interaction (E key)
- Lines play in sequence per NPC — no random shuffle for main quest lines
- Ambient lines shuffle randomly, 90-second cooldown between repeats
- No supernatural content — all information is practical survival/resource/threat data
- Character voices: Elder = deep/gravelly, Scout = clipped/alert, Forager = tired/pragmatic
- ElevenLabs voice IDs to assign when quota restored:
  - Elder: Adam (deep, authoritative)
  - Scout: Josh (sharp, military)
  - Forager: Bella (weary, practical)

---

## QUEST HOOKS EMBEDDED IN DIALOGUE
1. **Missing Scouts** — Elder assigns → player goes east → finds markers/bodies → returns
2. **Gather Roots** — Forager assigns → player goes to river bank → avoids herbivores → returns
3. **Threat Report** — Scout assigns → player confirms raptor location → reports back
4. **Flint Cache** — Scout tip → player gathers flint at dawn → crafting resource unlocked

---
*Generated by Agent #15 — Narrative & Dialogue Agent*
*Cycle: PROD_CYCLE_AUTO_20260622_008*
