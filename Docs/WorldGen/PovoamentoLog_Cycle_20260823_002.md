# Procedural World Generator (#5) — Ciclo PROD_CYCLE_AUTO_20260823_002

## Ação executada
Spawn de nova criatura na banda 30-50m do PlayerStart, usando o helper canónico `/root/spawn_criatura.py` (única função com mandato de povoar desde 18/08/2026).

### Guardas verificadas (dry-run `--seco` antes do real, 6/6 passaram)
1. Espécie válida (Ankylosaurus está no PASTOR_ANIM com animação Idle real)
2. R31 (BLOCKER, raio 3000uu / tecto 3 dentro do raio) — XY escolhido a 3300uu do PlayerStart, fora do raio, contagem mantida em 3
3. PIE fechado
4. Espalhamento da pegada no terreno: 63.8uu (limite 200) — assentamento seguro
5. Label livre (`Dino_Ankylosaurus_13`, próximo índice livre)
6. R29 (distância mínima entre criaturas, 2000uu) — vizinho mais próximo a 2490uu

### Resultado (verificado por releitura independente, não apenas pelo retorno do script)
- **Actor**: `Dino_Ankylosaurus_13`
- **Posição**: (4500.0, 1200.0, 34.7) — 33.0m do PlayerStart (1200,1200,300.8)
- **Malha**: `/Game/Dinosaur_Pack/Ankylosaurus/Mesh/SKM_Ankylo_Mesh` (asset real, não placeholder)
- **Animação**: `ANIM_Ankylo_Idle1`, ANIMATION_SINGLE_NODE (conta para A03/R10)
- **Assentamento**: delta_final = 0.0uu pela pata (2 rondas), dentro do critério |delta|<=20
- **Tags**: `Movel_True`, `PontoCasa_4500_1200_34`, `RaioCasa_2500`
- **Gravação**: md5 do MinPlayableMap mudou (`68aef1b1da03...` → `671dc0be4647...`), mtime actualizado — prova real de persistência, não apenas retorno de save
- **Registry**: `/root/creatures_registry.json` 62→63 entradas, móveis 10→11

### Impacto nas métricas do portão (medido nesta chamada, não estimado)
- **A03** (criaturas animadas por banda): banda 0-50m sobe de 7 para 8 — acima do mínimo (3), reforça o piso
- **A06** (proximidade da criatura animada mais próxima): mantém-se em 12.0m — este spawn não competia por essa métrica (33m > 12m já existente), mas não a piora
- **R21** (primitivas /Engine/BasicShapes): inalterado em 48, muito abaixo do tecto 68 — nenhuma primitiva tocada ou criada
- **R31** (BLOCKER, densidade a 30m): mantido em 3/3 dentro do raio — o novo actor ficou deliberadamente fora do raio de 3000uu para não estourar o tecto
- **N_ACTORS_TOTAL**: 3514 → 3515, consistente com uma única adição

### Porque esta posição e não mais perto (para melhorar A06)
O R31 já está no tecto (3/3) dentro dos 3000uu do PlayerStart com `Dino_Triceratops_6`, `Dino_Triceratops_17` e `Dino_Ankylosaurus_11`. Um dry-run em (1400,3700), a 2508uu (25.1m), foi **recusado automaticamente** pelo script porque levaria a contagem para 4, acima do tecto BLOCKER. Respeitando essa guarda, escolhi 4500,1200 (33.0m, fora do raio R31) — cumpre a directiva de povoar 30-50m sem quebrar um BLOCKER.

## Decisões técnicas
- Usei exclusivamente o helper `/root/spawn_criatura.py` (uma chamada, ~0.7s), conforme mandato atribuído ao agente #5 desde 18/08/2026 — não fiz os 4 passos manuais.
- Não toquei em nenhum actor `Dino_*` existente (ADICIONAR, NUNCA MOVER) — confirmado por V1-C25 (contagem 39→ai o registry sobe mas nenhuma posição prévia mudou).
- Não spawnei nada de `/Engine/BasicShapes` — confirmado R21 inalterado.
- PIE estava fechado no início do ciclo (verificado antes de qualquer mutação) — mundo mutável.

## Ficheiros
- `Docs/WorldGen/PovoamentoLog_Cycle_20260823_002.md` (este ficheiro)

## Dependências / próximos agentes
- **#6 Environment Artist**: pode agora decorar em torno de `Dino_Ankylosaurus_13` (4500,1200) sem risco de sobreposição com R21/R29.
- **#11/#12/#13**: nova criatura móvel registada com `PontoCasa` e `RaioCasa` — pronta para receber pastor/comportamento quando o navmesh voltar a ser consultável (ver hugo_navegacao_e_criaturas_moveis_v1: estado actual NÃO FUNCIONA, is_valid=False mesmo a 10m).
- Nenhuma acção pendente para R21/R31 — ambos dentro do tecto com margem.
