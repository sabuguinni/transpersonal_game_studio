#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Eng_ArchitecturalFramework.generated.h"

/**
 * ENGINE ARCHITECT FRAMEWORK - CYCLE 004
 * Defines the core architectural rules and standards for all 19 agents
 * Establishes module dependencies, communication protocols, and performance budgets
 */

UENUM(BlueprintType)
enum class EEng_ModulePriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),      // Core systems that must load first
    High        UMETA(DisplayName = "High"),         // Essential gameplay systems
    Medium      UMETA(DisplayName = "Medium"),       // Standard features
    Low         UMETA(DisplayName = "Low"),          // Optional enhancements
    Debug       UMETA(DisplayName = "Debug")         // Development tools only
};

UENUM(BlueprintType)
enum class EEng_AgentType : uint8
{
    Technical       UMETA(DisplayName = "Technical"),      // Agents #2-4: Architecture, Core, Performance
    WorldBuilding   UMETA(DisplayName = "WorldBuilding"), // Agents #5-8: World, Environment, Architecture, Lighting
    Character       UMETA(DisplayName = "Character"),      // Agents #9-10: Character, Animation
    AI              UMETA(DisplayName = "AI"),             // Agents #11-13: NPC, Combat, Crowd
    Content         UMETA(DisplayName = "Content"),        // Agents #14-16: Quest, Narrative, Audio
    Production      UMETA(DisplayName = "Production"),     // Agents #17-19: VFX, QA, Integration
    Management      UMETA(DisplayName = "Management")      // Agent #1: Studio Director
};

UENUM(BlueprintType)
enum class EEng_PerformanceTier : uint8
{
    Ultra       UMETA(DisplayName = "Ultra"),       // 60fps+ guaranteed
    High        UMETA(DisplayName = "High"),        // 45-60fps target
    Medium      UMETA(DisplayName = "Medium"),      // 30-45fps acceptable
    Low         UMETA(DisplayName = "Low"),         // 30fps minimum
    Emergency   UMETA(DisplayName = "Emergency")    // Performance critical fallback
};

USTRUCT(BlueprintType)
struct FEng_ModuleSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    EEng_ModulePriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    EEng_AgentType OwnerAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    float MemoryBudgetMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    float CPUBudgetMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    bool bRequiresUE5Compilation;

    FEng_ModuleSpec()
    {
        ModuleName = TEXT("");
        Priority = EEng_ModulePriority::Medium;
        OwnerAgent = EEng_AgentType::Technical;
        MemoryBudgetMB = 10.0f;
        CPUBudgetMS = 1.0f;
        bRequiresUE5Compilation = true;
    }
};

USTRUCT(BlueprintType)
struct FEng_PerformanceBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EEng_PerformanceTier TargetTier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTrianglesPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxCPUTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls;

    FEng_PerformanceBudget()
    {
        TargetTier = EEng_PerformanceTier::High;
        MaxActorsPerFrame = 1000;
        MaxTrianglesPerFrame = 2000000;
        MaxMemoryUsageMB = 512.0f;
        MaxCPUTimeMS = 16.67f; // 60fps target
        MaxDrawCalls = 500;
    }
};

USTRUCT(BlueprintType)
struct FEng_AssetPipelineRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    FString AssetType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    FString RequiredPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    FString NamingConvention;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    int32 MaxTextureSizeX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    int32 MaxTextureSizeY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    bool bRequiresLODChain;

    FEng_AssetPipelineRule()
    {
        AssetType = TEXT("StaticMesh");
        RequiredPath = TEXT("/Game/Assets/");
        NamingConvention = TEXT("SM_");
        MaxTextureSizeX = 2048;
        MaxTextureSizeY = 2048;
        bRequiresLODChain = true;
    }
};

/**
 * Engine Architectural Framework Component
 * Enforces architectural standards across all 19 agents
 * Manages module dependencies and performance budgets
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitecturalFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Module Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FEng_ModuleSpec> RegisteredModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TMap<FString, bool> ModuleLoadStatus;

    // Performance Budgets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FEng_PerformanceBudget GlobalBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TMap<EEng_AgentType, FEng_PerformanceBudget> AgentBudgets;

    // Asset Pipeline Rules
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<FEng_AssetPipelineRule> AssetRules;

    // Compilation Rules
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    bool bEnforceStrictCompilation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    TArray<FString> RequiredHeaderIncludes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    TArray<FString> ForbiddenAPIs;

    // Module Registration Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterModule(const FEng_ModuleSpec& ModuleSpec);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateModuleDependencies(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeCoreModules();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceBudget(EEng_AgentType AgentType, const FEng_PerformanceBudget& Budget);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool CheckPerformanceCompliance(EEng_AgentType AgentType);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnforcePerformanceLimits();

    // Asset Pipeline Functions
    UFUNCTION(BlueprintCallable, Category = "Assets")
    void AddAssetRule(const FEng_AssetPipelineRule& Rule);

    UFUNCTION(BlueprintCallable, Category = "Assets")
    bool ValidateAssetCompliance(const FString& AssetPath);

    UFUNCTION(BlueprintCallable, Category = "Assets")
    void PrepareCommercialAssetIntegration();

    // Compilation Enforcement
    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void EnforceCompilationRules();

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    bool ValidateHeaderStructure(const FString& HeaderPath);

    UFUNCTION(BlueprintCallable, Category = "Compilation")
    void GenerateModuleBuildFiles();

    // Query Functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Architecture")
    bool IsModuleLoaded(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Architecture")
    TArray<FString> GetLoadedModules() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    float GetCurrentPerformanceScore() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Assets")
    int32 GetTotalAssetsRegistered() const;

    // Emergency Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void EmergencyModuleShutdown(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ResetArchitecturalFramework();

private:
    void LogArchitecturalEvent(const FString& Message);
    void ValidateSystemIntegrity();
    void UpdatePerformanceMetrics();
    void CheckModuleHealth();
};