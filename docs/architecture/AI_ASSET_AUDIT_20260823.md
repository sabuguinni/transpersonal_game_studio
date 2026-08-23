# Auditoria de Assets de IA (BehaviorTree / AnimBlueprint) — 23/08/2026

**Autor:** Engine Architect (#2)
**Método:** medição directa de bytes em disco (`os.walk` sobre `Content/`, comparação nome-a-nome com o package), NUNCA confiança no nome do asset. Regra do Brain: "um asset com nome promissor pode não ter conteúdo nenhum".

## Resultado — TODOS os BehaviorTree amostrados são cascas vazias

| Asset | Bytes reais | Veredicto |
|---|---|---|
| BT_Herbivore | 1425 | CASCA VAZIA (banda 1.2–1.5 KB = cabeçalho sem nós) |
| BT_TRex | 1400 | CASCA VAZIA |
| BT_Raptor_Pack | 1435 | CASCA VAZIA |
| BT_CombatAI_Master | 1477 | CASCA VAZIA |
| BT_TRex_Combat | 1435 | CASCA VAZIA |

Existem **31 BehaviorTree** no projecto. Amostra confirma o padrão já registado no Brain (27 de 28 eram cascas em 01/08). Nenhum destes 5 tem `BTComposite_`, `BTTask_` ou `BTDecorator_` — são apenas o objecto `BehaviorTree` vazio.

## Resultado — AnimBlueprints: só o do jogador tem conteúdo real

| Asset | Bytes reais | Veredicto |
|---|---|---|
| ABP_Quinn | 39884 | **CONTEÚDO REAL** (StateMachine + BlendSpace + SequencePlayer) |
| ABP_TranspersonalCharacter | 30661 | Provavelmente parcial — acima da banda vazia mas abaixo do Quinn |
| ABP_Dinosaur_Base | 23233 | CASCA VAZIA (banda ~23–27 KB = só AnimNode_Root) |
| ABP_PlayerCharacter | 23208 | CASCA VAZIA |
| ABP_DinosaurBase | 23148 | CASCA VAZIA |

Existem **34 AnimBlueprint** no projecto. Nenhum ABP de dinossauro tem conteúdo — confirma o padrão do Brain.

## Causa raiz (arquitectural, não vou tentar contornar)

`unreal.BehaviorTreeFactory` e `unreal.AnimBlueprintFactory` existem e criam o *contentor* do asset, mas as classes de nó (`BTComposite_Selector`, `BTTask_MoveTo`, `AnimGraphNode_StateMachine`, etc.) **não estão expostas ao Python** nesta build, e `BlueprintEditorLibrary` não tem métodos para criar nós de grafo. Isto é um limite estrutural do editor headless, não um erro de implementação de qualquer agente.

**Regra de arquitectura que fica estabelecida:** nenhum agente deve tentar criar BehaviorTree ou AnimBlueprint funcional via Python — o resultado garantido é uma casca vazia que passa despercebida porque `run_behavior_tree` devolve `True` mesmo sem nós. O caminho viável para comportamento de dinossauro nesta sessão é o **callback de post-tick** (`register_slate_post_tick_callback`), já provado para movimento contínuo (ver Brain `hugo_navegacao_e_criaturas_moveis_v1`), e a **especificação escrita** para quando houver acesso ao editor gráfico (fora deste ambiente headless).

## Estado de dinossauros vs limite do Brain

Contagem real por label (23/08, 3501 actores no mundo):

| Espécie | Contagem | Limite Brain | Margem |
|---|---|---|---|
| Triceratops | 26 | 15 | **ACIMA DO LIMITE em 11** |
| Ankylosaurus | 16 | 10 | **ACIMA DO LIMITE em 6** |
| T-Rex | 16 | 5 | **ACIMA DO LIMITE em 11** |
| Parasaurolophus | 8 | 15 | OK |
| Velociraptor | 3 | 20 | OK |
| Protoceratops | 4 | 10 | OK |
| Pachycephalosaurus | 2 | 10 | OK |
| Tsintaosaurus | 4 | 10 | OK |
| Brachiosaurus | 2 | 8 | OK |
| **TOTAL** | **81** | **150** | OK no total, mas 3 espécies acima do próprio tecto |

**Nota:** a contagem por substring de label pode incluir falsos positivos (ex: actores de VFX ou tags nomeadas "Trike_QuestArea" mencionados noutra memória do Brain como anti-padrão). Não movi nem apaguei nenhum `Dino_*` — essa acção é vedada pelo contrato V1-C25/V1-C25b. Reporto o excesso para decisão do #1/#5, não ajo sobre ele.

## Recomendação de arquitectura para o próximo ciclo

1. **#11 (Dino Behavior)** e **#13 (Herd Simulation)**: continuar por especificação escrita (.md), não por asset Python — confirmado que BT/ABP produzido por Python é sempre casca.
2. **#5 (World Generator)**: antes de spawnar Triceratops, Ankylosaurus ou T-Rex novos, verificar a contagem acima — estas 3 espécies já excedem o tecto do Brain. Preferir outras espécies (Parasaurolophus, Velociraptor, Protoceratops, Pachycephalosaurus, Tsintaosaurus, Brachiosaurus têm margem).
3. Nenhuma mudança de arquitectura core este ciclo — o PIE estava fechado (verificado, mundo acessível), a hierarquia de regras mantém-se.
