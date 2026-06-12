#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_IntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bIsCompiled;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float LoadTime;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsCompiled = false;
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
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bActorCapEnforced;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float TotalBuildTime;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    EBuild_ValidationResult ValidationResult;

    FBuild_IntegrationReport()
    {
        TotalActorCount = 0;
        bActorCapEnforced = false;
        TotalBuildTime = 0.0f;
        ValidationResult = EBuild_ValidationResult::Pending;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_IntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_IntegrationManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RunFullIntegrationTest();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void EnforceActorCap(int32 MaxActors = 8000);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport GetLastIntegrationReport() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateModuleIntegration(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RegisterModuleForValidation(const FString& ModuleName, const TArray<FString>& RequiredClasses);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ClearValidationCache();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FBuild_IntegrationReport LastReport;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TMap<FString, TArray<FString>> RegisteredModules;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bIntegrationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float LastValidationTime;

private:
    void ValidateAllRegisteredModules();
    void CheckActorCounts();
    void GenerateIntegrationReport();
    bool TestClassLoading(const FString& ClassName);
    void CleanupExcessActors(int32 MaxActors);
};