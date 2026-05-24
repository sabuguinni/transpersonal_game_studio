#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "Eng_SystemArchitecture.generated.h"

// Forward declarations
class UEng_ModuleManager;
class UEng_PerformanceMonitor;

UENUM(BlueprintType)
enum class EEng_SystemType : uint8
{
    Core            UMETA(DisplayName = "Core Systems"),
    WorldGeneration UMETA(DisplayName = "World Generation"),
    Character       UMETA(DisplayName = "Character Systems"),
    AI              UMETA(DisplayName = "AI & Behavior"),
    Combat          UMETA(DisplayName = "Combat Systems"),
    Audio           UMETA(DisplayName = "Audio Systems"),
    VFX             UMETA(DisplayName = "Visual Effects"),
    Performance     UMETA(DisplayName = "Performance"),
    Integration     UMETA(DisplayName = "Integration")
};

UENUM(BlueprintType)
enum class EEng_SystemStatus : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Initializing    UMETA(DisplayName = "Initializing"),
    Active          UMETA(DisplayName = "Active"),
    Error           UMETA(DisplayName = "Error"),
    Shutdown        UMETA(DisplayName = "Shutdown")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    EEng_SystemType SystemType;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    EEng_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    FString ModulePath;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    float InitializationTime;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    FString LastError;

    FEng_SystemInfo()
    {
        SystemType = EEng_SystemType::Core;
        Status = EEng_SystemStatus::Inactive;
        SystemName = TEXT("Unknown");
        ModulePath = TEXT("");
        InitializationTime = 0.0f;
        LastUpdateTime = 0.0f;
        ErrorCount = 0;
        LastError = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActorCount;

    FEng_PerformanceMetrics()
    {
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
        CPUUsagePercent = 0.0f;
        GPUUsagePercent = 0.0f;
        ActiveActorCount = 0;
        VisibleActorCount = 0;
    }
};

/**
 * Central System Architecture Manager
 * Coordinates all game systems and ensures proper initialization order
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_SystemArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_SystemArchitecture();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Management
    UFUNCTION(BlueprintCallable, Category = "System Architecture")
    void RegisterSystem(EEng_SystemType SystemType, const FString& SystemName, const FString& ModulePath);

    UFUNCTION(BlueprintCallable, Category = "System Architecture")
    void UnregisterSystem(EEng_SystemType SystemType);

    UFUNCTION(BlueprintCallable, Category = "System Architecture")
    bool InitializeSystem(EEng_SystemType SystemType);

    UFUNCTION(BlueprintCallable, Category = "System Architecture")
    void ShutdownSystem(EEng_SystemType SystemType);

    UFUNCTION(BlueprintCallable, Category = "System Architecture")
    void ShutdownAllSystems();

    // System Status
    UFUNCTION(BlueprintCallable, Category = "System Architecture")
    EEng_SystemStatus GetSystemStatus(EEng_SystemType SystemType) const;

    UFUNCTION(BlueprintCallable, Category = "System Architecture")
    FEng_SystemInfo GetSystemInfo(EEng_SystemType SystemType) const;

    UFUNCTION(BlueprintCallable, Category = "System Architecture")
    TArray<FEng_SystemInfo> GetAllSystemInfo() const;

    UFUNCTION(BlueprintCallable, Category = "System Architecture")
    bool IsSystemActive(EEng_SystemType SystemType) const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    // System Dependencies
    UFUNCTION(BlueprintCallable, Category = "System Architecture")
    void SetSystemDependency(EEng_SystemType System, EEng_SystemType Dependency);

    UFUNCTION(BlueprintCallable, Category = "System Architecture")
    bool ValidateSystemDependencies() const;

    // Error Handling
    UFUNCTION(BlueprintCallable, Category = "System Architecture")
    void ReportSystemError(EEng_SystemType SystemType, const FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "System Architecture")
    void ClearSystemErrors(EEng_SystemType SystemType);

    // Debug and Diagnostics
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void PrintSystemStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void ValidateArchitecture() const;

protected:
    // System Registry
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TMap<EEng_SystemType, FEng_SystemInfo> RegisteredSystems;

    // System Dependencies
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TMap<EEng_SystemType, TArray<EEng_SystemType>> SystemDependencies;

    // Performance Data
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEng_PerformanceMetrics CurrentMetrics;

    // Timing
    UPROPERTY(BlueprintReadOnly, Category = "Internal")
    float LastMetricsUpdateTime;

private:
    // Internal system management
    void InitializeSystemDependencies();
    void UpdateSystemStatus(EEng_SystemType SystemType, EEng_SystemStatus NewStatus);
    bool CanInitializeSystem(EEng_SystemType SystemType) const;
    void LogSystemEvent(EEng_SystemType SystemType, const FString& Event) const;
};

/**
 * World-level Architecture Monitor
 * Monitors system health within a specific world/level
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_WorldArchitectureMonitor : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_WorldArchitectureMonitor();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // World-specific monitoring
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void MonitorWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    int32 GetActiveActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    int32 GetVisibleActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    float GetWorldPerformanceScore() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
    float MonitoringInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
    float LastMonitoringTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float WorldPerformanceScore;

private:
    void UpdateWorldMetrics();
    void CheckSystemHealth();
};