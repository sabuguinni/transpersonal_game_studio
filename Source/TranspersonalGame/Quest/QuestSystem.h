// QuestSystem.h
// Agent #14 — Quest & Mission Designer
// Transpersonal Game Studio — Prehistoric Survival Game
// Crafting + Quest system: recipes, resource pickups, quest objectives

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "QuestSystem.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// ENUMS — Quest & Crafting (prefixed Quest_ to avoid collisions)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt        UMETA(DisplayName = "Hunt"),
    Gather      UMETA(DisplayName = "Gather"),
    Craft       UMETA(DisplayName = "Craft"),
    Defend      UMETA(DisplayName = "Defend"),
    Explore     UMETA(DisplayName = "Explore"),
    Rescue      UMETA(DisplayName = "Rescue"),
    Follow      UMETA(DisplayName = "Follow"),
};

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    Rock        UMETA(DisplayName = "Rock"),
    Stick       UMETA(DisplayName = "Stick"),
    Leaf        UMETA(DisplayName = "Leaf"),
    Flint       UMETA(DisplayName = "Flint"),
    Bone        UMETA(DisplayName = "Bone"),
    Hide        UMETA(DisplayName = "Hide"),
    Meat        UMETA(DisplayName = "Meat"),
    Water       UMETA(DisplayName = "Water"),
};

UENUM(BlueprintType)
enum class EQuest_CraftedItem : uint8
{
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    Spear           UMETA(DisplayName = "Spear"),
    Shelter         UMETA(DisplayName = "Shelter"),
    BoneTrap        UMETA(DisplayName = "Bone Trap"),
};

// ─────────────────────────────────────────────────────────────────────────────
// STRUCTS
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_RecipeIngredient
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::Rock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    int32 Quantity = 1;
};

USTRUCT(BlueprintType)
struct FQuest_CraftingRecipe : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    EQuest_CraftedItem ResultItem = EQuest_CraftedItem::StoneAxe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    TArray<FQuest_RecipeIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    float CraftingTime = 2.0f;
};

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    EQuest_ObjectiveType ObjectiveType = EQuest_ObjectiveType::Gather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FName TargetTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    int32 RequiredCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    int32 CurrentCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Objective")
    bool bCompleted = false;
};

USTRUCT(BlueprintType)
struct FQuest_Data : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State State = EQuest_State::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit = 0.0f; // 0 = no time limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// RESOURCE PICKUP ACTOR
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Resource",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Resource",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* PickupRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Resource")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::Rock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Resource")
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Resource")
    bool bAutoPickup = true;

    UFUNCTION(BlueprintCallable, Category = "Quest|Resource")
    void OnPickedUp(AActor* Collector);

    UFUNCTION(BlueprintCallable, Category = "Quest|Resource")
    EQuest_ResourceType GetResourceType() const { return ResourceType; }

    UFUNCTION(BlueprintCallable, Category = "Quest|Resource")
    int32 GetQuantity() const { return Quantity; }

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};

// ─────────────────────────────────────────────────────────────────────────────
// QUEST TRIGGER ACTOR
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_TriggerZone : public AActor
{
    GENERATED_BODY()

public:
    AQuest_TriggerZone();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Trigger",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Trigger")
    FName QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Trigger")
    bool bActivatesQuest = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Trigger")
    bool bCompletesObjective = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Trigger")
    int32 ObjectiveIndex = 0;

    UFUNCTION(BlueprintCallable, Category = "Quest|Trigger")
    void ActivateQuest(AActor* Player);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};

// ─────────────────────────────────────────────────────────────────────────────
// CRAFTING SYSTEM COMPONENT
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = "Quest")
class TRANSPERSONALGAME_API UQuest_CraftingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CraftingComponent();

    // Inventory: resource type → count
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Crafting")
    TMap<EQuest_ResourceType, int32> Inventory;

    // Crafted items
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Crafting")
    TMap<EQuest_CraftedItem, int32> CraftedItems;

    // Built-in recipes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    TArray<FQuest_CraftingRecipe> Recipes;

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    void AddResource(EQuest_ResourceType Type, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    bool CanCraft(EQuest_CraftedItem Item) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    bool CraftItem(EQuest_CraftedItem Item);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    int32 GetResourceCount(EQuest_ResourceType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    TArray<FQuest_CraftingRecipe> GetAvailableRecipes() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Crafting")
    void InitializeDefaultRecipes();

protected:
    virtual void BeginPlay() override;

private:
    const FQuest_CraftingRecipe* FindRecipe(EQuest_CraftedItem Item) const;
};

// ─────────────────────────────────────────────────────────────────────────────
// QUEST MANAGER COMPONENT
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, ClassGroup = "Quest")
class TRANSPERSONALGAME_API UQuest_ManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_ManagerComponent();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Manager")
    TArray<FQuest_Data> ActiveQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Manager")
    TArray<FQuest_Data> CompletedQuests;

    UFUNCTION(BlueprintCallable, Category = "Quest|Manager")
    bool StartQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest|Manager")
    bool UpdateObjective(FName QuestID, int32 ObjectiveIndex, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest|Manager")
    bool CompleteQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest|Manager")
    bool FailQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest|Manager")
    FQuest_Data GetQuestData(FName QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Manager")
    bool IsQuestActive(FName QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Manager")
    bool IsQuestCompleted(FName QuestID) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Manager")
    void InitializeStartingQuests();

protected:
    virtual void BeginPlay() override;

private:
    FQuest_Data* FindActiveQuest(FName QuestID);
    void CheckQuestCompletion(FQuest_Data& Quest);
};
