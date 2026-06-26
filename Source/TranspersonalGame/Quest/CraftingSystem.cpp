// CraftingSystem.cpp — Agent #14 Quest & Mission Designer
// Crafting system: resource pickups, inventory management, recipe execution

#include "CraftingSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

// ============================================================
// AQuest_ResourcePickup Implementation
// ============================================================

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = true;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetSphereRadius(80.0f);
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    RootComponent = CollisionSphere;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    CollisionSphere->OnComponentBeginOverlap.AddDynamic(
        this, &AQuest_ResourcePickup::OnPlayerOverlap);
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
    bCollected = false;
    RespawnTimer = 0.0f;
}

void AQuest_ResourcePickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bCollected && bRespawns)
    {
        RespawnTimer += DeltaTime;
        if (RespawnTimer >= RespawnTimeSeconds)
        {
            ResetPickup();
        }
    }

    // Gentle bob animation when not collected
    if (!bCollected)
    {
        FVector CurrentLoc = GetActorLocation();
        float BobOffset = FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f) * 5.0f;
        SetActorLocation(FVector(CurrentLoc.X, CurrentLoc.Y, CurrentLoc.Z + BobOffset * DeltaTime));
    }
}

void AQuest_ResourcePickup::OnPlayerOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (bCollected) return;

    // Check if the overlapping actor is a character
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    // Find the crafting component on the player
    UQuest_CraftingManagerComponent* CraftingComp =
        PlayerChar->FindComponentByClass<UQuest_CraftingManagerComponent>();

    if (CraftingComp)
    {
        CraftingComp->AddResource(ResourceType, Quantity);
        CraftingComp->OnResourceCollected.Broadcast();

        // Log pickup
        FString ResourceName;
        switch (ResourceType)
        {
            case EQuest_ResourceType::Rock:  ResourceName = TEXT("Rock");  break;
            case EQuest_ResourceType::Stick: ResourceName = TEXT("Stick"); break;
            case EQuest_ResourceType::Leaf:  ResourceName = TEXT("Leaf");  break;
            case EQuest_ResourceType::Bone:  ResourceName = TEXT("Bone");  break;
            case EQuest_ResourceType::Flint: ResourceName = TEXT("Flint"); break;
            case EQuest_ResourceType::Vine:  ResourceName = TEXT("Vine");  break;
            default: ResourceName = TEXT("Unknown"); break;
        }

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
                FString::Printf(TEXT("Picked up: %s x%d"), *ResourceName, Quantity));
        }

        bCollected = true;
        RespawnTimer = 0.0f;

        // Hide mesh when collected
        MeshComponent->SetVisibility(false);
        CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void AQuest_ResourcePickup::ResetPickup()
{
    bCollected = false;
    RespawnTimer = 0.0f;
    MeshComponent->SetVisibility(true);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

// ============================================================
// UQuest_CraftingManagerComponent Implementation
// ============================================================

UQuest_CraftingManagerComponent::UQuest_CraftingManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UQuest_CraftingManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeRecipes();
}

void UQuest_CraftingManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsCrafting)
    {
        CraftingElapsed += DeltaTime;
        if (CraftingElapsed >= CraftingDuration)
        {
            CompleteCrafting();
        }
    }
}

void UQuest_CraftingManagerComponent::InitializeRecipes()
{
    Recipes.Empty();

    // ---- Recipe 1: Stone Axe (2 Rocks + 1 Stick) ----
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.OutputItem = EQuest_CraftedItem::StoneAxe;
        StoneAxe.RecipeName = TEXT("Stone Axe");
        StoneAxe.Description = TEXT("A crude but effective cutting tool. Useful for chopping wood and fighting off small predators.");
        StoneAxe.CraftingTimeSeconds = 3.0f;

        FQuest_CraftingIngredient Rock1;
        Rock1.ResourceType = EQuest_ResourceType::Rock;
        Rock1.Quantity = 2;
        StoneAxe.Ingredients.Add(Rock1);

        FQuest_CraftingIngredient Stick1;
        Stick1.ResourceType = EQuest_ResourceType::Stick;
        Stick1.Quantity = 1;
        StoneAxe.Ingredients.Add(Stick1);

        Recipes.Add(StoneAxe);
    }

    // ---- Recipe 2: Campfire (3 Sticks) ----
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.OutputItem = EQuest_CraftedItem::Campfire;
        Campfire.RecipeName = TEXT("Campfire");
        Campfire.Description = TEXT("A small fire for warmth, cooking, and keeping predators at bay during the night.");
        Campfire.CraftingTimeSeconds = 5.0f;

        FQuest_CraftingIngredient Stick2;
        Stick2.ResourceType = EQuest_ResourceType::Stick;
        Stick2.Quantity = 3;
        Campfire.Ingredients.Add(Stick2);

        Recipes.Add(Campfire);
    }

    // ---- Recipe 3: Water Container (1 Rock + 1 Leaf) ----
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.OutputItem = EQuest_CraftedItem::WaterContainer;
        WaterContainer.RecipeName = TEXT("Water Container");
        WaterContainer.Description = TEXT("A hollowed rock sealed with a large leaf. Holds enough water for one day of travel.");
        WaterContainer.CraftingTimeSeconds = 4.0f;

        FQuest_CraftingIngredient Rock2;
        Rock2.ResourceType = EQuest_ResourceType::Rock;
        Rock2.Quantity = 1;
        WaterContainer.Ingredients.Add(Rock2);

        FQuest_CraftingIngredient Leaf1;
        Leaf1.ResourceType = EQuest_ResourceType::Leaf;
        Leaf1.Quantity = 1;
        WaterContainer.Ingredients.Add(Leaf1);

        Recipes.Add(WaterContainer);
    }

    // ---- Recipe 4: Wooden Spear (2 Sticks + 1 Vine) ----
    {
        FQuest_CraftingRecipe Spear;
        Spear.OutputItem = EQuest_CraftedItem::WoodenSpear;
        Spear.RecipeName = TEXT("Wooden Spear");
        Spear.Description = TEXT("A sharpened stick bound with vine. Your first real weapon against the predators of this world.");
        Spear.CraftingTimeSeconds = 6.0f;

        FQuest_CraftingIngredient Stick3;
        Stick3.ResourceType = EQuest_ResourceType::Stick;
        Stick3.Quantity = 2;
        Spear.Ingredients.Add(Stick3);

        FQuest_CraftingIngredient Vine1;
        Vine1.ResourceType = EQuest_ResourceType::Vine;
        Vine1.Quantity = 1;
        Spear.Ingredients.Add(Vine1);

        Recipes.Add(Spear);
    }
}

void UQuest_CraftingManagerComponent::AddResource(EQuest_ResourceType Type, int32 Amount)
{
    switch (Type)
    {
        case EQuest_ResourceType::Rock:  Inventory.Rocks  += Amount; break;
        case EQuest_ResourceType::Stick: Inventory.Sticks += Amount; break;
        case EQuest_ResourceType::Leaf:  Inventory.Leaves += Amount; break;
        case EQuest_ResourceType::Bone:  Inventory.Bones  += Amount; break;
        case EQuest_ResourceType::Flint: Inventory.Flints += Amount; break;
        case EQuest_ResourceType::Vine:  Inventory.Vines  += Amount; break;
        default: break;
    }
}

int32 UQuest_CraftingManagerComponent::GetResourceCount(EQuest_ResourceType Type) const
{
    switch (Type)
    {
        case EQuest_ResourceType::Rock:  return Inventory.Rocks;
        case EQuest_ResourceType::Stick: return Inventory.Sticks;
        case EQuest_ResourceType::Leaf:  return Inventory.Leaves;
        case EQuest_ResourceType::Bone:  return Inventory.Bones;
        case EQuest_ResourceType::Flint: return Inventory.Flints;
        case EQuest_ResourceType::Vine:  return Inventory.Vines;
        default: return 0;
    }
}

bool UQuest_CraftingManagerComponent::HasIngredients(const FQuest_CraftingRecipe& Recipe) const
{
    for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

bool UQuest_CraftingManagerComponent::StartCrafting(EQuest_CraftedItem Item)
{
    if (bIsCrafting) return false;

    // Find the recipe
    const FQuest_CraftingRecipe* FoundRecipe = nullptr;
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.OutputItem == Item)
        {
            FoundRecipe = &Recipe;
            break;
        }
    }

    if (!FoundRecipe) return false;
    if (!HasIngredients(*FoundRecipe)) return false;

    // Start crafting
    ConsumeIngredients(*FoundRecipe);
    bIsCrafting = true;
    CraftingElapsed = 0.0f;
    CraftingDuration = FoundRecipe->CraftingTimeSeconds;
    CurrentlyCrafting = Item;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
            FString::Printf(TEXT("Crafting: %s..."), *FoundRecipe->RecipeName));
    }

    return true;
}

float UQuest_CraftingManagerComponent::GetCraftingProgress() const
{
    if (!bIsCrafting || CraftingDuration <= 0.0f) return 0.0f;
    return FMath::Clamp(CraftingElapsed / CraftingDuration, 0.0f, 1.0f);
}

TArray<FQuest_CraftingRecipe> UQuest_CraftingManagerComponent::GetAvailableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Available;
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (HasIngredients(Recipe))
        {
            Available.Add(Recipe);
        }
    }
    return Available;
}

void UQuest_CraftingManagerComponent::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    OnCraftingMenuToggled.Broadcast();

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White,
            bCraftingMenuOpen ? TEXT("[C] Crafting Menu OPEN") : TEXT("[C] Crafting Menu CLOSED"));
    }
}

void UQuest_CraftingManagerComponent::CompleteCrafting()
{
    bIsCrafting = false;
    CraftingElapsed = 0.0f;

    Inventory.CraftedItems.Add(CurrentlyCrafting);
    OnItemCrafted.Broadcast();

    FString ItemName;
    switch (CurrentlyCrafting)
    {
        case EQuest_CraftedItem::StoneAxe:       ItemName = TEXT("Stone Axe");       break;
        case EQuest_CraftedItem::Campfire:        ItemName = TEXT("Campfire");        break;
        case EQuest_CraftedItem::WaterContainer:  ItemName = TEXT("Water Container"); break;
        case EQuest_CraftedItem::WoodenSpear:     ItemName = TEXT("Wooden Spear");    break;
        case EQuest_CraftedItem::LeafShelter:     ItemName = TEXT("Leaf Shelter");    break;
        default: ItemName = TEXT("Item"); break;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green,
            FString::Printf(TEXT("Crafted: %s!"), *ItemName));
    }

    CurrentlyCrafting = EQuest_CraftedItem::None;
}

void UQuest_CraftingManagerComponent::ConsumeIngredients(const FQuest_CraftingRecipe& Recipe)
{
    for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
    {
        switch (Ingredient.ResourceType)
        {
            case EQuest_ResourceType::Rock:  Inventory.Rocks  -= Ingredient.Quantity; break;
            case EQuest_ResourceType::Stick: Inventory.Sticks -= Ingredient.Quantity; break;
            case EQuest_ResourceType::Leaf:  Inventory.Leaves -= Ingredient.Quantity; break;
            case EQuest_ResourceType::Bone:  Inventory.Bones  -= Ingredient.Quantity; break;
            case EQuest_ResourceType::Flint: Inventory.Flints -= Ingredient.Quantity; break;
            case EQuest_ResourceType::Vine:  Inventory.Vines  -= Ingredient.Quantity; break;
            default: break;
        }
    }
}
