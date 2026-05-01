#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
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
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString LastError;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bHasErrors = false;
        ClassCount = 0;
        LastError = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bIsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> ValidationErrors;

    FBuild_SystemValidation()
    {
        SystemName = TEXT("");
        bIsValid = false;
        ActorCount = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> GetModuleStatuses();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_SystemValidation> GetSystemValidations();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FString GetBuildReport();

    // Cross-system validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateAISystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateVFXSystems();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_SystemValidation> SystemValidations;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bLastValidationPassed;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FString LastBuildReport;

private:
    // Internal validation helpers
    FBuild_ModuleStatus ValidateModule(const FString& ModuleName);
    FBuild_SystemValidation ValidateSystem(const FString& SystemName, UWorld* World);
    bool CheckClassExists(const FString& ClassName);
    int32 CountActorsOfType(UWorld* World, const FString& ActorType);
    void LogValidationResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
};