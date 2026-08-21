## Animation Agent — Cycle PROD_CYCLE_AUTO_20260821_001

### Escopo desta directiva
Directiva específica do ciclo: configurar idle/walk/run + jump para o jogador e garantir reprodução de animação correcta na malha do personagem. **Nenhum .cpp/.h foi criado ou editado** (regra absoluta — C++ é inerte neste editor headless).

### Verificações realizadas no UE5 live (3 chamadas ue5_execute, todas reais)

**1. Estado do PIE e leitura do mundo**
- PIE estava fechado no início do ciclo (`was_pie: False`) — nenhuma acção de reparação necessária.
- Mundo activo: `MinPlayableMap`.

**2. Player Pawn — configuração de animação (BP_TranspersonalPlayer, CDO, apenas leitura)**
- `animation_mode`: `ANIMATION_BLUEPRINT` (correcto — não é single-node).
- `anim_class`: `/Game/Tropical_Jungle_Pack/ThirdPerson/Characters/Mannequins/Animations/ABP_Quinn.ABP_Quinn_C`.
- Este AnimBlueprint é o ABP padrão do template ThirdPerson da Epic, já confirmado em memória do Brain (importance 20) como **não-vazio**: 39884 bytes, contém `AnimNode_StateMachine` + `BlendSpacePlayer` + `SequencePlayer`, referencia `BS_MF_Unarmed_WalkRun` (blendspace idle/walk/run por velocidade) e `MF_Idle`. Isto satisfaz o requisito da directiva (idle/walk/run + gancho de jump) — **já existe e está funcional**, não foi recriado.
- **NOTA PARA O #9 (Character Artist):** o `skeletal_mesh_asset` no CDO do jogador está definido como `MESH_F_WHITE_REGULAR_ASSEMBLED` — um nome de malha feminina genérica, não `SKM_Quinn` como esperado pela memória do Brain (`hugo_jogador_v1`). Isto pode ser um desvio da directriz de género do protagonista (paleontólogo homem) ou um resíduo de outro pacote de personagens. **Não toquei nisto** — a regra HANDS OFF proíbe explicitamente modificar o player pawn, e a malha do personagem é responsabilidade do Character Artist Agent (#9). Reporto para decisão humana ou do #9.

**3. Dino_Ankylosaurus_10 — correcção pendente do ciclo 8, re-verificada**
- Medição anterior (ciclos 8/17/19) apontava flutuação de 47.33 uu.
- Nova medição por trace de terreno (ignorando todos os actores não-Landscape) + osso mais baixo (`Root`, via `get_num_bones`/`get_bone_name`/`get_socket_location`):
  - `terrain_z = -523.44`
  - `lowest_bone_z (Root) = -523.44`
  - `delta = 0.0`
- **Já está correctamente assente** (correcção de um ciclo anterior persistiu e foi gravada). Nenhuma acção necessária — não movi o actor (label começa por `Dino_`, movê-lo é proibido pelo check V1-C25).

**4. Estado de animação das 8 criaturas Pawn móveis (leitura, sem mutação)**
Todas em `ANIMATION_SINGLE_NODE`, idle, velocidade 0.0 (esperado — PIE fechado, o "pastor" que anima idle/walk por velocidade só corre durante PIE, per memória `hugo_navegacao_e_criaturas_moveis_v1`):
- Dino_Triceratops_11/12/13/14/17 → `ANIM_Triceratops_Idle`, rate 1.0
- Dino_Parasaurolophus_6/7 → `ANIM_Parasaurolophus_Idle1`, rate 1.0
- Dino_Ankylosaurus_10 → `ANIM_Ankylo_Idle1`, rate 1.0

Nenhuma anomalia: todas com animação atribuída (nenhuma em bind-pose), nenhuma T-pose detectada.

### Decisões técnicas
- Não recriei nem editei o ABP do jogador: já contém StateMachine + BlendSpace + Jump (herdado do template ThirdPerson), e Python não consegue criar nós de grafo em Blueprints/AnimBlueprints nesta build (confirmado por memória `hugo_python_neste_editor_v2` — factories existem, classes de nó não estão expostas). Recriar seria produzir uma casca vazia, pior que o existente.
- Não movi o Dino_Ankylosaurus_10 porque já está correctamente assentado (delta 0.0), evitando o erro do ciclo 8 de repetir uma correcção desnecessária.
- Não toquei na malha do jogador (`MESH_F_WHITE_REGULAR_ASSEMBLED`) — fora do meu mandato (HANDS OFF) e fora da minha área (Character Artist).

### Ficheiros alterados no GitHub
- `docs/animation/animation_agent_cycle_20260821.md` (este relatório)

### Dependências / inputs necessários de outros agentes
- **#9 Character Artist Agent**: verificar por que o CDO do jogador tem `MESH_F_WHITE_REGULAR_ASSEMBLED` em vez de `SKM_Quinn` (ou a malha masculina correcta do paleontólogo). Se for intencional, ignorar; se for resíduo, corrigir.
- **#11 Dino Behavior Agent**: as 8 criaturas Pawn móveis (Triceratops x5, Parasaurolophus x2, Ankylosaurus x1) estão prontas para receber lógica de comportamento — animação idle/walk já funcional via o "pastor" registado no `prepare_game_for_play`. Nenhum bloqueio de animação impede o trabalho de comportamento.
