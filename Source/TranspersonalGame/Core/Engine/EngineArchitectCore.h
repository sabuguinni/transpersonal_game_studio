#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "EngineArchitectCore.generated.h"

class UTranspersonalGameInstance;
class ATranspersonalGameMode;
class ATranspersonalCharacter;

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical = 0,
    High = 1,
    Medium = 2,
    Low = 3
};

UENUM(BlueprintType)
enum class EEng_ModuleState : uint8
{
    Uninitialized = 0,
    Initializing = 1,
    Active = 2,
    Error = 3,
    Shutdown = 4
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EEng_SystemPriority Priority;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EEng_ModuleState State;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    int32 ErrorCount;

    FEng_SystemInfo()
    {
        SystemName = TEXT("Unknown");
        Priority = EEng_SystemPriority::Medium;
        State = EEng_ModuleState::Uninitialized;
        LastUpdateTime = 0.0f;
        ErrorCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ComponentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    FEng_PerformanceMetrics()
    {
        FrameRate = 0.0f;
        FrameTime = 0.0f;
        ActorCount = 0;
        ComponentCount = 0;
        MemoryUsageMB = 0.0f;
    }
};

/**
 * Engine Architect Core System
 * Manages the technical architecture and system coordination for the entire game
 * Validates module dependencies, performance metrics, and system integrity
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectCore();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemRegistered(const FString& SystemName) const;

    // System State Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetSystemState(const FString& SystemName, EEng_ModuleState NewState);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    EEng_ModuleState GetSystemState(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_SystemInfo> GetAllSystemInfo() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UpdatePerformanceMetrics();

    // Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetSystemErrors() const;

    // Debug
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void LogSystemStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void ForceSystemValidation();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    float PerformanceUpdateInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    float MaxAllowedFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Configuration")
    int32 MaxActorCount;

private:
    FTimerHandle PerformanceTimerHandle;

    void InitializeDefaultSystems();
    void ValidateCoreSystems();
    void CheckPerformanceThresholds();
    void LogError(const FString& ErrorMessage);
};

/**
 * Engine Architect Component
 * Can be attached to actors that need direct access to engine architecture data
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEngineArchitectComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEngineArchitectComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    UEngineArchitectCore* GetEngineCore() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemHealthy(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetCurrentFrameRate() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
    bool bMonitorPerformance;

    UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
    float MonitoringInterval;

private:
    float LastMonitoringTime;
    UEngineArchitectCore* CachedEngineCore;
};