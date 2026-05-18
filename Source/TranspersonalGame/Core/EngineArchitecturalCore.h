#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EngineArchitecturalCore.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEngineArchitect, Log, All);

/**
 * Core Engine Architectural System
 * Defines and enforces technical standards across all game systems
 * Manages system registration, validation, and performance monitoring
 * 
 * ARCHITECTURAL PRINCIPLES:
 * 1. Modular Design - Each system is self-contained with clear interfaces
 * 2. Performance First - 60fps PC / 30fps Console targets enforced
 * 3. Scalability - Support for 8km worlds and 50k AI agents
 * 4. Data-Driven - Configuration over hard-coding
 * 5. Fail-Safe - Graceful degradation when systems fail
 */

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical = 0,    // Core engine systems (Physics, Rendering)
    High = 1,        // Gameplay systems (Character, AI)
    Medium = 2,      // Quality of life (Audio, VFX)
    Low = 3,         // Optional features (Debug, Analytics)
    Background = 4   // Non-essential (Telemetry, Logging)
};

UENUM(BlueprintType)
enum class EEng_SystemStatus : uint8
{
    Uninitialized = 0,
    Initializing = 1,
    Running = 2,
    Degraded = 3,    // Running but with reduced functionality
    Failed = 4,
    ShuttingDown = 5
};

UENUM(BlueprintType)
enum class EEng_PerformanceTier : uint8
{
    Ultra = 0,       // High-end PC (RTX 4080+, 32GB+ RAM)
    High = 1,        // Mid-range PC (RTX 3070+, 16GB+ RAM)
    Medium = 2,      // Console equivalent (PS5/Xbox Series X)
    Low = 3,         // Console equivalent (PS5/Xbox Series S)
    Minimum = 4      // Lowest supported spec
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
    EEng_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float InitializationTime;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    int32 MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    bool bIsPerformanceCritical;

    FEng_SystemInfo()
    {
        SystemName = TEXT("Unknown");
        Priority = EEng_SystemPriority::Medium;
        Status = EEng_SystemStatus::Uninitialized;
        InitializationTime = 0.0f;
        LastUpdateTime = 0.0f;
        MemoryUsageMB = 0;
        bIsPerformanceCritical = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveAIAgents;

    FEng_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        FrameTimeMS = 0.0f;
        GameThreadTimeMS = 0.0f;
        RenderThreadTimeMS = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsageMB = 0;
        ActiveActors = 0;
        ActiveAIAgents = 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEng_OnSystemStatusChanged, FString, SystemName, EEng_SystemStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEng_OnPerformanceAlert, FString, AlertMessage);

/**
 * Engine Architectural Core Subsystem
 * Manages all game systems and enforces architectural standards
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitecturalCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitecturalCore();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority, bool bIsPerformanceCritical = false);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UpdateSystemStatus(const FString& SystemName, EEng_SystemStatus NewStatus);

    // System Query
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Engine Architecture")
    bool IsSystemRegistered(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Engine Architecture")
    EEng_SystemStatus GetSystemStatus(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Engine Architecture")
    TArray<FEng_SystemInfo> GetAllSystemInfo() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Engine Architecture")
    TArray<FString> GetSystemsByPriority(EEng_SystemPriority Priority) const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTier(EEng_PerformanceTier NewTier);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    EEng_PerformanceTier GetCurrentPerformanceTier() const { return CurrentPerformanceTier; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceMonitoring(bool bEnable);

    // Architectural Validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetArchitecturalViolations() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FEng_OnSystemStatusChanged OnSystemStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FEng_OnPerformanceAlert OnPerformanceAlert;

    // Static Access
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Engine Architecture", meta = (CallInEditor = true))
    static UEngineArchitecturalCore* GetEngineArchitecturalCore(const UObject* WorldContext);

protected:
    // System Management
    UPROPERTY()
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    UPROPERTY()
    EEng_PerformanceTier CurrentPerformanceTier;

    UPROPERTY()
    bool bPerformanceMonitoringEnabled;

    // Performance Tracking
    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    float PerformanceUpdateInterval;

    UPROPERTY()
    float LastPerformanceUpdate;

    // Architectural Rules
    TArray<FString> ArchitecturalViolations;

    // Internal Methods
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void ValidateSystemDependencies();
    void EnforceArchitecturalStandards();

    // Performance Thresholds
    static constexpr float TARGET_FPS_PC = 60.0f;
    static constexpr float TARGET_FPS_CONSOLE = 30.0f;
    static constexpr float CRITICAL_FRAME_TIME_MS = 33.33f; // 30 FPS
    static constexpr int32 MAX_MEMORY_USAGE_MB = 6144; // 6GB
    static constexpr int32 MAX_DRAW_CALLS = 5000;
    static constexpr int32 MAX_AI_AGENTS = 50000;
};

/**
 * Engine Architectural Standards
 * Static class that defines the technical standards all systems must follow
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEngineArchitecturalStandards : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // System Naming Standards
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Standards")
    static bool IsValidSystemName(const FString& SystemName);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Standards")
    static FString GenerateSystemName(const FString& Category, const FString& Function);

    // Performance Standards
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Standards")
    static float GetTargetFrameRate(EEng_PerformanceTier Tier);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Standards")
    static int32 GetMaxMemoryBudget(EEng_PerformanceTier Tier);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Standards")
    static int32 GetMaxDrawCalls(EEng_PerformanceTier Tier);

    // Architectural Rules
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Standards")
    static bool ValidateModuleDependency(const FString& FromModule, const FString& ToModule);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Standards")
    static TArray<FString> GetAllowedDependencies(const FString& ModuleName);

    // World Standards
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Standards")
    static bool RequiresWorldPartition(float WorldSizeKM);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Standards")
    static int32 GetRecommendedLODLevels(EEng_PerformanceTier Tier);
};

#include "EngineArchitecturalCore.generated.h"