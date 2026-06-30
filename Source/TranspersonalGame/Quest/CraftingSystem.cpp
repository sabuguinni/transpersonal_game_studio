#include "Quest/CraftingSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// AQuest_ResourcePickup
// ─────────────────────────────────────────────────────────────────────────────

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
    PickupCollision->SetSphereRadius(80.0f);
    PickupCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    RootComponent = PickupCollision;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Default cube mesh placeholder
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        MeshComponent->SetStaticMesh(CubeMesh.Object);
    }
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();

    // Scale mesh based on resource type
    switch (ResourceType)
    {
    case EQuest_ResourceType::Rock:
        MeshComponent->SetWorldScale3D(FVector(0.3f, 0.3f, 0.2f));
        break;
    case EQuest_ResourceType::Stick:
        MeshComponent->SetWorldScale3D(FVector(0.1f, 0.6f, 0.1f));
        break;
    case EQuest_ResourceType::Leaf:
        MeshComponent->SetWorldScale3D(FVector(0.4f, 0.4f, 0.05f));
        break;
    default:
        MeshComponent->SetWorldScale3D(FVector(0.25f, 0.25f, 0.25f));
        break;
    }

    // Bind overlap
    OnActorBeginOverlap.AddDynamic(this, &AQuest_ResourcePickup::OnPlayerOverlap);
}

void AQuest_ResourcePickup::OnPlayerOverlap(AActor* OverlappingActor)
{
    if (bIsPickedUp) return;
    if (!OverlappingActor) return;

    // Check if overlapping actor is the player (has "Player" tag or is a Pawn)
    if (!OverlappingActor->ActorHasTag(TEXT("Player")) &&
        !OverlappingActor->IsA(APawn::StaticClass()))
    {
        return;
    }

    bIsPickedUp = true;
    MeshComponent->SetVisibility(false);
    PickupCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Try to add resource to crafting system
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(),
        AQuest_CraftingSystem::StaticClass(), AllActors);
    if (AllActors.Num() > 0)
    {
        AQuest_CraftingSystem* CraftingSystem =
            Cast<AQuest_CraftingSystem>(AllActors[0]);
        if (CraftingSystem)
        {
            CraftingSystem->AddResource(ResourceType, Quantity);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ResourcePickup: Picked up %s x%d"),
        *GetResourceDisplayName(), Quantity);

    // Schedule respawn
    if (bAutoRespawn)
    {
        GetWorldTimerManager().SetTimer(
            RespawnTimerHandle,
            this,
            &AQuest_ResourcePickup::RespawnResource,
            RespawnDelay,
            false
        );
    }
}

void AQuest_ResourcePickup::RespawnResource()
{
    bIsPickedUp = false;
    MeshComponent->SetVisibility(true);
    PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    UE_LOG(LogTemp, Log, TEXT("ResourcePickup: %s respawned"),
        *GetResourceDisplayName());
}

FString AQuest_ResourcePickup::GetResourceDisplayName() const
{
    switch (ResourceType)
    {
    case EQuest_ResourceType::Rock:   return TEXT("Rock");
    case EQuest_ResourceType::Stick:  return TEXT("Stick");
    case EQuest_ResourceType::Leaf:   return TEXT("Leaf");
    case EQuest_ResourceType::Bone:   return TEXT("Bone");
    case EQuest_ResourceType::Flint:  return TEXT("Flint");
    case EQuest_ResourceType::Vine:   return TEXT("Vine");
    default:                          return TEXT("Unknown");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AQuest_CraftingSystem
// ─────────────────────────────────────────────────────────────────────────────

AQuest_CraftingSystem::AQuest_CraftingSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz
}

void AQuest_CraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeRecipes();
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Initialized with %d recipes"),
        Recipes.Num());
}

void AQuest_CraftingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Progress active crafting
    if (bIsCrafting && CurrentlyCrafting != EQuest_CraftedItem::None)
    {
        CraftingProgress += DeltaTime;
        if (CraftingProgress >= CurrentCraftTime)
        {
            OnCraftingComplete(CurrentlyCrafting);
            bIsCrafting = false;
            CraftingProgress = 0.0f;
            CurrentlyCrafting = EQuest_CraftedItem::None;
        }
    }
}

void AQuest_CraftingSystem::InitializeRecipes()
{
    Recipes.Empty();

    // ── Recipe 1: Stone Axe (2 rocks + 1 stick) ──────────────────────────────
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.ResultItem = EQuest_CraftedItem::StoneAxe;
        StoneAxe.RecipeName = TEXT("Stone Axe");
        StoneAxe.Description = TEXT("A crude but effective chopping tool. "
            "Bind a sharp rock to a sturdy stick with vine.");
        StoneAxe.CraftTimeSeconds = 3.0f;

        FQuest_ResourceRequirement RockReq;
        RockReq.ResourceType = EQuest_ResourceType::Rock;
        RockReq.Quantity = 2;

        FQuest_ResourceRequirement StickReq;
        StickReq.ResourceType = EQuest_ResourceType::Stick;
        StickReq.Quantity = 1;

        StoneAxe.Requirements.Add(RockReq);
        StoneAxe.Requirements.Add(StickReq);
        Recipes.Add(StoneAxe);
    }

    // ── Recipe 2: Campfire (3 sticks) ────────────────────────────────────────
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.ResultItem = EQuest_CraftedItem::Campfire;
        Campfire.RecipeName = TEXT("Campfire");
        Campfire.Description = TEXT("Stack dry sticks and strike flint to ignite. "
            "Keeps predators away and cooks meat.");
        Campfire.CraftTimeSeconds = 5.0f;

        FQuest_ResourceRequirement StickReq;
        StickReq.ResourceType = EQuest_ResourceType::Stick;
        StickReq.Quantity = 3;

        Campfire.Requirements.Add(StickReq);
        Recipes.Add(Campfire);
    }

    // ── Recipe 3: Water Container (1 rock + 1 leaf) ───────────────────────────
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.ResultItem = EQuest_CraftedItem::WaterContainer;
        WaterContainer.RecipeName = TEXT("Water Container");
        WaterContainer.Description = TEXT("A hollowed rock lined with large leaves "
            "to carry water from rivers and streams.");
        WaterContainer.CraftTimeSeconds = 4.0f;

        FQuest_ResourceRequirement RockReq;
        RockReq.ResourceType = EQuest_ResourceType::Rock;
        RockReq.Quantity = 1;

        FQuest_ResourceRequirement LeafReq;
        LeafReq.ResourceType = EQuest_ResourceType::Leaf;
        LeafReq.Quantity = 1;

        WaterContainer.Requirements.Add(RockReq);
        WaterContainer.Requirements.Add(LeafReq);
        Recipes.Add(WaterContainer);
    }

    // ── Recipe 4: Wooden Spear (2 sticks + 1 flint) ──────────────────────────
    {
        FQuest_CraftingRecipe WoodenSpear;
        WoodenSpear.ResultItem = EQuest_CraftedItem::WoodenSpear;
        WoodenSpear.RecipeName = TEXT("Wooden Spear");
        WoodenSpear.Description = TEXT("A long sharpened stick tipped with flint. "
            "Essential for hunting and defense against raptors.");
        WoodenSpear.CraftTimeSeconds = 6.0f;

        FQuest_ResourceRequirement StickReq;
        StickReq.ResourceType = EQuest_ResourceType::Stick;
        StickReq.Quantity = 2;

        FQuest_ResourceRequirement FlintReq;
        FlintReq.ResourceType = EQuest_ResourceType::Flint;
        FlintReq.Quantity = 1;

        WoodenSpear.Requirements.Add(StickReq);
        WoodenSpear.Requirements.Add(FlintReq);
        Recipes.Add(WoodenSpear);
    }

    // ── Recipe 5: Leaf Shelter (3 leaves + 2 sticks) ─────────────────────────
    {
        FQuest_CraftingRecipe LeafShelter;
        LeafShelter.ResultItem = EQuest_CraftedItem::LeafShelter;
        LeafShelter.RecipeName = TEXT("Leaf Shelter");
        LeafShelter.Description = TEXT("A simple lean-to made from sticks and broad "
            "leaves. Provides protection from rain and cold nights.");
        LeafShelter.CraftTimeSeconds = 8.0f;

        FQuest_ResourceRequirement LeafReq;
        LeafReq.ResourceType = EQuest_ResourceType::Leaf;
        LeafReq.Quantity = 3;

        FQuest_ResourceRequirement StickReq;
        StickReq.ResourceType = EQuest_ResourceType::Stick;
        StickReq.Quantity = 2;

        LeafShelter.Requirements.Add(LeafReq);
        LeafShelter.Requirements.Add(StickReq);
        Recipes.Add(LeafShelter);
    }
}

void AQuest_CraftingSystem::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Menu %s"),
        bCraftingMenuOpen ? TEXT("OPENED") : TEXT("CLOSED"));
}

bool AQuest_CraftingSystem::CanCraft(EQuest_CraftedItem Item) const
{
    FQuest_CraftingRecipe Recipe = GetRecipeForItem(Item);
    if (Recipe.ResultItem == EQuest_CraftedItem::None) return false;

    for (const FQuest_ResourceRequirement& Req : Recipe.Requirements)
    {
        if (GetResourceCount(Req.ResourceType) < Req.Quantity)
            return false;
    }
    return true;
}

bool AQuest_CraftingSystem::CraftItem(EQuest_CraftedItem Item)
{
    if (!CanCraft(Item)) return false;
    if (bIsCrafting) return false;

    FQuest_CraftingRecipe Recipe = GetRecipeForItem(Item);
    if (!ConsumeResources(Recipe)) return false;

    bIsCrafting = true;
    CurrentlyCrafting = Item;
    CraftingProgress = 0.0f;
    CurrentCraftTime = Recipe.CraftTimeSeconds;

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Started crafting %s (%.1fs)"),
        *Recipe.RecipeName, Recipe.CraftTimeSeconds);
    return true;
}

void AQuest_CraftingSystem::AddResource(EQuest_ResourceType Resource, int32 Amount)
{
    switch (Resource)
    {
    case EQuest_ResourceType::Rock:   PlayerInventory.Rocks  += Amount; break;
    case EQuest_ResourceType::Stick:  PlayerInventory.Sticks += Amount; break;
    case EQuest_ResourceType::Leaf:   PlayerInventory.Leaves += Amount; break;
    case EQuest_ResourceType::Bone:   PlayerInventory.Bones  += Amount; break;
    case EQuest_ResourceType::Flint:  PlayerInventory.Flint  += Amount; break;
    case EQuest_ResourceType::Vine:   PlayerInventory.Vines  += Amount; break;
    default: break;
    }
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Added %d resource type %d"),
        Amount, (int32)Resource);
}

int32 AQuest_CraftingSystem::GetResourceCount(EQuest_ResourceType Resource) const
{
    switch (Resource)
    {
    case EQuest_ResourceType::Rock:   return PlayerInventory.Rocks;
    case EQuest_ResourceType::Stick:  return PlayerInventory.Sticks;
    case EQuest_ResourceType::Leaf:   return PlayerInventory.Leaves;
    case EQuest_ResourceType::Bone:   return PlayerInventory.Bones;
    case EQuest_ResourceType::Flint:  return PlayerInventory.Flint;
    case EQuest_ResourceType::Vine:   return PlayerInventory.Vines;
    default:                          return 0;
    }
}

TArray<FQuest_CraftingRecipe> AQuest_CraftingSystem::GetAvailableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Available;
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (CanCraft(Recipe.ResultItem))
            Available.Add(Recipe);
    }
    return Available;
}

FQuest_CraftingRecipe AQuest_CraftingSystem::GetRecipeForItem(
    EQuest_CraftedItem Item) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.ResultItem == Item)
            return Recipe;
    }
    return FQuest_CraftingRecipe(); // empty / None
}

bool AQuest_CraftingSystem::HasCraftedItem(EQuest_CraftedItem Item) const
{
    return CraftedItems.Contains(Item);
}

void AQuest_CraftingSystem::HandleCraftingInput()
{
    ToggleCraftingMenu();
}

bool AQuest_CraftingSystem::ConsumeResources(const FQuest_CraftingRecipe& Recipe)
{
    // Verify first
    for (const FQuest_ResourceRequirement& Req : Recipe.Requirements)
    {
        if (GetResourceCount(Req.ResourceType) < Req.Quantity)
            return false;
    }
    // Consume
    for (const FQuest_ResourceRequirement& Req : Recipe.Requirements)
    {
        AddResource(Req.ResourceType, -Req.Quantity);
    }
    return true;
}

void AQuest_CraftingSystem::OnCraftingComplete(EQuest_CraftedItem Item)
{
    CraftedItems.AddUnique(Item);
    FQuest_CraftingRecipe Recipe = GetRecipeForItem(Item);
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Crafted [%s] successfully!"),
        *Recipe.RecipeName);
}
