# Performance Optimizer #04 — Cycle PROD_CYCLE_AUTO_20260711_004

## Constraint absoluta respeitada
`hugo_no_cpp_h_v2` (imp:MAX): **zero ficheiros .cpp/.h escritos**. Toda a optimização foi aplicada ao vivo no editor via `ue5_execute` (Python + console commands), sem qualquer tentativa de recompilar C++ neste binário pré-construído.

## Contexto
Ciclo anterior (#03 Core Systems) reposicionou dinossauros e vegetação existentes para dentro do raio do hub de conteúdo (X=2100, Y=2400, raio 1500) para satisfazer `hugo_hub_quality_v2_fix`, sem criar novos actors. O meu trabalho neste ciclo foi garantir que essa densidade acrescida no hub **não parte o frame budget** (60fps PC / 30fps consola).

## Ações Executadas (3x ue5_execute python + 2x run_console_command)

1. **Pass 1 — Auditoria de performance (Python)**
   - Censo total de actors no nível + contagem de actors com tick activo.
   - Contagem de actors dentro do raio do hub (2100, 2400, r=1500) e breakdown por classe.
   - Resultado escrito em `/tmp/ue5_result_perf04.txt` para verificação.

2. **`stat unit`** — activado overlay de frame time (Game/Draw/GPU thread) para profiling contínuo da cena do hub.

3. **Pass 2 — Optimization pass (Python)**
   - Para todos os actors com `StaticMeshComponent` dentro do raio do hub: aplicado `cull_distance = 6000.0` para limitar draw calls de meshes distantes da câmara do jogador quando este se afasta do hub.
   - Desactivado `tick` (`set_actor_tick_enabled(False)`) em actors estáticos (árvores, rochas, foliage) cujo label contém "Tree", "Rock" ou cuja classe contém "Foliage" — estes são props estáticos sem lógica de gameplay, pelo que tick é desperdício de CPU por frame.

4. **`stat gpu`** — activado overlay de custo GPU para cross-check contra o custo de CPU/game thread já exposto por `stat unit`.

5. **Pass 3 — Verificação pós-optimização (Python)**
   - Recontagem de meshes no hub, confirmação de que os actors Tree/Rock têm tick desactivado, e **save do nível** (`unreal.EditorLevelLibrary.save_current_level()`) para persistir as alterações.
   - Resultado escrito em `/tmp/ue5_result_perf04_verify.txt`.

## Decisões Técnicas
- **Cull distance de 6000uu** escolhido como valor conservador que mantém os meshes do hub visíveis no screenshot hero (framing a X=2100,Y=2400) mas evita render de meshes fora do alcance útil da câmara em zonas periféricas do mundo.
- **Tick desactivado apenas em props estáticos** (árvores/rochas/foliage) — dinossauros e o character do jogador mantêm tick activo, pois precisam de lógica de movimento/AI/input.
- Não foram criados novos actors nem duplicados — respeitando `hugo_naming_dedup_v2`. Toda a optimização foi aplicada aos actors já existentes reposicionados pelo #03 Core Systems.
- Câmara do viewport **não foi tocada** em nenhum momento (`hugo_no_camera_v2` respeitado).

## Dependências para o Próximo Ciclo
- **#05 Procedural World Generator**: o hub agora tem densidade acrescida e optimizada — qualquer nova geração procedural de terreno/bioma deve considerar este raio (2100,2400,r=1500) como zona de alta prioridade visual e manter os cull distances já aplicados.
- **Hugo**: os overlays `stat unit` e `stat gpu` ficaram activos no editor para profiling contínuo — podem ser desligados com `stat unit` / `stat gpu` novamente se não forem necessários no próximo screenshot hero.
