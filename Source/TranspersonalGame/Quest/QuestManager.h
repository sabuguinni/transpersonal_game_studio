#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

// ============================================================
// Enums — Quest & Mission Designer Agent #14
// ============================================================

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Locked      UMETA(DisplayName = "Locked")
};

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    MainStory       UMETA(DisplayName = "Main Story"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Defense         UMETA(DisplayName = "Defense"),
    Rescue          UMETA(DisplayName = "Rescue"),
    Migration       UMETA(DisplayName = "Migration"),
    Survival        UMETA(DisplayName = "Survival")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation       UMETA(DisplayName = "Reach Location"),
    KillTarget          UMETA(DisplayName = "Kill Target"),
    CollectItem         UMETA(DisplayName = "Collect Item"),
    CraftItem           UMETA(DisplayName = "Craft Item"),
    ProtectNPC          UMETA(DisplayName = "Protect NPC"),
    TrackAnimal         UMETA(DisplayName = "Track Animal"),
    SurviveDuration     UMETA(DisplayName = "Survive Duration"),
    ObserveEvent        UMETA(DisplayName = "Observe Event")
};

// ============================================================
// Structs
// ============================================================

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float LocationRadius;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , RequiredCount(1)
        , CurrentCount(0)
        , bIsCompleted(false)
        , TargetLocation(FVector::ZeroVector)
        , LocationRadius(500.f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString UnlockedQuestID;

    FQuest_Reward()
        : ExperiencePoints(0)
        , UnlockedQuestID(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Data
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
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString GiverNPCLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsRepeatable;

    FQuest_Data()
        : QuestID(TEXT(""))
        , Title(TEXT(""))
        , Description(TEXT(""))
        , QuestType(EQuest_Type::Survival)
        , Status(EQuest_Status::Inactive)
        , GiverNPCLabel(TEXT(""))
        , bIsRepeatable(false)
    {}
};

// ============================================================
// UQuestManager — Actor Component managing all active quests
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Quest Lifecycle ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AbandonQuest(const FString& QuestID);

    // ---- Objective Progress ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerReachedLocation(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerKilledDinosaur(const FString& DinosaurSpecies);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerCollectedItem(const FString& ItemID, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerCraftedItem(const FString& ItemID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnHerdFleeTriggered(FVector ThreatLocation);

    // ---- Query ----

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // ---- Default Quest Library ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterDefaultQuests();

    // ---- Data ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Data> QuestLibrary;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FString> CompletedQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 MaxActiveQuests;

private:
    FQuest_Data* FindQuestByID(const FString& QuestID);
    void CheckAllObjectivesComplete(FQuest_Data& Quest);
    void GrantReward(const FQuest_Reward& Reward);
};
