// CraftingSystem.cpp
// Agent #14 — Quest & Mission Designer
// Implements: crafting recipes, resource pickup, inventory management
// Recipes: Stone Axe (2 Rock + 1 Flint), Campfire (3 Stick), Water Container (1 Rock + 1 Leaf)

#include "CraftingSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UCraftingSystem::UCraftingSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeRecipes();
}

// ============================================================
// InitializeRecipes — define the 5 base crafting recipes
// ============================================================
void UCraftingSystem::InitializeRecipes()
{
    AvailableRecipes.Empty();

    // --- Recipe 1: Stone Axe ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItem::StoneAxe;
        Recipe.DisplayName = TEXT("Stone Axe");
        Recipe.CraftingTimeSec = 3.0f;
        Recipe.bRequiresCraftingStation = false;

        FQuest_ResourceIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Rock;
        Ing1.Quantity = 2;

        FQuest_ResourceIngredient Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Flint;
        Ing2.Quantity = 1;

        Recipe.Ingredients.Add(Ing1);
        Recipe.Ingredients.Add(Ing2);
        AvailableRecipes.Add(Recipe);
    }

    // --- Recipe 2: Campfire ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItem::Campfire;
        Recipe.DisplayName = TEXT("Campfire");
        Recipe.CraftingTimeSec = 2.0f;
        Recipe.bRequiresCraftingStation = false;

        FQuest_ResourceIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Stick;
        Ing1.Quantity = 3;

        Recipe.Ingredients.Add(Ing1);
        AvailableRecipes.Add(Recipe);
    }

    // --- Recipe 3: Water Container ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItem::WaterContainer;
        Recipe.DisplayName = TEXT("Water Container");
        Recipe.CraftingTimeSec = 4.0f;
        Recipe.bRequiresCraftingStation = false;

        FQuest_ResourceIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Rock;
        Ing1.Quantity = 1;

        FQuest_ResourceIngredient Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Leaf;
        Ing2.Quantity = 1;

        Recipe.Ingredients.Add(Ing1);
        Recipe.Ingredients.Add(Ing2);
        AvailableRecipes.Add(Recipe);
    }

    // --- Recipe 4: Spear ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItem::Spear;
        Recipe.DisplayName = TEXT("Spear");
        Recipe.CraftingTimeSec = 5.0f;
        Recipe.bRequiresCraftingStation = false;

        FQuest_ResourceIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Stick;
        Ing1.Quantity = 2;

        FQuest_ResourceIngredient Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Flint;
        Ing2.Quantity = 1;

        Recipe.Ingredients.Add(Ing1);
        Recipe.Ingredients.Add(Ing2);
        AvailableRecipes.Add(Recipe);
    }

    // --- Recipe 5: Leather Wrap ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItem::LeatherWrap;
        Recipe.DisplayName = TEXT("Leather Wrap");
        Recipe.CraftingTimeSec = 6.0f;
        Recipe.bRequiresCraftingStation = true;

        FQuest_ResourceIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Hide;
        Ing1.Quantity = 2;

        FQuest_ResourceIngredient Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Bone;
        Ing2.Quantity = 1;

        Recipe.Ingredients.Add(Ing1);
        Recipe.Ingredients.Add(Ing2);
        AvailableRecipes.Add(Recipe);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Initialized %d recipes"), AvailableRecipes.Num());
}

// ============================================================
// ToggleCraftingMenu
// ============================================================
void UCraftingSystem::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Menu %s"), bCraftingMenuOpen ? TEXT("OPEN") : TEXT("CLOSED"));
}

// ============================================================
// PickupResource — add resource to inventory
// ============================================================
void UCraftingSystem::PickupResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (Amount <= 0) return;

    switch (ResourceType)
    {
        case EQuest_ResourceType::Rock:   Inventory.Rocks   += Amount; break;
        case EQuest_ResourceType::Stick:  Inventory.Sticks  += Amount; break;
        case EQuest_ResourceType::Leaf:   Inventory.Leaves  += Amount; break;
        case EQuest_ResourceType::Bone:   Inventory.Bones   += Amount; break;
        case EQuest_ResourceType::Hide:   Inventory.Hides   += Amount; break;
        case EQuest_ResourceType::Flint:  Inventory.Flint   += Amount; break;
        default: break;
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Picked up %d x %d"), (int32)ResourceType, Amount);
}

// ============================================================
// GetResourceCount
// ============================================================
int32 UCraftingSystem::GetResourceCount(EQuest_ResourceType ResourceType) const
{
    switch (ResourceType)
    {
        case EQuest_ResourceType::Rock:   return Inventory.Rocks;
        case EQuest_ResourceType::Stick:  return Inventory.Sticks;
        case EQuest_ResourceType::Leaf:   return Inventory.Leaves;
        case EQuest_ResourceType::Bone:   return Inventory.Bones;
        case EQuest_ResourceType::Hide:   return Inventory.Hides;
        case EQuest_ResourceType::Flint:  return Inventory.Flint;
        default: return 0;
    }
}

// ============================================================
// FindRecipe — internal lookup
// ============================================================
const FQuest_CraftingRecipe* UCraftingSystem::FindRecipe(EQuest_CraftedItem Item) const
{
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.ResultItem == Item)
        {
            return &Recipe;
        }
    }
    return nullptr;
}

// ============================================================
// CanCraft — check if player has all ingredients
// ============================================================
bool UCraftingSystem::CanCraft(EQuest_CraftedItem Item) const
{
    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (!Recipe) return false;

    if (Recipe->bRequiresCraftingStation && !bNearCraftingStation)
    {
        return false;
    }

    for (const FQuest_ResourceIngredient& Ing : Recipe->Ingredients)
    {
        if (GetResourceCount(Ing.ResourceType) < Ing.Quantity)
        {
            return false;
        }
    }
    return true;
}

// ============================================================
// ConsumeIngredients — deduct resources after crafting
// ============================================================
bool UCraftingSystem::ConsumeIngredients(const FQuest_CraftingRecipe& Recipe)
{
    // Verify first
    for (const FQuest_ResourceIngredient& Ing : Recipe.Ingredients)
    {
        if (GetResourceCount(Ing.ResourceType) < Ing.Quantity)
        {
            return false;
        }
    }

    // Deduct
    for (const FQuest_ResourceIngredient& Ing : Recipe.Ingredients)
    {
        switch (Ing.ResourceType)
        {
            case EQuest_ResourceType::Rock:   Inventory.Rocks   -= Ing.Quantity; break;
            case EQuest_ResourceType::Stick:  Inventory.Sticks  -= Ing.Quantity; break;
            case EQuest_ResourceType::Leaf:   Inventory.Leaves  -= Ing.Quantity; break;
            case EQuest_ResourceType::Bone:   Inventory.Bones   -= Ing.Quantity; break;
            case EQuest_ResourceType::Hide:   Inventory.Hides   -= Ing.Quantity; break;
            case EQuest_ResourceType::Flint:  Inventory.Flint   -= Ing.Quantity; break;
            default: break;
        }
    }
    return true;
}

// ============================================================
// CraftItem — attempt to craft, returns success
// ============================================================
bool UCraftingSystem::CraftItem(EQuest_CraftedItem Item)
{
    if (!CanCraft(Item))
    {
        UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Cannot craft item %d — missing ingredients or station"), (int32)Item);
        return false;
    }

    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (!Recipe) return false;

    if (!ConsumeIngredients(*Recipe))
    {
        return false;
    }

    Inventory.CraftedItems.Add(Item);
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Crafted '%s' successfully"), *Recipe->DisplayName);
    return true;
}

// ============================================================
// GetCraftableRecipes — returns only recipes player can make now
// ============================================================
TArray<FQuest_CraftingRecipe> UCraftingSystem::GetCraftableRecipes() const
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
// CheckProximityToCraftingStation — call each tick or on overlap
// ============================================================
void UCraftingSystem::CheckProximityToCraftingStation()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = Owner->GetWorld();
    if (!World) return;

    FVector PlayerLoc = Owner->GetActorLocation();

    // Find all actors labelled CraftingStation_*
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    bNearCraftingStation = false;
    for (AActor* A : AllActors)
    {
        if (A && A->GetActorLabel().StartsWith(TEXT("CraftingStation")))
        {
            float Dist = FVector::Dist(PlayerLoc, A->GetActorLocation());
            if (Dist <= CraftingStationRadius)
            {
                bNearCraftingStation = true;
                break;
            }
        }
    }
}
