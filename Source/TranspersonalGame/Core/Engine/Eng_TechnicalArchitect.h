#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_TechnicalArchitect.generated.h"

UENUM(BlueprintType)
enum class EEng_ArchitecturalLayer : uint8
{
    Foundation      UMETA(DisplayName = "Foundation Layer"),
    Core           UMETA(DisplayName = "Core Systems Layer"),
    Gameplay       UMETA(DisplayName = "Gameplay Layer"),
    Interface      UMETA(DisplayName = "Interface Layer"),
    Performance    UMETA(DisplayName = "Performance Layer")
};

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical       UMETA(DisplayName = "Critical Priority"),
    High          UMETA(DisplayName = "High Priority"),
    Medium        UMETA(DisplayName = "Medium Priority"),
    Low           UMETA(DisplayName = "Low Priority"),
    Deferred      UMETA(DisplayName = "Deferred Priority")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitecturalRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString RuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EEng_ArchitecturalLayer Layer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EEng_SystemPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnforced;

    FEng_ArchitecturalRule()
    {
        RuleName = TEXT("");
        Description = TEXT("");
        Layer = EEng_ArchitecturalLayer::Foundation;
        Priority = EEng_SystemPriority::Medium;
        bEnforced = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemCompliance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    float ComplianceScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    TArray<FString> ViolatedRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    bool bCriticalViolation;

    FEng_SystemCompliance()
    {
        SystemName = TEXT("");
        ComplianceScore = 0.0f;
        bCriticalViolation = false;
    }
};

/**
 * Technical Architect - Supreme architectural authority for the game engine
 * Defines and enforces architectural standards, governs system design decisions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_TechnicalArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_TechnicalArchitect();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architectural Governance
    UFUNCTION(BlueprintCallable, Category = "Technical Architect")
    void EstablishArchitecturalFoundation();

    UFUNCTION(BlueprintCallable, Category = "Technical Architect")
    void EnforceArchitecturalStandards();

    UFUNCTION(BlueprintCallable, Category = "Technical Architect")
    bool ValidateSystemCompliance(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Technical Architect")
    void RegisterArchitecturalRule(const FEng_ArchitecturalRule& Rule);

    // System Monitoring
    UFUNCTION(BlueprintCallable, Category = "Technical Architect")
    FEng_SystemCompliance AnalyzeSystemCompliance(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Technical Architect")
    TArray<FString> GetNonCompliantSystems();

    UFUNCTION(BlueprintCallable, Category = "Technical Architect")
    void GenerateComplianceReport();

    // Performance Architecture
    UFUNCTION(BlueprintCallable, Category = "Technical Architect")
    void EstablishPerformanceBaselines();

    UFUNCTION(BlueprintCallable, Category = "Technical Architect")
    void MonitorSystemPerformance();

    UFUNCTION(BlueprintCallable, Category = "Technical Architect")
    bool EnforcePerformanceLimits();

    // Module Integration
    UFUNCTION(BlueprintCallable, Category = "Technical Architect")
    void ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Technical Architect")
    void EstablishModuleInterfaces();

    UFUNCTION(BlueprintCallable, Category = "Technical Architect")
    bool ApproveSystemIntegration(const FString& SystemName);

protected:
    // Architectural Rules Registry
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<FEng_ArchitecturalRule> ArchitecturalRules;

    // System Compliance Tracking
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compliance", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FEng_SystemCompliance> SystemComplianceMap;

    // Performance Baselines
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float MaxMemoryUsage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxDrawCalls;

private:
    // Internal Methods
    void InitializeFoundationalRules();
    void InitializeCoreSystemRules();
    void InitializeGameplayRules();
    void InitializePerformanceRules();
    
    bool ValidateLayerCompliance(const FString& SystemName, EEng_ArchitecturalLayer Layer);
    float CalculateComplianceScore(const FString& SystemName);
    void LogComplianceViolation(const FString& SystemName, const FString& RuleName);
};