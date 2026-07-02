// CraftingSystem.cpp
// Agent #14 — Quest & Mission Designer
// Implements: AQuest_ResourcePickup, UCraftingSystemComponent
// Recipes: Stone Axe, Campfire, Water Container, Spear, Leather Wrap

#include "CraftingSystem.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// ─────────────────────────────────────────────────────────────────────────────
// AQuest_ResourcePickup
// ─────────────────────────────────────────────────────────────────────────────

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;
    bPickedUp = false;
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
}

bool AQuest_ResourcePickup::TryPickup(AActor* Collector)
{
    if (bPickedUp || !Collector)
    {
        return false;
    }

    // Find crafting component on collector
    UCraftingSystemComponent* CraftComp =
        Collector->FindComponentByClass<UCraftingSystemComponent>();

    if (!CraftComp)
    {
        return false;
    }

    CraftComp->AddResource(ResourceType, QuantityOnPickup);
    bPickedUp = true;

    // Hide actor visually
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    if (bAutoRespawn && RespawnDelaySec > 0.0f)
    {
        GetWorldTimerManager().SetTimer(
            RespawnTimer,
            this,
            &AQuest_ResourcePickup::HandleRespawn,
            RespawnDelaySec,
            false
        );
    }

    return true;
}

FString AQuest_ResourcePickup::GetResourceLabel() const
{
    switch (ResourceType)
    {
        case EQuest_ResourceType::Rock:   return TEXT("Rock");
        case EQuest_ResourceType::Stick:  return TEXT("Stick");
        case EQuest_ResourceType::Leaf:   return TEXT("Leaf");
        case EQuest_ResourceType::Bone:   return TEXT("Bone");
        case EQuest_ResourceType::Hide:   return TEXT("Hide");
        case EQuest_ResourceType::Flint:  return TEXT("Flint");
        case EQuest_ResourceType::Vine:   return TEXT("Vine");
        case EQuest_ResourceType::Meat:   return TEXT("Meat");
        default:                          return TEXT("Unknown");
    }
}

void AQuest_ResourcePickup::HandleRespawn()
{
    bPickedUp = false;
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
}

// ─────────────────────────────────────────────────────────────────────────────
// UCraftingSystemComponent
// ─────────────────────────────────────────────────────────────────────────────

UCraftingSystemComponent::UCraftingSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bIsCrafting = false;
    bCraftingMenuOpen = false;
    CraftingProgress = 0.0f;
    CraftingTimeRemaining = 0.0f;
}

void UCraftingSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    RegisterDefaultRecipes();
}

void UCraftingSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsCrafting) return;

    CraftingTimeRemaining -= DeltaTime;
    const FQuest_CraftingRecipe Recipe = GetRecipeForItem(PendingItem);
    if (Recipe.CraftingTimeSec > 0.0f)
    {
        CraftingProgress = 1.0f - (CraftingTimeRemaining / Recipe.CraftingTimeSec);
        CraftingProgress = FMath::Clamp(CraftingProgress, 0.0f, 1.0f);
    }

    if (CraftingTimeRemaining <= 0.0f)
    {
        CompleteCrafting();
    }
}

// ── Recipe Registration ──────────────────────────────────────────────────────

void UCraftingSystemComponent::RegisterDefaultRecipes()
{
    AllRecipes.Empty();

    // ── Recipe 1: Stone Axe (2 Rocks + 1 Stick) ──
    {
        FQuest_CraftingRecipe R;
        R.OutputItem = EQuest_CraftedItem::StoneAxe;
        R.DisplayName = TEXT("Stone Axe");
        R.Description = TEXT("A basic chopping tool. Useful for cutting wood and as a weapon against smaller predators.");
        R.CraftingTimeSec = 4.0f;
        R.bRequiresCampfire = false;

        FQuest_ResourceStack Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Rock;
        Ing1.Quantity = 2;
        R.Ingredients.Add(Ing1);

        FQuest_ResourceStack Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Stick;
        Ing2.Quantity = 1;
        R.Ingredients.Add(Ing2);

        AllRecipes.Add(R);
    }

    // ── Recipe 2: Campfire (3 Sticks) ──
    {
        FQuest_CraftingRecipe R;
        R.OutputItem = EQuest_CraftedItem::Campfire;
        R.DisplayName = TEXT("Campfire");
        R.Description = TEXT("Provides warmth, light, and the ability to cook meat. Deters smaller predators at night.");
        R.CraftingTimeSec = 6.0f;
        R.bRequiresCampfire = false;

        FQuest_ResourceStack Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Stick;
        Ing1.Quantity = 3;
        R.Ingredients.Add(Ing1);

        AllRecipes.Add(R);
    }

    // ── Recipe 3: Water Container (1 Rock + 1 Leaf) ──
    {
        FQuest_CraftingRecipe R;
        R.OutputItem = EQuest_CraftedItem::WaterContainer;
        R.DisplayName = TEXT("Water Container");
        R.Description = TEXT("A hollowed rock sealed with leaves. Holds enough water for half a day of travel.");
        R.CraftingTimeSec = 3.0f;
        R.bRequiresCampfire = false;

        FQuest_ResourceStack Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Rock;
        Ing1.Quantity = 1;
        R.Ingredients.Add(Ing1);

        FQuest_ResourceStack Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Leaf;
        Ing2.Quantity = 1;
        R.Ingredients.Add(Ing2);

        AllRecipes.Add(R);
    }

    // ── Recipe 4: Spear (2 Sticks + 1 Flint) ──
    {
        FQuest_CraftingRecipe R;
        R.OutputItem = EQuest_CraftedItem::Spear;
        R.DisplayName = TEXT("Spear");
        R.Description = TEXT("A throwing and thrusting weapon. Effective against raptors and medium-sized prey.");
        R.CraftingTimeSec = 8.0f;
        R.bRequiresCampfire = false;

        FQuest_ResourceStack Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Stick;
        Ing1.Quantity = 2;
        R.Ingredients.Add(Ing1);

        FQuest_ResourceStack Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Flint;
        Ing2.Quantity = 1;
        R.Ingredients.Add(Ing2);

        AllRecipes.Add(R);
    }

    // ── Recipe 5: Leather Wrap (2 Hide + 1 Vine) ──
    {
        FQuest_CraftingRecipe R;
        R.OutputItem = EQuest_CraftedItem::LeatherWrap;
        R.DisplayName = TEXT("Leather Wrap");
        R.Description = TEXT("Basic body protection. Reduces damage from bites and scratches. Requires campfire to cure.");
        R.CraftingTimeSec = 10.0f;
        R.bRequiresCampfire = true;

        FQuest_ResourceStack Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Hide;
        Ing1.Quantity = 2;
        R.Ingredients.Add(Ing1);

        FQuest_ResourceStack Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Vine;
        Ing2.Quantity = 1;
        R.Ingredients.Add(Ing2);

        AllRecipes.Add(R);
    }
}

// ── Inventory ────────────────────────────────────────────────────────────────

void UCraftingSystemComponent::AddResource(EQuest_ResourceType Type, int32 Amount)
{
    if (Type == EQuest_ResourceType::None || Amount <= 0) return;

    for (FQuest_ResourceStack& Stack : Inventory)
    {
        if (Stack.ResourceType == Type)
        {
            Stack.Quantity += Amount;
            return;
        }
    }

    // New stack
    FQuest_ResourceStack NewStack;
    NewStack.ResourceType = Type;
    NewStack.Quantity = Amount;
    Inventory.Add(NewStack);
}

int32 UCraftingSystemComponent::GetResourceCount(EQuest_ResourceType Type) const
{
    for (const FQuest_ResourceStack& Stack : Inventory)
    {
        if (Stack.ResourceType == Type)
        {
            return Stack.Quantity;
        }
    }
    return 0;
}

bool UCraftingSystemComponent::RemoveResource(EQuest_ResourceType Type, int32 Amount)
{
    for (FQuest_ResourceStack& Stack : Inventory)
    {
        if (Stack.ResourceType == Type)
        {
            if (Stack.Quantity < Amount) return false;
            Stack.Quantity -= Amount;
            return true;
        }
    }
    return false;
}

// ── Crafting Logic ───────────────────────────────────────────────────────────

bool UCraftingSystemComponent::CanCraft(EQuest_CraftedItem Item) const
{
    const FQuest_CraftingRecipe Recipe = GetRecipeForItem(Item);
    if (Recipe.OutputItem == EQuest_CraftedItem::None) return false;

    for (const FQuest_ResourceStack& Ingredient : Recipe.Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

bool UCraftingSystemComponent::StartCrafting(EQuest_CraftedItem Item)
{
    if (bIsCrafting) return false;
    if (!CanCraft(Item)) return false;

    const FQuest_CraftingRecipe Recipe = GetRecipeForItem(Item);

    // Consume ingredients
    for (const FQuest_ResourceStack& Ingredient : Recipe.Ingredients)
    {
        RemoveResource(Ingredient.ResourceType, Ingredient.Quantity);
    }

    PendingItem = Item;
    CraftingTimeRemaining = Recipe.CraftingTimeSec;
    CraftingProgress = 0.0f;
    bIsCrafting = true;
    CurrentlyCraftingItem = Item;

    return true;
}

void UCraftingSystemComponent::CancelCrafting()
{
    if (!bIsCrafting) return;

    // Refund ingredients
    const FQuest_CraftingRecipe Recipe = GetRecipeForItem(PendingItem);
    for (const FQuest_ResourceStack& Ingredient : Recipe.Ingredients)
    {
        AddResource(Ingredient.ResourceType, Ingredient.Quantity);
    }

    bIsCrafting = false;
    CraftingProgress = 0.0f;
    CraftingTimeRemaining = 0.0f;
    CurrentlyCraftingItem = EQuest_CraftedItem::None;
    PendingItem = EQuest_CraftedItem::None;
}

void UCraftingSystemComponent::CompleteCrafting()
{
    bIsCrafting = false;
    CraftingProgress = 1.0f;
    CraftingTimeRemaining = 0.0f;
    CurrentlyCraftingItem = EQuest_CraftedItem::None;

    // Item is now in player's possession — notify via log
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Crafted item %d successfully"),
           static_cast<int32>(PendingItem));

    PendingItem = EQuest_CraftedItem::None;
}

TArray<FQuest_CraftingRecipe> UCraftingSystemComponent::GetAvailableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Available;
    for (const FQuest_CraftingRecipe& Recipe : AllRecipes)
    {
        if (CanCraft(Recipe.OutputItem))
        {
            Available.Add(Recipe);
        }
    }
    return Available;
}

FQuest_CraftingRecipe UCraftingSystemComponent::GetRecipeForItem(EQuest_CraftedItem Item) const
{
    for (const FQuest_CraftingRecipe& Recipe : AllRecipes)
    {
        if (Recipe.OutputItem == Item)
        {
            return Recipe;
        }
    }
    return FQuest_CraftingRecipe(); // empty/None
}

void UCraftingSystemComponent::OpenCraftingMenu()
{
    bCraftingMenuOpen = true;
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Menu opened — %d recipes available"),
           AllRecipes.Num());
}

void UCraftingSystemComponent::CloseCraftingMenu()
{
    bCraftingMenuOpen = false;
}
