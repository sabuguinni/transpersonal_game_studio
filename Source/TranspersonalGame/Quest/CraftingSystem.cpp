#include "CraftingSystem.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Engine/World.h"

// ============================================================
// UQuest_CraftingManagerComponent
// ============================================================

UQuest_CraftingManagerComponent::UQuest_CraftingManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bCraftingMenuOpen = false;
    LastCraftedItem = EQuest_CraftedItem::None;
    MaxInventorySlots = 20;
}

void UQuest_CraftingManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    RegisterDefaultRecipes();
}

void UQuest_CraftingManagerComponent::InitializeDefaultRecipes()
{
    RegisterDefaultRecipes();
}

void UQuest_CraftingManagerComponent::RegisterDefaultRecipes()
{
    KnownRecipes.Empty();

    // --- Recipe 1: Stone Axe (2 Stones + 1 Stick) ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::StoneAxe;
        Recipe.DisplayName = TEXT("Stone Axe");
        Recipe.CraftingTimeSeconds = 3.0f;

        FQuest_CraftingIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Stone;
        Ing1.Quantity = 2;
        Recipe.Ingredients.Add(Ing1);

        FQuest_CraftingIngredient Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Stick;
        Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing2);

        KnownRecipes.Add(Recipe);
    }

    // --- Recipe 2: Campfire (3 Sticks + 1 Stone) ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::Campfire;
        Recipe.DisplayName = TEXT("Campfire");
        Recipe.CraftingTimeSeconds = 5.0f;

        FQuest_CraftingIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Stick;
        Ing1.Quantity = 3;
        Recipe.Ingredients.Add(Ing1);

        FQuest_CraftingIngredient Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Stone;
        Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing2);

        KnownRecipes.Add(Recipe);
    }

    // --- Recipe 3: Water Container (1 Stone + 1 Leaf) ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::WaterContainer;
        Recipe.DisplayName = TEXT("Water Container");
        Recipe.CraftingTimeSeconds = 4.0f;

        FQuest_CraftingIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Stone;
        Ing1.Quantity = 1;
        Recipe.Ingredients.Add(Ing1);

        FQuest_CraftingIngredient Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Leaf;
        Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing2);

        KnownRecipes.Add(Recipe);
    }

    // --- Recipe 4: Spear (2 Sticks + 1 Flint) ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::Spear;
        Recipe.DisplayName = TEXT("Spear");
        Recipe.CraftingTimeSeconds = 6.0f;

        FQuest_CraftingIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Stick;
        Ing1.Quantity = 2;
        Recipe.Ingredients.Add(Ing1);

        FQuest_CraftingIngredient Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Flint;
        Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing2);

        KnownRecipes.Add(Recipe);
    }

    // --- Recipe 5: Bone Dagger (2 Bones + 1 Vine) ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::BoneDagger;
        Recipe.DisplayName = TEXT("Bone Dagger");
        Recipe.CraftingTimeSeconds = 4.0f;

        FQuest_CraftingIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Bone;
        Ing1.Quantity = 2;
        Recipe.Ingredients.Add(Ing1);

        FQuest_CraftingIngredient Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Vine;
        Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing2);

        KnownRecipes.Add(Recipe);
    }

    // --- Recipe 6: Torch (1 Stick + 1 Vine + 1 Bone) ---
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::Torch;
        Recipe.DisplayName = TEXT("Torch");
        Recipe.CraftingTimeSeconds = 2.0f;

        FQuest_CraftingIngredient Ing1;
        Ing1.ResourceType = EQuest_ResourceType::Stick;
        Ing1.Quantity = 1;
        Recipe.Ingredients.Add(Ing1);

        FQuest_CraftingIngredient Ing2;
        Ing2.ResourceType = EQuest_ResourceType::Vine;
        Ing2.Quantity = 1;
        Recipe.Ingredients.Add(Ing2);

        FQuest_CraftingIngredient Ing3;
        Ing3.ResourceType = EQuest_ResourceType::Bone;
        Ing3.Quantity = 1;
        Recipe.Ingredients.Add(Ing3);

        KnownRecipes.Add(Recipe);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: %d recipes registered"), KnownRecipes.Num());
}

bool UQuest_CraftingManagerComponent::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (Amount <= 0 || ResourceType == EQuest_ResourceType::None) return false;

    // Find existing slot
    for (FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == ResourceType)
        {
            Slot.Count += Amount;
            UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Added %d %s (total: %d)"),
                Amount, *UEnum::GetValueAsString(ResourceType), Slot.Count);
            return true;
        }
    }

    // New slot
    if (Inventory.Num() < MaxInventorySlots)
    {
        FQuest_InventorySlot NewSlot;
        NewSlot.ResourceType = ResourceType;
        NewSlot.Count = Amount;
        Inventory.Add(NewSlot);
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Inventory full! Cannot add %s"), 
        *UEnum::GetValueAsString(ResourceType));
    return false;
}

bool UQuest_CraftingManagerComponent::RemoveResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (Amount <= 0 || ResourceType == EQuest_ResourceType::None) return false;

    for (FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == ResourceType)
        {
            if (Slot.Count >= Amount)
            {
                Slot.Count -= Amount;
                return true;
            }
            return false;
        }
    }
    return false;
}

int32 UQuest_CraftingManagerComponent::GetResourceCount(EQuest_ResourceType ResourceType) const
{
    for (const FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == ResourceType)
        {
            return Slot.Count;
        }
    }
    return 0;
}

bool UQuest_CraftingManagerComponent::HasIngredientsForRecipe(const FQuest_CraftingRecipe& Recipe) const
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

EQuest_CraftResult UQuest_CraftingManagerComponent::TryCraftItem(EQuest_CraftedItem ItemToCraft)
{
    // Find recipe
    const FQuest_CraftingRecipe* FoundRecipe = nullptr;
    for (const FQuest_CraftingRecipe& Recipe : KnownRecipes)
    {
        if (Recipe.OutputItem == ItemToCraft)
        {
            FoundRecipe = &Recipe;
            break;
        }
    }

    if (!FoundRecipe)
    {
        UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: No recipe found for %s"),
            *UEnum::GetValueAsString(ItemToCraft));
        return EQuest_CraftResult::InvalidRecipe;
    }

    if (!HasIngredientsForRecipe(*FoundRecipe))
    {
        UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Missing ingredients for %s"),
            *FoundRecipe->DisplayName);
        return EQuest_CraftResult::MissingIngredient;
    }

    // Consume ingredients
    for (const FQuest_CraftingIngredient& Ingredient : FoundRecipe->Ingredients)
    {
        RemoveResource(Ingredient.ResourceType, Ingredient.Quantity);
    }

    LastCraftedItem = ItemToCraft;
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Crafted %s successfully!"), *FoundRecipe->DisplayName);
    return EQuest_CraftResult::Success;
}

void UQuest_CraftingManagerComponent::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Menu %s"),
        bCraftingMenuOpen ? TEXT("OPENED") : TEXT("CLOSED"));
}

TArray<FQuest_CraftingRecipe> UQuest_CraftingManagerComponent::GetAvailableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Available;
    for (const FQuest_CraftingRecipe& Recipe : KnownRecipes)
    {
        if (HasIngredientsForRecipe(Recipe))
        {
            Available.Add(Recipe);
        }
    }
    return Available;
}

// ============================================================
// AQuest_ResourcePickup
// ============================================================

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    PickupRadius = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRadius"));
    PickupRadius->SetSphereRadius(120.0f);
    PickupRadius->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    RootComponent = PickupRadius;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionProfileName(TEXT("NoCollision"));

    ResourceType = EQuest_ResourceType::Stone;
    ResourceAmount = 1;
    bAutoRespawn = true;
    RespawnTimeSeconds = 120.0f;
    bIsAvailable = true;
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();

    PickupRadius->OnComponentBeginOverlap.AddDynamic(this, &AQuest_ResourcePickup::OnOverlapBegin);
}

void AQuest_ResourcePickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bIsAvailable || !OtherActor) return;

    // Check if the overlapping actor has a crafting component
    UQuest_CraftingManagerComponent* CraftComp = 
        OtherActor->FindComponentByClass<UQuest_CraftingManagerComponent>();
    
    if (CraftComp)
    {
        OnPickedUp(OtherActor);
    }
}

void AQuest_ResourcePickup::OnPickedUp(AActor* PickingActor)
{
    if (!bIsAvailable) return;

    UQuest_CraftingManagerComponent* CraftComp = 
        PickingActor->FindComponentByClass<UQuest_CraftingManagerComponent>();
    
    if (CraftComp)
    {
        CraftComp->AddResource(ResourceType, ResourceAmount);
        UE_LOG(LogTemp, Log, TEXT("ResourcePickup: Player picked up %d %s"),
            ResourceAmount, *UEnum::GetValueAsString(ResourceType));
    }

    bIsAvailable = false;
    MeshComponent->SetVisibility(false);
    PickupRadius->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (bAutoRespawn && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            RespawnTimer,
            this,
            &AQuest_ResourcePickup::Respawn,
            RespawnTimeSeconds,
            false
        );
    }
}

void AQuest_ResourcePickup::Respawn()
{
    bIsAvailable = true;
    MeshComponent->SetVisibility(true);
    PickupRadius->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    UE_LOG(LogTemp, Log, TEXT("ResourcePickup: %s respawned"), *GetName());
}
