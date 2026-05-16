#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_TechnicalArchitecture.generated.h"

// Engine Architect Technical Standards and Compliance System

UENUM(BlueprintType)
enum class EEng_ArchitecturalLayer : uint8
{
    Core            UMETA(DisplayName = "Core Systems"),
    Physics         UMETA(DisplayName = "Physics Layer"),
    World           UMETA(DisplayName = "World Generation"),
    Environment     UMETA(DisplayName = "Environment Art"),
    Characters      UMETA(DisplayName = "Character Systems"),
    AI              UMETA(DisplayName = "AI and Behavior"),
    Gameplay        UMETA(DisplayName = "Gameplay Logic"),
    UI              UMETA(DisplayName = "User Interface"),
    Audio           UMETA(DisplayName = "Audio Systems"),
    Performance     UMETA(DisplayName = "Performance Layer")
};

UENUM(BlueprintType)
enum class EEng_ComplianceLevel : uint8
{
    Critical        UMETA(DisplayName = "Critical - Must Fix"),
    High            UMETA(DisplayName = "High Priority"),
    Medium          UMETA(DisplayName = "Medium Priority"),
    Low             UMETA(DisplayName = "Low Priority"),
    Compliant       UMETA(DisplayName = "Fully Compliant")
};

UENUM(BlueprintType)
enum class EEng_ModuleStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    HeaderOnly      UMETA(DisplayName = "Header Only"),
    Implemented     UMETA(DisplayName = "Implemented"),
    Tested          UMETA(DisplayName = "Tested"),
    Validated       UMETA(DisplayName = "Validated"),
    Production      UMETA(DisplayName = "Production Ready")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleCompliance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    EEng_ArchitecturalLayer Layer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    EEng_ModuleStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    EEng_ComplianceLevel ComplianceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    TArray<FString> MissingImplementations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    TArray<FString> CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    bool bHasCppImplementation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    bool bPassesValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
    float PerformanceScore;

    FEng_ModuleCompliance()
    {
        ModuleName = TEXT("");
        Layer = EEng_ArchitecturalLayer::Core;
        Status = EEng_ModuleStatus::NotStarted;
        ComplianceLevel = EEng_ComplianceLevel::Critical;
        bHasCppImplementation = false;
        bPassesValidation = false;
        PerformanceScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitecturalRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    FString RuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    EEng_ComplianceLevel Severity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    bool bMandatory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    TArray<FString> AffectedModules;

    FEng_ArchitecturalRule()
    {
        RuleName = TEXT("");
        Description = TEXT("");
        Severity = EEng_ComplianceLevel::Medium;
        bMandatory = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    bool bCompilationSuccessful;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    int32 TotalErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    int32 TotalWarnings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    TArray<FString> ErrorMessages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    TArray<FString> WarningMessages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    TArray<FString> MissingCppFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    float CompilationTime;

    FEng_CompilationReport()
    {
        bCompilationSuccessful = false;
        TotalErrors = 0;
        TotalWarnings = 0;
        CompilationTime = 0.0f;
    }
};

/**
 * Engine Architect Technical Architecture Manager
 * Enforces coding standards, validates module compliance, and ensures architectural integrity
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_TechnicalArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_TechnicalArchitecture();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architectural Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateModuleCompliance(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_CompilationReport GenerateCompilationReport();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_ModuleCompliance> GetAllModuleCompliance();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool EnforceArchitecturalRules();

    // Code Quality Management
    UFUNCTION(BlueprintCallable, Category = "Code Quality")
    bool ValidateHeaderCppPairs();

    UFUNCTION(BlueprintCallable, Category = "Code Quality")
    TArray<FString> FindOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Code Quality")
    TArray<FString> FindMissingImplementations();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float CalculateModulePerformanceScore(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidatePerformanceCompliance();

    // Integration Testing
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool TestModuleIntegration(const FString& ModuleA, const FString& ModuleB);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemDependencies();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    bool RegisterAgentModule(int32 AgentNumber, const FString& ModuleName, EEng_ArchitecturalLayer Layer);

    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    TArray<FString> GetAgentDependencies(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    bool ValidateAgentCompliance(int32 AgentNumber);

protected:
    // Core architectural rules and standards
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    TArray<FEng_ArchitecturalRule> ArchitecturalRules;

    // Module compliance tracking
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modules")
    TArray<FEng_ModuleCompliance> ModuleCompliance;

    // Agent module mapping
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Agents")
    TMap<int32, FString> AgentModuleMap;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float MinPerformanceScore = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float MaxCompilationTime = 300.0f;

private:
    // Internal validation methods
    bool ValidateModuleStructure(const FString& ModuleName);
    bool CheckCppImplementation(const FString& HeaderPath);
    void InitializeArchitecturalRules();
    void ScanModuleDirectory(const FString& DirectoryPath);
    FEng_ModuleCompliance CreateModuleCompliance(const FString& ModuleName);
};

/**
 * Engine Architect Game Mode Integration
 * Provides architectural oversight during gameplay
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API AEng_ArchitecturalGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AEng_ArchitecturalGameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Real-time validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool PerformRuntimeValidation();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void LogArchitecturalStatus();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    bool bEnableRuntimeValidation = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    float ValidationInterval = 30.0f;

private:
    float LastValidationTime = 0.0f;
    UEng_TechnicalArchitecture* TechnicalArchitecture = nullptr;
};