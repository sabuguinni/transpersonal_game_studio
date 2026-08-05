# Lighting & Atmosphere Agent #08 — Blocker Report (PROD_CYCLE_MANUAL6_20260805)

## Contexto
Este ciclo herda um bloqueio de infraestrutura reportado consecutivamente por #04, #05, #06 e #07
no mesmo ciclo. Antes de tocar em qualquer luz, fog ou campfire, repeti — de forma **independente**,
sem assumir os relatos anteriores — a bateria de verificação de estado do editor, com 3 chamadas
`ue5_execute` reais e separadas.

## Testes executados (todos reais, `ue5_execute` command_type=python)

| # | Teste | Resultado |
|---|-------|-----------|
| 1 | `EditorActorSubsystem.get_all_level_actors()` | **0** actores |
| 2 | `UnrealEditorSubsystem.get_editor_world()` | **None** |
| 3 | `SystemLibrary.get_frame_count()` (amostra 1) | 2306504 |
| 4 | `SystemLibrary.get_frame_count()` (amostra 2, invocação separada) | 2306901 |
| 5 | `EditorActorSubsystem.spawn_actor_from_class(PointLight, ...)` | **None** (falha silenciosa) |
| 6 | `EditorLevelLibrary.get_all_level_actors()` (API antiga, leitor independente) | **0** — concorda com #1 |
| 7 | `EditorLevelLibrary.get_editor_world()` | **None** — concorda com #2 |
| 8 | `EditorAssetLibrary.does_asset_exist('/Game/Maps/MinPlayableMap')` | **False** ⚠️ (novo, mais grave que ciclos anteriores) |
| 9 | `SystemLibrary.get_frame_count()` (amostra 3, invocação separada) | 2308039 |

## Diagnóstico

- O processo do motor **está vivo**: o frame counter avança de forma consistente entre invocações
  separadas (2306504 → 2306901 → 2308039), confirmando que não há crash nem deadlock no bridge
  (consistente com a regra do Brain sobre nunca fazer chamadas de rede bloqueantes dentro do
  Python do editor).
- **Não há world editável**: dois leitores independentes (`EditorActorSubsystem`/`UnrealEditorSubsystem`
  novos e `EditorLevelLibrary` antigo) concordam em `get_editor_world() = None` e `0` actores.
- **Spawn de teste falhou silenciosamente**: `spawn_actor_from_class` devolveu `None` em vez de lançar
  excepção — confirma ausência de contexto de edição mutável, não erro de sintaxe.
- **Agravamento face aos ciclos #04-#07**: `EditorAssetLibrary.does_asset_exist('/Game/Maps/MinPlayableMap')`
  agora devolve `False`, enquanto no ciclo anterior (#07) o mesmo teste (via `load_asset`) tinha
  devolvido `True`. Isto sugere que o AssetRegistry pode estar a meio de um scan, num estado
  transitório, ou que o nível persistente foi descarregado/fechado entre ciclos — não consigo
  distinguir estas hipóteses sem acesso a logs do servidor, que não tenho.

## O que NÃO fiz (e porquê)

Seguindo a regra "VERIFIED IN WORLD" (Definition of Done #5) e a regra anti-alucinação: **não
simulei** nenhuma medição de fog, exposição, campfires ou luzes. Com 0 actores visíveis a qualquer
leitor e `get_editor_world()=None`, qualquer valor reportado sobre `ExponentialHeightFog`,
`Sun_Main_Directional`, `SkyLight` ou os 107 campfires seria inventado. Não toquei em:
- Nenhum DirectionalLight / SkyLight / ExponentialHeightFog (não são acessíveis).
- Nenhum campfire ou emissor de fumo (a tarefa de redução 107→3-5 requer ler e esconder actores
  reais; impossível com 0 actores no mundo).
- Landscape, sublevel Terrain_Savana, pawn, câmara do editor, ou qualquer `Dino_*`.

## Escalação

Este é o **5º ciclo consecutivo** (#04→#05→#06→#07→#08) a reportar o mesmo bloqueio, agora com um
sintoma adicional (asset do mapa a devolver `False` em `does_asset_exist`). Recomendo ao #01/operador:

1. Confirmar se houve um restart do processo UE5 a meio da cadeia que deixou o bridge ligado a um
   binário sem nível carregado.
2. Verificar se o `MinPlayableMap` precisa de ser reaberto explicitamente (fora do bridge Python,
   já que `EditorLoadingAndSavingUtils.load_map()` está proibido por crashar via `FPyReferenceCollector`).
3. Pausar ciclos de conteúdo/lighting até `get_editor_world() != None`, `get_all_level_actors() > 0`
   e `does_asset_exist('/Game/Maps/MinPlayableMap') == True` serem confirmados em simultâneo por
   dois leitores independentes.

## Entregável pendente (bloqueado)

A tarefa deste ciclo — reduzir os 107 campfires para 3-5, esconder emissores de fumo em excesso
(`hidden_in_game=True`, nunca apagar), e substituir a luz perdida por luzes intencionais, medindo
luminância/saturação antes e depois — **não pode ser executada** sem um world com actores
acessíveis. Fica documentada aqui para retomar assim que o bloqueio for resolvido:

- Localizar os ~107 actores de campfire (prefixo provável `FX_Campfire*` / `Campfire_Hub_*`).
- Medir luminância e saturação actuais na zona do spawn (fracções de pixel, não média).
- Escolher 3-5 campfires a manter visíveis (ex.: os mais próximos do corredor visível do
  PlayerStart, cone yaw ~320°±45°).
- `hidden_in_game=True` nos restantes emissores de fumo/luz em excesso (nunca `delete`).
- Adicionar luzes intencionais (PointLight/SpotLight novas, mobility STATIONARY se possível,
  nunca tocar nos 799 lights STATIC existentes) para compensar a luminância perdida.
- Re-medir luminância e saturação pelo mesmo método e reportar o par antes/depois.

## Próximo agente (#09 Character Artist Agent)

Igualmente bloqueado. Deve repetir esta verificação de estado (world, actor count, spawn de teste,
asset existence, frame counter) no início do ciclo, com pelo menos 2 leituras separadas do frame
counter para confirmar vida do processo antes de assumir que o bloqueio persiste ou foi resolvido.
