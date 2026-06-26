#include "CraftingSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

// ============================================================
// AQuest_ResourcePickup Implementation
// ============================================================

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    PickupCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("PickupCollision"));
    PickupCollision->SetupAttachment(RootComponent);
    PickupCollision->SetBoxExtent(FVector(50.f, 50.f, 50.f));
    PickupCollision->SetCollisionProfileName(TEXT("Trigger"));

    // Load default sphere mesh for resource visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
        TEXT("/Engine/BasicShapes/Sphere.Sphere")
    );
    if (SphereMesh.Succeeded())
    {
        MeshComponent->SetStaticMesh(SphereMesh.Object);
    }

    MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();

    if (PickupCollision)
    {
        PickupCollision->OnComponentBeginOverlap.AddDynamic(this, &AQuest_ResourcePickup::OnPickupOverlap);
    }
}

void AQuest_ResourcePickup::OnPickupOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || bHasBeenPickedUp)
    {
        return;
    }

    // Only respond to the player pawn
    APawn* Pawn = Cast<APawn>(OtherActor);
    if (Pawn && Pawn->IsPlayerControlled())
    {
        PickupResource(OtherActor);
    }
}

void AQuest_ResourcePickup::PickupResource(AActor* Collector)
{
    if (bHasBeenPickedUp || !Collector)
    {
        return;
    }

    bHasBeenPickedUp = true;

    // Hide the mesh — in a full implementation, add to player inventory component
    if (MeshComponent)
    {
        MeshComponent->SetVisibility(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    UE_LOG(LogTemp, Log, TEXT("ResourcePickup: %s collected %s x%d"),
        *Collector->GetName(),
        *UEnum::GetValueAsString(ResourceType),
        ResourceQuantity);

    // Destroy after short delay so overlap events resolve cleanly
    SetLifeSpan(0.5f);
}

// ============================================================
// AQuest_CraftingStation Implementation
// ============================================================

AQuest_CraftingStation::AQuest_CraftingStation()
{
    PrimaryActorTick.bCanEverTick = false;

    StationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StationMesh"));
    RootComponent = StationMesh;

    // Load default cube mesh for crafting station visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube")
    );
    if (CubeMesh.Succeeded())
    {
        StationMesh->SetStaticMesh(CubeMesh.Object);
    }
    StationMesh->SetCollisionProfileName(TEXT("BlockAll"));

    InteractionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionZone"));
    InteractionZone->SetupAttachment(RootComponent);
    InteractionZone->SetBoxExtent(FVector(150.f, 150.f, 100.f));
    InteractionZone->SetCollisionProfileName(TEXT("Trigger"));
}

void AQuest_CraftingStation::BeginPlay()
{
    Super::BeginPlay();

    InitializeDefaultRecipes();

    if (InteractionZone)
    {
        InteractionZone->OnComponentBeginOverlap.AddDynamic(
            this, &AQuest_CraftingStation::OnInteractionZoneBeginOverlap);
        InteractionZone->OnComponentEndOverlap.AddDynamic(
            this, &AQuest_CraftingStation::OnInteractionZoneEndOverlap);
    }
}

void AQuest_CraftingStation::InitializeDefaultRecipes()
{
    AvailableRecipes.Empty();

    // ---- Recipe 1: Stone Axe (2 Rocks + 1 Stick) ----
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.ResultItem   = EQuest_CraftedItem::StoneAxe;
        StoneAxe.RecipeName   = TEXT("Stone Axe");
        StoneAxe.CraftingTimeSec = 3.0f;

        FQuest_CraftingIngredient Rock;
        Rock.ResourceType = EQuest_ResourceType::Rock;
        Rock.Quantity     = 2;
        StoneAxe.Ingredients.Add(Rock);

        FQuest_CraftingIngredient Stick;
        Stick.ResourceType = EQuest_ResourceType::Stick;
        Stick.Quantity     = 1;
        StoneAxe.Ingredients.Add(Stick);

        AvailableRecipes.Add(StoneAxe);
    }

    // ---- Recipe 2: Campfire (3 Sticks) ----
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.ResultItem   = EQuest_CraftedItem::Campfire;
        Campfire.RecipeName   = TEXT("Campfire");
        Campfire.CraftingTimeSec = 5.0f;

        FQuest_CraftingIngredient Stick;
        Stick.ResourceType = EQuest_ResourceType::Stick;
        Stick.Quantity     = 3;
        Campfire.Ingredients.Add(Stick);

        AvailableRecipes.Add(Campfire);
    }

    // ---- Recipe 3: Water Container (1 Rock + 1 Leaf) ----
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.ResultItem   = EQuest_CraftedItem::WaterContainer;
        WaterContainer.RecipeName   = TEXT("Water Container");
        WaterContainer.CraftingTimeSec = 4.0f;

        FQuest_CraftingIngredient Rock;
        Rock.ResourceType = EQuest_ResourceType::Rock;
        Rock.Quantity     = 1;
        WaterContainer.Ingredients.Add(Rock);

        FQuest_CraftingIngredient Leaf;
        Leaf.ResourceType = EQuest_ResourceType::Leaf;
        Leaf.Quantity     = 1;
        WaterContainer.Ingredients.Add(Leaf);

        AvailableRecipes.Add(WaterContainer);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingStation: Initialized %d recipes"), AvailableRecipes.Num());
}

bool AQuest_CraftingStation::CanCraftItem(EQuest_CraftedItem Item, const FQuest_PlayerInventory& Inventory) const
{
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.ResultItem != Item)
        {
            continue;
        }

        // Check all ingredients
        for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
        {
            int32 Available = 0;
            switch (Ingredient.ResourceType)
            {
                case EQuest_ResourceType::Rock:   Available = Inventory.Rocks;   break;
                case EQuest_ResourceType::Stick:  Available = Inventory.Sticks;  break;
                case EQuest_ResourceType::Leaf:   Available = Inventory.Leaves;  break;
                case EQuest_ResourceType::Bone:   Available = Inventory.Bones;   break;
                case EQuest_ResourceType::Hide:   Available = Inventory.Hides;   break;
                case EQuest_ResourceType::Flint:  Available = Inventory.Flints;  break;
                default: break;
            }
            if (Available < Ingredient.Quantity)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool AQuest_CraftingStation::CraftItem(EQuest_CraftedItem Item, FQuest_PlayerInventory& Inventory)
{
    if (!CanCraftItem(Item, Inventory))
    {
        UE_LOG(LogTemp, Warning, TEXT("CraftingStation: Cannot craft %s — insufficient resources"),
            *UEnum::GetValueAsString(Item));
        return false;
    }

    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.ResultItem != Item)
        {
            continue;
        }

        // Consume ingredients
        for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
        {
            switch (Ingredient.ResourceType)
            {
                case EQuest_ResourceType::Rock:   Inventory.Rocks   -= Ingredient.Quantity; break;
                case EQuest_ResourceType::Stick:  Inventory.Sticks  -= Ingredient.Quantity; break;
                case EQuest_ResourceType::Leaf:   Inventory.Leaves  -= Ingredient.Quantity; break;
                case EQuest_ResourceType::Bone:   Inventory.Bones   -= Ingredient.Quantity; break;
                case EQuest_ResourceType::Hide:   Inventory.Hides   -= Ingredient.Quantity; break;
                case EQuest_ResourceType::Flint:  Inventory.Flints  -= Ingredient.Quantity; break;
                default: break;
            }
        }

        UE_LOG(LogTemp, Log, TEXT("CraftingStation: Successfully crafted %s"), *Recipe.RecipeName);
        return true;
    }

    return false;
}

void AQuest_CraftingStation::OnInteractionZoneBeginOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;
    APawn* Pawn = Cast<APawn>(OtherActor);
    if (Pawn && Pawn->IsPlayerControlled())
    {
        bPlayerInRange = true;
        UE_LOG(LogTemp, Log, TEXT("CraftingStation: Player entered range — press C to craft"));
    }
}

void AQuest_CraftingStation::OnInteractionZoneEndOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;
    APawn* Pawn = Cast<APawn>(OtherActor);
    if (Pawn && Pawn->IsPlayerControlled())
    {
        bPlayerInRange = false;
        UE_LOG(LogTemp, Log, TEXT("CraftingStation: Player left range"));
    }
}
