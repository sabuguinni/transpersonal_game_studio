#include "CraftingSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

// ============================================================
// AQuest_ResourcePickup
// ============================================================

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    PickupRadius = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRadius"));
    PickupRadius->SetupAttachment(RootComponent);
    PickupRadius->SetSphereRadius(100.0f);
    PickupRadius->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    // Default sphere mesh for pickup visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        MeshComponent->SetStaticMesh(SphereMesh.Object);
    }

    MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
}

bool AQuest_ResourcePickup::PickUp(AActor* Collector)
{
    if (bHasBeenPickedUp || !Collector)
    {
        return false;
    }

    bHasBeenPickedUp = true;

    // Hide and disable collision
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    UE_LOG(LogTemp, Log, TEXT("ResourcePickup: %s picked up %s x%d"),
        *Collector->GetName(),
        *GetResourceName(),
        Quantity);

    return true;
}

FString AQuest_ResourcePickup::GetResourceName() const
{
    switch (ResourceType)
    {
    case EQuest_ResourceType::FlintRock:   return TEXT("Flint Rock");
    case EQuest_ResourceType::Stick:       return TEXT("Stick");
    case EQuest_ResourceType::Leaf:        return TEXT("Leaf");
    case EQuest_ResourceType::Bone:        return TEXT("Bone");
    case EQuest_ResourceType::Vine:        return TEXT("Vine");
    case EQuest_ResourceType::Clay:        return TEXT("Clay");
    default:                               return TEXT("Unknown");
    }
}

// ============================================================
// AQuest_CraftingSystem
// ============================================================

AQuest_CraftingSystem::AQuest_CraftingSystem()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AQuest_CraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultRecipes();
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Initialized with %d recipes"), Recipes.Num());
}

void AQuest_CraftingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AQuest_CraftingSystem::InitializeDefaultRecipes()
{
    Recipes.Empty();

    // ---- Recipe 1: Stone Axe (2 Flint Rocks + 1 Stick) ----
    {
        FQuest_CraftingRecipe StoneAxeRecipe;
        StoneAxeRecipe.ResultItem = EQuest_CraftedItem::StoneAxe;
        StoneAxeRecipe.RecipeName = TEXT("Stone Axe");
        StoneAxeRecipe.Description = TEXT("A basic stone axe. Useful for chopping wood and fighting small predators.");
        StoneAxeRecipe.CraftingTime = 3.0f;

        FQuest_CraftingIngredient Flint;
        Flint.ResourceType = EQuest_ResourceType::FlintRock;
        Flint.Quantity = 2;

        FQuest_CraftingIngredient Stick;
        Stick.ResourceType = EQuest_ResourceType::Stick;
        Stick.Quantity = 1;

        StoneAxeRecipe.Ingredients.Add(Flint);
        StoneAxeRecipe.Ingredients.Add(Stick);
        Recipes.Add(StoneAxeRecipe);
    }

    // ---- Recipe 2: Campfire (3 Sticks) ----
    {
        FQuest_CraftingRecipe CampfireRecipe;
        CampfireRecipe.ResultItem = EQuest_CraftedItem::Campfire;
        CampfireRecipe.RecipeName = TEXT("Campfire");
        CampfireRecipe.Description = TEXT("A campfire for warmth and cooking. Keeps predators at bay during the night.");
        CampfireRecipe.CraftingTime = 5.0f;

        FQuest_CraftingIngredient Sticks;
        Sticks.ResourceType = EQuest_ResourceType::Stick;
        Sticks.Quantity = 3;

        CampfireRecipe.Ingredients.Add(Sticks);
        Recipes.Add(CampfireRecipe);
    }

    // ---- Recipe 3: Water Container (1 Flint Rock + 1 Leaf) ----
    {
        FQuest_CraftingRecipe WaterContainerRecipe;
        WaterContainerRecipe.ResultItem = EQuest_CraftedItem::WaterContainer;
        WaterContainerRecipe.RecipeName = TEXT("Water Container");
        WaterContainerRecipe.Description = TEXT("A crude container made from a large leaf shaped around a rock. Holds enough water for one day.");
        WaterContainerRecipe.CraftingTime = 2.0f;

        FQuest_CraftingIngredient Rock;
        Rock.ResourceType = EQuest_ResourceType::FlintRock;
        Rock.Quantity = 1;

        FQuest_CraftingIngredient Leaf;
        Leaf.ResourceType = EQuest_ResourceType::Leaf;
        Leaf.Quantity = 1;

        WaterContainerRecipe.Ingredients.Add(Rock);
        WaterContainerRecipe.Ingredients.Add(Leaf);
        Recipes.Add(WaterContainerRecipe);
    }

    // ---- Recipe 4: Spear (1 Stick + 1 Flint Rock + 1 Vine) ----
    {
        FQuest_CraftingRecipe SpearRecipe;
        SpearRecipe.ResultItem = EQuest_CraftedItem::Spear;
        SpearRecipe.RecipeName = TEXT("Spear");
        SpearRecipe.Description = TEXT("A sharpened spear. Effective against medium-sized dinosaurs from a safe distance.");
        SpearRecipe.CraftingTime = 6.0f;

        FQuest_CraftingIngredient SpearStick;
        SpearStick.ResourceType = EQuest_ResourceType::Stick;
        SpearStick.Quantity = 1;

        FQuest_CraftingIngredient SpearFlint;
        SpearFlint.ResourceType = EQuest_ResourceType::FlintRock;
        SpearFlint.Quantity = 1;

        FQuest_CraftingIngredient SpearVine;
        SpearVine.ResourceType = EQuest_ResourceType::Vine;
        SpearVine.Quantity = 1;

        SpearRecipe.Ingredients.Add(SpearStick);
        SpearRecipe.Ingredients.Add(SpearFlint);
        SpearRecipe.Ingredients.Add(SpearVine);
        Recipes.Add(SpearRecipe);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Loaded %d recipes"), Recipes.Num());
}

bool AQuest_CraftingSystem::CanCraft(EQuest_CraftedItem Item) const
{
    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (!Recipe)
    {
        return false;
    }

    for (const FQuest_CraftingIngredient& Ingredient : Recipe->Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

bool AQuest_CraftingSystem::CraftItem(EQuest_CraftedItem Item)
{
    if (!CanCraft(Item))
    {
        UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Cannot craft %d — insufficient resources"), (int32)Item);
        return false;
    }

    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (!Recipe)
    {
        return false;
    }

    // Consume ingredients
    if (!ConsumeIngredients(*Recipe))
    {
        return false;
    }

    // Add crafted item to inventory
    switch (Item)
    {
    case EQuest_CraftedItem::StoneAxe:       PlayerInventory.StoneAxes++;       break;
    case EQuest_CraftedItem::Campfire:       PlayerInventory.Campfires++;       break;
    case EQuest_CraftedItem::WaterContainer: PlayerInventory.WaterContainers++; break;
    case EQuest_CraftedItem::Spear:          PlayerInventory.Spears++;          break;
    default: break;
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Crafted %s successfully!"), *Recipe->RecipeName);
    return true;
}

void AQuest_CraftingSystem::AddResource(EQuest_ResourceType Resource, int32 Amount)
{
    switch (Resource)
    {
    case EQuest_ResourceType::FlintRock: PlayerInventory.FlintRocks += Amount; break;
    case EQuest_ResourceType::Stick:     PlayerInventory.Sticks += Amount;     break;
    case EQuest_ResourceType::Leaf:      PlayerInventory.Leaves += Amount;     break;
    case EQuest_ResourceType::Bone:      PlayerInventory.Bones += Amount;      break;
    case EQuest_ResourceType::Vine:      PlayerInventory.Vines += Amount;      break;
    case EQuest_ResourceType::Clay:      PlayerInventory.Clay += Amount;       break;
    default: break;
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Added %d x %d to inventory"), Amount, (int32)Resource);
}

int32 AQuest_CraftingSystem::GetResourceCount(EQuest_ResourceType Resource) const
{
    switch (Resource)
    {
    case EQuest_ResourceType::FlintRock: return PlayerInventory.FlintRocks;
    case EQuest_ResourceType::Stick:     return PlayerInventory.Sticks;
    case EQuest_ResourceType::Leaf:      return PlayerInventory.Leaves;
    case EQuest_ResourceType::Bone:      return PlayerInventory.Bones;
    case EQuest_ResourceType::Vine:      return PlayerInventory.Vines;
    case EQuest_ResourceType::Clay:      return PlayerInventory.Clay;
    default:                             return 0;
    }
}

void AQuest_CraftingSystem::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Menu %s"),
        bCraftingMenuOpen ? TEXT("OPENED") : TEXT("CLOSED"));
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

void AQuest_CraftingSystem::PrintInventoryDebug()
{
    UE_LOG(LogTemp, Log, TEXT("=== CRAFTING INVENTORY DEBUG ==="));
    UE_LOG(LogTemp, Log, TEXT("  FlintRocks: %d"), PlayerInventory.FlintRocks);
    UE_LOG(LogTemp, Log, TEXT("  Sticks:     %d"), PlayerInventory.Sticks);
    UE_LOG(LogTemp, Log, TEXT("  Leaves:     %d"), PlayerInventory.Leaves);
    UE_LOG(LogTemp, Log, TEXT("  Bones:      %d"), PlayerInventory.Bones);
    UE_LOG(LogTemp, Log, TEXT("  Vines:      %d"), PlayerInventory.Vines);
    UE_LOG(LogTemp, Log, TEXT("  Clay:       %d"), PlayerInventory.Clay);
    UE_LOG(LogTemp, Log, TEXT("  --- Crafted ---"));
    UE_LOG(LogTemp, Log, TEXT("  StoneAxes:       %d"), PlayerInventory.StoneAxes);
    UE_LOG(LogTemp, Log, TEXT("  Campfires:       %d"), PlayerInventory.Campfires);
    UE_LOG(LogTemp, Log, TEXT("  WaterContainers: %d"), PlayerInventory.WaterContainers);
    UE_LOG(LogTemp, Log, TEXT("  Spears:          %d"), PlayerInventory.Spears);
    UE_LOG(LogTemp, Log, TEXT("================================"));
}

const FQuest_CraftingRecipe* AQuest_CraftingSystem::FindRecipe(EQuest_CraftedItem Item) const
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

bool AQuest_CraftingSystem::ConsumeIngredients(const FQuest_CraftingRecipe& Recipe)
{
    // Verify all ingredients first
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
        switch (Ingredient.ResourceType)
        {
        case EQuest_ResourceType::FlintRock: PlayerInventory.FlintRocks -= Ingredient.Quantity; break;
        case EQuest_ResourceType::Stick:     PlayerInventory.Sticks     -= Ingredient.Quantity; break;
        case EQuest_ResourceType::Leaf:      PlayerInventory.Leaves     -= Ingredient.Quantity; break;
        case EQuest_ResourceType::Bone:      PlayerInventory.Bones      -= Ingredient.Quantity; break;
        case EQuest_ResourceType::Vine:      PlayerInventory.Vines      -= Ingredient.Quantity; break;
        case EQuest_ResourceType::Clay:      PlayerInventory.Clay       -= Ingredient.Quantity; break;
        default: break;
        }
    }
    return true;
}
