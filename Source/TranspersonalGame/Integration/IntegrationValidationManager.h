#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "IntegrationValidationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FInteg_BuildValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 IssueCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> DetectedIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 PlayabilityScore = 0;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FInteg_LightingValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    int32 DirectionalLightCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    int32 SkyAtmosphereCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    int32 SkyLightCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    int32 ExponentialHeightFogCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    bool bHasDuplicates = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FInteg_GameplayValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    int32 PlayerStartCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    int32 CharacterCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    int32 EnvironmentActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    bool bIsPlayable = false;
};

UENUM(BlueprintType)
enum class EInteg_ValidationSeverity : uint8
{
    Info        UMETA(DisplayName = "Info"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FInteg_ValidationIssue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Issue")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "Issue")
    EInteg_ValidationSeverity Severity = EInteg_ValidationSeverity::Info;

    UPROPERTY(BlueprintReadOnly, Category = "Issue")
    FString ActorName;

    UPROPERTY(BlueprintReadOnly, Category = "Issue")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Issue")
    float DetectionTime = 0.0f;
};

/**
 * Integration Validation Manager
 * Validates the overall state of the game build and integration between modules
 * Ensures all systems work together correctly and the game remains playable
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UIntegrationValidationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UIntegrationValidationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Main validation functions
    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FInteg_BuildValidationResult ValidateFullBuild();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FInteg_LightingValidation ValidateLightingSetup();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FInteg_GameplayValidation ValidateGameplayElements();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool ValidateModuleIntegration(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    TArray<FInteg_ValidationIssue> GetAllValidationIssues();

    // Cleanup and maintenance
    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    int32 CleanupDuplicateLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool FixCriticalIssues();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    void GenerateValidationReport(const FString& OutputPath);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    float GetCurrentPerformanceScore();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool IsGamePlayable();

    // Module-specific validation
    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool ValidateAISystems();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool ValidateVFXSystems();

    // Real-time monitoring
    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    void StartContinuousValidation(float IntervalSeconds = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    void StopContinuousValidation();

protected:
    // Internal validation helpers
    void ValidateActorCounts(TArray<AActor*>& AllActors, FInteg_BuildValidationResult& Result);
    void ValidateEssentialActors(TArray<AActor*>& AllActors, FInteg_BuildValidationResult& Result);
    void ValidateModuleClasses(FInteg_BuildValidationResult& Result);
    void CalculatePlayabilityScore(const FInteg_LightingValidation& Lighting, 
                                  const FInteg_GameplayValidation& Gameplay, 
                                  FInteg_BuildValidationResult& Result);

    // Issue tracking
    void AddValidationIssue(const FString& Description, EInteg_ValidationSeverity Severity, 
                           const FString& ActorName = TEXT(""), const FString& ModuleName = TEXT(""));
    void ClearValidationIssues();

    // Timer for continuous validation
    FTimerHandle ContinuousValidationTimer;
    void PerformContinuousValidation();

private:
    UPROPERTY()
    TArray<FInteg_ValidationIssue> CurrentValidationIssues;

    UPROPERTY()
    float LastValidationTime = 0.0f;

    UPROPERTY()
    bool bContinuousValidationActive = false;

    // Validation thresholds
    static constexpr int32 MAX_DIRECTIONAL_LIGHTS = 1;
    static constexpr int32 MAX_SKY_ATMOSPHERES = 1;
    static constexpr int32 MAX_SKY_LIGHTS = 1;
    static constexpr int32 MAX_HEIGHT_FOGS = 1;
    static constexpr int32 MIN_ENVIRONMENT_ACTORS = 5;
    static constexpr float PERFORMANCE_THRESHOLD_MS = 1000.0f;
};