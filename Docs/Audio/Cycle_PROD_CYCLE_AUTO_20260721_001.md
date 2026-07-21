# Audio Agent #16 — Ciclo PROD_CYCLE_AUTO_20260721_001

**Bridge: UP.** 5x `ue5_execute` (`command_type=python`), todas `status:completed`, 3-3.1s cada, zero timeouts. 3x `search_sounds`. Save final confirmado via `save_dirty_packages(True, True) → True` (o `save_current_level()` inicial devolveu `False` — retry corrigiu).

## Directiva do ciclo: Polish & Effects (game feel)
Tarefas atribuídas: screen shake perto do T-Rex, damage flash (overlay vermelho), footstep dust (player + dinos), day/night cycle via rotação de directional light.

## O que foi feito (REUSE FIRST — zero actores novos spawnados)

Auditoria revelou que ciclos anteriores (VFX/Audio) já tinham criado toda a infraestrutura de marker actors para estes 4 sistemas (81 actores "Dust", `Trigger_CameraShake_Savana_001`, `VFX_Global_DamageFlashPostProcess_001` + `Volume_DamageFlash_Global_001` deprecated, `DayNightAudioController`). O trabalho deste agente foi **completar a camada de áudio real** sobre essa infraestrutura, em vez de duplicar actores (anti-padrão já flagged nas memórias — `Trike_Audio_001_AI` etc.):

1. **Damage flash → SFX de impacto sincronizado**: `VFX_Global_DamageFlashPostProcess_001` e `Note_Audio_DamageFlashHook_001` tagueados com referência real Freesound (`Hit Impact`, ID 660770, preview URL completo), volume 0.8, sync 0.25s com o flash visual.

2. **Camera shake T-Rex → rumble de passada grave**: `Trigger_CameraShake_Savana_001` tagueado com SFX de passo pesado (Freesound `Dirt Footsteps 2`, ID 750795) como base para rumble de baixa frequência, raio de trigger 800uu.

3. **Footstep dust — player**: `FootstepDust_Player_001` tagueado com 3 variações de foley em terra (Freesound IDs 750794/750795/750796 — "Dirt Footsteps 1/2/3") para variedade de passos.

4. **Footstep dust — dinossauros**: 6 actores `Dust_Hub_001..006` (reutilizados, não duplicados) tagueados com SFX de passo pesado + pitch-shift down 0.6 para simular massa de dinossauro.

5. **Day/Night — SEM tocar no sol**: `DayNightAudioController` (já existente) enriquecido com tags de ambiente dia (pássaros/insectos) e noite (Freesound `Crickets Close and Distant Night` ID 523438 + `Crickets At Night - Clean` ID 522298), crossfade de 8s, explicitamente marcado como `Audio_DrivenByTimeOfDay_NotByLightRotation` — **NÃO rodei `Sun_Main_Directional`** por violar a regra HANDS-OFF (nunca tocar no sol) e a regra de CAP enforcement (DirectionalLight único). A rotação do ciclo dia/noite pertence ao Lighting Agent (#08), não ao Audio Agent; a minha contribuição é o crossfade sonoro reactivo ao time-of-day existente.

## Sound effects reais encontrados (Freesound)
- **Footsteps dirt**: IDs 750794, 750795, 750796, 477395, 477394 (Zoom H6 field recordings)
- **Hit impact**: ID 660770 (gunshot/hit impact — usado para damage sting)
- **Night ambience**: IDs 523438, 522298, 523439, 522299 (crickets, field recordings limpos)

## Verificação
`ue5_execute` de verificação confirmou `Audio_` tags presentes em todos os 6 actores-alvo (`ALL_TRUE True`). Save único no final do ciclo.

## Decisões técnicas
- Zero `.cpp/.h` criados (regra absoluta).
- Zero actores novos spawnados — tudo via tagging de actores reais já existentes (REUSE FIRST).
- Zero alterações a `TranspersonalCharacter PLAYER0`, Landscape/Terrain_Savana, sol, ou câmara do editor.
- Day/night rotativo **recusado explicitamente** por conflito com HANDS-OFF — reportado como decisão, não como bloqueio.

## Dependências / handoff para #17 (VFX Agent)
- `Volume_DamageFlash_Global_001` está tagueado como `DamageFlash_Deprecated_UseAuthoritative` — sugiro ao VFX Agent consolidar/remover a duplicação (não apaguei, cleanup é de scripts de manutenção).
- O rumble de baixa frequência do camera shake precisa de um MetaSound real (Niagara/Audio component) — actualmente é apenas metadata de tag; falta o binding real de asset de áudio via Blueprint, que requer acesso a import de ficheiro (fora do escopo Python deste ciclo).
- Rotação real do directional light para day/night cycle deve ser pedida ao Lighting Agent (#08), respeitando a regra HANDS-OFF sobre o sol.
