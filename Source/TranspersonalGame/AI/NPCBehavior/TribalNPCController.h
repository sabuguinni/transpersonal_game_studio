#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Pawn.h"
#include "TribalNPCController.generated.h"

UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"), 
    Scout       UMETA(DisplayName = "Scout"),
    Elder       UMETA(DisplayName = "Elder")
};

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Working     UMETA(DisplayName = "Working"),
    Socializing UMETA(DisplayName = "Socializing"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Investigating UMETA(DisplayName = "Investigating")
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Importance;

    FNPC_MemoryEntry()
    {
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        EventType = TEXT("Unknown");
        Importance = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATribalNPCController : public AAIController
{
    GENERATED_BODY()

public:
    ATribalNPCController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core NPC Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal NPC", meta = (AllowPrivateAccess = "true"))
    ENPC_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal NPC", meta = (AllowPrivateAccess = "true"))
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal NPC", meta = (AllowPrivateAccess = "true"))
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal NPC", meta = (AllowPrivateAccess = "true"))
    float WorkDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal NPC", meta = (AllowPrivateAccess = "true"))
    float SocialDistance;

    // Memory System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_MemoryEntry> ShortTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_MemoryEntry> LongTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory", meta = (AllowPrivateAccess = "true"))
    int32 MaxShortTermMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory", meta = (AllowPrivateAccess = "true"))
    int32 MaxLongTermMemories;

    // Behavior Timers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (AllowPrivateAccess = "true"))
    float StateChangeTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (AllowPrivateAccess = "true"))
    float NextStateChangeTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (AllowPrivateAccess = "true"))
    FVector HomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (AllowPrivateAccess = "true"))
    FVector CurrentTarget;

public:
    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetTribalRole(ENPC_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ChangeBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartWorking();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartSocializing();

    // Memory Functions
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemoryEntry(FVector Location, FString EventType, float Importance);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ProcessMemories();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool HasMemoryOfLocation(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPC_MemoryEntry GetMostImportantMemory();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetDistanceToPlayer();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsPlayerNearby(float Radius);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetRandomPatrolPoint();

private:
    void UpdateBehaviorState(float DeltaTime);
    void ExecuteCurrentBehavior();
    void HandleRoleSpecificBehavior();
};

#include "TribalNPCController.generated.h"