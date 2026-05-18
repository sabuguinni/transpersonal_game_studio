#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Pending,
    InProgress,
    Success,
    Failed,
    Timeout
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString SystemName;

    UPROPERTY(BlueprintReadOnly)
    EBuild_IntegrationStatus Status;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly)
    float ValidationTime;

    UPROPERTY(BlueprintReadOnly)
    int32 ActorCount;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("");
        Status = EBuild_IntegrationStatus::Pending;
        ErrorMessage = TEXT("");
        ValidationTime = 0.0f;
        ActorCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_BiomeValidationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly)
    FVector BiomeCenter;

    UPROPERTY(BlueprintReadOnly)
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly)
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly)
    int32 VegetationCount;

    UPROPERTY(BlueprintReadOnly)
    bool bHasLighting;

    FBuild_BiomeValidationData()
    {
        BiomeName = TEXT("");
        BiomeCenter = FVector::ZeroVector;
        ActorCount = 0;
        DinosaurCount = 0;
        VegetationCount = 0;
        bHasLighting = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void StartFullSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateDinosaurSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidatePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult GetValidationResult(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemValidationResult> GetAllValidationResults();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_BiomeValidationData> GetBiomeValidationData();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemHealthy();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CleanupOrphanedActors();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void OptimizeActorDistribution();

protected:
    UPROPERTY(BlueprintReadOnly)
    TArray<FBuild_SystemValidationResult> ValidationResults;

    UPROPERTY(BlueprintReadOnly)
    TArray<FBuild_BiomeValidationData> BiomeData;

    UPROPERTY(BlueprintReadOnly)
    bool bValidationInProgress;

    UPROPERTY(BlueprintReadOnly)
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalDinosaurCount;

    UPROPERTY(BlueprintReadOnly)
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly)
    float FrameRate;

private:
    void ValidateSystemInternal(const FString& SystemName);
    void ValidateBiomeInternal(const FString& BiomeName, const FVector& BiomeCenter);
    void UpdatePerformanceMetrics();
    void LogValidationResult(const FBuild_SystemValidationResult& Result);
};