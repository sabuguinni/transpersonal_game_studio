// Copyright Transpersonal Game Studio. All Rights Reserved.
// StudioDirector.h - Studio Director system for coordinating all game systems

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EngineArchitecture.h"
#include "StudioDirector.generated.h"

/**
 * STUDIO DIRECTOR SYSTEM
 * 
 * This system coordinates all game modules and ensures they work together
 * according to the creative vision. Acts as the central command system.
 * 
 * RESPONSIBILITIES:
 * - Coordinate between all 18 agent systems
 * - Enforce creative vision consistency
 * - Manage system priorities and dependencies
 * - Monitor overall game state and health
 */

UENUM(BlueprintType)
enum class EStudioDirectorPriority : uint8
{
    Critical    = 0,    // Core systems (Physics, Performance)
    High        = 1,    // Gameplay systems (AI, Combat)
    Medium      = 2,    // Content systems (Environment, Lighting)
    Low         = 3     // Polish systems (VFX, Audio)
};

UENUM(BlueprintType)
enum class ESystemStatus : uint8
{
    Offline     = 0,    // System not initialized
    Initializing = 1,   // System starting up
    Online      = 2,    // System running normally
    Warning     = 3,    // System has issues but functional
    Error       = 4,    // System has critical errors
    Disabled    = 5     // System intentionally disabled
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSystemInfo
{
    GENERATED_BODY()

    // System identification
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System")
    int32 AgentNumber;

    // System status
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
    ESystemStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
    EStudioDirectorPriority Priority;

    // Performance metrics
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float LastUpdateTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MemoryUsage;

    // Dependencies
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dependencies")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dependencies")
    TArray<FString> Dependents;

    FSystemInfo()
    {
        SystemName = TEXT("Unknown");
        AgentName = TEXT("Unknown");
        AgentNumber = 0;
        Status = ESystemStatus::Offline;
        Priority = EStudioDirectorPriority::Medium;
        LastUpdateTime = 0.0f;
        AverageFrameTime = 0.0f;
        MemoryUsage = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCreativeVisionSettings
{
    GENERATED_BODY()

    // Core vision parameters from B1 document
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vision")
    bool bMaintainConstantThreat = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vision")
    bool bDinosaursLiveIndependently = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vision")
    bool bEnableDomestication = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vision")
    bool bUniqueDinosaurVariations = true;

    // Emotional targets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emotion")
    float ThreatLevel = 0.8f; // 0.0 = safe, 1.0 = maximum threat

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emotion")
    float WonderLevel = 0.7f; // 0.0 = mundane, 1.0 = awe-inspiring

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emotion")
    float IsolationLevel = 0.6f; // 0.0 = crowded, 1.0 = completely alone
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSystemStatusChanged, const FString&, SystemName, ESystemStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreativeVisionViolation, const FString&, ViolationDescription);

/**
 * STUDIO DIRECTOR SUBSYSTEM
 * 
 * Central coordination system for all game modules
 */
UCLASS()
class TRANSPERSONALGAME_API UStudioDirectorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // System registration and management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RegisterSystem(const FSystemInfo& SystemInfo);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateSystemStatus(const FString& SystemName, ESystemStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    ESystemStatus GetSystemStatus(const FString& SystemName) const;

    // System coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanSystemStart(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RequestSystemShutdown(const FString& SystemName, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetSystemDependencies(const FString& SystemName) const;

    // Creative vision enforcement
    UFUNCTION(BlueprintCallable, Category = "Creative Vision")
    bool IsCreativeVisionCompliant() const;

    UFUNCTION(BlueprintCallable, Category = "Creative Vision")
    void ReportCreativeVisionViolation(const FString& ViolationDescription);

    UFUNCTION(BlueprintCallable, Category = "Creative Vision")
    FCreativeVisionSettings GetCreativeVisionSettings() const { return CreativeVisionSettings; }

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsOverallPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FString> GetPerformanceBottlenecks() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeSystemPriorities();

    // System queries
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FSystemInfo> GetAllSystems() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetSystemsByPriority(EStudioDirectorPriority Priority) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetSystemsByStatus(ESystemStatus Status) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnSystemStatusChanged OnSystemStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCreativeVisionViolation OnCreativeVisionViolation;

protected:
    // System registry
    UPROPERTY()
    TMap<FString, FSystemInfo> RegisteredSystems;

    // Creative vision settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    FCreativeVisionSettings CreativeVisionSettings;

    // Performance tracking
    UPROPERTY()
    float LastPerformanceCheck;

    UPROPERTY()
    TArray<FString> CurrentBottlenecks;

private:
    // Internal management
    void InitializeCoreSystems();
    void ValidateSystemDependencies();
    void MonitorSystemHealth();
    void EnforceCreativeVision();
    
    // System startup sequence
    void StartSystemsInOrder();
    bool AreSystemDependenciesMet(const FString& SystemName) const;
    
    // Performance analysis
    void AnalyzePerformanceMetrics();
    void RebalanceSystemPriorities();
    
    // Creative vision validation
    void ValidateThreatLevel();
    void ValidateDinosaurBehavior();
    void ValidatePlayerExperience();

    // Constants
    static constexpr float PERFORMANCE_CHECK_INTERVAL = 1.0f;
    static constexpr float SYSTEM_HEALTH_CHECK_INTERVAL = 0.5f;
    static constexpr float CREATIVE_VISION_CHECK_INTERVAL = 2.0f;
};

/**
 * STUDIO DIRECTOR COMPONENT
 * 
 * Component that can be added to actors to interface with the Studio Director
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UStudioDirectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorComponent();

    // Component interface
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // System interface
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RegisterWithDirector(const FString& SystemName, EStudioDirectorPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ReportSystemStatus(ESystemStatus Status);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ReportPerformanceMetrics(float FrameTime, int32 MemoryUsage);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    EStudioDirectorPriority SystemPriority;

    UPROPERTY()
    UStudioDirectorSubsystem* DirectorSubsystem;

private:
    void CacheDirectorSubsystem();
};