#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "QuestSystemManager.generated.h"

// ============================================================
// Quest & Mission Designer — Agent #14
// QuestSystemManager.h — Core quest management system
// Dinosaur survival game — realistic prehistoric world
// ============================================================

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Abandoned   UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation       UMETA(DisplayName = "Reach Location"),
    CollectItem         UMETA(DisplayName = "Collect Item"),
    KillTarget          UMETA(DisplayName = "Kill Target"),
    SurviveTime         UMETA(DisplayName = "Survive Time"),
    EscapeArea          UMETA(DisplayName = "Escape Area"),
    TrackAnimal         UMETA(DisplayName = "Track Animal"),
    RescueNPC           UMETA(DisplayName = "Rescue NPC"),
    CraftItem           UMETA(DisplayName = "Craft Item"),
    DefendLocation      UMETA(DisplayName = "Defend Location"),
    FollowHerd          UMETA(DisplayName = "Follow Herd")
};

UENUM(BlueprintType)
enum class EQuest_Category : uint8
{
    MainStory       UMETA(DisplayName = "Main Story"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Survival        UMETA(DisplayName = "Survival"),
    Rescue          UMETA(DisplayName = "Rescue"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Migration       UMETA(DisplayName = "Migration"),
    Defense         UMETA(DisplayName = "Defense")
};

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TargetActorTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;  // 0 = no time limit

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , TargetLocation(FVector::ZeroVector)
        , TargetActorTag(TEXT(""))
        , RequiredCount(1)
        , CurrentCount(0)
        , bIsCompleted(false)
        , TimeLimit(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ResourceBones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ResourceMeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ResourceHides;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString UnlockedRecipeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString UnlockedAreaTag;

    FQuest_Reward()
        : ResourceBones(0)
        , ResourceMeat(0)
        , ResourceHides(0)
        , UnlockedRecipeID(TEXT(""))
        , UnlockedAreaTag(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Definition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Category Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiverTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 MinPlayerLevel;

    FQuest_Definition()
        : QuestID(TEXT(""))
        , Title(TEXT(""))
        , Description(TEXT(""))
        , Category(EQuest_Category::Survival)
        , Status(EQuest_Status::Inactive)
        , QuestGiverTag(TEXT(""))
        , bIsRepeatable(false)
        , MinPlayerLevel(1)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestActivated, FString, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestObjectiveUpdated, FString, QuestID, FString, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, FString, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed, FString, QuestID);

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuestSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestSystemManager();

    // ---- Quest lifecycle ----
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID, int32 CountDelta = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AbandonQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Definition& QuestDef);

    // ---- Query ----
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    FQuest_Definition GetQuestDefinition(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FString> GetActiveQuestIDs() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    int32 GetTotalQuestsCompleted() const;

    // ---- Built-in quest registration ----
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterAllPrehistoricQuests();

    // ---- Delegates ----
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestActivated OnQuestActivated;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestObjectiveUpdated OnQuestObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestFailed OnQuestFailed;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    TMap<FString, FQuest_Definition> QuestRegistry;

    UPROPERTY()
    TArray<FString> ActiveQuestIDs;

    UPROPERTY()
    int32 TotalQuestsCompleted;

    void CheckQuestCompletion(const FString& QuestID);
    void GrantQuestReward(const FQuest_Reward& Reward);
    FQuest_Definition BuildQuest_SurviveStampede();
    FQuest_Definition BuildQuest_TrackTheMigration();
    FQuest_Definition BuildQuest_FindTheLostHunters();
    FQuest_Definition BuildQuest_CraftFirstWeapon();
    FQuest_Definition BuildQuest_DefendTheCamp();
};
