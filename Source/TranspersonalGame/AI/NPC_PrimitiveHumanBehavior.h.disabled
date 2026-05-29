#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "NPC_PrimitiveHumanBehavior.generated.h"

class ADinosaurBase;
class ATranspersonalCharacter;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float StartTime; // 0.0 = midnight, 12.0 = noon

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    FString ActivityName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float MovementSpeed;

    FNPC_DailyRoutine()
    {
        StartTime = 8.0f;
        EndTime = 18.0f;
        TargetLocation = FVector::ZeroVector;
        ActivityName = TEXT("Gathering");
        MovementSpeed = 200.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SurvivalNeeds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fatigue; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Warmth; // 0-100

    FNPC_SurvivalNeeds()
    {
        Hunger = 50.0f;
        Thirst = 50.0f;
        Fatigue = 30.0f;
        Fear = 10.0f;
        Warmth = 70.0f;
    }
};

UENUM(BlueprintType)
enum class ENPC_PrimitiveBehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Gathering UMETA(DisplayName = "Gathering"),
    Hunting UMETA(DisplayName = "Hunting"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Resting UMETA(DisplayName = "Resting"),
    Socializing UMETA(DisplayName = "Socializing"),
    Crafting UMETA(DisplayName = "Crafting"),
    Patrolling UMETA(DisplayName = "Patrolling")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_PrimitiveHumanBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_PrimitiveHumanBehavior();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core behavior state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_PrimitiveBehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FNPC_SurvivalNeeds SurvivalNeeds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<FNPC_DailyRoutine> DailyRoutines;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SocialRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float GatheringRange;

    // Timers and intervals
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float BehaviorUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SurvivalNeedsDecayRate;

    // Memory and awareness
    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    TArray<AActor*> NearbyDinosaurs;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    TArray<AActor*> NearbyHumans;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    FVector CurrentDestination;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateBehaviorState();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ScanForThreats();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ScanForResources();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ScanForSocialTargets();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ExecuteCurrentBehavior();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPC_PrimitiveBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void FleeFromDanger(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void MoveToLocation(FVector TargetLocation, float Speed = 200.0f);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateSurvivalNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ExecuteDailyRoutine();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    float GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool IsNightTime() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReactToPlayer(ATranspersonalCharacter* Player);

private:
    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle SurvivalNeedsTimer;
    
    float LastBehaviorChangeTime;
    float BehaviorStateDuration;
    
    void InitializeBehavior();
    void UpdateNearbyActors();
    bool IsActorDangerous(AActor* Actor) const;
    FVector FindSafeLocation() const;
    FVector FindGatheringLocation() const;
    void HandleIdleBehavior();
    void HandleGatheringBehavior();
    void HandleFleeingBehavior();
    void HandleRestingBehavior();
    void HandleSocializingBehavior();
};