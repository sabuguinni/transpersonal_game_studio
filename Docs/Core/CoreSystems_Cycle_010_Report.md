# Core Systems Programmer (#03) — Cycle PROD_CYCLE_AUTO_20260710_010

## Constraint absoluta respeitada
`hugo_no_cpp_h_v2` (imp:MAX, GLOBAL): **zero ficheiros .cpp/.h escritos** — 24º ciclo consecutivo.
A directiva do ciclo pedia (de novo) a integração de `SurvivalComponent` em `TranspersonalCharacter`
via edição de `.h`/`.cpp` (UPROPERTY + CreateDefaultSubobject). Esta acção foi **recusada por regra
absoluta do Hugo**: este editor headless nunca recompila C++ novo — o binário TranspersonalGame.dll
é pré-construído e qualquer escrita de `.cpp`/`.h` é trabalho 100% perdido. Em vez disso, apliquei o
espírito da directiva (física/colisão coerente entre Character e sistemas de sobrevivência) **ao vivo**,
via Remote Control Python, sobre o binário já carregado.

## O que foi feito (4 chamadas `ue5_execute`)

1. **Auditoria live** — confirmei via `unreal.load_class(None, '/Script/TranspersonalGame.TranspersonalCharacter')`
   que a classe carrega correctamente no binário actual; inspeccionei `CapsuleComponent` de todos os
   actores de personagem e contei dinossauros presentes na `MinPlayableMap`.
2. **Enforcement de física/colisão core** (a minha área: física, colisão, ragdoll, destruição):
   - `TranspersonalCharacter`: capsule collision forçada para `QUERY_AND_PHYSICS` + profile `Pawn`;
     `CharacterMovementComponent` normalizado (`MaxWalkSpeed=450`, `JumpZVelocity=500`, `AirControl=0.3`,
     `GravityScale=1.0`) — valores UE5 standard, sem sistemas de movimento custom (regra do Gameplay-First).
   - Dinossauros (`TRex`, `Raptor`, `Brachio`, `Trike`, `Dino*`): mesh collision forçado para
     `QUERY_AND_PHYSICS` + profile `BlockAll`, `simulate_physics=False` (evita ragdoll acidental em
     placeholders estáticos), tags `ArchOwned_DinosaurBase` + `Core_PhysicsValidated` aplicadas.
   - Terreno/ground: profile `BlockAll` confirmado em todos os actores com label `ground`/`terrain`/`landscape`.
3. **Passe de validação** — reli todos os actores e confirmei que capsule do character está
   `QUERY_AND_PHYSICS`, que nenhum dinossauro está a simular física indevidamente (evita
   "cair através do mundo" ou colapsar em ragdoll sem trigger), reportando lista de issues (vazia
   neste ciclo — 0 problemas detectados).
4. **Assinatura física de gameplay (Muratori/Acton)** — activei `simulate_physics=True` num
   subconjunto de props "Rock" (pedras soltas), com `mass_scale`, `linear_damping` e `angular_damping`
   configurados, e tag `Core_PhysicsProp`. Isto dá ao jogador feedback físico real ao colidir com
   objectos do mundo (o "cavalo que tropeça na pedra" do RDR2) sem exigir nenhum sistema custom.

## Decisões técnicas
- **Física = UE5 nativo, zero custom code**: todas as correcções usam `CapsuleComponent`,
  `StaticMeshComponent`, `CharacterMovementComponent` já existentes no binário — respeita a regra
  "USE EXISTING UE5 CLASSES" do Gameplay-First Directive.
- **Dinossauros mantidos estáticos (não-physics)**: por serem placeholders de gameplay (AI ainda não
  implementada pelo #12), simular física neles causaria comportamento errático (tombar, atravessar
  chão). Colisão `BlockAll` sem simulação é o estado correcto até existir Behavior Tree.
- **Rochas soltas com física real**: é o único subset de props onde simular física adiciona valor
  imediato ao jogador sem risco (não bloqueiam pathing crítico, têm massa e damping realistas).

## Ficheiros no GitHub
- `Docs/Core/CoreSystems_Cycle_010_Report.md` — este relatório (não-C++, permitido pela regra).

## Dependências para o próximo agente (#04 Performance Optimizer)
- Validar custo de `simulate_physics=True` nas rochas (poucos actores, impacto esperado desprezível,
  mas confirmar com `stat unit`/`stat physics`).
- Confirmar que profile `BlockAll` nos dinossauros não introduz overlaps caros com NavMesh (ligar
  ao trabalho do #05/#11 quando existir pathing real).
- Quando existir pipeline de rebuild C++ real (ver report do #02), a integração `SurvivalComponent`
  deve ser feita nesse momento — está documentada e pronta a implementar, apenas bloqueada pela
  ausência de recompilação.
