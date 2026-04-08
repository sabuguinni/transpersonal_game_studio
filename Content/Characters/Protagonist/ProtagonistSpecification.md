# PROTAGONIST SPECIFICATION
## Dr. [Nome] - Paleontologista Transportado no Tempo
## Character Artist Agent #09 — Transpersonal Game Studio

---

## BIOGRAFIA VISUAL

### IDENTIDADE BÁSICA
- **Nome**: Dr. [A definir pelo Miguel]
- **Idade**: 37 anos
- **Profissão**: Paleontologista de campo
- **Origem**: Universidade/Instituto de pesquisa
- **Situação**: Transportado para o período Jurássico/Cretáceo

### FÍSICO E CONSTITUIÇÃO
- **Altura**: 1.75m (constituição média)
- **Peso**: 75kg (físico de quem trabalha no campo)
- **Tipo Corporal**: Mesomorfo — nem atlético nem sedentário
- **Postura**: Ereta mas alerta, cabeça ligeiramente inclinada (observador)

### ROSTO E EXPRESSÃO
- **Formato do Rosto**: Oval, traços definidos mas não duros
- **Olhos**: Castanhos escuros, expressivos, com rugas de concentração
- **Sobrancelhas**: Espessas, ligeiramente franzidas (pensativo)
- **Nariz**: Proporcional, com pequena marca de óculos
- **Boca**: Lábios médios, canto ligeiramente descaído (preocupação)
- **Barba**: Por fazer há 3-4 dias (situação de sobrevivência)

### CABELO
- **Cor**: Castanho escuro com fios grisalhos nas têmporas
- **Estilo**: Curto, ligeiramente despenteado
- **Textura**: Ligeiramente ondulado, com marcas de suor e sujidade

### PELE E MARCAS
- **Tom**: Mediterrânico claro, ligeiramente bronzeado
- **Textura**: Pele de quem trabalha ao ar livre
- **Cicatrizes**: Pequena cicatriz na testa (acidente de campo)
- **Mãos**: Calejadas, unhas curtas, pequenos cortes recentes
- **Sinais de Desgaste**: Olheiras ligeiras, pele ligeiramente desidratada

---

## VESTUÁRIO E EQUIPAMENTO

### ROUPA PRINCIPAL
**Camisa de Campo**
- Material: Algodão resistente, cor caqui
- Estado: Ligeiramente rasgada no cotovelo direito
- Detalhes: Bolsos no peito, mangas arregaçadas
- Manchas: Terra, suor, pequenas manchas de sangue

**Calças de Expedição**
- Material: Canvas resistente, cor castanha
- Estado: Joelheiras desgastadas, pequenos rasgões
- Detalhes: Bolsos laterais, cinto de couro simples
- Sujidade: Lama seca, manchas de vegetação

**Calçado**
- Tipo: Botas de trekking de couro
- Estado: Desgastadas mas funcionais
- Detalhes: Atacadores reparados, sola com marcas de uso
- Sujidade: Lama, riscos, pequenos buracos

### ACESSÓRIOS SOBREVIVÊNCIA
**Mochila Improvisada**
- Material: Lona e cordas encontradas
- Conteúdo: Ferramentas básicas, amostras, água
- Estado: Reparos visíveis, desgaste evidente

**Ferramentas Visíveis**
- Machado de pedra (feito pelo próprio)
- Faca improvisada (metal encontrado)
- Corda de fibras vegetais

### SINAIS DE ADAPTAÇÃO
- Roupas adaptadas com materiais do ambiente
- Reparos improvisados com fibras vegetais
- Acessórios feitos com recursos locais
- Sinais de exposição aos elementos

---

## PERSONALIDADE ATRAVÉS DA APARÊNCIA

### POSTURA CORPORAL
- **Em Repouso**: Ligeiramente curvado, observando o ambiente
- **Em Movimento**: Passos cautelosos, cabeça em movimento constante
- **Interação**: Mantém distância segura, mãos visíveis
- **Stress**: Tensão nos ombros, mandíbula cerrada

### EXPRESSÕES FACIAIS
- **Padrão**: Concentração intensa, sobrancelhas ligeiramente franzidas
- **Alerta**: Olhos arregalados, cabeça inclinada para escutar
- **Descoberta**: Curiosidade científica misturada com cautela
- **Medo**: Controlado mas visível, respiração alterada

### GESTOS CARACTERÍSTICOS
- Ajustar óculos inexistentes (hábito do passado)
- Esfregar a barba por fazer quando pensa
- Verificar constantemente a mochila e ferramentas
- Olhar para o céu para orientação temporal

---

## ESPECIFICAÇÕES TÉCNICAS METAHUMAN

### PRESET BASE
- **MetaHuman Template**: Masculino, Europeu/Mediterrânico
- **Idade**: 35-40 anos
- **Tipo Facial**: Oval, traços definidos

### CUSTOMIZAÇÕES ESPECÍFICAS
```json
{
  "face_shape": {
    "jaw_width": 0.6,
    "cheek_prominence": 0.4,
    "forehead_height": 0.7
  },
  "eyes": {
    "color": "dark_brown",
    "size": 0.6,
    "depth": 0.7,
    "bags": 0.3
  },
  "nose": {
    "width": 0.5,
    "length": 0.6,
    "bridge_height": 0.6
  },
  "mouth": {
    "width": 0.5,
    "lip_thickness": 0.4,
    "corner_position": -0.1
  },
  "hair": {
    "style": "short_messy",
    "color": "dark_brown_grey",
    "density": 0.8
  },
  "facial_hair": {
    "style": "stubble_3day",
    "color": "dark_brown_grey",
    "density": 0.7
  },
  "skin": {
    "tone": "mediterranean_light",
    "roughness": 0.6,
    "tan": 0.3,
    "dirt": 0.4
  }
}
```

### MATERIAIS NECESSÁRIOS
- **Skin_Weathered**: Pele com sinais de exposição
- **Hair_Unkempt**: Cabelo despenteado e sujo
- **Clothing_Survival**: Roupas adaptadas e desgastadas
- **Accessories_Improvised**: Ferramentas e equipamento improvisado

### LOD CONFIGURATION
- **LOD0**: Full detail para cutscenes e close-ups
- **LOD1**: Medium detail para gameplay normal
- **LOD2**: Low detail para distância

---

## VARIAÇÕES CONTEXTUAIS

### ESTADO INICIAL (Recém-chegado)
- Roupas menos desgastadas
- Menos sujidade e adaptações
- Expressão mais confusa que determinada

### ESTADO MÉDIO (Adaptado)
- Roupas com reparos visíveis
- Ferramentas improvisadas
- Postura mais confiante

### ESTADO AVANÇADO (Sobrevivente Experiente)
- Equipamento totalmente adaptado
- Múltiplas cicatrizes e marcas
- Postura de quem conhece os perigos

---

## DIRETRIZES DE IMPLEMENTAÇÃO

### PRIORIDADES
1. **Realismo**: Deve parecer uma pessoa real em situação real
2. **Contextualização**: Cada detalhe deve fazer sentido na narrativa
3. **Expressividade**: Capaz de transmitir emoção sem diálogo
4. **Versatilidade**: Funcional em diferentes situações de jogo

### VALIDAÇÃO
- Teste em diferentes condições de iluminação
- Verificação de expressões faciais
- Teste de movimento e animação
- Validação com style guide visual

---

## ENTREGÁVEIS

1. **MetaHuman Asset**: Modelo 3D completo
2. **Material Library**: Todos os materiais necessários
3. **Blueprint Character**: Sistema técnico funcional
4. **Animation Targets**: Preparação para animação
5. **Documentation**: Guia para outros agentes

---

*"O Dr. [Nome] não é apenas um modelo 3D — é um homem de ciência perdido no tempo, e cada detalhe da sua aparência conta essa história."*

**Character Artist Agent #09**  
**Transpersonal Game Studio — Março 2026**