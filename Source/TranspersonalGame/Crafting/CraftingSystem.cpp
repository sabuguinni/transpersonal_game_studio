#include "CraftingSystem.h"
#include "Engine/World.h"

UCraftingSystem::UCraftingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    InitializeDefaultRecipes();
}

void UCraftingSystem::BeginPlay()
{
    Super::BeginPlay();
}

void UCraftingSystem::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsCrafting)
    {
        CraftingTimer += DeltaTime;
        CraftingProgress = FMath::Clamp(CraftingTimer / CurrentCraftDuration, 0.0f, 1.0f);

        if (CraftingTimer >= CurrentCraftDuration)
        {
            CompleteCrafting();
        }
    }
}

// ============================================================
// PUBLIC METHODS
// ============================================================

void UCraftingSystem::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    OnCraftingMenuToggled.Broadcast();
}

bool UCraftingSystem::CanCraftItem(EQuest_CraftedItemType ItemType) const
{
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.OutputItem != ItemType) continue;

        for (const FQuest_RecipeIngredient& Ingredient : Recipe.Ingredients)
        {
            if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool UCraftingSystem::StartCrafting(EQuest_CraftedItemType ItemType)
{
    if (bIsCrafting) return false;
    if (!CanCraftItem(ItemType)) return false;

    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.OutputItem != ItemType) continue;

        if (ConsumeIngredients(Recipe))
        {
            bIsCrafting = true;
            CurrentlyCraftingItem = ItemType;
            CraftingTimer = 0.0f;
            CurrentCraftDuration = Recipe.CraftTimeSeconds;
            CraftingProgress = 0.0f;
            return true;
        }
    }
    return false;
}

void UCraftingSystem::CancelCrafting()
{
    if (!bIsCrafting) return;

    // Refund ingredients
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.OutputItem != CurrentlyCraftingItem) continue;

        for (const FQuest_RecipeIngredient& Ingredient : Recipe.Ingredients)
        {
            AddResource(Ingredient.ResourceType, Ingredient.Quantity);
        }
        break;
    }

    bIsCrafting = false;
    CurrentlyCraftingItem = EQuest_CraftedItemType::None;
    CraftingTimer = 0.0f;
    CraftingProgress = 0.0f;
}

void UCraftingSystem::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    switch (ResourceType)
    {
        case EQuest_ResourceType::Rock:   Inventory.Rocks   += Amount; break;
        case EQuest_ResourceType::Stick:  Inventory.Sticks  += Amount; break;
        case EQuest_ResourceType::Leaf:   Inventory.Leaves  += Amount; break;
        case EQuest_ResourceType::Flint:  Inventory.Flint   += Amount; break;
        case EQuest_ResourceType::Bone:   Inventory.Bones   += Amount; break;
        case EQuest_ResourceType::Hide:   Inventory.Hides   += Amount; break;
        case EQuest_ResourceType::Fiber:  Inventory.Fiber   += Amount; break;
        default: break;
    }
}

int32 UCraftingSystem::GetResourceCount(EQuest_ResourceType ResourceType) const
{
    switch (ResourceType)
    {
        case EQuest_ResourceType::Rock:   return Inventory.Rocks;
        case EQuest_ResourceType::Stick:  return Inventory.Sticks;
        case EQuest_ResourceType::Leaf:   return Inventory.Leaves;
        case EQuest_ResourceType::Flint:  return Inventory.Flint;
        case EQuest_ResourceType::Bone:   return Inventory.Bones;
        case EQuest_ResourceType::Hide:   return Inventory.Hides;
        case EQuest_ResourceType::Fiber:  return Inventory.Fiber;
        default: return 0;
    }
}

TArray<FQuest_CraftingRecipe> UCraftingSystem::GetAvailableRecipes() const
{
    return AvailableRecipes;
}

// ============================================================
// PRIVATE METHODS
// ============================================================

void UCraftingSystem::InitializeDefaultRecipes()
{
    AvailableRecipes.Empty();

    // --- Recipe 1: Stone Axe (2 Rocks + 1 Stick) ---
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.OutputItem = EQuest_CraftedItemType::StoneAxe;
        StoneAxe.DisplayName = TEXT("Stone Axe");
        StoneAxe.Description = TEXT("A crude but effective chopping tool. Essential for cutting wood and processing carcasses.");
        StoneAxe.CraftTimeSeconds = 3.0f;
        StoneAxe.bRequiresCraftingStation = false;

        FQuest_RecipeIngredient RockIngredient;
        RockIngredient.ResourceType = EQuest_ResourceType::Rock;
        RockIngredient.Quantity = 2;
        StoneAxe.Ingredients.Add(RockIngredient);

        FQuest_RecipeIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity = 1;
        StoneAxe.Ingredients.Add(StickIngredient);

        AvailableRecipes.Add(StoneAxe);
    }

    // --- Recipe 2: Campfire (3 Sticks) ---
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.OutputItem = EQuest_CraftedItemType::Campfire;
        Campfire.DisplayName = TEXT("Campfire");
        Campfire.Description = TEXT("Warmth, light, and cooked food. The campfire is the heart of any survivor's camp.");
        Campfire.CraftTimeSeconds = 5.0f;
        Campfire.bRequiresCraftingStation = false;

        FQuest_RecipeIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity = 3;
        Campfire.Ingredients.Add(StickIngredient);

        AvailableRecipes.Add(Campfire);
    }

    // --- Recipe 3: Water Container (1 Rock + 1 Leaf) ---
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.OutputItem = EQuest_CraftedItemType::WaterContainer;
        WaterContainer.DisplayName = TEXT("Water Container");
        WaterContainer.Description = TEXT("A hollowed rock lined with large leaves. Holds enough water for one day.");
        WaterContainer.CraftTimeSeconds = 4.0f;
        WaterContainer.bRequiresCraftingStation = false;

        FQuest_RecipeIngredient RockIngredient;
        RockIngredient.ResourceType = EQuest_ResourceType::Rock;
        RockIngredient.Quantity = 1;
        WaterContainer.Ingredients.Add(RockIngredient);

        FQuest_RecipeIngredient LeafIngredient;
        LeafIngredient.ResourceType = EQuest_ResourceType::Leaf;
        LeafIngredient.Quantity = 1;
        WaterContainer.Ingredients.Add(LeafIngredient);

        AvailableRecipes.Add(WaterContainer);
    }

    // --- Recipe 4: Spear (2 Sticks + 1 Flint) ---
    {
        FQuest_CraftingRecipe Spear;
        Spear.OutputItem = EQuest_CraftedItemType::Spear;
        Spear.DisplayName = TEXT("Flint Spear");
        Spear.Description = TEXT("A long shaft tipped with sharp flint. Effective for hunting and keeping predators at bay.");
        Spear.CraftTimeSeconds = 6.0f;
        Spear.bRequiresCraftingStation = false;

        FQuest_RecipeIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity = 2;
        Spear.Ingredients.Add(StickIngredient);

        FQuest_RecipeIngredient FlintIngredient;
        FlintIngredient.ResourceType = EQuest_ResourceType::Flint;
        FlintIngredient.Quantity = 1;
        Spear.Ingredients.Add(FlintIngredient);

        AvailableRecipes.Add(Spear);
    }

    // --- Recipe 5: Skin Pouch (2 Hides + 1 Fiber) ---
    {
        FQuest_CraftingRecipe SkinPouch;
        SkinPouch.OutputItem = EQuest_CraftedItemType::SkinPouch;
        SkinPouch.DisplayName = TEXT("Skin Pouch");
        SkinPouch.Description = TEXT("A carrying pouch made from animal hide. Increases inventory capacity.");
        SkinPouch.CraftTimeSeconds = 8.0f;
        SkinPouch.bRequiresCraftingStation = true;

        FQuest_RecipeIngredient HideIngredient;
        HideIngredient.ResourceType = EQuest_ResourceType::Hide;
        HideIngredient.Quantity = 2;
        SkinPouch.Ingredients.Add(HideIngredient);

        FQuest_RecipeIngredient FiberIngredient;
        FiberIngredient.ResourceType = EQuest_ResourceType::Fiber;
        FiberIngredient.Quantity = 1;
        SkinPouch.Ingredients.Add(FiberIngredient);

        AvailableRecipes.Add(SkinPouch);
    }
}

bool UCraftingSystem::ConsumeIngredients(const FQuest_CraftingRecipe& Recipe)
{
    // Verify all ingredients available first
    for (const FQuest_RecipeIngredient& Ingredient : Recipe.Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
        {
            return false;
        }
    }

    // Consume them
    for (const FQuest_RecipeIngredient& Ingredient : Recipe.Ingredients)
    {
        AddResource(Ingredient.ResourceType, -Ingredient.Quantity);
    }
    return true;
}

void UCraftingSystem::CompleteCrafting()
{
    EQuest_CraftedItemType CompletedItem = CurrentlyCraftingItem;

    bIsCrafting = false;
    CurrentlyCraftingItem = EQuest_CraftedItemType::None;
    CraftingTimer = 0.0f;
    CraftingProgress = 1.0f;

    OnItemCrafted.Broadcast(CompletedItem);
}
