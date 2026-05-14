#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Eng_ArchitecturalFramework.generated.h"

/**
 * UE5.5 Compilation Standards and Architectural Rules
 * This system enforces technical standards across all 19 agents
 */

UENUM(BlueprintType)
enum class EEng_ModuleType : uint8
{
    Core,
    Gameplay,
    AI,
    Rendering,
    Audio,
    Physics,
    Network,
    UI,
    Tools
};

UENUM(BlueprintType)
enum class EEng_CompilationStatus : uint8
{
    Unknown,
    Compiling,
    Success,
    Failed,
    Blocked
};

UENUM(BlueprintType)
enum class EEng_PerformanceTier : uint8
{
    Critical,      // Must run at 60fps
    Important,     // Must run at 30fps
    Optional,      // Can drop frames if needed
    Background     // Lowest priority
};

USTRUCT(BlueprintType)
struct FEng_ModuleStandards
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Standards")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Standards")
    EEng_ModuleType ModuleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Standards")
    TArray<FString> RequiredDependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Standards")
    TArray<FString> OptionalDependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Standards")
    EEng_PerformanceTier PerformanceTier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Standards")
    bool bRequiresCPPImplementation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Standards")
    int32 MaxMemoryMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module Standards")
    float MaxFrameTimeMS;

    FEng_ModuleStandards()
    {
        ModuleName = TEXT("");
        ModuleType = EEng_ModuleType::Gameplay;
        PerformanceTier = EEng_PerformanceTier::Important;
        bRequiresCPPImplementation = true;
        MaxMemoryMB = 100;
        MaxFrameTimeMS = 16.67f; // 60fps
    }
};

USTRUCT(BlueprintType)
struct FEng_CompilationRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation Rule")
    FString RuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation Rule")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation Rule")
    bool bMandatory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation Rule")
    TArray<FString> ViolationExamples;

    FEng_CompilationRule()
    {
        RuleName = TEXT("");
        Description = TEXT("");
        bMandatory = true;
    }
};

USTRUCT(BlueprintType)
struct FEng_ArchitecturalConstraint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Constraint")
    FString ConstraintName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Constraint")
    FString SystemA;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Constraint")
    FString SystemB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Constraint")
    FString RelationshipType; // "depends_on", "communicates_with", "excludes", "requires"

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Constraint")
    bool bBidirectional;

    FEng_ArchitecturalConstraint()
    {
        ConstraintName = TEXT("");
        SystemA = TEXT("");
        SystemB = TEXT("");
        RelationshipType = TEXT("depends_on");
        bBidirectional = false;
    }
};

/**
 * Engine Architectural Framework - Defines and enforces technical standards
 * This is the master system that all other agents must comply with
 */
UCLASS()
class TRANSPERSONALGAME_API UEng_ArchitecturalFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalFramework();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Compilation Standards
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterCompilationRule(const FEng_CompilationRule& Rule);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateModuleCompliance(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetCompilationViolations(const FString& ModuleName);

    // Module Management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterModuleStandards(const FEng_ModuleStandards& Standards);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_ModuleStandards GetModuleStandards(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsModuleCompliant(const FString& ModuleName);

    // Performance Enforcement
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetPerformanceTarget(const FString& SystemName, float MaxFrameTimeMS, int32 MaxMemoryMB);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CheckPerformanceCompliance(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetPerformanceViolations();

    // Architectural Constraints
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void AddArchitecturalConstraint(const FEng_ArchitecturalConstraint& Constraint);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemInteraction(const FString& SystemA, const FString& SystemB);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetArchitecturalViolations();

    // Code Quality Standards
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateHeaderStructure(const FString& HeaderPath);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateImplementationPair(const FString& HeaderPath, const FString& SourcePath);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetOrphanedHeaders();

    // UE5.5 Specific Rules
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateUE55Compatibility(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetDeprecatedAPIUsages();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ApproveAgentImplementation(const FString& AgentName, const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void BlockAgentImplementation(const FString& AgentName, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsAgentBlocked(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetBlockedAgents();

protected:
    UPROPERTY()
    TArray<FEng_CompilationRule> CompilationRules;

    UPROPERTY()
    TMap<FString, FEng_ModuleStandards> ModuleStandardsMap;

    UPROPERTY()
    TArray<FEng_ArchitecturalConstraint> ArchitecturalConstraints;

    UPROPERTY()
    TMap<FString, bool> AgentApprovalStatus;

    UPROPERTY()
    TMap<FString, FString> AgentBlockReasons;

private:
    void InitializeDefaultRules();
    void InitializeModuleStandards();
    void InitializeArchitecturalConstraints();
    bool ValidateUSTRUCTDeclaration(const FString& StructName);
    bool ValidateUENUMDeclaration(const FString& EnumName);
    bool ValidateUCLASSDeclaration(const FString& ClassName);
};