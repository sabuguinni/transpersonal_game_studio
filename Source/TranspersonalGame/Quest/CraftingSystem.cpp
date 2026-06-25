// CraftingSystem.cpp
// Agent #14 — Quest & Mission Designer
// Implements resource pickup actors and crafting component
// Recipes: Stone Axe (2 rocks + 1 stick), Campfire (3 sticks), Water Container (1 rock + 1 leaf), Spear (1 flint + 2 sticks)

#include "CraftingSystem.h"
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

    // Load a basic sphere mesh as placeholder — replaced by real assets later
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        MeshComponent->SetStaticMesh(SphereMesh.Object);
    }

    MeshComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    bIsAvailable = true;
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
    // Scale visual indicator based on resource type
    switch (ResourceType)
    {
        case EQuest_ResourceType::Rock:
        case EQuest_ResourceType::Flint:
            SetActorScale3D(FVector(0.4f, 0.4f, 0.4f));
            break;
        case EQuest_ResourceType::Stick:
        case EQuest_ResourceType::Bone:
            SetActorScale3D(FVector(0.2f, 0.2f, 0.8f));
            break;
        case EQuest_ResourceType::Leaf:
        case EQuest_ResourceType::Hide:
            SetActorScale3D(FVector(0.5f, 0.5f, 0.1f));
            break;
        default:
            break;
    }
}

void AQuest_ResourcePickup::OnPlayerPickup(AActor* PlayerActor)
{
    if (!bIsAvailable || !PlayerActor)
    {
        return;
    }

    // Find crafting component on the player
    UQuest_CraftingComponent* CraftComp =
        PlayerActor->FindComponentByClass<UQuest_CraftingComponent>();

    if (CraftComp)
    {
        CraftComp->AddResource(ResourceType, QuantityOnPickup);
        bIsAvailable = false;
        // Hide the actor — respawn logic handled by game mode
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
        UE_LOG(LogTemp, Log, TEXT("ResourcePickup: Player collected %s x%d"),
               *UEnum::GetValueAsString(ResourceType), QuantityOnPickup);
    }
}

// ============================================================
// UQuest_CraftingComponent
// ============================================================

UQuest_CraftingComponent::UQuest_CraftingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UQuest_CraftingComponent::BeginPlay()
{
    Super::BeginPlay();
    InitialiseDefaultRecipes();
}

void UQuest_CraftingComponent::InitialiseDefaultRecipes()
{
    Recipes.Empty();

    // ── Recipe 1: Stone Axe ──────────────────────────────────
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.ResultItem     = EQuest_CraftedItemType::StoneAxe;
        StoneAxe.DisplayName    = TEXT("Stone Axe");
        StoneAxe.CraftTimeSeconds = 3.0f;

        FQuest_ResourceStack R1;
        R1.ResourceType = EQuest_ResourceType::Rock;
        R1.Quantity     = 2;
        StoneAxe.Ingredients.Add(R1);

        FQuest_ResourceStack R2;
        R2.ResourceType = EQuest_ResourceType::Stick;
        R2.Quantity     = 1;
        StoneAxe.Ingredients.Add(R2);

        Recipes.Add(StoneAxe);
    }

    // ── Recipe 2: Campfire ───────────────────────────────────
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.ResultItem     = EQuest_CraftedItemType::Campfire;
        Campfire.DisplayName    = TEXT("Campfire");
        Campfire.CraftTimeSeconds = 4.0f;

        FQuest_ResourceStack R1;
        R1.ResourceType = EQuest_ResourceType::Stick;
        R1.Quantity     = 3;
        Campfire.Ingredients.Add(R1);

        Recipes.Add(Campfire);
    }

    // ── Recipe 3: Water Container ────────────────────────────
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.ResultItem     = EQuest_CraftedItemType::WaterContainer;
        WaterContainer.DisplayName    = TEXT("Water Container");
        WaterContainer.CraftTimeSeconds = 2.5f;

        FQuest_ResourceStack R1;
        R1.ResourceType = EQuest_ResourceType::Rock;
        R1.Quantity     = 1;
        WaterContainer.Ingredients.Add(R1);

        FQuest_ResourceStack R2;
        R2.ResourceType = EQuest_ResourceType::Leaf;
        R2.Quantity     = 1;
        WaterContainer.Ingredients.Add(R2);

        Recipes.Add(WaterContainer);
    }

    // ── Recipe 4: Spear ──────────────────────────────────────
    {
        FQuest_CraftingRecipe Spear;
        Spear.ResultItem     = EQuest_CraftedItemType::Spear;
        Spear.DisplayName    = TEXT("Spear");
        Spear.CraftTimeSeconds = 5.0f;

        FQuest_ResourceStack R1;
        R1.ResourceType = EQuest_ResourceType::Flint;
        R1.Quantity     = 1;
        Spear.Ingredients.Add(R1);

        FQuest_ResourceStack R2;
        R2.ResourceType = EQuest_ResourceType::Stick;
        R2.Quantity     = 2;
        Spear.Ingredients.Add(R2);

        Recipes.Add(Spear);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: %d recipes initialised"), Recipes.Num());
}

void UQuest_CraftingComponent::AddResource(EQuest_ResourceType Type, int32 Quantity)
{
    for (FQuest_ResourceStack& Stack : Inventory)
    {
        if (Stack.ResourceType == Type)
        {
            Stack.Quantity += Quantity;
            UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Added %d %s (total: %d)"),
                   Quantity, *UEnum::GetValueAsString(Type), Stack.Quantity);
            return;
        }
    }
    // New resource type — add fresh stack
    FQuest_ResourceStack NewStack;
    NewStack.ResourceType = Type;
    NewStack.Quantity     = Quantity;
    Inventory.Add(NewStack);
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: New resource %s x%d"),
           *UEnum::GetValueAsString(Type), Quantity);
}

int32 UQuest_CraftingComponent::GetResourceCount(EQuest_ResourceType Type) const
{
    for (const FQuest_ResourceStack& Stack : Inventory)
    {
        if (Stack.ResourceType == Type)
        {
            return Stack.Quantity;
        }
    }
    return 0;
}

bool UQuest_CraftingComponent::HasIngredients(const FQuest_CraftingRecipe& Recipe) const
{
    for (const FQuest_ResourceStack& Ingredient : Recipe.Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

void UQuest_CraftingComponent::ConsumeIngredients(const FQuest_CraftingRecipe& Recipe)
{
    for (const FQuest_ResourceStack& Ingredient : Recipe.Ingredients)
    {
        for (FQuest_ResourceStack& Stack : Inventory)
        {
            if (Stack.ResourceType == Ingredient.ResourceType)
            {
                Stack.Quantity -= Ingredient.Quantity;
                break;
            }
        }
    }
}

bool UQuest_CraftingComponent::CraftItem(EQuest_CraftedItemType ItemType)
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.ResultItem != ItemType)
        {
            continue;
        }
        if (!HasIngredients(Recipe))
        {
            UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Not enough resources for %s"),
                   *Recipe.DisplayName);
            return false;
        }
        ConsumeIngredients(Recipe);
        UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Crafted %s successfully!"),
               *Recipe.DisplayName);
        return true;
    }
    UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Recipe not found for item %d"),
           static_cast<int32>(ItemType));
    return false;
}

TArray<FQuest_CraftingRecipe> UQuest_CraftingComponent::GetAvailableRecipes() const
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
