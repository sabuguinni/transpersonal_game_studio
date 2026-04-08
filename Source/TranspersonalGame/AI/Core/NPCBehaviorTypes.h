#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "NPCBehaviorTypes.generated.h"

// Enum para tipos básicos de comportamento
UENUM(BlueprintType)
enum class EBasicBehaviorType : uint8
{
    Idle,
    Feeding,
    Drinking,
    Patrolling,
    Resting,
    Socializing,
    Hunting,
    Fleeing,
    Territorial,
    Mating,
    Nesting,
    Grooming
};

// Enum para estados emocionais
UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm,
    Alert,
    Aggressive,
    Fearful,
    Curious,
    Tired,
    Hungry,
    Thirsty,
    Territorial,
    Protective
};

// Enum para tipos de dinossauros
UENUM(BlueprintType)
enum class EDinosaurType : uint8
{
    SmallHerbivore,     // Compsognathus, Parasaurolophus pequenos
    MediumHerbivore,    // Triceratops, Stegosaurus
    LargeHerbivore,     // Brontosaurus, Diplodocus
    SmallCarnivore,     // Velociraptor, Deinonychus
    MediumCarnivore,    // Allosaurus, Carnotaurus
    LargeCarnivore,     // T-Rex, Giganotosaurus
    Flying,             // Pteranodon, Quetzalcoatlus
    Aquatic,            // Mosasaurus, Plesiosaurs
    Omnivore            // Therizinosaurus, Oviraptor
};

// Estrutura para necessidades básicas
USTRUCT(BlueprintType)
struct FBasicNeeds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Energy = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Safety = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Social = 100.0f;

    FBasicNeeds()
    {
        Hunger = 100.0f;
        Thirst = 100.0f;
        Energy = 100.0f;
        Safety = 100.0f;
        Social = 100.0f;
    }
};

// Estrutura para características físicas únicas
USTRUCT(BlueprintType)
struct FPhysicalTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SizeVariation = 1.0f; // 0.8 a 1.2 para variação de tamanho

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor PrimaryColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SecondaryColor = FLinearColor::Gray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.5f; // 0.0 = pacífico, 1.0 = muito agressivo

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CuriosityLevel = 0.5f; // 0.0 = tímido, 1.0 = muito curioso

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IntelligenceLevel = 0.5f; // Afeta capacidade de aprendizagem

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> UniqueMarkings; // Cicatrizes, padrões especiais, etc.

    FPhysicalTraits()
    {
        SizeVariation = 1.0f;
        PrimaryColor = FLinearColor::White;
        SecondaryColor = FLinearColor::Gray;
        AggressionLevel = 0.5f;
        CuriosityLevel = 0.5f;
        IntelligenceLevel = 0.5f;
    }
};

// Estrutura para memória de eventos
USTRUCT(BlueprintType)
struct FMemoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* InvolvedActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalImpact = 0.0f; // -1.0 (muito negativo) a 1.0 (muito positivo)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MemoryStrength = 1.0f; // Degrada com o tempo

    FMemoryEvent()
    {
        Location = FVector::ZeroVector;
        InvolvedActor = nullptr;
        EmotionalImpact = 0.0f;
        Timestamp = 0.0f;
        MemoryStrength = 1.0f;
    }
};

// Estrutura para relacionamentos sociais
USTRUCT(BlueprintType)
struct FSocialRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Familiarity = 0.0f; // 0.0 = desconhecido, 1.0 = muito familiar

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Trust = 0.0f; // -1.0 = inimigo, 0.0 = neutro, 1.0 = aliado

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastInteractionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PositiveInteractions = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NegativeInteractions = 0;

    FSocialRelationship()
    {
        TargetActor = nullptr;
        Familiarity = 0.0f;
        Trust = 0.0f;
        LastInteractionTime = 0.0f;
        PositiveInteractions = 0;
        NegativeInteractions = 0;
    }
};

// Data Table Row para configuração de espécies
USTRUCT(BlueprintType)
struct FDinosaurSpeciesData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurType DinosaurType = EDinosaurType::SmallHerbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseDomesticationDifficulty = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseAggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseMovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SightRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HearingRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EBasicBehaviorType> AvailableBehaviors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> PreferredFoodTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> FearTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialGroupSize = 1.0f; // Tamanho médio do grupo social

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNocturnal = false;

    FDinosaurSpeciesData()
    {
        DinosaurType = EDinosaurType::SmallHerbivore;
        SpeciesName = TEXT("Unknown");
        bCanBeDomesticated = false;
        BaseDomesticationDifficulty = 1.0f;
        BaseAggressionLevel = 0.5f;
        BaseMovementSpeed = 300.0f;
        SightRange = 1000.0f;
        HearingRange = 800.0f;
        SocialGroupSize = 1.0f;
        bIsNocturnal = false;
    }
};