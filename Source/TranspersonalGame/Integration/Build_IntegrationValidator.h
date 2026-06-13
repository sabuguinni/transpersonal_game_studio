#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown,
    Healthy,
    Warning,
    Critical,
    Failed
};

USTRUCT(BlueprintType)
struct FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bCanSpawn;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ErrorMessage;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bCanSpawn = false;
        ErrorMessage = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 WorkingModules;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalModules;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float IntegrationScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> Recommendations;

    FBuild_IntegrationReport()
    {
        OverallStatus = EBuild_IntegrationStatus::Unknown;
        TotalActors = 0;
        WorkingModules = 0;
        TotalModules = 0;
        IntegrationScore = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_IntegrationValidator : public AActor
{
    GENERATED_BODY()

public:
    ABuild_IntegrationValidator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport CurrentReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> CriticalModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bAutoValidateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ValidationInterval;

    FTimerHandle ValidationTimerHandle;

public:
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport GetCurrentReport() const { return CurrentReport; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateModule(const FString& ModuleName, FBuild_ModuleStatus& OutStatus);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateLevelIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateBuildHealth();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float CalculateIntegrationScore();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateRecommendations();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Integration")
    void EditorValidateAll();

protected:
    void PeriodicValidation();
    
    bool TestModuleSpawning(const FString& ModuleName);
    
    void LogValidationResults();
};