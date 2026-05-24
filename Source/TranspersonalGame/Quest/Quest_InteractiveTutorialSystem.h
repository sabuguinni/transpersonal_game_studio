#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerVolume.h"
#include "Quest_InteractiveTutorialSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_TutorialStep : uint8
{
    None            UMETA(DisplayName = "None"),
    GatherResources UMETA(DisplayName = "Gather Resources"),
    CraftTool       UMETA(DisplayName = "Craft Tool"),
    FindWater       UMETA(DisplayName = "Find Water"),
    BuildShelter    UMETA(DisplayName = "Build Shelter"),
    HuntPrey        UMETA(DisplayName = "Hunt Prey"),
    SurviveNight    UMETA(DisplayName = "Survive Night"),
    Completed       UMETA(DisplayName = "Completed")
};

USTRUCT(BlueprintType)
struct FQuest_TutorialObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    EQuest_TutorialStep RequiredStep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    bool bIsCompleted;

    FQuest_TutorialObjective()
    {
        ObjectiveText = TEXT("");
        RequiredStep = EQuest_TutorialStep::None;
        RequiredCount = 1;
        CurrentCount = 0;
        bIsCompleted = false;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AQuest_InteractiveTutorialSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_InteractiveTutorialSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Tutorial Management
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void StartTutorial();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void AdvanceTutorialStep();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void CompleteObjective(EQuest_TutorialStep Step, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    bool IsObjectiveCompleted(EQuest_TutorialStep Step) const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    FString GetCurrentObjectiveText() const;

    // Resource Collection
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void OnResourceCollected(const FString& ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void OnToolCrafted(const FString& ToolType);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void OnWaterSourceFound();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void OnShelterBuilt();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void OnPreyHunted();

protected:
    // Tutorial State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial")
    EQuest_TutorialStep CurrentStep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TArray<FQuest_TutorialObjective> TutorialObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    bool bTutorialActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    bool bTutorialCompleted;

    // Audio Cues
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString ElderSurvivorAudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString ScoutRangerAudioURL;

    // Resource Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    int32 StonesCollected;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    int32 SticksCollected;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resources")
    int32 ToolsCrafted;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progress")
    bool bWaterSourceFound;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progress")
    bool bShelterBuilt;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progress")
    bool bPreyHunted;

private:
    void InitializeTutorialObjectives();
    void UpdateObjectiveProgress();
    void CheckTutorialCompletion();
    void PlayAudioCue(const FString& AudioURL);
};