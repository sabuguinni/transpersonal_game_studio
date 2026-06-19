#include "CraftingSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"

// ============================================================
// AQuest_ResourcePickup
// ============================================================

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));

    // Default mesh — sphere placeholder (overridden in Blueprint)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        MeshComponent->SetStaticMesh(SphereMesh.Object);
        MeshComponent->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
    }
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
    bCollected = false;
}

bool AQuest_ResourcePickup::PickUp(AActor* Collector)
{
    if (bCollected || !Collector)
    {
        return false;
    }

    bCollected = true;
    MeshComponent->SetVisibility(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    UE_LOG(LogTemp, Log, TEXT("ResourcePickup: %s collected %s x%d"),
        *Collector->GetName(),
        *UEnum::GetValueAsString(ResourceType),
        Quantity);

    // Respawn after 60 seconds
    FTimerHandle RespawnTimer;
    GetWorldTimerManager().SetTimer(RespawnTimer, [this]()
    {
        bCollected = false;
        MeshComponent->SetVisibility(true);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }, 60.0f, false);

    return true;
}

// ============================================================
// AQuest_CraftingStation
// ============================================================

AQuest_CraftingStation::AQuest_CraftingStation()
{
    PrimaryActorTick.bCanEverTick = false;

    StationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StationMesh"));
    RootComponent = StationMesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        StationMesh->SetStaticMesh(CubeMesh.Object);
        StationMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.5f));
    }

    InteractionRadius = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRadius"));
    InteractionRadius->SetupAttachment(RootComponent);
    InteractionRadius->SetSphereRadius(200.0f);
    InteractionRadius->SetCollisionProfileName(TEXT("OverlapAll"));
}

void AQuest_CraftingStation::BeginPlay()
{
    Super::BeginPlay();
    InitDefaultRecipes();
}

void AQuest_CraftingStation::InitDefaultRecipes()
{
    AvailableRecipes.Empty();

    // Recipe 1: Stone Axe = 2 Rocks + 1 Stick
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItemType::StoneAxe;
        Recipe.RecipeName = TEXT("Stone Axe");
        Recipe.CraftingTime = 3.0f;
        FQuest_ResourceRequirement R1; R1.ResourceType = EQuest_ResourceType::Rock;  R1.Quantity = 2;
        FQuest_ResourceRequirement R2; R2.ResourceType = EQuest_ResourceType::Stick; R2.Quantity = 1;
        Recipe.Requirements.Add(R1);
        Recipe.Requirements.Add(R2);
        AvailableRecipes.Add(Recipe);
    }

    // Recipe 2: Campfire = 3 Sticks
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItemType::Campfire;
        Recipe.RecipeName = TEXT("Campfire");
        Recipe.CraftingTime = 2.0f;
        FQuest_ResourceRequirement R1; R1.ResourceType = EQuest_ResourceType::Stick; R1.Quantity = 3;
        Recipe.Requirements.Add(R1);
        AvailableRecipes.Add(Recipe);
    }

    // Recipe 3: Water Container = 1 Rock + 1 Leaf
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItemType::WaterContainer;
        Recipe.RecipeName = TEXT("Water Container");
        Recipe.CraftingTime = 2.5f;
        FQuest_ResourceRequirement R1; R1.ResourceType = EQuest_ResourceType::Rock; R1.Quantity = 1;
        FQuest_ResourceRequirement R2; R2.ResourceType = EQuest_ResourceType::Leaf; R2.Quantity = 1;
        Recipe.Requirements.Add(R1);
        Recipe.Requirements.Add(R2);
        AvailableRecipes.Add(Recipe);
    }

    // Recipe 4: Spear = 1 Rock + 2 Sticks
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.ResultItem = EQuest_CraftedItemType::Spear;
        Recipe.RecipeName = TEXT("Spear");
        Recipe.CraftingTime = 4.0f;
        FQuest_ResourceRequirement R1; R1.ResourceType = EQuest_ResourceType::Rock;  R1.Quantity = 1;
        FQuest_ResourceRequirement R2; R2.ResourceType = EQuest_ResourceType::Stick; R2.Quantity = 2;
        Recipe.Requirements.Add(R1);
        Recipe.Requirements.Add(R2);
        AvailableRecipes.Add(Recipe);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingStation: Initialized %d recipes"), AvailableRecipes.Num());
}

bool AQuest_CraftingStation::CanCraft(EQuest_CraftedItemType Item,
    const TMap<EQuest_ResourceType, int32>& PlayerInventory) const
{
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.ResultItem != Item) continue;

        for (const FQuest_ResourceRequirement& Req : Recipe.Requirements)
        {
            const int32* Have = PlayerInventory.Find(Req.ResourceType);
            if (!Have || *Have < Req.Quantity)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

EQuest_CraftedItemType AQuest_CraftingStation::TryCraft(EQuest_CraftedItemType Item,
    TMap<EQuest_ResourceType, int32>& PlayerInventory)
{
    if (!CanCraft(Item, PlayerInventory))
    {
        return EQuest_CraftedItemType::None;
    }

    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.ResultItem != Item) continue;

        for (const FQuest_ResourceRequirement& Req : Recipe.Requirements)
        {
            PlayerInventory[Req.ResourceType] -= Req.Quantity;
        }

        UE_LOG(LogTemp, Log, TEXT("CraftingStation: Crafted %s"), *Recipe.RecipeName);
        return Recipe.ResultItem;
    }

    return EQuest_CraftedItemType::None;
}
