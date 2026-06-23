// QuestManager.h
// Agent #14 — Quest & Mission Designer
// PROD_CYCLE_AUTO_20260622_013
// Quest system manager: tracks active quests, objectives, and resource collection state.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "QuestManager.generated.h"

// ─── Enums (global scope — UHT requirement) ───────────────────────────────────

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
    Gather      UMETA(DisplayName = "Gather"),
    Explore     UMETA(DisplayName = "Explore"),
    Defend      UMETA(DisplayName = "Defend"),
    Rescue      UMETA(DisplayName = "Rescue"),
    Craft       UMETA(DisplayName = "Craft"),
    Follow      UMETA(DisplayName = "Follow")
};

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    FlintRock   UMETA(DisplayName = "Flint Rock"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Meat        UMETA(DisplayName = "Meat"),
    Hide        UMETA(DisplayName = "Hide"),
    Bone        UMETA(DisplayName = "Bone"),
    Water       UMETA(DisplayName = "Water")
};

// ─── Structs (global scope) ───────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount = 1;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    bool bCompleted = false;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::FlintRock;
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType = EQuest_Type::Hunt;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    EQuest_State QuestState = EQuest_State::Inactive;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString GiverNPCLabel;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString RewardDescription;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    float TimeLimit = 0.0f; // 0 = no limit

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest = false;
};

USTRUCT(BlueprintType)
struct FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crafting")
    FString RecipeID;

    UPROPERTY(BlueprintReadWrite, Category = "Crafting")
    FString OutputItemName;

    UPROPERTY(BlueprintReadWrite, Category = "Crafting")
    TArray<EQuest_ResourceType> RequiredResources;

    UPROPERTY(BlueprintReadWrite, Category = "Crafting")
    TArray<int32> RequiredCounts;

    UPROPERTY(BlueprintReadWrite, Category = "Crafting")
    FString Description;
};

// ─── UQuestManager ────────────────────────────────────────────────────────────

UCLASS()
class TRANSPERSONALGAME_API UQuestManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // WorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Data& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestComplete(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_State GetQuestState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    // Resource collection (links to crafting)
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnResourceCollected(EQuest_ResourceType ResourceType, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetResourceCount(EQuest_ResourceType ResourceType) const;

    // Crafting recipes
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraft(const FString& RecipeID) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool ExecuteCraft(const FString& RecipeID);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    // Crowd integration — called when crowd system triggers flee event
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnHerdFleeTriggered(FVector FleeOrigin, float Radius);

    // Delegate for UI/Blueprint notifications
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStateChanged, const FString&, QuestID);
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnQuestStateChanged OnQuestStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, const FString&, QuestID, const FString&, ObjectiveID);
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnObjectiveUpdated OnObjectiveUpdated;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemCrafted, const FString&, ItemName);
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnItemCrafted OnItemCrafted;

private:
    UPROPERTY()
    TMap<FString, FQuest_Data> QuestRegistry;

    UPROPERTY()
    TMap<uint8, int32> ResourceInventory;

    UPROPERTY()
    TArray<FQuest_CraftingRecipe> CraftingRecipes;

    void InitializeDefaultQuests();
    void InitializeCraftingRecipes();
    void CheckQuestCompletion(const FString& QuestID);
    void ConsumeResources(const FQuest_CraftingRecipe& Recipe);
};
