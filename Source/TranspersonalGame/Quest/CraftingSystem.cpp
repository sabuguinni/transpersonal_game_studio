#include "CraftingSystem.h"
#include "Engine/World.h"

UCraftingSystem::UCraftingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UCraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultRecipes();

    // Pre-populate inventory slots for all resource types
    for (uint8 i = 1; i < (uint8)EQuest_ResourceType::Meat + 1; ++i)
    {
        FQuest_InventorySlot Slot;
        Slot.ResourceType = (EQuest_ResourceType)i;
        Slot.Quantity = 0;
        Slot.MaxStack = 10;
        Inventory.Add(Slot);
    }
}

void UCraftingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsCrafting && PendingCraftItem != EQuest_CraftedItem::None)
    {
        const FQuest_CraftingRecipe* Recipe = FindRecipe(PendingCraftItem);
        if (Recipe)
        {
            CurrentCraftTimer += DeltaTime;
            CraftingProgress = FMath::Clamp(CurrentCraftTimer / Recipe->CraftingTimeSeconds, 0.0f, 1.0f);

            if (CurrentCraftTimer >= Recipe->CraftingTimeSeconds)
            {
                // Crafting complete
                bIsCrafting = false;
                CurrentCraftTimer = 0.0f;
                CraftingProgress = 0.0f;
                PendingCraftItem = EQuest_CraftedItem::None;
                UE_LOG(LogTemp, Log, TEXT("UCraftingSystem: Crafting complete!"));
            }
        }
        else
        {
            bIsCrafting = false;
            CurrentCraftTimer = 0.0f;
            CraftingProgress = 0.0f;
            PendingCraftItem = EQuest_CraftedItem::None;
        }
    }
}

void UCraftingSystem::InitializeDefaultRecipes()
{
    AvailableRecipes.Empty();

    // Recipe 1: Stone Axe — 2 Stones + 1 Stick
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::StoneAxe;
        Recipe.RecipeName = TEXT("Stone Axe");
        Recipe.CraftingTimeSeconds = 3.0f;
        Recipe.bRequiresCampfire = false;
        FQuest_CraftingIngredient Ing1; Ing1.ResourceType = EQuest_ResourceType::Stone; Ing1.Quantity = 2;
        FQuest_CraftingIngredient Ing2; Ing2.ResourceType = EQuest_ResourceType::Stick; Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing1);
        Recipe.Ingredients.Add(Ing2);
        AvailableRecipes.Add(Recipe);
    }

    // Recipe 2: Campfire — 3 Sticks
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::Campfire;
        Recipe.RecipeName = TEXT("Campfire");
        Recipe.CraftingTimeSeconds = 2.0f;
        Recipe.bRequiresCampfire = false;
        FQuest_CraftingIngredient Ing1; Ing1.ResourceType = EQuest_ResourceType::Stick; Ing1.Quantity = 3;
        Recipe.Ingredients.Add(Ing1);
        AvailableRecipes.Add(Recipe);
    }

    // Recipe 3: Water Container — 1 Stone + 1 Leaf
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::WaterContainer;
        Recipe.RecipeName = TEXT("Water Container");
        Recipe.CraftingTimeSeconds = 2.5f;
        Recipe.bRequiresCampfire = false;
        FQuest_CraftingIngredient Ing1; Ing1.ResourceType = EQuest_ResourceType::Stone; Ing1.Quantity = 1;
        FQuest_CraftingIngredient Ing2; Ing2.ResourceType = EQuest_ResourceType::Leaf; Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing1);
        Recipe.Ingredients.Add(Ing2);
        AvailableRecipes.Add(Recipe);
    }

    // Recipe 4: Spear — 2 Sticks + 1 Flint
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::Spear;
        Recipe.RecipeName = TEXT("Spear");
        Recipe.CraftingTimeSeconds = 4.0f;
        Recipe.bRequiresCampfire = false;
        FQuest_CraftingIngredient Ing1; Ing1.ResourceType = EQuest_ResourceType::Stick; Ing1.Quantity = 2;
        FQuest_CraftingIngredient Ing2; Ing2.ResourceType = EQuest_ResourceType::Flint; Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing1);
        Recipe.Ingredients.Add(Ing2);
        AvailableRecipes.Add(Recipe);
    }

    // Recipe 5: Leather Wrap — 2 Hides + 1 Vine
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::LeatherWrap;
        Recipe.RecipeName = TEXT("Leather Wrap");
        Recipe.CraftingTimeSeconds = 3.5f;
        Recipe.bRequiresCampfire = false;
        FQuest_CraftingIngredient Ing1; Ing1.ResourceType = EQuest_ResourceType::Hide; Ing1.Quantity = 2;
        FQuest_CraftingIngredient Ing2; Ing2.ResourceType = EQuest_ResourceType::Vine; Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing1);
        Recipe.Ingredients.Add(Ing2);
        AvailableRecipes.Add(Recipe);
    }

    // Recipe 6: Bone Dagger — 2 Bones + 1 Vine
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::BoneDagger;
        Recipe.RecipeName = TEXT("Bone Dagger");
        Recipe.CraftingTimeSeconds = 3.0f;
        Recipe.bRequiresCampfire = false;
        FQuest_CraftingIngredient Ing1; Ing1.ResourceType = EQuest_ResourceType::Bone; Ing1.Quantity = 2;
        FQuest_CraftingIngredient Ing2; Ing2.ResourceType = EQuest_ResourceType::Vine; Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing1);
        Recipe.Ingredients.Add(Ing2);
        AvailableRecipes.Add(Recipe);
    }

    // Recipe 7: Flint Knife — 1 Flint + 1 Stick
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::FlintKnife;
        Recipe.RecipeName = TEXT("Flint Knife");
        Recipe.CraftingTimeSeconds = 2.5f;
        Recipe.bRequiresCampfire = false;
        FQuest_CraftingIngredient Ing1; Ing1.ResourceType = EQuest_ResourceType::Flint; Ing1.Quantity = 1;
        FQuest_CraftingIngredient Ing2; Ing2.ResourceType = EQuest_ResourceType::Stick; Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing1);
        Recipe.Ingredients.Add(Ing2);
        AvailableRecipes.Add(Recipe);
    }

    // Recipe 8: Torch — 1 Stick + 1 Hide
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::Torch;
        Recipe.RecipeName = TEXT("Torch");
        Recipe.CraftingTimeSeconds = 2.0f;
        Recipe.bRequiresCampfire = false;
        FQuest_CraftingIngredient Ing1; Ing1.ResourceType = EQuest_ResourceType::Stick; Ing1.Quantity = 1;
        FQuest_CraftingIngredient Ing2; Ing2.ResourceType = EQuest_ResourceType::Hide; Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing1);
        Recipe.Ingredients.Add(Ing2);
        AvailableRecipes.Add(Recipe);
    }

    UE_LOG(LogTemp, Log, TEXT("UCraftingSystem: Initialized %d recipes"), AvailableRecipes.Num());
}

const FQuest_CraftingRecipe* UCraftingSystem::FindRecipe(EQuest_CraftedItem Item) const
{
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.OutputItem == Item)
        {
            return &Recipe;
        }
    }
    return nullptr;
}

bool UCraftingSystem::HasIngredientsForRecipe(const FQuest_CraftingRecipe& Recipe) const
{
    for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

bool UCraftingSystem::ConsumeIngredients(const FQuest_CraftingRecipe& Recipe)
{
    if (!HasIngredientsForRecipe(Recipe))
    {
        return false;
    }
    for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
    {
        RemoveResource(Ingredient.ResourceType, Ingredient.Quantity);
    }
    return true;
}

FQuest_CraftResultData UCraftingSystem::TryCraft(EQuest_CraftedItem ItemToCraft)
{
    FQuest_CraftResultData ResultData;

    if (bIsCrafting)
    {
        ResultData.Result = EQuest_CraftResult::InvalidRecipe;
        ResultData.Message = TEXT("Already crafting an item.");
        return ResultData;
    }

    const FQuest_CraftingRecipe* Recipe = FindRecipe(ItemToCraft);
    if (!Recipe)
    {
        ResultData.Result = EQuest_CraftResult::InvalidRecipe;
        ResultData.Message = TEXT("No recipe found for this item.");
        return ResultData;
    }

    if (!HasIngredientsForRecipe(*Recipe))
    {
        ResultData.Result = EQuest_CraftResult::MissingIngredients;
        ResultData.Message = FString::Printf(TEXT("Missing ingredients for %s."), *Recipe->RecipeName);
        return ResultData;
    }

    if (ConsumeIngredients(*Recipe))
    {
        bIsCrafting = true;
        PendingCraftItem = ItemToCraft;
        CurrentCraftTimer = 0.0f;
        CraftingProgress = 0.0f;

        ResultData.Result = EQuest_CraftResult::Success;
        ResultData.CraftedItem = ItemToCraft;
        ResultData.Message = FString::Printf(TEXT("Crafting %s..."), *Recipe->RecipeName);

        UE_LOG(LogTemp, Log, TEXT("UCraftingSystem: Started crafting %s"), *Recipe->RecipeName);
    }
    else
    {
        ResultData.Result = EQuest_CraftResult::MissingIngredients;
        ResultData.Message = TEXT("Failed to consume ingredients.");
    }

    return ResultData;
}

bool UCraftingSystem::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (ResourceType == EQuest_ResourceType::None || Amount <= 0)
    {
        return false;
    }

    for (FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == ResourceType)
        {
            int32 SpaceAvailable = Slot.MaxStack - Slot.Quantity;
            if (SpaceAvailable > 0)
            {
                Slot.Quantity += FMath::Min(Amount, SpaceAvailable);
                UE_LOG(LogTemp, Log, TEXT("UCraftingSystem: Added %d x %d (total: %d)"),
                    Amount, (int32)ResourceType, Slot.Quantity);
                return true;
            }
            return false; // Slot full
        }
    }

    // No existing slot — create new one if space available
    if (Inventory.Num() < MaxInventorySlots)
    {
        FQuest_InventorySlot NewSlot;
        NewSlot.ResourceType = ResourceType;
        NewSlot.Quantity = FMath::Min(Amount, NewSlot.MaxStack);
        Inventory.Add(NewSlot);
        return true;
    }

    return false; // Inventory full
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
            if (Slot.Quantity >= Amount)
            {
                Slot.Quantity -= Amount;
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
            return Slot.Quantity;
        }
    }
    return 0;
}

TArray<FQuest_CraftingRecipe> UCraftingSystem::GetAvailableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Craftable;
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (HasIngredientsForRecipe(Recipe))
        {
            Craftable.Add(Recipe);
        }
    }
    return Craftable;
}

void UCraftingSystem::OpenCraftingMenu()
{
    bCraftingMenuOpen = true;
    UE_LOG(LogTemp, Log, TEXT("UCraftingSystem: Crafting menu opened"));
}

void UCraftingSystem::CloseCraftingMenu()
{
    bCraftingMenuOpen = false;
    UE_LOG(LogTemp, Log, TEXT("UCraftingSystem: Crafting menu closed"));
}

void UCraftingSystem::ToggleCraftingMenu()
{
    if (bCraftingMenuOpen)
    {
        CloseCraftingMenu();
    }
    else
    {
        OpenCraftingMenu();
    }
}
