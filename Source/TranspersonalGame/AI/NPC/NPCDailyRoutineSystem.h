#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "NPCDailyRoutineSystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_ActivityType : uint8
{
    Sleeping,
    Foraging,
    Socializing,
    Crafting,
    Patrolling,
    Resting,
    Hunting,
    Gathering
};

USTRUCT(BlueprintType)
struct FNPC_ScheduledActivity
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPC_ActivityType ActivityType = ENPC_ActivityType::Resting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartHour = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EndHour = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFlexible = true;

    FNPC_ScheduledActivity()
    {
        ActivityType = ENPC_ActivityType::Resting;
        StartHour = 8.0f;
        EndHour = 10.0f;
        TargetLocation = FVector::ZeroVector;
        Priority = 1.0f;
        bIsFlexible = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DailyRoutineSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DailyRoutineSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    TArray<FNPC_ScheduledActivity> DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    ENPC_ActivityType CurrentActivity = ENPC_ActivityType::Resting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float ActivityTransitionTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    bool bUseGameTimeOfDay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float SimulatedTimeScale = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Daily Routine")
    float CurrentGameHour = 8.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Daily Routine")
    bool bIsTransitioning = false;

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void SetCurrentActivity(ENPC_ActivityType NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    ENPC_ActivityType GetScheduledActivityForTime(float GameHour);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void AddScheduledActivity(const FNPC_ScheduledActivity& NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void RemoveScheduledActivity(int32 ActivityIndex);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    FVector GetCurrentTargetLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    bool ShouldTransitionToNewActivity();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void ForceActivityTransition(ENPC_ActivityType NewActivity);

private:
    void UpdateCurrentGameTime();
    void CheckForActivityTransition();
    void ExecuteActivityTransition(ENPC_ActivityType NewActivity);
    
    FTimerHandle ActivityTransitionTimer;
    FNPC_ScheduledActivity* CurrentScheduledActivity;
    float LastActivityCheckTime;
};