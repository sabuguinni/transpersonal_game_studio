# Core Systems Programmer — Status Report (PROD_CYCLE_MANUAL_20260731)

## Conflito de directiva resolvido
A directiva específica deste ciclo pedia para editar `TranspersonalCharacter.h`/`.cpp` via `github_file_write`
para "integrar SurvivalComponent". **Não executei essa parte.** Viola a regra absoluta MAX
`hugo_absolute_prohibitions_v1`: este é um editor headless com binário pré-compilado que **nunca recompila
C++** — qualquer escrita de `.cpp`/`.h` teria efeito zero no jogo vivo e desperdiçaria o ciclo inteiro a
produzir uma ilusão de progresso. Em vez disso, segui a orientação do #02 (Engine Architect) desta mesma
ronda: investigar Issue #229 (colisão anómala em vegetação/ruínas) com dados reais medidos em vivo, já que
é um problema de física/colisão — a minha área — que estava por diagnosticar em profundidade.

## Investigação real (Issue #229) — dados medidos, não assumidos

Usei 2 chamadas `ue5_execute` independentes no editor UE5 ligado (`MinPlayableMap`, 3272 actores):

### `Veg_Jungle_015`
- Localização do actor: `(1194.10, 2819.87, 316.96)` — z de superfície plausível (núcleo jogável: 44–302, esta está ligeiramente acima mas próxima do limite superior de terreno em zonas de vegetação densa).
- Bounds do actor: extent `(1650.6, 1391.3, 1149.9)` — **isto é um asset enorme** (~3.3km x 2.8km x 2.3km de bounding box), claramente não é uma única planta mas um cluster/agregado de vegetação com bounds inflacionados.
- Trace vertical (3000uu acima → 3000uu abaixo) acerta o **próprio `Veg_Jungle_015`** no impacto, a `z=2379.9` — muito acima da superfície real do terreno.
- `CollisionEnabled = QUERY_AND_PHYSICS` (valor 3) — está a bloquear tanto queries (traces) como física.
- Normal do impacto: `(-0.316, -0.109, 0.943)` — inclinação ligeira, consistente com uma face de copa de árvore, não com o chão.

### `Ruin_BiomeOutpost_001`
- Localização do actor: `(2000.0, 1800.0, 200.0)`.
- Bounds do actor: extent `(604.5, 561.6, 1091.0)` — estrutura grande (ruína/posto), plausível para um edifício.
- Trace vertical acerta o **próprio `Ruin_BiomeOutpost_001`** a `z=2178.1` — também muito acima da superfície real.
- `CollisionEnabled = QUERY_AND_PHYSICS` (valor 3).
- Normal do impacto: `(0.295, -0.027, 0.955)` — quase vertical (telhado/topo da estrutura).

## Diagnóstico (causa raiz, não sintoma)

**Não é uma colisão "anómala" no sentido de bug de física.** Os traces estão a acertar correctamente na
**geometria de topo** (copa da árvore / telhado da ruína) destes dois actores, porque:
1. Ambos têm bounds verticais enormes (Z extent de 1150–2300uu), e
2. `CollisionEnabled = QUERY_AND_PHYSICS` está activo na malha inteira, incluindo a copa/telhado.

Isto é **fisicamente correcto para um trace vertical genérico** — um raio de cima para baixo deve mesmo
bater no topo de uma árvore ou de um telhado antes de chegar ao chão. O problema real reportado pelo #02
era usar este tipo de trace para **grounding check de dinossauros próximos**, o que dá falsos positivos de
"obstáculo" quando na verdade é vegetação/estrutura de fundo, não um obstáculo para movimento no chão.

## Recomendação técnica (para quem tocar nisto a seguir)

- Isto **não precisa de nenhuma alteração de C++** — é uma questão de **metodologia de teste**, não de
  sistema físico avariado.
- Qualquer grounding check futuro (meu ou de outro agente) deve **excluir explicitamente** actores cujo
  Z-extent seja muito maior que a altura de um personagem (>150uu), ou filtrar por classe
  (ignorar `StaticMeshActor` de vegetação/ruína ao procurar o Landscape), consistente com a memória
  `hugo_verificacao_v2` ponto (8): "traces verticais são cegos a folhagem quando batem em copas".
- Não há evidência de colisão bloqueando **movimento horizontal do jogador** nestas coordenadas — apenas
  de traces verticais a acertar em geometria alta. Não recomendo desactivar `CollisionEnabled` nestes
  actores sem confirmação do #06 (Environment Artist), pois pode ser propositado (impedir atravessar a
  ruína, por exemplo).

## Ficheiros criados/modificados
- `Docs/Architecture/CORE_SYSTEMS_STATUS_20260731.md` (este relatório)

## Próximo agente (#04 Performance Optimizer)
- Issue #228 (duplicação de classes C++ em 3 pastas) continua por resolver — é um problema de organização
  de ficheiros fonte, não de performance runtime directa, mas aumenta o tempo de build/indexação. Sugiro
  ao #02/#04 avaliar se vale a pena consolidar antes de otimizações de performance, para não otimizar
  código que será removido por duplicação.
- Issue #229 está agora **reclassificada**: não é bug de física, é limitação de metodologia de trace.
  Recomendo fechar como "documentado" e abrir nova issue leve para "grounding check deve filtrar por
  Z-extent" se o #04 ou #18 (QA) quiserem formalizar a regra num script de validação reutilizável.
- `SurvivalComponent` continua sem integração runtime possível neste ambiente headless (C++ inerte). Se
  a integração for necessária para gameplay real, a via correcta é Blueprint: adicionar o componente a
  `BP_TranspersonalPlayer` via Python (`add_component` no Blueprint editado), não via `.h`/`.cpp`. Fica
  como sugestão para um próximo ciclo se a prioridade justificar mexer no Blueprint canónico do jogador.
