#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

// ============================================================
// Enums — global scope (UHT requirement)
// ============================================================

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt        UMETA(DisplayName = "Hunt Dinosaur"),
    Gather      UMETA(DisplayName = "Gather Resource"),
    Reach       UMETA(DisplayName = "Reach Location"),
    Survive     UMETA(DisplayName = "Survive Duration"),
    Craft       UMETA(DisplayName = "Craft Item"),
    Protect     UMETA(DisplayName = "Protect NPC"),
    Track       UMETA(DisplayName = "Track Animal")
};

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    MainStory   UMETA(DisplayName = "Main Story"),
    Survival    UMETA(DisplayName = "Survival"),
    Exploration UMETA(DisplayName = "Exploration"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Defense     UMETA(DisplayName = "Defense"),
    Migration   UMETA(DisplayName = "Migration")
};

// ============================================================
// Structs — global scope
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
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float LocationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::Hunt)
        , RequiredCount(1)
        , CurrentCount(0)
        , TargetLocation(FVector::ZeroVector)
        , LocationRadius(500.f)
        , bIsCompleted(false)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 BoneTokens;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> UnlockedRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> UnlockedAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString NarrativeUnlock;

    FQuest_Reward()
        : BoneTokens(0)
        , NarrativeUnlock(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString GiverNPCID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , QuestType(EQuest_Type::Survival)
        , Status(EQuest_Status::Inactive)
        , GiverNPCID(TEXT(""))
        , bIsRepeatable(false)
        , TimeLimit(0.f)
    {}
};

// ============================================================
// UQuestManager — World Subsystem
// ============================================================

UCLASS()
class TRANSPERSONALGAME_API UQuestManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest lifecycle
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AbandonQuest(const FString& QuestID);

    // Objective tracking
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyDinosaurKilled(const FString& DinosaurSpecies);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyResourceGathered(const FString& ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyLocationReached(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyItemCrafted(const FString& ItemID);

    // Crowd integration — stampede/alert quests
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyHerdAlerted(const FString& GroupID, float AlertLevel);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyPlayerEscapedAlert(float DistanceFromHerd);

    // Query
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // Registration
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Data& QuestData);

private:
    void RegisterDefaultQuests();
    void CheckObjectiveCompletion(FQuest_Data& Quest);
    void GrantReward(const FQuest_Reward& Reward);
    bool AllObjectivesComplete(const FQuest_Data& Quest) const;

    UPROPERTY()
    TMap<FString, FQuest_Data> QuestRegistry;

    UPROPERTY()
    TArray<FString> ActiveQuestIDs;

    UPROPERTY()
    TArray<FString> CompletedQuestIDs;

    // Stampede quest state
    float StampedeQuestTimer;
    bool bStampedeQuestActive;
};
