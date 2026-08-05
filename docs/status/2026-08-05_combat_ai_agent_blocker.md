# Combat & Enemy AI Agent #12 — Blocker Report
**Cycle:** PROD_CYCLE_MANUAL6_20260805

## Verificação real efectuada (4 chamadas `ue5_execute`, invocações separadas)

**Chamada 1 (diagnóstico inicial):**
- `EditorActorSubsystem.get_all_level_actors()` = **0** actores (0 `Dino_*` encontrados)
- `UnrealEditorSubsystem.get_editor_world()` = **None**
- `LevelEditorSubsystem.get_current_level()` = **None**
- `EditorAssetLibrary.does_asset_exist('/Game/Maps/MinPlayableMap')` = **False**
- frame_count amostra 1 = 2338722

**Chamada 2 (~1.3s depois, invocação separada):**
- `get_all_level_actors()` = **0** (confirmado, não é transitório)
- `get_editor_world()` = **None** (confirmado)
- frame_count amostra 2 = 2340057 (+1335 frames — processo UE5 vivo e a correr, não crashado)
- Tentativa de `spawn_actor_from_class(PointLight, (0,0,0))` = **None** (mutação impossível sem world)

**Chamada 3 (confirmação via API alternativa):**
- `EditorLevelLibrary.get_all_level_actors()` = **[]** (via classe legacy, mesmo resultado)
- `AssetRegistryHelpers.get_asset_registry().get_asset_by_object_path('/Game/Maps/MinPlayableMap.MinPlayableMap')` = **válido** (asset existe no registry, package_name confirmado, asset_class World)
- frame_count amostra 3 = 2342271 (processo continua vivo)
- **Discordância confirmada:** o AssetRegistry conhece o mapa; nenhum subsystem de editor tem o world carregado.

**Chamada 4 (diagnóstico de vias seguras de recuperação, SEM executar nenhuma):**
- Listados os métodos de `LevelEditorSubsystem`: existe `load_level` (distinto de `load_map`)
- Listados os métodos de `UnrealEditorSubsystem`: `get_editor_world`, `get_game_world` confirmados como as vias de leitura correctas
- frame_count amostra 4 = 2344194

## Decisão: NÃO tentei `load_level`

A memória do Brain (`hugo_terrain_savana_v5`) proíbe explicitamente `EditorLoadingAndSavingUtils.load_map()` do Python por crashar o editor (Fatal error EditorServer.cpp:1934 — FPyReferenceCollector retém referências e impede o GC do package antigo). `LevelEditorSubsystem.load_level` não está testado nesta sessão e pode ter o mesmo perfil de risco (crash da sessão partilhada do editor, afectando os 18 outros agentes da cadeia). Isto conta como **DECISÃO IRREVERSÍVEL** fora do meu mandato específico (Combat AI trabalha sobre tags em actores `Dino_*` existentes — não é agente de recuperação de infraestrutura). Não executei.

## Conclusão

Bloqueio de infraestrutura confirmado pela **9ª vez consecutiva na cadeia** (#04→#05→...→#10→#11→#12). O processo UE5 está vivo e a avançar frames normalmente, mas **nenhum world está carregado em nenhum subsystem de editor Python-acessível** desde antes do ciclo do #10. Isto bloqueia toda a directiva específica deste agente: tagging de combate nos 36 `Dino_*` e 4 `TRexPatrolMarker_*` requer `get_all_level_actors() > 0`, e neste momento devolve 0.

**Não fabriquei nenhuma tag, zona ou coordenada.** Fazê-lo sem world real significaria escrever python_code que "parece" ter corrido mas não tocou em nenhum actor real — exactamente o tipo de alucinação que a regra anti-alucinação e o DEFINITION OF DONE (item 5, VERIFIED IN WORLD) proíbem.

## O que fica pronto para quando o world recuperar

Recipe de tagging de combate já validada em ciclos anteriores (ver memória `hugo_combat_label_consistency_v1`):
1. Ler os 36 `Dino_*` + 4 `TRexPatrolMarker_*` por label exacto.
2. Antes de criar qualquer `CombatZone_*`/`BehaviorTag_*`, fazer query de labels existentes num raio de ~3500uu do hub para evitar duplicação (já há ~191 `CombatZone_Raptor` + ~185 `BehaviorTag_Raptor` acumulados de ciclos anteriores — **não criar mais destes**, apenas reutilizar/actualizar).
3. Verificar R30 (predador-presa ≥5000uu) e R31 (≤1 criatura a <3000uu do PlayerStart) antes e depois — são BLOCKER.
4. Nunca mover, rodar, reescalar ou apagar nenhum `Dino_*`.

## Ficheiro criado
- `docs/status/2026-08-05_combat_ai_agent_blocker.md` (este ficheiro)

## Dependências / próximo agente (#13 Crowd & Traffic Simulation)
- Repetir a verificação de 4 chamadas (actor_count via `get_all_level_actors()`, `get_editor_world()`, spawn_test, frame_count em invocações separadas) antes de assumir qualquer actor acessível para simulação de multidões.
- Recomenda-se que **#01/#02** (Studio Director / Engine Architect) investiguem a recuperação do world fora do fluxo normal de agentes — isto excede o mandato de qualquer agente individual da cadeia de produção, dado o risco de crash documentado em `load_map`/`load_level`.
- Quando o world recuperar: a lista de ~191 `CombatZone_Raptor`/~185 `BehaviorTag_Raptor` duplicados deveria ser objecto de limpeza (fora do mandato deste agente per as regras — "cleanup belongs to maintenance scripts, not to agents" — mas reportado para conhecimento do #18 QA).
