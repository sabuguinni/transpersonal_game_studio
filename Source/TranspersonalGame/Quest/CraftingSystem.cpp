#include "CraftingSystem.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
UCraftingSystem::UCraftingSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    bCraftingMenuOpen = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay — register default recipes
// ─────────────────────────────────────────────────────────────────────────────
void UCraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    RegisterDefaultRecipes();
}

// ─────────────────────────────────────────────────────────────────────────────
// RegisterDefaultRecipes — 3 core survival recipes
// ─────────────────────────────────────────────────────────────────────────────
void UCraftingSystem::RegisterDefaultRecipes()
{
    Recipes.Empty();

    // ── Recipe 1: Stone Axe (2 Stones + 1 Stick) ─────────────────────────────
    {
        FQuest_CraftingRecipe Axe;
        Axe.RecipeID = FName("StoneAxe");
        Axe.DisplayName = FText::FromString("Stone Axe");
        Axe.CraftDurationSeconds = 3.0f;
        Axe.bUnlocksQuestObjective = true;
        Axe.LinkedQuestID = FName("Quest_CraftFirstWeapon");

        FQuest_CraftingIngredient Stone1;
        Stone1.ResourceType = EQuest_ResourceType::Stone;
        Stone1.Quantity = 2;

        FQuest_CraftingIngredient Stick1;
        Stick1.ResourceType = EQuest_ResourceType::Stick;
        Stick1.Quantity = 1;

        Axe.Ingredients.Add(Stone1);
        Axe.Ingredients.Add(Stick1);
        Recipes.Add(Axe);
    }

    // ── Recipe 2: Campfire (3 Sticks) ────────────────────────────────────────
    {
        FQuest_CraftingRecipe Fire;
        Fire.RecipeID = FName("Campfire");
        Fire.DisplayName = FText::FromString("Campfire");
        Fire.CraftDurationSeconds = 5.0f;
        Fire.bUnlocksQuestObjective = true;
        Fire.LinkedQuestID = FName("Quest_SurviveFirstNight");

        FQuest_CraftingIngredient Stick1;
        Stick1.ResourceType = EQuest_ResourceType::Stick;
        Stick1.Quantity = 3;

        Fire.Ingredients.Add(Stick1);
        Recipes.Add(Fire);
    }

    // ── Recipe 3: Water Container (1 Stone + 1 Leaf) ─────────────────────────
    {
        FQuest_CraftingRecipe Container;
        Container.RecipeID = FName("WaterContainer");
        Container.DisplayName = FText::FromString("Water Container");
        Container.CraftDurationSeconds = 2.0f;
        Container.bUnlocksQuestObjective = true;
        Container.LinkedQuestID = FName("Quest_FindWaterSource");

        FQuest_CraftingIngredient Stone1;
        Stone1.ResourceType = EQuest_ResourceType::Stone;
        Stone1.Quantity = 1;

        FQuest_CraftingIngredient Leaf1;
        Leaf1.ResourceType = EQuest_ResourceType::Leaf;
        Leaf1.Quantity = 1;

        Container.Ingredients.Add(Stone1);
        Container.Ingredients.Add(Leaf1);
        Recipes.Add(Container);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Inventory management
// ─────────────────────────────────────────────────────────────────────────────
void UCraftingSystem::AddResource(EQuest_ResourceType Type, int32 Amount)
{
    if (Type == EQuest_ResourceType::None || Amount <= 0) return;

    int32& Current = InventoryMap.FindOrAdd(Type);
    Current += Amount;

    OnInventoryChanged.Broadcast(Type);
}

bool UCraftingSystem::RemoveResource(EQuest_ResourceType Type, int32 Amount)
{
    if (Type == EQuest_ResourceType::None || Amount <= 0) return false;

    int32* Current = InventoryMap.Find(Type);
    if (!Current || *Current < Amount) return false;

    *Current -= Amount;
    if (*Current <= 0) InventoryMap.Remove(Type);

    OnInventoryChanged.Broadcast(Type);
    return true;
}

int32 UCraftingSystem::GetResourceCount(EQuest_ResourceType Type) const
{
    const int32* Count = InventoryMap.Find(Type);
    return Count ? *Count : 0;
}

TArray<FQuest_InventorySlot> UCraftingSystem::GetInventory() const
{
    TArray<FQuest_InventorySlot> Result;
    for (const auto& Pair : InventoryMap)
    {
        if (Pair.Value > 0)
        {
            FQuest_InventorySlot Slot;
            Slot.ResourceType = Pair.Key;
            Slot.Count = Pair.Value;
            Result.Add(Slot);
        }
    }
    return Result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Recipe queries
// ─────────────────────────────────────────────────────────────────────────────
bool UCraftingSystem::CanCraft(FName RecipeID) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.RecipeID == RecipeID)
        {
            for (const FQuest_CraftingIngredient& Ing : Recipe.Ingredients)
            {
                if (GetResourceCount(Ing.ResourceType) < Ing.Quantity)
                {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

bool UCraftingSystem::GetRecipe(FName RecipeID, FQuest_CraftingRecipe& OutRecipe) const
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

TArray<FQuest_CraftingRecipe> UCraftingSystem::GetAllRecipes() const
{
    return Recipes;
}

// ─────────────────────────────────────────────────────────────────────────────
// Crafting execution
// ─────────────────────────────────────────────────────────────────────────────
void UCraftingSystem::ConsumeIngredients(const FQuest_CraftingRecipe& Recipe)
{
    for (const FQuest_CraftingIngredient& Ing : Recipe.Ingredients)
    {
        RemoveResource(Ing.ResourceType, Ing.Quantity);
    }
}

FQuest_CraftResult UCraftingSystem::TryCraft(FName RecipeID)
{
    FQuest_CraftResult Result;
    Result.RecipeID = RecipeID;

    FQuest_CraftingRecipe Recipe;
    if (!GetRecipe(RecipeID, Recipe))
    {
        Result.bSuccess = false;
        Result.ResultMessage = FText::FromString(FString::Printf(TEXT("Unknown recipe: %s"), *RecipeID.ToString()));
        return Result;
    }

    if (!CanCraft(RecipeID))
    {
        Result.bSuccess = false;
        Result.ResultMessage = FText::FromString(
            FString::Printf(TEXT("Not enough materials to craft %s"), *Recipe.DisplayName.ToString()));
        return Result;
    }

    ConsumeIngredients(Recipe);

    Result.bSuccess = true;
    Result.ResultMessage = FText::FromString(
        FString::Printf(TEXT("Crafted: %s"), *Recipe.DisplayName.ToString()));

    OnCraftSuccess.Broadcast(RecipeID);
    return Result;
}

// ─────────────────────────────────────────────────────────────────────────────
// UI toggle
// ─────────────────────────────────────────────────────────────────────────────
void UCraftingSystem::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    OnCraftingMenuToggled.Broadcast(bCraftingMenuOpen);
}
