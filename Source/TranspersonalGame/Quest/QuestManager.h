// QuestManager.h
// Agent #14 — Quest & Mission Designer
// PROD_CYCLE_AUTO_20260627_009
// Manages quest state, objectives, and progression for the dinosaur survival game.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestManager.generated.h"

// ============================================================
// ENUMS — Quest system state types
// ============================================================

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    Active          UMETA(DisplayName = "Active"),
    ObjectiveReached UMETA(DisplayName = "Objective Reached"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    CollectResource UMETA(DisplayName = "Collect Resource"),
    CraftItem       UMETA(DisplayName = "Craft Item"),
    SurviveTime     UMETA(DisplayName = "Survive Duration"),
    AvoidPredator   UMETA(DisplayName = "Avoid Predator"),
    FollowHerd      UMETA(DisplayName = "Follow Herd"),
    DefendCamp      UMETA(DisplayName = "Defend Camp"),
    RescueNPC       UMETA(DisplayName = "Rescue NPC")
};

UENUM(BlueprintType)
enum class EQuest_CraftRecipe : uint8
{
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    SpearTip        UMETA(DisplayName = "Spear Tip"),
    LeatherWrap     UMETA(DisplayName = "Leather Wrap")
};

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    Rock            UMETA(DisplayName = "Rock"),
    Stick           UMETA(DisplayName = "Stick"),
    Leaf            UMETA(DisplayName = "Leaf"),
    Bone            UMETA(DisplayName = "Bone"),
    Hide            UMETA(DisplayName = "Hide"),
    Flint           UMETA(DisplayName = "Flint")
};

// ============================================================
// STRUCTS — Quest data types
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
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CompletionRadius;

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
        , CompletionRadius(200.0f)
        , RequiredCount(1)
        , CurrentCount(0)
        , bIsCompleted(false)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_CraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    EQuest_CraftRecipe RecipeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<EQuest_ResourceType> RequiredResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<int32> RequiredAmounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    float CraftingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    bool bUnlocked;

    FQuest_CraftingRecipe()
        : RecipeType(EQuest_CraftRecipe::StoneAxe)
        , ItemName(TEXT(""))
        , CraftingTime(3.0f)
        , bUnlocked(true)
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
    bool bIsMainQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString GiverNPCName;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , Status(EQuest_Status::NotStarted)
        , bIsMainQuest(false)
        , GiverNPCName(TEXT(""))
    {}
};

// ============================================================
// UCLASS — Quest Manager Actor
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Quest Management ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_Status GetQuestStatus(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    // ---- Crafting System ----

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraft(EQuest_CraftRecipe Recipe) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CraftItem(EQuest_CraftRecipe Recipe);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void AddResource(EQuest_ResourceType ResourceType, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    int32 GetResourceCount(EQuest_ResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    // ---- Trigger Callbacks ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerEnteredTriggerZone(const FString& TriggerLabel);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnHerdAlarmTriggered();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnRaptorAmbushActivated();

    // ---- Properties ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Data> AllQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
    TArray<FQuest_CraftingRecipe> CraftingRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCraftingMenuOpen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float QuestCheckRadius;

private:
    void InitializeQuests();
    void InitializeCraftingRecipes();
    void CheckObjectiveCompletion(FQuest_Data& Quest);

    // Resource inventory: indexed by EQuest_ResourceType
    TArray<int32> ResourceInventory;

    FTimerHandle CraftingTimerHandle;
};
