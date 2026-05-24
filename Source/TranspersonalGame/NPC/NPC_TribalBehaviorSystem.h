#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "NPC_TribalBehaviorSystem.generated.h"

// Forward declarations
class UBehaviorTreeComponent;
class UBlackboardComponent;
class AAIController;

UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Guard       UMETA(DisplayName = "Guard"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Child       UMETA(DisplayName = "Child"),
    Chief       UMETA(DisplayName = "Chief"),
    Crafter     UMETA(DisplayName = "Crafter")
};

UENUM(BlueprintType)
enum class ENPC_TribalState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alert       UMETA(DisplayName = "Alert"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Fighting    UMETA(DisplayName = "Fighting"),
    Socializing UMETA(DisplayName = "Socializing"),
    Resting     UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct FNPC_TribalMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> KnownDangerLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> KnownResourceLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownFriendlyActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownHostileActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastDangerTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector HomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastWorkLocation;

    FNPC_TribalMemory()
    {
        LastDangerTime = 0.0f;
        HomeLocation = FVector::ZeroVector;
        LastWorkLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FNPC_TribalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Morale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ExperiencePoints;

    FNPC_TribalStats()
    {
        Health = 100.0f;
        Stamina = 100.0f;
        Hunger = 50.0f;
        Fear = 0.0f;
        Morale = 75.0f;
        ExperiencePoints = 0.0f;
    }
};

/**
 * Advanced tribal NPC behavior system that manages complex social dynamics,
 * role-based behaviors, memory systems, and tribal hierarchy.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_TribalBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TribalBehaviorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core tribal properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    ENPC_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    int32 TribalRank; // 1=lowest, 10=chief

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    FString TribeName;

    // Current state and behavior
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior State")
    ENPC_TribalState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior State")
    float StateChangeTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior State")
    AActor* CurrentTarget;

    // Stats and memory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    FNPC_TribalStats TribalStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FNPC_TribalMemory TribalMemory;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Config")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Config")
    float FleeThreshold; // Fear level that triggers fleeing

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Config")
    float SocialRadius; // Distance for social interactions

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Config")
    float WorkRadius; // Distance from home for work activities

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Config")
    TArray<FVector> PatrolWaypoints;

    // Social relationships
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TMap<AActor*, float> SocialBonds; // Actor -> relationship strength (-100 to 100)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    AActor* TribalLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<AActor*> TribalMembers;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void SetTribalRole(ENPC_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void ChangeState(ENPC_TribalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void AddMemoryLocation(FVector Location, bool bIsDangerous);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void UpdateSocialBond(AActor* OtherActor, float BondChange);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    bool ShouldFlee();

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    AActor* FindNearestThreat();

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    AActor* FindNearestTribalMember();

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void PerformRoleBasedAction();

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void AlertTribe(FVector DangerLocation);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void ReceiveTribalAlert(FVector DangerLocation, AActor* AlertSource);

    // Role-specific behavior functions
    UFUNCTION(BlueprintCallable, Category = "Role Behaviors")
    void HunterBehavior();

    UFUNCTION(BlueprintCallable, Category = "Role Behaviors")
    void GathererBehavior();

    UFUNCTION(BlueprintCallable, Category = "Role Behaviors")
    void GuardBehavior();

    UFUNCTION(BlueprintCallable, Category = "Role Behaviors")
    void ShamanBehavior();

    UFUNCTION(BlueprintCallable, Category = "Role Behaviors")
    void ChildBehavior();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float GetDistanceToHome() const;

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool IsInWorkingHours() const;

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void UpdateStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void RegenerateStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void TakeDamage(float Damage, AActor* DamageSource);

private:
    // Internal state management
    float TimeSinceLastStateChange;
    float TimeSinceLastSocialInteraction;
    int32 CurrentPatrolIndex;
    
    // Cached references
    APawn* OwnerPawn;
    AAIController* AIController;
    
    // Internal behavior functions
    void UpdateBehaviorState(float DeltaTime);
    void ProcessSocialInteractions(float DeltaTime);
    void UpdateMemory(float DeltaTime);
    void HandleEmergencyBehavior();
    FVector FindSafeLocation();
    bool CanPerformAction() const;
};