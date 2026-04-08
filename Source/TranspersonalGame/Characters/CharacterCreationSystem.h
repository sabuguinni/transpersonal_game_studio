#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataTable.h"
#include "CharacterCreationSystem.generated.h"

// Estrutura para definir características físicas base
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCharacterPhysicalTraits
{
    GENERATED_BODY()

    // Características faciais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float FaceWidth = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float FaceHeight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float EyeSize = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float NoseSize = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float MouthSize = 0.5f;

    // Características corporais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Features")
    float Height = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Features")
    float Weight = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Features")
    float MuscleDefinition = 0.5f;

    // Características de pele
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
    float SkinRoughness = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
    float WeatheringLevel = 0.2f; // Marcas de exposição ao sol/elementos
};

// Estrutura para roupas e acessórios pré-históricos
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPrehistoricClothing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<FString> AvailableClothingPieces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString PrimaryMaterial = "Animal Hide"; // Pele de animal, fibras vegetais, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor ClothingColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    float WearLevel = 0.3f; // Desgaste das roupas

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessories")
    TArray<FString> Accessories; // Colares de osso, penas, pinturas corporais
};

// Arquétipos sociais para NPCs
UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    None            UMETA(DisplayName = "None"),
    TribalLeader    UMETA(DisplayName = "Tribal Leader"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Shaman          UMETA(DisplayName = "Shaman"),
    Craftsperson    UMETA(DisplayName = "Craftsperson"),
    Elder           UMETA(DisplayName = "Elder"),
    Child           UMETA(DisplayName = "Child"),
    Warrior         UMETA(DisplayName = "Warrior"),
    Storyteller     UMETA(DisplayName = "Storyteller")
};

// Estrutura principal do personagem
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCharacterDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterArchetype Archetype = ECharacterArchetype::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    bool bIsMale = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    FCharacterPhysicalTraits PhysicalTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FPrehistoricClothing Clothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FString MetaHumanAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
    int32 UniqueID; // Para garantir que cada NPC seja único
};

/**
 * Sistema de criação de personagens usando MetaHuman Creator
 * Gera NPCs únicos e autênticos para o período pré-histórico
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCharacterCreationSystem : public UObject
{
    GENERATED_BODY()

public:
    UCharacterCreationSystem();

    // Gera um personagem aleatório baseado em parâmetros
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    FCharacterDefinition GenerateRandomCharacter(ECharacterArchetype DesiredArchetype = ECharacterArchetype::None);

    // Cria o protagonista (paleontologista)
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    FCharacterDefinition CreateProtagonist();

    // Aplica características físicas a um MetaHuman
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    bool ApplyPhysicalTraits(USkeletalMeshComponent* MetaHumanMesh, const FCharacterPhysicalTraits& Traits);

    // Aplica roupas e acessórios pré-históricos
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    bool ApplyPrehistoricClothing(USkeletalMeshComponent* MetaHumanMesh, const FPrehistoricClothing& Clothing);

    // Gera variações únicas para evitar clones
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    FCharacterPhysicalTraits GenerateUniqueVariation(int32 SeedValue);

protected:
    // Tabelas de dados para geração procedural
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Tables")
    UDataTable* ArchetypeTraitsTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Tables")
    UDataTable* ClothingVariationsTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Tables")
    UDataTable* NameGenerationTable;

    // Configurações de diversidade
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity Settings")
    float PhysicalVariationRange = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity Settings")
    TArray<FLinearColor> SkinToneVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity Settings")
    float WeatheringVariationRange = 0.4f;

private:
    // Funções auxiliares
    FCharacterPhysicalTraits GenerateTraitsForArchetype(ECharacterArchetype Archetype);
    FPrehistoricClothing GenerateClothingForArchetype(ECharacterArchetype Archetype);
    FString GenerateNameForCharacter(bool bIsMale, ECharacterArchetype Archetype);
    
    // Contador para IDs únicos
    static int32 NextUniqueID;
};