#include "CraftingSystem.h"
#include "Engine/World.h"

// ============================================================
// UCraftingSystemComponent
// ============================================================

UCraftingSystemComponent::UCraftingSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCraftingSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeRecipes();
}

void UCraftingSystemComponent::InitializeRecipes()
{
    Recipes.Empty();

    // Recipe 1: Stone Axe — 2 Rocks + 1 Stick
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItemType::StoneAxe;
        Recipe.DisplayName = TEXT("Stone Axe");

        FQuest_ResourceRequirement RockReq;
        RockReq.ResourceType = EQuest_ResourceType::Rock;
        RockReq.Quantity = 2;

        FQuest_ResourceRequirement StickReq;
        StickReq.ResourceType = EQuest_ResourceType::Stick;
        StickReq.Quantity = 1;

        Recipe.Requirements.Add(RockReq);
        Recipe.Requirements.Add(StickReq);
        Recipes.Add(Recipe);
    }

    // Recipe 2: Campfire — 3 Sticks
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItemType::Campfire;
        Recipe.DisplayName = TEXT("Campfire");

        FQuest_ResourceRequirement StickReq;
        StickReq.ResourceType = EQuest_ResourceType::Stick;
        StickReq.Quantity = 3;

        Recipe.Requirements.Add(StickReq);
        Recipes.Add(Recipe);
    }

    // Recipe 3: Water Container — 1 Rock + 1 Leaf
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItemType::WaterContainer;
        Recipe.DisplayName = TEXT("Water Container");

        FQuest_ResourceRequirement RockReq;
        RockReq.ResourceType = EQuest_ResourceType::Rock;
        RockReq.Quantity = 1;

        FQuest_ResourceRequirement LeafReq;
        LeafReq.ResourceType = EQuest_ResourceType::Leaf;
        LeafReq.Quantity = 1;

        Recipe.Requirements.Add(RockReq);
        Recipe.Requirements.Add(LeafReq);
        Recipes.Add(Recipe);
    }

    // Recipe 4: Spear — 2 Sticks + 1 Rock
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItemType::Spear;
        Recipe.DisplayName = TEXT("Spear");

        FQuest_ResourceRequirement StickReq;
        StickReq.ResourceType = EQuest_ResourceType::Stick;
        StickReq.Quantity = 2;

        FQuest_ResourceRequirement RockReq;
        RockReq.ResourceType = EQuest_ResourceType::Rock;
        RockReq.Quantity = 1;

        Recipe.Requirements.Add(StickReq);
        Recipe.Requirements.Add(RockReq);
        Recipes.Add(Recipe);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Initialized %d recipes"), Recipes.Num());
}

bool UCraftingSystemComponent::CanCraft(EQuest_CraftedItemType ItemType) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.OutputItem == ItemType)
        {
            for (const FQuest_ResourceRequirement& Req : Recipe.Requirements)
            {
                if (GetResourceCount(Req.ResourceType) < Req.Quantity)
                {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

bool UCraftingSystemComponent::CraftItem(EQuest_CraftedItemType ItemType)
{
    if (!CanCraft(ItemType))
    {
        UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Cannot craft %d — insufficient resources"), (int32)ItemType);
        return false;
    }

    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.OutputItem == ItemType)
        {
            ConsumeResources(Recipe);
            UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Crafted %s"), *Recipe.DisplayName);
            return true;
        }
    }
    return false;
}

void UCraftingSystemComponent::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    switch (ResourceType)
    {
        case EQuest_ResourceType::Rock:  PlayerInventory.Rocks  += Amount; break;
        case EQuest_ResourceType::Stick: PlayerInventory.Sticks += Amount; break;
        case EQuest_ResourceType::Leaf:  PlayerInventory.Leaves += Amount; break;
        case EQuest_ResourceType::Bone:  PlayerInventory.Bones  += Amount; break;
        case EQuest_ResourceType::Hide:  PlayerInventory.Hides  += Amount; break;
        default: break;
    }
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Added %d of resource type %d"), Amount, (int32)ResourceType);
}

void UCraftingSystemComponent::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Menu %s"), bCraftingMenuOpen ? TEXT("OPEN") : TEXT("CLOSED"));
}

TArray<FQuest_CraftingRecipe> UCraftingSystemComponent::GetAvailableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Available;
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (CanCraft(Recipe.OutputItem))
        {
            Available.Add(Recipe);
        }
    }
    return Available;
}

int32 UCraftingSystemComponent::GetResourceCount(EQuest_ResourceType ResourceType) const
{
    switch (ResourceType)
    {
        case EQuest_ResourceType::Rock:  return PlayerInventory.Rocks;
        case EQuest_ResourceType::Stick: return PlayerInventory.Sticks;
        case EQuest_ResourceType::Leaf:  return PlayerInventory.Leaves;
        case EQuest_ResourceType::Bone:  return PlayerInventory.Bones;
        case EQuest_ResourceType::Hide:  return PlayerInventory.Hides;
        default: return 0;
    }
}

bool UCraftingSystemComponent::ConsumeResources(const FQuest_CraftingRecipe& Recipe)
{
    for (const FQuest_ResourceRequirement& Req : Recipe.Requirements)
    {
        switch (Req.ResourceType)
        {
            case EQuest_ResourceType::Rock:  PlayerInventory.Rocks  -= Req.Quantity; break;
            case EQuest_ResourceType::Stick: PlayerInventory.Sticks -= Req.Quantity; break;
            case EQuest_ResourceType::Leaf:  PlayerInventory.Leaves -= Req.Quantity; break;
            case EQuest_ResourceType::Bone:  PlayerInventory.Bones  -= Req.Quantity; break;
            case EQuest_ResourceType::Hide:  PlayerInventory.Hides  -= Req.Quantity; break;
            default: break;
        }
    }
    return true;
}

// ============================================================
// UResourcePickupComponent
// ============================================================

UResourcePickupComponent::UResourcePickupComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UResourcePickupComponent::PickUp(UCraftingSystemComponent* CraftingComp)
{
    if (bHasBeenPickedUp || !CraftingComp)
    {
        return;
    }

    CraftingComp->AddResource(ResourceType, ResourceAmount);
    bHasBeenPickedUp = true;

    // Hide the actor after pickup
    AActor* Owner = GetOwner();
    if (Owner)
    {
        Owner->SetActorHiddenInGame(true);
        Owner->SetActorEnableCollision(false);
        UE_LOG(LogTemp, Log, TEXT("ResourcePickup: Picked up %d of type %d from %s"),
            ResourceAmount, (int32)ResourceType, *Owner->GetActorLabel());
    }
}
