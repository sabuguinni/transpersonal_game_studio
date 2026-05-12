#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_TechnicalArchitecture.generated.h"

/**
 * TECHNICAL ARCHITECTURE SYSTEM - Engine Architect Agent #2
 * 
 * This system defines the absolute technical rules and constraints that ALL agents
 * must follow. It establishes performance budgets, memory limits, and architectural
 * patterns that ensure the game can scale from prototype to full production.
 * 
 * CRITICAL: This is the technical law of the studio. No agent can violate these rules.
 */

UENUM(BlueprintType)
enum class EEng_PerformanceProfile : uint8
{
    Development,    // No limits, debug mode
    Console,        // 30 FPS target, strict memory
    PC_High,        // 60 FPS target, moderate limits
    PC_Ultra        // 120 FPS target, minimal limits
};

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
    Critical,       // Core gameplay systems
    High,           // Performance-sensitive systems
    Medium,         // Standard game features
    Low,            // Polish and extras
    Debug           // Development tools only
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxFrameTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxParticleCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryMB;

    FEng_PerformanceBudget()
    {
        MaxFrameTimeMS = 16.67f; // 60 FPS
        MaxActiveActors = 5000;
        MaxPhysicsBodies = 1000;
        MaxParticleCount = 10000;
        MaxMemoryMB = 8192.0f; // 8 GB
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemConstraints
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraints")
    bool bEnforcePerformanceLimits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraints")
    bool bRequireModularDesign;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraints")
    bool bEnforceMemoryPools;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraints")
    bool bRequireThreadSafety;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraints")
    float MaxSystemInitTimeMS;

    FEng_SystemConstraints()
    {
        bEnforcePerformanceLimits = true;
        bRequireModularDesign = true;
        bEnforceMemoryPools = true;
        bRequireThreadSafety = true;
        MaxSystemInitTimeMS = 100.0f;
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
    EEng_SystemPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    bool bMandatory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    TArray<FString> AffectedAgents;

    FEng_ArchitecturalRule()
    {
        Priority = EEng_SystemPriority::Medium;
        bMandatory = true;
    }
};

/**
 * Technical Architecture Subsystem - Global Rules Engine
 * 
 * This subsystem enforces technical constraints across all game systems.
 * It monitors performance, validates system compliance, and blocks
 * operations that would violate architectural rules.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_TechnicalArchitectureSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void SetPerformanceProfile(EEng_PerformanceProfile Profile);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    FEng_PerformanceBudget GetCurrentPerformanceBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool ValidatePerformanceCompliance(const FString& SystemName, float FrameTimeMS, int32 ActorCount);

    // System Registration and Validation
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority, const FString& AgentOwner);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool ValidateSystemArchitecture(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void BlockSystemOperation(const FString& SystemName, const FString& Reason);

    // Architectural Rules Management
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void AddArchitecturalRule(const FEng_ArchitecturalRule& Rule);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool ValidateAgentCompliance(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    TArray<FString> GetViolatingAgents() const;

    // Memory and Resource Management
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    float GetCurrentMemoryUsageMB() const;

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool RequestMemoryAllocation(const FString& SystemName, float SizeMB);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void ReleaseMemoryAllocation(const FString& SystemName, float SizeMB);

    // Thread Safety Validation
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool ValidateThreadSafety(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void ReportThreadSafetyViolation(const FString& SystemName, const FString& Details);

    // Development Tools
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture", CallInEditor = true)
    void GenerateArchitectureReport();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture", CallInEditor = true)
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture", CallInEditor = true)
    void EnforceArchitecturalCompliance();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    EEng_PerformanceProfile CurrentProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FEng_PerformanceBudget PerformanceBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FEng_SystemConstraints SystemConstraints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
    TArray<FEng_ArchitecturalRule> ArchitecturalRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Systems")
    TMap<FString, EEng_SystemPriority> RegisteredSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Systems")
    TMap<FString, FString> SystemOwners;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TMap<FString, float> SystemMemoryAllocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violations")
    TArray<FString> ViolatingAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violations")
    TArray<FString> BlockedSystems;

private:
    void InitializeDefaultRules();
    void InitializePerformanceProfiles();
    bool CheckSystemCompliance(const FString& SystemName);
    void LogArchitecturalViolation(const FString& SystemName, const FString& Violation);
};

/**
 * Technical Architecture Component - Per-Actor Rules Enforcement
 * 
 * Attach this component to actors that need architectural validation.
 * It monitors the actor's performance and enforces technical constraints.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_TechnicalArchitectureComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_TechnicalArchitectureComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void ValidateActorCompliance();

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    bool IsActorCompliant() const;

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void ReportPerformanceMetrics(float FrameTime, int32 ComponentCount);

    UFUNCTION(BlueprintCallable, Category = "Technical Architecture")
    void SetSystemPriority(EEng_SystemPriority Priority);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    EEng_SystemPriority ActorPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnforcePerformanceLimits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxFrameTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float CurrentFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    bool bIsCompliant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    TArray<FString> ComplianceViolations;

private:
    void CheckPerformanceCompliance();
    void ValidateComponentArchitecture();
    void ReportViolation(const FString& Violation);
};