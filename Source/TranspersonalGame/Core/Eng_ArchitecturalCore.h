#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ArchitecturalCore.generated.h"

// Forward declarations
class UEng_SystemsRegistry;
class UBiomeManager;
class UCore_PhysicsManager;

/**
 * ARCHITECTURAL CORE SYSTEM
 * Central hub for all engine architecture management
 * Ensures proper initialization order and system dependencies
 * Provides unified access point for all architectural systems
 */

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical = 0,    // Physics, Core Systems
    High = 1,        // World Generation, Character Systems
    Medium = 2,      // AI, Combat, Audio
    Low = 3,         // UI, VFX, Analytics
    Background = 4   // Logging, Debugging
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    EEng_SystemPriority Priority;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    bool bIsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    bool bIsActive;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    float InitializationTime;

    FEng_SystemInfo()
    {
        SystemName = TEXT("Unknown");
        Priority = EEng_SystemPriority::Medium;
        bIsInitialized = false;
        bIsActive = false;
        InitializationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitecturalConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Config")
    bool bEnablePerformanceMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Config")
    bool bEnableSystemValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Config")
    bool bEnableDebugLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Config")
    float SystemTimeoutThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture Config")
    int32 MaxConcurrentSystems;

    FEng_ArchitecturalConfig()
    {
        bEnablePerformanceMonitoring = true;
        bEnableSystemValidation = true;
        bEnableDebugLogging = false;
        SystemTimeoutThreshold = 5.0f;
        MaxConcurrentSystems = 32;
    }
};

/**
 * Game Instance Subsystem for architectural core management
 * Persists across level changes and manages global architecture state
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitecturalCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalCore();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core architectural functions
    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool InitializeArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool IsSystemRegistered(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool IsSystemInitialized(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    TArray<FEng_SystemInfo> GetAllSystemInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    FEng_SystemInfo GetSystemInfo(const FString& SystemName) const;

    // System lifecycle management
    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool StartSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool StopSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool RestartSystem(const FString& SystemName);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    float GetSystemPerformanceMetric(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    void EnablePerformanceMonitoring(bool bEnable);

    // Configuration management
    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    void SetArchitecturalConfig(const FEng_ArchitecturalConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    FEng_ArchitecturalConfig GetArchitecturalConfig() const;

    // System validation
    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    bool ValidateSystemDependencies();

    // Debug and diagnostics
    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    void PrintSystemStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Architectural Core")
    void GenerateArchitecturalReport() const;

    // Static access
    UFUNCTION(BlueprintCallable, Category = "Architectural Core", meta = (CallInEditor = true))
    static UEng_ArchitecturalCore* GetArchitecturalCore(const UObject* WorldContext);

protected:
    // Internal system management
    void InitializeSystemsByPriority();
    void ShutdownAllSystems();
    bool ValidateSystemConfiguration() const;
    void UpdateSystemMetrics();

    // System registry
    UPROPERTY()
    TMap<FString, FEng_SystemInfo> RegisteredSystems;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    FEng_ArchitecturalConfig ArchitecturalConfig;

    // Performance tracking
    UPROPERTY()
    TMap<FString, float> SystemPerformanceMetrics;

    // Initialization state
    UPROPERTY(BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    bool bIsArchitectureInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    float TotalInitializationTime;

    // System references (cached for performance)
    UPROPERTY()
    TWeakObjectPtr<UEng_SystemsRegistry> SystemsRegistry;

    UPROPERTY()
    TWeakObjectPtr<UBiomeManager> BiomeManager;

    UPROPERTY()
    TWeakObjectPtr<UCore_PhysicsManager> PhysicsManager;

private:
    // Internal helpers
    void LogSystemEvent(const FString& SystemName, const FString& Event) const;
    bool IsValidSystemName(const FString& SystemName) const;
    void CleanupInvalidSystems();

    // Performance monitoring timer
    FTimerHandle PerformanceMonitoringTimer;
    void OnPerformanceMonitoringTick();

    // Validation timer
    FTimerHandle ValidationTimer;
    void OnValidationTick();
};

/**
 * World Subsystem for per-level architectural management
 * Handles level-specific architecture concerns
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_ArchitecturalWorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalWorldSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // World-specific architectural functions
    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool InitializeWorldArchitecture();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool ValidateWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void CleanupWorldSystems();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    bool bIsWorldArchitectureInitialized;

    // Reference to global architectural core
    UPROPERTY()
    TWeakObjectPtr<UEng_ArchitecturalCore> ArchitecturalCore;
};

#include "Eng_ArchitecturalCore.generated.h"