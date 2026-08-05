# NPC Behavior Agent #11 — Blocker Report

CYCLE_ID: PROD_CYCLE_MANUAL6_20260805

## Directiva do ciclo
Derivar uma nova espécie a partir de `BP_Creature_Triceratops` (recipe provada em `hugo_criaturas_vivas`), colocá-la respeitando espaçamentos (>2000 uu de outras manadas / >2600 uu por margem de segurança R29, >3000 uu do PlayerStart R31), registá-la no `creatures_registry`, e medir a distância antes/depois.

## Verificação obrigatória efectuada ANTES de qualquer criação (2 chamadas `ue5_execute`, invocações separadas, seguindo a regra "separar a verificação, nunca os passos da mutação")

**Chamada 1:**
- `EditorActorSubsystem.get_all_level_actors()` → **0 actores**
- `UnrealEditorSubsystem.get_editor_world()` → **None**
- `EditorAssetLibrary.does_asset_exist('/Game/Maps/MinPlayableMap')` → **False**
- `EditorLevelLibrary.spawn_actor_from_class(PointLight, ...)` → **None** (mutação falha silenciosamente — nenhum actor criado, nenhuma excepção)
- `frame_count` amostra 1 = 2331505

**Chamada 2 (invocação separada, ~15s depois):**
- `frame_count` amostra 2 = 2332413 (+908 frames → processo UE5 vivo, não crashado, não congelado — apenas o world do editor está indisponível)
- `AssetRegistryHelpers.get_asset_registry().get_asset_by_object_path('/Game/Maps/MinPlayableMap.MinPlayableMap')` → **válido (is_valid=True)**
- `LevelEditorSubsystem.get_current_level()` → **None**

## Conclusão

**Bloqueio de infraestrutura confirmado pela 8ª vez consecutiva na cadeia** (#04→#05→#06→#07→#08→#09→#10→#11). Este é o mesmo padrão que o Animation Agent #10 relatou no ciclo anterior: o `MinPlayableMap` está registado no AssetRegistry (persistido em disco, path válido) mas **não está carregado como world activo do editor** — `get_editor_world()=None`, `get_current_level()=None`, `get_all_level_actors()=0`, e qualquer tentativa de `spawn_actor_from_class` devolve `None` sem excepção.

**Não derivei nenhuma espécie nova, não criei nenhum Blueprint, não coloquei nenhuma criatura, e não editei o `creatures_registry`.** Fazê-lo exigiria um world mutável para:
1. Duplicar `BP_Creature_Triceratops` como base de asset (isto teoricamente não depende do world carregado — é operação de AssetTools/Content Browser)
2. Medir a posição do PlayerStart e das criaturas Dino_Triceratops_11..14 existentes no NÍVEL (isto SIM depende do world — `get_all_level_actors()=0` torna impossível medir distâncias reais)
3. Espaçar a nova criatura correctamente (impossível sem coordenadas reais de referência)
4. Fazer spawn de um actor Pawn no nível para prova por contagem de ossos (spawn devolve None)

Criar o Blueprint da espécie sem poder colocá-lo, medi-lo, e provar espaçamento seria produzir exactamente o tipo de "sucesso reportado sem evidência" que as regras anti-alucinação e a regra `hugo_r29_medir_depois_de_colocar` proíbem — colocar sem medir já causou um FAIL de portão em 05/08 (19m vs mínimo 20m). Sem world, não há medição possível, logo não há colocação responsável possível.

## Decisões técnicas
- Priorizei "VERIFIED IN WORLD" sobre inventar uma criação fictícia de espécie com coordenadas fabricadas.
- Não criei `.cpp`/`.h` (regra absoluta).
- Não usei `load_map()` (crasha o editor, confirmado em memórias).
- Não tentei workarounds de rede/HTTP dentro do Python do UE5 (proibido — bloqueia o bridge single-thread).

## Dependências / próximo agente (#12 Combat & Enemy AI Agent)
- **Repetir esta verificação de 2 chamadas** (frame_count + get_all_level_actors + get_editor_world + spawn_test) antes de assumir que existem Pawns/Characters acessíveis para IA de combate.
- Se o world recuperar (actor_count > 0 E editor_world != None E spawn devolver um actor real, confirmados em simultâneo), a criação de espécie derivada de `BP_Creature_Triceratops` fica pronta para execução imediata — a recipe está documentada em `hugo_criaturas_vivas` e não precisa de reinvenção.
- Sugestão de diagnóstico para quem tiver acesso a reiniciar o processo UE5: o `AssetRegistry` reconhece o mapa mas o `LevelEditorSubsystem`/`UnrealEditorSubsystem` não têm nenhum world carregado — isto é consistente com o editor ter sido aberto sem um nível persistente activo (ex.: crashou a meio de um `save_map` ou ficou num estado transitório pós-PIE). Um restart limpo do editor UE5 (fora do alcance deste agente) provavelmente resolve.

## Ficheiro criado
- `docs/status/2026-08-05_npc_behavior_agent_blocker.md` (este ficheiro)
