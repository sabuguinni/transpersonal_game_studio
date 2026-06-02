#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Crowd_BehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ECrowd_ActivityState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Moving      UMETA(DisplayName = "Moving"),
    Working     UMETA(DisplayName = "Working"),
    Resting     UMETA(DisplayName = "Resting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Following   UMETA(DisplayName = "Following")
};

USTRUCT(BlueprintType)
struct FCrowd_BehaviorSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float RotationSpeed = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PersonalSpace = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ViewDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ReactionTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bAvoidObstacles = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bFollowLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float LeaderFollowDistance = 200.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_EmotionalState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Energy = 1.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_BehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_BehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FCrowd_BehaviorSettings BehaviorSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    ECrowd_ActivityState CurrentActivity = ECrowd_ActivityState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    FCrowd_EmotionalState EmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "Navigation")
    FVector CurrentDestination;

    UPROPERTY(BlueprintReadOnly, Category = "Navigation")
    FVector LastKnownPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    AActor* LeaderActor;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TimeSinceLastActivity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bCanSeePlayer = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float DistanceToPlayer = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetActivityState(ECrowd_ActivityState NewState);

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    void SetDestination(const FVector& NewDestination);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReactToPlayer(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReactToThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Emotion")
    void ModifyEmotionalState(float FearDelta, float CuriosityDelta, float AggressionDelta, float SociabilityDelta);

    UFUNCTION(BlueprintCallable, Category = "Group")
    TArray<AActor*> FindNearbyGroupMembers(float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    FVector CalculateFlockingMovement();

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    FVector CalculateObstacleAvoidance();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldFleeFromPlayer();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldApproachPlayer();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdatePlayerAwareness(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerEmotionalAnimation();

private:
    void UpdateActivityBehavior(float DeltaTime);
    void UpdateEmotionalDecay(float DeltaTime);
    void ProcessPlayerInteraction(float DeltaTime);
    void UpdateLeaderFollowing(float DeltaTime);
    
    float ActivityTimer = 0.0f;
    float EmotionDecayRate = 0.1f;
    float PlayerDetectionRadius = 1000.0f;
    float LastPlayerCheckTime = 0.0f;
    float PlayerCheckInterval = 0.2f;
};