#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "UObject/NoExportTypes.h"
#include "Eng_ArchitecturalFramework.generated.h"

// Engine Architect System Types - Global Classification
UENUM(BlueprintType)
enum class EEng_SystemType : uint8
{
    Core            UMETA(DisplayName = "Core Systems"),
    World           UMETA(DisplayName = "World Generation"),
    Character       UMETA(DisplayName = "Character Systems"),
    AI              UMETA(DisplayName = "AI & Behavior"),
    Gameplay        UMETA(DisplayName = "Gameplay Mechanics"),
    Audio           UMETA(DisplayName = "Audio Systems"),
    VFX             UMETA(DisplayName = "Visual Effects"),
    Performance     UMETA(DisplayName = "Performance & Optimization"),
    Integration     UMETA(DisplayName = "Integration & Build")
};

// Performance Monitoring Tiers
UENUM(BlueprintType)
enum class EEng_PerformanceTier : uint8
{
    Critical        UMETA(DisplayName = "Critical - Must be 60fps"),
    High            UMETA(DisplayName = "High - Target 60fps"),
    Medium          UMETA(DisplayName = "Medium - Target 30fps"),
    Low             UMETA(DisplayName = "Low - Background processing")
};

// Compilation Status Tracking
UENUM(BlueprintType)
enum class EEng_CompilationStatus : uint8
{
    NotCompiled     UMETA(DisplayName = "Not Compiled"),
    Compiling       UMETA(DisplayName = "Compiling"),
    Success         UMETA(DisplayName = "Compilation Success"),
    Failed          UMETA(DisplayName = "Compilation Failed"),
    Warning         UMETA(DisplayName = "Compiled with Warnings")
};

// Module Dependency Levels
UENUM(BlueprintType)
enum class EEng_DependencyLevel : uint8
{
    Core            UMETA(DisplayName = "Core - No dependencies"),
    Foundation      UMETA(DisplayName = "Foundation - Core only"),
    System          UMETA(DisplayName = "System - Foundation + Core"),
    Gameplay        UMETA(DisplayName = "Gameplay - All lower levels"),
    Integration     UMETA(DisplayName = "Integration - All systems")
};

// System Requirements Definition
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemRequirements
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Requirements")
    EEng_SystemType SystemType = EEng_SystemType::Core;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Requirements")
    EEng_PerformanceTier PerformanceTier = EEng_PerformanceTier::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Requirements")
    EEng_DependencyLevel DependencyLevel = EEng_DependencyLevel::Core;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Requirements")
    TArray<FString> RequiredModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Requirements")
    int32 MaxActorsPerSystem = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Requirements")
    float TargetFrameTime = 16.67f; // 60fps = 16.67ms

    FEng_SystemRequirements()
    {
        RequiredModules.Add(TEXT("Core"));
        RequiredModules.Add(TEXT("Engine"));
        RequiredModules.Add(TEXT("TranspersonalGame"));
    }
};

// Module Compilation Status
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Status")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Status")
    EEng_CompilationStatus CompilationStatus = EEng_CompilationStatus::NotCompiled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Status")
    FString LastError;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Status")
    float CompilationTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Status")
    int32 ClassCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Status")
    FDateTime LastCompiled;
};

// Agent System Integration Rules
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_AgentIntegrationRules
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Rules")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Rules")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Rules")
    EEng_SystemType PrimarySystemType = EEng_SystemType::Core;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Rules")
    TArray<EEng_SystemType> AllowedSystemTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Rules")
    int32 MaxFilesPerCycle = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Rules")
    bool bRequiresCppImplementation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Rules")
    TArray<FString> MandatoryIncludes;
};

/**
 * Engine Architectural Framework - Central authority for all technical architecture decisions
 * This system enforces compilation rules, module dependencies, and agent coordination
 * Created by Agent #2 (Engine Architect) - Cycle 010
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitecturalFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalFramework();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration and Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterSystem(const FString& SystemName, const FEng_SystemRequirements& Requirements);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemDependencies(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    EEng_CompilationStatus CheckModuleCompilation(const FString& ModuleName);

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateAgentOutput(int32 AgentID, const TArray<FString>& CreatedFiles);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_AgentIntegrationRules GetAgentRules(int32 AgentID);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CheckPerformanceCompliance(const FString& SystemName, float CurrentFrameTime);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void LogPerformanceMetrics(const FString& SystemName, float FrameTime, int32 ActorCount);

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_ModuleStatus> GetAllModuleStatus();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ForceModuleRecompilation(const FString& ModuleName);

    // Architectural Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    void ValidateEntireArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    void GenerateArchitectureReport();

protected:
    // System Registry
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture State")
    TMap<FString, FEng_SystemRequirements> RegisteredSystems;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture State")
    TMap<FString, FEng_ModuleStatus> ModuleStatusMap;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture State")
    TMap<int32, FEng_AgentIntegrationRules> AgentRulesMap;

    // Performance Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    TMap<FString, float> SystemPerformanceMap;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float GlobalFrameTimeTarget = 16.67f; // 60fps

private:
    void InitializeAgentRules();
    void InitializeDefaultSystems();
    bool ValidateFileStructure(const FString& FilePath);
    void LogArchitecturalViolation(const FString& Violation);
};