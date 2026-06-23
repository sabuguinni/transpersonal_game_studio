#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "QuestManager.generated.h"

// ============================================================
// QuestManager.h — Agent #14 Quest & Mission Designer
// Transpersonal Game Studio — Dinosaur Survival Game
// ============================================================

// ---- Enums (global scope, Quest_ prefix) ----

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
    Reach       UMETA(DisplayName = "Reach Location"),
    Collect     UMETA(DisplayName = "Collect Resource"),
    Hunt        UMETA(DisplayName = "Hunt Target"),
    Survive     UMETA(DisplayName = "Survive Duration"),
    Defend      UMETA(DisplayName = "Defend Location"),
    Craft       UMETA(DisplayName = "Craft Item"),
    Follow      UMETA(DisplayName = "Follow Entity")
};

UENUM(BlueprintType)
enum class EQuest_ID : uint8
{
    None                UMETA(DisplayName = "None"),
    FollowTheHerd       UMETA(DisplayName = "Follow the Herd"),
    RaptorTerritory     UMETA(DisplayName = "Raptor Territory"),
    TribeCampDefense    UMETA(DisplayName = "Tribe Camp Defense"),
    FindWaterSource     UMETA(DisplayName = "Find Water Source"),
    CaveExploration     UMETA(DisplayName = "Cave Exploration"),
    CraftStoneAxe       UMETA(DisplayName = "Craft Stone Axe"),
    CraftCampfire       UMETA(DisplayName = "Craft Campfire")
};

// ---- Structs (global scope, Quest_ prefix) ----

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType = EQuest_ObjectiveType::Reach;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float AcceptanceRadius = 300.0f;
};

USTRUCT(BlueprintType)
struct FQuest_Definition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ID QuestID = EQuest_ID::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status = EQuest_Status::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit = 0.0f;  // 0 = no time limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ElapsedTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest = false;
};

USTRUCT(BlueprintType)
struct FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString RecipeName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FString> RequiredResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<int32> RequiredCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString OutputItem = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bUnlocked = false;
};

// ============================================================
// UQuestManager — World Subsystem
// ============================================================

UCLASS()
class TRANSPERSONALGAME_API UQuestManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UQuestManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest lifecycle
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(EQuest_ID QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(EQuest_ID QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(EQuest_ID QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_Status GetQuestStatus(EQuest_ID QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(EQuest_ID QuestID) const;

    // Objective progress
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AdvanceObjective(EQuest_ID QuestID, int32 ObjectiveIndex, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CheckObjectiveComplete(EQuest_ID QuestID, int32 ObjectiveIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // Resource / crafting
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool AddResource(const FString& ResourceName, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetResourceCount(const FString& ResourceName) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraft(const FString& RecipeName) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CraftItem(const FString& RecipeName);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    // Tick
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void TickQuests(float DeltaTime);

    // Data
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_Definition> AllQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> CraftingRecipes;

private:
    void RegisterDefaultQuests();
    void RegisterCraftingRecipes();
    FQuest_Definition* FindQuest(EQuest_ID QuestID);
    const FQuest_Definition* FindQuestConst(EQuest_ID QuestID) const;

    UPROPERTY()
    TMap<FString, int32> ResourceInventory;
};
