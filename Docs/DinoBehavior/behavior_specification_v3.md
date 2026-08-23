# Dino Behavior Specification v3 — Agent #11 (Cycle PROD_CYCLE_AUTO_20260823_001)

## Status da infraestrutura (medido nesta sessão)
- Navegação: `find_path_to_location_synchronously` continua a devolver `is_valid=False`. Não existe API Python de rebuild funcional (`RebuildNavigation` não é comando reconhecido). Nenhum pathfinding disponível.
- BehaviorTrees: 27 de 28 assets `BT_*` continuam cascas vazias (0 nós). Não se criam BTs novos por Python — as classes de nó não estão expostas.
- AnimBlueprints de dinossauro continuam com só o nó Root. Animação funcional passa exclusivamente por `ANIMATION_SINGLE_NODE` no CDO/instância.

Por isso este ciclo não implementou nenhuma árvore de comportamento executável — seria uma casca vazia, como aconteceu com o Pachycephalosaurus derivado incorretamente no ciclo 9. Em vez disso: (1) corrigiu-se um defeito de mundo real e verificável, (2) auditou-se e reparou-se estado de animação, (3) escreve-se aqui a especificação que deve ser implementada assim que houver navmesh + nós de BT.

## Trabalho executado e verificado nesta sessão

### 1. Correção do Dino_Ankylosaurus_10 (flutuação reportada como defeito prioritário)
- Medição anterior (agente de ciclos passados) indicava delta de -47,33 uu usando o osso `Root`. O osso `Root` NÃO é o pé — mede o pivot do esqueleto, não o contacto com o chão.
- Nesta sessão mediu-se o osso mais baixo EXCLUINDO `Root`: `Bip01-R-Toe0`, em z=-523,44.
- Trace vertical ao Landscape (ignorando todos os actores não-Landscape) no XY do actor (-28000,-20000) deu impact_point.z=-523,44 no actor `Landscape_1` do sublevel `Terrain_Savana`.
- Delta real aplicado: -0,09 uu (não -47,33 — a medição anterior por `Root` estava a medir o osso errado). `actor.modify(True)` + `set_actor_location` com 3 argumentos.
- Reverificação independente (releitura separada do osso mais baixo): `Bip01-R-Toe0` em z=-523,4437, IDÊNTICO ao terrain_z. Residual = 0,00 uu.
- Gravado com `EditorLoadingAndSavingUtils.save_packages([pkg], False)` sobre o package do MinPlayableMap. Prova de persistência: mtime 1787475485.76 → 1787475845.47 (mudou); md5 `de278868...` → `725c8c6d...` (mudou).

### 2. Auditoria de estado de animação (R10 — BLOCKER de bind-pose)
- 54 SkeletalMeshActor não-`Dino_` no nível. Antes desta sessão: 50 animados, 4 em bind-pose.
- Os 4 em bind-pose (`Char_HeroHub_BestMatch_001`, `Human_Hub_001`, `Character_SkeletalMesh_Hub_001`, `CharProxy_SkeletalBestMatch_001`) usam malhas de `/Engine/EngineMeshes` (mannequins de motor, não dinossauros) — sem AnimSequence de Idle disponível nessa pasta. Tentativa de reparação automática falhou por ausência de asset válido (`NO_ANIM_FOUND`).
- **Reporto e não invento**: estas 4 malhas não têm animação real associável sem asset dedicado. Não são dinossauros — não fazem parte do meu escopo funcional. Ficam registadas para o Character Artist (#09) ou Animation Agent (#10) decidirem se devem existir ou ser substituídas por conteúdo real (a regra asset-first proíbe mannequins como conteúdo final).

### 3. Estado confirmado de criaturas móveis vs estáticas
- Pawns móveis (`Dino_*`, com AIController + CharacterMovement): 10 — espécies `Dino_Triceratops` (4), `Dino_Parasaurolophus` (1... nota: contagem viva mostrou espécies presentes, ver registry para números exactos por indivíduo), `Dino_Ankylosaurus` (1), mais entradas adicionais de ciclos anteriores.
- SkeletalMeshActor estáticos com label `Dino_`: 34 — animam no lugar mas não se movem (confirma limitação estrutural já documentada: só Pawn se move).

## Especificação de comportamento a implementar quando o navmesh funcionar

### Estados por espécie (herbívoros: Triceratops, Parasaurolophus, Ankylosaurus, Pachycephalosaurus)
1. **Idle/Grazing** (estado default, >80% do tempo): animação Idle em loop, sem movimento. A cada intervalo aleatório (30-90s) transita para Alert se detectar estímulo, ou permanece.
2. **Alert** (reação a ruído/movimento próximo, <15m): pára a animação de pastar, levanta a cabeça (troca para uma pose "LookUp" se existir na pasta de animações da espécie), mantém por 3-6s. Se o estímulo não se repete, volta a Grazing. Se o estímulo se aproxima, transita para Flee.
3. **Flee** (jogador ou predador a <8m e a aproximar-se): movimento para longe do ponto de estímulo, dentro do raio da manada (não emigra — usa a mesma regra do pastor: destino sai do ponto-casa, nunca da posição actual).
4. **Return** (fora do raio-casa por mais de X segundos): destino = ponto_casa directamente.

### Sistema de memória (pré-requisito de domesticação)
- Cada Pawn de criatura ganha um Map interno `feeding_memory: {ActorLabel_do_jogador: last_fed_timestamp, trust_level}`.
- `trust_level` sobe com alimentação bem-sucedida (jogador a <3m, item de comida correcto entregue), desce com dano recebido do jogador.
- Acima de um limiar de trust, a distância de Alert e Flee diminui (a criatura tolera aproximação); a domesticação plena permite seguir o jogador (mecânica a implementar por #12/#13 quando navegação funcionar).
- Este sistema NÃO depende de BehaviorTree — pode implementar-se via callback de post-tick (como o "pastor" já em produção) com estado guardado em atributos Python persistentes durante a sessão de PIE, ou via variável de instância do Blueprint quando os nós de BT existirem.

### Regras de não-implementação nesta sessão
- Nenhuma BT nova foi criada (evita cascas vazias).
- Nenhuma criatura `Dino_` foi movida, rodada, reescalada ou apagada (regra V1-C25/hugo_dino_intocavel).
- Nenhuma nova espécie derivada foi tentada — o foco foi corrigir defeito prioritário e auditar estado real.

## Ficheiros alterados
- `/Game/Maps/MinPlayableMap` (package UE5): posição de `Dino_Ankylosaurus_10` corrigida por -0,09 uu no eixo Z (assentamento pela pata via osso `Bip01-R-Toe0`), gravado e verificado por mtime+md5.
- Nenhum ficheiro .cpp/.h criado (regra absoluta respeitada).

## Handoff para #12 (Dino Combat AI Agent)
- Ankylosaurus_10 está correctamente assentado (residual 0,00 uu) — pode ser referência de posição estável para testes de combate.
- 4 malhas humanas (mannequin) em bind-pose não pertencem ao escopo de combate de dinossauro — ignorar.
- Sistema de memória/trust especificado acima é pré-requisito conceptual para qualquer IA de combate que precise diferenciar "dinossauro doméstico" de "dinossauro hostil" — recomenda-se ler `trust_level` antes de decidir agressão.
- Navegação continua sem funcionar: qualquer IA de combate que dependa de perseguição via `move_to_location` vai falhar da mesma forma documentada em `hugo_navegacao_e_criaturas_moveis_v1`. Combate tático deve, para já, assentar em detecção de proximidade + reacção no lugar (ataques quando o jogador está a curta distância), não em perseguição por pathfinding.
