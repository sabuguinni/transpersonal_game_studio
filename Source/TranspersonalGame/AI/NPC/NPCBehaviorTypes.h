#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NPCBehaviorTypes.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"), 
    Investigating   UMETA(DisplayName = "Investigating"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Following       UMETA(DisplayName = "Following"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Resting         UMETA(DisplayName = "Resting"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Working         UMETA(DisplayName = "Working")
};

UENUM(BlueprintType)
enum class ENPC_PersonalityType : uint8
{
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Curious         UMETA(DisplayName = "Curious"),
    Friendly        UMETA(DisplayName = "Friendly"),
    Hostile         UMETA(DisplayName = "Hostile"),
    Neutral         UMETA(DisplayName = "Neutral"),
    Protective      UMETA(DisplayName = "Protective"),
    Territorial     UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    Leader          UMETA(DisplayName = "Leader"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Guard           UMETA(DisplayName = "Guard"),
    Shaman          UMETA(DisplayName = "Shaman"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Scout           UMETA(DisplayName = "Scout"),
    Elder           UMETA(DisplayName = "Elder")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Stats")
    float Courage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Stats")
    float Aggression = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Stats")
    float Curiosity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Stats")
    float Sociability = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Stats")
    float Intelligence = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Stats")
    float Loyalty = 50.0f;

    FNPC_BehaviorStats()
    {
        Courage = 50.0f;
        Aggression = 50.0f;
        Curiosity = 50.0f;
        Sociability = 50.0f;
        Intelligence = 50.0f;
        Loyalty = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* Target = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsHostile = false;

    FNPC_MemoryEntry()
    {
        Target = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        Timestamp = 0.0f;
        ThreatLevel = 0.0f;
        bIsHostile = false;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float EndTime = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    ENPC_BehaviorState Activity = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float Priority = 1.0f;

    FNPC_DailyRoutine()
    {
        StartTime = 0.0f;
        EndTime = 24.0f;
        Activity = ENPC_BehaviorState::Idle;
        TargetLocation = FVector::ZeroVector;
        Priority = 1.0f;
    }
};