#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "QuestSystemManager.generated.h"

// Forward declarations
class UQuest_QuestComponent;
class UQuest_ObjectiveComponent;

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    MainStory       UMETA(DisplayName = "Main Story"),
    SideQuest       UMETA(DisplayName = "Side Quest"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Survival        UMETA(DisplayName = "Survival"),
    SpiritualJourney UMETA(DisplayName = "Spiritual Journey"),
    CommunityBuilding UMETA(DisplayName = "Community Building"),
    Discovery       UMETA(DisplayName = "Discovery")
};

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EQuest_EmotionalState : uint8
{
    Curiosity       UMETA(DisplayName = "Curiosity"),
    Determination   UMETA(DisplayName = "Determination"),
    Challenge       UMETA(DisplayName = "Challenge"),
    Revelation      UMETA(DisplayName = "Revelation"),
    Fulfillment     UMETA(DisplayName = "Fulfillment"),
    Wonder          UMETA(DisplayName = "Wonder"),
    Fear            UMETA(DisplayName = "Fear"),
    Hope            UMETA(DisplayName = "Hope"),
    Transformation  UMETA(DisplayName = "Transformation")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    GoToLocation    UMETA(DisplayName = "Go To Location"),
    TalkToNPC       UMETA(DisplayName = "Talk To NPC"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    DefeatEnemy     UMETA(DisplayName = "Defeat Enemy"),
    Meditate        UMETA(DisplayName = "Meditate"),
    Observe         UMETA(DisplayName = "Observe"),
    Craft           UMETA(DisplayName = "Craft"),
    Survive         UMETA(DisplayName = "Survive"),
    Discover        UMETA(DisplayName = "Discover")
};

USTRUCT(BlueprintType)
struct FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString TargetActorName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentProgress;

    FQuest_ObjectiveData()
    {
        ObjectiveID = "";
        Description = "";
        Type = EQuest_ObjectiveType::GoToLocation;
        Status = EQuest_Status::NotStarted;
        TargetLocation = FVector::ZeroVector;
        TargetActorName = "";
        RequiredQuantity = 1;
        CurrentProgress = 0;
    }
};

USTRUCT(BlueprintType)
struct FQuest_EmotionalJourney
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Journey")
    TArray<EQuest_EmotionalState> EmotionalArc;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Journey")
    int32 CurrentEmotionalStage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Journey")
    float EmotionalIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Journey")
    FString EmotionalContext;

    FQuest_EmotionalJourney()
    {
        CurrentEmotionalStage = 0;
        EmotionalIntensity = 1.0f;
        EmotionalContext = "";
    }
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_ObjectiveData> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_EmotionalJourney EmotionalJourney;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float EstimatedDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> Rewards;

    FQuest_Data()
    {
        QuestID = "";
        Title = "";
        Description = "";
        QuestType = EQuest_Type::SideQuest;
        Status = EQuest_Status::NotStarted;
        Priority = 1;
        EstimatedDuration = 10.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnQuestStarted, const FString&, QuestID, const FQuest_Data&, QuestData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnQuestCompleted, const FString&, QuestID, const FQuest_Data&, QuestData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FQuest_OnObjectiveUpdated, const FString&, QuestID, const FString&, ObjectiveID, EQuest_Status, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnEmotionalStateChanged, EQuest_EmotionalState, NewState, float, Intensity);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AbandonQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_Data> GetCompletedQuests() const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_ObjectiveData GetCurrentObjective(const FString& QuestID) const;

    // Emotional Journey
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void AdvanceEmotionalState(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    EQuest_EmotionalState GetCurrentEmotionalState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void SetEmotionalIntensity(const FString& QuestID, float Intensity);

    // Quest Creation and Registration
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuest(const FQuest_Data& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateSpiritualJourneyQuest(const FString& QuestID, const FString& Title, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateExplorationQuest(const FString& QuestID, const FString& Title, FVector TargetLocation);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FQuest_OnQuestStarted OnQuestStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FQuest_OnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FQuest_OnObjectiveUpdated OnObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FQuest_OnEmotionalStateChanged OnEmotionalStateChanged;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TMap<FString, FQuest_Data> RegisteredQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FString> CompletedQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    bool bDebugMode;

private:
    void InitializeDefaultQuests();
    void UpdateQuestProgress(float DeltaTime);
    void CheckObjectiveCompletion();
    bool IsQuestValid(const FString& QuestID) const;
    void LogQuestEvent(const FString& Message) const;
};