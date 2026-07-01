// CraftingSystem.cpp
// Quest & Mission Designer — Agent #14
// Transpersonal Game Studio — Cycle AUTO_20260701_006

#include "CraftingSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// Constructor
// ============================================================

UCraftingSystem::UCraftingSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    bCraftingMenuOpen = false;
    bNearCampfire = false;
    MaxInventorySlots = 20;

    // Pre-size inventory array
    Inventory.Reserve(MaxInventorySlots);
}

// ============================================================
// Lifecycle
// ============================================================

void UCraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultRecipes();
}

void UCraftingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// ============================================================
// Inventory
// ============================================================

int32 UCraftingSystem::FindInventorySlot(EQuest_ResourceType ResourceType) const
{
    for (int32 i = 0; i < Inventory.Num(); ++i)
    {
        if (Inventory[i].ResourceType == ResourceType)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

bool UCraftingSystem::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (Amount <= 0 || ResourceType == EQuest_ResourceType::None)
    {
        return false;
    }

    int32 SlotIndex = FindInventorySlot(ResourceType);

    if (SlotIndex != INDEX_NONE)
    {
        FQuest_InventorySlot& Slot = Inventory[SlotIndex];
        int32 NewCount = FMath::Min(Slot.Count + Amount, Slot.MaxStack);
        Slot.Count = NewCount;
        OnInventoryChanged.Broadcast(ResourceType, NewCount);
        return true;
    }

    // New slot
    if (Inventory.Num() < MaxInventorySlots)
    {
        FQuest_InventorySlot NewSlot;
        NewSlot.ResourceType = ResourceType;
        NewSlot.Count = FMath::Min(Amount, NewSlot.MaxStack);
        Inventory.Add(NewSlot);
        OnInventoryChanged.Broadcast(ResourceType, NewSlot.Count);
        return true;
    }

    return false; // Inventory full
}

bool UCraftingSystem::RemoveResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (Amount <= 0 || ResourceType == EQuest_ResourceType::None)
    {
        return false;
    }

    int32 SlotIndex = FindInventorySlot(ResourceType);
    if (SlotIndex == INDEX_NONE)
    {
        return false;
    }

    FQuest_InventorySlot& Slot = Inventory[SlotIndex];
    if (Slot.Count < Amount)
    {
        return false;
    }

    Slot.Count -= Amount;
    OnInventoryChanged.Broadcast(ResourceType, Slot.Count);

    if (Slot.Count == 0)
    {
        Inventory.RemoveAt(SlotIndex);
    }

    return true;
}

int32 UCraftingSystem::GetResourceCount(EQuest_ResourceType ResourceType) const
{
    int32 SlotIndex = FindInventorySlot(ResourceType);
    if (SlotIndex != INDEX_NONE)
    {
        return Inventory[SlotIndex].Count;
    }
    return 0;
}

bool UCraftingSystem::HasResources(const TArray<FQuest_CraftingIngredient>& Ingredients) const
{
    for (const FQuest_CraftingIngredient& Ingredient : Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

// ============================================================
// Recipes
// ============================================================

void UCraftingSystem::InitializeDefaultRecipes()
{
    Recipes.Empty();

    // --- Recipe 1: Stone Axe ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.RecipeID = FName("Recipe_StoneAxe");
        Recipe.DisplayName = FText::FromString("Stone Axe");
        Recipe.OutputItemID = FName("StoneAxe_Item");
        Recipe.CraftTimeSeconds = 5.0f;
        Recipe.bRequiresCampfire = false;

        FQuest_CraftingIngredient Stone;
        Stone.ResourceType = EQuest_ResourceType::Stone;
        Stone.Quantity = 2;

        FQuest_CraftingIngredient Stick;
        Stick.ResourceType = EQuest_ResourceType::Stick;
        Stick.Quantity = 1;

        Recipe.Ingredients.Add(Stone);
        Recipe.Ingredients.Add(Stick);
        Recipes.Add(Recipe);
    }

    // --- Recipe 2: Campfire ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.RecipeID = FName("Recipe_Campfire");
        Recipe.DisplayName = FText::FromString("Campfire");
        Recipe.OutputItemID = FName("Campfire_Placed");
        Recipe.CraftTimeSeconds = 8.0f;
        Recipe.bRequiresCampfire = false;

        FQuest_CraftingIngredient Stick;
        Stick.ResourceType = EQuest_ResourceType::Stick;
        Stick.Quantity = 3;

        Recipe.Ingredients.Add(Stick);
        Recipes.Add(Recipe);
    }

    // --- Recipe 3: Water Container ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.RecipeID = FName("Recipe_WaterContainer");
        Recipe.DisplayName = FText::FromString("Water Container");
        Recipe.OutputItemID = FName("WaterContainer_Item");
        Recipe.CraftTimeSeconds = 6.0f;
        Recipe.bRequiresCampfire = false;

        FQuest_CraftingIngredient Stone;
        Stone.ResourceType = EQuest_ResourceType::Stone;
        Stone.Quantity = 1;

        FQuest_CraftingIngredient Leaf;
        Leaf.ResourceType = EQuest_ResourceType::Leaf;
        Leaf.Quantity = 1;

        Recipe.Ingredients.Add(Stone);
        Recipe.Ingredients.Add(Leaf);
        Recipes.Add(Recipe);
    }

    // --- Recipe 4: Bone Spear ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.RecipeID = FName("Recipe_BoneSpear");
        Recipe.DisplayName = FText::FromString("Bone Spear");
        Recipe.OutputItemID = FName("BoneSpear_Item");
        Recipe.CraftTimeSeconds = 10.0f;
        Recipe.bRequiresCampfire = false;

        FQuest_CraftingIngredient Bone;
        Bone.ResourceType = EQuest_ResourceType::Bone;
        Bone.Quantity = 2;

        FQuest_CraftingIngredient Vine;
        Vine.ResourceType = EQuest_ResourceType::Vine;
        Vine.Quantity = 1;

        Recipe.Ingredients.Add(Bone);
        Recipe.Ingredients.Add(Vine);
        Recipes.Add(Recipe);
    }

    // --- Recipe 5: Hide Wrap (warmth/armor) ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.RecipeID = FName("Recipe_HideWrap");
        Recipe.DisplayName = FText::FromString("Hide Wrap");
        Recipe.OutputItemID = FName("HideWrap_Item");
        Recipe.CraftTimeSeconds = 7.0f;
        Recipe.bRequiresCampfire = false;

        FQuest_CraftingIngredient Hide;
        Hide.ResourceType = EQuest_ResourceType::Hide;
        Hide.Quantity = 2;

        FQuest_CraftingIngredient Vine;
        Vine.ResourceType = EQuest_ResourceType::Vine;
        Vine.Quantity = 1;

        Recipe.Ingredients.Add(Hide);
        Recipe.Ingredients.Add(Vine);
        Recipes.Add(Recipe);
    }
}

TArray<FQuest_CraftingRecipe> UCraftingSystem::GetAvailableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Available;
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (!Recipe.bRequiresCampfire || bNearCampfire)
        {
            Available.Add(Recipe);
        }
    }
    return Available;
}

bool UCraftingSystem::GetRecipeByID(FName RecipeID, FQuest_CraftingRecipe& OutRecipe) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.RecipeID == RecipeID)
        {
            OutRecipe = Recipe;
            return true;
        }
    }
    return false;
}

// ============================================================
// Crafting
// ============================================================

FQuest_CraftResultData UCraftingSystem::TryCraft(FName RecipeID)
{
    FQuest_CraftResultData ResultData;

    FQuest_CraftingRecipe Recipe;
    if (!GetRecipeByID(RecipeID, Recipe))
    {
        ResultData.Result = EQuest_CraftResult::InvalidRecipe;
        ResultData.ResultMessage = FText::FromString("Unknown recipe.");
        OnCraftComplete.Broadcast(ResultData);
        return ResultData;
    }

    if (!HasResources(Recipe.Ingredients))
    {
        ResultData.Result = EQuest_CraftResult::MissingIngredients;
        ResultData.ResultMessage = FText::FromString("Not enough materials.");
        OnCraftComplete.Broadcast(ResultData);
        return ResultData;
    }

    // Consume ingredients
    for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
    {
        RemoveResource(Ingredient.ResourceType, Ingredient.Quantity);
    }

    ResultData.Result = EQuest_CraftResult::Success;
    ResultData.OutputItemID = Recipe.OutputItemID;
    ResultData.ResultMessage = FText::Format(
        FText::FromString("Crafted: {0}"),
        Recipe.DisplayName
    );

    OnCraftComplete.Broadcast(ResultData);
    return ResultData;
}

void UCraftingSystem::OpenCraftingMenu()
{
    if (!bCraftingMenuOpen)
    {
        bCraftingMenuOpen = true;
        OnCraftingMenuToggled.Broadcast(true);
    }
}

void UCraftingSystem::CloseCraftingMenu()
{
    if (bCraftingMenuOpen)
    {
        bCraftingMenuOpen = false;
        OnCraftingMenuToggled.Broadcast(false);
    }
}
