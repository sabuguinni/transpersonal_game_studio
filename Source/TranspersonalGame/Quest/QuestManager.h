// QuestManager.h
// Agent #14 — Quest & Mission Designer
// CYCLE: PROD_CYCLE_AUTO_20260627_008
// Manages all active quests, objectives, and quest state for the dinosaur survival game

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestManager.generated.h"

// ─── ENUMS (global scope — RULE 1) ───

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Hunt        UMETA(DisplayName = "Hunt"),
    Explore     UMETA(DisplayName = "Explore"),
    Rescue      UMETA(DisplayName = "Rescue"),
    Craft       UMETA(DisplayName = "Craft"),
    Survive     UMETA(DisplayName = "Survive"),
    Track       UMETA(DisplayName = "Track")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    KillTarget      UMETA(DisplayName = "Kill Target"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    ProtectNPC      UMETA(DisplayName = "Protect NPC"),
    CraftItem       UMETA(DisplayName = "Craft Item"),
    SurviveTime     UMETA(DisplayName = "Survive Time")
};

// ─── STRUCTS (global scope — RULE 1) ───

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
    float AcceptanceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , TargetLocation(FVector::ZeroVector)
        , AcceptanceRadius(200.0f)
        , RequiredCount(1)
        , CurrentCount(0)
        , bIsCompleted(false)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 FoodReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 MaterialsReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString UnlockRecipeID;

    FQuest_Reward()
        : FoodReward(0)
        , MaterialsReward(0)
        , UnlockRecipeID(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State QuestState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString GiverNPCLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestName(TEXT(""))
        , Description(TEXT(""))
        , QuestType(EQuest_Type::Explore)
        , QuestState(EQuest_State::Inactive)
        , GiverNPCLabel(TEXT(""))
        , bIsMainQuest(false)
    {}
};

// ─── QUEST MANAGER COMPONENT ───

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ─── Quest Registration ───
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Data& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    // ─── Objective Tracking ───
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 ProgressDelta);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerReachedLocation(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerCollectedItem(const FString& ItemID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerKilledTarget(const FString& TargetSpecies);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerCraftedItem(const FString& RecipeID);

    // ─── Query ───
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    int32 GetActiveQuestCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    float GetQuestProgress(const FString& QuestID) const;

    // ─── Crowd Event Integration ───
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnCrowdPanicTriggered(const FString& ZoneLabel);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnHerdMigrationReachedWaypoint(int32 WaypointIndex);

protected:
    void InitializeDefaultQuests();
    void CheckLocationObjectives(FVector PlayerLocation);
    bool AreAllObjectivesComplete(const FQuest_Data& Quest) const;
    void GrantReward(const FQuest_Reward& Reward);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest", meta = (AllowPrivateAccess = "true"))
    TArray<FQuest_Data> AllQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest", meta = (AllowPrivateAccess = "true"))
    int32 TotalQuestsCompleted;

    float LocationCheckInterval;
    float TimeSinceLastLocationCheck;
};
