// CraftingSystem.cpp
// Quest & Mission Designer — Agent #14
// Full implementation of UCraftingSystem ActorComponent
// Recipes: Stone Axe, Campfire, Water Container

#include "CraftingSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UCraftingSystem::UCraftingSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    bCraftingMenuOpen = false;
    MaxInventorySlots = 20;

    // Initialize recipes
    InitializeDefaultRecipes();
}

void UCraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultRecipes();
    unreal_log("CraftingSystem initialized with " + FString::FromInt(AvailableRecipes.Num()) + " recipes");
}

void UCraftingSystem::InitializeDefaultRecipes()
{
    AvailableRecipes.Empty();

    // --- Recipe 1: Stone Axe ---
    FQuest_CraftingRecipe StoneAxe;
    StoneAxe.RecipeName = FName("StoneAxe");
    StoneAxe.DisplayName = FText::FromString("Stone Axe");
    StoneAxe.Description = FText::FromString("A crude but effective cutting tool made from river stones and a sturdy branch.");
    StoneAxe.CraftingTimeSeconds = 5.0f;
    StoneAxe.bRequiresCampfire = false;

    FQuest_CraftingIngredient StoneIngredient;
    StoneIngredient.ResourceType = EQuest_ResourceType::Stone;
    StoneIngredient.Quantity = 2;
    StoneIngredient.DisplayName = FText::FromString("River Stone");

    FQuest_CraftingIngredient StickIngredient;
    StickIngredient.ResourceType = EQuest_ResourceType::Stick;
    StickIngredient.Quantity = 1;
    StickIngredient.DisplayName = FText::FromString("Sturdy Stick");

    StoneAxe.Ingredients.Add(StoneIngredient);
    StoneAxe.Ingredients.Add(StickIngredient);
    StoneAxe.OutputItemName = FName("StoneAxe_Item");
    StoneAxe.OutputQuantity = 1;
    AvailableRecipes.Add(StoneAxe);

    // --- Recipe 2: Campfire ---
    FQuest_CraftingRecipe Campfire;
    Campfire.RecipeName = FName("Campfire");
    Campfire.DisplayName = FText::FromString("Campfire");
    Campfire.Description = FText::FromString("A small fire for warmth, cooking, and keeping predators at bay through the night.");
    Campfire.CraftingTimeSeconds = 8.0f;
    Campfire.bRequiresCampfire = false;

    FQuest_CraftingIngredient StickIngredient2;
    StickIngredient2.ResourceType = EQuest_ResourceType::Stick;
    StickIngredient2.Quantity = 3;
    StickIngredient2.DisplayName = FText::FromString("Dry Stick");

    Campfire.Ingredients.Add(StickIngredient2);
    Campfire.OutputItemName = FName("Campfire_Placed");
    Campfire.OutputQuantity = 1;
    AvailableRecipes.Add(Campfire);

    // --- Recipe 3: Water Container ---
    FQuest_CraftingRecipe WaterContainer;
    WaterContainer.RecipeName = FName("WaterContainer");
    WaterContainer.DisplayName = FText::FromString("Water Container");
    WaterContainer.Description = FText::FromString("A hollowed stone bowl lined with large leaves — holds enough water to survive a day's march.");
    WaterContainer.CraftingTimeSeconds = 6.0f;
    WaterContainer.bRequiresCampfire = false;

    FQuest_CraftingIngredient StoneIngredient2;
    StoneIngredient2.ResourceType = EQuest_ResourceType::Stone;
    StoneIngredient2.Quantity = 1;
    StoneIngredient2.DisplayName = FText::FromString("Flat Stone");

    FQuest_CraftingIngredient LeafIngredient;
    LeafIngredient.ResourceType = EQuest_ResourceType::Leaf;
    LeafIngredient.Quantity = 1;
    LeafIngredient.DisplayName = FText::FromString("Large Leaf");

    WaterContainer.Ingredients.Add(StoneIngredient2);
    WaterContainer.Ingredients.Add(LeafIngredient);
    WaterContainer.OutputItemName = FName("WaterContainer_Item");
    WaterContainer.OutputQuantity = 1;
    AvailableRecipes.Add(WaterContainer);
}

bool UCraftingSystem::AddResourceToInventory(EQuest_ResourceType ResourceType, int32 Quantity)
{
    if (Quantity <= 0) return false;

    // Find existing slot
    for (FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == ResourceType)
        {
            Slot.Quantity += Quantity;
            OnInventoryChanged.Broadcast(ResourceType, Slot.Quantity);
            return true;
        }
    }

    // New slot
    if (Inventory.Num() >= MaxInventorySlots) return false;

    FQuest_InventorySlot NewSlot;
    NewSlot.ResourceType = ResourceType;
    NewSlot.Quantity = Quantity;
    Inventory.Add(NewSlot);
    OnInventoryChanged.Broadcast(ResourceType, Quantity);
    return true;
}

bool UCraftingSystem::RemoveResourceFromInventory(EQuest_ResourceType ResourceType, int32 Quantity)
{
    for (FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == ResourceType)
        {
            if (Slot.Quantity < Quantity) return false;
            Slot.Quantity -= Quantity;
            OnInventoryChanged.Broadcast(ResourceType, Slot.Quantity);
            return true;
        }
    }
    return false;
}

int32 UCraftingSystem::GetResourceCount(EQuest_ResourceType ResourceType) const
{
    for (const FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == ResourceType)
            return Slot.Quantity;
    }
    return 0;
}

bool UCraftingSystem::CanCraftRecipe(FName RecipeName) const
{
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.RecipeName == RecipeName)
        {
            for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
            {
                if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
                    return false;
            }
            return true;
        }
    }
    return false;
}

FQuest_CraftResult UCraftingSystem::CraftItem(FName RecipeName)
{
    FQuest_CraftResult Result;
    Result.bSuccess = false;
    Result.OutputItemName = NAME_None;
    Result.OutputQuantity = 0;

    if (!CanCraftRecipe(RecipeName))
    {
        Result.FailureReason = FText::FromString("Missing ingredients.");
        OnCraftingFailed.Broadcast(RecipeName, Result.FailureReason);
        return Result;
    }

    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.RecipeName == RecipeName)
        {
            // Consume ingredients
            for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
            {
                RemoveResourceFromInventory(Ingredient.ResourceType, Ingredient.Quantity);
            }

            Result.bSuccess = true;
            Result.OutputItemName = Recipe.OutputItemName;
            Result.OutputQuantity = Recipe.OutputQuantity;
            Result.FailureReason = FText::GetEmpty();

            OnItemCrafted.Broadcast(Recipe.OutputItemName, Recipe.OutputQuantity);
            return Result;
        }
    }

    Result.FailureReason = FText::FromString("Recipe not found.");
    OnCraftingFailed.Broadcast(RecipeName, Result.FailureReason);
    return Result;
}

void UCraftingSystem::OpenCraftingMenu()
{
    if (bCraftingMenuOpen) return;
    bCraftingMenuOpen = true;
    OnCraftingMenuToggled.Broadcast(true);
}

void UCraftingSystem::CloseCraftingMenu()
{
    if (!bCraftingMenuOpen) return;
    bCraftingMenuOpen = false;
    OnCraftingMenuToggled.Broadcast(false);
}

void UCraftingSystem::ToggleCraftingMenu()
{
    if (bCraftingMenuOpen)
        CloseCraftingMenu();
    else
        OpenCraftingMenu();
}

TArray<FQuest_CraftingRecipe> UCraftingSystem::GetAvailableRecipes() const
{
    return AvailableRecipes;
}

TArray<FQuest_InventorySlot> UCraftingSystem::GetInventory() const
{
    return Inventory;
}
