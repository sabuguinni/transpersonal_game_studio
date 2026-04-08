#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "DinosaurBehaviorTypes.generated.h"

// Enum para tipos de comportamento base
UENUM(BlueprintType)
enum class EDinosaurBehaviorType : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Resting         UMETA(DisplayName = "Resting"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Mating          UMETA(DisplayName = "Mating"),
    Nesting         UMETA(DisplayName = "Nesting"),
    Migrating       UMETA(DisplayName = "Migrating")
};

// Enum para estados emocionais
UENUM(BlueprintType)
enum class EDinosaurMoodState : uint8
{
    Calm            UMETA(DisplayName = "Calm"),
    Agitated        UMETA(DisplayName = "Agitated"),
    Curious         UMETA(DisplayName = "Curious"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Playful         UMETA(DisplayName = "Playful"),
    Tired           UMETA(DisplayName = "Tired"),
    Hungry          UMETA(DisplayName = "Hungry"),
    Thirsty         UMETA(DisplayName = "Thirsty"),
    Protective      UMETA(DisplayName = "Protective")
};

// Enum para níveis de domesticação
UENUM(BlueprintType)
enum class EDomesticationLevel : uint8
{
    Wild            UMETA(DisplayName = "Wild"),
    Wary            UMETA(DisplayName = "Wary"),
    Curious         UMETA(DisplayName = "Curious"),
    Tolerant        UMETA(DisplayName = "Tolerant"),
    Friendly        UMETA(DisplayName = "Friendly"),
    Bonded          UMETA(DisplayName = "Bonded"),
    Domesticated    UMETA(DisplayName = "Domesticated")
};

// Estrutura para memórias individuais
USTRUCT(BlueprintType)
struct FDinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* AssociatedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag MemoryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeStamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DecayRate;

    FDinosaurMemory()
    {
        Location = FVector::ZeroVector;
        AssociatedActor = nullptr;
        EmotionalWeight = 0.0f;
        TimeStamp = 0.0f;
        DecayRate = 1.0f;
    }
};

// Estrutura para características físicas únicas
USTRUCT(BlueprintType)
struct FDinosaurPhysicalTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float SizeVariation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float HornSizeVariation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float TailLengthVariation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor PrimaryColorTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SecondaryColorTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ScarIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> ScarLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float MovementSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float AggressionMultiplier = 1.0f;
};

// Estrutura para rotinas diárias
USTRUCT(BlueprintType)
struct FDailyRoutineEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float StartHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float EndHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurBehaviorType BehaviorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeInterrupted = true;

    FDailyRoutineEntry()
    {
        StartHour = 0.0f;
        EndHour = 1.0f;
        BehaviorType = EDinosaurBehaviorType::Idle;
        PreferredLocation = FVector::ZeroVector;
        Priority = 0.5f;
        bCanBeInterrupted = true;
    }
};

// Estrutura para perfil de personalidade
USTRUCT(BlueprintType)
struct FDinosaurPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggressiveness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fearfulness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Territoriality = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ActivityLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AdaptabilityToPlayer = 0.5f;
};

// Data Table Row para configuração de espécies
USTRUCT(BlueprintType)
struct FDinosaurSpeciesConfig : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCarnivore = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHerbivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "50"))
    int32 MaxPackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDailyRoutineEntry> DefaultDailyRoutine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDinosaurPersonality BasePersonality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EDinosaurBehaviorType> AvailableBehaviors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    float DetectionRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "50.0", ClampMax = "5000.0"))
    float FleeDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DomesticationRate = 1.0f;
};