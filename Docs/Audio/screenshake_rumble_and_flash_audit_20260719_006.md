# Audio Agent #16 — Ciclo PROD_CYCLE_AUTO_20260719_006

## Diretiva do ciclo
Foco: game feel/feedback — screen shake (proximidade T-Rex), damage flash, footstep dust, day/night.
Nota: screen shake, damage flash, footstep dust e day/night são sistemas majoritariamente de
VFX/Camera/Lighting; a contribuição do Audio Agent foca-se na componente sonora que os alimenta/sincroniza
(cues de áudio, tags de trigger, design notes), evitando duplicar trabalho de outros agentes.

## Estado verificado no mundo (bridge UP, 5x ue5_execute OK, zero timeouts)

### 1. Screen shake por proximidade de T-Rex
- Auditoria: 245 atores `TRex_Savana_*` no nível; 3 já tinham `_ShakeSource` de ciclos anteriores
  (`TRex_Savana_001` + 2 VFX de dust kick).
- **46 T-Rex dentro do núcleo jogável** (x -3000..5000, y -1000..5500) identificados sem tag de shake.
- Todos os 46 receberam tags: `FX_ScreenShake_Radius_2000` + `FX_ScreenShake_Intensity_Heavy`
  (consumidas por Blueprint/gameplay de câmara — não criei .cpp/.h, conforme regra absoluta).
- Nenhum T-Rex existente foi duplicado ou movido; apenas tagging aditivo.

### 2. Cues de áudio sincronizadas com o shake (contribuição real do Audio Agent)
- Criados **15 marcadores `Note`** (`TRex_Savana_XXX_RumbleCue`) posicionados na localização exata
  de 15 dos 46 T-Rex tagged (batch limitado para manter chamadas atómicas e seguras).
- Tags aplicadas: `AudioCue_HeavyFootstep`, `MetaSound_TRexRumble`, `SyncWith_ScreenShake`.
- Texto de design gravado em cada Note: cue MetaSound de sub-bass (30–80Hz) no impacto do passo,
  raio 2000uu, atenuação por distância e estado de stealth/crouch do jogador.
- Os 31 T-Rex restantes (tagged mas sem Note ainda) ficam para o próximo ciclo — evita sobrecarregar
  o nível com 46 novos actors numa única chamada.

### 3. Damage flash — já existente (auditado, não duplicado)
Confirmunderlying sistema já presente de ciclos anteriores (provavelmente VFX/Lighting agent):
- `VFX_Global_DamageFlashPostProcess_001` (PostProcessVolume)
- `Volume_DamageFlash_Global_001` (PostProcessVolume)
- `Audio_DamageFlashAnchor_001` / `Note_Audio_DamageFlashHook_001` (Notes de hook áudio)
- `Marker_DamageFlashConfig` (TargetPoint)
Nenhuma ação necessária — sistema coberto. Não recriado.

### 4. Footstep dust — já existente (auditado, não duplicado)
281 atores relacionados (`FootstepDust_Player`, `FootstepVFX_Zone`, `FootstepTrigger`,
`FootstepEcho_Zone`, `FootstepEmitter_PanicZone`, `Dust_Hub`, etc.) já cobrem jogador e zonas.
Não recriado para evitar duplicação (regra `hugo_naming_dedup_v2`).

### 5. Day/night cycle
- **Sun_Hub_Main NÃO tocado** (regra absoluta — hands off sun).
- Confirmada existência prévia de `DayNightAudioController`, `Ambient_DayNightAmbientBed_Hub_001`,
  `Audio_DayNightCue_Hub_001` — a componente sonora do ciclo dia/noite já está coberta.
- Rotação de luz direcional é responsabilidade do Lighting Agent (#08); não é alterada aqui.

## Sound design (Freesound) — pesquisa realizada
- Query "heavy dinosaur footstep thud ground impact": 0 resultados diretos.
- Query "giant footstep boom impact": 1 candidato viável —
  `TF_cyberverse_heavy_robot_footstep_recreation_02232025` (id 789755, 7.6s, preview em
  cdn.freesound.org) — som de impacto pesado reaproveitável como base para footstep de T-Rex
  após pitch-down e layering com sub-bass.
- Query "deep low growl rumble monster": resultados não relevantes (sons de ASMR/estômago) —
  descartados, não usados no jogo.

## Voice-over
- Linha gerada via ElevenLabs: "Warning. Large predator detected nearby. Movement heavy.
  Stay low and do not run." (tom factual, não-místico, alinhado com a regra anti-alucinação).
- Upload para Supabase falhou (`storage_failed: Invalid Compact JWS`, HTTP 403) — áudio devolvido
  como base64 inline; requer reconfiguração de credencial de storage antes do próximo VO.

## Ficheiros
- `.cpp/.h`: ZERO criados (regra absoluta respeitada).
- Documentação: este ficheiro.

## Verificação
- `save_dirty_packages_result: True` — save único no fim do ciclo, confirmado.
- Contagem final de atores no nível: 3903 (3888 no início + 15 Notes de rumble cue).
- Nenhum toque em: PLAYER0, Landscape/Terrain_Savana, câmara do editor, Sun_Hub_Main (posição/rotação).

## Próximo agente (#17 VFX Agent)
- Consumir tags `FX_ScreenShake_Radius_2000` / `FX_ScreenShake_Intensity_Heavy` nos 46 T-Rex do
  núcleo para implementar a lógica real de Camera Shake (Blueprint/Niagara), sincronizada com os
  15 `RumbleCue` Notes já colocados.
- Completar cobertura de rumble cues nos 31 T-Rex tagged restantes.
- Corrigir credencial de storage do ElevenLabs (JWS inválido) antes de próxima geração de VO.
