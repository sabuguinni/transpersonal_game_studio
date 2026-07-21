#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorTypes.h"
#include "NPCDailyRoutineManager.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCDailyRoutineManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCDailyRoutineManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Routine management
    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void AddRoutineActivity(float StartTime, float EndTime, ENPC_BehaviorState Activity, const FVector& Location, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void RemoveRoutineActivity(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void ClearAllRoutines();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    FNPC_DailyRoutine GetCurrentActivity();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    FNPC_DailyRoutine GetNextActivity();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    bool HasActiveRoutine();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void SetTribalRole(ENPC_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void GenerateDefaultRoutine(ENPC_TribalRole Role);

    // Time management
    UFUNCTION(BlueprintCallable, Category = "Time")
    float GetCurrentGameTime();

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetTimeScale(float NewTimeScale);

    UFUNCTION(BlueprintCallable, Category = "Time")
    bool IsNightTime();

    UFUNCTION(BlueprintCallable, Category = "Time")
    bool IsDayTime();

    // Routine interruption
    UFUNCTION(BlueprintCallable, Category = "Routine Interruption")
    void InterruptRoutine(ENPC_BehaviorState NewActivity, float Duration = 60.0f);

    UFUNCTION(BlueprintCallable, Category = "Routine Interruption")
    void ResumeRoutine();

    UFUNCTION(BlueprintCallable, Category = "Routine Interruption")
    bool IsRoutineInterrupted();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine Settings")
    TArray<FNPC_DailyRoutine> DailyRoutines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine Settings")
    ENPC_TribalRole TribalRole = ENPC_TribalRole::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine Settings")
    float TimeScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine Settings")
    float DayLengthInMinutes = 20.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Routine State")
    int32 CurrentRoutineIndex = -1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Routine State")
    bool bRoutineInterrupted = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Routine State")
    ENPC_BehaviorState InterruptedActivity = ENPC_BehaviorState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Routine State")
    float InterruptionEndTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Routine State")
    FNPC_DailyRoutine SavedRoutine;

private:
    void UpdateCurrentRoutine();
    void SortRoutinesByTime();
    int32 FindCurrentRoutineIndex();
    void GenerateHunterRoutine();
    void GenerateGathererRoutine();
    void GenerateGuardRoutine();
    void GenerateLeaderRoutine();
    void GenerateShamanRoutine();
    void GenerateCrafterRoutine();
    void GenerateScoutRoutine();
    void GenerateElderRoutine();
};