# Audio Agent — Ciclo PROD_CYCLE_AUTO_20260721_007

## Contexto
Handoff do #15 (Narrative & Dialogue Agent): `NPC_Scout_Floresta_001` foi corrigido de grounding (z=1176 → z=135, junto ao `CraftingUITrigger_Hub_001`) e tagueado como quest-giver de crafting no hub, com 5 tags de diálogo/quest. Pedido explícito: gravar VO da linha de introdução e anexar SFX/dialogue trigger ao `CraftingUITrigger_Hub_001` existente (sem criar novo volume).

## Trabalho realizado neste ciclo

### 1. Voice-over gerado (ElevenLabs)
Linha de introdução do `NPC_Scout_Floresta_001` como quest-giver, tom pragmático de sobrevivência (sem misticismo):

> "Ainda estás vivo. Boa. Vem cá — preciso que vejas isto. Aquela pedra ali, se a lascares bem, corta como um dente de raptor. É o teu primeiro corte. Não desperdices a lâmina, e não te afastes muito do fogo depois do sol se ir embora."

- Upload para Supabase Storage **falhou** (`403 Invalid Compact JWS` no bucket) — o áudio foi gerado (base64 MP3, ~16s) mas não ficou persistido num URL público estável. **Acção necessária de outro agente/infra**: verificar credenciais/JWT do bucket de storage do ElevenLabs pipeline.
- Referência da linha registada como tag `VO_ScoutHubIntro_Line01` no actor `NPC_Scout_Floresta_001`.

### 2. Sound effects encontrados (Freesound)
Busca "stone knapping" não devolveu resultados directos no Freesound (biblioteca sem sample específico de lascar pedra) — registado como gap para produção futura (pode precisar de foley custom).

Campfire crackling — 2 candidatos aprovados para o hub:
- `Fireplace` (id 852107) — 8.5s, field recording Munique, boa para one-shot/stinger.
- `Campfire crackling - Loop` (id 620324) — 30s, loop nativo, ideal para ambiente contínuo junto à fogueira do hub.

### 3. Wiring no mundo vivo (sem duplicar actores, `hugo_naming_dedup_v3`)
Reutilizados os actores já existentes, sem criar novos:
- **`NPC_Scout_Floresta_001`** (2078.7, 2172.8, 135.0) — 4 tags novas: `VO_ScoutHubIntro_Line01`, `SFX_Ref_StoneKnapping_Freesound`, `SFX_Ref_CampfireCrackle_852107`, `AudioAttach_DialogueTrigger_Crafting`.
- **`CraftingUITrigger_Hub_001`** (2198.7, 2252.8, 135.0) — 3 tags novas: `SFX_Ref_CampfireLoop_620324`, `SFX_Ref_StoneCrafting_Impact`, `AudioZone_HubCraftingAmbience`.
- **`AudioManager`** (registro central, -4800,-1900,200) — 4 tags de registo cruzado para rastreabilidade: `Registry_VO_ScoutHubIntro_NPC_Scout_Floresta_001`, `Registry_SFX_StoneKnapping_CraftingUITrigger_Hub_001`, `Registry_SFX_CampfireCrackle_Freesound_852107`, `Registry_SFX_CampfireLoop_Freesound_620324`.

### 4. Auditoria da directiva #16 (Polish & Effects) — já implementada em ciclos anteriores
Confirmado via `get_all_level_actors()`:
- **46/46 T-Rex** já têm tags de screen shake (`...Shake...`) e footstep dust (`...Dust...`) — 100% de cobertura, nada a duplicar.
- **Damage flash**: já existem `Audio_DamageFlashAnchor_001`, `Marker_DamageFlashConfig` (`VFX_DamageFlash`, `UI_RedOverlay`, `Config_FlashDuration_0_3s`), e `VFX_Global_DamageFlashPostProcess_001` — sistema já configurado (0.3s vermelho).
- **Day/night audio**: já existem `DayNightAudioController` (ambiente dia/noite drivido por time-of-day, crossfade 8s, grilos/aves via Freesound), `Ambient_DayNightAmbientBed_Hub_001`, `Audio_DayNightCue_Hub_001`. **Nota**: a directiva pedia "rotating directional light" para day/night — **não foi tocada**, porque a regra HANDS OFF (PLAYABLE-FIRST v4) proíbe explicitamente modificar o sol. O sistema de áudio já está desenhado para ser `Audio_DrivenByTimeOfDay_NotByLightRotation`, coerente com essa restrição.

## Decisões técnicas
- Nenhum `.cpp`/`.h` criado (regra `hugo_no_cpp_h_v2`).
- Nenhum actor novo criado — todo o trabalho foi tagging/wiring sobre actores existentes (regra `hugo_naming_dedup_v3`).
- Sol, Landscape, foliage, câmara e PLAYER0 não foram tocados.
- Save único no fim do ciclo (`save_current_level()` → `True`).

## Dependências / próximos passos
- **#17 VFX Agent**: o post-process de damage flash (`VFX_Global_DamageFlashPostProcess_001`) já existe — validar se está realmente a disparar visualmente ao receber dano do Combat AI (#12).
- **Infra/Storage**: resolver o erro `403 Invalid Compact JWS` no bucket de upload de áudio ElevenLabs para persistir VO gerado.
- **Foley/SFX**: falta sample real de "stone knapping" — candidato a gravação custom ou biblioteca paga.
