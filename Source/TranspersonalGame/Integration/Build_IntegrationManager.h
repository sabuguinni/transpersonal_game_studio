#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Build_IntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown,
    Initializing,
    Ready,
    Testing,
    Failed,
    Complete
};

USTRUCT(BlueprintType)
struct FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly)
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly)
    bool bHasErrors;

    UPROPERTY(BlueprintReadOnly)
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly)
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
struct FBuild_SystemValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString SystemName;

    UPROPERTY(BlueprintReadOnly)
    bool bIsValid;

    UPROPERTY(BlueprintReadOnly)
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly)
    FString ValidationMessage;

    FBuild_SystemValidation()
    {
        SystemName = TEXT("");
        bIsValid = false;
        ActorCount = 0;
        ValidationMessage = TEXT("");
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_IntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_IntegrationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Integration management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void StartIntegrationValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateMapSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void EnforceActorCap(int32 MaxActors = 8000);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CheckCompilationStatus();

    // Status queries
    UFUNCTION(BlueprintPure, Category = "Build Integration")
    EBuild_IntegrationStatus GetIntegrationStatus() const { return CurrentStatus; }

    UFUNCTION(BlueprintPure, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> GetModuleStatuses() const { return ModuleStatuses; }

    UFUNCTION(BlueprintPure, Category = "Build Integration")
    TArray<FBuild_SystemValidation> GetSystemValidations() const { return SystemValidations; }

    UFUNCTION(BlueprintPure, Category = "Build Integration")
    int32 GetTotalActorCount() const { return TotalActorCount; }

    UFUNCTION(BlueprintPure, Category = "Build Integration")
    bool IsIntegrationComplete() const { return CurrentStatus == EBuild_IntegrationStatus::Complete; }

    // Error reporting
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ReportIntegrationError(const FString& ErrorMessage);

    UFUNCTION(BlueprintPure, Category = "Build Integration")
    TArray<FString> GetIntegrationErrors() const { return IntegrationErrors; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    EBuild_IntegrationStatus CurrentStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_SystemValidation> SystemValidations;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> IntegrationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float LastValidationTime;

private:
    void ValidateModule(const FString& ModuleName);
    void ValidateSystem(const FString& SystemName, UClass* SystemClass);
    void CleanupExcessActors(int32 MaxActors);
    void LogIntegrationStatus();

    // Core module classes to validate
    TArray<FString> CoreModuleNames;
    TMap<FString, UClass*> SystemClasses;
};