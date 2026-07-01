#include "CraftingSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UCraftingSystem::UCraftingSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    bCraftingMenuOpen = false;

    // Pre-size inventory slots for all resource types
    Inventory.SetNum(10); // One slot per EQuest_ResourceType (excluding None)
    Inventory[0].ResourceType = EQuest_ResourceType::Stone;
    Inventory[1].ResourceType = EQuest_ResourceType::Stick;
    Inventory[2].ResourceType = EQuest_ResourceType::Leaf;
    Inventory[3].ResourceType = EQuest_ResourceType::Bone;
    Inventory[4].ResourceType = EQuest_ResourceType::Hide;
    Inventory[5].ResourceType = EQuest_ResourceType::Flint;
    Inventory[6].ResourceType = EQuest_ResourceType::Vine;
    Inventory[7].ResourceType = EQuest_ResourceType::Water;
    Inventory[8].ResourceType = EQuest_ResourceType::Meat;
    Inventory[9].ResourceType = EQuest_ResourceType::None; // spare
}

void UCraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitialiseRecipes();
}

void UCraftingSystem::InitialiseRecipes()
{
    Recipes.Empty();

    // ── Recipe 1: Stone Axe (2 Stones + 1 Stick) ──────────────────────────
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItem::StoneAxe;
        Recipe.Description = TEXT("A crude but effective axe. Useful for chopping wood and defending against small predators.");
        Recipe.CraftTimeSeconds = 3.0f;
        FQuest_CraftingIngredient Ing1; Ing1.ResourceType = EQuest_ResourceType::Stone; Ing1.Quantity = 2;
        FQuest_CraftingIngredient Ing2; Ing2.ResourceType = EQuest_ResourceType::Stick; Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing1);
        Recipe.Ingredients.Add(Ing2);
        Recipes.Add(Recipe);
    }

    // ── Recipe 2: Campfire (3 Sticks) ─────────────────────────────────────
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItem::Campfire;
        Recipe.Description = TEXT("A small campfire. Keeps predators at bay and allows cooking meat.");
        Recipe.CraftTimeSeconds = 2.0f;
        FQuest_CraftingIngredient Ing1; Ing1.ResourceType = EQuest_ResourceType::Stick; Ing1.Quantity = 3;
        Recipe.Ingredients.Add(Ing1);
        Recipes.Add(Recipe);
    }

    // ── Recipe 3: Water Container (1 Stone + 1 Leaf) ──────────────────────
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItem::WaterContainer;
        Recipe.Description = TEXT("A hollowed stone lined with large leaves. Holds enough water for one day.");
        Recipe.CraftTimeSeconds = 2.5f;
        FQuest_CraftingIngredient Ing1; Ing1.ResourceType = EQuest_ResourceType::Stone; Ing1.Quantity = 1;
        FQuest_CraftingIngredient Ing2; Ing2.ResourceType = EQuest_ResourceType::Leaf; Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing1);
        Recipe.Ingredients.Add(Ing2);
        Recipes.Add(Recipe);
    }

    // ── Recipe 4: Bone Sword (2 Bones + 1 Vine) ───────────────────────────
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItem::BoneSword;
        Recipe.Description = TEXT("Sharpened dinosaur bone bound with vine. More reach than a knife.");
        Recipe.CraftTimeSeconds = 4.0f;
        FQuest_CraftingIngredient Ing1; Ing1.ResourceType = EQuest_ResourceType::Bone; Ing1.Quantity = 2;
        FQuest_CraftingIngredient Ing2; Ing2.ResourceType = EQuest_ResourceType::Vine; Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing1);
        Recipe.Ingredients.Add(Ing2);
        Recipes.Add(Recipe);
    }

    // ── Recipe 5: Hide Shield (2 Hides + 1 Stick) ─────────────────────────
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItem::HideShield;
        Recipe.Description = TEXT("Thick dinosaur hide stretched over a wooden frame. Deflects claw strikes.");
        Recipe.CraftTimeSeconds = 5.0f;
        FQuest_CraftingIngredient Ing1; Ing1.ResourceType = EQuest_ResourceType::Hide; Ing1.Quantity = 2;
        FQuest_CraftingIngredient Ing2; Ing2.ResourceType = EQuest_ResourceType::Stick; Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing1);
        Recipe.Ingredients.Add(Ing2);
        Recipes.Add(Recipe);
    }

    // ── Recipe 6: Flint Knife (1 Flint + 1 Stick) ─────────────────────────
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItem::FlintKnife;
        Recipe.Description = TEXT("A razor-sharp flint blade. Fast and lethal at close range.");
        Recipe.CraftTimeSeconds = 2.0f;
        FQuest_CraftingIngredient Ing1; Ing1.ResourceType = EQuest_ResourceType::Flint; Ing1.Quantity = 1;
        FQuest_CraftingIngredient Ing2; Ing2.ResourceType = EQuest_ResourceType::Stick; Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing1);
        Recipe.Ingredients.Add(Ing2);
        Recipes.Add(Recipe);
    }

    // ── Recipe 7: Vine Rope (3 Vines) ─────────────────────────────────────
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItem::VineRope;
        Recipe.Description = TEXT("Braided vine rope. Used for traps, climbing, and binding tools.");
        Recipe.CraftTimeSeconds = 1.5f;
        FQuest_CraftingIngredient Ing1; Ing1.ResourceType = EQuest_ResourceType::Vine; Ing1.Quantity = 3;
        Recipe.Ingredients.Add(Ing1);
        Recipes.Add(Recipe);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Initialised %d recipes"), Recipes.Num());
}

bool UCraftingSystem::AddResource(EQuest_ResourceType Type, int32 Amount)
{
    if (Type == EQuest_ResourceType::None || Amount <= 0) return false;

    for (FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == Type)
        {
            int32 NewCount = FMath::Min(Slot.Count + Amount, Slot.MaxStack);
            Slot.Count = NewCount;
            OnResourceChanged.Broadcast(Type, NewCount);
            return true;
        }
    }

    // No existing slot — find empty spare slot
    for (FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == EQuest_ResourceType::None)
        {
            Slot.ResourceType = Type;
            Slot.Count = FMath::Min(Amount, Slot.MaxStack);
            OnResourceChanged.Broadcast(Type, Slot.Count);
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Inventory full — cannot add %s"), *UEnum::GetValueAsString(Type));
    return false;
}

bool UCraftingSystem::RemoveResource(EQuest_ResourceType Type, int32 Amount)
{
    if (Type == EQuest_ResourceType::None || Amount <= 0) return false;

    for (FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == Type)
        {
            if (Slot.Count < Amount) return false;
            Slot.Count -= Amount;
            OnResourceChanged.Broadcast(Type, Slot.Count);
            return true;
        }
    }
    return false;
}

int32 UCraftingSystem::GetResourceCount(EQuest_ResourceType Type) const
{
    for (const FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == Type)
        {
            return Slot.Count;
        }
    }
    return 0;
}

bool UCraftingSystem::CanCraft(EQuest_CraftedItem Item) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.ResultItem == Item)
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
    }
    return false;
}

FQuest_CraftResultData UCraftingSystem::TryCraft(EQuest_CraftedItem Item)
{
    FQuest_CraftResultData ResultData;

    // Find recipe
    const FQuest_CraftingRecipe* FoundRecipe = nullptr;
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.ResultItem == Item)
        {
            FoundRecipe = &Recipe;
            break;
        }
    }

    if (!FoundRecipe)
    {
        ResultData.Result = EQuest_CraftResult::InvalidRecipe;
        ResultData.Message = TEXT("No recipe found for this item.");
        OnCraftingComplete.Broadcast(ResultData);
        return ResultData;
    }

    // Check ingredients
    for (const FQuest_CraftingIngredient& Ingredient : FoundRecipe->Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
        {
            ResultData.Result = EQuest_CraftResult::MissingIngredients;
            ResultData.Message = FString::Printf(TEXT("Need more %s."), *UEnum::GetValueAsString(Ingredient.ResourceType));
            OnCraftingComplete.Broadcast(ResultData);
            return ResultData;
        }
    }

    // Consume ingredients
    for (const FQuest_CraftingIngredient& Ingredient : FoundRecipe->Ingredients)
    {
        RemoveResource(Ingredient.ResourceType, Ingredient.Quantity);
    }

    ResultData.Result = EQuest_CraftResult::Success;
    ResultData.CraftedItem = Item;
    ResultData.Message = FString::Printf(TEXT("Crafted: %s"), *UEnum::GetValueAsString(Item));

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: %s"), *ResultData.Message);
    OnCraftingComplete.Broadcast(ResultData);
    return ResultData;
}

TArray<FQuest_CraftingRecipe> UCraftingSystem::GetAvailableRecipes() const
{
    return Recipes;
}

FQuest_CraftingRecipe UCraftingSystem::GetRecipeForItem(EQuest_CraftedItem Item) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.ResultItem == Item)
        {
            return Recipe;
        }
    }
    return FQuest_CraftingRecipe();
}

void UCraftingSystem::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    OnCraftingMenuToggled.Broadcast(bCraftingMenuOpen);
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Menu %s"), bCraftingMenuOpen ? TEXT("OPEN") : TEXT("CLOSED"));
}
