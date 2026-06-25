#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

// ============================================================
// Quest enums and structs — Quest_ prefix for uniqueness
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
    Hunt        UMETA(DisplayName = "Hunt"),
    Gather      UMETA(DisplayName = "Gather"),
    Explore     UMETA(DisplayName = "Explore"),
    Defend      UMETA(DisplayName = "Defend"),
    Rescue      UMETA(DisplayName = "Rescue"),
    Craft       UMETA(DisplayName = "Craft"),
    Survive     UMETA(DisplayName = "Survive")
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
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::Hunt)
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
    int32 ResourceRocks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ResourceSticks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ResourceFood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float HealthBonus;

    FQuest_Reward()
        : ResourceRocks(0)
        , ResourceSticks(0)
        , ResourceFood(0)
        , HealthBonus(0.0f)
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
    EQuest_Status Status;

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
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , Status(EQuest_Status::Inactive)
        , GiverNPCLabel(TEXT(""))
        , bIsMainQuest(false)
    {}
};

// ============================================================
// AQuestManager — placed in level, manages all active quests
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

    // All quests defined in the game
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Database")
    TArray<FQuest_Data> QuestDatabase;

    // Currently active quests (subset of database)
    UPROPERTY(BlueprintReadOnly, Category = "Quest|Active",
        meta = (AllowPrivateAccess = "true"))
    TArray<FQuest_Data> ActiveQuests;

    // Start a quest by ID
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    // Advance objective progress
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AdvanceObjective(const FString& QuestID, const FString& ObjectiveID, int32 Amount = 1);

    // Complete a quest and grant rewards
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    // Fail a quest
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(const FString& QuestID);

    // Get quest data by ID
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool GetQuestData(const FString& QuestID, FQuest_Data& OutData);

    // Check if quest is active
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    // Initialize default quests
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Setup")
    void InitializeDefaultQuests();

protected:
    virtual void BeginPlay() override;

private:
    // Find quest index in database
    int32 FindQuestIndex(const FString& QuestID) const;
    int32 FindActiveQuestIndex(const FString& QuestID) const;
};
