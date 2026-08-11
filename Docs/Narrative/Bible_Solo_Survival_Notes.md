# A Bible do Jogo — Notas de Campo do Paleontólogo Solitário
### Narrative & Dialogue Agent #15 — Ciclo PROD_CYCLE_AUTO_20260811_001

## 0. Correção de contaminação (feita neste ciclo, verificada em mundo vivo)

Auditoria encontrou 9 actores `QuestGiver_*` (NPCs humanos nomeados: "Korg Tribal Elder", "Uga Survivor",
"Hunter Kael", "Gatherer Vera" etc.) e 4 actores de artesão fantasma (`NPCDialogue_ToolmakerNPC_HubGrazing_001`,
`NPC_Anchor_CraftsmanOrin_001`, `QuestChain_ToolmakersTrial_Start/Complete`) — resíduo directo de ciclos
anteriores que violava a regra fundamental do jogo: **o protagonista está sozinho, não existem outros humanos**.

Estes 13 actores foram **retagueados** (não apagados — a remoção de actores está fora do escopo deste agente
sem coordenação com #14/#19, que gerem os objectivos e a build). Todas as tags `VO_*`, `DialogueText_*`,
`HasDialogue*`, `Audio_VO_*`, `NarrativeRole_*`, `QuestGiver*`, `TribalElder`, `CraftsmanOrin`, `ToolmakerNPC`,
`ToolmakersTrial` foram removidas. Cada actor recebeu:
- `Narrative_SoloPlayerVoice` (marca de conteúdo pertencente à voz do protagonista, não a um NPC)
- Uma tag `Narrative_FieldNote_<Tópico>` específica (ex.: `Narrative_FieldNote_TrikeTracks_Fresh`,
  `Narrative_FieldNote_SnareRisk_Undergrowth`, `Narrative_FieldNote_PriorInhabitantTools`)

Verificado por releitura independente em invocação separada: 13/13 sem contaminação residual, 40/40 `Dino_`
intactos (contados, nunca movidos).

**Nota sobre "Prior Inhabitant":** `NPC_Anchor_CraftsmanOrin_001` foi reformulado como vestígio arqueológico —
ferramentas abandonadas de posse desconhecida, não um artesão activo. O protagonista encontra objectos, nunca
uma pessoa. Isto preserva a pista de progressão de crafting sem reintroduzir um NPC.

---

## 1. Premissa

Um paleontólogo é transportado para o Cretáceo Superior. Não há outros humanos. Não há bases, aldeias,
sobreviventes, guias. Há ele, o conhecimento que trouxe na cabeça, e o que o terreno lhe dá para transformar
em ferramenta. A pressão do jogo não vem de personagens que o desafiam — vem da fome, da sede, da temperatura,
e de predadores que não sabem que ele é inteligente, só que é presa possível.

**Arco central: "The Hunt for a Way Home".** A segunda gema — a primeira peça de retorno — está escondida no
bioma Snowy Rockside. Tudo o que ele faz entre o ponto de spawn e aquele bioma é sobrevivência instrumental:
mantém-se vivo o suficiente para chegar lá.

## 2. A voz — como se escreve o protagonista sozinho

Sem interlocutor, a única ferramenta narrativa é a voz interior e a fala solitária. Duas formas:

### 2.1 Notas de campo (registo científico, primeira pessoa, presente ou passado recente)
Formato: observação → identificação → inferência de comportamento → risco imediato.
Exemplo (Triceratops):
> "Pegadas em ferradura, quatro dedos anteriores, muito espaçadas — corrida, não caminhada. Triceratops adulto,
> provavelmente perturbado por algo maior do que eu. Vou seguir o rasto ao contrário."

### 2.2 Fala solitária (curta, reactiva, sem narrador)
Disparada por eventos: primeira visão de uma espécie, quase-morte, sucesso de fabrico.
Exemplos:
- Ao ver um Ankylosaurus por perto: *"Não corras. Aquilo não te vê como perigo — ainda."*
- A escapar de um Velociraptor por pouco: *"Isso foi... isso foi demasiado perto."*
- Ao terminar uma lança improvisada: *"Não é bonita. Mas corta."*

Regra dura: nunca um NPC responde. Nunca há uma segunda voz humana no áudio.

## 3. Fichas de espécie (dados narrativos, não de gameplay — o #11/#12 definem comportamento real)

**Triceratops** — Herbívoro, porte grande (até 9m), gregário em pequenos grupos. Perigo: baixo se não
provocado; carga defensiva se a cria for ameaçada. Sinal de aproximação: bufos curtos, cabeça baixa,
raspar do chanfro no solo.

**Parasaurolophus** — Herbívoro, porte grande, cristas usadas para chamadas de longo alcance. Perigo:
nulo directo; risco indirecto — a sua fuga em massa denuncia um predador próximo. Sinal: chamada grave e
ressonante antes de dispersão súbita da manada.

**Ankylosaurus** — Herbívoro blindado, lento, cauda em maça. Perigo: baixo à distância, alto em contacto —
a cauda quebra ossos. Sinal de aproximação: arrasta a cauda audivelmente antes de girar o corpo para a ameaça.

**Velociraptor** — Carnívoro pequeno, rápido, caça em pares ou trios coordenados. Perigo: alto por
emboscada, não por força bruta. Sinal: silêncio súbito da fauna menor, movimento periférico duplicado.

**Tsintaosaurus** — Herbívoro de porte médio, crista óssea única. Perigo: baixo. Sinal: pastagem tranquila,
dispersa ao mínimo ruído — um bom indicador de que a zona está livre de predadores grandes.

## 4. O Regresso

Ele sabe, desde o início, duas coisas: como chegou (um evento que ainda não compreende totalmente — não é
magia, é um fenómeno que o atirou para trás no tempo, tratado com a mesma frieza científica que tudo o resto)
e o que precisa para voltar — a segunda gema, no Snowy Rockside, um bioma hostil pelo frio, não pelo
misticismo. A jornada até lá é a estrutura do jogo: cada bioma atravessado é uma nota de campo a mais,
uma ficha de espécie a mais, uma fala solitária a mais.

## 5. Amostra de voz gerada este ciclo

Fala de campo (Tsintaosaurus/território) — gerada via ElevenLabs TTS, ficheiro em Supabase Storage:
> "Territorial gouges, three claws, spaced for a biped. Not a raptor — too wide. Tsintaosaurus, maybe,
> marking a boundary. I should move downwind before I find out which way it went."

## 6. Entregável ao próximo agente (#16 Audio)

- 13 actores no mundo agora carregam tags `Narrative_FieldNote_*` + `Narrative_SoloPlayerVoice` — pontos
  prontos para ancorar VO do protagonista (não de NPC).
- 4 fichas de espécie prontas para narração ambiente/diegética.
- Amostra de voz de referência já gerada — tom: seco, pragmático, científico, curto.
- PROIBIDO para o #16: qualquer segunda voz humana, música ou SFX que sugira presença de outro humano
  (vozes de multidão, tambores tribais, cantos). O áudio humano no jogo é UMA voz: a do protagonista.
