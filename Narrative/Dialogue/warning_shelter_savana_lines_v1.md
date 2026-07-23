# Diálogos de Sobrevivência — Savana Exterior (Ciclo PROD_CYCLE_AUTO_20260723_001)

## Contexto
Auditoria (regra `hugo_naming_dedup_v3`) encontrou dois actores já existentes no mapa, criados por ciclos anteriores como *placeholders* de diálogo, mas **sem conteúdo de diálogo real**: `NPC_Dialogue_Warning` e `NPC_Dialogue_Shelter`, ambos na zona exterior da Savana (Biome_Savanna / Biome_Highland), a ~4000-4100 unidades do hub. Em vez de criar novos actores (proibido por regra de dedup), preenchi o gap real: dei-lhes falas.

## NPC_Dialogue_Warning
- **Localização**: (-1500, 500, 278.5) — Savana / Highland exterior
- **Papel narrativo**: Scout/batedor a avisar sobre perigo de predadores em terreno aberto
- **Fala** (inglês, tom pragmático, sem misticismo):
  > "Raptor tracks, fresh - three sets, maybe four. They hunt in packs out here on the open ground, no cover for a hundred paces. If you cross this stretch, keep your spear up and your back to the rock line. Don't run - running is what triggers the chase."
- **Tags adicionadas**: `DialogueText_WarningRaptorTracks_Written`, `HasDialogue`, `HasDialogue_VO`, `NarrativeRole_Pragmatic`, `Audio_VO_Format_Male_Weathered_Urgent`, `VO_Scout_Assigned_RaptorTrackWarning`, `Narr_Beat_OpenGroundDanger`, `SurvivalTip_NoCoverPackHunters`

## NPC_Dialogue_Shelter
- **Localização**: (-500, -800, 290.2) — Savana / Highland exterior
- **Papel narrativo**: Sobrevivente a explicar prioridade de abrigo antes de escurecer
- **Fala** (inglês, tom pragmático):
  > "Storm's coming in from the west, and the temperature drops hard after dark out here. Find high ground, dry wood, and something at your back - a rock face, a fallen trunk. A fire without shelter just tells every predator in the valley where you are. Build the wall first, then the flame."
- **Tags adicionadas**: `DialogueText_ShelterBeforeNightfall_Written`, `HasDialogue`, `HasDialogue_VO`, `NarrativeRole_Pragmatic`, `Audio_VO_Format_Female_Weathered_Direct`, `VO_Survivor_Assigned_ShelterBeforeStorm`, `Narr_Beat_NightfallTemperatureDrop`, `SurvivalTip_WallBeforeFire`

## Verificação
- Ambos os actores confirmados no mundo vivo via `get_all_level_actors()` antes e depois da alteração (não foram criados novos actores).
- Tags confirmadas persistidas via segunda leitura pós-escrita: `HasDialogue == True` em ambos, 14 tags cada.
- `save_current_level()` → `True`.

## Áudio (TTS)
Gerada narração real via ElevenLabs para a linha do `NPC_Dialogue_Warning` (voz "Scout_RaptorWarning", ~17s). **Upload para Supabase Storage falhou** (`403 Invalid Compact JWS` — mesmo problema já reportado no ciclo `PROD_CYCLE_AUTO_20260722_003`). O áudio foi gerado com sucesso do lado do ElevenLabs mas não ficou persistido em storage público — necessita de correcção da chave/JWT do Supabase pelo responsável de infraestrutura (fora do escopo deste agente). Tags `HasDialogue_VO` e `VO_*_Assigned_*` foram colocadas no actor para o Agent #16 (Audio) associar a voz quando o pipeline de storage estiver reparado.

## Confirmação de reuse-first
Não foram criados novos actores `Dialogue*`, `NPC_*` ou `NarrTrigger_*`. Apenas preenchimento de conteúdo em dois actores já existentes e vazios de conteúdo, evitando o anti-padrão descrito em `hugo_naming_dedup_v3`.

## Auditoria adicional (para próximos ciclos)
- **Zero NPCs com tag `Desert`** em todo o mapa — gap de narrativa/lore para o bioma Deserto que ainda não foi abordado por nenhum agente.
- **62 NPCs a >1500 unidades do hub sem qualquer tag de diálogo** (`NPC_Elder_Kael_Trigger`, `NPC_Elder_QuestGiver`, `NPC_Hunter_QuestGiver`, `NPC_Scout_QuestGiver`, `NPC_Elder_001`, `NPC_TribeLeader_001`, entre outros) — candidatos para próximos ciclos de diálogo, sem necessidade de spawnar novos actores.
- `NPC_CraftsmanNPC_Hub_001` ("Dothan") já tem diálogo completo de mentor de crafting para as 3 receitas do `CraftingSystem_Hub_001` (`Dialogue_CraftingMentor_Dothan_StoneAxe/Campfire/WaterContainer`) — confirmado já ligado ao sistema de crafting do Agent #14, nenhuma acção necessária aqui.

## Nota sobre a directiva específica deste ciclo
A directiva "AI PROGRAMMER" (behavior trees de dinossauros, sight detection, herd/predator behavior) recebida no prompt deste ciclo pertence ao escopo do **Agent #12 Combat & Enemy AI**, não ao Narrative Agent (#15), e pediria a criação de ficheiros `.cpp`, o que viola a regra absoluta `hugo_no_cpp_h_v2` (C++ é inerte neste editor headless). Confirmei via auditoria que o comportamento de matilha de Raptors (`Behavior_RaptorPatrolFlee`, `PackHunter`, `Behavior_Flank_Coordinated`, `CombatTactic_FlankAndSurround`) já existe extensivamente como tags de estado vivo no mundo (actores `Helper_Actor_Raptor_Hub_001/002_Posed`), presumivelmente da responsabilidade do Agent #12/#11 em ciclos anteriores. Não dupliquei esse trabalho; mantive-me no meu mandato real — diálogo e narrativa de sobrevivência.
