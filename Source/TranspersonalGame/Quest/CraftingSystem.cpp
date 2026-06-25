// CraftingSystem.cpp
// Agent #14 — Quest & Mission Designer
// Implements crafting recipes: Stone Axe, Campfire, Water Container, Spear, Shelter
// Resource pickup actors for rocks, sticks, leaves scattered in the world

#include "CraftingSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

// ============================================================
// AQuest_ResourcePickup
// ============================================================

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetupAttachment(RootComponent);
    CollisionSphere->SetSphereRadius(80.0f);
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
}

void AQuest_ResourcePickup::CollectResource(AActor* Collector)
{
    if (bHasBeenCollected || !Collector)
    {
        return;
    }

    bHasBeenCollected = true;

    // Notify the crafting system if present in world
    AQuest_CraftingSystem* CraftingSystem = nullptr;
    if (GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AQuest_CraftingSystem::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            CraftingSystem = Cast<AQuest_CraftingSystem>(FoundActors[0]);
        }
    }

    if (CraftingSystem)
    {
        CraftingSystem->AddResource(ResourceType, Quantity);
    }

    // Hide the actor after collection
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    UE_LOG(LogTemp, Log, TEXT("ResourcePickup: Collected %d x %d"), (int32)ResourceType, Quantity);
}

bool AQuest_ResourcePickup::IsAvailable() const
{
    return !bHasBeenCollected;
}

// ============================================================
// AQuest_CraftingSystem
// ============================================================

AQuest_CraftingSystem::AQuest_CraftingSystem()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AQuest_CraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeRecipes();
}

void AQuest_CraftingSystem::InitializeRecipes()
{
    Recipes.Empty();

    // ---- Recipe 1: Stone Axe (2 Rocks + 1 Stick) ----
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.ResultItem      = EQuest_CraftedItemType::StoneAxe;
        StoneAxe.DisplayName     = TEXT("Stone Axe");
        StoneAxe.Description     = TEXT("A crude but effective chopping tool. Useful for cutting wood and fighting off smaller predators.");
        StoneAxe.CraftTimeSeconds = 3.0f;

        FQuest_ResourceIngredient RockIngredient;
        RockIngredient.ResourceType = EQuest_ResourceType::Rock;
        RockIngredient.Quantity     = 2;
        StoneAxe.Ingredients.Add(RockIngredient);

        FQuest_ResourceIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity     = 1;
        StoneAxe.Ingredients.Add(StickIngredient);

        Recipes.Add(StoneAxe);
    }

    // ---- Recipe 2: Campfire (3 Sticks) ----
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.ResultItem       = EQuest_CraftedItemType::Campfire;
        Campfire.DisplayName      = TEXT("Campfire");
        Campfire.Description      = TEXT("A basic fire for warmth, cooking food, and keeping predators at bay during the night.");
        Campfire.CraftTimeSeconds = 4.0f;

        FQuest_ResourceIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity     = 3;
        Campfire.Ingredients.Add(StickIngredient);

        Recipes.Add(Campfire);
    }

    // ---- Recipe 3: Water Container (1 Rock + 1 Leaf) ----
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.ResultItem       = EQuest_CraftedItemType::WaterContainer;
        WaterContainer.DisplayName      = TEXT("Water Container");
        WaterContainer.Description      = TEXT("A hollowed rock lined with large leaves. Holds enough water to survive a day in the field.");
        WaterContainer.CraftTimeSeconds = 2.5f;

        FQuest_ResourceIngredient RockIngredient;
        RockIngredient.ResourceType = EQuest_ResourceType::Rock;
        RockIngredient.Quantity     = 1;
        WaterContainer.Ingredients.Add(RockIngredient);

        FQuest_ResourceIngredient LeafIngredient;
        LeafIngredient.ResourceType = EQuest_ResourceType::Leaf;
        LeafIngredient.Quantity     = 1;
        WaterContainer.Ingredients.Add(LeafIngredient);

        Recipes.Add(WaterContainer);
    }

    // ---- Recipe 4: Spear (2 Sticks + 1 Flint) ----
    {
        FQuest_CraftingRecipe Spear;
        Spear.ResultItem       = EQuest_CraftedItemType::Spear;
        Spear.DisplayName      = TEXT("Spear");
        Spear.Description      = TEXT("A sharpened flint tip lashed to a long stick. Effective for hunting and keeping dinosaurs at distance.");
        Spear.CraftTimeSeconds = 5.0f;

        FQuest_ResourceIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity     = 2;
        Spear.Ingredients.Add(StickIngredient);

        FQuest_ResourceIngredient FlintIngredient;
        FlintIngredient.ResourceType = EQuest_ResourceType::Flint;
        FlintIngredient.Quantity     = 1;
        Spear.Ingredients.Add(FlintIngredient);

        Recipes.Add(Spear);
    }

    // ---- Recipe 5: Shelter (5 Sticks + 3 Leaves) ----
    {
        FQuest_CraftingRecipe Shelter;
        Shelter.ResultItem       = EQuest_CraftedItemType::Shelter;
        Shelter.DisplayName      = TEXT("Lean-To Shelter");
        Shelter.Description      = TEXT("A basic lean-to shelter made from branches and leaves. Provides protection from rain and reduces predator detection.");
        Shelter.CraftTimeSeconds = 8.0f;

        FQuest_ResourceIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity     = 5;
        Shelter.Ingredients.Add(StickIngredient);

        FQuest_ResourceIngredient LeafIngredient;
        LeafIngredient.ResourceType = EQuest_ResourceType::Leaf;
        LeafIngredient.Quantity     = 3;
        Shelter.Ingredients.Add(LeafIngredient);

        Recipes.Add(Shelter);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Initialized %d recipes"), Recipes.Num());
}

bool AQuest_CraftingSystem::CanCraft(EQuest_CraftedItemType ItemType) const
{
    const FQuest_CraftingRecipe* Recipe = FindRecipe(ItemType);
    if (!Recipe)
    {
        return false;
    }

    for (const FQuest_ResourceIngredient& Ingredient : Recipe->Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

bool AQuest_CraftingSystem::CraftItem(EQuest_CraftedItemType ItemType)
{
    if (!CanCraft(ItemType))
    {
        UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Cannot craft %d — insufficient resources"), (int32)ItemType);
        return false;
    }

    const FQuest_CraftingRecipe* Recipe = FindRecipe(ItemType);
    if (!Recipe)
    {
        return false;
    }

    ConsumeIngredients(*Recipe);
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Crafted %s"), *Recipe->DisplayName);
    return true;
}

void AQuest_CraftingSystem::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (Amount <= 0)
    {
        return;
    }

    switch (ResourceType)
    {
    case EQuest_ResourceType::Rock:   PlayerInventory.Rocks   += Amount; break;
    case EQuest_ResourceType::Stick:  PlayerInventory.Sticks  += Amount; break;
    case EQuest_ResourceType::Leaf:   PlayerInventory.Leaves  += Amount; break;
    case EQuest_ResourceType::Flint:  PlayerInventory.Flint   += Amount; break;
    case EQuest_ResourceType::Bone:   PlayerInventory.Bones   += Amount; break;
    case EQuest_ResourceType::Hide:   PlayerInventory.Hides   += Amount; break;
    default: break;
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Added %d x ResourceType %d"), Amount, (int32)ResourceType);
}

int32 AQuest_CraftingSystem::GetResourceCount(EQuest_ResourceType ResourceType) const
{
    switch (ResourceType)
    {
    case EQuest_ResourceType::Rock:   return PlayerInventory.Rocks;
    case EQuest_ResourceType::Stick:  return PlayerInventory.Sticks;
    case EQuest_ResourceType::Leaf:   return PlayerInventory.Leaves;
    case EQuest_ResourceType::Flint:  return PlayerInventory.Flint;
    case EQuest_ResourceType::Bone:   return PlayerInventory.Bones;
    case EQuest_ResourceType::Hide:   return PlayerInventory.Hides;
    default: return 0;
    }
}

void AQuest_CraftingSystem::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Menu %s"), bCraftingMenuOpen ? TEXT("OPEN") : TEXT("CLOSED"));
}

TArray<FQuest_CraftingRecipe> AQuest_CraftingSystem::GetAvailableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Available;
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (CanCraft(Recipe.ResultItem))
        {
            Available.Add(Recipe);
        }
    }
    return Available;
}

void AQuest_CraftingSystem::DebugPrintInventory()
{
    UE_LOG(LogTemp, Log, TEXT("=== CRAFTING INVENTORY ==="));
    UE_LOG(LogTemp, Log, TEXT("  Rocks:  %d"), PlayerInventory.Rocks);
    UE_LOG(LogTemp, Log, TEXT("  Sticks: %d"), PlayerInventory.Sticks);
    UE_LOG(LogTemp, Log, TEXT("  Leaves: %d"), PlayerInventory.Leaves);
    UE_LOG(LogTemp, Log, TEXT("  Flint:  %d"), PlayerInventory.Flint);
    UE_LOG(LogTemp, Log, TEXT("  Bones:  %d"), PlayerInventory.Bones);
    UE_LOG(LogTemp, Log, TEXT("  Hides:  %d"), PlayerInventory.Hides);
    UE_LOG(LogTemp, Log, TEXT("  Craftable items:"));
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        bool bCan = CanCraft(Recipe.ResultItem);
        UE_LOG(LogTemp, Log, TEXT("    %s: %s"), *Recipe.DisplayName, bCan ? TEXT("YES") : TEXT("NO"));
    }
}

void AQuest_CraftingSystem::ConsumeIngredients(const FQuest_CraftingRecipe& Recipe)
{
    for (const FQuest_ResourceIngredient& Ingredient : Recipe.Ingredients)
    {
        switch (Ingredient.ResourceType)
        {
        case EQuest_ResourceType::Rock:   PlayerInventory.Rocks   -= Ingredient.Quantity; break;
        case EQuest_ResourceType::Stick:  PlayerInventory.Sticks  -= Ingredient.Quantity; break;
        case EQuest_ResourceType::Leaf:   PlayerInventory.Leaves  -= Ingredient.Quantity; break;
        case EQuest_ResourceType::Flint:  PlayerInventory.Flint   -= Ingredient.Quantity; break;
        case EQuest_ResourceType::Bone:   PlayerInventory.Bones   -= Ingredient.Quantity; break;
        case EQuest_ResourceType::Hide:   PlayerInventory.Hides   -= Ingredient.Quantity; break;
        default: break;
        }
    }
}

const FQuest_CraftingRecipe* AQuest_CraftingSystem::FindRecipe(EQuest_CraftedItemType ItemType) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.ResultItem == ItemType)
        {
            return &Recipe;
        }
    }
    return nullptr;
}
