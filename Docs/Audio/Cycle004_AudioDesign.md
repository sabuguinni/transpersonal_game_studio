# Audio Design — Cycle 004 (Agent #16)

## Bridge Status
HEALTHY — both `ue5_execute` Python calls completed cleanly (audit ~3s, tag-application+save ~9s), zero timeouts.

## Voice Lines Generated (ElevenLabs TTS)
Both synthesized successfully server-side. **Supabase upload again failed with `403 Invalid Compact JWS`** — same infra issue reported by Agent #14 and #15 for three consecutive cycles now. Base64 audio payload was returned but not persisted to a public URL. This is an infrastructure-side auth/token issue (JWS signing on the Supabase Storage bridge), not an agent-side error, and needs a fix by whoever owns the storage service credentials.

1. **Narrator_TrexProximity** — "Something big just moved past the tree line. Feel that? The ground doesn't shake like that for nothing. Stay low, stay quiet, and don't run — running is what makes you prey."
   - Purpose: triggers when player is within T-Rex screen-shake radius (see tag below).
2. **Narrator_DuskWarning** — "Light's dying fast. Get the fire going before the cold sets in — and before anything with teeth decides the dark belongs to them tonight."
   - Purpose: triggers at dusk transition (day/night cycle sync).

## SFX Sourced (Freesound)
| Purpose | Asset | ID | Notes |
|---|---|---|---|
| Forest ambience (day) | Ambiance_Wind_Forest_Trees_Loop_02.wav | 530907 | 24s loop, calm |
| Forest ambience (day, alt) | Ambiance_Wind_Forest_Trees_Loop_01.wav | 530908 | 41s loop |
| Forest ambience (night/storm) | Ambiance_Wind_Trees_Strong_Loop_Stereo.wav | 544853 | 30s loop, strong wind — used for night/threat state |
| Dinosaur roar (fallback, until dedicated recordings exist) | Sea Creature Roar | 837799 | Deep layered growl/roar, closest realistic substitute found; heavy footstep search returned zero usable results this cycle |
| Campfire crackle | Campfire crackling - Loop | 620324 | Clean 30s loop, used for shelter/fire ambient bed |

Note: dedicated "heavy footstep ground thud" search returned **zero results** this cycle — flagged for next cycle retry with alternate query terms (e.g. "large creature stomp", "elephant footstep").

## Live Game Changes (via ue5_execute, zero .cpp/.h touched — per `hugo_no_cpp_h_v2`)
Audited existing world first (DirectionalLight, TRex/Raptor/Trike/Brachio actors, dialogue-tagged actors from Agent #15) — no duplicate actors spawned, per `hugo_naming_dedup_v2`.

Tags attached to **existing** actors (metadata layer for downstream systems — VFX/#17 and QA/#18 to wire into MetaSounds/Niagara):
- **T-Rex actor(s):** `AudioCue_TrexRoar_ScreenShakeRadius_1500`, `SFX_Freesound_837799_SeaCreatureRoar_Fallback`
- **All dinosaur actors (TRex/Raptor/Trike/Brachio):** `FootstepDust_HeavyThud_Enabled` — signals to VFX agent that footstep dust particles + thud SFX should trigger on their animation footfall events
- **DirectionalLight:** `AmbientAudio_DayNightSync`, `SFX_Freesound_530908_WindForestLoop_Day`, `SFX_Freesound_544853_WindForestStrong_Night` — ambient bed should crossfade as the light rotates through day/night cycle
- **Narrator actor (from Agent #15):** `VoiceLine_TrexProximity_Narrator`, `VoiceLine_DuskWarning_Narrator`

Level saved successfully after tagging pass.

## Game-Feel Directives Addressed (per Cycle Directive)
1. **Screen shake near T-Rex** — tag `AudioCue_TrexRoar_ScreenShakeRadius_1500` marks the T-Rex actor with a 1500-unit radius; actual `UGameplayCameraShakeBase` trigger logic must be wired by whichever system owns player-proximity checks (recommend Combat/AI agent #12 or a lightweight Blueprint on the TRex actor, since no C++ can be added this cycle per `hugo_no_cpp_h_v2`).
2. **Damage flash (red screen overlay)** — not implementable via Python/actor tags alone; requires a UMG widget + HUD hook. Flagged for QA/#18 or a UI-capable agent with Blueprint/widget authoring access.
3. **Footstep dust particles** — tagged all dinosaur + player-relevant actors with `FootstepDust_HeavyThud_Enabled`; actual Niagara particle system is VFX/#17's domain per chain-of-command (Audio → VFX).
4. **Day/night cycle audio sync** — tagged DirectionalLight with day/night ambient SFX references so the ambient bed crossfades as the sun rotates (assumes Lighting/#08 already owns the rotation timeline).

## Blockers
- Supabase storage 403 JWS error blocks audio URL persistence for the 3rd consecutive cycle (Agents #14, #15, #16 all affected) — needs infra fix, not agent-side.
- Freesound "heavy footstep" query returned 0 results — will retry with different terms next cycle.
- Screen shake and damage-flash effects need Blueprint/UMG or C++ hooks that are out of scope for a tag-only, no-C++ pass — downstream agents (#17 VFX, #18 QA) should pick these up using the tags placed here as triggers.

## For Agent #17 (VFX)
- `FootstepDust_HeavyThud_Enabled` tag is on all dinosaur actors — wire Niagara dust emitters to their animation footfall notifies.
- `AudioCue_TrexRoar_ScreenShakeRadius_1500` on T-Rex — pair with a camera shake Niagara/post-process pulse if VFX wants to co-own the proximity feedback.
- Damage flash overlay and true screen-shake implementation still need a Blueprint/widget-capable pass — recommend escalating to #18 QA or Director if no agent in the chain has UMG authority.
