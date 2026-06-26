#include "CraftingSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

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

    CollisionSphere->OnComponentBeginOverlap.AddDynamic(
        this, &AQuest_ResourcePickup::OnOverlapBegin);
}

void AQuest_ResourcePickup::OnOverlapBegin(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!bIsAvailable) return;
    if (!OtherActor) return;

    // Only player characters can collect
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (!Character) return;

    Collect(OtherActor);
}

void AQuest_ResourcePickup::Collect(AActor* Collector)
{
    if (!bIsAvailable) return;

    // Try to add to collector's crafting manager
    UQuest_CraftingManager* CraftingMgr =
        Collector ? Collector->FindComponentByClass<UQuest_CraftingManager>() : nullptr;

    if (CraftingMgr)
    {
        CraftingMgr->AddResource(ResourceType, Quantity);
    }

    bIsAvailable = false;
    MeshComponent->SetVisibility(false);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (bAutoRespawn && RespawnDelay > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            RespawnTimer,
            this,
            &AQuest_ResourcePickup::Respawn,
            RespawnDelay,
            false);
    }
}

void AQuest_ResourcePickup::Respawn()
{
    bIsAvailable = true;
    MeshComponent->SetVisibility(true);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

// ─────────────────────────────────────────────────────────────────────────────
// UQuest_CraftingManager
// ─────────────────────────────────────────────────────────────────────────────

UQuest_CraftingManager::UQuest_CraftingManager()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UQuest_CraftingManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultRecipes();
}

void UQuest_CraftingManager::InitializeDefaultRecipes()
{
    Recipes.Empty();

    // ── Recipe 1: Stone Axe (2 Rocks + 1 Stick) ──
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.RecipeID = FName("StoneAxe");
        StoneAxe.DisplayName = FText::FromString(TEXT("Stone Axe"));
        StoneAxe.Description = FText::FromString(
            TEXT("A crude but effective chopping tool. Useful for cutting wood and fighting small predators."));

        FQuest_CraftingIngredient RockIngredient;
        RockIngredient.ResourceType = EQuest_ResourceType::Rock;
        RockIngredient.Quantity = 2;

        FQuest_CraftingIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity = 1;

        StoneAxe.Ingredients.Add(RockIngredient);
        StoneAxe.Ingredients.Add(StickIngredient);
        Recipes.Add(StoneAxe);
    }

    // ── Recipe 2: Campfire (3 Sticks) ──
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.RecipeID = FName("Campfire");
        Campfire.DisplayName = FText::FromString(TEXT("Campfire"));
        Campfire.Description = FText::FromString(
            TEXT("A small fire to keep warm and cook food. Keeps predators at bay during the night."));

        FQuest_CraftingIngredient StickIngredient;
        StickIngredient.ResourceType = EQuest_ResourceType::Stick;
        StickIngredient.Quantity = 3;

        Campfire.Ingredients.Add(StickIngredient);
        Recipes.Add(Campfire);
    }

    // ── Recipe 3: Water Container (1 Rock + 1 Leaf) ──
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.RecipeID = FName("WaterContainer");
        WaterContainer.DisplayName = FText::FromString(TEXT("Water Container"));
        WaterContainer.Description = FText::FromString(
            TEXT("A hollowed rock sealed with large leaves. Holds enough water for one day of travel."));

        FQuest_CraftingIngredient RockIngredient;
        RockIngredient.ResourceType = EQuest_ResourceType::Rock;
        RockIngredient.Quantity = 1;

        FQuest_CraftingIngredient LeafIngredient;
        LeafIngredient.ResourceType = EQuest_ResourceType::Leaf;
        LeafIngredient.Quantity = 1;

        WaterContainer.Ingredients.Add(RockIngredient);
        WaterContainer.Ingredients.Add(LeafIngredient);
        Recipes.Add(WaterContainer);
    }
}

void UQuest_CraftingManager::AddResource(EQuest_ResourceType Type, int32 Amount)
{
    if (Type == EQuest_ResourceType::None || Amount <= 0) return;

    int32& Current = Inventory.FindOrAdd(Type);
    Current += Amount;
}

bool UQuest_CraftingManager::CanCraft(FName RecipeID) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.RecipeID != RecipeID) continue;

        for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
        {
            int32 Have = GetResourceCount(Ingredient.ResourceType);
            if (Have < Ingredient.Quantity) return false;
        }
        return true;
    }
    return false;
}

bool UQuest_CraftingManager::CraftItem(FName RecipeID)
{
    if (!CanCraft(RecipeID)) return false;

    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.RecipeID != RecipeID) continue;

        // Consume ingredients
        for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
        {
            int32& Current = Inventory.FindOrAdd(Ingredient.ResourceType);
            Current = FMath::Max(0, Current - Ingredient.Quantity);
        }
        return true;
    }
    return false;
}

int32 UQuest_CraftingManager::GetResourceCount(EQuest_ResourceType Type) const
{
    const int32* Count = Inventory.Find(Type);
    return Count ? *Count : 0;
}

TArray<FQuest_CraftingRecipe> UQuest_CraftingManager::GetAvailableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Available;
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (CanCraft(Recipe.RecipeID))
        {
            Available.Add(Recipe);
        }
    }
    return Available;
}
