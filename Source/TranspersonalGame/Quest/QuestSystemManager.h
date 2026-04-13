#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "QuestSystemManager.generated.h"

// Forward declarations
class UQuest_QuestComponent;
class UQuest_ObjectiveComponent;

UENUM(BlueprintType)
enum class EQuest_QuestType : uint8
{
    MainStory       UMETA(DisplayName = "Main Story"),
    SideQuest       UMETA(DisplayName = "Side Quest"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Survival        UMETA(DisplayName = "Survival"),
    SpiritualJourney UMETA(DisplayName = "Spiritual Journey"),
    CommunityBuilding UMETA(DisplayName = "Community Building"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Meditation      UMETA(DisplayName = "Meditation"),
    Crafting        UMETA(DisplayName = "Crafting")
};

UENUM(BlueprintType)
enum class EQuest_QuestStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    TurnedIn        UMETA(DisplayName = "Turned In")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    KillTarget      UMETA(DisplayName = "Kill Target"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    TalkToNPC       UMETA(DisplayName = "Talk to NPC"),
    Meditation      UMETA(DisplayName = "Meditation"),
    CraftItem       UMETA(DisplayName = "Craft Item"),
    SurviveTime     UMETA(DisplayName = "Survive Time"),
    DiscoverSecret  UMETA(DisplayName = "Discover Secret"),
    BuildStructure  UMETA(DisplayName = "Build Structure")
};

UENUM(BlueprintType)
enum class EQuest_EmotionalJourney : uint8
{
    Wonder          UMETA(DisplayName = "Wonder"),
    Fear            UMETA(DisplayName = "Fear"),
    Hope            UMETA(DisplayName = "Hope"),
    Determination   UMETA(DisplayName = "Determination"),
    Enlightenment   UMETA(DisplayName = "Enlightenment"),
    Connection      UMETA(DisplayName = "Connection"),
    Growth          UMETA(DisplayName = "Growth"),
    Transcendence   UMETA(DisplayName = "Transcendence")
};

USTRUCT(BlueprintType)
struct FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString TargetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsOptional;

    FQuest_ObjectiveData()
    {
        ObjectiveID = "";
        Description = FText::GetEmpty();
        Type = EQuest_ObjectiveType::ReachLocation;
        TargetID = "";
        RequiredAmount = 1;
        CurrentAmount = 0;
        bIsCompleted = false;
        bIsOptional = false;
    }
};

USTRUCT(BlueprintType)
struct FQuest_QuestData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText LoreText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_EmotionalJourney EmotionalTheme;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_ObjectiveData> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float SpiritualGrowthReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float EstimatedDurationMinutes;

    FQuest_QuestData()
    {
        QuestID = "";
        Title = FText::GetEmpty();
        Description = FText::GetEmpty();
        LoreText = FText::GetEmpty();
        Type = EQuest_QuestType::SideQuest;
        EmotionalTheme = EQuest_EmotionalJourney::Wonder;
        ExperienceReward = 0;
        SpiritualGrowthReward = 0.0f;
        bIsRepeatable = false;
        EstimatedDurationMinutes = 15.0f;
    }
};

USTRUCT(BlueprintType)
struct FQuest_ActiveQuest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Quest")
    FQuest_QuestData QuestData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Quest")
    EQuest_QuestStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Quest")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Quest")
    float CompletionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Quest")
    int32 CompletionCount;

    FQuest_ActiveQuest()
    {
        Status = EQuest_QuestStatus::NotStarted;
        StartTime = 0.0f;
        CompletionTime = 0.0f;
        CompletionCount = 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnQuestStatusChanged, const FString&, QuestID, EQuest_QuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FQuest_OnObjectiveUpdated, const FString&, QuestID, const FString&, ObjectiveID, int32, NewAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnQuestCompleted, const FString&, QuestID);

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
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    EQuest_QuestStatus GetQuestStatus(const FString& QuestID) const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    int32 GetObjectiveProgress(const FString& QuestID, const FString& ObjectiveID) const;

    // Quest Discovery
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FString> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FString> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FString> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData GetQuestData(const FString& QuestID) const;

    // Emotional Journey Tracking
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void TrackEmotionalMoment(const FString& QuestID, EQuest_EmotionalJourney Emotion, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    float GetSpiritualGrowthLevel() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FQuest_OnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FQuest_OnObjectiveUpdated OnObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FQuest_OnQuestCompleted OnQuestCompleted;

protected:
    // Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    UDataTable* QuestDataTable;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TMap<FString, FQuest_ActiveQuest> ActiveQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TArray<FString> CompletedQuestIDs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    float TotalSpiritualGrowth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TMap<EQuest_EmotionalJourney, float> EmotionalJourneyProgress;

private:
    // Internal methods
    bool CanStartQuest(const FString& QuestID) const;
    bool ArePrerequisitesMet(const TArray<FString>& Prerequisites) const;
    void CheckQuestCompletion(const FString& QuestID);
    void GiveQuestRewards(const FQuest_QuestData& QuestData);
    void UpdateEmotionalJourney(EQuest_EmotionalJourney Emotion, float Intensity);
};