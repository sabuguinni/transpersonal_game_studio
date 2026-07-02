// CraftingSystem.cpp
// Agent #14 — Quest & Mission Designer
// Implements: Stone Axe, Campfire, Water Container recipes + resource pickup actors + crafting UI trigger (C key)

#include "CraftingSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────────────────────
// AQuest_ResourcePickup
// ─────────────────────────────────────────────────────────────────────────────

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
    RootComponent = PickupMesh;
    PickupMesh->SetCollisionProfileName(TEXT("BlockAll"));

    PickupRadius = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRadius"));
    PickupRadius->SetupAttachment(RootComponent);
    PickupRadius->SetSphereRadius(80.0f);
    PickupRadius->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    ResourceType = EQuest_ResourceType::Rock;
    QuantityOnPickup = 1;
    bHasBeenPickedUp = false;
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
    PickupRadius->OnComponentBeginOverlap.AddDynamic(this, &AQuest_ResourcePickup::OnOverlapBegin);
}

void AQuest_ResourcePickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bHasBeenPickedUp) return;
    if (!OtherActor || OtherActor == this) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    // Find crafting manager and add resource
    TArray<AActor*> FoundManagers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AQuest_CraftingManager::StaticClass(), FoundManagers);
    if (FoundManagers.Num() > 0)
    {
        AQuest_CraftingManager* Manager = Cast<AQuest_CraftingManager>(FoundManagers[0]);
        if (Manager)
        {
            Manager->AddResource(ResourceType, QuantityOnPickup);
            bHasBeenPickedUp = true;
            // Hide mesh, disable collision
            PickupMesh->SetVisibility(false);
            PickupRadius->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            unreal_log_pickup();
        }
    }
}

void AQuest_ResourcePickup::unreal_log_pickup()
{
    UE_LOG(LogTemp, Log, TEXT("[Quest] Resource picked up: Type=%d Qty=%d"), (int32)ResourceType, QuantityOnPickup);
}

// ─────────────────────────────────────────────────────────────────────────────
// AQuest_CraftingManager
// ─────────────────────────────────────────────────────────────────────────────

AQuest_CraftingManager::AQuest_CraftingManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bCraftingMenuOpen = false;
    RegisterRecipes();
}

void AQuest_CraftingManager::BeginPlay()
{
    Super::BeginPlay();
    RegisterRecipes();
    UE_LOG(LogTemp, Log, TEXT("[Quest] CraftingManager initialized with %d recipes"), Recipes.Num());
}

void AQuest_CraftingManager::RegisterRecipes()
{
    Recipes.Empty();

    // ── Recipe 1: Stone Axe (2 Rocks + 1 Stick) ──────────────────────────────
    FQuest_CraftingRecipe StoneAxe;
    StoneAxe.RecipeName = FName("StoneAxe");
    StoneAxe.DisplayName = FText::FromString(TEXT("Stone Axe"));
    StoneAxe.Description = FText::FromString(TEXT("A crude but effective axe. Chop wood and defend against small predators."));
    StoneAxe.OutputItemName = FName("StoneAxe");
    StoneAxe.CraftTimeSeconds = 3.0f;
    StoneAxe.bUnlockedByDefault = true;
    FQuest_ResourceCost RockCost1; RockCost1.ResourceType = EQuest_ResourceType::Rock; RockCost1.Quantity = 2;
    FQuest_ResourceCost StickCost1; StickCost1.ResourceType = EQuest_ResourceType::Stick; StickCost1.Quantity = 1;
    StoneAxe.Ingredients.Add(RockCost1);
    StoneAxe.Ingredients.Add(StickCost1);
    Recipes.Add(StoneAxe);

    // ── Recipe 2: Campfire (3 Sticks) ────────────────────────────────────────
    FQuest_CraftingRecipe Campfire;
    Campfire.RecipeName = FName("Campfire");
    Campfire.DisplayName = FText::FromString(TEXT("Campfire"));
    Campfire.Description = FText::FromString(TEXT("A small fire. Keeps predators away at night and cooks raw meat."));
    Campfire.OutputItemName = FName("Campfire");
    Campfire.CraftTimeSeconds = 5.0f;
    Campfire.bUnlockedByDefault = true;
    FQuest_ResourceCost StickCost2; StickCost2.ResourceType = EQuest_ResourceType::Stick; StickCost2.Quantity = 3;
    Campfire.Ingredients.Add(StickCost2);
    Recipes.Add(Campfire);

    // ── Recipe 3: Water Container (1 Rock + 1 Leaf) ──────────────────────────
    FQuest_CraftingRecipe WaterContainer;
    WaterContainer.RecipeName = FName("WaterContainer");
    WaterContainer.DisplayName = FText::FromString(TEXT("Water Container"));
    WaterContainer.Description = FText::FromString(TEXT("A hollowed rock lined with large leaves. Holds enough water for one day."));
    WaterContainer.OutputItemName = FName("WaterContainer");
    WaterContainer.CraftTimeSeconds = 4.0f;
    WaterContainer.bUnlockedByDefault = true;
    FQuest_ResourceCost RockCost3; RockCost3.ResourceType = EQuest_ResourceType::Rock; RockCost3.Quantity = 1;
    FQuest_ResourceCost LeafCost3; LeafCost3.ResourceType = EQuest_ResourceType::Leaf; LeafCost3.Quantity = 1;
    WaterContainer.Ingredients.Add(RockCost3);
    WaterContainer.Ingredients.Add(LeafCost3);
    Recipes.Add(WaterContainer);
}

void AQuest_CraftingManager::AddResource(EQuest_ResourceType Type, int32 Quantity)
{
    int32& CurrentQty = ResourceInventory.FindOrAdd(Type);
    CurrentQty += Quantity;
    UE_LOG(LogTemp, Log, TEXT("[Quest] Resource added: Type=%d, Total=%d"), (int32)Type, CurrentQty);
}

int32 AQuest_CraftingManager::GetResourceCount(EQuest_ResourceType Type) const
{
    const int32* Found = ResourceInventory.Find(Type);
    return Found ? *Found : 0;
}

bool AQuest_CraftingManager::CanCraft(FName RecipeName) const
{
    const FQuest_CraftingRecipe* Recipe = FindRecipe(RecipeName);
    if (!Recipe) return false;
    if (!Recipe->bUnlockedByDefault) return false;

    for (const FQuest_ResourceCost& Cost : Recipe->Ingredients)
    {
        if (GetResourceCount(Cost.ResourceType) < Cost.Quantity)
            return false;
    }
    return true;
}

bool AQuest_CraftingManager::CraftItem(FName RecipeName)
{
    if (!CanCraft(RecipeName))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Quest] Cannot craft %s — insufficient resources"), *RecipeName.ToString());
        return false;
    }

    const FQuest_CraftingRecipe* Recipe = FindRecipe(RecipeName);
    if (!Recipe) return false;

    // Deduct ingredients
    for (const FQuest_ResourceCost& Cost : Recipe->Ingredients)
    {
        int32& Qty = ResourceInventory.FindOrAdd(Cost.ResourceType);
        Qty -= Cost.Quantity;
        if (Qty < 0) Qty = 0;
    }

    CraftedItems.Add(Recipe->OutputItemName);
    UE_LOG(LogTemp, Log, TEXT("[Quest] Crafted: %s"), *Recipe->OutputItemName.ToString());
    OnItemCrafted.Broadcast(Recipe->OutputItemName);
    return true;
}

void AQuest_CraftingManager::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    UE_LOG(LogTemp, Log, TEXT("[Quest] Crafting menu: %s"), bCraftingMenuOpen ? TEXT("OPEN") : TEXT("CLOSED"));
    OnCraftingMenuToggled.Broadcast(bCraftingMenuOpen);
}

TArray<FQuest_CraftingRecipe> AQuest_CraftingManager::GetAvailableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Available;
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.bUnlockedByDefault)
            Available.Add(Recipe);
    }
    return Available;
}

const FQuest_CraftingRecipe* AQuest_CraftingManager::FindRecipe(FName RecipeName) const
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.RecipeName == RecipeName)
            return &Recipe;
    }
    return nullptr;
}
