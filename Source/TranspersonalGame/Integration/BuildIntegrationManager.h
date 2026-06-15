#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float LoadTime;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        ClassCount = 0;
        LoadTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 CustomActors;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float BuildTime;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bBuildSuccess;

    FBuild_IntegrationReport()
    {
        TotalActors = 0;
        CustomActors = 0;
        BuildTime = 0.0f;
        bBuildSuccess = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateModuleIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CheckActorCaps();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void EnforceActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetLoadedModules();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateCrossSystemCompatibility();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FBuild_IntegrationReport LastReport;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bIntegrationValid;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 MaxDinosaurs;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 MaxTotalActors;

private:
    void ValidateModuleStatus(const FString& ModuleName, FBuild_ModuleStatus& OutStatus);
    void CheckActorIntegrity();
    void ValidateSystemDependencies();
    bool TestCrossSystemInteraction();
};