force write
# Architecture & Interior Agent #07 — Relatório de Bloqueio (PROD_CYCLE_MANUAL6_20260805)

## Contexto
Este ciclo era dedicado a: (1) corrigir o assentamento dos 26 `Ruin*` + 1 `StoneRuin*` que ficaram enterrados no ciclo de 31/07 (ex.: `RuinOutpost_Biome_001` -96uu, `StoneRuin_Biome50k_001` -80uu, `Ruin_BiomeOutpost_001` -160uu); (2) compor o acampamento com pelo menos 3 props reais de `/Game/Tropical_Jungle_Pack` (shelter, storage, work surface), reutilizando os 71 `Camp*`/16 `Hearth*`/12 `Fire*` já existentes por label lookup.

**Nenhum destes objectivos foi executado neste ciclo.** O motivo é um bloqueio de infraestrutura confirmado de forma independente, que já tinha sido reportado nos 3 ciclos anteriores por #04 (Performance), #05 (World Generator) e #06 (Environment Artist).

## Verificação independente feita neste ciclo (4ª confirmação consecutiva na cadeia)

Repeti a bateria de testes do #06, sem assumir o relato dele, com 3 chamadas `ue5_execute` separadas:

| Teste | Resultado |
|---|---|
| `UnrealEditorSubsystem.get_editor_world()` | `None` |
| `EditorActorSubsystem.get_all_level_actors()` | `0` actores |
| `LevelEditorSubsystem.get_current_level()` | `None` |
| `EditorActorSubsystem.spawn_actor_from_class(PointLight, ...)` | `None` (spawn falha silenciosamente, sem excepção) |
| Releitura da contagem de actores após tentativa de spawn | `0` (confirma que nada foi criado) |
| `unreal.load_asset('/Game/Maps/MinPlayableMap.MinPlayableMap')` | `True` — o asset existe e carrega em memória |
| `AssetRegistry.get_assets_by_package_name('/Game/Maps/MinPlayableMap')` | `['MinPlayableMap']` — o package está registado |
| `EditorLevelLibrary.get_all_level_actors()` (API antiga, leitor independente) | `0` — concordante com o leitor novo |
| `EditorLevelLibrary.get_editor_world()` (leitor independente) | `None` — concordante |
| `SystemLibrary.get_frame_count()` em duas chamadas separadas | `2296944` → `2298836` — **o processo do engine está vivo e a tickar** (não é um crash, é ausência de world de edição) |

## Diagnóstico
- O binário UE5 está a correr (frame counter avança ~1900 frames entre chamadas).
- O asset `MinPlayableMap` existe intacto em disco e é carregável como objecto.
- **Não existe nenhum `UWorld` activo como contexto de edição.** Dois leitores independentes (API nova `EditorActorSubsystem`/`UnrealEditorSubsystem` e API antiga `EditorLevelLibrary`) concordam: `None` / `0` actores.
- A prova mais grave (já levantada por #06 e reconfirmada aqui): a API de **mutação** (`spawn_actor_from_class`) não lança excepção — devolve `None` silenciosamente. Isto significa que qualquer agente que não verifique por releitura independente poderia reportar falsamente "actor criado" sem nada ter sido escrito no mundo.

## O que NÃO fiz, por não haver world mutável
- Não corrigi o assentamento dos 26 `Ruin*`/`StoneRuin*` — não há actores para ler nem mover (`get_all_level_actors()` = 0, os 26+1 ruins não aparecem em nenhum leitor).
- Não spawnei nem reposicionei props do `/Game/Tropical_Jungle_Pack` no acampamento — a mesma razão.
- Não toquei no Landscape, sublevel `Terrain_Savana`, pawn do jogador, câmara do viewport nem qualquer actor `Dino_*` — nem seria possível, dado que não há mutação possível neste estado.
- Não usei `EditorLoadingAndSavingUtils.load_map()` — proibido por memória do Brain (crasha o editor via `FPyReferenceCollector`).

## Plano de trabalho pendente para quando o editor recuperar
1. **Ruins (prioridade 1)**: para cada um dos 26 `Ruin*` + 1 `StoneRuin*`, ler a base dos bounds (9 pontos do AABB em XY, não um ponto único), fazer line trace ao Landscape ignorando todos os actores não-Landscape, e reportar delta antes/depois por actor. Referência do estado conhecido antes do bloqueio (ciclo 02/08): todos os 9 meshes estruturais confirmados a delta 0.0uu — mas o relato de 31/07 (herdado, não verificado por mim) indica que pelo menos 3 destes (`RuinOutpost_Biome_001`, `StoneRuin_Biome50k_001`, `Ruin_BiomeOutpost_001`) voltaram a ficar enterrados. Isto precisa de re-medição do zero quando houver world, sem assumir nenhum dos dois relatos anteriores.
2. **Acampamento (prioridade 2)**: fazer label lookup dos `Camp*`/`Hearth*`/`Fire*` existentes (71/16/12) antes de qualquer spawn — não duplicar. Adicionar no mínimo 3 props reais de `/Game/Tropical_Jungle_Pack` (abrigo, armazenamento, superfície de trabalho), seatados pela FOOTPRINT completa (não um único trace), medindo R18/R36 antes e depois.
3. Ambos os passos exigem `spawn_actor_from_class` a devolver um actor real e `get_all_level_actors()` a devolver >0, verificados por releitura independente — nenhuma das duas condições se verifica neste momento.

## Decisão técnica
Segui a hierarquia de instruções (regra "VERIFIED IN WORLD" da Playable-First Directive, ponto 5) em vez de fabricar qualquer relato de mutação. Reportar ruínas corrigidas ou props colocados este ciclo seria falso, dado que 0 actores são visíveis a qualquer leitor da API.

## Escalação
4º ciclo consecutivo (contando #04, #05, #06 e este #07) a confirmar o mesmo bloqueio com evidência crescente:
- #04/#05: leitura do world devolve `None`.
- #06: mutação (`spawn_actor_from_class`) falha silenciosamente.
- #07 (este ciclo): confirmado com 2 leitores independentes em simultâneo (API nova + API antiga), e confirmado que o processo do engine está vivo (frame counter avança) — ou seja, **não é um crash do binário, é ausência específica de contexto de edição/world**.

**Recomendação ao #01/operador**: restart real do processo UE5 fora do bridge Python continua a ser a única via conhecida para resolver isto. Não avançar mais ciclos de conteúdo (Architecture, Lighting, Character Artist, etc.) até `get_editor_world() != None` **e** `spawn_actor_from_class` devolver um actor real, ambos verificados por releitura independente em invocações separadas.

## Próximo agente (#08 Lighting & Atmosphere Agent)
Igualmente bloqueado. Deve repetir a mesma verificação de estado (world, actor count, spawn de teste, frame counter) no início do ciclo antes de qualquer trabalho de iluminação/atmosfera, e não assumir que o restart já ocorreu apenas porque este relatório existe.
