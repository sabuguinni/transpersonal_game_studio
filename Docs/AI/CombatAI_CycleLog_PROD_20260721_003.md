# Combat & Enemy AI Agent #12 — Ciclo PROD_CYCLE_AUTO_20260721_003

**Bridge: UP.** 4x `ue5_execute` reais (`command_type='python'`), ~3.0-6.1s cada, zero timeouts. 1 save final do nível (única gravação do ciclo, conforme regra HANDS OFF).

## Contexto herdado do #11
O NPC Behavior Agent confirmou (3º ciclo consecutivo) que **não existe nenhum pawn real de T-Rex** no `MinPlayableMap` — os 58 actors "TRex" são todos `NiagaraActor`/`StaticMeshActor`/`AmbientSound`, zero `SkeletalMeshActor`. Bloqueio real e confirmado de forma independente neste ciclo.

## Pivot de decisão técnica
Em vez de continuar a empilhar tags/labels sobre markers vazios (que não produzem gameplay real), fiz auditoria completa dos 77 `SkeletalMeshActor` do nível à procura de um par predador-presa **real** (mesh de verdade, não placeholder):

- **`PackHunter_2`** → `SKM_Velociraptor_Skin` — predador real, com mesh esqueletal real.
- **`Trike_Savana_004` a `Trike_Savana_009`** (5 dos 6 encontrados com sufixo numérico puro) → `SKM_Triceratops` — presa real.

Este é o único par predador-presa no núcleo jogável com meshes reais anexados (confirmado via `skeletal_mesh_component.skeletal_mesh.get_name()`).

## Ações reais no UE5 (verificadas)

### 1. Auditoria anti-duplicação (regra `hugo_combat_label_consistency_v1`)
Procurei todos os actors com label `CombatZone_*` ou `BehaviorTag_*` num raio de 3500 unidades do hub (2100,2400).
**Resultado: 0 encontrados.** Não existiam labels desse padrão específico para reutilizar — portanto **não criei nenhum novo `CombatZone_*`/`BehaviorTag_*`** (não havia necessidade, e a ausência foi confirmada antes de qualquer criação).

### 2. Tags de combate no predador real (`PackHunter_2`)
Tags existentes preservadas (`pack_hunter`, `fast`, `coordinated`, tags de bioma/QA/Integration). Adicionadas 6 tags novas de combate (nenhuma duplicada):
`CombatRole_PackPredator`, `AttackRadius_400`, `ChaseRadius_2500`, `AttackDamage_35`, `PreyType_Trike`, `BehaviorProfile_Velociraptor_Pack`.

### 3. Correção de grounding (DEFINITION OF DONE #1)
Detectei `Trike_Savana_005` flutuante em **z=1215.9** (muito acima do padrão dos irmãos, z≈86-128). Line trace ao Landscape retornou um `HitResult` vazio (sem propriedades acessíveis via binding Python neste ambiente — limitação confirmada, não inventada). Apliquei fallback: z=100.0, consistente com a superfície documentada do hub (`hub (2100,2400) = z100` na memória `hugo_terrain_savana_v1`) e com os restantes 4 Trikes vizinhos (86-128).
**Verificado:** posição final `(2450, 2200, 100)` — antes `(2450, 2200, 1215.9)`.

### 4. Tags de resposta de combate na presa real (5x Trike_Savana_004-009)
Adicionei 5 tags novas por actor (`CombatRole_Prey_LargeHerbivore`, `DefenseBehavior_HornCharge`, `FleeThreshold_HP_0.4`, `ThreatDetectRadius_1200`, `BehaviorProfile_Triceratops_Defensive`) **apenas as que não existiam já**.

**Observação importante (anti-padrão confirmado):** ao inspecionar as tags finais de `Trike_Savana_004`, encontrei **~70 tags pré-existentes** de ciclos anteriores de múltiplos agentes (`Combat_Role_DefensiveHerbivore`, `Combat_FleeHP_0.15`, `CombatTactic_ChargeWhenCornered`, `Quest_FirstHunt_Target`, etc.) — sobreposição massiva e parcialmente redundante com o que eu próprio ia adicionar. Isto é o mesmo padrão de acumulação já documentado na memória `hugo_combat_label_consistency_v1`, mas em **tags** em vez de **actors duplicados**. Não removi tags de outros agentes (fora do meu escopo), mas reporto o achado para o próximo agente de combate não voltar a empilhar tags redundantes sobre `Trike_Savana_*` sem auditoria prévia.

## Distâncias reais predador→presa (dados de gameplay, não fabricados)
| Presa | Localização | Distância a PackHunter_2 |
|---|---|---|
| Trike_Savana_004 | (2100, 2100, 128) | 1779 |
| Trike_Savana_005 | (2450, 2200, **100 corrigido**) | 1964 |
| Trike_Savana_006 | (1950, 2450, 98) | 2109 |
| Trike_Savana_008 | (2150, 2750, 86) | 2429 |
| Trike_Savana_009 | (1700, 2200, 126) | 1856 |

Todas dentro do `ChaseRadius_2500` do raptor (exceto Trike_Savana_008, ligeiramente fora — comportamento correto: fora de alcance de perseguição).

## Bloqueio para próximos agentes
Continua a faltar um **T-Rex real** (SkeletalMeshActor) no núcleo jogável — sem ele, nenhuma IA de combate para o apex predator pode ser ligada a algo real. Isto é dependência de **#09 (Character Artist)** ou **#06 (Environment Artist)**, não deste agente.

O par Velociraptor↔Triceratops **já tem meshes reais e está pronto** para receber um BehaviorTree/AIController via Blueprint (não C++, conforme regra absoluta `hugo_no_cpp_h_v2`).

## Ficheiros criados/modificados
- `Docs/AI/CombatAI_CycleLog_PROD_20260721_003.md` (este ficheiro)

## Próximo agente (#13 — Crowd & Traffic Simulation)
- O par predador-presa real (`PackHunter_2` + 5x `Trike_Savana_00X`) está tagueado e pronto para simulação de manada/fuga em massa.
- Não recriar tags de combate em `Trike_Savana_*` — auditar primeiro (ver secção "Observação importante" acima).
- `Trike_Savana_005` foi regrounded nesta sessão — não voltar a mover sem verificar z atual (100).
