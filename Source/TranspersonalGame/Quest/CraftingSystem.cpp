// CraftingSystem.cpp
// Agent #14 — Quest & Mission Designer
// Full implementation of crafting system: recipes, resource pickups, inventory
// CYCLE: PROD_CYCLE_AUTO_20260628_009

#include "CraftingSystem.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─── AQuest_ResourcePickup ────────────────────────────────────────────────────

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

    PickupRadius = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRadius"));
    PickupRadius->SetupAttachment(RootComponent);
    PickupRadius->SetSphereRadius(120.0f);
    PickupRadius->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PickupRadius->SetCollisionResponseToAllChannels(ECR_Ignore);
    PickupRadius->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
    PickupRadius->OnComponentBeginOverlap.AddDynamic(this, &AQuest_ResourcePickup::OnOverlapBegin);
}

void AQuest_ResourcePickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || bHasBeenPickedUp) return;

    // Auto-pickup when player walks over resource
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar && PlayerChar->IsPlayerControlled())
    {
        PickUp(OtherActor);
    }
}

void AQuest_ResourcePickup::PickUp(AActor* Collector)
{
    if (bHasBeenPickedUp || !Collector) return;

    bHasBeenPickedUp = true;

    // Find crafting manager in world and add resource
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AQuest_CraftingManager::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            AQuest_CraftingManager* CraftingMgr = Cast<AQuest_CraftingManager>(FoundActors[0]);
            if (CraftingMgr)
            {
                CraftingMgr->AddResource(ResourceType, Quantity);
                UE_LOG(LogTemp, Log, TEXT("ResourcePickup: Added %d of resource type %d to inventory"),
                    Quantity, (int32)ResourceType);
            }
        }
    }

    // Hide the pickup mesh
    MeshComponent->SetVisibility(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PickupRadius->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Destroy after short delay
    SetLifeSpan(0.5f);
}

bool AQuest_ResourcePickup::IsAvailable() const
{
    return !bHasBeenPickedUp;
}

// ─── AQuest_CraftingManager ───────────────────────────────────────────────────

AQuest_CraftingManager::AQuest_CraftingManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AQuest_CraftingManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultRecipes();
    UE_LOG(LogTemp, Log, TEXT("CraftingManager: Initialized with %d recipes"), Recipes.Num());
}

void AQuest_CraftingManager::InitializeDefaultRecipes()
{
    Recipes.Empty();

    // ── Recipe 1: Stone Axe (2 Rocks + 1 Stick) ──────────────────────────────
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.ResultItem = EQuest_CraftedItemType::StoneAxe;
        StoneAxe.RecipeName = TEXT("Stone Axe");
        StoneAxe.Description = TEXT("A crude but effective chopping tool. Useful for cutting wood and defending against smaller predators.");
        StoneAxe.CraftingTimeSec = 4.0f;
        StoneAxe.bRequiresCampfire = false;

        FQuest_ResourceIngredient RockIngredient;
        RockIngredient.ResourceType = EQuest_ResourceType::Rock;
        RockIngredient.Quantity = 2;
        StoneAxe.Ingredients.Add(RockIngredient);

        FQuest_ResourceIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity = 1;
        StoneAxe.Ingredients.Add(StickIngredient);

        Recipes.Add(StoneAxe);
    }

    // ── Recipe 2: Campfire (3 Sticks) ────────────────────────────────────────
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.ResultItem = EQuest_CraftedItemType::Campfire;
        Campfire.RecipeName = TEXT("Campfire");
        Campfire.Description = TEXT("A fire for warmth, cooking, and keeping predators at bay during the night.");
        Campfire.CraftingTimeSec = 6.0f;
        Campfire.bRequiresCampfire = false;

        FQuest_ResourceIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity = 3;
        Campfire.Ingredients.Add(StickIngredient);

        Recipes.Add(Campfire);
    }

    // ── Recipe 3: Water Container (1 Rock + 1 Leaf) ───────────────────────────
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.ResultItem = EQuest_CraftedItemType::WaterContainer;
        WaterContainer.RecipeName = TEXT("Water Container");
        WaterContainer.Description = TEXT("A hollowed rock sealed with large leaves. Holds enough water to survive a day's march.");
        WaterContainer.CraftingTimeSec = 5.0f;
        WaterContainer.bRequiresCampfire = false;

        FQuest_ResourceIngredient RockIngredient;
        RockIngredient.ResourceType = EQuest_ResourceType::Rock;
        RockIngredient.Quantity = 1;
        WaterContainer.Ingredients.Add(RockIngredient);

        FQuest_ResourceIngredient LeafIngredient;
        LeafIngredient.ResourceType = EQuest_ResourceType::Leaf;
        LeafIngredient.Quantity = 1;
        WaterContainer.Ingredients.Add(LeafIngredient);

        Recipes.Add(WaterContainer);
    }

    // ── Recipe 4: Spear (2 Sticks + 1 Flint) ─────────────────────────────────
    {
        FQuest_CraftingRecipe Spear;
        Spear.ResultItem = EQuest_CraftedItemType::Spear;
        Spear.RecipeName = TEXT("Spear");
        Spear.Description = TEXT("A long wooden shaft tipped with a sharp flint head. Effective for hunting and keeping raptors at distance.");
        Spear.CraftingTimeSec = 8.0f;
        Spear.bRequiresCampfire = false;

        FQuest_ResourceIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity = 2;
        Spear.Ingredients.Add(StickIngredient);

        FQuest_ResourceIngredient FlintIngredient;
        FlintIngredient.ResourceType = EQuest_ResourceType::Flint;
        FlintIngredient.Quantity = 1;
        Spear.Ingredients.Add(FlintIngredient);

        Recipes.Add(Spear);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingManager: %d recipes initialized"), Recipes.Num());
}

bool AQuest_CraftingManager::CanCraft(EQuest_CraftedItemType ItemType) const
{
    const FQuest_CraftingRecipe* Recipe = FindRecipe(ItemType);
    if (!Recipe) return false;

    for (const FQuest_ResourceIngredient& Ingredient : Recipe->Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

bool AQuest_CraftingManager::CraftItem(EQuest_CraftedItemType ItemType)
{
    if (!CanCraft(ItemType)) return false;

    const FQuest_CraftingRecipe* Recipe = FindRecipe(ItemType);
    if (!Recipe) return false;

    // Consume ingredients
    for (const FQuest_ResourceIngredient& Ingredient : Recipe->Ingredients)
    {
        AddResource(Ingredient.ResourceType, -Ingredient.Quantity);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingManager: Crafted %s"), *Recipe->RecipeName);
    return true;
}

void AQuest_CraftingManager::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    switch (ResourceType)
    {
    case EQuest_ResourceType::Rock:
        PlayerInventory.Rocks = FMath::Max(0, PlayerInventory.Rocks + Amount);
        break;
    case EQuest_ResourceType::Stick:
        PlayerInventory.Sticks = FMath::Max(0, PlayerInventory.Sticks + Amount);
        break;
    case EQuest_ResourceType::Leaf:
        PlayerInventory.Leaves = FMath::Max(0, PlayerInventory.Leaves + Amount);
        break;
    case EQuest_ResourceType::Bone:
        PlayerInventory.Bones = FMath::Max(0, PlayerInventory.Bones + Amount);
        break;
    case EQuest_ResourceType::Flint:
        PlayerInventory.Flint = FMath::Max(0, PlayerInventory.Flint + Amount);
        break;
    case EQuest_ResourceType::Hide:
        PlayerInventory.Hide = FMath::Max(0, PlayerInventory.Hide + Amount);
        break;
    case EQuest_ResourceType::Fiber:
        PlayerInventory.Fiber = FMath::Max(0, PlayerInventory.Fiber + Amount);
        break;
    default:
        break;
    }
}

int32 AQuest_CraftingManager::GetResourceCount(EQuest_ResourceType ResourceType) const
{
    switch (ResourceType)
    {
    case EQuest_ResourceType::Rock:   return PlayerInventory.Rocks;
    case EQuest_ResourceType::Stick:  return PlayerInventory.Sticks;
    case EQuest_ResourceType::Leaf:   return PlayerInventory.Leaves;
    case EQuest_ResourceType::Bone:   return PlayerInventory.Bones;
    case EQuest_ResourceType::Flint:  return PlayerInventory.Flint;
    case EQuest_ResourceType::Hide:   return PlayerInventory.Hide;
    case EQuest_ResourceType::Fiber:  return PlayerInventory.Fiber;
    default: return 0;
    }
}

void AQuest_CraftingManager::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    UE_LOG(LogTemp, Log, TEXT("CraftingManager: Menu %s"),
        bCraftingMenuOpen ? TEXT("OPENED") : TEXT("CLOSED"));
}

TArray<FQuest_CraftingRecipe> AQuest_CraftingManager::GetAvailableRecipes() const
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

const FQuest_CraftingRecipe* AQuest_CraftingManager::FindRecipe(EQuest_CraftedItemType ItemType) const
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

bool AQuest_CraftingManager::ConsumeIngredients(const FQuest_CraftingRecipe& Recipe)
{
    // Verify first
    for (const FQuest_ResourceIngredient& Ingredient : Recipe.Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
        {
            return false;
        }
    }
    // Consume
    for (const FQuest_ResourceIngredient& Ingredient : Recipe.Ingredients)
    {
        AddResource(Ingredient.ResourceType, -Ingredient.Quantity);
    }
    return true;
}
