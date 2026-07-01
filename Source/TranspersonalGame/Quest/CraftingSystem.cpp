#include "CraftingSystem.h"
#include "Engine/Engine.h"

UCraftingSystem::UCraftingSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultRecipes();
}

// ─── Inventory ────────────────────────────────────────────────────────────────

void UCraftingSystem::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (ResourceType == EQuest_ResourceType::None || Amount <= 0) return;

    FQuest_InventorySlot* Slot = FindSlot(ResourceType);
    if (Slot)
    {
        Slot->Count += Amount;
    }
    else
    {
        FQuest_InventorySlot NewSlot;
        NewSlot.ResourceType = ResourceType;
        NewSlot.Count = Amount;
        Inventory.Add(NewSlot);
    }
    OnInventoryChanged.Broadcast(ResourceType);
}

bool UCraftingSystem::RemoveResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (ResourceType == EQuest_ResourceType::None || Amount <= 0) return false;

    FQuest_InventorySlot* Slot = FindSlot(ResourceType);
    if (!Slot || Slot->Count < Amount) return false;

    Slot->Count -= Amount;
    if (Slot->Count == 0)
    {
        Inventory.RemoveAll([ResourceType](const FQuest_InventorySlot& S)
        {
            return S.ResourceType == ResourceType;
        });
    }
    OnInventoryChanged.Broadcast(ResourceType);
    return true;
}

int32 UCraftingSystem::GetResourceCount(EQuest_ResourceType ResourceType) const
{
    const FQuest_InventorySlot* Slot = FindSlotConst(ResourceType);
    return Slot ? Slot->Count : 0;
}

TArray<FQuest_InventorySlot> UCraftingSystem::GetInventory() const
{
    return Inventory;
}

// ─── Recipes ──────────────────────────────────────────────────────────────────

void UCraftingSystem::InitializeDefaultRecipes()
{
    Recipes.Empty();

    // ── Recipe 1: Stone Axe ──────────────────────────────────────────────
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.RecipeID = FName("Recipe_StoneAxe");
        StoneAxe.DisplayName = FText::FromString(TEXT("Stone Axe"));
        StoneAxe.OutputItemID = FName("StoneAxe_Item");
        StoneAxe.CraftTimeSeconds = 5.0f;
        StoneAxe.bUnlockedByDefault = true;

        FQuest_CraftingIngredient Stone1;
        Stone1.ResourceType = EQuest_ResourceType::Stone;
        Stone1.Quantity = 2;

        FQuest_CraftingIngredient Stick1;
        Stick1.ResourceType = EQuest_ResourceType::Stick;
        Stick1.Quantity = 1;

        StoneAxe.Ingredients.Add(Stone1);
        StoneAxe.Ingredients.Add(Stick1);
        Recipes.Add(StoneAxe);
    }

    // ── Recipe 2: Campfire ───────────────────────────────────────────────
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.RecipeID = FName("Recipe_Campfire");
        Campfire.DisplayName = FText::FromString(TEXT("Campfire"));
        Campfire.OutputItemID = FName("Campfire_Placed");
        Campfire.CraftTimeSeconds = 8.0f;
        Campfire.bUnlockedByDefault = true;

        FQuest_CraftingIngredient Sticks;
        Sticks.ResourceType = EQuest_ResourceType::Stick;
        Sticks.Quantity = 3;

        Campfire.Ingredients.Add(Sticks);
        Recipes.Add(Campfire);
    }

    // ── Recipe 3: Water Container ────────────────────────────────────────
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.RecipeID = FName("Recipe_WaterContainer");
        WaterContainer.DisplayName = FText::FromString(TEXT("Water Container"));
        WaterContainer.OutputItemID = FName("WaterContainer_Item");
        WaterContainer.CraftTimeSeconds = 6.0f;
        WaterContainer.bUnlockedByDefault = true;

        FQuest_CraftingIngredient Stone2;
        Stone2.ResourceType = EQuest_ResourceType::Stone;
        Stone2.Quantity = 1;

        FQuest_CraftingIngredient Leaf1;
        Leaf1.ResourceType = EQuest_ResourceType::Leaf;
        Leaf1.Quantity = 1;

        WaterContainer.Ingredients.Add(Stone2);
        WaterContainer.Ingredients.Add(Leaf1);
        Recipes.Add(WaterContainer);
    }

    // ── Recipe 4: Bone Spear ─────────────────────────────────────────────
    {
        FQuest_CraftingRecipe BoneSpear;
        BoneSpear.RecipeID = FName("Recipe_BoneSpear");
        BoneSpear.DisplayName = FText::FromString(TEXT("Bone Spear"));
        BoneSpear.OutputItemID = FName("BoneSpear_Item");
        BoneSpear.CraftTimeSeconds = 10.0f;
        BoneSpear.bUnlockedByDefault = false;

        FQuest_CraftingIngredient Bone1;
        Bone1.ResourceType = EQuest_ResourceType::Bone;
        Bone1.Quantity = 2;

        FQuest_CraftingIngredient Vine1;
        Vine1.ResourceType = EQuest_ResourceType::Vine;
        Vine1.Quantity = 1;

        BoneSpear.Ingredients.Add(Bone1);
        BoneSpear.Ingredients.Add(Vine1);
        Recipes.Add(BoneSpear);
    }

    // ── Recipe 5: Hide Wrap (cold protection) ────────────────────────────
    {
        FQuest_CraftingRecipe HideWrap;
        HideWrap.RecipeID = FName("Recipe_HideWrap");
        HideWrap.DisplayName = FText::FromString(TEXT("Hide Wrap"));
        HideWrap.OutputItemID = FName("HideWrap_Item");
        HideWrap.CraftTimeSeconds = 12.0f;
        HideWrap.bUnlockedByDefault = false;

        FQuest_CraftingIngredient Hide1;
        Hide1.ResourceType = EQuest_ResourceType::Hide;
        Hide1.Quantity = 2;

        FQuest_CraftingIngredient Vine2;
        Vine2.ResourceType = EQuest_ResourceType::Vine;
        Vine2.Quantity = 1;

        HideWrap.Ingredients.Add(Hide1);
        HideWrap.Ingredients.Add(Vine2);
        Recipes.Add(HideWrap);
    }
}

TArray<FQuest_CraftingRecipe> UCraftingSystem::GetAvailableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Available;
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.bUnlockedByDefault)
        {
            Available.Add(Recipe);
        }
    }
    return Available;
}

bool UCraftingSystem::CanCraft(FName RecipeID) const
{
    const FQuest_CraftingRecipe* Recipe = FindRecipe(RecipeID);
    if (!Recipe) return false;

    for (const FQuest_CraftingIngredient& Ingredient : Recipe->Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

FQuest_CraftResult UCraftingSystem::TryCraft(FName RecipeID)
{
    FQuest_CraftResult Result;

    const FQuest_CraftingRecipe* Recipe = FindRecipe(RecipeID);
    if (!Recipe)
    {
        Result.bSuccess = false;
        Result.FailReason = FText::FromString(TEXT("Recipe not found."));
        return Result;
    }

    if (!CanCraft(RecipeID))
    {
        Result.bSuccess = false;
        Result.FailReason = FText::FromString(TEXT("Not enough resources."));
        return Result;
    }

    // Consume ingredients
    for (const FQuest_CraftingIngredient& Ingredient : Recipe->Ingredients)
    {
        RemoveResource(Ingredient.ResourceType, Ingredient.Quantity);
    }

    Result.bSuccess = true;
    Result.OutputItemID = Recipe->OutputItemID;
    OnItemCrafted.Broadcast(Recipe->OutputItemID);

    return Result;
}

// ─── UI Toggle ────────────────────────────────────────────────────────────────

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

// ─── Internal Helpers ─────────────────────────────────────────────────────────

FQuest_InventorySlot* UCraftingSystem::FindSlot(EQuest_ResourceType ResourceType)
{
    for (FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == ResourceType) return &Slot;
    }
    return nullptr;
}

const FQuest_InventorySlot* UCraftingSystem::FindSlotConst(EQuest_ResourceType ResourceType) const
{
    for (const FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == ResourceType) return &Slot;
    }
    return nullptr;
}

const FQuest_CraftingRecipe* UCraftingSystem::FindRecipe(FName RecipeID) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.RecipeID == RecipeID) return &Recipe;
    }
    return nullptr;
}
