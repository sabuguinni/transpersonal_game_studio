// CraftingSystem.cpp
// Agent #14 — Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260627_007

#include "CraftingSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// ─── AQuest_ResourcePickup ────────────────────────────────────────────────────

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));

    PickupRadius = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRadius"));
    PickupRadius->SetupAttachment(RootComponent);
    PickupRadius->SetSphereRadius(120.0f);
    PickupRadius->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
}

bool AQuest_ResourcePickup::TryCollect(AActor* Collector)
{
    if (bIsCollected || !Collector)
    {
        return false;
    }

    // Mark as collected and hide
    bIsCollected = true;
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    UE_LOG(LogTemp, Log, TEXT("ResourcePickup collected: %s x%d by %s"),
        *GetResourceLabel(), Quantity, *Collector->GetName());

    return true;
}

FString AQuest_ResourcePickup::GetResourceLabel() const
{
    switch (ResourceType)
    {
        case EQuest_ResourceType::Rock:  return TEXT("Rock");
        case EQuest_ResourceType::Stick: return TEXT("Stick");
        case EQuest_ResourceType::Leaf:  return TEXT("Leaf");
        case EQuest_ResourceType::Bone:  return TEXT("Bone");
        case EQuest_ResourceType::Hide:  return TEXT("Hide");
        default:                         return TEXT("Unknown");
    }
}

// ─── UQuest_CraftingManagerComponent ─────────────────────────────────────────

UQuest_CraftingManagerComponent::UQuest_CraftingManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Initialise empty inventory
    ResourceInventory.Add(EQuest_ResourceType::Rock,  0);
    ResourceInventory.Add(EQuest_ResourceType::Stick, 0);
    ResourceInventory.Add(EQuest_ResourceType::Leaf,  0);
    ResourceInventory.Add(EQuest_ResourceType::Bone,  0);
    ResourceInventory.Add(EQuest_ResourceType::Hide,  0);
}

void UQuest_CraftingManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    InitialiseDefaultRecipes();
    UE_LOG(LogTemp, Log, TEXT("CraftingManagerComponent initialised with %d recipes"), Recipes.Num());
}

void UQuest_CraftingManagerComponent::InitialiseDefaultRecipes()
{
    Recipes.Empty();

    // Recipe 1: Stone Axe — 2 Rocks + 1 Stick
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.Result = EQuest_CraftedItem::StoneAxe;
        StoneAxe.RecipeName = TEXT("Stone Axe");
        StoneAxe.CraftTimeSeconds = 5.0f;

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

    // Recipe 2: Campfire — 3 Sticks
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.Result = EQuest_CraftedItem::Campfire;
        Campfire.RecipeName = TEXT("Campfire");
        Campfire.CraftTimeSeconds = 8.0f;

        FQuest_ResourceIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity = 3;
        Campfire.Ingredients.Add(StickIngredient);

        Recipes.Add(Campfire);
    }

    // Recipe 3: Water Container — 1 Rock + 1 Leaf
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.Result = EQuest_CraftedItem::WaterContainer;
        WaterContainer.RecipeName = TEXT("Water Container");
        WaterContainer.CraftTimeSeconds = 4.0f;

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

    // Recipe 4: Spear — 1 Stick + 1 Rock
    {
        FQuest_CraftingRecipe Spear;
        Spear.Result = EQuest_CraftedItem::Spear;
        Spear.RecipeName = TEXT("Spear");
        Spear.CraftTimeSeconds = 6.0f;

        FQuest_ResourceIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity = 1;
        Spear.Ingredients.Add(StickIngredient);

        FQuest_ResourceIngredient RockIngredient;
        RockIngredient.ResourceType = EQuest_ResourceType::Rock;
        RockIngredient.Quantity = 1;
        Spear.Ingredients.Add(RockIngredient);

        Recipes.Add(Spear);
    }

    // Recipe 5: Leaf Bandage — 2 Leaves
    {
        FQuest_CraftingRecipe Bandage;
        Bandage.Result = EQuest_CraftedItem::LeafBandage;
        Bandage.RecipeName = TEXT("Leaf Bandage");
        Bandage.CraftTimeSeconds = 2.0f;

        FQuest_ResourceIngredient LeafIngredient;
        LeafIngredient.ResourceType = EQuest_ResourceType::Leaf;
        LeafIngredient.Quantity = 2;
        Bandage.Ingredients.Add(LeafIngredient);

        Recipes.Add(Bandage);
    }
}

void UQuest_CraftingManagerComponent::AddResource(EQuest_ResourceType Type, int32 Amount)
{
    if (Type == EQuest_ResourceType::None || Amount <= 0)
    {
        return;
    }

    if (ResourceInventory.Contains(Type))
    {
        ResourceInventory[Type] += Amount;
    }
    else
    {
        ResourceInventory.Add(Type, Amount);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingManager: Added %d x %d. New total: %d"),
        (int32)Type, Amount, ResourceInventory[Type]);
}

bool UQuest_CraftingManagerComponent::CanCraft(EQuest_CraftedItem Item) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.Result == Item)
        {
            for (const FQuest_ResourceIngredient& Ingredient : Recipe.Ingredients)
            {
                int32 Available = GetResourceCount(Ingredient.ResourceType);
                if (Available < Ingredient.Quantity)
                {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

bool UQuest_CraftingManagerComponent::TryCraft(EQuest_CraftedItem Item)
{
    if (!CanCraft(Item))
    {
        UE_LOG(LogTemp, Warning, TEXT("CraftingManager: Cannot craft %d — insufficient resources"), (int32)Item);
        return false;
    }

    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.Result == Item)
        {
            ConsumeIngredients(Recipe);
            CraftedItems.Add(Item);
            UE_LOG(LogTemp, Log, TEXT("CraftingManager: Crafted %s successfully"), *Recipe.RecipeName);
            return true;
        }
    }

    return false;
}

int32 UQuest_CraftingManagerComponent::GetResourceCount(EQuest_ResourceType Type) const
{
    const int32* Count = ResourceInventory.Find(Type);
    return Count ? *Count : 0;
}

TArray<FQuest_CraftingRecipe> UQuest_CraftingManagerComponent::GetAvailableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Available;
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (CanCraft(Recipe.Result))
        {
            Available.Add(Recipe);
        }
    }
    return Available;
}

void UQuest_CraftingManagerComponent::ConsumeIngredients(const FQuest_CraftingRecipe& Recipe)
{
    for (const FQuest_ResourceIngredient& Ingredient : Recipe.Ingredients)
    {
        if (ResourceInventory.Contains(Ingredient.ResourceType))
        {
            ResourceInventory[Ingredient.ResourceType] -= Ingredient.Quantity;
            if (ResourceInventory[Ingredient.ResourceType] < 0)
            {
                ResourceInventory[Ingredient.ResourceType] = 0;
            }
        }
    }
}
