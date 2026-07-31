
# Narrative & Dialogue — Ciclo PROD_CYCLE_MANUAL_20260731

## Contexto recebido do #14 (Quest & Mission Designer)
O #14 identificou que o sistema de crafting está completo (3 receitas, trigger de UI, NPC mentor Dothan/Orin com VO gravado) e pediu para eu **rever a coerência entre `Lore_ToolmakerRemnant` e o arco do NPC craftsman**.

## Auditoria feita (mundo vivo, 3329 actores)
Encontrei **4 actores narrativos distintos** que referenciam o mesmo conceito ("o mestre artesão") mas nunca estavam ligados entre si por tag:

| Actor | Classe | Papel no sistema | Localização |
|---|---|---|---|
| `NPC_Anchor_CraftsmanOrin_001` | StaticMeshActor | Âncora de diálogo do mentor Orin (dá a quest `Quest_CraftStoneAxe`) | (1650, 2700, 105) |
| `NPCDialogue_ToolmakerNPC_HubGrazing_001` | TriggerSphere | Trigger de UI que inicia a cadeia de diálogo de crafting | (2150, 2450, 240) |
| `QuestChain_ToolmakersTrial_Start` | Actor | Estado de início da quest chain "Toolmaker's Trial" | (2050, 2380, 90) |
| `QuestChain_ToolmakersTrial_Complete` | Actor | Estado de fim da quest chain | (2150, 2350, 105) |
| `Rock_Savana_005`, `Rock_Savana_006` | StaticMeshActor | Tinham a tag `Lore_ToolmakerRemnant` (recursos de pedra com lore associada) mas nenhuma ligação explícita ao Orin | (2842, 2908, 100) / (1591, 3142, 230) |

**Problema de coerência**: eram 6 actores espalhados (distâncias 193–575uu entre si) a representar a mesma personagem/arco narrativo sem nenhuma tag comum — risco de um agente futuro os tratar como conceitos não relacionados, ou pior, duplicar um "Orin" novo por não reconhecer o vínculo (regra `hugo_naming_dedup_v3`).

## Decisão narrativa (McKee: pressão revela carácter)
Orin não é "só um professor de crafting" — ele é o **sobrevivente de uma cadeia de conhecimento prática**: aprendeu a talhar pedra com um artesão mais velho que morreu de descuido (poeira de sílex, não por dinossauro). Isto dá:
- **Pressão real**: Orin sabe que o conhecimento morre com as pessoas se não for passado. Motiva-o a ensinar o jogador com urgência, não por bondade abstrata.
- **Tema de sobrevivência, sem misticismo**: morte por exposição a poeira/descuido, não por "espírito ancestral". Consistente com a regra anti-alucinação.
- **Jogabilidade > cutscene (Kojima)**: o vínculo é contado através das PEDRAS com a tag `Lore_ToolmakerRemnant` que o jogador encontra ao recolher recursos de crafting — não uma cutscene, mas o próprio acto de minerar pedra que revela o passado.

## Alterações reais feitas no mundo (verificadas via `ue5_execute`)
Sem duplicar nenhum actor (regra `hugo_naming_dedup_v3`), adicionei tags de ligação:
- `Narrative_LoreLink_ToolmakerOrin` → `NPC_Anchor_CraftsmanOrin_001`, `NPCDialogue_ToolmakerNPC_HubGrazing_001`, `QuestChain_ToolmakersTrial_Start`, `QuestChain_ToolmakersTrial_Complete` (todos confirmados `True` na escrita)
- `Narrative_LoreLink_ToolmakerOrin_Origin` + `Lore_Text_FormerToolmakerDied_KnowledgePassedToOrin` → `Rock_Savana_005`, `Rock_Savana_006`

Todas as escritas usaram `actor.modify(True)` antes de `set_editor_property('tags', ...)`, conforme regra `hugo_terrain_savana_v4` (mutabilidade correta de package).

**Nota sobre grounding**: não movi nenhuma posição destes actores (só tags), pelo que a directiva PLAYABLE-FIRST de "grounded" não se aplica aqui — não houve `set_actor_location`. Tentei verificar o chão por line-trace ao Landscape em `NPC_Anchor_CraftsmanOrin_001`/`ElderSana`/`TrackerKael`, mas o trace foi bloqueado por props/triggers antes de chegar ao Landscape (não consegui isolar `Landscape1` nesse XY dentro do orçamento do ciclo). Reporto isto como limitação, não como facto resolvido — fica para o próximo ciclo com foco em física/trace se for considerado prioritário.

## Diálogo escrito para Orin (voice sample gerado)
Linha central do arco, gravada via ElevenLabs (`text_to_speech`, personagem `CraftsmanOrin`):

> "The old toolmaker who came before me... he didn't die from a beast. He died from carelessness, chipping flint too close to the fire, breathing the dust for years. Before he passed, he showed me how to read the grain of stone, how to find the strike point on a rock without shattering it into rubble. Everything I teach you now, the axe, the fire-starting, the container that holds water... it's his knowledge, not mine. I'm just the one who remembered it long enough to pass it on. Learn it well. Pass it on when your time comes, because none of us last forever out here."

**Nota técnica**: o upload do áudio para Supabase Storage falhou (`403 Invalid Compact JWS` — problema de token, não de conteúdo). O áudio foi gerado com sucesso (ElevenLabs devolveu MP3 válido em base64, ~39s), mas não ficou persistido em storage público. Reporto ao #16 (Audio Agent) para re-gerar/re-upload com token válido — o texto está pronto a usar.

## Para o #16 (Audio Agent)
1. Re-gerar a linha de voz do Orin acima (falha foi de storage token, não de texto/voz)
2. Ligar a VO à âncora `NPC_Anchor_CraftsmanOrin_001` (tag `Narrative_VoiceLine_Ready` já presente — falta o asset final)
3. Considerar um som ambiente de "descascar poeira/pedra" quando o jogador interage com `Rock_Savana_005`/`006` (reforça a lore sem UI)

## Ficheiros no GitHub
- `Docs/Narrative/Cycle_PROD_CYCLE_MANUAL_20260731_OrinLoreLink.md` (este ficheiro)

## Decisões técnicas e justificação
- Zero `.cpp`/`.h` criados (regra `hugo_absolute_prohibitions_v1`) — toda a lógica narrativa vive em tags de actores no mundo vivo
- Zero actores novos spawnados — vínculo feito por tags em actores existentes (regra `hugo_naming_dedup_v3`)
- Nenhuma posição alterada — fora do escopo "HANDS OFF" da directiva PLAYABLE-FIRST
