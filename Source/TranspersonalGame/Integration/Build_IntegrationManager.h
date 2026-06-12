#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_IntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_ValidationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastCheckTime;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("");
        Status = EBuild_ValidationStatus::Unknown;
        ErrorMessage = TEXT("");
        LastCheckTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActiveSystemCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float BuildTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsPlayable;

    FBuild_IntegrationReport()
    {
        TotalActorCount = 0;
        ActiveSystemCount = 0;
        BuildTime = 0.0f;
        bIsPlayable = false;
    }
};

/**
 * Integration Manager - Orchestrates all game systems and validates build integrity
 * Ensures all 18 agent outputs work together as a cohesive game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_IntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void EnforceActorCap(int32 MaxActors = 8000);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemStatus> GetSystemStatuses();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsGamePlayable();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Integration")
    void RunFullIntegrationTest();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemStatus> CachedSystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIntegrationValid;

private:
    void ValidateWorldGeneration();
    void ValidateCharacterSystems();
    void ValidateAISystems();
    void ValidateQASystems();
    void UpdateSystemStatus(const FString& SystemName, EBuild_ValidationStatus Status, const FString& ErrorMessage = TEXT(""));
};