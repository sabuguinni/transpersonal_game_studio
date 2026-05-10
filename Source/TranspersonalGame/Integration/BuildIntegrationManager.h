#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bIsCompiled;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bHasImplementation;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString LastError;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsCompiled = false;
        bHasImplementation = false;
        ClassCount = 0;
        LastError = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bBuildSuccess;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 TotalModules;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 CompiledModules;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString BuildTimestamp;

    FBuild_ValidationResult()
    {
        bBuildSuccess = false;
        TotalModules = 0;
        CompiledModules = 0;
        BuildTimestamp = TEXT("");
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
    FBuild_ValidationResult ValidateCurrentBuild();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CheckModuleCompilation(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateSharedTypes();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void GenerateBuildReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool FixCommonBuildErrors();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FBuild_ValidationResult LastValidationResult;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> KnownModules;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bIntegrationActive;

private:
    void ScanForModules();
    void ValidateModuleStructure(const FString& ModuleName, FBuild_ModuleStatus& OutStatus);
    bool CheckHeaderImplementationPair(const FString& HeaderPath);
    void LogBuildStatus(const FString& Message, bool bIsError = false);
};