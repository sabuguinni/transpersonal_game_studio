#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "../Core/SharedTypes.h"
#include "TribalSurvivalBehavior.generated.h"

class APawn;
class ACharacter;

// Tribal NPC survival instincts and needs
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SurvivalNeeds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Energy = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Comfort = 60.0f;

    FNPC_SurvivalNeeds()
    {
        Hunger = 50.0f;
        Thirst = 50.0f;
        Energy = 80.0f;
        Health = 100.0f;
        Fear = 10.0f;
        Comfort = 60.0f;
    }
};

// Tribal NPC role types
UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Explorer    UMETA(DisplayName = "Explorer"),
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Guard       UMETA(DisplayName = "Guard")
};

// Current NPC behavior state
UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Searching       UMETA(DisplayName = "Searching"),
    Working         UMETA(DisplayName = "Working"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Resting         UMETA(DisplayName = "Resting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Investigating   UMETA(DisplayName = "Investigating")
};

// Threat detection data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_ThreatInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float LastSeenTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    bool bIsActive = false;

    FNPC_ThreatInfo()
    {
        ThreatActor = nullptr;
        ThreatLocation = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        LastSeenTime = 0.0f;
        bIsActive = false;
    }
};

/**
 * Enhanced tribal survival behavior system for NPCs
 * Handles survival needs, role-based behavior, threat detection, and social interactions
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTribalSurvivalBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UTribalSurvivalBehavior();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core survival system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FNPC_SurvivalNeeds SurvivalNeeds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Role")
    ENPC_TribalRole TribalRole = ENPC_TribalRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentBehaviorState = ENPC_BehaviorState::Idle;

    // Threat detection and response
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float ThreatDetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float DinosaurThreatLevel = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float PlayerThreatLevel = 30.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Threat Detection")
    FNPC_ThreatInfo CurrentThreat;

    // Territory and movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed = 400.0f;

    // Social behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float CallForHelpRadius = 1200.0f;

    // Behavior timers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float BehaviorUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float SurvivalUpdateInterval = 5.0f;

    // Core behavior functions
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateBehaviorState();

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void ScanForThreats();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveToLocation(const FVector& TargetLocation, bool bRunning = false);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void CallForHelp();

    UFUNCTION(BlueprintCallable, Category = "Social")
    void RespondToDistressCall(const FVector& DistressLocation);

    // Role-specific behaviors
    UFUNCTION(BlueprintCallable, Category = "Role Behavior")
    void ExecuteHunterBehavior();

    UFUNCTION(BlueprintCallable, Category = "Role Behavior")
    void ExecuteGathererBehavior();

    UFUNCTION(BlueprintCallable, Category = "Role Behavior")
    void ExecuteExplorerBehavior();

    UFUNCTION(BlueprintCallable, Category = "Role Behavior")
    void ExecuteElderBehavior();

    UFUNCTION(BlueprintCallable, Category = "Role Behavior")
    void ExecuteChildBehavior();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsInDanger() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool NeedsFood() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool NeedsWater() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool NeedsRest() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetDistanceToHome() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetRandomPatrolPoint() const;

private:
    // Internal timers
    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle SurvivalUpdateTimer;

    // Cached references
    UPROPERTY()
    APawn* OwnerPawn;

    UPROPERTY()
    ACharacter* OwnerCharacter;

    // Internal state tracking
    float LastThreatScanTime;
    float LastSocialInteractionTime;
    FVector LastKnownPlayerLocation;
    
    // Helper functions
    void InitializeBehavior();
    void SetBehaviorState(ENPC_BehaviorState NewState);
    AActor* FindNearestThreat() const;
    TArray<AActor*> FindNearbyTribalNPCs() const;
    bool IsLocationSafe(const FVector& Location) const;
    void HandleCriticalNeeds();
    void ExecuteRoleBasedBehavior();
};