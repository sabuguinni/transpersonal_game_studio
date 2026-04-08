#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataAsset.h"
#include "CharacterSystem.generated.h"

/**
 * Enumeração dos tipos de personagens no jogo
 */
UENUM(BlueprintType)
enum class ECharacterType : uint8
{
    Protagonist     UMETA(DisplayName = "Protagonist"),
    Survivor        UMETA(DisplayName = "Survivor"),
    TribalNative    UMETA(DisplayName = "Tribal Native"),
    LostExplorer    UMETA(DisplayName = "Lost Explorer"),
    Researcher      UMETA(DisplayName = "Researcher"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Elder           UMETA(DisplayName = "Elder"),
    Child           UMETA(DisplayName = "Child"),
    Injured         UMETA(DisplayName = "Injured")
};

/**
 * Enumeração dos biomas que afetam a aparência dos personagens
 */
UENUM(BlueprintType)
enum class ECharacterBiome : uint8
{
    Forest          UMETA(DisplayName = "Forest"),
    Desert          UMETA(DisplayName = "Desert"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Mountain        UMETA(DisplayName = "Mountain"),
    Coast           UMETA(DisplayName = "Coast"),
    Cave            UMETA(DisplayName = "Cave"),
    Plains          UMETA(DisplayName = "Plains")
};

/**
 * Estrutura de dados para variação genética de personagens
 */
USTRUCT(BlueprintType)
struct FCharacterGeneticVariation
{
    GENERATED_BODY()

    // Variações faciais (0.0 a 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FaceWidth = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EyeSize = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NoseSize = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MouthWidth = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float JawWidth = 0.5f;

    // Variações corporais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Height = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BodyWeight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MuscleDefinition = 0.5f;

    // Cor da pele, olhos, cabelo
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    // Marcas e cicatrizes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ScarIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatheringLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DirtLevel = 0.2f;
};

/**
 * Estrutura para roupas e equipamentos baseados no ambiente
 */
USTRUCT(BlueprintType)
struct FCharacterClothing
{
    GENERATED_BODY()

    // Referências para meshes de roupa
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> TorsoMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> LegsMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> FeetMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> AccessoryMesh;

    // Materiais para variação
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    // Nível de desgaste (0 = novo, 1 = muito desgastado)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WearLevel = 0.3f;

    // Adaptação ao bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterBiome AdaptedBiome = ECharacterBiome::Forest;
};

/**
 * Data Asset principal para definir um arquétipo de personagem
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterArchetype : public UDataAsset
{
    GENERATED_BODY()

public:
    // Informações básicas
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info")
    FString ArchetypeName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info")
    ECharacterType CharacterType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Basic Info")
    FText Description;

    // MetaHuman base
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaHuman")
    TSoftObjectPtr<USkeletalMesh> BaseMetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaHuman")
    TSoftObjectPtr<UClass> BaseAnimationBlueprint;

    // Variações genéticas permitidas
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Genetics")
    FCharacterGeneticVariation MinVariation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Genetics")
    FCharacterGeneticVariation MaxVariation;

    // Roupas por bioma
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clothing")
    TMap<ECharacterBiome, FCharacterClothing> ClothingByBiome;

    // Probabilidade de spawn
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnProbability = 0.5f;

    // Histórias de fundo possíveis
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    TArray<FText> BackstoryOptions;
};

/**
 * Sistema principal de geração de personagens
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCharacterGenerationSystem : public UObject
{
    GENERATED_BODY()

public:
    // Gera um personagem único baseado num arquétipo
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    static FCharacterGeneticVariation GenerateUniqueVariation(const UCharacterArchetype* Archetype);

    // Aplica variação genética a um MetaHuman
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    static void ApplyGeneticVariation(USkeletalMeshComponent* MeshComponent, const FCharacterGeneticVariation& Variation);

    // Seleciona roupa adequada ao bioma
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    static FCharacterClothing GetClothingForBiome(const UCharacterArchetype* Archetype, ECharacterBiome Biome);

    // Gera uma história de fundo aleatória
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    static FText GenerateBackstory(const UCharacterArchetype* Archetype);

private:
    // Seed para garantir variações consistentes mas únicas
    static int32 GenerationSeed;
};