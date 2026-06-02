#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "NPC_DailyRoutineComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_RoutineActivity
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float StartTime; // Hour of day (0-24)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float Duration; // Hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    ENPCBehaviorState ActivityType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    bool bCanBeInterrupted;

    FNPC_RoutineActivity()
    {
        StartTime = 0.0f;
        Duration = 1.0f;
        ActivityType = ENPCBehaviorState::Idle;
        TargetLocation = FVector::ZeroVector;
        Priority = 1.0f;
        bCanBeInterrupted = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DailyRoutineComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DailyRoutineComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    TArray<FNPC_RoutineActivity> DailyActivities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    FNPC_RoutineActivity* CurrentActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float RoutineFlexibility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    bool bUseRandomVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float RandomVariationRange;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void AddRoutineActivity(float StartTime, float Duration, ENPCBehaviorState ActivityType, FVector TargetLocation, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    FNPC_RoutineActivity* GetCurrentActivity();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    FNPC_RoutineActivity* GetNextActivity();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    bool ShouldSwitchActivity();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void InterruptCurrentActivity(ENPCBehaviorState NewActivity, float InterruptionDuration);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    float GetCurrentTimeOfDay();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void SetupBasicDinosaurRoutine(ESpeciesType SpeciesType);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void UpdateActivityBasedOnNeeds(float Hunger, float Thirst, float Energy);

private:
    FNPC_RoutineActivity* FindActivityForTime(float TimeOfDay);
    void SortActivitiesByTime();
    float GetRandomizedTime(float BaseTime);
};