#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_SystemStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Pending     UMETA(DisplayName = "Pending"),
    Active      UMETA(DisplayName = "Active"),
    Validated   UMETA(DisplayName = "Validated"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    EBuild_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "System Info")
    FString ValidationDetails;

    FBuild_SystemInfo()
    {
        SystemName = TEXT("Unknown");
        Status = EBuild_SystemStatus::Unknown;
        ActorCount = 0;
        LastValidationTime = 0.0f;
        ValidationDetails = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    TArray<FBuild_SystemInfo> SystemReports;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    int32 ValidatedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    float BuildHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    FString BuildStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    FDateTime LastIntegrationTime;

    FBuild_IntegrationReport()
    {
        TotalActors = 0;
        ValidatedSystems = 0;
        BuildHealth = 0.0f;
        BuildStatus = TEXT("Unknown");
        LastIntegrationTime = FDateTime::Now();
    }
};

/**
 * Final Build Integration Orchestrator
 * Coordinates all game systems and validates build integrity
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RunFullSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateSystemIntegrity(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RegisterSystemForValidation(const FString& SystemName, int32 ExpectedActorCount);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    float CalculateBuildHealth();

    // System Status Management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    EBuild_SystemStatus GetSystemStatus(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SetSystemStatus(const FString& SystemName, EBuild_SystemStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FString> GetFailedSystems();

    // Build Orchestration
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void OrchestrateFullBuild();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsBuildReady();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ForceBuildValidation();

protected:
    // System Registry
    UPROPERTY(BlueprintReadOnly, Category = "Integration Data")
    TMap<FString, FBuild_SystemInfo> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Data")
    FBuild_IntegrationReport LastIntegrationReport;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Data")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Data")
    bool bIsValidationInProgress;

    // Core System Names
    UPROPERTY(BlueprintReadOnly, Category = "Integration Data")
    TArray<FString> CoreSystemNames;

private:
    // Internal validation functions
    void ValidateCharacterSystem();
    void ValidateEnvironmentSystem();
    void ValidateLightingSystem();
    void ValidateVFXSystem();
    void ValidateAudioSystem();
    void ValidatePhysicsSystem();
    void ValidateAISystem();

    // Utility functions
    int32 CountActorsOfType(const FString& ActorTypeName);
    void LogSystemValidation(const FString& SystemName, bool bSuccess, const FString& Details);
    void UpdateSystemInfo(const FString& SystemName, EBuild_SystemStatus Status, int32 ActorCount, const FString& Details);
};