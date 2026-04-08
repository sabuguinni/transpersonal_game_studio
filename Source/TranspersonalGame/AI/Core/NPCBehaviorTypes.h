#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "NPCBehaviorTypes.generated.h"

// Enum para tipos de dinossauros
UENUM(BlueprintType)
enum class EDinosaurType : uint8
{
    SmallHerbivore      UMETA(DisplayName = "Small Herbivore"),
    MediumHerbivore     UMETA(DisplayName = "Medium Herbivore"),
    LargeHerbivore      UMETA(DisplayName = "Large Herbivore"),
    SmallCarnivore      UMETA(DisplayName = "Small Carnivore"),
    MediumCarnivore     UMETA(DisplayName = "Medium Carnivore"),
    LargeCarnivore      UMETA(DisplayName = "Large Carnivore"),
    Omnivore           UMETA(DisplayName = "Omnivore"),
    Scavenger          UMETA(DisplayName = "Scavenger")
};

// Enum para estados comportamentais
UENUM(BlueprintType)
enum class EBehaviorState : uint8
{
    Idle               UMETA(DisplayName = "Idle"),
    Foraging           UMETA(DisplayName = "Foraging"),
    Hunting            UMETA(DisplayName = "Hunting"),
    Drinking           UMETA(DisplayName = "Drinking"),
    Resting            UMETA(DisplayName = "Resting"),
    Socializing        UMETA(DisplayName = "Socializing"),
    Migrating          UMETA(DisplayName = "Migrating"),
    Fleeing            UMETA(DisplayName = "Fleeing"),
    Territorial        UMETA(DisplayName = "Territorial"),
    Mating             UMETA(DisplayName = "Mating"),
    Nesting            UMETA(DisplayName = "Nesting"),
    Investigating      UMETA(DisplayName = "Investigating")
};

// Enum para níveis de domesticação
UENUM(BlueprintType)
enum class EDomesticationLevel : uint8
{
    Wild               UMETA(DisplayName = "Wild"),
    Wary               UMETA(DisplayName = "Wary"),
    Curious            UMETA(DisplayName = "Curious"),
    Tolerant           UMETA(DisplayName = "Tolerant"),
    Friendly           UMETA(DisplayName = "Friendly"),
    Bonded             UMETA(DisplayName = "Bonded"),
    Domesticated       UMETA(DisplayName = "Domesticated")
};

// Enum para períodos do dia
UENUM(BlueprintType)
enum class EDayPeriod : uint8
{
    Dawn               UMETA(DisplayName = "Dawn"),
    Morning            UMETA(DisplayName = "Morning"),
    Midday             UMETA(DisplayName = "Midday"),
    Afternoon          UMETA(DisplayName = "Afternoon"),
    Dusk               UMETA(DisplayName = "Dusk"),
    Night              UMETA(DisplayName = "Night"),
    DeepNight          UMETA(DisplayName = "Deep Night")
};

// Struct para características físicas únicas
USTRUCT(BlueprintType)
struct FPhysicalTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SizeVariation = 1.0f; // 0.8 - 1.2 multiplicador do tamanho base

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor PrimaryColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SecondaryColor = FLinearColor::Gray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AccentColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HornLength = 1.0f; // Multiplicador para cornos/cristas

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TailLength = 1.0f; // Multiplicador para cauda

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NeckLength = 1.0f; // Multiplicador para pescoço

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> DistinctiveMarks; // Cicatrizes, manchas, etc.

    FPhysicalTraits()
    {
        SizeVariation = FMath::RandRange(0.85f, 1.15f);
        HornLength = FMath::RandRange(0.9f, 1.1f);
        TailLength = FMath::RandRange(0.95f, 1.05f);
        NeckLength = FMath::RandRange(0.95f, 1.05f);
    }
};

// Struct para memórias do NPC
USTRUCT(BlueprintType)
struct FNPCMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 1.0f; // Quão marcante foi o evento

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* RelatedActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;
};

// Struct para rotina diária
USTRUCT(BlueprintType)
struct FDailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDayPeriod Period;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBehaviorState PreferredBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 60.0f; // Em minutos de jogo

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority = 1.0f;
};

// Struct para relações sociais
USTRUCT(BlueprintType)
struct FSocialRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Affinity = 0.0f; // -1.0 (hostil) a 1.0 (amigável)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Familiarity = 0.0f; // 0.0 (desconhecido) a 1.0 (muito familiar)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastInteraction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FNPCMemory> SharedMemories;
};

// Data Table Row para configuração de espécies
USTRUCT(BlueprintType)
struct FDinosaurSpeciesData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseSize = 100.0f; // Em centímetros

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseSpeed = 300.0f; // Unreal Units por segundo

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.5f; // 0.0 pacífico - 1.0 muito agressivo

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialTendency = 0.5f; // 0.0 solitário - 1.0 muito social

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDailyRoutine> DefaultRoutines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> PreferredHabitats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> FoodSources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> Predators;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> Prey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MemoryDuration = 300.0f; // Tempo em segundos que lembram de eventos

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DomesticationRate = 0.1f; // Velocidade de domesticação por interação positiva
};