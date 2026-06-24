#include "CraftingSystem.h"
#include "Engine/Engine.h"

// -------------------------------------------------------
// CraftingSystem.cpp — Agent #14 Quest & Mission Designer
// Full implementation of 3 core crafting recipes
// -------------------------------------------------------

UCraftingSystem::UCraftingSystem()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Initialize inventory with zero counts for all resource types
    PlayerInventory.Add(EQuest_ResourceType::Rock,  0);
    PlayerInventory.Add(EQuest_ResourceType::Stick, 0);
    PlayerInventory.Add(EQuest_ResourceType::Leaf,  0);
    PlayerInventory.Add(EQuest_ResourceType::Bone,  0);
    PlayerInventory.Add(EQuest_ResourceType::Hide,  0);
}

void UCraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeRecipes();
}

void UCraftingSystem::InitializeRecipes()
{
    Recipes.Empty();

    // -------------------------------------------------------
    // Recipe 1: Stone Axe — 2 Rocks + 1 Stick
    // Primary melee weapon for early game
    // -------------------------------------------------------
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.OutputItem       = EQuest_CraftedItem::StoneAxe;
        StoneAxe.DisplayName      = TEXT("Stone Axe");
        StoneAxe.CraftingTimeSeconds = 3.0f;

        FQuest_ResourceStack RockReq;
        RockReq.ResourceType = EQuest_ResourceType::Rock;
        RockReq.Quantity     = 2;
        StoneAxe.RequiredIngredients.Add(RockReq);

        FQuest_ResourceStack StickReq;
        StickReq.ResourceType = EQuest_ResourceType::Stick;
        StickReq.Quantity     = 1;
        StoneAxe.RequiredIngredients.Add(StickReq);

        Recipes.Add(StoneAxe);
    }

    // -------------------------------------------------------
    // Recipe 2: Campfire — 3 Sticks
    // Provides warmth, cooking, and deters predators at night
    // -------------------------------------------------------
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.OutputItem       = EQuest_CraftedItem::Campfire;
        Campfire.DisplayName      = TEXT("Campfire");
        Campfire.CraftingTimeSeconds = 2.0f;

        FQuest_ResourceStack StickReq;
        StickReq.ResourceType = EQuest_ResourceType::Stick;
        StickReq.Quantity     = 3;
        Campfire.RequiredIngredients.Add(StickReq);

        Recipes.Add(Campfire);
    }

    // -------------------------------------------------------
    // Recipe 3: Water Container — 1 Rock + 1 Leaf
    // Stores water to manage thirst survival stat
    // -------------------------------------------------------
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.OutputItem       = EQuest_CraftedItem::WaterContainer;
        WaterContainer.DisplayName      = TEXT("Water Container");
        WaterContainer.CraftingTimeSeconds = 2.5f;

        FQuest_ResourceStack RockReq;
        RockReq.ResourceType = EQuest_ResourceType::Rock;
        RockReq.Quantity     = 1;
        WaterContainer.RequiredIngredients.Add(RockReq);

        FQuest_ResourceStack LeafReq;
        LeafReq.ResourceType = EQuest_ResourceType::Leaf;
        LeafReq.Quantity     = 1;
        WaterContainer.RequiredIngredients.Add(LeafReq);

        Recipes.Add(WaterContainer);
    }

    // -------------------------------------------------------
    // Recipe 4: Spear Head — 2 Rocks + 1 Bone
    // Ranged weapon component — attach to stick for full spear
    // -------------------------------------------------------
    {
        FQuest_CraftingRecipe SpearHead;
        SpearHead.OutputItem       = EQuest_CraftedItem::SpearHead;
        SpearHead.DisplayName      = TEXT("Spear Head");
        SpearHead.CraftingTimeSeconds = 4.0f;

        FQuest_ResourceStack RockReq;
        RockReq.ResourceType = EQuest_ResourceType::Rock;
        RockReq.Quantity     = 2;
        SpearHead.RequiredIngredients.Add(RockReq);

        FQuest_ResourceStack BoneReq;
        BoneReq.ResourceType = EQuest_ResourceType::Bone;
        BoneReq.Quantity     = 1;
        SpearHead.RequiredIngredients.Add(BoneReq);

        Recipes.Add(SpearHead);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Initialized %d recipes"), Recipes.Num());
}

void UCraftingSystem::PickupResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (Amount <= 0) return;

    int32& CurrentCount = PlayerInventory.FindOrAdd(ResourceType);
    CurrentCount += Amount;

    int32 NewTotal = CurrentCount;
    OnResourcePickedUp.Broadcast(ResourceType, NewTotal);

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Picked up %d x ResourceType=%d. New total: %d"),
        Amount, (int32)ResourceType, NewTotal);
}

bool UCraftingSystem::TryCraft(EQuest_CraftedItem Item)
{
    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (!Recipe)
    {
        UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: No recipe found for item %d"), (int32)Item);
        return false;
    }

    if (!CanCraft(Item))
    {
        UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Not enough ingredients to craft %s"), *Recipe->DisplayName);
        return false;
    }

    ConsumeIngredients(*Recipe);
    OnItemCrafted.Broadcast(Item);

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Successfully crafted %s"), *Recipe->DisplayName);
    return true;
}

void UCraftingSystem::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    OnCraftingMenuToggled.Broadcast(bCraftingMenuOpen);

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Menu %s"), bCraftingMenuOpen ? TEXT("OPENED") : TEXT("CLOSED"));
}

bool UCraftingSystem::CanCraft(EQuest_CraftedItem Item) const
{
    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (!Recipe) return false;

    for (const FQuest_ResourceStack& Ingredient : Recipe->RequiredIngredients)
    {
        int32 Available = GetResourceCount(Ingredient.ResourceType);
        if (Available < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

int32 UCraftingSystem::GetResourceCount(EQuest_ResourceType ResourceType) const
{
    const int32* Count = PlayerInventory.Find(ResourceType);
    return Count ? *Count : 0;
}

void UCraftingSystem::LogInventoryState() const
{
    UE_LOG(LogTemp, Log, TEXT("=== CraftingSystem Inventory ==="));
    UE_LOG(LogTemp, Log, TEXT("  Rocks:  %d"), GetResourceCount(EQuest_ResourceType::Rock));
    UE_LOG(LogTemp, Log, TEXT("  Sticks: %d"), GetResourceCount(EQuest_ResourceType::Stick));
    UE_LOG(LogTemp, Log, TEXT("  Leaves: %d"), GetResourceCount(EQuest_ResourceType::Leaf));
    UE_LOG(LogTemp, Log, TEXT("  Bones:  %d"), GetResourceCount(EQuest_ResourceType::Bone));
    UE_LOG(LogTemp, Log, TEXT("  Hides:  %d"), GetResourceCount(EQuest_ResourceType::Hide));
    UE_LOG(LogTemp, Log, TEXT("=== Recipes Available ==="));
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        bool bCanMake = CanCraft(Recipe.OutputItem);
        UE_LOG(LogTemp, Log, TEXT("  %s — %s"), *Recipe.DisplayName, bCanMake ? TEXT("CRAFTABLE") : TEXT("missing ingredients"));
    }
}

// -------------------------------------------------------
// Private helpers
// -------------------------------------------------------

const FQuest_CraftingRecipe* UCraftingSystem::FindRecipe(EQuest_CraftedItem Item) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.OutputItem == Item)
        {
            return &Recipe;
        }
    }
    return nullptr;
}

void UCraftingSystem::ConsumeIngredients(const FQuest_CraftingRecipe& Recipe)
{
    for (const FQuest_ResourceStack& Ingredient : Recipe.RequiredIngredients)
    {
        int32& Count = PlayerInventory.FindOrAdd(Ingredient.ResourceType);
        Count = FMath::Max(0, Count - Ingredient.Quantity);
    }
}
