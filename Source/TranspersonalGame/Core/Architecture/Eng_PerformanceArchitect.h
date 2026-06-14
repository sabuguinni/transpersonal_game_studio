#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_PerformanceArchitect.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisiblePrimitives = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    int32 MaxActors = 8000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    int32 MaxDinosaurs = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float MaxMemoryMB = 4096.0f;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_PerformanceArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnforcePerformanceBudgets();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceBudget(const FEng_PerformanceBudget& NewBudget);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ValidateCurrentPerformance();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FEng_PerformanceBudget PerformanceBudget;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEng_PerformanceMetrics CachedMetrics;

    FTimerHandle MetricsUpdateTimer;

    void UpdateMetrics();
    void EnforceDinosaurCap();
    void EnforceActorCap();
};