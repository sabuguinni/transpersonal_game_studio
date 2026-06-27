#include "CraftingSystem.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

// ============================================================
// ACraftingSystem
// ============================================================

ACraftingSystem::ACraftingSystem()
{
    PrimaryActorTick.bCanEverTick = false;
    InitializeRecipes();
}

void ACraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    // Ensure recipes are initialized at runtime too
    if (Recipes.Num() == 0)
    {
        InitializeRecipes();
    }
}

void ACraftingSystem::InitializeRecipes()
{
    Recipes.Empty();

    // --- Recipe 1: Stone Axe (2 Rocks + 1 Stick) ---
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.OutputItem   = EQuest_CraftedItemType::StoneAxe;
        StoneAxe.RecipeName   = TEXT("Stone Axe");
        StoneAxe.CraftingTimeSec = 3.0f;

        FQuest_CraftingIngredient Rock;
        Rock.ResourceType = EQuest_ResourceType::Rock;
        Rock.Quantity     = 2;

        FQuest_CraftingIngredient Stick;
        Stick.ResourceType = EQuest_ResourceType::Stick;
        Stick.Quantity     = 1;

        StoneAxe.Ingredients.Add(Rock);
        StoneAxe.Ingredients.Add(Stick);
        Recipes.Add(StoneAxe);
    }

    // --- Recipe 2: Campfire (3 Sticks) ---
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.OutputItem   = EQuest_CraftedItemType::Campfire;
        Campfire.RecipeName   = TEXT("Campfire");
        Campfire.CraftingTimeSec = 5.0f;

        FQuest_CraftingIngredient Stick;
        Stick.ResourceType = EQuest_ResourceType::Stick;
        Stick.Quantity     = 3;

        Campfire.Ingredients.Add(Stick);
        Recipes.Add(Campfire);
    }

    // --- Recipe 3: Water Container (1 Rock + 1 Leaf) ---
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.OutputItem   = EQuest_CraftedItemType::WaterContainer;
        WaterContainer.RecipeName   = TEXT("Water Container");
        WaterContainer.CraftingTimeSec = 4.0f;

        FQuest_CraftingIngredient Rock;
        Rock.ResourceType = EQuest_ResourceType::Rock;
        Rock.Quantity     = 1;

        FQuest_CraftingIngredient Leaf;
        Leaf.ResourceType = EQuest_ResourceType::Leaf;
        Leaf.Quantity     = 1;

        WaterContainer.Ingredients.Add(Rock);
        WaterContainer.Ingredients.Add(Leaf);
        Recipes.Add(WaterContainer);
    }

    // --- Recipe 4: Spear (2 Sticks + 1 Rock) ---
    {
        FQuest_CraftingRecipe Spear;
        Spear.OutputItem   = EQuest_CraftedItemType::Spear;
        Spear.RecipeName   = TEXT("Spear");
        Spear.CraftingTimeSec = 6.0f;

        FQuest_CraftingIngredient Stick;
        Stick.ResourceType = EQuest_ResourceType::Stick;
        Stick.Quantity     = 2;

        FQuest_CraftingIngredient Rock;
        Rock.ResourceType = EQuest_ResourceType::Rock;
        Rock.Quantity     = 1;

        Spear.Ingredients.Add(Stick);
        Spear.Ingredients.Add(Rock);
        Recipes.Add(Spear);
    }
}

void ACraftingSystem::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (ResourceType == EQuest_ResourceType::None || Amount <= 0)
    {
        return;
    }

    if (PlayerInventory.Contains(ResourceType))
    {
        PlayerInventory[ResourceType] += Amount;
    }
    else
    {
        PlayerInventory.Add(ResourceType, Amount);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Added %d of resource type %d. New total: %d"),
        Amount, (int32)ResourceType, PlayerInventory[ResourceType]);
}

bool ACraftingSystem::CanCraft(EQuest_CraftedItemType ItemType) const
{
    FQuest_CraftingRecipe Recipe;
    if (!GetRecipe(ItemType, Recipe))
    {
        return false;
    }

    for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
    {
        const int32* HavePtr = PlayerInventory.Find(Ingredient.ResourceType);
        int32 Have = HavePtr ? *HavePtr : 0;
        if (Have < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

bool ACraftingSystem::TryCraft(EQuest_CraftedItemType ItemType)
{
    if (!CanCraft(ItemType))
    {
        UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Cannot craft item %d — insufficient resources"), (int32)ItemType);
        return false;
    }

    FQuest_CraftingRecipe Recipe;
    GetRecipe(ItemType, Recipe);

    // Consume ingredients
    for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
    {
        PlayerInventory[Ingredient.ResourceType] -= Ingredient.Quantity;
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Crafted '%s' successfully!"), *Recipe.RecipeName);
    return true;
}

bool ACraftingSystem::GetRecipe(EQuest_CraftedItemType ItemType, FQuest_CraftingRecipe& OutRecipe) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.OutputItem == ItemType)
        {
            OutRecipe = Recipe;
            return true;
        }
    }
    return false;
}

void ACraftingSystem::ToggleCraftingUI()
{
    bCraftingUIOpen = !bCraftingUIOpen;
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: UI %s"), bCraftingUIOpen ? TEXT("OPENED") : TEXT("CLOSED"));
}

// ============================================================
// AResourcePickupActor
// ============================================================

AResourcePickupActor::AResourcePickupActor()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    // Load a basic sphere mesh as placeholder for all resource types
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        MeshComponent->SetStaticMesh(SphereMesh.Object);
    }

    MeshComponent->SetCollisionProfileName(TEXT("OverlapAll"));
}

void AResourcePickupActor::BeginPlay()
{
    Super::BeginPlay();
}

void AResourcePickupActor::Collect(ACraftingSystem* CraftingSystem)
{
    if (bCollected || !CraftingSystem)
    {
        return;
    }

    bCollected = true;
    CraftingSystem->AddResource(ResourceType, ResourceAmount);

    UE_LOG(LogTemp, Log, TEXT("ResourcePickup: Collected %s (amount=%d)"),
        *GetName(), ResourceAmount);

    // Hide and disable collision after collection
    MeshComponent->SetVisibility(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Destroy after short delay
    SetLifeSpan(0.5f);
}
