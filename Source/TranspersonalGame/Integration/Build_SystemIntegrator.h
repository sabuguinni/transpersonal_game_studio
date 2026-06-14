#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "Build_SystemIntegrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Healthy,
    Moderate,
    NeedsWork,
    Critical
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 CriticalSystemsLoaded = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActiveSystems = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float QualityScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus Status = EBuild_IntegrationStatus::NeedsWork;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bLevelPlayable = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 HighImpactActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 MediumImpactActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 LowImpactActors = 0;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CrossSystemTest
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIntegrationHealthy = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> Dependencies;
};

/**
 * System Integrator - Manages cross-system integration and build validation
 * Ensures all game systems work together cohesively
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_SystemIntegrator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UBuild_SystemIntegrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemMetrics ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_CrossSystemTest> RunCrossSystemTests();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCriticalSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void EnforceActorCaps();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float CalculatePerformanceScore();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsLevelPlayable();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void RunFullIntegrationValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_SystemMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_CrossSystemTest> LastTestResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxActorCount = 8000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxDinosaurCount = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float QualityThreshold = 0.8f;

private:
    void ValidateWorldGeneration();
    void ValidateEnvironmentSystems();
    void ValidateCharacterSystems();
    void ValidateAISystems();
    void ValidateAudioVFXSystems();
    void ValidateQuestNarrativeSystems();

    bool CheckSystemDependencies(const FString& SystemName);
    void CleanupRedundantActors();
    void OptimizePerformance();

    TArray<FString> CriticalSystemClasses;
    TMap<FString, TArray<FString>> SystemDependencies;
};