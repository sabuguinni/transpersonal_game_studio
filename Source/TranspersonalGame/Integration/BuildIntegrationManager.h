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
    Operational UMETA(DisplayName = "Operational"),
    Failed      UMETA(DisplayName = "Failed"),
    Disabled    UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct FBuild_SystemReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float PerformanceScore;

    FBuild_SystemReport()
    {
        SystemName = TEXT("");
        Status = EBuild_SystemStatus::Unknown;
        ErrorMessage = TEXT("");
        ActorCount = 0;
        PerformanceScore = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Integration validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemReport ValidateSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemReport> GetSystemReports() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemOperational(const FString& SystemName) const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetOverallPerformanceScore() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetTotalActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void OptimizePerformance();

    // Build management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool SaveCurrentBuild();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateBuildReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemReport> SystemReports;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsValidating;

private:
    void ValidateCharacterSystem();
    void ValidateWorldGeneration();
    void ValidateVFXSystem();
    void ValidatePhysicsSystem();
    void ValidateAISystem();
    void ValidateQAFramework();

    FBuild_SystemReport CreateSystemReport(const FString& SystemName, EBuild_SystemStatus Status, const FString& ErrorMsg = TEXT(""));
    void UpdateSystemReport(const FString& SystemName, const FBuild_SystemReport& Report);
    float CalculatePerformanceScore(const FBuild_SystemReport& Report) const;
};