#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/TriggerVolume.h"
#include "Quest_SurvivalCraftingTutorial.generated.h"

UENUM(BlueprintType)
enum class EQuest_CraftingTutorialStage : uint8
{
    NotStarted UMETA(DisplayName = "Not Started"),
    CollectSticks UMETA(DisplayName = "Collect Sticks"),
    CollectStones UMETA(DisplayName = "Collect Stones"),
    CraftAxe UMETA(DisplayName = "Craft Stone Axe"),
    UseAxe UMETA(DisplayName = "Use Axe on Tree"),
    BuildShelter UMETA(DisplayName = "Build Basic Shelter"),
    Completed UMETA(DisplayName = "Tutorial Completed")
};

USTRUCT(BlueprintType)
struct FQuest_CraftingStep
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Step")
    FString StepName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Step")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Step")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Step")
    int32 CurrentAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Step")
    bool bIsCompleted;

    FQuest_CraftingStep()
    {
        StepName = TEXT("Unknown Step");
        Description = TEXT("No description");
        RequiredAmount = 1;
        CurrentAmount = 0;
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalCraftingTutorial : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalCraftingTutorial();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* QuestMarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* QuestText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest State")
    EQuest_CraftingTutorialStage CurrentStage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Steps")
    TArray<FQuest_CraftingStep> CraftingSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    FLinearColor ActiveColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    FLinearColor CompletedColor;

public:
    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void StartTutorial();

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void AdvanceStage();

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void UpdateStepProgress(const FString& StepName, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool IsStepCompleted(const FString& StepName) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    FString GetCurrentObjective() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnTutorialStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnStageAdvanced(EQuest_CraftingTutorialStage NewStage);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnTutorialCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnPlayerEnteredTrigger(class APawn* Player);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnPlayerExitedTrigger(class APawn* Player);

protected:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    void InitializeCraftingSteps();
    void UpdateQuestDisplay();
    void SetMarkerColor(const FLinearColor& Color);

private:
    bool bPlayerInRange;
    class APawn* CurrentPlayer;
};