#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "EngineArchitectureManager.generated.h"

UENUM(BlueprintType)
enum class EEng_ArchitectureValidationResult : uint8
{
    Valid UMETA(DisplayName = "Valid"),
    Warning UMETA(DisplayName = "Warning"),
    Error UMETA(DisplayName = "Error"),
    Critical UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemRequirements
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    int32 MinMemoryMB = 8192;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    int32 MaxActorsPerLevel = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    bool bRequiresWorldPartition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    bool bRequiresLumen = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    bool bRequiresNanite = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    bool bRequiresChaosPhysics = true;

    FEng_SystemRequirements()
    {
        MinMemoryMB = 8192;
        MaxActorsPerLevel = 50000;
        TargetFrameRate = 60.0f;
        bRequiresWorldPartition = true;
        bRequiresLumen = true;
        bRequiresNanite = true;
        bRequiresChaosPhysics = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitectureRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    FString RuleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    EEng_ArchitectureValidationResult Severity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    bool bEnforced = true;

    FEng_ArchitectureRule()
    {
        RuleName = TEXT("");
        Description = TEXT("");
        Severity = EEng_ArchitectureValidationResult::Warning;
        bEnforced = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    TArray<FString> Warnings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    TArray<FString> Errors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    TArray<FString> CriticalIssues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    bool bPassedValidation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    float ValidationTime = 0.0f;

    FEng_ValidationReport()
    {
        bPassedValidation = false;
        ValidationTime = 0.0f;
    }
};

/**
 * Engine Architecture Manager - Defines and enforces technical architecture rules
 * This is the master system that validates all other systems comply with architecture requirements
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FEng_ValidationReport ValidateSystemArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateWorldPartitionSetup(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateRenderingPipeline();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidatePhysicsSetup();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateMemoryRequirements();

    // Rule management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddArchitectureRule(const FEng_ArchitectureRule& Rule);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RemoveArchitectureRule(const FString& RuleName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FEng_ArchitectureRule> GetAllRules() const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetCurrentActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsageMB() const;

    // System requirements
    UFUNCTION(BlueprintCallable, Category = "Requirements")
    void SetSystemRequirements(const FEng_SystemRequirements& Requirements);

    UFUNCTION(BlueprintCallable, Category = "Requirements")
    FEng_SystemRequirements GetSystemRequirements() const;

    // Debug and testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void RunFullArchitectureValidation();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogSystemStatus();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    FEng_SystemRequirements SystemRequirements;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<FEng_ArchitectureRule> ArchitectureRules;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    FEng_ValidationReport LastValidationReport;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float PerformanceCheckInterval = 1.0f;

private:
    void InitializeDefaultRules();
    void ValidateEngineFeatures();
    bool CheckWorldPartitionCompatibility(UWorld* World);
    bool CheckRenderingFeatures();
    void LogValidationResult(const FEng_ValidationReport& Report);
};