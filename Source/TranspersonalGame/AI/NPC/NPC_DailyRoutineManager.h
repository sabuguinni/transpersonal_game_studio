#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "../../SharedTypes.h"
#include "NPC_DailyRoutineManager.generated.h"

UENUM(BlueprintType)
enum class ENPC_ActivityType : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Resting         UMETA(DisplayName = "Resting"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Exploring       UMETA(DisplayName = "Exploring")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_ScheduledActivity
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    ENPC_ActivityType ActivityType = ENPC_ActivityType::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    float StartTimeHours = 8.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    float DurationHours = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    float Priority = 1.0f; // 0-10, higher = more important

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    bool bCanBeInterrupted = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    float MovementSpeed = 300.0f;

    FNPC_ScheduledActivity()
    {
        ActivityType = ENPC_ActivityType::Idle;
        StartTimeHours = 8.0f;
        DurationHours = 2.0f;
        TargetLocation = FVector::ZeroVector;
        Priority = 1.0f;
        bCanBeInterrupted = true;
        MovementSpeed = 300.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DailyRoutineManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DailyRoutineManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    TArray<FNPC_ScheduledActivity> DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    FNPC_ScheduledActivity CurrentActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    bool bIsFollowingSchedule = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float ScheduleFlexibility = 0.5f; // How much activities can vary in time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float ActivityStartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    bool bActivityInProgress = false;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void SetupDefaultSchedule();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void AddScheduledActivity(ENPC_ActivityType ActivityType, float StartTime, float Duration, FVector Location, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    FNPC_ScheduledActivity GetCurrentActivity() const { return CurrentActivity; }

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    ENPC_ActivityType GetCurrentActivityType() const { return CurrentActivity.ActivityType; }

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    bool IsActivityInProgress() const { return bActivityInProgress; }

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void InterruptCurrentActivity();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void ResumeSchedule();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void ForceActivity(ENPC_ActivityType ActivityType, FVector Location, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    float GetCurrentTimeOfDay();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    FNPC_ScheduledActivity* FindNextActivity();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void CompleteCurrentActivity();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    bool CanInterruptCurrentActivity() const;

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void ClearSchedule();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    TArray<FNPC_ScheduledActivity> GetDailySchedule() const { return DailySchedule; }

private:
    void UpdateCurrentActivity();
    void StartActivity(const FNPC_ScheduledActivity& Activity);
    bool IsTimeForActivity(const FNPC_ScheduledActivity& Activity, float CurrentTime);
};