#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "Perf_MemoryManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_MemoryLevel : uint8
{
    Low         UMETA(DisplayName = "Low Memory"),
    Medium      UMETA(DisplayName = "Medium Memory"),
    High        UMETA(DisplayName = "High Memory"),
    Critical    UMETA(DisplayName = "Critical Memory")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float AvailableMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MemoryUsagePercent = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 ActiveActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 ActiveComponentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    EPerf_MemoryLevel MemoryLevel = EPerf_MemoryLevel::Low;
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_MemoryManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_MemoryManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Memory monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    FPerf_MemoryStats GetCurrentMemoryStats();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void OptimizeMemoryUsage();

    // Memory thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory", meta = (ClampMin = "50.0", ClampMax = "95.0"))
    float MemoryWarningThreshold = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory", meta = (ClampMin = "80.0", ClampMax = "99.0"))
    float MemoryCriticalThreshold = 90.0f;

    // Auto-optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    bool bAutoOptimizeMemory = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float MemoryCheckInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    bool bAutoGarbageCollection = true;

    // Memory optimization actions
    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void ClearUnusedAssets();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void ReduceActorLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void DisableNonEssentialComponents();

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryLevelChanged, EPerf_MemoryLevel, NewLevel);
    UPROPERTY(BlueprintAssignable, Category = "Performance|Memory")
    FOnMemoryLevelChanged OnMemoryLevelChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryOptimized, float, MemorySavedMB);
    UPROPERTY(BlueprintAssignable, Category = "Performance|Memory")
    FOnMemoryOptimized OnMemoryOptimized;

private:
    float LastMemoryCheck = 0.0f;
    EPerf_MemoryLevel CurrentMemoryLevel = EPerf_MemoryLevel::Low;
    
    void UpdateMemoryLevel();
    void HandleMemoryPressure();
    float GetSystemMemoryUsagePercent();
};