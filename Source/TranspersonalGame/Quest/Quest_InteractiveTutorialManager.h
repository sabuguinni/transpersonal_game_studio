#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerBox.h"
#include "Quest_InteractiveTutorialManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_TutorialStep : uint8
{
    None UMETA(DisplayName = "None"),
    Movement UMETA(DisplayName = "Learn Movement"),
    ResourceCollection UMETA(DisplayName = "Collect Resources"),
    Crafting UMETA(DisplayName = "Craft Tools"),
    ShelterBuilding UMETA(DisplayName = "Build Shelter"),
    WaterFinding UMETA(DisplayName = "Find Water"),
    DangerAwareness UMETA(DisplayName = "Danger Awareness"),
    Completed UMETA(DisplayName = "Tutorial Completed")
};

USTRUCT(BlueprintType)
struct FQuest_TutorialObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FString HintText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    bool bIsCompleted;

    FQuest_TutorialObjective()
    {
        ObjectiveText = TEXT("Complete objective");
        HintText = TEXT("Follow the marker");
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 200.0f;
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_InteractiveTutorialManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_InteractiveTutorialManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Tutorial State Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial State")
    EQuest_TutorialStep CurrentStep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial State")
    TArray<FQuest_TutorialObjective> TutorialObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial State")
    float TutorialTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial State")
    bool bTutorialActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial State")
    bool bShowHints;

    // Player Reference
    UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
    class ATranspersonalCharacter* PlayerCharacter;

    // Tutorial Functions
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void StartTutorial();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void AdvanceToNextStep();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void CompleteCurrentObjective();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void ShowTutorialMessage(const FString& Message, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void UpdateObjectiveProgress(const FString& ObjectiveName, float Progress);

    // Step-specific functions
    UFUNCTION(BlueprintCallable, Category = "Tutorial Steps")
    void InitializeMovementTutorial();

    UFUNCTION(BlueprintCallable, Category = "Tutorial Steps")
    void InitializeResourceCollectionTutorial();

    UFUNCTION(BlueprintCallable, Category = "Tutorial Steps")
    void InitializeCraftingTutorial();

    UFUNCTION(BlueprintCallable, Category = "Tutorial Steps")
    void InitializeShelterBuildingTutorial();

    UFUNCTION(BlueprintCallable, Category = "Tutorial Steps")
    void InitializeWaterFindingTutorial();

    UFUNCTION(BlueprintCallable, Category = "Tutorial Steps")
    void InitializeDangerAwarenessTutorial();

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "Tutorial Validation")
    bool ValidateMovementCompletion();

    UFUNCTION(BlueprintCallable, Category = "Tutorial Validation")
    bool ValidateResourceCollection();

    UFUNCTION(BlueprintCallable, Category = "Tutorial Validation")
    bool ValidateCraftingCompletion();

    UFUNCTION(BlueprintCallable, Category = "Tutorial Validation")
    bool ValidateShelterBuilding();

    UFUNCTION(BlueprintCallable, Category = "Tutorial Validation")
    bool ValidateWaterFinding();

    // Event handlers
    UFUNCTION()
    void OnPlayerEnterTrigger(class AActor* OverlappedActor, class AActor* OtherActor);

    UFUNCTION()
    void OnPlayerExitTrigger(class AActor* OverlappedActor, class AActor* OtherActor);

    // Tutorial completion
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void CompleteTutorial();

    UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial")
    void OnTutorialStepChanged(EQuest_TutorialStep NewStep);

    UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial")
    void OnTutorialCompleted();

private:
    void UpdateCurrentObjective();
    void CheckObjectiveCompletion();
    void SetupTutorialObjectives();
};