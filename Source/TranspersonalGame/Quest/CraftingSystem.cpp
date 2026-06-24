// CraftingSystem.cpp
// Agent #14 — Quest & Mission Designer
// Crafting system: recipes, inventory, resource pickups
// Cycle: PROD_CYCLE_AUTO_20260624_005

#include "CraftingSystem.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"

// ============================================================
// UCraftingSystem
// ============================================================

UCraftingSystem::UCraftingSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultRecipes();
}

void UCraftingSystem::InitializeDefaultRecipes()
{
    Recipes.Empty();

    // Recipe 1: Stone Axe — 2 Rocks + 1 Stick
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItem::StoneAxe;
        Recipe.RecipeName = TEXT("Stone Axe");
        Recipe.Description = TEXT("A crude but effective cutting tool. Useful for chopping wood and fighting off small predators.");
        Recipe.CraftingTime = 3.0f;

        FQuest_RecipeIngredient RockIngredient;
        RockIngredient.ResourceType = EQuest_ResourceType::Rock;
        RockIngredient.Quantity = 2;
        Recipe.Ingredients.Add(RockIngredient);

        FQuest_RecipeIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity = 1;
        Recipe.Ingredients.Add(StickIngredient);

        Recipes.Add(Recipe);
    }

    // Recipe 2: Campfire — 3 Sticks
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItem::Campfire;
        Recipe.RecipeName = TEXT("Campfire");
        Recipe.Description = TEXT("A basic fire for warmth, cooking, and keeping predators at bay during the night.");
        Recipe.CraftingTime = 5.0f;

        FQuest_RecipeIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity = 3;
        Recipe.Ingredients.Add(StickIngredient);

        Recipes.Add(Recipe);
    }

    // Recipe 3: Water Container — 1 Rock + 1 Leaf
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItem::WaterContainer;
        Recipe.RecipeName = TEXT("Water Container");
        Recipe.Description = TEXT("A hollowed rock lined with large leaves. Holds enough water for a day of travel.");
        Recipe.CraftingTime = 4.0f;

        FQuest_RecipeIngredient RockIngredient;
        RockIngredient.ResourceType = EQuest_ResourceType::Rock;
        RockIngredient.Quantity = 1;
        Recipe.Ingredients.Add(RockIngredient);

        FQuest_RecipeIngredient LeafIngredient;
        LeafIngredient.ResourceType = EQuest_ResourceType::Leaf;
        LeafIngredient.Quantity = 1;
        Recipe.Ingredients.Add(LeafIngredient);

        Recipes.Add(Recipe);
    }

    // Recipe 4: Spear — 1 Stick + 1 Flint
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItem::Spear;
        Recipe.RecipeName = TEXT("Spear");
        Recipe.Description = TEXT("A sharpened flint tip bound to a long stick. Effective against medium-sized predators.");
        Recipe.CraftingTime = 6.0f;

        FQuest_RecipeIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity = 1;
        Recipe.Ingredients.Add(StickIngredient);

        FQuest_RecipeIngredient FlintIngredient;
        FlintIngredient.ResourceType = EQuest_ResourceType::Flint;
        FlintIngredient.Quantity = 1;
        Recipe.Ingredients.Add(FlintIngredient);

        Recipes.Add(Recipe);
    }
}

void UCraftingSystem::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (ResourceType == EQuest_ResourceType::None || Amount <= 0)
    {
        return;
    }

    for (FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == ResourceType)
        {
            Slot.Count += Amount;
            return;
        }
    }

    // New slot
    FQuest_InventorySlot NewSlot;
    NewSlot.ResourceType = ResourceType;
    NewSlot.Count = Amount;
    Inventory.Add(NewSlot);
}

bool UCraftingSystem::RemoveResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (ResourceType == EQuest_ResourceType::None || Amount <= 0)
    {
        return false;
    }

    for (FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == ResourceType)
        {
            if (Slot.Count >= Amount)
            {
                Slot.Count -= Amount;
                return true;
            }
            return false;
        }
    }
    return false;
}

int32 UCraftingSystem::GetResourceCount(EQuest_ResourceType ResourceType) const
{
    for (const FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == ResourceType)
        {
            return Slot.Count;
        }
    }
    return 0;
}

bool UCraftingSystem::CanCraft(EQuest_CraftedItem Item) const
{
    FQuest_CraftingRecipe Recipe;
    if (!GetRecipe(Item, Recipe))
    {
        return false;
    }

    for (const FQuest_RecipeIngredient& Ingredient : Recipe.Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

bool UCraftingSystem::TryCraft(EQuest_CraftedItem Item)
{
    if (!CanCraft(Item))
    {
        return false;
    }

    FQuest_CraftingRecipe Recipe;
    if (!GetRecipe(Item, Recipe))
    {
        return false;
    }

    // Consume ingredients
    for (const FQuest_RecipeIngredient& Ingredient : Recipe.Ingredients)
    {
        RemoveResource(Ingredient.ResourceType, Ingredient.Quantity);
    }

    return true;
}

void UCraftingSystem::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
}

bool UCraftingSystem::GetRecipe(EQuest_CraftedItem Item, FQuest_CraftingRecipe& OutRecipe) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.ResultItem == Item)
        {
            OutRecipe = Recipe;
            return true;
        }
    }
    return false;
}

// ============================================================
// AResourcePickup
// ============================================================

AResourcePickup::AResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
}

void AResourcePickup::BeginPlay()
{
    Super::BeginPlay();
}

void AResourcePickup::OnPickup(AActor* PickingActor)
{
    if (bHasBeenPickedUp || !PickingActor)
    {
        return;
    }

    // Find crafting system on the picking actor
    UCraftingSystem* CraftingComp = PickingActor->FindComponentByClass<UCraftingSystem>();
    if (CraftingComp)
    {
        CraftingComp->AddResource(ResourceType, ResourceAmount);
        bHasBeenPickedUp = true;
        // Hide the actor after pickup
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
    }
}
