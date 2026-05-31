#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "../../Shared/SharedTypes.h"
#include "NPC_BaseBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Interacting UMETA(DisplayName = "Interacting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Working     UMETA(DisplayName = "Working")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastPlayerInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bPlayerIsHostile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float FearLevel;

    FNPC_BehaviorMemory()
    {
        LastPlayerInteractionTime = 0.0f;
        LastKnownPlayerLocation = FVector::ZeroVector;
        bPlayerIsHostile = false;
        FearLevel = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BaseBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BaseBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateChangeTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float WanderRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PlayerDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FNPC_BehaviorMemory BehaviorMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    bool bHasDailyRoutine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float DailyRoutineStartHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float DailyRoutineEndHour;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool IsPlayerNearby();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReactToPlayer(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateMemory(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Routine")
    void ExecuteDailyRoutine();

protected:
    UFUNCTION()
    void OnStateTimerExpired();

    FVector InitialLocation;
    FTimerHandle StateTimerHandle;
    AActor* CachedPlayerActor;
};