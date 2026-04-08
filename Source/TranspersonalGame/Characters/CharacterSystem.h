#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "MetaHumanCreator/Public/MetaHumanCharacter.h"
#include "CharacterSystem.generated.h"

// Enum para tipos de personagens
UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist         UMETA(DisplayName = "Protagonista"),
    TribalElder        UMETA(DisplayName = "Ancião Tribal"),
    TribalWarrior      UMETA(DisplayName = "Guerreiro Tribal"),
    TribalShaman       UMETA(DisplayName = "Xamã Tribal"),
    TribalCrafter      UMETA(DisplayName = "Artesão Tribal"),
    TribalChild        UMETA(DisplayName = "Criança Tribal"),
    TimeTraveler       UMETA(DisplayName = "Viajante Temporal"),
    Survivor           UMETA(DisplayName = "Sobrevivente"),
    Hermit             UMETA(DisplayName = "Eremita"),
    Trader             UMETA(DisplayName = "Comerciante")
};

// Estrutura para características físicas procedurais
USTRUCT(BlueprintType)
struct FCharacterGeneticTraits
{
    GENERATED_BODY()

    // Características faciais (0.0 - 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float FaceWidth = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float EyeSize = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float NoseSize = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float MouthSize = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float CheekboneHeight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float JawWidth = 0.5f;

    // Características corporais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float BodyHeight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float BodyWeight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float MuscleDefinition = 0.5f;

    // Características de idade/desgaste
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float AgeWear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float BattleScars = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float WeatherExposure = 0.0f;
};

// Estrutura para aparência cultural
USTRUCT(BlueprintType)
struct FCharacterCulturalAppearance
{
    GENERATED_BODY()

    // Cor da pele (tons naturais)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    // Cor dos olhos
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);

    // Cor do cabelo
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    // Estilo de cabelo (referência para MetaHuman)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString HairStyle = "Tribal_Long";

    // Tatuagens/pinturas corporais
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> BodyMarkings;

    // Cicatrizes rituais
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RitualScars;
};

// Estrutura para vestuário
USTRUCT(BlueprintType)
struct FCharacterClothing
{
    GENERATED_BODY()

    // Materiais base
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ClothingMaterials; // Pele, fibras vegetais, osso, etc.

    // Nível de desgaste (0 = novo, 1 = muito desgastado)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float WearLevel = 0.3f;

    // Ornamentos
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Ornaments; // Dentes, garras, pedras, etc.

    // Cor dominante
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor PrimaryColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);

    // Cor secundária
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SecondaryColor = FLinearColor(0.2f, 0.15f, 0.1f, 1.0f);
};

// Estrutura principal do personagem
USTRUCT(BlueprintType)
struct FCharacterDefinition
{
    GENERATED_BODY()

    // Identificação
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CharacterName = "Unnamed";

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterArchetype Archetype = ECharacterArchetype::Survivor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMale = true;

    // Características físicas
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FCharacterGeneticTraits GeneticTraits;

    // Aparência cultural
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FCharacterCulturalAppearance CulturalAppearance;

    // Vestuário
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FCharacterClothing Clothing;

    // História pessoal (para expressão facial)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PersonalHistory = "A vida deixou marcas subtis no rosto.";

    // Seed para geração procedural
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GeneticSeed = 0;
};

// Data Asset para configuração de arquétipos
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterArchetypeData : public UDataAsset
{
    GENERATED_BODY()

public:
    // Mapeamento de arquétipos para configurações base
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<ECharacterArchetype, FCharacterDefinition> ArchetypeDefaults;

    // Variações permitidas por arquétipo
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<ECharacterArchetype, float> VariationRanges;
};

// Sistema principal de geração de personagens
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCharacterGenerationSystem : public UObject
{
    GENERATED_BODY()

public:
    UCharacterGenerationSystem();

    // Gerar personagem procedural
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FCharacterDefinition GenerateCharacter(ECharacterArchetype Archetype, int32 Seed = -1);

    // Aplicar definição a MetaHuman
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    bool ApplyDefinitionToMetaHuman(class AMetaHumanCharacter* MetaHuman, const FCharacterDefinition& Definition);

    // Gerar variação genética
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FCharacterGeneticTraits GenerateGeneticVariation(const FCharacterGeneticTraits& BaseTraits, float VariationStrength, int32 Seed);

    // Validar unicidade do personagem
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    bool IsCharacterUnique(const FCharacterDefinition& Character, float SimilarityThreshold = 0.8f);

protected:
    // Data Asset com configurações de arquétipos
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UCharacterArchetypeData* ArchetypeData;

    // Lista de personagens já gerados (para evitar duplicatas)
    UPROPERTY()
    TArray<FCharacterDefinition> GeneratedCharacters;

    // Funções auxiliares
    float GenerateRandomFloat(int32& Seed, float Min = 0.0f, float Max = 1.0f);
    FLinearColor GenerateNaturalSkinTone(int32& Seed);
    FLinearColor GenerateNaturalEyeColor(int32& Seed);
    FLinearColor GenerateNaturalHairColor(int32& Seed);
    FString SelectRandomHairStyle(int32& Seed, bool bIsMale);
    TArray<FString> GenerateTribalMarkings(int32& Seed, ECharacterArchetype Archetype);
};