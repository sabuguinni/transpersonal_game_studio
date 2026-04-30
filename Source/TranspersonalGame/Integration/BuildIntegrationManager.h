#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
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
    bool bHasErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ClassCount;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bHasErrors = false;
        ErrorMessage = TEXT("");
        ClassCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bAllModulesLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 TotalModules;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 LoadedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ErrorModules;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float OverallHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString LastBuildTime;

    FBuild_SystemHealth()
    {
        bAllModulesLoaded = false;
        TotalModules = 0;
        LoadedModules = 0;
        ErrorModules = 0;
        OverallHealth = 0.0f;
        LastBuildTime = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_SystemHealth GetSystemHealth() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> GetModuleStatuses() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsModuleHealthy(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RefreshModuleStatus();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void LogSystemStatus() const;

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TestCoreSystemsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TestWorldGenerationIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool TestCharacterSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RunFullIntegrationTest();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FBuild_SystemHealth SystemHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bInitialized;

private:
    void ScanForModules();
    void ValidateModule(const FString& ModuleName);
    void UpdateSystemHealth();
    void LogModuleStatus(const FBuild_ModuleStatus& Status) const;
};