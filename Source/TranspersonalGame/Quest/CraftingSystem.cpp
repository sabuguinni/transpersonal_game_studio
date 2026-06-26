// CraftingSystem.cpp
// Agent #14 — Quest & Mission Designer
// Implements resource pickup actor and crafting component with 3 core recipes.

#include "CraftingSystem.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

// ============================================================
// AQuest_ResourcePickup
// ============================================================

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
}

bool AQuest_ResourcePickup::TryPickup(FQuest_PlayerInventory& OutInventory)
{
    if (bHasBeenPickedUp)
    {
        return false;
    }

    switch (ResourceType)
    {
        case EQuest_ResourceType::Rock:
            OutInventory.Rocks += Quantity;
            break;
        case EQuest_ResourceType::Stick:
            OutInventory.Sticks += Quantity;
            break;
        case EQuest_ResourceType::Leaf:
            OutInventory.Leaves += Quantity;
            break;
        case EQuest_ResourceType::Bone:
            OutInventory.Bones += Quantity;
            break;
        case EQuest_ResourceType::Hide:
            OutInventory.Hides += Quantity;
            break;
        case EQuest_ResourceType::Flint:
            OutInventory.Flint += Quantity;
            break;
        default:
            return false;
    }

    bHasBeenPickedUp = true;
    // Hide the actor after pickup
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    return true;
}

// ============================================================
// UQuest_CraftingComponent
// ============================================================

UQuest_CraftingComponent::UQuest_CraftingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UQuest_CraftingComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeRecipes();
}

void UQuest_CraftingComponent::InitializeRecipes()
{
    Recipes.Empty();

    // --- Recipe 1: Stone Axe (2 Rocks + 1 Stick) ---
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.ResultItem         = EQuest_CraftedItem::StoneAxe;
        StoneAxe.RecipeName         = TEXT("Stone Axe");
        StoneAxe.Description        = TEXT("A crude but effective chopping tool. Used to cut wood and defend against small predators.");
        StoneAxe.CraftingTimeSeconds = 4.0f;

        FQuest_CraftingIngredient Rock1;
        Rock1.ResourceType = EQuest_ResourceType::Rock;
        Rock1.Quantity     = 2;
        StoneAxe.Ingredients.Add(Rock1);

        FQuest_CraftingIngredient Stick1;
        Stick1.ResourceType = EQuest_ResourceType::Stick;
        Stick1.Quantity     = 1;
        StoneAxe.Ingredients.Add(Stick1);

        Recipes.Add(StoneAxe);
    }

    // --- Recipe 2: Campfire (3 Sticks) ---
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.ResultItem          = EQuest_CraftedItem::Campfire;
        Campfire.RecipeName          = TEXT("Campfire");
        Campfire.Description         = TEXT("A basic fire. Keeps predators away at night, cooks meat, and restores warmth.");
        Campfire.CraftingTimeSeconds = 6.0f;

        FQuest_CraftingIngredient Stick2;
        Stick2.ResourceType = EQuest_ResourceType::Stick;
        Stick2.Quantity     = 3;
        Campfire.Ingredients.Add(Stick2);

        Recipes.Add(Campfire);
    }

    // --- Recipe 3: Water Container (1 Rock + 1 Leaf) ---
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.ResultItem          = EQuest_CraftedItem::WaterContainer;
        WaterContainer.RecipeName          = TEXT("Water Container");
        WaterContainer.Description         = TEXT("A hollowed rock lined with large leaves. Holds water for 2 days of travel.");
        WaterContainer.CraftingTimeSeconds = 5.0f;

        FQuest_CraftingIngredient Rock2;
        Rock2.ResourceType = EQuest_ResourceType::Rock;
        Rock2.Quantity     = 1;
        WaterContainer.Ingredients.Add(Rock2);

        FQuest_CraftingIngredient Leaf1;
        Leaf1.ResourceType = EQuest_ResourceType::Leaf;
        Leaf1.Quantity     = 1;
        WaterContainer.Ingredients.Add(Leaf1);

        Recipes.Add(WaterContainer);
    }

    // --- Recipe 4: Spear (2 Sticks + 1 Flint) ---
    {
        FQuest_CraftingRecipe Spear;
        Spear.ResultItem          = EQuest_CraftedItem::Spear;
        Spear.RecipeName          = TEXT("Flint Spear");
        Spear.Description         = TEXT("A sharpened flint tip bound to a long stick. Effective against medium-sized predators.");
        Spear.CraftingTimeSeconds = 8.0f;

        FQuest_CraftingIngredient Stick3;
        Stick3.ResourceType = EQuest_ResourceType::Stick;
        Stick3.Quantity     = 2;
        Spear.Ingredients.Add(Stick3);

        FQuest_CraftingIngredient Flint1;
        Flint1.ResourceType = EQuest_ResourceType::Flint;
        Flint1.Quantity     = 1;
        Spear.Ingredients.Add(Flint1);

        Recipes.Add(Spear);
    }

    // --- Recipe 5: Shelter (3 Sticks + 2 Leaves) ---
    {
        FQuest_CraftingRecipe Shelter;
        Shelter.ResultItem          = EQuest_CraftedItem::Shelter;
        Shelter.RecipeName          = TEXT("Lean-To Shelter");
        Shelter.Description         = TEXT("A simple lean-to made from branches and leaves. Provides protection from rain and cold nights.");
        Shelter.CraftingTimeSeconds = 12.0f;

        FQuest_CraftingIngredient Stick4;
        Stick4.ResourceType = EQuest_ResourceType::Stick;
        Stick4.Quantity     = 3;
        Shelter.Ingredients.Add(Stick4);

        FQuest_CraftingIngredient Leaf2;
        Leaf2.ResourceType = EQuest_ResourceType::Leaf;
        Leaf2.Quantity     = 2;
        Shelter.Ingredients.Add(Leaf2);

        Recipes.Add(Shelter);
    }
}

bool UQuest_CraftingComponent::CanCraft(EQuest_CraftedItem Item) const
{
    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (!Recipe)
    {
        return false;
    }

    for (const FQuest_CraftingIngredient& Ingredient : Recipe->Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

bool UQuest_CraftingComponent::CraftItem(EQuest_CraftedItem Item)
{
    if (!CanCraft(Item))
    {
        return false;
    }

    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (!Recipe)
    {
        return false;
    }

    if (!ConsumeIngredients(*Recipe))
    {
        return false;
    }

    Inventory.CraftedItems.Add(Item);
    return true;
}

void UQuest_CraftingComponent::AddResource(EQuest_ResourceType Type, int32 Amount)
{
    if (Amount <= 0)
    {
        return;
    }

    switch (Type)
    {
        case EQuest_ResourceType::Rock:   Inventory.Rocks   += Amount; break;
        case EQuest_ResourceType::Stick:  Inventory.Sticks  += Amount; break;
        case EQuest_ResourceType::Leaf:   Inventory.Leaves  += Amount; break;
        case EQuest_ResourceType::Bone:   Inventory.Bones   += Amount; break;
        case EQuest_ResourceType::Hide:   Inventory.Hides   += Amount; break;
        case EQuest_ResourceType::Flint:  Inventory.Flint   += Amount; break;
        default: break;
    }
}

int32 UQuest_CraftingComponent::GetResourceCount(EQuest_ResourceType Type) const
{
    switch (Type)
    {
        case EQuest_ResourceType::Rock:   return Inventory.Rocks;
        case EQuest_ResourceType::Stick:  return Inventory.Sticks;
        case EQuest_ResourceType::Leaf:   return Inventory.Leaves;
        case EQuest_ResourceType::Bone:   return Inventory.Bones;
        case EQuest_ResourceType::Hide:   return Inventory.Hides;
        case EQuest_ResourceType::Flint:  return Inventory.Flint;
        default: return 0;
    }
}

void UQuest_CraftingComponent::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
}

TArray<FQuest_CraftingRecipe> UQuest_CraftingComponent::GetAvailableRecipes() const
{
    return Recipes;
}

FQuest_CraftingRecipe UQuest_CraftingComponent::GetRecipeForItem(EQuest_CraftedItem Item) const
{
    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (Recipe)
    {
        return *Recipe;
    }
    return FQuest_CraftingRecipe();
}

// --- Private ---

bool UQuest_CraftingComponent::ConsumeIngredients(const FQuest_CraftingRecipe& Recipe)
{
    // Verify all ingredients available first
    for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
        {
            return false;
        }
    }

    // Consume
    for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
    {
        switch (Ingredient.ResourceType)
        {
            case EQuest_ResourceType::Rock:   Inventory.Rocks   -= Ingredient.Quantity; break;
            case EQuest_ResourceType::Stick:  Inventory.Sticks  -= Ingredient.Quantity; break;
            case EQuest_ResourceType::Leaf:   Inventory.Leaves  -= Ingredient.Quantity; break;
            case EQuest_ResourceType::Bone:   Inventory.Bones   -= Ingredient.Quantity; break;
            case EQuest_ResourceType::Hide:   Inventory.Hides   -= Ingredient.Quantity; break;
            case EQuest_ResourceType::Flint:  Inventory.Flint   -= Ingredient.Quantity; break;
            default: break;
        }
    }
    return true;
}

const FQuest_CraftingRecipe* UQuest_CraftingComponent::FindRecipe(EQuest_CraftedItem Item) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.ResultItem == Item)
        {
            return &Recipe;
        }
    }
    return nullptr;
}
