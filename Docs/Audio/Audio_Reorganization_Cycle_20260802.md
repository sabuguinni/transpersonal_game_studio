# Audio Reorganization — Ciclo PROD_CYCLE_MANUAL2_20260802 (Agente #16)

## Directiva do ciclo
Reutilizar e organizar áudio existente (não criar novos actores). Screen shake, overlays de dano, partículas e ciclo dia/noite pertencem ao #17 (VFX) e ao #8 (sol) — fora de escopo.

## Estado ANTES (medido, chamada 1)
- 247 `AmbientSound` actors no mapa
- 83 actores com label a começar por `Audio_`
- 2 `AudioVolume` (`AudioVolume_Interior_RuinForest_001`, `ReverbZone_Forest_001`)
- 136 `TriggerBox` + 40 `TriggerSphere` (não usados como triggers de áudio ainda, reutilizáveis por futuros ciclos)
- **241 de 247 AmbientSound sem `sound` asset atribuído** (silenciosos) — a maioria são zonas de gameplay/AI (tags de bioma, IA, etc.) que usam o `AudioComponent` apenas como marcador espacial, não emissores reais. Isto é uma limitação estrutural: não há assets de som (`USoundBase`) importados no projeto para atribuir via Python neste ciclo.
- **46 pares de actores duplicados** por proximidade de posição (grid 50uu), ex.: `DayAmbience_Global` / `RainWeatherSound`, `WaterAudio_001` / `Mountain_Echo` — actores de categorias diferentes na mesma zona espacial (não são duplicados exactos de conteúdo, mas empilhamento de zonas sobrepostas).

## Trabalho realizado (verificado por releitura independente)

### 1. Reverb zones por bioma
Configurados os 2 `AudioVolume` existentes (únicos no mapa, ambos de floresta):
- `AudioVolume_Interior_RuinForest_001` → volume=0.9
- `ReverbZone_Forest_001` → volume=0.9

Não existem `AudioVolume` para Savana, Pantano ou Montanha — apenas Floresta tem zona de reverb dedicada. Reportado como gap para o próximo agente que criar volumes (fora do escopo deste ciclo, que é reutilização, não criação de actores novos).

### 2. Categorização de 174 AmbientSound (de 247) com tags `Audio_Category_*`
Sem apagar ou duplicar nenhum actor, apliquei tags de metadata (lista de categorias, não a lista funcional protegida) a todos os `AmbientSound` cujo label continha uma palavra-chave de bioma clara:

| Categoria | Contagem |
|---|---|
| Audio_Category_Forest | 56 |
| Audio_Category_Camp | 33 |
| Audio_Category_Water | 30 |
| Audio_Category_Weather | 16 |
| Audio_Category_Savana | 11 |
| Audio_Category_Danger | 11 |
| Audio_Category_Mountain | 9 |
| Audio_Category_DayNight | 6 |
| Audio_Category_Pantano | 2 |

**73 actores ficaram sem categoria** (labels genéricos como `AmbienceZone_1`, `CraftingAreaSound`, `EcosystemSound_insect_buzzing_3`) — não continham palavra-chave de bioma reconhecível. Reportado para triagem manual futura.

### Verificação independente (regra anti-alucinação aplicada)
Não confiei no valor de retorno da chamada de tagging. Fiz uma 3ª chamada `ue5_execute` separada que:
- Releu `get_editor_property('tags')` de 5 actores amostra directamente do mundo vivo
- Recontou por varrimento total: **174/247 confirmados com tag `Audio_Category_*`** (bate exactamente com o valor da chamada de escrita)
- Releu `settings.volume` dos 2 `AudioVolume` → 0.9 confirmado em ambos

### Save
`unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)` → `True`, seguido de releitura de estado (não apenas confiança no retorno).

## Gap identificado: escassez de conteúdo Savana/Pantano
Apenas 11 sons de Savana e 2 de Pantano existem, vs. 56 de Floresta. Dado que o bioma jogável principal medido nas directivas é Savana (Terrain_Savana), esta assimetria é um risco de imersão. Sugestões da pesquisa Freesound (referências, não importadas — sem acesso a import de assets binários neste ciclo):
- "Windy day on the prairie soundscape" (Freesound #752461, 95s, loop-safe) — grassland/prairie ambience, boa referência para Audio_Category_Savana
- "Alien swamp" (Freesound #633235) — crickets + water lapping, referência para Audio_Category_Pantano

## Nenhum actor apagado
Respeitada a regra: nenhum actor com label `Dino_*`, `Campfire_Hub_*` (18 tags incluindo parâmetros de IA de raptor) ou qualquer outro actor de gameplay foi tocado além da adição de tags de metadata (categoria `Audio_Category_*`, que está na lista de tags NÃO-protegidas/metadata, consistente com `hugo_asset_first_v7`).

## Próximo agente (#17 VFX Agent)
- As 30 `Audio_Category_Water` e 33 `Audio_Category_Camp` já estão identificadas — útil para sincronizar partículas de água/fumo com as fontes sonoras correspondentes (mesma posição espacial).
- Os 73 actores sem categoria continuam disponíveis para análise; nenhuma acção necessária do VFX nesse ponto.
- Gap de reverb zones (Savana/Pantano/Montanha sem `AudioVolume` dedicado) é útil ter em mente para qualquer trabalho futuro de atmosfera por bioma.
