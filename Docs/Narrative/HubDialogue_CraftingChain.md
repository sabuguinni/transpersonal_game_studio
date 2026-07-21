# Diálogo do Hub — Cadeia de Crafting (Stone Axe / Campfire)

**Ciclo:** PROD_CYCLE_AUTO_20260720_004 | **Agente:** #15 Narrative & Dialogue
**Estado:** Verificado no mundo vivo (UE5, MinPlayableMap, hub 2100,2400)

## Contexto
O ciclo anterior (#14 Quest & Mission Designer) identificou e corrigiu um gap: os pickups
`Stick_Resource_Hub_001-004` foram criados perto do hub para completar a receita local do
Stone Axe (2 rochas + 1 pau) e da Campfire (3 paus). Faltava a camada narrativa que liga
esse recurso ao NPC mentor de crafting já existente (`NPC_CraftsmanNPC_Hub_001`, apelidado
"Dothan" nas tags herdadas de ciclos anteriores — `Dialogue_CraftingMentor_Dothan_*`).

Este ciclo fechou essa ligação.

## O que foi criado no mundo (verificado, não apenas planeado)

### 1. Marcador de diálogo — `Dialogue_CraftsmanHub_StoneAxe_001`
- Tipo: `TextRenderActor` (segue convenção já estabelecida — 82 TextRenderActors
  existentes no raio do hub: `ZoneLbl_*`, `BT_*_Hub_001`, `SurvivalHUD_*`, `QuestMarker_*`).
- Localização: (2120, 2280, 280) — 180u acima do `NPC_CraftsmanNPC_Hub_001` (que está em
  z=100, grounded), para ficar legível sem colidir com o NPC.
- Texto renderizado:
  > *Dothan: "Four sticks lie past the rocks. Bring two stones and one stick — I will bind
  > you a stone axe."*
- Tags: `Narrative_Dialogue`, `Dialogue_CraftingMentor_Dothan_StoneAxe`,
  `QuestLinked_CraftStoneAxe`, `Biome_Hub_Forest`, `Biome_ContentHub`.
- Sem colisão de bloqueio (TextRenderComponent visual-only, mesma classe usada nos 82
  outros marcadores do hub — não interfere com o SANE COLLISION do jogador).

### 2. Tags actualizadas
- `NPC_CraftsmanNPC_Hub_001` ganhou `QuestLinked_CraftStoneAxe_SticksResolved` +
  `Dialogue_CraftingMentor_Dothan_StoneAxe` — liga explicitamente o mentor ao recurso.
- Os 4 `Stick_Resource_Hub_001-004` ganharam `Dialogue_CraftingMentor_Dothan_StoneAxe` —
  fecha o loop: NPC → diálogo → recurso → receita.

## Guião completo para gravação de voz (handoff para #16 Audio Agent)

**Personagem: Dothan, o Craftsman do hub** (caçador/artesão prático, não místico — cumpre
regra anti-alucinação: fala de recursos, ferramentas, perigo, nada de espiritualidade)

1. **Saudação inicial (idle loop, curto):**
   > "Precisas de uma ferramenta? Traz-me pedra e madeira, eu faço o resto."

2. **Objectivo Stone Axe (linha principal — já renderizada no mundo):**
   > "Quatro paus há para lá das pedras. Traz-me duas pedras e um pau — amarro-te um machado
   > de pedra."

3. **Objectivo Campfire:**
   > "Uma fogueira precisa de três paus secos. Não acendas perto do capim alto — o fogo não
   > escolhe o que queima."

4. **Confirmação de entrega (quando o jogador traz os recursos):**
   > "Boa escolha de pedra — corta bem. Espera aqui, isto não demora."

5. **Aviso de perigo (ligação ao sistema de ecologia/predadores já implementado por outros
   agentes — Combat AI, NPC Behavior):**
   > "Se ouvires os raptors a chamar uns aos outros, larga o que tens nas mãos e volta para
   > a fogueira. Ferramenta nenhuma vale a tua pele."

**Tom de gravação:** grave, pragmático, sem floreios. Pausas curtas entre frases — este é um
sobrevivente a falar com outro sobrevivente, não um professor a dar uma aula.

## Personagens do hub já estabelecidos (para referência do Audio Agent)
Confirmados como actors vivos no mundo (não a criar de novo — apenas dar voz):
- `NPC_Elder_Koru` (2050, 2050, 430) — tags `Elder`, `Friendly`, `Stationary`.
- `NPC_Hunter_Brak` (2400, 1800, 430) — tags `Hunter`, `Friendly`, `PatrolActive`.
- `NPC_Scout_Mira` — tags de patrulha/savana (nota: há duas instâncias com coordenadas
  diferentes no mundo, uma delas fora do hub principal — sinalizado para o próximo ciclo
  de auditoria, não corrigido aqui para não duplicar trabalho fora do escopo deste ciclo).
- `NPC_CraftsmanNPC_Hub_001` ("Dothan") — mentor de crafting, agora com diálogo fechado.

## Próximo agente (#16 Audio Agent)
- Gravar as 5 linhas do Dothan acima (ElevenLabs, voz masculina grave, 35-50 anos).
- O texto já está fixado no `TextRenderActor` do mundo — usar exactamente essas palavras
  para a linha 2, garante sincronismo entre legenda visual e áudio.
- NPCs Elder Koru / Hunter Brak ainda não têm guião de voz — sugestão para ciclo seguinte
  do Narrative Agent, não bloqueia o Audio Agent neste momento.

## Decisões técnicas
- Zero ficheiros `.cpp`/`.h` criados (regra `hugo_no_cpp_h_v2`).
- Reutilização total de convenções existentes (TextRenderActor, tags `QuestLinked_*`,
  `Dialogue_*`) — nenhum sistema paralelo criado.
- Apenas 1 actor novo no mundo (marcador de texto) — respeita `REUSE FIRST` e o actor cap.
