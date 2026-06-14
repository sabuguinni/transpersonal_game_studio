#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationStatus : uint8
{
    Unknown,
    Passed,
    Failed,
    Warning
};

USTRUCT(BlueprintType)
struct FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly)
    EBuild_ValidationStatus Status;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly)
    int32 ClassesLoaded;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalClasses;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        Status = EBuild_ValidationStatus::Unknown;
        ErrorMessage = TEXT("");
        ClassesLoaded = 0;
        TotalClasses = 0;
    }
};

USTRUCT(BlueprintType)
struct FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly)
    int32 DinosaurActors;

    UPROPERTY(BlueprintReadOnly)
    bool bGameStateValid;

    UPROPERTY(BlueprintReadOnly)
    bool bCharacterSystemValid;

    UPROPERTY(BlueprintReadOnly)
    FString BuildTimestamp;

    FBuild_IntegrationReport()
    {
        TotalActors = 0;
        DinosaurActors = 0;
        bGameStateValid = false;
        bCharacterSystemValid = false;
        BuildTimestamp = TEXT("");
    }
};

/**
 * Integration validation subsystem for build quality assurance
 * Validates module loading, cross-system dependencies, and runtime integrity
 */
UCLASS()
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport ValidateFullSystem();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuild_ValidationStatus ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateCrossSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void LogIntegrationReport(const FBuild_IntegrationReport& Report);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Build Integration")
    void RunEditorValidation();

protected:
    UPROPERTY()
    FBuild_IntegrationReport LastReport;

    UPROPERTY()
    TArray<FString> CriticalModules;

    void InitializeCriticalModules();
    bool ValidateActorCounts();
    bool ValidateGameState();
    bool ValidateCharacterSystem();
    FBuild_ModuleStatus ValidateModuleClasses(const FString& ModuleName);
};