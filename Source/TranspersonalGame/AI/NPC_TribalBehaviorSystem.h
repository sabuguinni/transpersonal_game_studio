#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "AI/BehaviorTree/BehaviorTree.h"
#include "AI/BehaviorTree/BlackboardComponent.h"
#include "AI/BehaviorTree/BehaviorTreeComponent.h"
#include "SharedTypes.h"
#include "NPC_TribalBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    Chief,
    Hunter,
    Gatherer,
    Crafter,
    Scout,
    Elder
};

UENUM(BlueprintType)
enum class ENPC_TribalActivity : uint8
{
    Idle,
    Patrolling,
    Hunting,
    Gathering,
    Crafting,
    Socializing,
    Fleeing,
    Defending
};

USTRUCT(BlueprintType)
struct FNPC_TribalMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> KnownDangerLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> KnownResourceLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> KnownTribeMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastDangerTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownSafeLocation;

    FNPC_TribalMemory()
    {
        LastDangerTime = 0.0f;
        LastKnownSafeLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FNPC_TribalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Energy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Loyalty;

    FNPC_TribalStats()
    {
        Hunger = 50.0f;
        Thirst = 50.0f;
        Energy = 100.0f;
        Fear = 0.0f;
        Loyalty = 75.0f;
    }
};

UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    ENPC_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    ENPC_TribalActivity CurrentActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    FNPC_TribalStats TribalStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    FNPC_TribalMemory TribalMemory;

    // Behavior settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float DangerDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float SocialInteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float ResourceSearchRadius;

    // AI References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* TribalBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBlackboardComponent* TribalBlackboard;

    // Tribal hierarchy
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Hierarchy")
    AActor* TribalChief;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Hierarchy")
    TArray<AActor*> TribeMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Hierarchy")
    FVector TribalSettlementCenter;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void SetTribalRole(ENPC_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void SetCurrentActivity(ENPC_TribalActivity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void UpdateTribalStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void ProcessTribalDecisions();

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void HandleDangerResponse(FVector DangerLocation);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void HandleResourceFound(FVector ResourceLocation);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void HandleSocialInteraction(AActor* OtherTribeMember);

    // Decision making
    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    ENPC_TribalActivity DetermineNextActivity();

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    FVector FindBestPatrolLocation();

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    AActor* FindNearestTribeMember();

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    bool ShouldFleeFromDanger();

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    bool ShouldSeekResources();

private:
    // Internal state
    float LastDecisionTime;
    float DecisionInterval;
    FVector LastKnownPosition;
    
    // Helper functions
    void InitializeTribalBehavior();
    void UpdateMemory(float DeltaTime);
    void ProcessRoleSpecificBehavior();
    void UpdateBlackboardValues();
};