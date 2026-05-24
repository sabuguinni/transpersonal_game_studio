#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "UObject/NoExportTypes.h"
#include "../SharedTypes.h"
#include "Eng_ArchitecturalFramework.generated.h"

/**
 * Engine Architectural Framework - Core system that enforces architectural rules
 * across all game systems. Ensures consistency, performance, and maintainability.
 * 
 * This system is the technical foundation that all other agents must follow.
 * It defines module boundaries, communication protocols, and performance constraints.
 */

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical = 0,    // Core systems (Physics, Rendering, Input)
    High = 1,        // Gameplay systems (Character, AI, Combat)
    Medium = 2,      // Content systems (Environment, Audio, VFX)
    Low = 3,         // Utility systems (UI, Analytics, Debug)
    Background = 4   // Non-essential systems (Telemetry, Logging)
};

UENUM(BlueprintType)
enum class EEng_ModuleState : uint8
{
    Uninitialized = 0,
    Initializing = 1,
    Active = 2,
    Suspended = 3,
    Error = 4,
    Shutdown = 5
};

UENUM(BlueprintType)
enum class EEng_PerformanceProfile : uint8
{
    Development = 0,  // No performance constraints
    Testing = 1,      // Basic performance monitoring
    Shipping = 2      // Full performance enforcement
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TickingComponents;

    FEng_SystemMetrics()
    {
        FrameTime = 0.0f;
        CPUUsage = 0.0f;
        MemoryUsage = 0.0f;
        ActiveActors = 0;
        TickingComponents = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    EEng_SystemPriority Priority;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    EEng_ModuleState State;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    float InitializationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> Dependencies;

    FEng_ModuleInfo()
    {
        ModuleName = TEXT("Unknown");
        Priority = EEng_SystemPriority::Medium;
        State = EEng_ModuleState::Uninitialized;
        InitializationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitecturalRule
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Rule")
    FString RuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Rule")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "Rule")
    bool bMandatory;

    UPROPERTY(BlueprintReadOnly, Category = "Rule")
    float ViolationPenalty;

    FEng_ArchitecturalRule()
    {
        RuleName = TEXT("UnnamedRule");
        Description = TEXT("No description");
        bMandatory = true;
        ViolationPenalty = 1.0f;
    }
};

/**
 * Core architectural framework component that enforces system-wide rules
 * and monitors performance across all game modules.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_ArchitecturalFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalFramework();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core architectural enforcement
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeFramework();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RegisterModule(const FEng_ModuleInfo& ModuleInfo);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool UnregisterModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void EnforceArchitecturalRules();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_SystemMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceProfile(EEng_PerformanceProfile Profile);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinLimits() const;

    // Module management
    UFUNCTION(BlueprintCallable, Category = "Modules")
    TArray<FEng_ModuleInfo> GetRegisteredModules() const;

    UFUNCTION(BlueprintCallable, Category = "Modules")
    EEng_ModuleState GetModuleState(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Modules")
    bool SetModuleState(const FString& ModuleName, EEng_ModuleState NewState);

    // Rule management
    UFUNCTION(BlueprintCallable, Category = "Rules")
    void AddArchitecturalRule(const FEng_ArchitecturalRule& Rule);

    UFUNCTION(BlueprintCallable, Category = "Rules")
    void RemoveArchitecturalRule(const FString& RuleName);

    UFUNCTION(BlueprintCallable, Category = "Rules")
    TArray<FEng_ArchitecturalRule> GetActiveRules() const;

    // Editor utilities
    UFUNCTION(CallInEditor, Category = "Debug")
    void ValidateSystemArchitecture();

    UFUNCTION(CallInEditor, Category = "Debug")
    void GenerateArchitecturalReport();

    UFUNCTION(CallInEditor, Category = "Debug")
    void ResetFramework();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Framework")
    TArray<FEng_ModuleInfo> RegisteredModules;

    UPROPERTY(BlueprintReadOnly, Category = "Framework")
    TArray<FEng_ArchitecturalRule> ArchitecturalRules;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EEng_PerformanceProfile CurrentProfile;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEng_SystemMetrics LastMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Framework")
    bool bFrameworkInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MetricsUpdateInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastMetricsUpdate;

private:
    void UpdateSystemMetrics();
    void CheckPerformanceLimits();
    void ValidateModuleDependencies();
    bool IsModuleRegistered(const FString& ModuleName) const;
    void InitializeDefaultRules();
};

/**
 * Game Mode extension that integrates the architectural framework
 * into the core game loop and ensures all systems follow architectural rules.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_ArchitecturalGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AEng_ArchitecturalGameMode();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    UEng_ArchitecturalFramework* GetArchitecturalFramework() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsSystemArchitectureValid() const;

    UFUNCTION(CallInEditor, Category = "Debug")
    void ValidateAllSystems();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    UEng_ArchitecturalFramework* ArchitecturalFramework;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bArchitectureValidated;
};