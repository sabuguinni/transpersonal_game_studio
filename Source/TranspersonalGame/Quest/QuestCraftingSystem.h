#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestCraftingSystem.generated.h"

// ============================================================
// Quest_CraftingSystem — Agent #14 Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260628_010
// Converts crafting milestones into quest objective completions.
// ============================================================

UENUM(BlueprintType)
enum class EQuest_CraftingObjectiveType : uint8
{
    GatherResource      UMETA(DisplayName = "Gather Resource"),
    CraftItem           UMETA(DisplayName = "Craft Item"),
    UseItem             UMETA(DisplayName = "Use Item"),
    DeliverItem         UMETA(DisplayName = "Deliver Item"),
    BuildStructure      UMETA(DisplayName = "Build Structure")
};

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    FlintStone          UMETA(DisplayName = "Flint Stone"),
    DryWood             UMETA(DisplayName = "Dry Wood"),
    Leaf                UMETA(DisplayName = "Leaf"),
    Bone                UMETA(DisplayName = "Bone"),
    Hide                UMETA(DisplayName = "Hide"),
    Meat                UMETA(DisplayName = "Meat"),
    Vine                UMETA(DisplayName = "Vine"),
    Clay                UMETA(DisplayName = "Clay")
};

UENUM(BlueprintType)
enum class EQuest_CraftedItemType : uint8
{
    StoneAxe            UMETA(DisplayName = "Stone Axe"),
    Campfire            UMETA(DisplayName = "Campfire"),
    WaterContainer      UMETA(DisplayName = "Water Container"),
    SpearHead           UMETA(DisplayName = "Spear Head"),
    LeatherWrap         UMETA(DisplayName = "Leather Wrap"),
    BoneTrap            UMETA(DisplayName = "Bone Trap")
};

USTRUCT(BlueprintType)
struct FQuest_CraftingObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    FName ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    EQuest_CraftingObjectiveType ObjectiveType = EQuest_CraftingObjectiveType::GatherResource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    EQuest_ResourceType TargetResource = EQuest_ResourceType::FlintStone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    EQuest_CraftedItemType TargetItem = EQuest_CraftedItemType::StoneAxe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    int32 RequiredCount = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Crafting")
    int32 CurrentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Crafting")
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    FText ObjectiveDescription;
};

USTRUCT(BlueprintType)
struct FQuest_CraftingMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    FName MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    FText MissionTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    FText MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    TArray<FQuest_CraftingObjective> Objectives;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Crafting")
    bool bActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Crafting")
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    EQuest_CraftedItemType RewardItem = EQuest_CraftedItemType::StoneAxe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    FText CompletionNarrativeLine;
};

// ============================================================
// UQuestCraftingSystem — ActorComponent
// Attach to PlayerCharacter. Tracks crafting quest progress.
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuest_CraftingObjectiveUpdated,
    FName, MissionID, FName, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuest_CraftingMissionCompleted,
    FName, MissionID);

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuestCraftingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestCraftingSystem();

    virtual void BeginPlay() override;

    // --- Active Missions ---
    UPROPERTY(BlueprintReadOnly, Category = "Quest|Crafting")
    TArray<FQuest_CraftingMission> ActiveMissions;

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "Quest|Crafting")
    FOnQuest_CraftingObjectiveUpdated OnObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Crafting")
    FOnQuest_CraftingMissionCompleted OnMissionCompleted;

    // --- Public API ---

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    void StartMission(FName MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    void ReportResourceGathered(EQuest_ResourceType ResourceType, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    void ReportItemCrafted(EQuest_CraftedItemType ItemType);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    void ReportItemUsed(EQuest_CraftedItemType ItemType);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    bool IsMissionActive(FName MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    bool IsMissionCompleted(FName MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    float GetMissionProgress(FName MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    FQuest_CraftingMission GetMissionData(FName MissionID) const;

private:
    void InitializeDefaultMissions();
    void CheckMissionCompletion(FQuest_CraftingMission& Mission);
    void UpdateObjectiveProgress(FQuest_CraftingMission& Mission,
        EQuest_CraftingObjectiveType ObjType,
        EQuest_ResourceType Resource,
        EQuest_CraftedItemType Item,
        int32 Count);

    // Pre-defined missions loaded at BeginPlay
    TArray<FQuest_CraftingMission> MissionDatabase;
};
