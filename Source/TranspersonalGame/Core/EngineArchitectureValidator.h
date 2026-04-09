#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineArchitectureValidator.generated.h"

UENUM(BlueprintType)
enum class EArchitectureValidationResult : uint8
{
    Valid,
    Warning,
    Error,
    Critical
};

USTRUCT(BlueprintType)
struct FArchitectureValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString SystemName;

    UPROPERTY(BlueprintReadOnly)
    EArchitectureValidationResult Result;

    UPROPERTY(BlueprintReadOnly)
    FString Message;

    UPROPERTY(BlueprintReadOnly)
    FString Recommendation;

    FArchitectureValidation()
    {
        Result = EArchitectureValidationResult::Valid;
    }

    FArchitectureValidation(const FString& InSystemName, EArchitectureValidationResult InResult, 
                           const FString& InMessage, const FString& InRecommendation = TEXT(""))
        : SystemName(InSystemName), Result(InResult), Message(InMessage), Recommendation(InRecommendation)
    {
    }
};

USTRUCT(BlueprintType)
struct FEngineRequirements
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bRequiresWorldPartition;

    UPROPERTY(BlueprintReadOnly)
    bool bRequiresNanite;

    UPROPERTY(BlueprintReadOnly)
    bool bRequiresLumen;

    UPROPERTY(BlueprintReadOnly)
    bool bRequiresVirtualShadowMaps;

    UPROPERTY(BlueprintReadOnly)
    bool bRequiresMassEntity;

    UPROPERTY(BlueprintReadOnly)
    bool bRequiresPCG;

    UPROPERTY(BlueprintReadOnly)
    int32 MinimumWorldSize;

    UPROPERTY(BlueprintReadOnly)
    int32 TargetFrameRate;

    UPROPERTY(BlueprintReadOnly)
    FString TargetPlatform;

    FEngineRequirements()
    {
        bRequiresWorldPartition = true;
        bRequiresNanite = true;
        bRequiresLumen = true;
        bRequiresVirtualShadowMaps = true;
        bRequiresMassEntity = true;
        bRequiresPCG = true;
        MinimumWorldSize = 4000; // 4km x 4km minimum
        TargetFrameRate = 60;
        TargetPlatform = TEXT("PC/Console");
    }
};

/**
 * Engine Architecture Validator
 * Validates that UE5 systems meet the requirements for Transpersonal Game
 * Ensures all required features are available and properly configured
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectureValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectureValidator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Core Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FArchitectureValidation> ValidateEngineArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FArchitectureValidation ValidateWorldPartition();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FArchitectureValidation ValidateNaniteSupport();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FArchitectureValidation ValidateLumenSupport();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FArchitectureValidation ValidateVirtualShadowMaps();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FArchitectureValidation ValidateMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FArchitectureValidation ValidatePCGSystem();

    // Performance Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FArchitectureValidation ValidatePerformanceTargets();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FArchitectureValidation ValidateMemoryRequirements();

    // Project Configuration Validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FArchitectureValidation ValidateProjectSettings();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FArchitectureValidation ValidateRenderingSettings();

    // Utility Functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Engine Architecture")
    FEngineRequirements GetEngineRequirements() const { return Requirements; }

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsArchitectureValid();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void GenerateArchitectureReport();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ApplyRecommendedSettings();

protected:
    UPROPERTY()
    FEngineRequirements Requirements;

    UPROPERTY()
    TArray<FArchitectureValidation> LastValidationResults;

    // Internal validation helpers
    bool CheckFeatureAvailability(const FString& FeatureName, UClass* RequiredClass);
    bool CheckProjectSetting(const FString& SettingPath, const FString& ExpectedValue);
    bool CheckRenderingFeature(const FString& FeatureName);
    void LogValidationResult(const FArchitectureValidation& Result);

private:
    // Cache for validation results
    bool bValidationCacheValid;
    float LastValidationTime;
    static constexpr float ValidationCacheTimeout = 30.0f; // 30 seconds
};