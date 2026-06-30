#include "CraftingSystem.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

// ============================================================
// AQuest_ResourcePickup
// ============================================================

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetSphereRadius(80.0f);
    CollisionSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = CollisionSphere;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_ResourcePickup::OnOverlapBegin);
}

void AQuest_ResourcePickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || bHasBeenPickedUp) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (PlayerChar)
    {
        PickUp(OtherActor);
    }
}

void AQuest_ResourcePickup::PickUp(AActor* Collector)
{
    if (bHasBeenPickedUp || !Collector) return;

    // Find crafting component on collector
    UQuest_CraftingComponent* CraftingComp = Collector->FindComponentByClass<UQuest_CraftingComponent>();
    if (CraftingComp)
    {
        CraftingComp->AddResource(ResourceType, Quantity);
        bHasBeenPickedUp = true;

        // Log pickup for debugging
        FString ResourceName;
        switch (ResourceType)
        {
            case EQuest_ResourceType::Rock:  ResourceName = TEXT("Rock");  break;
            case EQuest_ResourceType::Stick: ResourceName = TEXT("Stick"); break;
            case EQuest_ResourceType::Leaf:  ResourceName = TEXT("Leaf");  break;
            case EQuest_ResourceType::Sinew: ResourceName = TEXT("Sinew"); break;
            case EQuest_ResourceType::Flint: ResourceName = TEXT("Flint"); break;
            case EQuest_ResourceType::Bone:  ResourceName = TEXT("Bone");  break;
            default: ResourceName = TEXT("Unknown"); break;
        }

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
                FString::Printf(TEXT("Picked up: %s x%d"), *ResourceName, Quantity));
        }

        // Hide and schedule destroy
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);

        FTimerHandle DestroyTimer;
        GetWorldTimerManager().SetTimer(DestroyTimer, [this]()
        {
            Destroy();
        }, 1.0f, false);
    }
}

// ============================================================
// UQuest_CraftingComponent
// ============================================================

UQuest_CraftingComponent::UQuest_CraftingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UQuest_CraftingComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeRecipes();
}

void UQuest_CraftingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsCrafting && CraftingDuration > 0.0f)
    {
        CraftingTimer += DeltaTime;
        CraftingProgress = FMath::Clamp(CraftingTimer / CraftingDuration, 0.0f, 1.0f);

        if (CraftingTimer >= CraftingDuration)
        {
            CompleteCrafting();
        }
    }
}

void UQuest_CraftingComponent::InitializeRecipes()
{
    AvailableRecipes.Empty();

    // Recipe 1: Stone Axe — 2 Rocks + 1 Sinew
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.ResultItem = EQuest_CraftedItem::StoneAxe;
        StoneAxe.DisplayName = TEXT("Stone Axe");
        StoneAxe.Description = TEXT("A crude but effective axe. Useful for chopping wood and defending against small predators.");
        StoneAxe.CraftingTimeSeconds = 5.0f;

        FQuest_ResourceIngredient Rock1;
        Rock1.ResourceType = EQuest_ResourceType::Rock;
        Rock1.Quantity = 2;
        StoneAxe.Ingredients.Add(Rock1);

        FQuest_ResourceIngredient Sinew1;
        Sinew1.ResourceType = EQuest_ResourceType::Sinew;
        Sinew1.Quantity = 1;
        StoneAxe.Ingredients.Add(Sinew1);

        AvailableRecipes.Add(StoneAxe);
    }

    // Recipe 2: Campfire — 3 Sticks
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.ResultItem = EQuest_CraftedItem::Campfire;
        Campfire.DisplayName = TEXT("Campfire");
        Campfire.Description = TEXT("A small fire. Provides warmth, light, and a place to cook meat. Deters small predators.");
        Campfire.CraftingTimeSeconds = 4.0f;

        FQuest_ResourceIngredient Stick1;
        Stick1.ResourceType = EQuest_ResourceType::Stick;
        Stick1.Quantity = 3;
        Campfire.Ingredients.Add(Stick1);

        AvailableRecipes.Add(Campfire);
    }

    // Recipe 3: Water Container — 1 Rock + 1 Leaf
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.ResultItem = EQuest_CraftedItem::WaterContainer;
        WaterContainer.DisplayName = TEXT("Water Container");
        WaterContainer.Description = TEXT("A hollowed rock lined with a broad leaf. Holds enough water for one day of travel.");
        WaterContainer.CraftingTimeSeconds = 6.0f;

        FQuest_ResourceIngredient Rock2;
        Rock2.ResourceType = EQuest_ResourceType::Rock;
        Rock2.Quantity = 1;
        WaterContainer.Ingredients.Add(Rock2);

        FQuest_ResourceIngredient Leaf1;
        Leaf1.ResourceType = EQuest_ResourceType::Leaf;
        Leaf1.Quantity = 1;
        WaterContainer.Ingredients.Add(Leaf1);

        AvailableRecipes.Add(WaterContainer);
    }

    // Recipe 4: Spear — 1 Stick + 1 Flint + 1 Sinew
    {
        FQuest_CraftingRecipe Spear;
        Spear.ResultItem = EQuest_CraftedItem::Spear;
        Spear.DisplayName = TEXT("Spear");
        Spear.Description = TEXT("A sharpened flint tip lashed to a long stick. Effective for hunting and keeping raptors at distance.");
        Spear.CraftingTimeSeconds = 8.0f;

        FQuest_ResourceIngredient Stick2;
        Stick2.ResourceType = EQuest_ResourceType::Stick;
        Stick2.Quantity = 1;
        Spear.Ingredients.Add(Stick2);

        FQuest_ResourceIngredient Flint1;
        Flint1.ResourceType = EQuest_ResourceType::Flint;
        Flint1.Quantity = 1;
        Spear.Ingredients.Add(Flint1);

        FQuest_ResourceIngredient Sinew2;
        Sinew2.ResourceType = EQuest_ResourceType::Sinew;
        Sinew2.Quantity = 1;
        Spear.Ingredients.Add(Sinew2);

        AvailableRecipes.Add(Spear);
    }
}

bool UQuest_CraftingComponent::CanCraft(EQuest_CraftedItem Item) const
{
    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
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

bool UQuest_CraftingComponent::StartCrafting(EQuest_CraftedItem Item)
{
    if (bIsCrafting) return false;
    if (!CanCraft(Item)) return false;

    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (!Recipe) return false;

    if (!ConsumeIngredients(*Recipe)) return false;

    CurrentlyCraftingItem = Item;
    CraftingTimer = 0.0f;
    CraftingDuration = Recipe->CraftingTimeSeconds;
    CraftingProgress = 0.0f;
    bIsCrafting = true;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
            FString::Printf(TEXT("Crafting: %s..."), *Recipe->DisplayName));
    }

    return true;
}

void UQuest_CraftingComponent::CancelCrafting()
{
    bIsCrafting = false;
    CraftingTimer = 0.0f;
    CraftingProgress = 0.0f;
    CurrentlyCraftingItem = EQuest_CraftedItem::None;
}

void UQuest_CraftingComponent::AddResource(EQuest_ResourceType Type, int32 Amount)
{
    switch (Type)
    {
        case EQuest_ResourceType::Rock:  Inventory.Rocks  += Amount; break;
        case EQuest_ResourceType::Stick: Inventory.Sticks += Amount; break;
        case EQuest_ResourceType::Leaf:  Inventory.Leaves += Amount; break;
        case EQuest_ResourceType::Sinew: Inventory.Sinew  += Amount; break;
        case EQuest_ResourceType::Flint: Inventory.Flint  += Amount; break;
        case EQuest_ResourceType::Bone:  Inventory.Bones  += Amount; break;
        default: break;
    }
}

int32 UQuest_CraftingComponent::GetResourceCount(EQuest_ResourceType Type) const
{
    switch (Type)
    {
        case EQuest_ResourceType::Rock:  return Inventory.Rocks;
        case EQuest_ResourceType::Stick: return Inventory.Sticks;
        case EQuest_ResourceType::Leaf:  return Inventory.Leaves;
        case EQuest_ResourceType::Sinew: return Inventory.Sinew;
        case EQuest_ResourceType::Flint: return Inventory.Flint;
        case EQuest_ResourceType::Bone:  return Inventory.Bones;
        default: return 0;
    }
}

void UQuest_CraftingComponent::ToggleCraftingMenu()
{
    bIsCraftingMenuOpen = !bIsCraftingMenuOpen;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan,
            bIsCraftingMenuOpen ? TEXT("Crafting Menu: OPEN") : TEXT("Crafting Menu: CLOSED"));
    }
}

TArray<FQuest_CraftingRecipe> UQuest_CraftingComponent::GetCraftableRecipes() const
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

void UQuest_CraftingComponent::CompleteCrafting()
{
    bIsCrafting = false;
    CraftingTimer = 0.0f;
    CraftingProgress = 1.0f;

    FString ItemName;
    switch (CurrentlyCraftingItem)
    {
        case EQuest_CraftedItem::StoneAxe:       ItemName = TEXT("Stone Axe");       break;
        case EQuest_CraftedItem::Campfire:        ItemName = TEXT("Campfire");        break;
        case EQuest_CraftedItem::WaterContainer:  ItemName = TEXT("Water Container"); break;
        case EQuest_CraftedItem::Spear:           ItemName = TEXT("Spear");           break;
        case EQuest_CraftedItem::Shelter:         ItemName = TEXT("Shelter");         break;
        default: ItemName = TEXT("Item"); break;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
            FString::Printf(TEXT("Crafted: %s!"), *ItemName));
    }

    CurrentlyCraftingItem = EQuest_CraftedItem::None;
}

bool UQuest_CraftingComponent::ConsumeIngredients(const FQuest_CraftingRecipe& Recipe)
{
    // Verify we still have everything (race condition guard)
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
        switch (Ingredient.ResourceType)
        {
            case EQuest_ResourceType::Rock:  Inventory.Rocks  -= Ingredient.Quantity; break;
            case EQuest_ResourceType::Stick: Inventory.Sticks -= Ingredient.Quantity; break;
            case EQuest_ResourceType::Leaf:  Inventory.Leaves -= Ingredient.Quantity; break;
            case EQuest_ResourceType::Sinew: Inventory.Sinew  -= Ingredient.Quantity; break;
            case EQuest_ResourceType::Flint: Inventory.Flint  -= Ingredient.Quantity; break;
            case EQuest_ResourceType::Bone:  Inventory.Bones  -= Ingredient.Quantity; break;
            default: break;
        }
    }
    return true;
}

const FQuest_CraftingRecipe* UQuest_CraftingComponent::FindRecipe(EQuest_CraftedItem Item) const
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
