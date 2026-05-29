#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "TranspersonalGameState.h"
#include "SharedTypes.h"
#include "Quest_PrehistoricQuestManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_QuestType : uint8
{
    Hunt           UMETA(DisplayName = "Hunt Dinosaur"),
    Explore        UMETA(DisplayName = "Explore Territory"),
    Craft          UMETA(DisplayName = "Craft Tools"),
    Survive        UMETA(DisplayName = "Survive Conditions"),
    Rescue         UMETA(DisplayName = "Rescue NPC"),
    Gather         UMETA(DisplayName = "Gather Resources"),
    Defend         UMETA(DisplayName = "Defend Location"),
    Migration      UMETA(DisplayName = "Follow Migration")
};

UENUM(BlueprintType)
enum class EQuest_QuestStatus : uint8
{
    NotStarted     UMETA(DisplayName = "Not Started"),
    Active         UMETA(DisplayName = "Active"),
    Completed      UMETA(DisplayName = "Completed"),
    Failed         UMETA(DisplayName = "Failed"),
    Abandoned      UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EQuest_QuestPriority : uint8
{
    Low            UMETA(DisplayName = "Low Priority"),
    Normal         UMETA(DisplayName = "Normal Priority"),
    High           UMETA(DisplayName = "High Priority"),
    Critical       UMETA(DisplayName = "Critical Priority"),
    Emergency      UMETA(DisplayName = "Emergency")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    FString ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objective")
    float CompletionRadius;

    FQuest_QuestObjective()
    {
        ObjectiveText = TEXT("Default Objective");
        ObjectiveDescription = TEXT("Complete this objective");
        bIsCompleted = false;
        bIsOptional = false;
        TargetCount = 1;
        CurrentCount = 0;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_PrehistoricQuest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestStatus QuestStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_QuestObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector QuestLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float QuestRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredLevel;

    FQuest_PrehistoricQuest()
    {
        QuestID = TEXT("QUEST_001");
        QuestTitle = TEXT("Survive the Day");
        QuestDescription = TEXT("Survive until nightfall in the prehistoric wilderness");
        QuestType = EQuest_QuestType::Survive;
        QuestStatus = EQuest_QuestStatus::NotStarted;
        Priority = EQuest_QuestPriority::Normal;
        TimeLimit = 0.0f;
        ElapsedTime = 0.0f;
        ExperienceReward = 100;
        QuestLocation = FVector::ZeroVector;
        QuestRadius = 1000.0f;
        bIsRepeatable = false;
        RequiredLevel = 1;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnQuestStarted, const FQuest_PrehistoricQuest&, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnQuestCompleted, const FQuest_PrehistoricQuest&, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnQuestFailed, const FQuest_PrehistoricQuest&, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnObjectiveUpdated, const FQuest_PrehistoricQuest&, Quest, const FQuest_QuestObjective&, Objective);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_PrehistoricQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_PrehistoricQuestManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Quest Storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_PrehistoricQuest> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_PrehistoricQuest> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_PrehistoricQuest> AvailableQuests;

    // Quest Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    bool bAutoGenerateQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float QuestGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float LastQuestGenerationTime;

    // Player Reference
    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    class ATranspersonalCharacter* PlayerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    class ATranspersonalGameState* GameState;

    // Quest Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FQuest_OnQuestStarted OnQuestStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FQuest_OnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FQuest_OnQuestFailed OnQuestFailed;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FQuest_OnObjectiveUpdated OnObjectiveUpdated;

public:
    // Quest Management Functions
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AbandonQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjective(const FString& QuestID, int32 ObjectiveIndex, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteObjective(const FString& QuestID, int32 ObjectiveIndex);

    // Quest Query Functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    TArray<FQuest_PrehistoricQuest> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    TArray<FQuest_PrehistoricQuest> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    FQuest_PrehistoricQuest GetQuestByID(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    int32 GetActiveQuestCount() const;

    // Quest Generation Functions
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GenerateRandomQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GenerateHuntQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GenerateExploreQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GenerateCraftQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GenerateSurvivalQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GenerateRescueQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GenerateGatherQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GenerateDefendQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GenerateMigrationQuest();

    // Quest Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CheckQuestCompletion();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CheckQuestFailure();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CheckLocationObjectives();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CheckTimeBasedObjectives(float DeltaTime);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void InitializeQuestSystem();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void SaveQuestProgress();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void LoadQuestProgress();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void ResetQuestSystem();

private:
    // Internal Helper Functions
    void UpdateQuestTimers(float DeltaTime);
    void ProcessQuestGeneration(float DeltaTime);
    FQuest_PrehistoricQuest* FindQuestByID(const FString& QuestID);
    bool CanStartQuest(const FQuest_PrehistoricQuest& Quest) const;
    void GiveQuestRewards(const FQuest_PrehistoricQuest& Quest);
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius) const;
    FString GenerateUniqueQuestID() const;
};