#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerVolume.h"
#include "TimerManager.h"
#include "Quest_SurvivalTutorialSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_TutorialStep : uint8
{
    None,
    Movement,
    ResourceGathering,
    CraftingBasics,
    ShelterBuilding,
    DinosaurAwareness,
    Completed
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
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    float TimeLimit;

    FQuest_TutorialObjective()
    {
        ObjectiveText = TEXT("");
        HintText = TEXT("");
        bIsCompleted = false;
        TimeLimit = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalTutorialSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalTutorialSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Tutorial progression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    EQuest_TutorialStep CurrentStep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TMap<EQuest_TutorialStep, FQuest_TutorialObjective> TutorialObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    bool bTutorialActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    float TutorialTimer;

    // Tutorial functions
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void StartTutorial();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void CompleteCurrentStep();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void AdvanceToNextStep();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void ShowTutorialHint(const FString& HintText);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    bool CheckMovementObjective();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    bool CheckResourceGatheringObjective();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    bool CheckCraftingObjective();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void OnPlayerMovement();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void OnResourceCollected(const FString& ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void OnItemCrafted(const FString& ItemName);

    // Tutorial UI
    UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial")
    void DisplayTutorialMessage(const FString& Message);

    UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial")
    void UpdateTutorialProgress(float Progress);

    UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial")
    void ShowTutorialComplete();

private:
    void InitializeTutorialObjectives();
    void UpdateTutorialProgress();
    void CheckTutorialCompletion();

    // Movement tracking
    FVector LastPlayerPosition;
    float MovementDistance;
    bool bPlayerHasMoved;

    // Resource tracking
    TArray<FString> CollectedResources;
    int32 RequiredResourceCount;

    // Crafting tracking
    TArray<FString> CraftedItems;
    bool bFirstItemCrafted;

    // Timer handles
    FTimerHandle TutorialTimerHandle;
    FTimerHandle HintTimerHandle;
};