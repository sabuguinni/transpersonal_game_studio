#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Eng_TechnicalArchitecture.generated.h"

/**
 * Engine Architecture Technical Rules and Validation System
 * 
 * CRITICAL RESPONSIBILITIES:
 * 1. Enforce compilation rules across all agents
 * 2. Validate biome coordinate system
 * 3. Manage module dependencies and header structure
 * 4. Prevent duplicate type definitions
 * 5. Ensure .h/.cpp pairing compliance
 * 
 * This system is the technical foundation that all other agents must follow.
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeValidationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FVector Center;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FVector MinBounds;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FVector MaxBounds;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FString BiomeName;

    FEng_BiomeValidationData()
    {
        Center = FVector::ZeroVector;
        MinBounds = FVector::ZeroVector;
        MaxBounds = FVector::ZeroVector;
        BiomeType = EBiomeType::Savanna;
        BiomeName = TEXT("Unknown");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationRule
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString RuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    bool bIsCritical;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 ViolationCount;

    FEng_CompilationRule()
    {
        RuleName = TEXT("Unknown");
        Description = TEXT("No description");
        bIsCritical = false;
        ViolationCount = 0;
    }
};

UENUM(BlueprintType)
enum class EEng_ArchitectureStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Validating      UMETA(DisplayName = "Validating"),
    Valid           UMETA(DisplayName = "Valid"),
    HasWarnings     UMETA(DisplayName = "Has Warnings"),
    HasErrors       UMETA(DisplayName = "Has Errors"),
    Critical        UMETA(DisplayName = "Critical Failure")
};

/**
 * Technical Architecture Manager - Core System
 * 
 * This subsystem enforces all technical rules and validates the codebase structure.
 * It runs continuously and blocks problematic code from being integrated.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_TechnicalArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_TechnicalArchitecture();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * BIOME COORDINATE VALIDATION
     * All agents MUST use these functions for spawn validation
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture|Biome")
    bool IsValidBiomeLocation(const FVector& Location, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture|Biome")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture|Biome")
    FVector GetRandomLocationInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture|Biome")
    FEng_BiomeValidationData GetBiomeData(EBiomeType BiomeType) const;

    /**
     * COMPILATION RULE ENFORCEMENT
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture|Compilation")
    bool ValidateHeaderCppPairing(const FString& HeaderPath) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture|Compilation")
    bool ValidateTypeUniqueness(const FString& TypeName) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture|Compilation")
    TArray<FEng_CompilationRule> GetActiveRules() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture|Compilation")
    EEng_ArchitectureStatus GetSystemStatus() const;

    /**
     * MODULE DEPENDENCY VALIDATION
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture|Modules")
    bool ValidateModuleDependency(const FString& FromModule, const FString& ToModule) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture|Modules")
    TArray<FString> GetRequiredModules() const;

    /**
     * AGENT COORDINATION
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture|Agents")
    bool CanAgentProceed(int32 AgentID, const FString& TaskType) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture|Agents")
    void RegisterAgentTask(int32 AgentID, const FString& TaskType, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "Architecture|Agents")
    void ReportAgentCompletion(int32 AgentID, bool bSuccess, const FString& Result);

protected:
    /**
     * BIOME DEFINITIONS
     * These coordinates are MANDATORY and come from brain memories
     */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<EBiomeType, FEng_BiomeValidationData> BiomeDefinitions;

    /**
     * COMPILATION RULES
     */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FEng_CompilationRule> CompilationRules;

    /**
     * SYSTEM STATUS
     */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    EEng_ArchitectureStatus CurrentStatus;

    /**
     * AGENT TRACKING
     */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<int32, FString> ActiveAgentTasks;

private:
    void InitializeBiomeDefinitions();
    void InitializeCompilationRules();
    void ValidateCurrentCodebase();
    
    bool ValidateActorSpawnLocation(const FVector& Location) const;
    void LogArchitectureViolation(const FString& Violation, bool bIsCritical = false) const;
};