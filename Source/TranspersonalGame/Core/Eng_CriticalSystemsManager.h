#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_CriticalSystemsManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEngCriticalSystems, Log, All);

/**
 * Critical system status tracking
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    bool bIsInitialized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    bool bIsRunning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    float LastUpdateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    int32 ErrorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    FString LastError;

    FEng_SystemStatus()
    {
        SystemName = TEXT("");
        bIsInitialized = false;
        bIsRunning = false;
        LastUpdateTime = 0.0f;
        ErrorCount = 0;
        LastError = TEXT("");
    }
};

/**
 * System dependency definition
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemDependency
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependencies")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependencies")
    TArray<FString> RequiredSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependencies")
    int32 InitializationPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dependencies")
    bool bIsCritical;

    FEng_SystemDependency()
    {
        SystemName = TEXT("");
        InitializationPriority = 0;
        bIsCritical = false;
    }
};

/**
 * Engine Architect Critical Systems Manager
 * Manages initialization order, dependencies, and health monitoring of all game systems
 */
UCLASS(ClassGroup=(EngineArchitect), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_CriticalSystemsManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CriticalSystemsManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // System registration and management
    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    bool RegisterSystem(const FString& SystemName, int32 Priority = 0, bool bIsCritical = false);

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    void AddSystemDependency(const FString& SystemName, const FString& RequiredSystem);

    // System initialization and shutdown
    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    void ShutdownAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    bool InitializeSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    bool ShutdownSystem(const FString& SystemName);

    // System status monitoring
    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    void UpdateSystemStatus(const FString& SystemName, bool bIsRunning, const FString& ErrorMessage = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    FEng_SystemStatus GetSystemStatus(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    TArray<FEng_SystemStatus> GetAllSystemStatuses() const;

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    bool IsSystemRunning(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    bool AreAllCriticalSystemsRunning() const;

    // System health monitoring
    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    void PerformSystemHealthCheck();

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    TArray<FString> GetFailedSystems() const;

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    int32 GetTotalErrorCount() const;

    // System restart and recovery
    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    bool RestartSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    void RestartFailedSystems();

    // Configuration and settings
    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    void SetSystemCheckInterval(float IntervalSeconds);

    UFUNCTION(BlueprintCallable, Category = "Critical Systems")
    void EnableSystemLogging(bool bEnable);

protected:
    // Core system tracking
    UPROPERTY()
    TMap<FString, FEng_SystemStatus> SystemStatuses;

    UPROPERTY()
    TMap<FString, FEng_SystemDependency> SystemDependencies;

    UPROPERTY()
    TArray<FString> InitializationOrder;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float SystemCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableSystemLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoRestartFailedSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxRestartAttempts;

    // Internal state
    UPROPERTY()
    bool bIsInitialized;

    UPROPERTY()
    float LastHealthCheckTime;

    UPROPERTY()
    TMap<FString, int32> RestartAttempts;

private:
    // Internal methods
    void CalculateInitializationOrder();
    bool CheckSystemDependencies(const FString& SystemName) const;
    void LogSystemEvent(const FString& SystemName, const FString& Event, const FString& Details = TEXT(""));
    void InitializeDefaultSystems();

    // Timer handle for periodic health checks
    FTimerHandle HealthCheckTimerHandle;
    void OnHealthCheckTimer();
};