// CraftingSystem.cpp
// Agent #14 — Quest & Mission Designer
// Crafting system: Stone Axe, Campfire, Water Container, Spear
// Recipes, inventory management, resource pickup integration

#include "CraftingSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACraftingSystem::ACraftingSystem()
{
    PrimaryActorTick.bCanEverTick = false;
    bCraftingMenuOpen = false;
    SelectedRecipeIndex = 0;
    InitializeDefaultRecipes();
}

void ACraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    // Re-initialize recipes in case they were cleared
    if (Recipes.Num() == 0)
    {
        InitializeDefaultRecipes();
    }
}

void ACraftingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACraftingSystem::InitializeDefaultRecipes()
{
    Recipes.Empty();

    // Recipe 1: Stone Axe — 2 Rocks + 1 Stick
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.OutputItem = EQuest_CraftedItemType::StoneAxe;
        StoneAxe.RecipeName = TEXT("Stone Axe");
        StoneAxe.CraftingTimeSec = 3.0f;

        FQuest_ResourceIngredient RockIngredient;
        RockIngredient.ResourceType = EQuest_ResourceType::Rock;
        RockIngredient.Quantity = 2;
        StoneAxe.Ingredients.Add(RockIngredient);

        FQuest_ResourceIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity = 1;
        StoneAxe.Ingredients.Add(StickIngredient);

        Recipes.Add(StoneAxe);
    }

    // Recipe 2: Campfire — 3 Sticks
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.OutputItem = EQuest_CraftedItemType::Campfire;
        Campfire.RecipeName = TEXT("Campfire");
        Campfire.CraftingTimeSec = 4.0f;

        FQuest_ResourceIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity = 3;
        Campfire.Ingredients.Add(StickIngredient);

        Recipes.Add(Campfire);
    }

    // Recipe 3: Water Container — 1 Rock + 1 Leaf
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.OutputItem = EQuest_CraftedItemType::WaterContainer;
        WaterContainer.RecipeName = TEXT("Water Container");
        WaterContainer.CraftingTimeSec = 2.5f;

        FQuest_ResourceIngredient RockIngredient;
        RockIngredient.ResourceType = EQuest_ResourceType::Rock;
        RockIngredient.Quantity = 1;
        WaterContainer.Ingredients.Add(RockIngredient);

        FQuest_ResourceIngredient LeafIngredient;
        LeafIngredient.ResourceType = EQuest_ResourceType::Leaf;
        LeafIngredient.Quantity = 1;
        WaterContainer.Ingredients.Add(LeafIngredient);

        Recipes.Add(WaterContainer);
    }

    // Recipe 4: Spear — 1 Stick + 1 Rock + 1 Bone
    {
        FQuest_CraftingRecipe Spear;
        Spear.OutputItem = EQuest_CraftedItemType::Spear;
        Spear.RecipeName = TEXT("Spear");
        Spear.CraftingTimeSec = 5.0f;

        FQuest_ResourceIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity = 1;
        Spear.Ingredients.Add(StickIngredient);

        FQuest_ResourceIngredient RockIngredient;
        RockIngredient.ResourceType = EQuest_ResourceType::Rock;
        RockIngredient.Quantity = 1;
        Spear.Ingredients.Add(RockIngredient);

        FQuest_ResourceIngredient BoneIngredient;
        BoneIngredient.ResourceType = EQuest_ResourceType::Bone;
        BoneIngredient.Quantity = 1;
        Spear.Ingredients.Add(BoneIngredient);

        Recipes.Add(Spear);
    }
}

void ACraftingSystem::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Menu %s"), bCraftingMenuOpen ? TEXT("OPENED") : TEXT("CLOSED"));
}

bool ACraftingSystem::CanCraftRecipe(int32 RecipeIndex) const
{
    if (!Recipes.IsValidIndex(RecipeIndex))
    {
        return false;
    }

    const FQuest_CraftingRecipe& Recipe = Recipes[RecipeIndex];
    for (const FQuest_ResourceIngredient& Ingredient : Recipe.Ingredients)
    {
        int32 Available = GetResourceCount(Ingredient.ResourceType);
        if (Available < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

bool ACraftingSystem::TryCraftItem(int32 RecipeIndex)
{
    if (!Recipes.IsValidIndex(RecipeIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Invalid recipe index %d"), RecipeIndex);
        return false;
    }

    if (!CanCraftRecipe(RecipeIndex))
    {
        UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Cannot craft %s — insufficient resources"),
            *Recipes[RecipeIndex].RecipeName);
        OnCraftingFailed(RecipeIndex);
        return false;
    }

    const FQuest_CraftingRecipe& Recipe = Recipes[RecipeIndex];
    if (ConsumeResources(Recipe))
    {
        UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Crafted [%s] successfully!"), *Recipe.RecipeName);
        OnItemCrafted(Recipe.OutputItem);
        return true;
    }

    return false;
}

bool ACraftingSystem::ConsumeResources(const FQuest_CraftingRecipe& Recipe)
{
    // Verify once more before consuming
    for (const FQuest_ResourceIngredient& Ingredient : Recipe.Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
        {
            return false;
        }
    }

    // Consume
    for (const FQuest_ResourceIngredient& Ingredient : Recipe.Ingredients)
    {
        switch (Ingredient.ResourceType)
        {
            case EQuest_ResourceType::Rock:
                PlayerInventory.Rocks = FMath::Max(0, PlayerInventory.Rocks - Ingredient.Quantity);
                break;
            case EQuest_ResourceType::Stick:
                PlayerInventory.Sticks = FMath::Max(0, PlayerInventory.Sticks - Ingredient.Quantity);
                break;
            case EQuest_ResourceType::Leaf:
                PlayerInventory.Leaves = FMath::Max(0, PlayerInventory.Leaves - Ingredient.Quantity);
                break;
            case EQuest_ResourceType::Bone:
                PlayerInventory.Bones = FMath::Max(0, PlayerInventory.Bones - Ingredient.Quantity);
                break;
            case EQuest_ResourceType::Hide:
                PlayerInventory.Hides = FMath::Max(0, PlayerInventory.Hides - Ingredient.Quantity);
                break;
            default:
                break;
        }
    }
    return true;
}

void ACraftingSystem::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (Amount <= 0) return;

    switch (ResourceType)
    {
        case EQuest_ResourceType::Rock:
            PlayerInventory.Rocks += Amount;
            UE_LOG(LogTemp, Log, TEXT("CraftingSystem: +%d Rock (total: %d)"), Amount, PlayerInventory.Rocks);
            break;
        case EQuest_ResourceType::Stick:
            PlayerInventory.Sticks += Amount;
            UE_LOG(LogTemp, Log, TEXT("CraftingSystem: +%d Stick (total: %d)"), Amount, PlayerInventory.Sticks);
            break;
        case EQuest_ResourceType::Leaf:
            PlayerInventory.Leaves += Amount;
            UE_LOG(LogTemp, Log, TEXT("CraftingSystem: +%d Leaf (total: %d)"), Amount, PlayerInventory.Leaves);
            break;
        case EQuest_ResourceType::Bone:
            PlayerInventory.Bones += Amount;
            UE_LOG(LogTemp, Log, TEXT("CraftingSystem: +%d Bone (total: %d)"), Amount, PlayerInventory.Bones);
            break;
        case EQuest_ResourceType::Hide:
            PlayerInventory.Hides += Amount;
            UE_LOG(LogTemp, Log, TEXT("CraftingSystem: +%d Hide (total: %d)"), Amount, PlayerInventory.Hides);
            break;
        default:
            break;
    }
}

int32 ACraftingSystem::GetResourceCount(EQuest_ResourceType ResourceType) const
{
    switch (ResourceType)
    {
        case EQuest_ResourceType::Rock:   return PlayerInventory.Rocks;
        case EQuest_ResourceType::Stick:  return PlayerInventory.Sticks;
        case EQuest_ResourceType::Leaf:   return PlayerInventory.Leaves;
        case EQuest_ResourceType::Bone:   return PlayerInventory.Bones;
        case EQuest_ResourceType::Hide:   return PlayerInventory.Hides;
        default:                          return 0;
    }
}
