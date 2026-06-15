#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Eng_PerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EEng_PerformanceTarget : uint8
{
    PC_60FPS        UMETA(DisplayName = "PC 60 FPS"),
    Console_30FPS   UMETA(DisplayName = "Console 30 FPS"),
    Mobile_30FPS    UMETA(DisplayName = "Mobile 30 FPS")
};

USTRUCT(BlueprintType)
struct FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_PerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EEng_PerformanceTarget TargetPerformance = EEng_PerformanceTarget::PC_60FPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActors = 8000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDinosaurs = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryMB = 4096.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEng_PerformanceMetrics CurrentMetrics;

    FTimerHandle PerformanceTimerHandle;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(EEng_PerformanceTarget NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnforceActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForTarget();

private:
    void UpdatePerformanceMetrics();
    void ApplyPerformanceSettings();
};