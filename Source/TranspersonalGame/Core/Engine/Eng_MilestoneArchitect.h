#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_MilestoneArchitect.generated.h"

UENUM(BlueprintType)
enum class EEng_MilestoneType : uint8
{
    WalkAround      UMETA(DisplayName = "Walk Around"),
    BasicSurvival   UMETA(DisplayName = "Basic Survival"),
    DinosaurInteraction UMETA(DisplayName = "Dinosaur Interaction"),
    CraftingSystem  UMETA(DisplayName = "Crafting System"),
    WorldExploration UMETA(DisplayName = "World Exploration")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_MilestoneRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString RequirementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    FEng_MilestoneRequirement()
    {
        RequirementName = TEXT("Default Requirement");
        bIsCompleted = false;
        CompletionPercentage = 0.0f;
        Description = TEXT("Default milestone requirement");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_MilestoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EEng_MilestoneType MilestoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FEng_MilestoneRequirement> Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 Priority;

    FEng_MilestoneData()
    {
        MilestoneType = EEng_MilestoneType::WalkAround;
        MilestoneName = TEXT("Default Milestone");
        OverallProgress = 0.0f;
        bIsActive = false;
        Priority = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_MilestoneArchitect : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_MilestoneArchitect();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Milestone Architecture")
    void InitializeMilestones();

    UFUNCTION(BlueprintCallable, Category = "Milestone Architecture")
    bool ValidateWalkAroundMilestone();

    UFUNCTION(BlueprintCallable, Category = "Milestone Architecture")
    bool ValidateTerrainRequirements();

    UFUNCTION(BlueprintCallable, Category = "Milestone Architecture")
    bool ValidateCharacterRequirements();

    UFUNCTION(BlueprintCallable, Category = "Milestone Architecture")
    bool ValidateDinosaurRequirements();

    UFUNCTION(BlueprintCallable, Category = "Milestone Architecture")
    bool ValidateLightingRequirements();

    UFUNCTION(BlueprintCallable, Category = "Milestone Architecture")
    void UpdateMilestoneProgress(EEng_MilestoneType MilestoneType, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Milestone Architecture")
    float GetMilestoneProgress(EEng_MilestoneType MilestoneType);

    UFUNCTION(BlueprintCallable, Category = "Milestone Architecture")
    TArray<FEng_MilestoneData> GetAllMilestones();

    UFUNCTION(BlueprintCallable, Category = "Milestone Architecture")
    FEng_MilestoneData GetCurrentMilestone();

    // Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Milestone Architecture")
    bool CheckPlayerCanMove();

    UFUNCTION(BlueprintCallable, Category = "Milestone Architecture")
    bool CheckTerrainHasVariation();

    UFUNCTION(BlueprintCallable, Category = "Milestone Architecture")
    bool CheckDinosaursExist();

    UFUNCTION(BlueprintCallable, Category = "Milestone Architecture")
    bool CheckLightingSetup();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Milestone Architecture")
    FString GenerateMilestoneReport();

    UFUNCTION(BlueprintCallable, Category = "Milestone Architecture", CallInEditor = true)
    void LogMilestoneStatus();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone Architecture")
    TArray<FEng_MilestoneData> Milestones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone Architecture")
    EEng_MilestoneType CurrentActiveMilestone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone Architecture")
    float WalkAroundProgress;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone Architecture")
    bool bTerrainValid;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone Architecture")
    bool bCharacterValid;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone Architecture")
    bool bDinosaursValid;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone Architecture")
    bool bLightingValid;

private:
    void SetupWalkAroundMilestone();
    void ValidateWorldActors();
    void CheckCharacterMovement();
    void AnalyzeTerrainGeometry();
    void CountDinosaurActors();
    void ValidateLightingSources();
};