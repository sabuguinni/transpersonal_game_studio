#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
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
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ErrorMessage;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        ClassCount = 0;
        ErrorMessage = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 LoadedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString BuildTimestamp;

    FBuild_SystemReport()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        LoadedModules = 0;
        OverallStatus = EBuild_IntegrationStatus::Unknown;
        BuildTimestamp = TEXT("");
    }
};

/**
 * Integration validation system for Transpersonal Game Studio
 * Monitors build health, module loading, and cross-system compatibility
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    FBuild_SystemReport ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool ValidateModuleLoading();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool ValidateActorCounts();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool ValidateCoreGameplay();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    void GenerateBuildReport();

    // Status getters
    UFUNCTION(BlueprintPure, Category = "Integration")
    EBuild_IntegrationStatus GetCurrentBuildStatus() const { return CurrentBuildStatus; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    FBuild_SystemReport GetLastSystemReport() const { return LastSystemReport; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    bool IsSystemHealthy() const { return CurrentBuildStatus == EBuild_IntegrationStatus::Healthy; }

protected:
    // Internal validation helpers
    FBuild_ModuleStatus ValidateModule(const FString& ModuleName, const FString& ClassName);
    int32 CountActorsByKeyword(const TArray<FString>& Keywords);
    void LogIntegrationStatus(const FString& Message, bool bIsError = false);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    EBuild_IntegrationStatus CurrentBuildStatus;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    FBuild_SystemReport LastSystemReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    int32 MaxDinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CoreModuleNames;

    float LastValidationTime;
    bool bValidationEnabled;
};