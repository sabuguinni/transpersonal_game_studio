#include "CraftingSystem.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"

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
    InitializeDefaultRecipes();
}

void UCraftingSystemComponent::InitializeDefaultRecipes()
{
    Recipes.Empty();

    // Recipe 1: Stone Axe — 2 Rocks + 1 Stick
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::StoneAxe;
        Recipe.RecipeName = TEXT("Stone Axe");
        Recipe.CraftingTime = 3.0f;
        FQuest_CraftingIngredient Rock; Rock.ResourceType = EQuest_ResourceType::Rock; Rock.Quantity = 2;
        FQuest_CraftingIngredient Stick; Stick.ResourceType = EQuest_ResourceType::Stick; Stick.Quantity = 1;
        Recipe.Ingredients.Add(Rock);
        Recipe.Ingredients.Add(Stick);
        Recipes.Add(Recipe);
    }

    // Recipe 2: Campfire — 3 Sticks
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::Campfire;
        Recipe.RecipeName = TEXT("Campfire");
        Recipe.CraftingTime = 2.0f;
        FQuest_CraftingIngredient Stick; Stick.ResourceType = EQuest_ResourceType::Stick; Stick.Quantity = 3;
        Recipe.Ingredients.Add(Stick);
        Recipes.Add(Recipe);
    }

    // Recipe 3: Water Container — 1 Rock + 1 Leaf
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::WaterContainer;
        Recipe.RecipeName = TEXT("Water Container");
        Recipe.CraftingTime = 2.5f;
        FQuest_CraftingIngredient Rock; Rock.ResourceType = EQuest_ResourceType::Rock; Rock.Quantity = 1;
        FQuest_CraftingIngredient Leaf; Leaf.ResourceType = EQuest_ResourceType::Leaf; Leaf.Quantity = 1;
        Recipe.Ingredients.Add(Rock);
        Recipe.Ingredients.Add(Leaf);
        Recipes.Add(Recipe);
    }

    // Recipe 4: Spear — 1 Bone + 2 Sticks
    {
        FQuest_CraftingRecipe Recipe;
        Recipe.OutputItem = EQuest_CraftedItem::Spear;
        Recipe.RecipeName = TEXT("Spear");
        Recipe.CraftingTime = 4.0f;
        FQuest_CraftingIngredient Bone; Bone.ResourceType = EQuest_ResourceType::Bone; Bone.Quantity = 1;
        FQuest_CraftingIngredient Stick; Stick.ResourceType = EQuest_ResourceType::Stick; Stick.Quantity = 2;
        Recipe.Ingredients.Add(Bone);
        Recipe.Ingredients.Add(Stick);
        Recipes.Add(Recipe);
    }
}

const FQuest_CraftingRecipe* UCraftingSystemComponent::FindRecipe(EQuest_CraftedItem Item) const
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

bool UCraftingSystemComponent::CanCraft(EQuest_CraftedItem Item) const
{
    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (!Recipe) return false;

    for (const FQuest_CraftingIngredient& Ingredient : Recipe->Ingredients)
    {
        int32 Available = GetResourceCount(Ingredient.ResourceType);
        if (Available < Ingredient.Quantity)
        {
            return false;
        }
    }
    return true;
}

bool UCraftingSystemComponent::CraftItem(EQuest_CraftedItem Item)
{
    if (!CanCraft(Item)) return false;

    const FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (!Recipe) return false;

    for (const FQuest_CraftingIngredient& Ingredient : Recipe->Ingredients)
    {
        int32& Count = Inventory.FindOrAdd(Ingredient.ResourceType);
        Count -= Ingredient.Quantity;
        if (Count < 0) Count = 0;
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Crafted %s"), *Recipe->RecipeName);
    return true;
}

void UCraftingSystemComponent::AddResource(EQuest_ResourceType Resource, int32 Amount)
{
    int32& Count = Inventory.FindOrAdd(Resource);
    Count += Amount;
    UE_LOG(LogTemp, Log, TEXT("CraftingSystem: Added %d x %d"), (int32)Resource, Amount);
}

int32 UCraftingSystemComponent::GetResourceCount(EQuest_ResourceType Resource) const
{
    const int32* Count = Inventory.Find(Resource);
    return Count ? *Count : 0;
}

TArray<EQuest_CraftedItem> UCraftingSystemComponent::GetAvailableRecipes() const
{
    TArray<EQuest_CraftedItem> Available;
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (CanCraft(Recipe.OutputItem))
        {
            Available.Add(Recipe.OutputItem);
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
    PickupRadius->OnComponentBeginOverlap.AddDynamic(this, &AQuest_ResourcePickup::OnOverlapBegin);
}

void AQuest_ResourcePickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this && !bHasBeenPickedUp)
    {
        PickUp(OtherActor);
    }
}

void AQuest_ResourcePickup::PickUp(AActor* Collector)
{
    if (bHasBeenPickedUp) return;

    UCraftingSystemComponent* CraftComp = Collector->FindComponentByClass<UCraftingSystemComponent>();
    if (CraftComp)
    {
        CraftComp->AddResource(ResourceType, ResourceAmount);
        UE_LOG(LogTemp, Log, TEXT("ResourcePickup: %s collected resource type %d x%d"),
            *Collector->GetName(), (int32)ResourceType, ResourceAmount);
    }

    bHasBeenPickedUp = true;
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    // Destroy after short delay
    SetLifeSpan(0.5f);
}
