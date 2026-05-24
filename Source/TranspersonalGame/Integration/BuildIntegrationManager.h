#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "BuildIntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Success     UMETA(DisplayName = "Success"),
    Failed      UMETA(DisplayName = "Failed"),
    Warning     UMETA(DisplayName = "Warning")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float ValidationTime;

    FBuild_SystemReport()
    {
        SystemName = TEXT("");
        Status = EBuild_IntegrationStatus::Pending;
        ActorCount = 0;
        ErrorMessage = TEXT("");
        ValidationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_BiomeValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FVector BiomeCenter;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorsInBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 EnvironmentCount;

    FBuild_BiomeValidation()
    {
        BiomeName = TEXT("");
        BiomeCenter = FVector::ZeroVector;
        ActorsInBiome = 0;
        DinosaurCount = 0;
        EnvironmentCount = 0;
    }
};

/**
 * Build Integration Manager - Coordinates integration between all game systems
 * Validates cross-system compatibility and maintains build stability
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
    bool ValidateSystemIntegration(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemReport> GetSystemReports() const;

    // Biome validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_BiomeValidation> GetBiomeValidation() const;

    // Actor management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetTotalActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetActorCountByType(const FString& ActorType) const;

    // Build status
    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_IntegrationStatus GetOverallBuildStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FString GetBuildStatusMessage() const;

    // Map persistence
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool SaveCurrentMap();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool LoadMap(const FString& MapPath);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetMemoryUsageMB() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemReport> SystemReports;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_BiomeValidation> BiomeValidations;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString StatusMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastValidationTime;

private:
    // Internal validation methods
    bool ValidateWorldGeneration();
    bool ValidateCharacterSystems();
    bool ValidateDinosaurSystems();
    bool ValidateVFXSystems();
    bool ValidateAudioSystems();
    bool ValidateQuestSystems();

    // Helper methods
    void UpdateSystemReport(const FString& SystemName, EBuild_IntegrationStatus Status, const FString& Message = TEXT(""));
    void CalculateOverallStatus();
    FVector GetBiomeCenter(const FString& BiomeName) const;
};