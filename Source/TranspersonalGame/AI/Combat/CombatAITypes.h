#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "CombatAITypes.generated.h"

UENUM(BlueprintType)
enum class EDinosaurThreatLevel : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Territorial UMETA(DisplayName = "Territorial"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Apex        UMETA(DisplayName = "Apex Predator")
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Stalk       UMETA(DisplayName = "Stalk"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Territorial UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class EHuntingStrategy : uint8
{
    Ambush      UMETA(DisplayName = "Ambush"),
    Pack        UMETA(DisplayName = "Pack Hunt"),
    Pursuit     UMETA(DisplayName = "Direct Pursuit"),
    Territorial UMETA(DisplayName = "Territorial Defense"),
    Opportunist UMETA(DisplayName = "Opportunist")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatAIProfile : public FTableRowBase
{
    GENERATED_BODY()

    // Basic Threat Assessment
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    EDinosaurThreatLevel ThreatLevel = EDinosaurThreatLevel::Cautious;

    // Detection Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float SightAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float HearingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float SmellRadius = 1200.0f;

    // Combat Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    EHuntingStrategy PreferredStrategy = EHuntingStrategy::Pursuit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float OptimalDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeHealthThreshold = 0.3f;

    // Personality Traits (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Patience = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;

    // Gameplay Tags for abilities and behaviors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
    FGameplayTagContainer AbilityTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behaviors")
    FGameplayTagContainer BehaviorTags;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FThreatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<AActor> ThreatActor;

    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bIsActive = false;

    FThreatMemory()
    {
        ThreatActor = nullptr;
        ThreatLevel = 0.0f;
        LastSeenTime = 0.0f;
        bIsActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatDecision
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    ECombatState RecommendedState = ECombatState::Idle;

    UPROPERTY(BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(BlueprintReadWrite)
    float Confidence = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    FString Reasoning;
};