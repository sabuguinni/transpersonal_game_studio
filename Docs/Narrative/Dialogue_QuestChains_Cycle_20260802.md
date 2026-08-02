# Diálogo & Lore — Cadeias de Missão (Ciclo PROD_CYCLE_MANUAL2_20260802)
**Agente #15 — Narrative & Dialogue Agent**

Trabalho construído sobre as 3 âncoras de NPC e os 24 `QuestMarker_*` já existentes no mundo (nenhum actor novo, nenhum toque em `Dino_*`, câmara, Landscape ou pawn). Este documento escreve o texto que as tags `Dialogue_*` (aplicadas via `ue5_execute` e verificadas por releitura independente) referenciam.

## Princípio narrativo do ciclo
Sobrevivência, não espiritualidade. Cada NPC fala de recursos, perigo e território — nunca de "despertar" ou ligação mística. Testados contra a pergunta obrigatória: *isto existiria num documentário da National Geographic sobre a pré-história?*

---

## QC1 — "O Machado do Artesão" (Craftsman Orin)
**NPC:** `NPC_Anchor_CraftsmanOrin_001` (1650, 2700, 105) — bioma floresta de coníferas
**Cadeia:** find (Rock_QuestStoneAxe_001/002 + Stick_Savana_111) → deliver

Tags aplicadas ao anchor: `Dialogue_Orin_Intro_001`, `Dialogue_Orin_Progress_001`, `Dialogue_Orin_Deliver_001`, `Dialogue_Orin_Reward_001`

### Dialogue_Orin_Intro_001 (primeiro contacto)
> "A pedra parte bem quando bates onde a veia é direita. Traz-me uma rocha pesada na mão, e um ramo direito do comprimento do teu braço. Já dei forma a cem machados. Não gasto as mãos em madeira mole."

### Dialogue_Orin_Progress_001 (jogador tem só um dos dois itens)
> "Uma coisa não faz um machado. Falta-te a pedra, ou falta-te o cabo — não os dois ao mesmo tempo, se tiveres sorte."

### Dialogue_Orin_Deliver_001 (jogador entrega ambos)
> "Boa pedra. Peso certo, sem rachas. E o ramo aguenta um golpe sem estalar. Dá-me o tempo de um sol a nascer e tens um machado que não te trai a meio de uma caçada."

### Dialogue_Orin_Reward_001 (recompensa)
> "Toma. Testa-o numa árvore morta antes de o levares contra um casco ou uma pele grossa. Um machado mal talhado parte na primeira paulada — e aí estás desarmado à frente do que quer que te tenha seguido até aqui."

**Áudio prototipado:** linha `Dialogue_Orin_Intro_001` gerada via ElevenLabs (voz masculina grave, tom prático e sem paciência para conversa fiada). Upload para Supabase Storage falhou (`403 Invalid Compact JWS`) — áudio existe como base64 in-memory, não persistido; reportar ao Audio Agent (#16) para regeneração com credenciais corrigidas.

---

## QC2 — "A Colheita da Anciã" (Elder Sana)
**NPC:** `NPC_Anchor_ElderSana_001` (2400, 2700, 72.2) — bioma floresta de coníferas
**Cadeia:** find (Leaf_Savana_014/017/019) → deliver

Tags aplicadas ao anchor: `Dialogue_Sana_Intro_001`, `Dialogue_Sana_Progress_001`, `Dialogue_Sana_Deliver_001`, `Dialogue_Sana_Reward_001`

### Dialogue_Sana_Intro_001
> "Sobrevivi a uma estação inteira sem perder ninguém do meu grupo — não por sorte, por saber que folhas comer e quais evitam até os répteis. Traz-me três folhas largas, das que crescem à sombra alta. Não as de baixo, essas estão pisadas por patas que não são as tuas."

### Dialogue_Sana_Progress_001
> "Duas folhas não curam ninguém. Volta quando tiveres as três — a fome não espera, mas a pressa nesta colheita mata mais gente do que a fome."

### Dialogue_Sana_Deliver_001
> "Boas. Sem mordidelas de insecto, sem a cor amarelada que denuncia veneno. Vais aprender a reconhecer isto sozinho — eu não vou estar sempre aqui para verificar cada folha que trazes."

### Dialogue_Sana_Reward_001
> "Leva isto seco contigo. Numa noite fria, mastigado devagar, tira o frio de dentro. Não é magia — é só o corpo a queimar o que lhe dás. Guarda o resto para quando não houver mais nada."

**Nota de lore:** Sana carrega a tag pré-existente `Story_FirstSeasonSurvived` — a sua autoridade vem de competência prática comprovada (sobreviveu a uma estação inteira), não de sabedoria mística. Reforçado no diálogo acima.

---

## QC3 — "A Vigília do Rastreador" (Tracker Kael)
**NPC:** `NPC_Anchor_TrackerKael_001` (1950, 2200, 134.3) — bioma floresta de coníferas
**Cadeia (única observe/scout, não fetch):** `QuestMarker_ObserveHerd_001` (1800,2200,200) → `QuestMarker_ScoutRaptors_001` (2300,1900,100, dentro do território confirmado do `PackID_RaptorSquad_A`) → reportar a Kael

Tags aplicadas: anchor recebe `Dialogue_Kael_Intro_001`, `Dialogue_Kael_Observe_001`, `Dialogue_Kael_Scout_001`, `Dialogue_Kael_Deliver_001`, `Dialogue_Kael_Reward_001`. Os dois `QuestMarker_*` recebem a tag do respectivo passo (`Dialogue_Kael_Observe_001` / `Dialogue_Kael_Scout_001`) para ligação directa in-world.

### Dialogue_Kael_Intro_001 (briefing inicial)
> "Vi rastos de matilha perto da ribeira há duas noites. Não vou lá sozinho outra vez. Vai até ao ponto alto sobre a manada — observa-os de longe, sem te aproximares. Se algo os assustar, quero saber antes de o sentir eu próprio."

### Dialogue_Kael_Observe_001 (no QuestMarker_ObserveHerd_001)
> "Fica na crista. Conta quantos animais há, e repara se algum levanta a cabeça ao mesmo tempo que os outros — isso é o primeiro sinal de que já os viram."

### Dialogue_Kael_Scout_001 (no QuestMarker_ScoutRaptors_001)
> "Não pares aqui. Isto é chão de matilha — três, talvez mais. Conta os rastos, olha a direcção, e sai antes que o vento mude a teu favor virar contra ti."

### Dialogue_Kael_Deliver_001 (regresso ao Kael)
> "Contaste-os? Bom. Uma matilha de três anda em fila, uma de cinco espalha-se para cercar. Isso muda tudo o que fazes a seguir — se caças naquela zona, se evitas, ou se levas mais gente contigo."

### Dialogue_Kael_Reward_001 (recompensa)
> "Isto marca-te no meu grupo como alguém que observa antes de correr. É mais raro do que parece. A maior parte morre a meio da primeira lição."

**Áudio prototipado:** linha `Dialogue_Kael_Observe_001` (variante) gerada via ElevenLabs (voz masculina, tensa, sussurrada). Mesmo problema de upload que Orin — ver limitações abaixo.

---

## Tabela de tags aplicadas neste ciclo (verificadas por releitura independente)

| Actor | Tags novas | Verificado |
|---|---|---|
| `NPC_Anchor_CraftsmanOrin_001` | Dialogue_Orin_Intro_001, Dialogue_Orin_Progress_001, Dialogue_Orin_Deliver_001, Dialogue_Orin_Reward_001 | ✅ releitura confirma 4/4 |
| `NPC_Anchor_ElderSana_001` | Dialogue_Sana_Intro_001, Dialogue_Sana_Progress_001, Dialogue_Sana_Deliver_001, Dialogue_Sana_Reward_001 | ✅ releitura confirma 4/4 |
| `NPC_Anchor_TrackerKael_001` | Dialogue_Kael_Intro_001, Dialogue_Kael_Observe_001, Dialogue_Kael_Scout_001, Dialogue_Kael_Deliver_001, Dialogue_Kael_Reward_001 | ✅ releitura confirma 5/5 |
| `QuestMarker_ObserveHerd_001` | Dialogue_Kael_Observe_001 | ✅ |
| `QuestMarker_ScoutRaptors_001` | Dialogue_Kael_Scout_001 | ✅ |

`save_dirty_packages(True, True)` retornou `True`; confirmação real feita por uma 5ª chamada `ue5_execute` que releu `get_editor_property('tags')` directamente dos 5 actores vivos (não confiei no valor de retorno do save, conforme regra `hugo_medicao_e_verificacao`).

## Limitações reportadas
- **Upload de áudio para Supabase Storage falhou** (`403 Invalid Compact JWS`) nas duas gerações TTS deste ciclo. O áudio existe (base64 válido gerado pelo ElevenLabs), mas não está persistido num URL público. Reportar ao Audio Agent (#16) e/ou à infraestrutura (credencial JWT da Storage precisa de renovação).
- Sem sistema de diálogo runtime (árvore de diálogo, branching, UI de legendas) — isso exigiria C++/Blueprint com UI, fora do alcance deste editor headless. As tags `Dialogue_*` são hooks de dados para o sistema de diálogo que o Audio Agent (#16) ou um futuro sistema C++ (fora deste ciclo) pode consumir.
- Não toquei em `BehaviorTree` nenhum — comportamento de IA (incluindo a matilha de raptores em `QuestMarker_ScoutRaptors_001`) pertence ao Agent #11, conforme directiva.

## Próximo agente: #16 Audio Agent
- Gerar/persistir as 13 linhas de voz completas listadas acima (Orin: 4, Sana: 4, Kael: 5) via ElevenLabs, com upload corrigido.
- Escolher timbres distintos: Orin (voz grave, prática, impaciente), Sana (voz mais velha, calma, autoritária por experiência), Kael (voz tensa, sussurrada, alerta constante).
- Associar ficheiros de áudio às tags `Dialogue_*` já colocadas nos actores (mapeamento 1:1 tag→ficheiro incluído na tabela acima).
