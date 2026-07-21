// CraftingSystem.cpp
// Agent #14 — Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260702_008
// Implements the crafting system: recipe validation, resource management, item creation

#include "CraftingSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

// ============================================================
// UCraftingSystemComponent — Implementation
// ============================================================

UCraftingSystemComponent::UCraftingSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bCraftingMenuOpen = false;
    CraftingRange = 200.0f;

    // Initialize inventory with zero quantities
    for (int32 i = 0; i < (int32)EQuest_ResourceType::Meat; i++)
    {
        FQuest_ResourceStack Stack;
        Stack.ResourceType = (EQuest_ResourceType)i;
        Stack.Quantity = 0;
        PlayerInventory.Add(Stack);
    }

    // Register all base recipes
    RegisterBaseRecipes();
}

void UCraftingSystemComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCraftingSystemComponent::RegisterBaseRecipes()
{
    AvailableRecipes.Empty();

    // Recipe 1: Stone Axe — 2 Rocks + 1 Stick
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::StoneAxe;
        Recipe.DisplayName = FText::FromString(TEXT("Stone Axe"));
        Recipe.Description = FText::FromString(TEXT("A crude but effective cutting tool. Chop wood faster and deal more damage in combat."));
        Recipe.CraftingTimeSeconds = 5.0f;
        Recipe.bRequiresCampfire = false;

        FQuest_ResourceStack Rock;
        Rock.ResourceType = EQuest_ResourceType::Rock;
        Rock.Quantity = 2;

        FQuest_ResourceStack Stick;
        Stick.ResourceType = EQuest_ResourceType::Stick;
        Stick.Quantity = 1;

        Recipe.RequiredResources.Add(Rock);
        Recipe.RequiredResources.Add(Stick);
        AvailableRecipes.Add(Recipe);
    }

    // Recipe 2: Campfire — 3 Sticks
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::Campfire;
        Recipe.DisplayName = FText::FromString(TEXT("Campfire"));
        Recipe.Description = FText::FromString(TEXT("Provides warmth, light, and the ability to cook meat. Predators avoid fire."));
        Recipe.CraftingTimeSeconds = 8.0f;
        Recipe.bRequiresCampfire = false;

        FQuest_ResourceStack Stick;
        Stick.ResourceType = EQuest_ResourceType::Stick;
        Stick.Quantity = 3;

        Recipe.RequiredResources.Add(Stick);
        AvailableRecipes.Add(Recipe);
    }

    // Recipe 3: Water Container — 1 Rock + 1 Leaf
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::WaterContainer;
        Recipe.DisplayName = FText::FromString(TEXT("Leaf Water Cup"));
        Recipe.Description = FText::FromString(TEXT("A crude vessel made from a large leaf folded around a rock. Holds one drink of water."));
        Recipe.CraftingTimeSeconds = 3.0f;
        Recipe.bRequiresCampfire = false;

        FQuest_ResourceStack Rock;
        Rock.ResourceType = EQuest_ResourceType::Rock;
        Rock.Quantity = 1;

        FQuest_ResourceStack Leaf;
        Leaf.ResourceType = EQuest_ResourceType::Leaf;
        Leaf.Quantity = 1;

        Recipe.RequiredResources.Add(Rock);
        Recipe.RequiredResources.Add(Leaf);
        AvailableRecipes.Add(Recipe);
    }

    // Recipe 4: Spear — 1 Stick + 1 Flint + 1 Vine
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::Spear;
        Recipe.DisplayName = FText::FromString(TEXT("Flint Spear"));
        Recipe.Description = FText::FromString(TEXT("A sharpened flint tip lashed to a long stick. Effective for hunting and keeping predators at distance."));
        Recipe.CraftingTimeSeconds = 10.0f;
        Recipe.bRequiresCampfire = false;

        FQuest_ResourceStack Stick;
        Stick.ResourceType = EQuest_ResourceType::Stick;
        Stick.Quantity = 1;

        FQuest_ResourceStack Flint;
        Flint.ResourceType = EQuest_ResourceType::Flint;
        Flint.Quantity = 1;

        FQuest_ResourceStack Vine;
        Vine.ResourceType = EQuest_ResourceType::Vine;
        Vine.Quantity = 1;

        Recipe.RequiredResources.Add(Stick);
        Recipe.RequiredResources.Add(Flint);
        Recipe.RequiredResources.Add(Vine);
        AvailableRecipes.Add(Recipe);
    }

    // Recipe 5: Leather Wrap — 2 Hide + 1 Vine
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::LeatherWrap;
        Recipe.DisplayName = FText::FromString(TEXT("Hide Wrap"));
        Recipe.Description = FText::FromString(TEXT("Crude protection made from dinosaur hide. Reduces damage from bites and scratches."));
        Recipe.CraftingTimeSeconds = 12.0f;
        Recipe.bRequiresCampfire = true;

        FQuest_ResourceStack Hide;
        Hide.ResourceType = EQuest_ResourceType::Hide;
        Hide.Quantity = 2;

        FQuest_ResourceStack Vine;
        Vine.ResourceType = EQuest_ResourceType::Vine;
        Vine.Quantity = 1;

        Recipe.RequiredResources.Add(Hide);
        Recipe.RequiredResources.Add(Vine);
        AvailableRecipes.Add(Recipe);
    }

    UE_LOG(LogTemp, Log, TEXT("[CraftingSystem] Registered %d recipes."), AvailableRecipes.Num());
}

bool UCraftingSystemComponent::HasRequiredResources(const FQuest_CraftingRecipe& Recipe) const
{
    for (const FQuest_ResourceStack& Required : Recipe.RequiredResources)
    {
        int32 PlayerQty = GetResourceQuantity(Required.ResourceType);
        if (PlayerQty < Required.Quantity)
        {
            return false;
        }
    }
    return true;
}

int32 UCraftingSystemComponent::GetResourceQuantity(EQuest_ResourceType ResourceType) const
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

bool UCraftingSystemComponent::AddResource(EQuest_ResourceType ResourceType, int32 Quantity)
{
    if (Quantity <= 0) return false;

    for (FQuest_ResourceStack& Stack : PlayerInventory)
    {
        if (Stack.ResourceType == ResourceType)
        {
            Stack.Quantity += Quantity;
            UE_LOG(LogTemp, Log, TEXT("[CraftingSystem] Added %d x %d. New total: %d"),
                (int32)ResourceType, Quantity, Stack.Quantity);
            return true;
        }
    }

    // Resource type not yet in inventory — add new stack
    FQuest_ResourceStack NewStack;
    NewStack.ResourceType = ResourceType;
    NewStack.Quantity = Quantity;
    PlayerInventory.Add(NewStack);
    return true;
}

bool UCraftingSystemComponent::ConsumeResources(const FQuest_CraftingRecipe& Recipe)
{
    if (!HasRequiredResources(Recipe)) return false;

    for (const FQuest_ResourceStack& Required : Recipe.RequiredResources)
    {
        for (FQuest_ResourceStack& Stack : PlayerInventory)
        {
            if (Stack.ResourceType == Required.ResourceType)
            {
                Stack.Quantity -= Required.Quantity;
                break;
            }
        }
    }
    return true;
}

EQuest_CraftResult UCraftingSystemComponent::TryCraft(EQuest_CraftedItem ItemToCraft)
{
    // Find the recipe
    const FQuest_CraftingRecipe* FoundRecipe = nullptr;
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.OutputItem == ItemToCraft)
        {
            FoundRecipe = &Recipe;
            break;
        }
    }

    if (!FoundRecipe)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CraftingSystem] No recipe found for item %d"), (int32)ItemToCraft);
        return EQuest_CraftResult::NoRecipe;
    }

    if (!HasRequiredResources(*FoundRecipe))
    {
        UE_LOG(LogTemp, Warning, TEXT("[CraftingSystem] Insufficient resources for %s"),
            *FoundRecipe->DisplayName.ToString());
        return EQuest_CraftResult::InsufficientResources;
    }

    // Consume resources and craft
    if (!ConsumeResources(*FoundRecipe))
    {
        return EQuest_CraftResult::InsufficientResources;
    }

    // Add to crafted items
    CraftedItems.Add(ItemToCraft);

    UE_LOG(LogTemp, Log, TEXT("[CraftingSystem] Successfully crafted: %s"),
        *FoundRecipe->DisplayName.ToString());

    OnItemCrafted.Broadcast(ItemToCraft);
    return EQuest_CraftResult::Success;
}

void UCraftingSystemComponent::OpenCraftingMenu()
{
    if (bCraftingMenuOpen) return;
    bCraftingMenuOpen = true;
    UE_LOG(LogTemp, Log, TEXT("[CraftingSystem] Crafting menu opened."));
    OnCraftingMenuToggled.Broadcast(true);
}

void UCraftingSystemComponent::CloseCraftingMenu()
{
    if (!bCraftingMenuOpen) return;
    bCraftingMenuOpen = false;
    UE_LOG(LogTemp, Log, TEXT("[CraftingSystem] Crafting menu closed."));
    OnCraftingMenuToggled.Broadcast(false);
}

void UCraftingSystemComponent::ToggleCraftingMenu()
{
    if (bCraftingMenuOpen)
        CloseCraftingMenu();
    else
        OpenCraftingMenu();
}

TArray<FQuest_CraftingRecipe> UCraftingSystemComponent::GetCraftableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Craftable;
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (HasRequiredResources(Recipe))
        {
            Craftable.Add(Recipe);
        }
    }
    return Craftable;
}

// ============================================================
// AQuest_ResourcePickup — Implementation
// ============================================================

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;
    ResourceType = EQuest_ResourceType::Rock;
    Quantity = 1;
    bHasBeenCollected = false;
    RespawnTimeSeconds = 120.0f;

    // Root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
}

void AQuest_ResourcePickup::CollectResource(AActor* Collector)
{
    if (bHasBeenCollected) return;
    if (!Collector) return;

    bHasBeenCollected = true;

    // Try to add resource to collector's crafting component
    UCraftingSystemComponent* CraftComp = Collector->FindComponentByClass<UCraftingSystemComponent>();
    if (CraftComp)
    {
        CraftComp->AddResource(ResourceType, Quantity);
        UE_LOG(LogTemp, Log, TEXT("[ResourcePickup] %s collected %d x ResourceType %d"),
            *Collector->GetName(), Quantity, (int32)ResourceType);
    }

    OnResourceCollected.Broadcast(ResourceType, Quantity);

    // Hide actor — respawn after delay
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    if (RespawnTimeSeconds > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            RespawnTimerHandle,
            this,
            &AQuest_ResourcePickup::RespawnResource,
            RespawnTimeSeconds,
            false
        );
    }
}

void AQuest_ResourcePickup::RespawnResource()
{
    bHasBeenCollected = false;
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    UE_LOG(LogTemp, Log, TEXT("[ResourcePickup] Resource respawned at %s"), *GetActorLabel());
}
