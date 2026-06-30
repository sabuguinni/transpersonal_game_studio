#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "QuestSystem.generated.h"

// ============================================================
// ENUMS — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Locked       UMETA(DisplayName = "Locked"),
    Available    UMETA(DisplayName = "Available"),
    Active       UMETA(DisplayName = "Active"),
    Completed    UMETA(DisplayName = "Completed"),
    Failed       UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt         UMETA(DisplayName = "Hunt Target"),
    Gather       UMETA(DisplayName = "Gather Resource"),
    Reach        UMETA(DisplayName = "Reach Location"),
    Survive      UMETA(DisplayName = "Survive Duration"),
    Craft        UMETA(DisplayName = "Craft Item"),
    Escort       UMETA(DisplayName = "Escort NPC"),
    Defend       UMETA(DisplayName = "Defend Location"),
    Observe      UMETA(DisplayName = "Observe Creature")
};

UENUM(BlueprintType)
enum class EQuest_RewardType : uint8
{
    Resource     UMETA(DisplayName = "Resource"),
    Tool         UMETA(DisplayName = "Crafted Tool"),
    Knowledge    UMETA(DisplayName = "Survival Knowledge"),
    TribeRank    UMETA(DisplayName = "Tribe Rank"),
    MapReveal    UMETA(DisplayName = "Map Reveal")
};

UENUM(BlueprintType)
enum class EQuest_Category : uint8
{
    Survival     UMETA(DisplayName = "Survival"),
    Hunting      UMETA(DisplayName = "Hunting"),
    Exploration  UMETA(DisplayName = "Exploration"),
    Crafting     UMETA(DisplayName = "Crafting"),
    Tribal       UMETA(DisplayName = "Tribal"),
    Emergency    UMETA(DisplayName = "Emergency")
};

// ============================================================
// STRUCTS
// ============================================================

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
    FString TargetTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;       // 0 = no limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float LocationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsOptional;

    FQuest_Objective()
        : ObjectiveType(EQuest_ObjectiveType::Reach)
        , RequiredCount(1)
        , CurrentCount(0)
        , TimeLimit(0.0f)
        , TargetLocation(FVector::ZeroVector)
        , LocationRadius(500.0f)
        , bIsCompleted(false)
        , bIsOptional(false)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_RewardType RewardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    FQuest_Reward()
        : RewardType(EQuest_RewardType::Resource)
        , Quantity(1)
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
    EQuest_Category Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Reward> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> PrerequisiteQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiverActorTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsRepeatable;

    FQuest_Data()
        : Category(EQuest_Category::Survival)
        , Status(EQuest_Status::Locked)
        , bIsMainQuest(false)
        , bIsRepeatable(false)
    {}
};

// ============================================================
// QUEST MANAGER ACTOR
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestSystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Quest Registration ---
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Data& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    // --- Objective Progress ---
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 ProgressDelta);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportTagProgress(const FString& TargetTag, int32 ProgressDelta);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportLocationReached(const FVector& PlayerLocation);

    // --- Query ---
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    FQuest_Data GetQuestByID(const FString& QuestID) const;

    // --- Initialization ---
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void InitializePrehistoricQuests();

    // --- Events (bindable in Blueprint) ---
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FTimerDelegate OnQuestActivated;

    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FTimerDelegate OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FTimerDelegate OnObjectiveUpdated;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FQuest_Data> QuestRegistry;

    void CheckQuestUnlocks();
    void CheckObjectiveCompletion(FQuest_Data& Quest);
    void GrantRewards(const FQuest_Data& Quest);
    bool ArePrerequisitesMet(const FQuest_Data& Quest) const;
};
