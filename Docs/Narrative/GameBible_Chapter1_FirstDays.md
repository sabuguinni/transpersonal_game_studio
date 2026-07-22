\# GAME BIBLE — Capítulo 1: Os Primeiros Dias

## Contexto de produção (verificado neste ciclo, PROD_CYCLE_AUTO_20260722_001)

Este documento amarra **beats narrativos reais** às tags que já existem no mundo vivo (MinPlayableMap, zona do hub em X=2100,Y=2400). Nenhum actor novo foi criado — apenas tags `Narr_*` / `NarrativeBeat_*` / `LoreLine_*` foram adicionadas a actores já existentes (reuse-first, conforme `hugo_naming_dedup_v3`).

Actores verificados e tags confirmadas via `ue5_execute` neste ciclo:

| Actor | Localização | Tags novas desta sessão |
|---|---|---|
| `CraftingMenuTrigger_Hub_001` | (2100, 2400, 144) | `NarrativeBeat_FirstTool`, `LoreLine_CraftStoneAxe` |
| `Trigger_Quest_CraftStoneAxe_001` | (2026, 2135, 139) | `NarrativeBeat_StoneAxe_Pressure`, `LoreLine_FirstWeapon` |
| `Helper_Actor_Raptor_Hub_001_Posed` | (2500, 2100, 98) | `NarrativeBeat_RaptorThreat`, `LoreLine_PackHunters` |
| `Helper_Actor_Raptor_Hub_002_Posed` | (2100, 2950, 100) | `NarrativeBeat_RaptorThreat`, `LoreLine_PackHunters` |
| `Helper_Actor_Brach_Hub_001` | (2220, 2400, 99) | `NarrativeBeat_HerdSafety`, `LoreLine_GentleGiants` |

Nota: `Helper_Actor_Raptor_Hub_001/002` já traziam tags de outros agentes (#11 NPC Behavior, #12 Combat AI, #16 Audio — `Narr_Beat_FirstPredatorWarning`, `VO_Cue_Survivor_PredatorWarning`). Este ciclo **reforça** essas tags com a camada de Bible (`NarrativeBeat_RaptorThreat`), sem duplicar sistemas.

---

## Premissa (recapitulação, realismo obrigatório)

O jogador é um sobrevivente solitário largado na savana do Cretáceo. Não há magia, não há espíritos, não há "despertar de consciência". Há fome, sede, frio à noite, predadores em bando, e a competência de quem sabe fazer uma ferramenta afiada com uma pedra e um pau.

O arco do Capítulo 1 é: **sobrevivente isolado → sobrevivente equipado → sobrevivente que entende o território.**

---

## Beat 1 — `NarrativeBeat_FirstTool` / `LoreLine_CraftStoneAxe`
**Anchor:** `CraftingMenuTrigger_Hub_001` (2100,2400)

O jogador chega ao clareira do hub com mãos vazias. A primeira pressão do jogo não é um monstro — é a ausência de ferramenta. Sem machado, não corta madeira, não abre carcaça, não se defende a sério.

**Linha de diálogo interno (texto UI, não cutscene):**
> "Mãos nuas não cortam nada aqui. Preciso de uma pedra com fio e um cabo forte."

**Design intent:** o trigger de crafting É a cena. Não há cutscene a explicar — o jogador aperta "C", vê a receita (2 Rock + 1 Stick), e a compreensão nasce da mecânica (princípio Kojima: o jogo ensina jogando).

---

## Beat 2 — `NarrativeBeat_StoneAxe_Pressure` / `LoreLine_FirstWeapon`
**Anchor:** `Trigger_Quest_CraftStoneAxe_001` (2026,2135)

A pressão (McKee) que força a primeira escolha real: ficar parado a recolher recursos com um bando de Raptors a ~450 unidades de distância, ou recuar. O jogo não diz "tens medo" — o jogador sente a distância.

**Linha (texto de objetivo, HUD):**
> "Machado de pedra: 2 Rock, 1 Stick. Os Raptors não atacam enquanto patrulham — mas patrulham perto."

**Linha (diálogo do sobrevivente, quando o machado é craftado):**
> "Primeira arma de verdade. Ainda não é suficiente contra um bando — mas já corto, já talho, já me defendo de um sozinho."

---

## Beat 3 — `NarrativeBeat_RaptorThreat` / `LoreLine_PackHunters`
**Anchor:** `Helper_Actor_Raptor_Hub_001_Posed` + `Helper_Actor_Raptor_Hub_002_Posed`

Estes dois Raptors já têm IA de flanking configurada por #12 (`CombatTactic_FlankAndSurround`, `PackRole_Beta_FlankRight`) e aviso sonoro por #16 (`SFX_Roar_LowGrowl_Radius2000`). A camada narrativa acrescenta o **porquê** disto importar para a história: é o primeiro sinal de que o território tem donos, e o jogador é o intruso.

**Linha (aviso ambiental, disparado por `VO_Trigger_SightRadius_2500` já existente):**
> "Dois. Nunca há só um. Se avistei dois, há mais perto — e não vou ver os outros até ser tarde."

**Tema de escrita (`Narr_Theme_FlankAwareness`, já tag existente):** todo o texto relacionado com Raptors nesta zona deve reforçar contagem de bando e flanco, nunca "monstro" genérico — o jogador aprende ecologia de predador através da ameaça.

---

## Beat 4 — `NarrativeBeat_HerdSafety` / `LoreLine_GentleGiants`
**Anchor:** `Helper_Actor_Brach_Hub_001` (2220,2400)

Contraste deliberado ao Beat 3. O Brachiosaurus é `CombatStyle_Flee_Only`, `AttackDamage_0` — nunca ataca. Narrativamente, é o sinal de "zona relativamente segura": onde há Brach pastando calmo, não há predador activo por perto (regra de leitura de ambiente que o jogo ensina sem HUD).

**Linha (pensamento do sobrevivente, primeira vez que vê o rebanho):**
> "Enquanto os gigantes pastam tranquilos, ninguém os está a caçar agora. Aprendi a ler isso primeiro — antes de aprender a ler o resto."

**Design intent:** esta linha estabelece uma regra de jogo ensinada por diálogo interno, sem tutorial explícito — reforça o arco "sobrevivente que entende o território" mencionado na premissa.

---

## Dependências para próximos agentes

- **#16 Audio Agent**: as linhas de texto acima (Beat 1-4) precisam de gravação por `text_to_speech` como voz interior do sobrevivente (tom pragmático, sem dramatismo místico). Sugestão de voice_id neutro/grave. As tags de áudio já existentes (`SFX_Roar_LowGrowl_Radius2000`, `VO_Cue_Survivor_PredatorWarning`) já cobrem o ambiente — falta só a camada de voz interior do jogador para os 4 beats.
- **#14 Quest Designer**: `Trigger_Quest_CraftStoneAxe_001` já está ligado a `Quest_CraftStoneAxe`; sugerimos encadear Beat 2 → Beat 3 como sequência de tutorial natural (craft → primeiro encontro com ameaça).
- **#18 QA**: validar que as tags `Narr_*` não colidem com nomes usados por #11/#12 (verificado neste ciclo: coexistem sem conflito, ver lista completa acima).

## Ficheiros
- `Docs/Narrative/GameBible_Chapter1_FirstDays.md` (este ficheiro)
