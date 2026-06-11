#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Healthy         UMETA(DisplayName = "Healthy"),
    Warning         UMETA(DisplayName = "Warning"),
    Critical        UMETA(DisplayName = "Critical"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> ErrorMessages;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        Status = EBuild_IntegrationStatus::Unknown;
        ClassCount = 0;
        ActorCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float OverallHealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 LoadedClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime LastValidationTime;

    FBuild_IntegrationReport()
    {
        OverallHealthPercentage = 0.0f;
        OverallStatus = EBuild_IntegrationStatus::Unknown;
        TotalActorsInLevel = 0;
        LoadedClassCount = 0;
        LastValidationTime = FDateTime::Now();
    }
};

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
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_IntegrationReport ValidateFullIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_ModuleStatus ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateClassLoading(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    int32 CountActorsInLevel();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    TArray<FString> GetLoadedModuleNames();

    // Health monitoring
    UFUNCTION(BlueprintCallable, Category = "Integration")
    float CalculateIntegrationHealth();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_IntegrationStatus DetermineOverallStatus(float HealthPercentage);

    // Diagnostic functions
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void LogIntegrationReport(const FBuild_IntegrationReport& Report);

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void CreateIntegrationTestActors();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateMinPlayableMap();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport LastReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bAutoValidateOnTick;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> RequiredModules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> RequiredClasses;

private:
    float TimeSinceLastValidation;
    
    // Helper functions
    bool IsModuleLoaded(const FString& ModuleName);
    int32 CountClassesInModule(const FString& ModuleName);
    TArray<FString> GetModuleErrors(const FString& ModuleName);
};