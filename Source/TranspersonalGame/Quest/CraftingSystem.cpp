// ============================================================
// CraftingSystem.cpp — Agent #14 Quest & Mission Designer
// Prehistoric survival crafting implementation
// Recipes: Stone Axe (2 Rock + 1 Stick), Campfire (3 Stick),
//          Water Container (1 Rock + 1 Leaf), Spear (2 Stick + 1 Flint)
// ============================================================

#include "CraftingSystem.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogCraftingSystem, Log, All);

// ============================================================
// Constructor
// ============================================================
ACraftingSystem::ACraftingSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    bCraftingMenuOpen = false;
    bIsCrafting = false;
    CurrentlyCraftingItem = EQuest_CraftedItem::None;
    CraftingProgress = 0.0f;
    CraftingTimer = 0.0f;
    CraftingDuration = 0.0f;
}

// ============================================================
// BeginPlay — Initialize default recipes
// ============================================================
void ACraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultRecipes();
    UE_LOG(LogCraftingSystem, Log, TEXT("CraftingSystem initialized with %d recipes"), AvailableRecipes.Num());
}

// ============================================================
// InitializeDefaultRecipes — Define the 4 core prehistoric recipes
// ============================================================
void ACraftingSystem::InitializeDefaultRecipes()
{
    AvailableRecipes.Empty();

    // --- Recipe 1: Stone Axe (2 Rock + 1 Stick) ---
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.ResultItem = EQuest_CraftedItem::StoneAxe;
        StoneAxe.RecipeName = TEXT("Stone Axe");
        StoneAxe.Description = TEXT("A crude but effective cutting tool. Useful for chopping wood and defending against small predators.");
        StoneAxe.CraftingTimeSeconds = 5.0f;
        StoneAxe.bRequiresCampfire = false;

        FQuest_ResourceStack RockReq;
        RockReq.ResourceType = EQuest_ResourceType::Rock;
        RockReq.Quantity = 2;

        FQuest_ResourceStack StickReq;
        StickReq.ResourceType = EQuest_ResourceType::Stick;
        StickReq.Quantity = 1;

        StoneAxe.RequiredResources.Add(RockReq);
        StoneAxe.RequiredResources.Add(StickReq);
        AvailableRecipes.Add(StoneAxe);
    }

    // --- Recipe 2: Campfire (3 Stick) ---
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.ResultItem = EQuest_CraftedItem::Campfire;
        Campfire.RecipeName = TEXT("Campfire");
        Campfire.Description = TEXT("A basic fire that provides warmth, light, and the ability to cook meat. Keeps predators at bay at night.");
        Campfire.CraftingTimeSeconds = 8.0f;
        Campfire.bRequiresCampfire = false;

        FQuest_ResourceStack StickReq;
        StickReq.ResourceType = EQuest_ResourceType::Stick;
        StickReq.Quantity = 3;

        Campfire.RequiredResources.Add(StickReq);
        AvailableRecipes.Add(Campfire);
    }

    // --- Recipe 3: Water Container (1 Rock + 1 Leaf) ---
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.ResultItem = EQuest_CraftedItem::WaterContainer;
        WaterContainer.RecipeName = TEXT("Water Container");
        WaterContainer.Description = TEXT("A hollowed rock lined with large leaves. Holds enough water for one day of travel.");
        WaterContainer.CraftingTimeSeconds = 6.0f;
        WaterContainer.bRequiresCampfire = false;

        FQuest_ResourceStack RockReq;
        RockReq.ResourceType = EQuest_ResourceType::Rock;
        RockReq.Quantity = 1;

        FQuest_ResourceStack LeafReq;
        LeafReq.ResourceType = EQuest_ResourceType::Leaf;
        LeafReq.Quantity = 1;

        WaterContainer.RequiredResources.Add(RockReq);
        WaterContainer.RequiredResources.Add(LeafReq);
        AvailableRecipes.Add(WaterContainer);
    }

    // --- Recipe 4: Spear (2 Stick + 1 Flint) ---
    {
        FQuest_CraftingRecipe Spear;
        Spear.ResultItem = EQuest_CraftedItem::Spear;
        Spear.RecipeName = TEXT("Flint Spear");
        Spear.Description = TEXT("A sharpened flint tip bound to a long stick. Essential for hunting and defending against raptors.");
        Spear.CraftingTimeSeconds = 10.0f;
        Spear.bRequiresCampfire = false;

        FQuest_ResourceStack StickReq;
        StickReq.ResourceType = EQuest_ResourceType::Stick;
        StickReq.Quantity = 2;

        FQuest_ResourceStack FlintReq;
        FlintReq.ResourceType = EQuest_ResourceType::Flint;
        FlintReq.Quantity = 1;

        Spear.RequiredResources.Add(StickReq);
        Spear.RequiredResources.Add(FlintReq);
        AvailableRecipes.Add(Spear);
    }

    UE_LOG(LogCraftingSystem, Log, TEXT("Initialized %d crafting recipes"), AvailableRecipes.Num());
}

// ============================================================
// ToggleCraftingMenu — Press C to open/close
// ============================================================
void ACraftingSystem::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    UE_LOG(LogCraftingSystem, Log, TEXT("Crafting menu %s"), bCraftingMenuOpen ? TEXT("OPENED") : TEXT("CLOSED"));
}

// ============================================================
// CanCraft — Check if player has required resources
// ============================================================
bool ACraftingSystem::CanCraft(EQuest_CraftedItem Item) const
{
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.ResultItem != Item)
        {
            continue;
        }

        for (const FQuest_ResourceStack& Required : Recipe.RequiredResources)
        {
            int32 HaveAmount = GetResourceCount(Required.ResourceType);
            if (HaveAmount < Required.Quantity)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

// ============================================================
// StartCrafting — Begin crafting process
// ============================================================
bool ACraftingSystem::StartCrafting(EQuest_CraftedItem Item)
{
    if (bIsCrafting)
    {
        UE_LOG(LogCraftingSystem, Warning, TEXT("Already crafting %d — cannot start new craft"), (int32)CurrentlyCraftingItem);
        return false;
    }

    if (!CanCraft(Item))
    {
        UE_LOG(LogCraftingSystem, Warning, TEXT("Cannot craft item %d — insufficient resources"), (int32)Item);
        return false;
    }

    // Find the recipe
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.ResultItem == Item)
        {
            ConsumeResources(Recipe);
            bIsCrafting = true;
            CurrentlyCraftingItem = Item;
            CraftingProgress = 0.0f;
            CraftingTimer = 0.0f;
            CraftingDuration = Recipe.CraftingTimeSeconds;
            UE_LOG(LogCraftingSystem, Log, TEXT("Started crafting: %s (%.1fs)"), *Recipe.RecipeName, Recipe.CraftingTimeSeconds);
            return true;
        }
    }

    return false;
}

// ============================================================
// AddResource — Add resource to player inventory
// ============================================================
void ACraftingSystem::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    for (FQuest_ResourceStack& Stack : PlayerInventory)
    {
        if (Stack.ResourceType == ResourceType)
        {
            Stack.Quantity += Amount;
            UE_LOG(LogCraftingSystem, Log, TEXT("Added %d x %d (total: %d)"), Amount, (int32)ResourceType, Stack.Quantity);
            return;
        }
    }

    // New resource type
    FQuest_ResourceStack NewStack;
    NewStack.ResourceType = ResourceType;
    NewStack.Quantity = Amount;
    PlayerInventory.Add(NewStack);
    UE_LOG(LogCraftingSystem, Log, TEXT("New resource type %d added: %d units"), (int32)ResourceType, Amount);
}

// ============================================================
// GetResourceCount — Query inventory
// ============================================================
int32 ACraftingSystem::GetResourceCount(EQuest_ResourceType ResourceType) const
{
    for (const FQuest_ResourceStack& Stack : PlayerInventory)
    {
        if (Stack.ResourceType == ResourceType)
        {
            return Stack.Quantity;
        }
    }
    return 0;
}

// ============================================================
// GetCraftableRecipes — Return only recipes player can currently craft
// ============================================================
TArray<FQuest_CraftingRecipe> ACraftingSystem::GetCraftableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Craftable;
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (CanCraft(Recipe.ResultItem))
        {
            Craftable.Add(Recipe);
        }
    }
    return Craftable;
}

// ============================================================
// Tick — Update crafting progress
// ============================================================
void ACraftingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsCrafting || CraftingDuration <= 0.0f)
    {
        return;
    }

    CraftingTimer += DeltaTime;
    CraftingProgress = FMath::Clamp(CraftingTimer / CraftingDuration, 0.0f, 1.0f);

    if (CraftingTimer >= CraftingDuration)
    {
        CompleteCrafting(CurrentlyCraftingItem);
    }
}

// ============================================================
// ConsumeResources — Remove required resources from inventory
// ============================================================
void ACraftingSystem::ConsumeResources(const FQuest_CraftingRecipe& Recipe)
{
    for (const FQuest_ResourceStack& Required : Recipe.RequiredResources)
    {
        for (FQuest_ResourceStack& InvStack : PlayerInventory)
        {
            if (InvStack.ResourceType == Required.ResourceType)
            {
                InvStack.Quantity -= Required.Quantity;
                if (InvStack.Quantity < 0)
                {
                    InvStack.Quantity = 0;
                }
                break;
            }
        }
    }
}

// ============================================================
// CompleteCrafting — Finish crafting, reset state
// ============================================================
void ACraftingSystem::CompleteCrafting(EQuest_CraftedItem Item)
{
    bIsCrafting = false;
    CraftingProgress = 1.0f;
    CraftingTimer = 0.0f;

    UE_LOG(LogCraftingSystem, Log, TEXT("CRAFTING COMPLETE: Item %d crafted successfully!"), (int32)Item);

    CurrentlyCraftingItem = EQuest_CraftedItem::None;
    CraftingProgress = 0.0f;
}
