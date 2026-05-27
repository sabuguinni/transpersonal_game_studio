#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "BuildIntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_SystemStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Loading     UMETA(DisplayName = "Loading"),
    Ready       UMETA(DisplayName = "Ready"),
    Error       UMETA(DisplayName = "Error"),
    Disabled    UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Report")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System Report")
    EBuild_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "System Report")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "System Report")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "System Report")
    float PerformanceScore;

    FBuild_SystemReport()
    {
        SystemName = TEXT("");
        Status = EBuild_SystemStatus::Unknown;
        ActorCount = 0;
        LastError = TEXT("");
        PerformanceScore = 0.0f;
    }
};

/**
 * Build Integration Manager - Coordinates all game systems and validates integration
 * Ensures all agent outputs work together cohesively
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Integration validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemReport> GetSystemReports();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool SaveMapSafely(const FString& MapPath);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetTotalActorCount();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void PerformGarbageCollection();

    // System status tracking
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterSystemStatus(const FString& SystemName, EBuild_SystemStatus Status, int32 ActorCount = 0);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_SystemStatus GetSystemStatus(const FString& SystemName);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetOverallPerformanceScore();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemOverloaded();

protected:
    // System tracking
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TMap<FString, FBuild_SystemReport> SystemReports;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bAllSystemsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastValidationTime;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceThreshold;

private:
    // Internal validation methods
    bool ValidateCharacterSystem();
    bool ValidateDinosaurSystem();
    bool ValidateEnvironmentSystem();
    bool ValidateAudioSystem();
    bool ValidateVFXSystem();
    
    void UpdateSystemReport(const FString& SystemName, EBuild_SystemStatus Status, int32 ActorCount, const FString& ErrorMessage = TEXT(""));
    float CalculatePerformanceScore();
};