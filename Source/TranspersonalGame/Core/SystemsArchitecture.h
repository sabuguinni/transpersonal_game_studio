#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "SystemsArchitecture.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSystemsArchitecture, Log, All);

/**
 * Defines the architectural patterns and rules for all game systems
 * Ensures consistent integration between different subsystems
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSystemArchitectureRule
{
    GENERATED_BODY()

    /** Name of the architectural rule */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString RuleName;

    /** Description of what this rule enforces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString Description;

    /** Priority level (higher = more important) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 Priority = 1;

    /** Whether this rule is currently enforced */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnabled = true;

    FSystemArchitectureRule()
    {
        RuleName = TEXT("DefaultRule");
        Description = TEXT("Default architecture rule");
        Priority = 1;
        bEnabled = true;
    }
};

/**
 * System integration pattern definitions
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSystemIntegrationPattern
{
    GENERATED_BODY()

    /** Pattern name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FString PatternName;

    /** Systems involved in this pattern */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> InvolvedSystems;

    /** Communication protocol */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    ESystemCommunicationType CommunicationType;

    /** Performance impact level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    EPerformanceImpactLevel PerformanceImpact;

    FSystemIntegrationPattern()
    {
        PatternName = TEXT("DefaultPattern");
        CommunicationType = ESystemCommunicationType::Event;
        PerformanceImpact = EPerformanceImpactLevel::Low;
    }
};

/**
 * World Subsystem that manages system architecture and integration patterns
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API USystemsArchitecture : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    USystemsArchitecture();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Get the singleton instance for a world */
    UFUNCTION(BlueprintCallable, Category = "Systems Architecture")
    static USystemsArchitecture* Get(const UObject* WorldContext);

    /** Register a new architectural rule */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterArchitectureRule(const FSystemArchitectureRule& Rule);

    /** Register a system integration pattern */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterIntegrationPattern(const FSystemIntegrationPattern& Pattern);

    /** Validate system against architectural rules */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemArchitecture(const FString& SystemName);

    /** Get all registered architecture rules */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FSystemArchitectureRule> GetArchitectureRules() const;

    /** Get integration patterns for a system */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FSystemIntegrationPattern> GetIntegrationPatternsForSystem(const FString& SystemName) const;

    /** Force architectural compliance check on all systems */
    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor = true)
    void EnforceArchitecturalCompliance();

protected:
    /** Registered architecture rules */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FSystemArchitectureRule> ArchitectureRules;

    /** Registered integration patterns */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FSystemIntegrationPattern> IntegrationPatterns;

    /** Systems that have been validated */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TSet<FString> ValidatedSystems;

    /** Enable automatic compliance checking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bAutoComplianceCheck = true;

private:
    /** Initialize default architecture rules */
    void InitializeDefaultRules();

    /** Initialize default integration patterns */
    void InitializeDefaultPatterns();

    /** Validate a specific rule against a system */
    bool ValidateRule(const FSystemArchitectureRule& Rule, const FString& SystemName);
};