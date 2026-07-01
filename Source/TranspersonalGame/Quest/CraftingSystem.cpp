#include "CraftingSystem.h"
#include "Engine/Engine.h"

// ============================================================
// Constructor
// ============================================================

UCraftingSystem::UCraftingSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    bCraftingMenuOpen = false;
}

// ============================================================
// BeginPlay
// ============================================================

void UCraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultRecipes();
}

// ============================================================
// InitializeDefaultRecipes — 3 core recipes + 4 advanced
// ============================================================

void UCraftingSystem::InitializeDefaultRecipes()
{
    Recipes.Empty();

    // --- Recipe 1: Stone Axe (2 Stones + 1 Stick + 1 Vine) ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::StoneAxe;
        Recipe.RecipeName = TEXT("Stone Axe");
        Recipe.Description = TEXT("A crude but effective chopping tool. Splits bone and cuts hide.");
        Recipe.CraftTimeSeconds = 3.0f;
        Recipe.bRequiresCampfire = false;

        FQuest_CraftingIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Stone;
        Ing1.Quantity = 2;
        Recipe.Ingredients.Add(Ing1);

        FQuest_CraftingIngredient Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Stick;
        Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing2);

        FQuest_CraftingIngredient Ing3;
        Ing3.ResourceType = EQuest_ResourceType::Vine;
        Ing3.Quantity = 1;
        Recipe.Ingredients.Add(Ing3);

        Recipes.Add(Recipe);
    }

    // --- Recipe 2: Campfire (3 Sticks + 1 Stone) ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::Campfire;
        Recipe.RecipeName = TEXT("Campfire");
        Recipe.Description = TEXT("Provides warmth, cooks meat, and deters small predators at night.");
        Recipe.CraftTimeSeconds = 5.0f;
        Recipe.bRequiresCampfire = false;

        FQuest_CraftingIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Stick;
        Ing1.Quantity = 3;
        Recipe.Ingredients.Add(Ing1);

        FQuest_CraftingIngredient Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Stone;
        Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing2);

        Recipes.Add(Recipe);
    }

    // --- Recipe 3: Water Container (1 Stone + 1 Leaf) ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::WaterContainer;
        Recipe.RecipeName = TEXT("Water Container");
        Recipe.Description = TEXT("A hollowed stone lined with large leaves. Holds enough water for one day.");
        Recipe.CraftTimeSeconds = 2.0f;
        Recipe.bRequiresCampfire = false;

        FQuest_CraftingIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Stone;
        Ing1.Quantity = 1;
        Recipe.Ingredients.Add(Ing1);

        FQuest_CraftingIngredient Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Leaf;
        Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing2);

        Recipes.Add(Recipe);
    }

    // --- Recipe 4: Flint Knife (2 Flint + 1 Vine) ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::FlintKnife;
        Recipe.RecipeName = TEXT("Flint Knife");
        Recipe.Description = TEXT("A razor-sharp blade for skinning animals and close combat.");
        Recipe.CraftTimeSeconds = 4.0f;
        Recipe.bRequiresCampfire = false;

        FQuest_CraftingIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Flint;
        Ing1.Quantity = 2;
        Recipe.Ingredients.Add(Ing1);

        FQuest_CraftingIngredient Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Vine;
        Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing2);

        Recipes.Add(Recipe);
    }

    // --- Recipe 5: Bone Sword (3 Bone + 2 Vine) — requires campfire ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::BoneSword;
        Recipe.RecipeName = TEXT("Bone Sword");
        Recipe.Description = TEXT("A long blade shaped from dinosaur bone. Hardened over fire.");
        Recipe.CraftTimeSeconds = 8.0f;
        Recipe.bRequiresCampfire = true;

        FQuest_CraftingIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Bone;
        Ing1.Quantity = 3;
        Recipe.Ingredients.Add(Ing1);

        FQuest_CraftingIngredient Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Vine;
        Ing2.Quantity = 2;
        Recipe.Ingredients.Add(Ing2);

        Recipes.Add(Recipe);
    }

    // --- Recipe 6: Hide Armor (4 Hide + 3 Vine) ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::HideArmor;
        Recipe.RecipeName = TEXT("Hide Armor");
        Recipe.Description = TEXT("Rough armor from dinosaur hide. Reduces bite and claw damage.");
        Recipe.CraftTimeSeconds = 10.0f;
        Recipe.bRequiresCampfire = false;

        FQuest_CraftingIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Hide;
        Ing1.Quantity = 4;
        Recipe.Ingredients.Add(Ing1);

        FQuest_CraftingIngredient Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Vine;
        Ing2.Quantity = 3;
        Recipe.Ingredients.Add(Ing2);

        Recipes.Add(Recipe);
    }

    // --- Recipe 7: Vine Rope (4 Vine) ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::VineRope;
        Recipe.RecipeName = TEXT("Vine Rope");
        Recipe.Description = TEXT("Braided vine rope. Used for traps, climbing, and binding.");
        Recipe.CraftTimeSeconds = 2.0f;
        Recipe.bRequiresCampfire = false;

        FQuest_CraftingIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Vine;
        Ing1.Quantity = 4;
        Recipe.Ingredients.Add(Ing1);

        Recipes.Add(Recipe);
    }
}

// ============================================================
// Inventory — Add Resource
// ============================================================

bool UCraftingSystem::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (ResourceType == EQuest_ResourceType::None || Amount <= 0)
    {
        return false;
    }

    FQuest_InventorySlot* Slot = FindOrCreateSlot(ResourceType);
    if (!Slot)
    {
        return false; // Inventory full
    }

    int32 SpaceAvailable = Slot->MaxStack - Slot->Quantity;
    int32 ToAdd = FMath::Min(Amount, SpaceAvailable);
    Slot->Quantity += ToAdd;

    OnResourceChanged.Broadcast(ResourceType);
    return ToAdd == Amount;
}

// ============================================================
// Inventory — Remove Resource
// ============================================================

bool UCraftingSystem::RemoveResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (ResourceType == EQuest_ResourceType::None || Amount <= 0)
    {
        return false;
    }

    FQuest_InventorySlot* Slot = FindSlot(ResourceType);
    if (!Slot || Slot->Quantity < Amount)
    {
        return false;
    }

    Slot->Quantity -= Amount;
    if (Slot->Quantity <= 0)
    {
        Inventory.RemoveAll([ResourceType](const FQuest_InventorySlot& S)
        {
            return S.ResourceType == ResourceType;
        });
    }

    OnResourceChanged.Broadcast(ResourceType);
    return true;
}

// ============================================================
// Inventory — Query
// ============================================================

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

bool UCraftingSystem::HasResource(EQuest_ResourceType ResourceType, int32 Amount) const
{
    return GetResourceCount(ResourceType) >= Amount;
}

// ============================================================
// Crafting — CanCraft
// ============================================================

bool UCraftingSystem::CanCraft(EQuest_CraftedItem ItemToCraft) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.OutputItem == ItemToCraft)
        {
            for (const FQuest_CraftingIngredient& Ing : Recipe.Ingredients)
            {
                if (!HasResource(Ing.ResourceType, Ing.Quantity))
                {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

// ============================================================
// Crafting — TryCraft
// ============================================================

FQuest_CraftResultData UCraftingSystem::TryCraft(EQuest_CraftedItem ItemToCraft)
{
    FQuest_CraftResultData ResultData;

    FQuest_CraftingRecipe* Recipe = FindRecipe(ItemToCraft);
    if (!Recipe)
    {
        ResultData.Result = EQuest_CraftResult::InvalidRecipe;
        ResultData.Message = TEXT("No recipe found for this item.");
        return ResultData;
    }

    // Check all ingredients
    for (const FQuest_CraftingIngredient& Ing : Recipe->Ingredients)
    {
        if (!HasResource(Ing.ResourceType, Ing.Quantity))
        {
            ResultData.Result = EQuest_CraftResult::MissingIngredients;
            ResultData.Message = FString::Printf(TEXT("Missing ingredients for %s."), *Recipe->RecipeName);
            return ResultData;
        }
    }

    // Consume ingredients
    for (const FQuest_CraftingIngredient& Ing : Recipe->Ingredients)
    {
        RemoveResource(Ing.ResourceType, Ing.Quantity);
    }

    // Success
    ResultData.Result = EQuest_CraftResult::Success;
    ResultData.CraftedItem = ItemToCraft;
    ResultData.Message = FString::Printf(TEXT("Crafted: %s"), *Recipe->RecipeName);

    OnItemCrafted.Broadcast(ItemToCraft);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green,
            FString::Printf(TEXT("[Crafting] %s"), *ResultData.Message));
    }

    return ResultData;
}

// ============================================================
// Crafting — GetAvailableRecipes
// ============================================================

TArray<FQuest_CraftingRecipe> UCraftingSystem::GetAvailableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Available;
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        bool bHasAll = true;
        for (const FQuest_CraftingIngredient& Ing : Recipe.Ingredients)
        {
            if (!HasResource(Ing.ResourceType, Ing.Quantity))
            {
                bHasAll = false;
                break;
            }
        }
        if (bHasAll)
        {
            Available.Add(Recipe);
        }
    }
    return Available;
}

// ============================================================
// Crafting Menu
// ============================================================

void UCraftingSystem::ToggleCraftingMenu()
{
    bCraftingMenuOpen ? CloseCraftingMenu() : OpenCraftingMenu();
}

void UCraftingSystem::OpenCraftingMenu()
{
    bCraftingMenuOpen = true;
    OnCraftingMenuToggled.Broadcast(true);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
            TEXT("[Crafting] Menu opened — press C to close"));
    }
}

void UCraftingSystem::CloseCraftingMenu()
{
    bCraftingMenuOpen = false;
    OnCraftingMenuToggled.Broadcast(false);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White,
            TEXT("[Crafting] Menu closed"));
    }
}

// ============================================================
// Private Helpers
// ============================================================

FQuest_CraftingRecipe* UCraftingSystem::FindRecipe(EQuest_CraftedItem ItemToCraft)
{
    for (FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.OutputItem == ItemToCraft)
        {
            return &Recipe;
        }
    }
    return nullptr;
}

FQuest_InventorySlot* UCraftingSystem::FindSlot(EQuest_ResourceType ResourceType)
{
    for (FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == ResourceType)
        {
            return &Slot;
        }
    }
    return nullptr;
}

FQuest_InventorySlot* UCraftingSystem::FindOrCreateSlot(EQuest_ResourceType ResourceType)
{
    // Try to find existing slot
    FQuest_InventorySlot* Existing = FindSlot(ResourceType);
    if (Existing)
    {
        return Existing;
    }

    // Create new slot if inventory not full
    if (Inventory.Num() >= MaxInventorySlots)
    {
        return nullptr;
    }

    FQuest_InventorySlot NewSlot;
    NewSlot.ResourceType = ResourceType;
    NewSlot.Quantity = 0;
    NewSlot.MaxStack = 10;
    Inventory.Add(NewSlot);

    return &Inventory.Last();
}
