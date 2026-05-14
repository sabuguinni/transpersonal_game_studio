#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Eng_ArchitecturalFramework.generated.h"

// ============================================================================
// ENGINE ARCHITECTURE FRAMEWORK - AGENT #2
// Defines the core architectural rules and systems that ALL agents must follow
// ============================================================================

UENUM(BlueprintType)
enum class EEng_SystemType : uint8
{
    Core,           // Essential systems (Physics, Rendering, Input)
    World,          // World generation, environment, terrain
    Character,      // Player and NPC character systems
    AI,             // Dinosaur AI, behavior trees, crowd simulation
    Gameplay,       // Combat, quests, survival mechanics
    Audio,          // Sound, music, voice systems
    VFX,            // Visual effects, particles, shaders
    Performance,    // Optimization, LOD, streaming
    Integration     // Build, QA, testing systems
};

UENUM(BlueprintType)
enum class EEng_PerformanceTier : uint8
{
    Critical,       // 60fps PC / 30fps Console - MANDATORY
    High,           // 45fps PC / 25fps Console - ACCEPTABLE
    Medium,         // 30fps PC / 20fps Console - WARNING
    Low             // <30fps PC / <20fps Console - BLOCKED
};

UENUM(BlueprintType)
enum class EEng_CompilationStatus : uint8
{
    Clean,          // Compiles with 0 errors, 0 warnings
    Warnings,       // Compiles with warnings only
    Errors,         // Has compilation errors
    Blocked         // Cannot compile - missing dependencies
};

USTRUCT(BlueprintType)
struct FEng_SystemRequirements
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Requirements")
    EEng_SystemType SystemType;

    UPROPERTY(BlueprintReadOnly, Category = "Requirements")
    TArray<FString> RequiredModules;

    UPROPERTY(BlueprintReadOnly, Category = "Requirements")
    TArray<FString> RequiredClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Requirements")
    EEng_PerformanceTier MinPerformanceTier;

    UPROPERTY(BlueprintReadOnly, Category = "Requirements")
    int32 MaxActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Requirements")
    float MaxMemoryMB;

    FEng_SystemRequirements()
    {
        SystemType = EEng_SystemType::Core;
        RequiredModules.Empty();
        RequiredClasses.Empty();
        MinPerformanceTier = EEng_PerformanceTier::Critical;
        MaxActorCount = 1000;
        MaxMemoryMB = 512.0f;
    }
};

USTRUCT(BlueprintType)
struct FEng_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    EEng_CompilationStatus CompilationStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 HeaderCount;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 SourceCount;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 OrphanedHeaders;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> MissingImplementations;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> CompilationErrors;

    FEng_ModuleStatus()
    {
        ModuleName = TEXT("");
        CompilationStatus = EEng_CompilationStatus::Blocked;
        HeaderCount = 0;
        SourceCount = 0;
        OrphanedHeaders = 0;
        MissingImplementations.Empty();
        CompilationErrors.Empty();
    }
};

USTRUCT(BlueprintType)
struct FEng_ArchitecturalRules
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Rules")
    bool bEnforceHeaderCppPairs;

    UPROPERTY(BlueprintReadOnly, Category = "Rules")
    bool bEnforcePerformanceLimits;

    UPROPERTY(BlueprintReadOnly, Category = "Rules")
    bool bEnforceModuleDependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Rules")
    bool bEnforceNamingConventions;

    UPROPERTY(BlueprintReadOnly, Category = "Rules")
    int32 MaxActorsPerSystem;

    UPROPERTY(BlueprintReadOnly, Category = "Rules")
    float MaxFrameTimeMS;

    FEng_ArchitecturalRules()
    {
        bEnforceHeaderCppPairs = true;
        bEnforcePerformanceLimits = true;
        bEnforceModuleDependencies = true;
        bEnforceNamingConventions = true;
        MaxActorsPerSystem = 1000;
        MaxFrameTimeMS = 16.67f; // 60fps target
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitecturalFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalFramework();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ========================================================================
    // ARCHITECTURAL VALIDATION
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ValidateModuleStructure();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_ModuleStatus GetModuleStatus(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_ModuleStatus> GetAllModuleStatuses() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemRequirements(EEng_SystemType SystemType) const;

    // ========================================================================
    // COMPILATION MANAGEMENT
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ScanForOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void GenerateMissingImplementations();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CheckCompilationStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void RunArchitecturalAudit();

    // ========================================================================
    // PERFORMANCE MONITORING
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void MonitorSystemPerformance();

    UFUNCTION(BlueprintPure, Category = "Engine Architecture")
    EEng_PerformanceTier GetCurrentPerformanceTier() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void EnforcePerformanceLimits();

    // ========================================================================
    // SYSTEM INTEGRATION
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystemRequirements(EEng_SystemType SystemType, const FEng_SystemRequirements& Requirements);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemIntegration(EEng_SystemType SystemA, EEng_SystemType SystemB) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeSystemDependencies();

    // ========================================================================
    // AGENT COORDINATION
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetArchitecturalRules(const FEng_ArchitecturalRules& Rules);

    UFUNCTION(BlueprintPure, Category = "Engine Architecture")
    FEng_ArchitecturalRules GetArchitecturalRules() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateAgentOutput(int32 AgentID, const TArray<FString>& CreatedFiles) const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TMap<EEng_SystemType, FEng_SystemRequirements> SystemRequirements;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TMap<FString, FEng_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    FEng_ArchitecturalRules CurrentRules;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    EEng_PerformanceTier CurrentPerformanceTier;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    bool bFrameworkInitialized;

private:
    void InitializeSystemRequirements();
    void ScanModuleDirectory(const FString& ModulePath);
    void ValidateHeaderCppPairs(const FString& ModulePath);
    void CheckSystemDependencies();
    void UpdatePerformanceMetrics();
};

#include "Eng_ArchitecturalFramework.generated.h"