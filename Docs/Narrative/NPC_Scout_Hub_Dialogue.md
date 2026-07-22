
# Diálogo — NPC_Scout_Floresta_001 (Hub Crafting Quest Giver)

## Contexto de mundo (verificado neste ciclo)
- Actor: `NPC_Scout_Floresta_001`
- Estava a flutuar em `z=1176` (muito acima do hub, chão real ~z100-135). Reposicionado para `(2078.7, 2172.8, 135.0)`, 144 unidades do `CraftingUITrigger_Hub_001` (2198.7, 2252.8, 135.0).
- Tags de diálogo/quest adicionadas ao actor (sem remover as 29 tags de comportamento já existentes — patrulha, ameaça, biome, etc.):
  - `Dialogue_QuestGiver_Crafting`
  - `Quest_PrimeiroCorte`
  - `Quest_PrimeiraNoite`
  - `Quest_AguaQueDura`
  - `DialogueID_ScoutHubIntro_001`

Este NPC é um **batedor** (scout) prático — não um guia espiritual. Fala de perigo, recursos e sobrevivência. Sem misticismo, sem "despertar", sem conexão sobrenatural com a fauna.

---

## Linha de diálogo — Introdução (dispara ao entrar em `CraftingUITrigger_Hub_001`)

**NPC_Scout_Floresta_001:**
> "Ainda não tens nada nas mãos. Isso mata mais depressa que qualquer dente lá fora."
> "Pedra, pau, fibra — o que está no chão à tua volta é o que te mantém vivo esta noite."
> "Aperta 'C' e olha para o que consegues fazer com as mãos que tens."

## Missão 1 — "O Primeiro Corte" (unlock: Machado de Pedra)
**Gatilho:** jogador abre o menu de crafting pela primeira vez.

**NPC_Scout_Floresta_001:**
> "Uma pedra com fio corta lenha, corta carne, corta pescoço se for preciso. É a primeira coisa que fazes, sempre."
> "Precisas de uma pedra com aresta e um pau direito. Há de ambos ali perto — não vás longe demais à procura."

**Objectivo:** recolher 1x Rock + 1x Stick → craft Machado de Pedra.
**Recompensa narrativa:** o Scout reconhece o jogador como "alguém que já não chega de mãos vazias".

## Missão 2 — "A Primeira Noite" (unlock: Fogueira)
**Gatilho:** conclusão da Missão 1, ou anoitecer próximo do jogador.

**NPC_Scout_Floresta_001:**
> "Machado feito. Bom. Agora pensa no que vem depois do sol se ir embora."
> "Sem fogo, a noite não perdoa — nem o frio, nem o que caça no escuro. Traz lenha e folha seca antes que escureça."

**Objectivo:** recolher Lenha (Stick pile) + Folhas secas (Leaf) → craft Fogueira.
**Consequência de falha:** se o jogador não tiver fogueira ao anoitecer, perda de temperatura corporal acelerada (gancho para sistema de sobrevivência, não narrativo).

## Missão 3 — "Água que Dura" (unlock: Recipiente de Água)
**Gatilho:** conclusão da Missão 2.

**NPC_Scout_Floresta_001:**
> "Fogo aceso, isso resolve a noite. Mas não vais viver de rio em rio o dia inteiro."
> "Faz um recipiente — casca, pele curtida, o que aguentar água sem se desfazer. Enche-o sempre que passares por água limpa."

**Objectivo:** craft Recipiente de Água usando recursos já auditados no hub (Hide/Leaf conforme receita do #14).
**Fecho de arco:** o Scout deixa de repetir tutoriais e passa a comentar o estado do jogador (fome/sede/temperatura) de forma pragmática — gancho para o Audio Agent (#16) adicionar variações de voz reactivas.

---

## Tom e regras de escrita (para consistência com #16 Audio e #14 Quest)
- Frases curtas, directas, sem floreado. O Scout fala como alguém cansado que já viu gente morrer por falta de preparação.
- Nunca usar: "sentir a energia", "conexão com a floresta", "escutar os espíritos". Usar sempre: recursos, distância, tempo, perigo concreto.
- Cada linha deve ser gravável isoladamente (ver dependência de VO abaixo).

## Dependências para #16 Audio Agent
- Gravar as 5 linhas acima como VO de `NPC_Scout_Floresta_001` (voz masculina/neutra, tom grave-cansado, sotaque neutro).
- Anexar áudio ao actor via `AmbientSound` component junto ao trigger `CraftingUITrigger_Hub_001`, disparado por overlap (mesma trigger já criada pelo #14 — não duplicar volume).
