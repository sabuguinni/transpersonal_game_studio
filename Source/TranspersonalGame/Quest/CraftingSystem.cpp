// CraftingSystem.cpp
// Agent #14 — Quest & Mission Designer
// Crafting system: recipes, resource pickups, crafting station interaction
// PROD_CYCLE_AUTO_20260623_007

#include "CraftingSystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// UCraftingSystemComponent
// ============================================================

UCraftingSystemComponent::UCraftingSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bCraftingMenuOpen = false;

    // Define 3 base recipes
    FQuest_CraftingRecipe StoneAxe;
    StoneAxe.RecipeName = FName("StoneAxe");
    StoneAxe.DisplayName = FText::FromString("Stone Axe");
    StoneAxe.Description = FText::FromString("A crude but effective cutting tool. Requires 2 flint rocks and 1 dry branch.");
    StoneAxe.OutputItemName = FName("StoneAxe");
    StoneAxe.CraftingTimeSeconds = 3.0f;
    FQuest_ResourceRequirement Req1; Req1.ResourceType = EQuest_ResourceType::FlintRock; Req1.Quantity = 2;
    FQuest_ResourceRequirement Req2; Req2.ResourceType = EQuest_ResourceType::DryBranch; Req2.Quantity = 1;
    StoneAxe.Requirements.Add(Req1);
    StoneAxe.Requirements.Add(Req2);
    Recipes.Add(StoneAxe);

    FQuest_CraftingRecipe Campfire;
    Campfire.RecipeName = FName("Campfire");
    Campfire.DisplayName = FText::FromString("Campfire");
    Campfire.Description = FText::FromString("A fire for warmth and cooking. Requires 3 dry branches.");
    Campfire.OutputItemName = FName("Campfire");
    Campfire.CraftingTimeSeconds = 5.0f;
    FQuest_ResourceRequirement Req3; Req3.ResourceType = EQuest_ResourceType::DryBranch; Req3.Quantity = 3;
    Campfire.Requirements.Add(Req3);
    Recipes.Add(Campfire);

    FQuest_CraftingRecipe WaterContainer;
    WaterContainer.RecipeName = FName("WaterContainer");
    WaterContainer.DisplayName = FText::FromString("Water Container");
    WaterContainer.Description = FText::FromString("A hollowed stone for carrying water. Requires 1 flint rock and 1 large leaf.");
    WaterContainer.OutputItemName = FName("WaterContainer");
    WaterContainer.CraftingTimeSeconds = 4.0f;
    FQuest_ResourceRequirement Req4; Req4.ResourceType = EQuest_ResourceType::FlintRock; Req4.Quantity = 1;
    FQuest_ResourceRequirement Req5; Req5.ResourceType = EQuest_ResourceType::LargeLeaf; Req5.Quantity = 1;
    WaterContainer.Requirements.Add(Req4);
    WaterContainer.Requirements.Add(Req5);
    Recipes.Add(WaterContainer);
}

void UCraftingSystemComponent::BeginPlay()
{
    Super::BeginPlay();
}

bool UCraftingSystemComponent::HasRequiredResources(const FQuest_CraftingRecipe& Recipe) const
{
    for (const FQuest_ResourceRequirement& Req : Recipe.Requirements)
    {
        const int32* Count = Inventory.Find(Req.ResourceType);
        if (!Count || *Count < Req.Quantity)
        {
            return false;
        }
    }
    return true;
}

bool UCraftingSystemComponent::CraftItem(FName RecipeName)
{
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.RecipeName == RecipeName)
        {
            if (!HasRequiredResources(Recipe))
            {
                OnCraftingFailed.Broadcast(RecipeName, FText::FromString("Not enough resources."));
                return false;
            }

            // Consume resources
            for (const FQuest_ResourceRequirement& Req : Recipe.Requirements)
            {
                int32& Count = Inventory.FindOrAdd(Req.ResourceType);
                Count -= Req.Quantity;
                if (Count <= 0) Inventory.Remove(Req.ResourceType);
            }

            // Start crafting timer
            FTimerHandle Handle;
            FTimerDelegate Delegate;
            FName OutputItem = Recipe.OutputItemName;
            Delegate.BindLambda([this, OutputItem]()
            {
                CraftedItems.Add(OutputItem);
                OnCraftingComplete.Broadcast(OutputItem);
            });
            GetWorld()->GetTimerManager().SetTimer(Handle, Delegate, Recipe.CraftingTimeSeconds, false);

            return true;
        }
    }
    return false;
}

void UCraftingSystemComponent::AddResource(EQuest_ResourceType ResourceType, int32 Quantity)
{
    int32& Count = Inventory.FindOrAdd(ResourceType);
    Count += Quantity;
    OnResourceAdded.Broadcast(ResourceType, Count);
}

void UCraftingSystemComponent::OpenCraftingMenu()
{
    bCraftingMenuOpen = true;
    OnCraftingMenuToggled.Broadcast(true);
}

void UCraftingSystemComponent::CloseCraftingMenu()
{
    bCraftingMenuOpen = false;
    OnCraftingMenuToggled.Broadcast(false);
}

TArray<FQuest_CraftingRecipe> UCraftingSystemComponent::GetAvailableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Available;
    for (const FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (HasRequiredResources(Recipe))
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
    ResourceType = EQuest_ResourceType::FlintRock;
    Quantity = 1;
    bHasBeenPickedUp = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
}

void AQuest_ResourcePickup::PickUp(AActor* Picker)
{
    if (bHasBeenPickedUp) return;

    UCraftingSystemComponent* CraftComp = Picker->FindComponentByClass<UCraftingSystemComponent>();
    if (CraftComp)
    {
        CraftComp->AddResource(ResourceType, Quantity);
        bHasBeenPickedUp = true;
        OnPickedUp.Broadcast(Picker, ResourceType, Quantity);
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
    }
}

// ============================================================
// AQuest_CraftingStation
// ============================================================

AQuest_CraftingStation::AQuest_CraftingStation()
{
    PrimaryActorTick.bCanEverTick = false;
    InteractionRadius = 200.0f;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
}

void AQuest_CraftingStation::BeginPlay()
{
    Super::BeginPlay();
}

void AQuest_CraftingStation::Interact(AActor* Interactor)
{
    UCraftingSystemComponent* CraftComp = Interactor->FindComponentByClass<UCraftingSystemComponent>();
    if (CraftComp)
    {
        if (CraftComp->bCraftingMenuOpen)
        {
            CraftComp->CloseCraftingMenu();
        }
        else
        {
            CraftComp->OpenCraftingMenu();
        }
        OnInteracted.Broadcast(Interactor);
    }
}
