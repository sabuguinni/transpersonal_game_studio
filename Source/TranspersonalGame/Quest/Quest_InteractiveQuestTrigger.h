#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Quest_InteractiveQuestTrigger.generated.h"

UENUM(BlueprintType)
enum class EQuest_TriggerType : uint8
{
    Hunting         UMETA(DisplayName = "Hunting Quest"),
    Exploration     UMETA(DisplayName = "Exploration Quest"),
    Survival        UMETA(DisplayName = "Survival Quest"),
    Resource        UMETA(DisplayName = "Resource Quest"),
    Rescue          UMETA(DisplayName = "Rescue Quest"),
    Combat          UMETA(DisplayName = "Combat Quest"),
    Crafting        UMETA(DisplayName = "Crafting Quest")
};

UENUM(BlueprintType)
enum class EQuest_TriggerState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    int32 RequiredProgress;

    FQuest_ObjectiveData()
    {
        ObjectiveText = TEXT("Default Objective");
        bIsCompleted = false;
        CurrentProgress = 0;
        RequiredProgress = 1;
    }
};

USTRUCT(BlueprintType)
struct FQuest_RewardData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Reward")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Reward")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Reward")
    float SurvivalBonus;

    FQuest_RewardData()
    {
        ExperiencePoints = 100;
        SurvivalBonus = 0.0f;
    }
};

/**
 * Sistema de triggers de quest interactivos para o jogo de sobrevivência pré-histórico.
 * Gere activação, progressão e conclusão de quests baseadas em proximidade e interacção.
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AQuest_InteractiveQuestTrigger : public ATriggerVolume
{
    GENERATED_BODY()

public:
    AQuest_InteractiveQuestTrigger();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Quest Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Configuration")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Configuration")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Configuration")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Configuration")
    EQuest_TriggerType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Configuration")
    EQuest_TriggerState CurrentState;

    // Quest Objectives
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objectives")
    TArray<FQuest_ObjectiveData> Objectives;

    // Quest Rewards
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Rewards")
    FQuest_RewardData QuestRewards;

    // Visual Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual Components")
    UStaticMeshComponent* QuestMarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual Components")
    UPointLightComponent* QuestIndicatorLight;

    // Quest Timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Timing")
    float QuestTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Timing")
    float CurrentQuestTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Timing")
    bool bHasTimeLimit;

    // Quest Prerequisites
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Prerequisites")
    TArray<FString> RequiredCompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Prerequisites")
    int32 RequiredPlayerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Prerequisites")
    float RequiredSurvivalRating;

    // Quest Functions
    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void ActivateQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void CompleteQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void FailQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void UpdateObjectiveProgress(int32 ObjectiveIndex, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool CheckQuestPrerequisites(class ATranspersonalCharacter* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void GiveQuestRewards(class ATranspersonalCharacter* PlayerCharacter);

    // Visual Updates
    UFUNCTION(BlueprintCallable, Category = "Quest Visuals")
    void UpdateQuestVisuals();

    UFUNCTION(BlueprintCallable, Category = "Quest Visuals")
    void SetQuestLightColor(FLinearColor NewColor);

    // Trigger Events
    UFUNCTION()
    void OnPlayerEnterTrigger(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION()
    void OnPlayerExitTrigger(AActor* OverlappedActor, AActor* OtherActor);

    // Quest State Checks
    UFUNCTION(BlueprintPure, Category = "Quest State")
    bool IsQuestActive() const { return CurrentState == EQuest_TriggerState::Active; }

    UFUNCTION(BlueprintPure, Category = "Quest State")
    bool IsQuestCompleted() const { return CurrentState == EQuest_TriggerState::Completed; }

    UFUNCTION(BlueprintPure, Category = "Quest State")
    bool AreAllObjectivesCompleted() const;

    UFUNCTION(BlueprintPure, Category = "Quest State")
    float GetQuestCompletionPercentage() const;

private:
    // Internal quest management
    void InitializeQuestComponents();
    void UpdateQuestTimer(float DeltaTime);
    void CheckQuestCompletion();
    void HandleQuestTimeout();

    // Player reference for active quest tracking
    UPROPERTY()
    class ATranspersonalCharacter* ActivePlayerCharacter;

    // Quest activation tracking
    bool bPlayerInRange;
    float PlayerProximityTime;
    const float RequiredProximityTime = 2.0f; // Seconds player must stay in range to activate
};