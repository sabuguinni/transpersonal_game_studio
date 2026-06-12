#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "NPCDailyRoutine.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_RoutineActivity
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    ENPC_ActivityType ActivityType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    float StartHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    bool bCanBeInterrupted;

    FNPC_RoutineActivity()
    {
        ActivityType = ENPC_ActivityType::Idle;
        StartHour = 8.0f;
        Duration = 1.0f;
        TargetLocation = FVector::ZeroVector;
        Priority = 1.0f;
        bCanBeInterrupted = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DailyRoutine : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DailyRoutine();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FNPC_RoutineActivity> DailyActivities;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FNPC_RoutineActivity CurrentActivity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    int32 CurrentActivityIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float TimeScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bUseGameTimeOfDay;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Routine")
    void AddActivity(ENPC_ActivityType Type, float StartHour, float Duration, FVector Location, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Routine")
    void RemoveActivity(int32 ActivityIndex);

    UFUNCTION(BlueprintCallable, Category = "Routine")
    FNPC_RoutineActivity GetCurrentActivity();

    UFUNCTION(BlueprintCallable, Category = "Routine")
    FNPC_RoutineActivity GetNextActivity();

    UFUNCTION(BlueprintCallable, Category = "Routine")
    void InterruptCurrentActivity(ENPC_ActivityType NewActivity, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Routine")
    bool CanInterruptCurrentActivity();

    UFUNCTION(BlueprintCallable, Category = "Routine")
    void ResetRoutine();

    UFUNCTION(BlueprintCallable, Category = "Routine")
    float GetCurrentGameHour();

private:
    void UpdateCurrentActivity();
    int32 FindActivityForTime(float GameHour);
};