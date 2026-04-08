# MetaHuman Character Blueprint Documentation

## Blueprint Class: BP_MetaHumanCharacter
**Parent Class:** Character  
**Description:** Base character blueprint for all MetaHuman characters in the Jurassic survival game

This blueprint serves as the foundation for all human characters in the Jurassic survival game. It integrates with the MetaHuman Creator system and provides comprehensive character variation and survival mechanics.

## Core Features

### 1. Character Variation System
- Procedural generation of unique individuals
- Ethnicity and age-appropriate features  
- Survival wear and environmental adaptation
- Archetype-based appearance customization

### 2. MetaHuman Integration
- Seamless MetaHuman Creator compatibility
- LOD system for performance optimization
- Animation blueprint integration
- Facial expression system

### 3. Survival Visual Elements
- Dynamic dirt/wear system
- Clothing degradation over time
- Scar and injury visualization
- Weather exposure effects

## Components

### Primary Components
- **MetaHuman Skeletal Mesh Component**: Core visual representation
- **Character Variation Data Component**: Stores procedural variation data
- **Survival State Component**: Tracks survival condition and wear
- **Clothing System Component**: Manages modular clothing and equipment

### Secondary Components
- **Facial Expression Component**: Drives emotion-based expressions
- **Environmental Wear Component**: Handles weather and dirt accumulation
- **Injury Visualization Component**: Shows scars, cuts, and bruises

## Blueprint Events

### Core Events
- **OnCharacterGenerated**: Fired when character variation is applied
- **OnSurvivalStateChanged**: Triggered when survival condition changes
- **OnClothingDegraded**: Called when clothing wear reaches thresholds
- **OnEnvironmentalExposure**: Responds to weather and environmental damage

### Animation Events
- **OnEmotionChanged**: Updates facial expressions
- **OnInjuryReceived**: Triggers injury visualization
- **OnHealingProgress**: Updates healing visual effects

## Blueprint Functions

### Character Generation
```cpp
GenerateCharacterVariation(ECharacterArchetype Archetype) -> FCharacterVariationData
ApplyCharacterData(FCharacterVariationData Data) -> void
RandomizeWithinArchetype(ECharacterArchetype Archetype) -> void
```

### Survival System
```cpp
ApplySurvivalWear(float WearLevel) -> void
UpdateClothingState(float DegradationLevel) -> void
AddEnvironmentalDamage(float Amount) -> void
```

### Appearance Management
```cpp
SetArchetypeAppearance(ECharacterArchetype Archetype) -> void
UpdateFacialExpression(ESurvivalEmotion Emotion) -> void
ApplyInjuryVisualization(FInjuryData Injury) -> void
```

## Blueprint Variables

### Character Data
- **CharacterData** (FCharacterVariationData): Complete character variation data
- **CurrentArchetype** (ECharacterArchetype): Character's role/type
- **SurvivalDays** (float): Days survived in wilderness
- **CurrentWearLevel** (float): Overall wear and tear level

### Visual State
- **ArchetypeClothing** (TArray<UStaticMeshComponent>): Modular clothing pieces
- **FacialExpressionBlends** (TMap<FString, float>): Facial expression weights
- **InjuryMarkers** (TArray<FInjuryData>): Active injuries and scars
- **DirtAccumulation** (float): Environmental dirt and grime level

### MetaHuman Integration
- **MetaHumanPreset** (UMetaHumanAsset): Base MetaHuman configuration
- **BodyTypeVariation** (float): Body shape variation within archetype
- **FacialFeatureVariation** (TMap<FString, float>): Facial feature adjustments

## Implementation Notes

### Performance Considerations
- All characters use the same base skeleton for animation compatibility
- LOD system automatically reduces detail at distance
- Clothing components use instanced static meshes for optimization
- Facial expressions use morph targets with LOD scaling

### Visual Consistency
- Clothing and equipment are modular components
- Each character maintains a unique visual identity
- Facial expressions are driven by the survival state
- Color palettes are consistent with prehistoric setting

### Technical Requirements
- Requires MetaHuman Creator Core Data package
- Compatible with UE5 Nanite and Lumen systems
- Supports Virtual Shadow Maps for detailed shadows
- Optimized for 60fps on PC, 30fps on console

## Usage Examples

### Creating a Tribal Warrior
```cpp
// Generate random tribal warrior
FCharacterVariationData WarriorData = UCharacterSystem::GenerateRandomCharacter(ECharacterArchetype::TribalWarrior);
ApplyCharacterData(WarriorData);

// Apply battle-hardened appearance
ApplySurvivalWear(0.7f);
AddEnvironmentalDamage(0.5f);
```

### Creating the Protagonist
```cpp
// Create specific protagonist
FCharacterVariationData ProtagonistData;
ProtagonistData.Archetype = ECharacterArchetype::Protagonist;
ProtagonistData.CharacterName = "Dr. Sarah Chen";
ProtagonistData.Age = ECharacterAge::MiddleAged;
ProtagonistData.Ethnicity = ECharacterEthnicity::Asian;

ApplyCharacterData(ProtagonistData);
SetArchetypeAppearance(ECharacterArchetype::Protagonist);
```

## File Structure
```
Content/Characters/MetaHuman/
├── BP_MetaHumanCharacter.uasset
├── Animations/
│   ├── ABP_MetaHumanAnimBP.uasset
│   └── Expressions/
├── Materials/
│   ├── M_SkinVariations.uasset
│   ├── M_ClothingBase.uasset
│   └── M_SurvivalWear.uasset
├── Clothing/
│   ├── Tribal/
│   ├── Modern/
│   └── Survival/
└── Presets/
    ├── Protagonist/
    ├── Tribal/
    └── Survivors/
```

This blueprint forms the foundation of the character system and will be extended by the Animation Agent for movement and behavior integration.