// CraftingSystem.cpp
// Agent #14 — Quest & Mission Designer
// Full implementation: 3 recipes, resource pickup, crafting tick

#include "Quest/CraftingSystem.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─── AQuest_ResourcePickup ──────────────────────────────────────────────────

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

    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_ResourcePickup::OnPlayerOverlap);
}

void AQuest_ResourcePickup::OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || bHasBeenPickedUp) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    PickUp(OtherActor);
}

void AQuest_ResourcePickup::PickUp(AActor* Collector)
{
    if (bHasBeenPickedUp) return;
    bHasBeenPickedUp = true;

    // Find crafting manager in world and add resource
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AQuest_CraftingManager::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        AQuest_CraftingManager* Manager = Cast<AQuest_CraftingManager>(FoundActors[0]);
        if (Manager)
        {
            Manager->AddResource(ResourceType, Quantity);
            UE_LOG(LogTemp, Log, TEXT("ResourcePickup: Added %d of type %d to inventory"),
                Quantity, (int32)ResourceType);
        }
    }

    // Hide and destroy after short delay
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    FTimerHandle DestroyTimer;
    GetWorldTimerManager().SetTimer(DestroyTimer, [this]()
    {
        Destroy();
    }, 1.0f, false);
}

// ─── AQuest_CraftingManager ─────────────────────────────────────────────────

AQuest_CraftingManager::AQuest_CraftingManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AQuest_CraftingManager::BeginPlay()
{
    Super::BeginPlay();
    RegisterDefaultRecipes();
    UE_LOG(LogTemp, Log, TEXT("CraftingManager: Initialized with %d recipes"), Recipes.Num());
}

void AQuest_CraftingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsCrafting)
    {
        CraftingTimer += DeltaTime;
        CraftingProgress = FMath::Clamp(CraftingTimer / ActiveRecipe.CraftTimeSeconds, 0.0f, 1.0f);

        if (CraftingTimer >= ActiveRecipe.CraftTimeSeconds)
        {
            CompleteCrafting();
        }
    }
}

void AQuest_CraftingManager::RegisterDefaultRecipes()
{
    Recipes.Empty();

    // ── Recipe 1: Stone Axe (2 Rocks + 1 Stick) ─────────────────────────
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.ResultItem = EQuest_CraftedItem::StoneAxe;
        StoneAxe.DisplayName = TEXT("Stone Axe");
        StoneAxe.CraftTimeSeconds = 3.0f;
        StoneAxe.Description = TEXT("A crude but effective axe. Useful for chopping wood and fighting predators.");

        FQuest_RecipeIngredient Rock1;
        Rock1.ResourceType = EQuest_ResourceType::Rock;
        Rock1.Quantity = 2;
        StoneAxe.Ingredients.Add(Rock1);

        FQuest_RecipeIngredient Stick1;
        Stick1.ResourceType = EQuest_ResourceType::Stick;
        Stick1.Quantity = 1;
        StoneAxe.Ingredients.Add(Stick1);

        Recipes.Add(StoneAxe);
    }

    // ── Recipe 2: Campfire (3 Sticks) ────────────────────────────────────
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.ResultItem = EQuest_CraftedItem::Campfire;
        Campfire.DisplayName = TEXT("Campfire");
        Campfire.CraftTimeSeconds = 5.0f;
        Campfire.Description = TEXT("A fire that keeps predators away and cooks meat. Essential for survival.");

        FQuest_RecipeIngredient Stick2;
        Stick2.ResourceType = EQuest_ResourceType::Stick;
        Stick2.Quantity = 3;
        Campfire.Ingredients.Add(Stick2);

        Recipes.Add(Campfire);
    }

    // ── Recipe 3: Water Container (1 Rock + 1 Leaf) ──────────────────────
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.ResultItem = EQuest_CraftedItem::WaterContainer;
        WaterContainer.DisplayName = TEXT("Water Container");
        WaterContainer.CraftTimeSeconds = 4.0f;
        WaterContainer.Description = TEXT("A hollowed rock lined with large leaves. Holds water for drinking.");

        FQuest_RecipeIngredient Rock2;
        Rock2.ResourceType = EQuest_ResourceType::Rock;
        Rock2.Quantity = 1;
        WaterContainer.Ingredients.Add(Rock2);

        FQuest_RecipeIngredient Leaf1;
        Leaf1.ResourceType = EQuest_ResourceType::Leaf;
        Leaf1.Quantity = 1;
        WaterContainer.Ingredients.Add(Leaf1);

        Recipes.Add(WaterContainer);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingManager: Registered %d recipes"), Recipes.Num());
}

bool AQuest_CraftingManager::CanCraft(EQuest_CraftedItem Item) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.ResultItem != Item) continue;

        for (const FQuest_RecipeIngredient& Ingredient : Recipe.Ingredients)
        {
            if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool AQuest_CraftingManager::StartCrafting(EQuest_CraftedItem Item)
{
    if (bIsCrafting) return false;
    if (!CanCraft(Item)) return false;

    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.ResultItem == Item)
        {
            ActiveRecipe = Recipe;

            // Consume ingredients
            for (const FQuest_RecipeIngredient& Ingredient : Recipe.Ingredients)
            {
                AddResource(Ingredient.ResourceType, -Ingredient.Quantity);
            }

            bIsCrafting = true;
            CraftingTimer = 0.0f;
            CraftingProgress = 0.0f;
            CurrentlyCrafting = Item;

            UE_LOG(LogTemp, Log, TEXT("CraftingManager: Started crafting %s (%.1fs)"),
                *Recipe.DisplayName, Recipe.CraftTimeSeconds);
            return true;
        }
    }
    return false;
}

void AQuest_CraftingManager::CompleteCrafting()
{
    bIsCrafting = false;
    CraftingProgress = 1.0f;

    UE_LOG(LogTemp, Log, TEXT("CraftingManager: Completed crafting %s!"),
        *ActiveRecipe.DisplayName);

    CurrentlyCrafting = EQuest_CraftedItem::None;
    CraftingTimer = 0.0f;
    CraftingProgress = 0.0f;
}

void AQuest_CraftingManager::CancelCrafting()
{
    if (!bIsCrafting) return;

    // Refund ingredients
    for (const FQuest_RecipeIngredient& Ingredient : ActiveRecipe.Ingredients)
    {
        AddResource(Ingredient.ResourceType, Ingredient.Quantity);
    }

    bIsCrafting = false;
    CraftingTimer = 0.0f;
    CraftingProgress = 0.0f;
    CurrentlyCrafting = EQuest_CraftedItem::None;

    UE_LOG(LogTemp, Log, TEXT("CraftingManager: Crafting cancelled, ingredients refunded."));
}

void AQuest_CraftingManager::AddResource(EQuest_ResourceType Type, int32 Amount)
{
    switch (Type)
    {
    case EQuest_ResourceType::Rock:   PlayerInventory.Rocks  = FMath::Max(0, PlayerInventory.Rocks  + Amount); break;
    case EQuest_ResourceType::Stick:  PlayerInventory.Sticks = FMath::Max(0, PlayerInventory.Sticks + Amount); break;
    case EQuest_ResourceType::Leaf:   PlayerInventory.Leaves = FMath::Max(0, PlayerInventory.Leaves + Amount); break;
    case EQuest_ResourceType::Bone:   PlayerInventory.Bones  = FMath::Max(0, PlayerInventory.Bones  + Amount); break;
    case EQuest_ResourceType::Hide:   PlayerInventory.Hides  = FMath::Max(0, PlayerInventory.Hides  + Amount); break;
    case EQuest_ResourceType::Flint:  PlayerInventory.Flints = FMath::Max(0, PlayerInventory.Flints + Amount); break;
    default: break;
    }
}

int32 AQuest_CraftingManager::GetResourceCount(EQuest_ResourceType Type) const
{
    switch (Type)
    {
    case EQuest_ResourceType::Rock:  return PlayerInventory.Rocks;
    case EQuest_ResourceType::Stick: return PlayerInventory.Sticks;
    case EQuest_ResourceType::Leaf:  return PlayerInventory.Leaves;
    case EQuest_ResourceType::Bone:  return PlayerInventory.Bones;
    case EQuest_ResourceType::Hide:  return PlayerInventory.Hides;
    case EQuest_ResourceType::Flint: return PlayerInventory.Flints;
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
