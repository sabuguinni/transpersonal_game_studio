#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "EngArch_TechnicalStandards.generated.h"

/**
 * Technical Standards Manager - Engine Architect's core system for enforcing
 * coding conventions, performance baselines, and architectural compliance
 * across all 19 agents in the production pipeline.
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceBaseline
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate_PC = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate_Console = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerBiome = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDinosaursSimultaneous = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxDrawDistance = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxLODLevels = 4;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CodingStandard
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standards")
    FString AgentPrefix = "Eng_";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standards")
    bool bRequireHeaderCppPairs = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standards")
    bool bRequireUPROPERTYForExposedMembers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standards")
    bool bRequireSharedTypesUsage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standards")
    int32 MaxIncludesPerFile = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standards")
    int32 MaxLinesPerFunction = 100;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleIntegration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bIsCompiling = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bHasValidationErrors = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 ClassCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float LastCompileTime = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngArch_TechnicalStandards : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngArch_TechnicalStandards();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceBaseline(const FEng_PerformanceBaseline& NewBaseline);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceBaseline GetPerformanceBaseline() const { return PerformanceBaseline; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidateFrameRate(float CurrentFPS) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidateActorCount(int32 ActorCount, const FString& BiomeName) const;

    // Coding Standards
    UFUNCTION(BlueprintCallable, Category = "Standards")
    void SetCodingStandard(const FEng_CodingStandard& NewStandard);

    UFUNCTION(BlueprintCallable, Category = "Standards")
    FEng_CodingStandard GetCodingStandard() const { return CodingStandard; }

    UFUNCTION(BlueprintCallable, Category = "Standards")
    bool ValidateClassName(const FString& ClassName) const;

    UFUNCTION(BlueprintCallable, Category = "Standards")
    bool ValidateFileStructure(const FString& HeaderPath, const FString& CppPath) const;

    // Module Integration
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterModule(const FString& ModuleName, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UpdateModuleStatus(const FString& ModuleName, bool bCompiling, bool bHasErrors, int32 ClassCount);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FEng_ModuleIntegration> GetAllModules() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateModuleDependencies(const FString& ModuleName) const;

    // Build System
    UFUNCTION(BlueprintCallable, Category = "Build")
    void TriggerIncrementalCompilation();

    UFUNCTION(BlueprintCallable, Category = "Build")
    void ValidateSharedTypes();

    UFUNCTION(BlueprintCallable, Category = "Build")
    int32 GetDisabledFileCount() const { return DisabledFileCount; }

    UFUNCTION(BlueprintCallable, Category = "Build")
    void ReactivateModule(const FString& ModuleName);

    // Quality Gates
    UFUNCTION(BlueprintCallable, Category = "Quality")
    bool CanAgentProceed(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Quality")
    void SetQualityGate(int32 AgentNumber, bool bPassed);

    UFUNCTION(BlueprintCallable, Category = "Quality")
    TArray<int32> GetBlockedAgents() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FEng_PerformanceBaseline PerformanceBaseline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FEng_CodingStandard CodingStandard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modules")
    TArray<FEng_ModuleIntegration> RegisteredModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
    int32 DisabledFileCount = 882;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    TMap<int32, bool> AgentQualityGates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    bool bArchitectureApproved = false;

private:
    void InitializeDefaultStandards();
    void ValidateExistingModules();
    bool CheckDependencyChain(const FString& ModuleName, TSet<FString>& VisitedModules) const;
};