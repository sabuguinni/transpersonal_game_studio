#include "CraftingSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UCraftingSystem::UCraftingSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    bCraftingMenuOpen = false;
    MaxInventorySlots = 20;
    MaxStackSize = 99;
    bRequireProximityToCampfire = false;
}

void UCraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultRecipes();
    UE_LOG(LogTemp, Log, TEXT("[CraftingSystem] Initialized with %d recipes"), RecipeMap.Num());
}

// ─── Recipe Management ────────────────────────────────────────────────────────

void UCraftingSystem::InitializeDefaultRecipes()
{
    RecipeMap.Empty();

    // ── Recipe 1: Stone Axe ──────────────────────────────────────────────
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.RecipeID       = FName("Recipe_StoneAxe");
        StoneAxe.DisplayName    = FText::FromString("Stone Axe");
        StoneAxe.OutputItemID   = FName("StoneAxe_Item");
        StoneAxe.OutputQuantity = 1;
        StoneAxe.CraftTimeSeconds = 5.0f;
        StoneAxe.Description    = FText::FromString("A crude but effective cutting tool. Useful for chopping wood and processing animal hides.");

        FQuest_CraftingIngredient Stone;
        Stone.ResourceType = EQuest_ResourceType::Stone;
        Stone.Quantity     = 2;

        FQuest_CraftingIngredient Stick;
        Stick.ResourceType = EQuest_ResourceType::Stick;
        Stick.Quantity     = 1;

        StoneAxe.Ingredients.Add(Stone);
        StoneAxe.Ingredients.Add(Stick);
        RecipeMap.Add(StoneAxe.RecipeID, StoneAxe);
    }

    // ── Recipe 2: Campfire ───────────────────────────────────────────────
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.RecipeID       = FName("Recipe_Campfire");
        Campfire.DisplayName    = FText::FromString("Campfire");
        Campfire.OutputItemID   = FName("Campfire_Placed");
        Campfire.OutputQuantity = 1;
        Campfire.CraftTimeSeconds = 8.0f;
        Campfire.Description    = FText::FromString("A basic campfire. Provides warmth, deters predators, and allows cooking of raw meat.");

        FQuest_CraftingIngredient Sticks;
        Sticks.ResourceType = EQuest_ResourceType::Stick;
        Sticks.Quantity     = 3;

        Campfire.Ingredients.Add(Sticks);
        RecipeMap.Add(Campfire.RecipeID, Campfire);
    }

    // ── Recipe 3: Water Container ────────────────────────────────────────
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.RecipeID       = FName("Recipe_WaterContainer");
        WaterContainer.DisplayName    = FText::FromString("Water Container");
        WaterContainer.OutputItemID   = FName("WaterContainer_Item");
        WaterContainer.OutputQuantity = 1;
        WaterContainer.CraftTimeSeconds = 6.0f;
        WaterContainer.Description    = FText::FromString("A hollowed stone vessel sealed with leaf pulp. Holds enough water for one day of travel.");

        FQuest_CraftingIngredient Stone;
        Stone.ResourceType = EQuest_ResourceType::Stone;
        Stone.Quantity     = 1;

        FQuest_CraftingIngredient Leaf;
        Leaf.ResourceType = EQuest_ResourceType::Leaf;
        Leaf.Quantity     = 1;

        WaterContainer.Ingredients.Add(Stone);
        WaterContainer.Ingredients.Add(Leaf);
        RecipeMap.Add(WaterContainer.RecipeID, WaterContainer);
    }

    // ── Recipe 4: Bone Spear ─────────────────────────────────────────────
    {
        FQuest_CraftingRecipe BoneSpear;
        BoneSpear.RecipeID       = FName("Recipe_BoneSpear");
        BoneSpear.DisplayName    = FText::FromString("Bone Spear");
        BoneSpear.OutputItemID   = FName("BoneSpear_Item");
        BoneSpear.OutputQuantity = 1;
        BoneSpear.CraftTimeSeconds = 10.0f;
        BoneSpear.Description    = FText::FromString("A sharpened bone lashed to a long stick. Effective against small predators from a safe distance.");

        FQuest_CraftingIngredient Bone;
        Bone.ResourceType = EQuest_ResourceType::Bone;
        Bone.Quantity     = 1;

        FQuest_CraftingIngredient Stick;
        Stick.ResourceType = EQuest_ResourceType::Stick;
        Stick.Quantity     = 2;

        FQuest_CraftingIngredient Vine;
        Vine.ResourceType = EQuest_ResourceType::Vine;
        Vine.Quantity     = 1;

        BoneSpear.Ingredients.Add(Bone);
        BoneSpear.Ingredients.Add(Stick);
        BoneSpear.Ingredients.Add(Vine);
        RecipeMap.Add(BoneSpear.RecipeID, BoneSpear);
    }

    UE_LOG(LogTemp, Log, TEXT("[CraftingSystem] %d default recipes loaded"), RecipeMap.Num());
}

bool UCraftingSystem::AddRecipe(const FQuest_CraftingRecipe& Recipe)
{
    if (Recipe.RecipeID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("[CraftingSystem] AddRecipe: RecipeID is None — skipped"));
        return false;
    }
    RecipeMap.Add(Recipe.RecipeID, Recipe);
    return true;
}

bool UCraftingSystem::GetRecipe(FName RecipeID, FQuest_CraftingRecipe& OutRecipe) const
{
    const FQuest_CraftingRecipe* Found = RecipeMap.Find(RecipeID);
    if (Found)
    {
        OutRecipe = *Found;
        return true;
    }
    return false;
}

TArray<FQuest_CraftingRecipe> UCraftingSystem::GetAllRecipes() const
{
    TArray<FQuest_CraftingRecipe> Out;
    RecipeMap.GenerateValueArray(Out);
    return Out;
}

TArray<FQuest_CraftingRecipe> UCraftingSystem::GetAvailableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Out;
    for (const auto& Pair : RecipeMap)
    {
        if (HasIngredients(Pair.Key))
        {
            Out.Add(Pair.Value);
        }
    }
    return Out;
}

// ─── Inventory ────────────────────────────────────────────────────────────────

bool UCraftingSystem::AddResource(EQuest_ResourceType Type, int32 Amount)
{
    if (Type == EQuest_ResourceType::None || Amount <= 0) return false;

    int32 SlotIdx = FindInventorySlot(Type);
    if (SlotIdx != INDEX_NONE)
    {
        Inventory[SlotIdx].Quantity = FMath::Min(Inventory[SlotIdx].Quantity + Amount, MaxStackSize);
        OnResourceChanged.Broadcast(Type, Inventory[SlotIdx].Quantity);
        return true;
    }

    // New slot
    if (Inventory.Num() >= MaxInventorySlots) return false;

    FQuest_InventorySlot NewSlot;
    NewSlot.ResourceType = Type;
    NewSlot.Quantity     = FMath::Min(Amount, MaxStackSize);
    Inventory.Add(NewSlot);
    OnResourceChanged.Broadcast(Type, NewSlot.Quantity);
    return true;
}

bool UCraftingSystem::RemoveResource(EQuest_ResourceType Type, int32 Amount)
{
    if (Type == EQuest_ResourceType::None || Amount <= 0) return false;

    int32 SlotIdx = FindInventorySlot(Type);
    if (SlotIdx == INDEX_NONE) return false;
    if (Inventory[SlotIdx].Quantity < Amount) return false;

    Inventory[SlotIdx].Quantity -= Amount;
    int32 NewCount = Inventory[SlotIdx].Quantity;

    if (NewCount == 0)
    {
        Inventory.RemoveAt(SlotIdx);
    }

    OnResourceChanged.Broadcast(Type, NewCount);
    return true;
}

int32 UCraftingSystem::GetResourceCount(EQuest_ResourceType Type) const
{
    int32 SlotIdx = FindInventorySlot(Type);
    if (SlotIdx == INDEX_NONE) return 0;
    return Inventory[SlotIdx].Quantity;
}

TArray<FQuest_InventorySlot> UCraftingSystem::GetInventory() const
{
    return Inventory;
}

bool UCraftingSystem::HasIngredients(FName RecipeID) const
{
    const FQuest_CraftingRecipe* Recipe = RecipeMap.Find(RecipeID);
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

// ─── Crafting ─────────────────────────────────────────────────────────────────

FQuest_CraftResultData UCraftingSystem::TryCraft(FName RecipeID)
{
    FQuest_CraftResultData ResultData;

    const FQuest_CraftingRecipe* Recipe = RecipeMap.Find(RecipeID);
    if (!Recipe)
    {
        ResultData.Result        = EQuest_CraftResult::RecipeNotFound;
        ResultData.ResultMessage = FText::FromString("Recipe not found.");
        OnCraftFailed.Broadcast(EQuest_CraftResult::RecipeNotFound);
        return ResultData;
    }

    if (!HasIngredients(RecipeID))
    {
        ResultData.Result        = EQuest_CraftResult::InsufficientItems;
        ResultData.ResultMessage = FText::FromString("Not enough materials.");
        OnCraftFailed.Broadcast(EQuest_CraftResult::InsufficientItems);
        return ResultData;
    }

    if (!ConsumeIngredients(*Recipe))
    {
        ResultData.Result        = EQuest_CraftResult::InsufficientItems;
        ResultData.ResultMessage = FText::FromString("Failed to consume ingredients.");
        OnCraftFailed.Broadcast(EQuest_CraftResult::InsufficientItems);
        return ResultData;
    }

    ResultData.Result         = EQuest_CraftResult::Success;
    ResultData.OutputItemID   = Recipe->OutputItemID;
    ResultData.OutputQuantity = Recipe->OutputQuantity;
    ResultData.ResultMessage  = FText::Format(
        FText::FromString("Crafted {0} x{1}"),
        Recipe->DisplayName,
        FText::AsNumber(Recipe->OutputQuantity)
    );

    OnCraftSuccess.Broadcast(ResultData);
    UE_LOG(LogTemp, Log, TEXT("[CraftingSystem] Crafted: %s"), *Recipe->OutputItemID.ToString());
    return ResultData;
}

void UCraftingSystem::OpenCraftingMenu()
{
    if (!bCraftingMenuOpen)
    {
        bCraftingMenuOpen = true;
        OnCraftingMenuToggled.Broadcast(true);
        UE_LOG(LogTemp, Log, TEXT("[CraftingSystem] Crafting menu opened"));
    }
}

void UCraftingSystem::CloseCraftingMenu()
{
    if (bCraftingMenuOpen)
    {
        bCraftingMenuOpen = false;
        OnCraftingMenuToggled.Broadcast(false);
        UE_LOG(LogTemp, Log, TEXT("[CraftingSystem] Crafting menu closed"));
    }
}

// ─── Private Helpers ──────────────────────────────────────────────────────────

int32 UCraftingSystem::FindInventorySlot(EQuest_ResourceType Type) const
{
    for (int32 i = 0; i < Inventory.Num(); ++i)
    {
        if (Inventory[i].ResourceType == Type)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

bool UCraftingSystem::ConsumeIngredients(const FQuest_CraftingRecipe& Recipe)
{
    // Verify first (atomic check before any removal)
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
        RemoveResource(Ingredient.ResourceType, Ingredient.Quantity);
    }
    return true;
}
