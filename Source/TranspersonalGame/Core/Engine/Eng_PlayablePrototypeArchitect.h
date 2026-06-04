#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Eng_PlayablePrototypeArchitect.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PrototypeRequirement
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString RequirementName;

    UPROPERTY(BlueprintReadOnly)
    FString Description;

    UPROPERTY(BlueprintReadOnly)
    bool bIsCompleted;

    UPROPERTY(BlueprintReadOnly)
    float CompletionPercentage;

    UPROPERTY(BlueprintReadOnly)
    FString ResponsibleAgent;

    FEng_PrototypeRequirement()
    {
        RequirementName = TEXT("");
        Description = TEXT("");
        bIsCompleted = false;
        CompletionPercentage = 0.0f;
        ResponsibleAgent = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_MilestoneData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString MilestoneName;

    UPROPERTY(BlueprintReadOnly)
    TArray<FEng_PrototypeRequirement> Requirements;

    UPROPERTY(BlueprintReadOnly)
    float OverallProgress;

    UPROPERTY(BlueprintReadOnly)
    bool bIsMilestoneComplete;

    FEng_MilestoneData()
    {
        MilestoneName = TEXT("");
        OverallProgress = 0.0f;
        bIsMilestoneComplete = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_PlayablePrototypeArchitect : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_PlayablePrototypeArchitect();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core milestone tracking
    UPROPERTY(BlueprintReadOnly, Category = "Prototype Architecture")
    FEng_MilestoneData WalkAroundMilestone;

    UPROPERTY(BlueprintReadOnly, Category = "Prototype Architecture")
    TArray<FString> CriticalCompilationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Prototype Architecture")
    TArray<FString> ArchitectureViolations;

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    void ValidateWalkAroundMilestone();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool ValidateCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool ValidateTerrainSystem();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool ValidateLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool ValidateDinosaurPlacements();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool ValidateCameraSystem();

    // Architecture enforcement
    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    void EnforceArchitecturalStandards();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    void ValidateCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    float CalculateOverallProgress();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    FString GenerateProgressReport();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    void LogArchitectureStatus();

private:
    void InitializeWalkAroundRequirements();
    void UpdateRequirementProgress(const FString& RequirementName, float Progress, bool bCompleted);
    bool CheckActorExistence(const FString& ActorClass);
    int32 CountActorsOfClass(const FString& ActorClass);
    void ValidateSystemIntegration();

    UPROPERTY()
    float LastValidationTime;

    UPROPERTY()
    bool bValidationInProgress;
};