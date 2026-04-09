// Copyright Transpersonal Game Studio. All Rights Reserved.
// TranspersonalCoreSubsystem.h - Master core subsystem that coordinates all other subsystems

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EngineArchitecture.h"
#include "TranspersonalCoreSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalCore, Log, All);

/**
 * TRANSPERSONAL CORE SUBSYSTEM
 * 
 * This is the master subsystem that coordinates all other subsystems in the game.
 * It ensures proper initialization order and manages inter-system communication.
 * 
 * RESPONSIBILITIES:
 * - Initialize all core systems in correct order
 * - Manage system dependencies
 * - Enforce architectural compliance
 * - Monitor performance across all systems
 * - Provide unified interface for all core functionality
 */

UENUM(BlueprintType)
enum class ETranspersonalSystemStatus : uint8
{
    Uninitialized,
    Initializing,
    Ready,
    Error,
    Disabled
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTranspersonalSystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    ETranspersonalSystemStatus Status = ETranspersonalSystemStatus::Uninitialized;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float InitializationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    TArray<FString> Dependencies;
};

UCLASS()
class TRANSPERSONALGAME_API UTranspersonalCoreSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Get subsystem instance
    UFUNCTION(BlueprintCallable, Category = "Core", CallInEditor = true)
    static UTranspersonalCoreSubsystem* Get(const UObject* WorldContext);

    // System management
    UFUNCTION(BlueprintCallable, Category = "Core")
    bool InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Core")
    void ShutdownAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Core")
    bool IsSystemReady(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Core")
    ETranspersonalSystemStatus GetSystemStatus(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Core")
    TArray<FTranspersonalSystemInfo> GetAllSystemsInfo() const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentMemoryUsage() const;

    // Architecture validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateArchitecturalCompliance() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetArchitecturalViolations() const;

    // World Partition management
    UFUNCTION(BlueprintCallable, Category = "World Partition")
    bool InitializeWorldPartition();

    UFUNCTION(BlueprintCallable, Category = "World Partition")
    bool IsWorldPartitionActive() const;

    // Nanite management
    UFUNCTION(BlueprintCallable, Category = "Nanite")
    bool InitializeNaniteSettings();

    UFUNCTION(BlueprintCallable, Category = "Nanite")
    bool IsNaniteEnabled() const;

    // Lumen management
    UFUNCTION(BlueprintCallable, Category = "Lumen")
    bool InitializeLumenSettings();

    UFUNCTION(BlueprintCallable, Category = "Lumen")
    bool IsLumenEnabled() const;

    // Virtual Shadow Maps management
    UFUNCTION(BlueprintCallable, Category = "VSM")
    bool InitializeVSMSettings();

    UFUNCTION(BlueprintCallable, Category = "VSM")
    bool IsVSMEnabled() const;

    // Mass AI management
    UFUNCTION(BlueprintCallable, Category = "Mass AI")
    bool InitializeMassAISettings();

    UFUNCTION(BlueprintCallable, Category = "Mass AI")
    bool IsMassAIEnabled() const;

    // Event delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemStatusChanged, const FString&, SystemName);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnSystemStatusChanged OnSystemStatusChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllSystemsReady);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAllSystemsReady OnAllSystemsReady;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceWarning, float, CurrentFrameTime);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPerformanceWarning OnPerformanceWarning;

protected:
    // System initialization order (CRITICAL - DO NOT CHANGE)
    void InitializeSystemsInOrder();
    bool InitializePhysicsCore();
    bool InitializeRenderingCore();
    bool InitializeAudioCore();
    bool InitializeAICore();
    bool InitializeGameplayCore();

    // System status tracking
    UPROPERTY()
    TMap<FString, FTranspersonalSystemInfo> SystemsStatus;

    // Performance monitoring
    UPROPERTY()
    float LastFrameTime = 0.0f;

    UPROPERTY()
    float LastMemoryUsage = 0.0f;

    // Architecture settings
    UPROPERTY()
    FTranspersonalWorldPartitionSettings WorldPartitionSettings;

    UPROPERTY()
    FTranspersonalNaniteSettings NaniteSettings;

    UPROPERTY()
    FTranspersonalLumenSettings LumenSettings;

    UPROPERTY()
    FTranspersonalVSMSettings VSMSettings;

    UPROPERTY()
    FTranspersonalMassAISettings MassAISettings;

    UPROPERTY()
    FTranspersonalPerformanceTargets PerformanceTargets;

    // Internal state
    UPROPERTY()
    bool bAllSystemsInitialized = false;

    UPROPERTY()
    bool bArchitecturalComplianceValidated = false;

    // Timers
    FTimerHandle PerformanceMonitoringTimer;
    FTimerHandle ArchitecturalValidationTimer;

    // Performance monitoring functions
    void MonitorPerformance();
    void ValidateArchitecture();
    void LogSystemStatus() const;

private:
    // Critical system dependencies
    static const TArray<FString> CoreSystemInitializationOrder;
    
    // Performance thresholds
    static constexpr float PerformanceWarningThreshold = 20.0f; // ms
    static constexpr float PerformanceMonitoringInterval = 1.0f; // seconds
    static constexpr float ArchitecturalValidationInterval = 5.0f; // seconds
};