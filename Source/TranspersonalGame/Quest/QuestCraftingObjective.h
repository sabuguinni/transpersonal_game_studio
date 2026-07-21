#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestCraftingObjective.generated.h"

// ============================================================
// Quest Crafting Objective System — Agent #14
// Tracks player crafting progress for quest objectives.
// Recipes: Stone Axe, Campfire, Water Container
// ============================================================

UENUM(BlueprintType)
enum class EQuest_CraftingRecipe : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    Campfire        UMETA(DisplayName = "Campfire"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    Spear           UMETA(DisplayName = "Spear"),
    LeatherWrap     UMETA(DisplayName = "Leather Wrap"),
};

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    None    UMETA(DisplayName = "None"),
    Rock    UMETA(DisplayName = "Rock"),
    Stick   UMETA(DisplayName = "Stick"),
    Leaf    UMETA(DisplayName = "Leaf"),
    Bone    UMETA(DisplayName = "Bone"),
    Hide    UMETA(DisplayName = "Hide"),
};

USTRUCT(BlueprintType)
struct FQuest_CraftingIngredient
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    EQuest_ResourceType ResourceType = EQuest_ResourceType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    int32 RequiredCount = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Crafting")
    int32 CurrentCount = 0;

    bool IsSatisfied() const { return CurrentCount >= RequiredCount; }
};

USTRUCT(BlueprintType)
struct FQuest_CraftingRecipeDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    EQuest_CraftingRecipe Recipe = EQuest_CraftingRecipe::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    TArray<FQuest_CraftingIngredient> Ingredients;

    bool CanCraft() const
    {
        for (const FQuest_CraftingIngredient& Ing : Ingredients)
        {
            if (!Ing.IsSatisfied()) return false;
        }
        return true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecipeCrafted, EQuest_CraftingRecipe, Recipe);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceCollected, EQuest_ResourceType, Resource, int32, NewCount);

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuestCraftingObjective : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestCraftingObjective();

    // ---- Events ----
    UPROPERTY(BlueprintAssignable, Category = "Quest|Crafting")
    FOnRecipeCrafted OnRecipeCrafted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Crafting")
    FOnResourceCollected OnResourceCollected;

    // ---- Inventory ----
    UPROPERTY(BlueprintReadOnly, Category = "Quest|Crafting")
    TMap<EQuest_ResourceType, int32> ResourceInventory;

    // ---- Recipes ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Crafting")
    TArray<FQuest_CraftingRecipeDef> AvailableRecipes;

    // ---- Crafted items ----
    UPROPERTY(BlueprintReadOnly, Category = "Quest|Crafting")
    TArray<EQuest_CraftingRecipe> CraftedItems;

    // ---- Methods ----
    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    void InitializeDefaultRecipes();

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    bool CollectResource(EQuest_ResourceType Resource, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    bool TryCraftRecipe(EQuest_CraftingRecipe Recipe);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    bool HasCrafted(EQuest_CraftingRecipe Recipe) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    int32 GetResourceCount(EQuest_ResourceType Resource) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    bool CanCraftRecipe(EQuest_CraftingRecipe Recipe) const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Crafting")
    TArray<FQuest_CraftingRecipeDef> GetAvailableRecipes() const;

protected:
    virtual void BeginPlay() override;

private:
    FQuest_CraftingRecipeDef* FindRecipe(EQuest_CraftingRecipe Recipe);
    void ConsumeIngredients(FQuest_CraftingRecipeDef& RecipeDef);
};
