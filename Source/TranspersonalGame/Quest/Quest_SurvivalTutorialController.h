#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalTutorialController.generated.h"

class ACharacter;
class UQuest_SurvivalTutorialController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTutorialStepCompleted, int32, StepIndex, FString, StepDescription);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTutorialCompleted, UQuest_SurvivalTutorialController*, TutorialController);

UENUM(BlueprintType)
enum class EQuest_TutorialStep : uint8
{
    None            UMETA(DisplayName = "None"),
    Movement        UMETA(DisplayName = "Learn Movement"),
    LookAround      UMETA(DisplayName = "Look Around"),
    CheckStats      UMETA(DisplayName = "Check Survival Stats"),
    FindWater       UMETA(DisplayName = "Find Water Source"),
    GatherSticks    UMETA(DisplayName = "Gather Sticks"),
    GatherStones    UMETA(DisplayName = "Gather Stones"),
    CraftTool       UMETA(DisplayName = "Craft Stone Tool"),
    HuntSmallPrey   UMETA(DisplayName = "Hunt Small Prey"),
    BuildShelter    UMETA(DisplayName = "Build Basic Shelter"),
    MakeFire        UMETA(DisplayName = "Make Fire"),
    CookFood        UMETA(DisplayName = "Cook Food"),
    AvoidPredator   UMETA(DisplayName = "Avoid Predator"),
    Completed       UMETA(DisplayName = "Tutorial Completed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_TutorialStepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    EQuest_TutorialStep StepType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FString StepTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FString StepDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FString StepInstructions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    int32 ExperienceReward;

    FQuest_TutorialStepData()
    {
        StepType = EQuest_TutorialStep::None;
        StepTitle = TEXT("");
        StepDescription = TEXT("");
        StepInstructions = TEXT("");
        TimeLimit = 0.0f;
        bIsOptional = false;
        ExperienceReward = 10;
    }
};

/**
 * Survival Tutorial Controller - Guides new players through essential survival mechanics
 * Teaches movement, resource gathering, crafting, hunting, and basic survival strategies
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_SurvivalTutorialController : public UObject
{
    GENERATED_BODY()

public:
    UQuest_SurvivalTutorialController();

    // Tutorial Management
    UFUNCTION(BlueprintCallable, Category = "Survival Tutorial")
    void StartTutorial(ACharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category = "Survival Tutorial")
    void StopTutorial();

    UFUNCTION(BlueprintCallable, Category = "Survival Tutorial")
    void NextStep();

    UFUNCTION(BlueprintCallable, Category = "Survival Tutorial")
    void CompleteCurrentStep();

    UFUNCTION(BlueprintCallable, Category = "Survival Tutorial")
    void SkipCurrentStep();

    // Step Validation
    UFUNCTION(BlueprintCallable, Category = "Survival Tutorial")
    bool ValidateStepCompletion(EQuest_TutorialStep StepType);

    UFUNCTION(BlueprintCallable, Category = "Survival Tutorial")
    void CheckStepProgress();

    // Tutorial State
    UFUNCTION(BlueprintPure, Category = "Survival Tutorial")
    bool IsTutorialActive() const { return bIsTutorialActive; }

    UFUNCTION(BlueprintPure, Category = "Survival Tutorial")
    EQuest_TutorialStep GetCurrentStep() const { return CurrentStep; }

    UFUNCTION(BlueprintPure, Category = "Survival Tutorial")
    int32 GetCurrentStepIndex() const { return CurrentStepIndex; }

    UFUNCTION(BlueprintPure, Category = "Survival Tutorial")
    float GetStepProgress() const { return StepProgress; }

    UFUNCTION(BlueprintPure, Category = "Survival Tutorial")
    FString GetCurrentStepInstructions() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Survival Tutorial")
    FOnTutorialStepCompleted OnStepCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Survival Tutorial")
    FOnTutorialCompleted OnTutorialCompleted;

protected:
    // Tutorial State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial State")
    bool bIsTutorialActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial State")
    EQuest_TutorialStep CurrentStep;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial State")
    int32 CurrentStepIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial State")
    float StepProgress;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial State")
    float StepStartTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial State")
    TWeakObjectPtr<ACharacter> PlayerCharacterRef;

    // Tutorial Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial Config")
    TArray<FQuest_TutorialStepData> TutorialSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial Config")
    bool bShowVisualHints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial Config")
    bool bAllowSkipping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial Config")
    float HintDisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial Config")
    int32 TotalExperienceReward;

    // Step Validation Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    FVector LastPlayerPosition;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    float MovementDistance;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    int32 ItemsGathered;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    bool bHasCraftedItem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation")
    bool bHasBuiltShelter;

private:
    // Internal Methods
    void InitializeTutorialSteps();
    void ShowStepInstructions();
    void HideStepInstructions();
    void UpdateStepProgress();
    
    // Step Validation Methods
    bool ValidateMovementStep();
    bool ValidateLookAroundStep();
    bool ValidateStatsCheckStep();
    bool ValidateWaterFindingStep();
    bool ValidateGatheringStep(const FString& ItemType);
    bool ValidateCraftingStep();
    bool ValidateHuntingStep();
    bool ValidateShelterStep();
    bool ValidateFireStep();
    bool ValidateCookingStep();
    bool ValidateAvoidanceStep();

    // Helper Methods
    void RewardPlayer(int32 Experience);
    void DisplayMessage(const FString& Message, float Duration = 5.0f);
    void DisplayHint(const FString& Hint);
    FString GetStepInstructions(EQuest_TutorialStep StepType) const;
};