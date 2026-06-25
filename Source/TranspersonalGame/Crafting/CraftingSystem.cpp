// CraftingSystem.cpp
// Agent #14 — Quest & Mission Designer
// Full implementation of crafting system: 3 core recipes + resource pickup

#include "CraftingSystem.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

// ─────────────────────────────────────────────────────────────────────────────
// AQuest_ResourcePickup
// ─────────────────────────────────────────────────────────────────────────────

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
}

void AQuest_ResourcePickup::OnPickedUp(AActor* Collector)
{
    if (!Collector) return;

    // Notify collector's crafting component
    UQuest_CraftingSystemComponent* CraftComp =
        Collector->FindComponentByClass<UQuest_CraftingSystemComponent>();
    if (CraftComp)
    {
        CraftComp->AddResource(ResourceType, Quantity);
        UE_LOG(LogTemp, Log, TEXT("[CraftingSystem] %s picked up %d x %s"),
            *Collector->GetName(),
            Quantity,
            *UEnum::GetValueAsString(ResourceType));
    }

    // Hide and schedule respawn
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    if (bAutoRespawn && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            RespawnTimer,
            this,
            &AQuest_ResourcePickup::Respawn,
            RespawnDelay,
            false
        );
    }
}

void AQuest_ResourcePickup::Respawn()
{
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    UE_LOG(LogTemp, Log, TEXT("[CraftingSystem] Resource respawned: %s"), *GetActorLabel());
}

// ─────────────────────────────────────────────────────────────────────────────
// UQuest_CraftingSystemComponent
// ─────────────────────────────────────────────────────────────────────────────

UQuest_CraftingSystemComponent::UQuest_CraftingSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UQuest_CraftingSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    InitialiseDefaultRecipes();
}

void UQuest_CraftingSystemComponent::InitialiseDefaultRecipes()
{
    Recipes.Empty();

    // ── Recipe 1: Stone Axe (2 Rocks + 1 Stick) ──────────────────────────────
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.OutputItem = EQuest_CraftedItem::StoneAxe;
        StoneAxe.ItemName   = TEXT("Stone Axe");
        StoneAxe.Description = TEXT("A crude but effective chopping tool. Useful for cutting wood and defending against small predators.");

        FQuest_ResourceIngredient RockIngredient;
        RockIngredient.ResourceType = EQuest_ResourceType::Rock;
        RockIngredient.Quantity     = 2;

        FQuest_ResourceIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity     = 1;

        StoneAxe.Ingredients.Add(RockIngredient);
        StoneAxe.Ingredients.Add(StickIngredient);
        Recipes.Add(StoneAxe);
    }

    // ── Recipe 2: Campfire (3 Sticks) ────────────────────────────────────────
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.OutputItem  = EQuest_CraftedItem::Campfire;
        Campfire.ItemName    = TEXT("Campfire");
        Campfire.Description = TEXT("A small fire that keeps predators away at night and allows cooking of raw meat.");

        FQuest_ResourceIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity     = 3;

        Campfire.Ingredients.Add(StickIngredient);
        Recipes.Add(Campfire);
    }

    // ── Recipe 3: Water Container (1 Rock + 1 Leaf) ──────────────────────────
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.OutputItem  = EQuest_CraftedItem::WaterContainer;
        WaterContainer.ItemName    = TEXT("Water Container");
        WaterContainer.Description = TEXT("A hollowed rock lined with large leaves. Holds enough water for one day of travel.");

        FQuest_ResourceIngredient RockIngredient;
        RockIngredient.ResourceType = EQuest_ResourceType::Rock;
        RockIngredient.Quantity     = 1;

        FQuest_ResourceIngredient LeafIngredient;
        LeafIngredient.ResourceType = EQuest_ResourceType::Leaf;
        LeafIngredient.Quantity     = 1;

        WaterContainer.Ingredients.Add(RockIngredient);
        WaterContainer.Ingredients.Add(LeafIngredient);
        Recipes.Add(WaterContainer);
    }

    // ── Recipe 4: Spear (2 Sticks + 1 Flint) ────────────────────────────────
    {
        FQuest_CraftingRecipe Spear;
        Spear.OutputItem  = EQuest_CraftedItem::Spear;
        Spear.ItemName    = TEXT("Spear");
        Spear.Description = TEXT("A long wooden shaft tipped with a sharpened flint head. Effective against medium-sized dinosaurs at range.");

        FQuest_ResourceIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity     = 2;

        FQuest_ResourceIngredient FlintIngredient;
        FlintIngredient.ResourceType = EQuest_ResourceType::Flint;
        FlintIngredient.Quantity     = 1;

        Spear.Ingredients.Add(StickIngredient);
        Spear.Ingredients.Add(FlintIngredient);
        Recipes.Add(Spear);
    }

    // ── Recipe 5: Leather Pouch (2 Hides + 1 Bone) ──────────────────────────
    {
        FQuest_CraftingRecipe LeatherPouch;
        LeatherPouch.OutputItem  = EQuest_CraftedItem::LeatherPouch;
        LeatherPouch.ItemName    = TEXT("Leather Pouch");
        LeatherPouch.Description = TEXT("A carrying pouch made from dinosaur hide. Doubles your resource carrying capacity.");

        FQuest_ResourceIngredient HideIngredient;
        HideIngredient.ResourceType = EQuest_ResourceType::Hide;
        HideIngredient.Quantity     = 2;

        FQuest_ResourceIngredient BoneIngredient;
        BoneIngredient.ResourceType = EQuest_ResourceType::Bone;
        BoneIngredient.Quantity     = 1;

        LeatherPouch.Ingredients.Add(HideIngredient);
        LeatherPouch.Ingredients.Add(BoneIngredient);
        Recipes.Add(LeatherPouch);
    }

    UE_LOG(LogTemp, Log, TEXT("[CraftingSystem] Initialised %d recipes"), Recipes.Num());
}

const FQuest_CraftingRecipe* UQuest_CraftingSystemComponent::FindRecipe(EQuest_CraftedItem Item) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.OutputItem == Item)
        {
            return &Recipe;
        }
    }
    return nullptr;
}

bool UQuest_CraftingSystemComponent::CanCraft(EQuest_CraftedItem Item) const
{
    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (!Recipe) return false;

    for (const FQuest_ResourceIngredient& Ingredient : Recipe->Ingredients)
    {
        const int32* Count = ResourceInventory.Find(Ingredient.ResourceType);
        int32 Available = Count ? *Count : 0;
        if (Available < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

bool UQuest_CraftingSystemComponent::TryCraft(EQuest_CraftedItem Item)
{
    if (!CanCraft(Item)) return false;

    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (!Recipe) return false;

    // Consume ingredients
    for (const FQuest_ResourceIngredient& Ingredient : Recipe->Ingredients)
    {
        int32& Count = ResourceInventory.FindOrAdd(Ingredient.ResourceType);
        Count -= Ingredient.Quantity;
        if (Count <= 0)
        {
            ResourceInventory.Remove(Ingredient.ResourceType);
        }
    }

    // Add crafted item
    CraftedItems.Add(Item);
    UE_LOG(LogTemp, Log, TEXT("[CraftingSystem] Crafted: %s"), *Recipe->ItemName);
    return true;
}

void UQuest_CraftingSystemComponent::AddResource(EQuest_ResourceType Type, int32 Amount)
{
    int32& Count = ResourceInventory.FindOrAdd(Type);
    Count += Amount;
    UE_LOG(LogTemp, Log, TEXT("[CraftingSystem] Added %d x %s — Total: %d"),
        Amount,
        *UEnum::GetValueAsString(Type),
        Count);
}

int32 UQuest_CraftingSystemComponent::GetResourceCount(EQuest_ResourceType Type) const
{
    const int32* Count = ResourceInventory.Find(Type);
    return Count ? *Count : 0;
}

bool UQuest_CraftingSystemComponent::HasCraftedItem(EQuest_CraftedItem Item) const
{
    return CraftedItems.Contains(Item);
}

TArray<FQuest_CraftingRecipe> UQuest_CraftingSystemComponent::GetAvailableRecipes() const
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

void UQuest_CraftingSystemComponent::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    UE_LOG(LogTemp, Log, TEXT("[CraftingSystem] Crafting menu %s"),
        bCraftingMenuOpen ? TEXT("OPENED") : TEXT("CLOSED"));
}
