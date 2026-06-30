// CraftingSystem.cpp
// Agent #14 — Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260630_005
// Implements: Resource pickup actors + crafting recipe system
// 3 core recipes: Stone Axe, Campfire, Water Container

#include "CraftingSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"

// ============================================================
// AQuest_ResourcePickup
// ============================================================

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetSphereRadius(80.0f);
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    RootComponent = CollisionSphere;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoPickup)
    {
        CollisionSphere->OnComponentBeginOverlap.AddDynamic(
            this, &AQuest_ResourcePickup::OnPlayerOverlap);
    }
}

void AQuest_ResourcePickup::OnPlayerOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        Pickup();
    }
}

void AQuest_ResourcePickup::Pickup()
{
    // Notify player HUD
    FString ResourceName;
    switch (ResourceType)
    {
        case EQuest_ResourceType::Rock:  ResourceName = TEXT("Rock");  break;
        case EQuest_ResourceType::Stick: ResourceName = TEXT("Stick"); break;
        case EQuest_ResourceType::Leaf:  ResourceName = TEXT("Leaf");  break;
        case EQuest_ResourceType::Flint: ResourceName = TEXT("Flint"); break;
        case EQuest_ResourceType::Sinew: ResourceName = TEXT("Sinew"); break;
        case EQuest_ResourceType::Bone:  ResourceName = TEXT("Bone");  break;
        default: ResourceName = TEXT("Resource"); break;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
            FString::Printf(TEXT("Picked up: %s x%d"), *ResourceName, Quantity));
    }

    // Destroy pickup from world
    Destroy();
}

// ============================================================
// AQuest_CraftingSystemManager
// ============================================================

AQuest_CraftingSystemManager::AQuest_CraftingSystemManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AQuest_CraftingSystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultRecipes();
}

void AQuest_CraftingSystemManager::InitializeDefaultRecipes()
{
    Recipes.Empty();

    // ---- Recipe 1: Stone Axe ----
    // Requires: 2 Rocks + 1 Stick
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.ResultItem = EQuest_CraftedItem::StoneAxe;
        StoneAxe.DisplayName = TEXT("Stone Axe");
        StoneAxe.Description = TEXT("A crude but effective chopping tool. Splits wood and bone.");
        StoneAxe.CraftingTimeSec = 5.0f;

        FQuest_ResourceIngredient Rock;
        Rock.ResourceType = EQuest_ResourceType::Rock;
        Rock.Quantity = 2;
        StoneAxe.Ingredients.Add(Rock);

        FQuest_ResourceIngredient Stick;
        Stick.ResourceType = EQuest_ResourceType::Stick;
        Stick.Quantity = 1;
        StoneAxe.Ingredients.Add(Stick);

        Recipes.Add(StoneAxe);
    }

    // ---- Recipe 2: Campfire ----
    // Requires: 3 Sticks
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.ResultItem = EQuest_CraftedItem::Campfire;
        Campfire.DisplayName = TEXT("Campfire");
        Campfire.Description = TEXT("Warmth and light through the night. Keeps predators at bay.");
        Campfire.CraftingTimeSec = 4.0f;

        FQuest_ResourceIngredient Sticks;
        Sticks.ResourceType = EQuest_ResourceType::Stick;
        Sticks.Quantity = 3;
        Campfire.Ingredients.Add(Sticks);

        Recipes.Add(Campfire);
    }

    // ---- Recipe 3: Water Container ----
    // Requires: 1 Rock + 1 Leaf
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.ResultItem = EQuest_CraftedItem::WaterContainer;
        WaterContainer.DisplayName = TEXT("Water Container");
        WaterContainer.Description = TEXT("A hollowed stone lined with leaves. Holds enough water for one day.");
        WaterContainer.CraftingTimeSec = 6.0f;

        FQuest_ResourceIngredient Rock;
        Rock.ResourceType = EQuest_ResourceType::Rock;
        Rock.Quantity = 1;
        WaterContainer.Ingredients.Add(Rock);

        FQuest_ResourceIngredient Leaf;
        Leaf.ResourceType = EQuest_ResourceType::Leaf;
        Leaf.Quantity = 1;
        WaterContainer.Ingredients.Add(Leaf);

        Recipes.Add(WaterContainer);
    }

    // ---- Recipe 4: Spear ----
    // Requires: 2 Sticks + 1 Flint
    {
        FQuest_CraftingRecipe Spear;
        Spear.ResultItem = EQuest_CraftedItem::Spear;
        Spear.DisplayName = TEXT("Hunting Spear");
        Spear.Description = TEXT("A sharpened flint tip bound to a long branch. Effective at range.");
        Spear.CraftingTimeSec = 8.0f;

        FQuest_ResourceIngredient Sticks;
        Sticks.ResourceType = EQuest_ResourceType::Stick;
        Sticks.Quantity = 2;
        Spear.Ingredients.Add(Sticks);

        FQuest_ResourceIngredient Flint;
        Flint.ResourceType = EQuest_ResourceType::Flint;
        Flint.Quantity = 1;
        Spear.Ingredients.Add(Flint);

        Recipes.Add(Spear);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Initialized %d recipes"), Recipes.Num());
}

bool AQuest_CraftingSystemManager::CanCraft(EQuest_CraftedItem Item) const
{
    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (!Recipe) return false;

    for (const FQuest_ResourceIngredient& Ingredient : Recipe->Ingredients)
    {
        const int32* Count = PlayerInventory.Find(Ingredient.ResourceType);
        int32 Have = Count ? *Count : 0;
        if (Have < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

bool AQuest_CraftingSystemManager::CraftItem(EQuest_CraftedItem Item)
{
    if (!CanCraft(Item)) return false;

    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (!Recipe) return false;

    // Consume ingredients
    for (const FQuest_ResourceIngredient& Ingredient : Recipe->Ingredients)
    {
        int32& Count = PlayerInventory.FindOrAdd(Ingredient.ResourceType);
        Count -= Ingredient.Quantity;
        if (Count <= 0) PlayerInventory.Remove(Ingredient.ResourceType);
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
            FString::Printf(TEXT("Crafted: %s"), *Recipe->DisplayName));
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Crafted %s"), *Recipe->DisplayName);
    return true;
}

void AQuest_CraftingSystemManager::AddResource(EQuest_ResourceType Resource, int32 Amount)
{
    int32& Count = PlayerInventory.FindOrAdd(Resource);
    Count += Amount;

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Added %d x %d — total: %d"),
        (int32)Resource, Amount, Count);
}

int32 AQuest_CraftingSystemManager::GetResourceCount(EQuest_ResourceType Resource) const
{
    const int32* Count = PlayerInventory.Find(Resource);
    return Count ? *Count : 0;
}

void AQuest_CraftingSystemManager::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan,
            bCraftingMenuOpen ? TEXT("[C] Crafting Menu OPEN") : TEXT("[C] Crafting Menu CLOSED"));
    }
}

TArray<FQuest_CraftingRecipe> AQuest_CraftingSystemManager::GetAvailableRecipes() const
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

const FQuest_CraftingRecipe* AQuest_CraftingSystemManager::FindRecipe(EQuest_CraftedItem Item) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.ResultItem == Item)
        {
            return &Recipe;
        }
    }
    return nullptr;
}
