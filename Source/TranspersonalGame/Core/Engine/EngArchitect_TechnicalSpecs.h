#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/NoExportTypes.h"
#include "EngArchitect_TechnicalSpecs.generated.h"

/**
 * ENGINE ARCHITECT TECHNICAL SPECIFICATIONS
 * Defines core architecture standards for all agents in the production chain
 * Agent #02 - Engine Architect - Cycle PROD_CYCLE_AUTO_20260525_002
 */

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low")
};

UENUM(BlueprintType)
enum class EEng_ModuleStatus : uint8
{
    NotStarted  UMETA(DisplayName = "Not Started"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Testing     UMETA(DisplayName = "Testing"),
    Complete    UMETA(DisplayName = "Complete"),
    Blocked     UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_ModuleStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    int32 AgentResponsible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxMemoryMB;

    FEng_SystemSpec()
    {
        SystemName = TEXT("Unnamed System");
        Priority = EEng_SystemPriority::Medium;
        Status = EEng_ModuleStatus::NotStarted;
        AgentResponsible = 0;
        MaxFrameTime = 16.67f; // 60 FPS target
        MaxMemoryMB = 512;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationRules
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
    bool bEnforceUE55Compatibility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
    bool bRequireMatchingCppFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
    bool bEnforceNamingConventions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
    bool bRequireSharedTypesUsage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxIncludeDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxCompileTimeSeconds;

    FEng_CompilationRules()
    {
        bEnforceUE55Compatibility = true;
        bRequireMatchingCppFiles = true;
        bEnforceNamingConventions = true;
        bRequireSharedTypesUsage = true;
        MaxIncludeDepth = 10;
        MaxCompileTimeSeconds = 30.0f;
    }
};

/**
 * Engine Architect Core Component
 * Manages technical specifications and architecture validation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngArchitect_TechnicalSpecs : public UObject
{
    GENERATED_BODY()

public:
    UEngArchitect_TechnicalSpecs();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FEng_SystemSpec> SystemSpecifications;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    FEng_CompilationRules CompilationRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTotalActors;

    // Architecture validation functions
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool CheckCompilationCompliance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidatePerformanceTargets();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterSystemSpec(const FEng_SystemSpec& NewSpec);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FEng_SystemSpec GetSystemSpec(const FString& SystemName);

protected:
    UPROPERTY()
    TMap<FString, FEng_SystemSpec> SystemRegistry;

    void InitializeDefaultSpecs();
    bool ValidateAgentChain();
    bool CheckModuleDependencies();
};