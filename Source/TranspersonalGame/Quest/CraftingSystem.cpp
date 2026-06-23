#include "CraftingSystem.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"

// ─────────────────────────────────────────────────────────────────────────────
// AQuest_ResourcePickup
// ─────────────────────────────────────────────────────────────────────────────

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionProfileName(TEXT("OverlapAll"));
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
}

void AQuest_ResourcePickup::PickUp(AActor* PickingActor)
{
    if (bHasBeenPickedUp || !PickingActor)
    {
        return;
    }

    // Try to add resource to the picking actor's crafting component
    UQuest_CraftingComponent* CraftComp = PickingActor->FindComponentByClass<UQuest_CraftingComponent>();
    if (CraftComp)
    {
        CraftComp->AddResource(ResourceType, QuantityOnPickup);
        bHasBeenPickedUp = true;

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
                FString::Printf(TEXT("Picked up: %s x%d"), *GetResourceDisplayName(), QuantityOnPickup));
        }

        // Hide and schedule destroy
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
        SetLifeSpan(1.0f);
    }
}

FString AQuest_ResourcePickup::GetResourceDisplayName() const
{
    switch (ResourceType)
    {
        case EQuest_ResourceType::Flint:  return TEXT("Flint Stone");
        case EQuest_ResourceType::Stick:  return TEXT("Dry Stick");
        case EQuest_ResourceType::Leaf:   return TEXT("Large Leaf");
        case EQuest_ResourceType::Bone:   return TEXT("Animal Bone");
        case EQuest_ResourceType::Hide:   return TEXT("Animal Hide");
        case EQuest_ResourceType::Vine:   return TEXT("Vine");
        default:                          return TEXT("Unknown Resource");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// UQuest_CraftingComponent
// ─────────────────────────────────────────────────────────────────────────────

UQuest_CraftingComponent::UQuest_CraftingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UQuest_CraftingComponent::BeginPlay()
{
    Super::BeginPlay();
    InitialiseRecipes();
}

void UQuest_CraftingComponent::InitialiseRecipes()
{
    AvailableRecipes.Empty();

    // ── Recipe 1: Stone Axe (2 Flint + 1 Stick) ──────────────────────────────
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.OutputItem = EQuest_CraftedItemType::StoneAxe;
        StoneAxe.DisplayName = TEXT("Stone Axe");
        StoneAxe.CraftTimeSeconds = 5.0f;

        FQuest_CraftingIngredient Flint1;
        Flint1.ResourceType = EQuest_ResourceType::Flint;
        Flint1.Quantity = 2;
        StoneAxe.Ingredients.Add(Flint1);

        FQuest_CraftingIngredient Stick1;
        Stick1.ResourceType = EQuest_ResourceType::Stick;
        Stick1.Quantity = 1;
        StoneAxe.Ingredients.Add(Stick1);

        AvailableRecipes.Add(StoneAxe);
    }

    // ── Recipe 2: Campfire (3 Sticks) ────────────────────────────────────────
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.OutputItem = EQuest_CraftedItemType::Campfire;
        Campfire.DisplayName = TEXT("Campfire");
        Campfire.CraftTimeSeconds = 8.0f;

        FQuest_CraftingIngredient Sticks;
        Sticks.ResourceType = EQuest_ResourceType::Stick;
        Sticks.Quantity = 3;
        Campfire.Ingredients.Add(Sticks);

        AvailableRecipes.Add(Campfire);
    }

    // ── Recipe 3: Water Container (1 Flint + 1 Leaf) ─────────────────────────
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.OutputItem = EQuest_CraftedItemType::WaterContainer;
        WaterContainer.DisplayName = TEXT("Water Container");
        WaterContainer.CraftTimeSeconds = 4.0f;

        FQuest_CraftingIngredient Flint2;
        Flint2.ResourceType = EQuest_ResourceType::Flint;
        Flint2.Quantity = 1;
        WaterContainer.Ingredients.Add(Flint2);

        FQuest_CraftingIngredient Leaf;
        Leaf.ResourceType = EQuest_ResourceType::Leaf;
        Leaf.Quantity = 1;
        WaterContainer.Ingredients.Add(Leaf);

        AvailableRecipes.Add(WaterContainer);
    }

    // ── Recipe 4: Spear (1 Flint + 2 Sticks + 1 Vine) ───────────────────────
    {
        FQuest_CraftingRecipe Spear;
        Spear.OutputItem = EQuest_CraftedItemType::Spear;
        Spear.DisplayName = TEXT("Hunting Spear");
        Spear.CraftTimeSeconds = 10.0f;

        FQuest_CraftingIngredient FlintTip;
        FlintTip.ResourceType = EQuest_ResourceType::Flint;
        FlintTip.Quantity = 1;
        Spear.Ingredients.Add(FlintTip);

        FQuest_CraftingIngredient ShaftSticks;
        ShaftSticks.ResourceType = EQuest_ResourceType::Stick;
        ShaftSticks.Quantity = 2;
        Spear.Ingredients.Add(ShaftSticks);

        FQuest_CraftingIngredient Binding;
        Binding.ResourceType = EQuest_ResourceType::Vine;
        Binding.Quantity = 1;
        Spear.Ingredients.Add(Binding);

        AvailableRecipes.Add(Spear);
    }
}

bool UQuest_CraftingComponent::CanCraft(EQuest_CraftedItemType ItemType) const
{
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.OutputItem != ItemType)
        {
            continue;
        }

        for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
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

bool UQuest_CraftingComponent::CraftItem(EQuest_CraftedItemType ItemType)
{
    if (!CanCraft(ItemType))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
                TEXT("Not enough materials to craft!"));
        }
        return false;
    }

    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.OutputItem == ItemType)
        {
            ConsumeIngredients(Recipe);

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green,
                    FString::Printf(TEXT("Crafted: %s"), *Recipe.DisplayName));
            }
            return true;
        }
    }
    return false;
}

void UQuest_CraftingComponent::AddResource(EQuest_ResourceType ResourceType, int32 Quantity)
{
    switch (ResourceType)
    {
        case EQuest_ResourceType::Flint: Inventory.FlintCount += Quantity; break;
        case EQuest_ResourceType::Stick: Inventory.StickCount += Quantity; break;
        case EQuest_ResourceType::Leaf:  Inventory.LeafCount  += Quantity; break;
        case EQuest_ResourceType::Bone:  Inventory.BoneCount  += Quantity; break;
        case EQuest_ResourceType::Hide:  Inventory.HideCount  += Quantity; break;
        case EQuest_ResourceType::Vine:  Inventory.VineCount  += Quantity; break;
        default: break;
    }
}

int32 UQuest_CraftingComponent::GetResourceCount(EQuest_ResourceType ResourceType) const
{
    switch (ResourceType)
    {
        case EQuest_ResourceType::Flint: return Inventory.FlintCount;
        case EQuest_ResourceType::Stick: return Inventory.StickCount;
        case EQuest_ResourceType::Leaf:  return Inventory.LeafCount;
        case EQuest_ResourceType::Bone:  return Inventory.BoneCount;
        case EQuest_ResourceType::Hide:  return Inventory.HideCount;
        case EQuest_ResourceType::Vine:  return Inventory.VineCount;
        default:                         return 0;
    }
}

void UQuest_CraftingComponent::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White,
            bCraftingMenuOpen ? TEXT("[C] Crafting Menu OPEN") : TEXT("[C] Crafting Menu CLOSED"));
    }
}

TArray<FQuest_CraftingRecipe> UQuest_CraftingComponent::GetCraftableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Craftable;
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (CanCraft(Recipe.OutputItem))
        {
            Craftable.Add(Recipe);
        }
    }
    return Craftable;
}

void UQuest_CraftingComponent::ConsumeIngredients(const FQuest_CraftingRecipe& Recipe)
{
    for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
    {
        AddResource(Ingredient.ResourceType, -Ingredient.Quantity);
    }
}
