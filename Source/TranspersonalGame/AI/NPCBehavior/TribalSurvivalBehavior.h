#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TribalSurvivalBehavior.generated.h"

// Tribal survival needs and states
UENUM(BlueprintType)
enum class ENPC_SurvivalNeed : uint8
{
    None        UMETA(DisplayName = "None"),
    Hunger      UMETA(DisplayName = "Hunger"),
    Thirst      UMETA(DisplayName = "Thirst"),
    Rest        UMETA(DisplayName = "Rest"),
    Safety      UMETA(DisplayName = "Safety"),
    Social      UMETA(DisplayName = "Social"),
    Shelter     UMETA(DisplayName = "Shelter")
};

// Tribal roles with specific behaviors
UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Scout       UMETA(DisplayName = "Scout"),
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Crafter     UMETA(DisplayName = "Crafter")
};

// Current behavior state
UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Resting     UMETA(DisplayName = "Resting"),
    Socializing UMETA(DisplayName = "Socializing"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Sleeping    UMETA(DisplayName = "Sleeping")
};

// Survival stats structure
USTRUCT(BlueprintType)
struct FNPC_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Energy = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Fear = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float SocialNeed = 60.0f;

    FNPC_SurvivalStats()
    {
        Hunger = 80.0f;
        Thirst = 75.0f;
        Energy = 90.0f;
        Health = 100.0f;
        Fear = 10.0f;
        SocialNeed = 60.0f;
    }
};

// Memory of important locations
USTRUCT(BlueprintType)
struct FNPC_LocationMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString LocationName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Importance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastVisitTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsSafe = true;

    FNPC_LocationMemory()
    {
        Location = FVector::ZeroVector;
        LocationName = TEXT("");
        Importance = 0.0f;
        LastVisitTime = 0.0f;
        bIsSafe = true;
    }
};

/**
 * Advanced tribal survival behavior component for NPCs
 * Handles complex survival needs, role-based behavior, and social dynamics
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_TribalSurvivalBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TribalSurvivalBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    ENPC_TribalRole TribalRole = ENPC_TribalRole::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    FString NPCName = TEXT("Tribal Member");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FNPC_SurvivalStats SurvivalStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentBehaviorState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float BehaviorChangeInterval = 30.0f;

    // Territory and Movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float MaxWanderDistance = 1500.0f;

    // Social Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<AActor*> KnownTribalMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialInteractionRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastSocialInteraction = 0.0f;

    // Memory System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_LocationMemory> KnownLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemoryLocations = 20;

    // Danger Detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger")
    float DangerDetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger")
    float FleeDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Danger")
    TArray<AActor*> KnownThreats;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateSurvivalNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void EvaluateBehaviorState();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ExecuteCurrentBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveToLocation(const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector FindRandomLocationInTerritory();

    // Social Functions
    UFUNCTION(BlueprintCallable, Category = "Social")
    void DetectNearbyTribalMembers();

    UFUNCTION(BlueprintCallable, Category = "Social")
    void InteractWithNearbyNPCs();

    UFUNCTION(BlueprintCallable, Category = "Social")
    void CallForHelp();

    // Memory Functions
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberLocation(const FVector& Location, const FString& Name, float Importance, bool bSafe = true);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPC_LocationMemory FindBestLocationForNeed(ENPC_SurvivalNeed Need);

    // Danger Functions
    UFUNCTION(BlueprintCallable, Category = "Danger")
    void DetectThreats();

    UFUNCTION(BlueprintCallable, Category = "Danger")
    void FleeFromDanger();

    UFUNCTION(BlueprintCallable, Category = "Danger")
    bool IsLocationSafe(const FVector& Location);

    // Role-specific Behaviors
    UFUNCTION(BlueprintCallable, Category = "Role Behavior")
    void ExecuteHunterBehavior();

    UFUNCTION(BlueprintCallable, Category = "Role Behavior")
    void ExecuteGathererBehavior();

    UFUNCTION(BlueprintCallable, Category = "Role Behavior")
    void ExecuteScoutBehavior();

    UFUNCTION(BlueprintCallable, Category = "Role Behavior")
    void ExecuteElderBehavior();

    UFUNCTION(BlueprintCallable, Category = "Role Behavior")
    void ExecuteChildBehavior();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    ENPC_SurvivalNeed GetHighestPriorityNeed();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetNeedUrgency(ENPC_SurvivalNeed Need);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsNearHomeLocation(float Tolerance = 200.0f);

private:
    // Internal state
    float LastBehaviorChange = 0.0f;
    float LastNeedUpdate = 0.0f;
    FVector CurrentTarget = FVector::ZeroVector;
    bool bHasValidTarget = false;
    
    // Cached references
    UPROPERTY()
    ACharacter* OwnerCharacter = nullptr;
    
    UPROPERTY()
    UCharacterMovementComponent* MovementComponent = nullptr;

    // Internal helper functions
    void InitializeNPC();
    void UpdateMovement(float DeltaTime);
    bool HasReachedTarget(float Tolerance = 100.0f);
    float CalculateDistanceToTarget();
    void SetRandomBehaviorState();
};