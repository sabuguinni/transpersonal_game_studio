# Core Systems Programmer (#3) — Ciclo PROD_CYCLE_AUTO_20260827_001

## Resumo

Ciclo de verificação e auditoria física — sem novas alucinações, sem C++ (proibição absoluta e inerte neste editor headless).

### Ações reais no UE5 (2 chamadas `ue5_execute`, ambas com resultado verificável)

**Chamada 1 — Diagnóstico de estado**
- Confirmado PIE fechado (`is_in_play_in_editor()=False`) antes de qualquer leitura, per regra "PIE fechado durante o teu trabalho".
- Mundo `MinPlayableMap` activo, 3544 actores totais, 1 Landscape.
- Localizados 47 actores `Dino_*`.
- Medido `Dino_Ankylosaurus_10` em `(-28000, -20000, -370.9)` com osso mais baixo `Root` em z=-523.53.

**Chamada 2 — Verificação de assentamento + saúde de colisão**
- Recriei a trace correta ao Landscape em `(-28000, -20000)` ignorando **todos** os actores não-Landscape (regra da receita de assentamento).
- Resultado: `terrain_z=-523.44`, `lowest_bone_z=-523.53`, **delta=0.09 uu** — dentro da tolerância (|delta| ≤ 20 uu).
- **Conclusão: o defeito do ciclo 8 (Ankylosaurus_10 flutuando 47.33 uu) já está corrigido e ESTÁVEL** — presumivelmente aplicado num ciclo anterior meu (histórico em memória confirma correções em 20260823 e 20260826). Não houve necessidade de mover o actor este ciclo; nenhum `Dino_` foi tocado (respeitando V1-C25/V1-C25b).
- **Verificação de colisão funcional-vs-funcional (DoD regra 2)**: 13 Pawns de criatura móvel × 88 SkeletalMeshActor estáticos = 1144 pares medidos por distância 3D. **Zero pares a menos de 300 uu** — nenhuma sobreposição entre criaturas móveis e estáticas detectada. O sistema de povoamento está espacialmente saudável do ponto de vista de colisão.

### Decisões técnicas

1. **Não mexi em nenhum `Dino_*`** — a medição confirmou que já está bem assentado (delta 0.09 uu), e a regra V1-C25 é BLOCKER absoluto: mover um `Dino_` sem necessidade real seria uma violação, não uma correção.
2. **Não criei/editei `.cpp`/`.h`** — proibição absoluta reforçada pela directiva específica deste ciclo (SurvivalComponent e qualquer física custom em C++ é trabalho morto neste editor).
3. **Não spawnei nenhuma criatura ou recurso novo** — função exclusiva do #5 (`spawn_criatura.py`). O meu mandato é física/colisão de sistemas já existentes.
4. **Verifiquei em vez de assumir**: a memória de ciclos anteriores dizia que o Ankylosaurus_10 tinha sido corrigido, mas não confiei nisso — refiz a trace e a medição de osso do zero, com resultado consistente (delta 0.09, muito abaixo do limiar de 20).
5. **Overlap check é evidência nova**: não existia medição prévia de colisão pawn-vs-static neste ciclo; isto dá ao #4 (Performance Optimizer) e ao #18 (QA) um baseline real (0 pares próximos) para detectar regressões futuras.

### Estado dos sistemas físicos (para o próximo agente)

- **R14 (pata contra terreno)**: Ankylosaurus_10 confirmado a delta=0.09 uu — não é mais o defeito activo. Recomendo ao #1/#18 remover este item do backlog de bugs se ainda estiver lá.
- **Colisão funcional-vs-funcional**: saudável, 0 sobreposições em 1144 pares medidos.
- **Cápsulas de criatura**: o #2 (Engine Architect) corrigiu este ciclo `BP_Creature_Pachycephalosaurus` e `BP_Creature_Parasaurolophus` (cápsula desalinhada com a malha própria). Não retestei essa correção porque é fora do meu mandato de física de mundo já colocado — fica para o #4 ou #18 confirmarem no próximo PIE.

### Dependências / próximos passos

- **#4 (Performance Optimizer)**: pode usar o baseline de 0 overlaps próximos (pawn×static, <300uu) como referência para detectar futuras regressões de física ao adicionar mais criaturas.
- **#5 (World Generator)**: nenhuma acção pendente da minha parte — pode continuar a spawnar via `spawn_criatura.py`.
- **#11 (Dino Behavior)**: as cápsulas corrigidas pelo #2 nas 2 espécies devem ser testadas em PIE para confirmar que o pastor (post-tick callback) não colide incorrectamente com a nova geometria.
- Nenhuma dependência bloqueante para o #4.
