#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_FinalCycleIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_FinalIntegrationStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"), 
    QAValidation    UMETA(DisplayName = "QA Validation"),
    ModuleCheck     UMETA(DisplayName = "Module Check"),
    AssetValidation UMETA(DisplayName = "Asset Validation"),
    BuildReady      UMETA(DisplayName = "Build Ready"),
    DeliveryReady   UMETA(DisplayName = "Delivery Ready"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_FinalIntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_FinalIntegrationStatus Status = EBuild_FinalIntegrationStatus::NotStarted;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 LoadedClasses = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalAssets = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 QATestsPassed = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 QATestsFailed = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 IntegrationScore = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bBuildReady = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> ValidationWarnings;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_FinalCycleIntegrationOrchestrator : public AActor
{
    GENERATED_BODY()

public:
    ABuild_FinalCycleIntegrationOrchestrator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === CORE INTEGRATION FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void StartFinalIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void ValidateQAResults();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void ValidateAssetCounts();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void GenerateFinalReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_FinalIntegrationReport GetIntegrationReport() const { return IntegrationReport; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsBuildReady() const { return IntegrationReport.bBuildReady; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetIntegrationScore() const { return IntegrationReport.IntegrationScore; }

protected:
    // === INTEGRATION STATE ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_FinalIntegrationReport IntegrationReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ValidationInterval = 5.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastValidationTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MinRequiredScore = 75;

    // === VALIDATION HELPERS ===
    
    void UpdateIntegrationStatus();
    void LogValidationResult(const FString& Component, bool bPassed, const FString& Details = "");
    void CalculateIntegrationScore();
    bool ValidateWorldState();
    bool ValidateCoreClasses();
    bool ValidateGameAssets();
    void SaveIntegrationReport();
};