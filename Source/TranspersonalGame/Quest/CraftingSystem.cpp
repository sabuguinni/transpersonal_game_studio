#include "CraftingSystem.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

// ============================================================
// AQuest_ResourcePickup Implementation
// ============================================================

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));

    PickupRadius = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRadius"));
    PickupRadius->SetupAttachment(RootComponent);
    PickupRadius->SetSphereRadius(PickupRadius_CM);
    PickupRadius->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoPickup)
    {
        PickupRadius->OnComponentBeginOverlap.AddDynamic(this, &AQuest_ResourcePickup::OnOverlapBegin);
    }
}

void AQuest_ResourcePickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bHasBeenPickedUp && OtherActor && OtherActor != this)
    {
        OnPickedUp(OtherActor);
    }
}

void AQuest_ResourcePickup::OnPickedUp(AActor* Collector)
{
    if (bHasBeenPickedUp) return;
    bHasBeenPickedUp = true;

    // Notify collector (would integrate with CraftingManager in full implementation)
    if (GEngine)
    {
        FString ResourceName = UEnum::GetValueAsString(ResourceType);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
            FString::Printf(TEXT("Picked up: %s x%d"), *ResourceName, ResourceAmount));
    }

    // Destroy after pickup
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    // Delayed destroy
    FTimerHandle DestroyTimer;
    GetWorldTimerManager().SetTimer(DestroyTimer, [this]()
    {
        Destroy();
    }, 0.5f, false);
}

// ============================================================
// AQuest_CraftingManager Implementation
// ============================================================

AQuest_CraftingManager::AQuest_CraftingManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize inventory slots
    Inventory.SetNum(MaxInventorySlots);
    for (int32 i = 0; i < MaxInventorySlots; i++)
    {
        Inventory[i].ResourceType = EQuest_ResourceType::None;
        Inventory[i].Count = 0;
        Inventory[i].MaxStack = 10;
    }
}

void AQuest_CraftingManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultRecipes();
}

void AQuest_CraftingManager::InitializeDefaultRecipes()
{
    AvailableRecipes.Empty();

    // Recipe 1: Stone Axe — 2 Stones + 1 Stick
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.OutputItem = EQuest_CraftedItem::StoneAxe;
        StoneAxe.DisplayName = TEXT("Stone Axe");
        StoneAxe.Description = TEXT("A primitive axe made from sharp stone bound to a wooden handle. Used for chopping and combat.");
        StoneAxe.CraftTimeSeconds = 3.0f;

        FQuest_CraftingIngredient Stone1;
        Stone1.ResourceType = EQuest_ResourceType::Stone;
        Stone1.Quantity = 2;
        StoneAxe.Ingredients.Add(Stone1);

        FQuest_CraftingIngredient Stick1;
        Stick1.ResourceType = EQuest_ResourceType::Stick;
        Stick1.Quantity = 1;
        StoneAxe.Ingredients.Add(Stick1);

        AvailableRecipes.Add(StoneAxe);
    }

    // Recipe 2: Campfire — 3 Sticks
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.OutputItem = EQuest_CraftedItem::Campfire;
        Campfire.DisplayName = TEXT("Campfire");
        Campfire.Description = TEXT("A basic campfire for warmth, cooking meat, and keeping predators away at night.");
        Campfire.CraftTimeSeconds = 2.0f;

        FQuest_CraftingIngredient Sticks;
        Sticks.ResourceType = EQuest_ResourceType::Stick;
        Sticks.Quantity = 3;
        Campfire.Ingredients.Add(Sticks);

        AvailableRecipes.Add(Campfire);
    }

    // Recipe 3: Water Container — 1 Stone + 1 Leaf
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.OutputItem = EQuest_CraftedItem::WaterContainer;
        WaterContainer.DisplayName = TEXT("Water Container");
        WaterContainer.Description = TEXT("A crude stone bowl lined with large leaves. Holds water for drinking.");
        WaterContainer.CraftTimeSeconds = 2.5f;

        FQuest_CraftingIngredient Stone2;
        Stone2.ResourceType = EQuest_ResourceType::Stone;
        Stone2.Quantity = 1;
        WaterContainer.Ingredients.Add(Stone2);

        FQuest_CraftingIngredient Leaf;
        Leaf.ResourceType = EQuest_ResourceType::Leaf;
        Leaf.Quantity = 1;
        WaterContainer.Ingredients.Add(Leaf);

        AvailableRecipes.Add(WaterContainer);
    }

    // Recipe 4: Flint Knife — 2 Flints + 1 Vine
    {
        FQuest_CraftingRecipe FlintKnife;
        FlintKnife.OutputItem = EQuest_CraftedItem::FlintKnife;
        FlintKnife.DisplayName = TEXT("Flint Knife");
        FlintKnife.Description = TEXT("A sharp flint blade wrapped with vine for grip. Effective for skinning and cutting.");
        FlintKnife.CraftTimeSeconds = 4.0f;

        FQuest_CraftingIngredient Flint;
        Flint.ResourceType = EQuest_ResourceType::Flint;
        Flint.Quantity = 2;
        FlintKnife.Ingredients.Add(Flint);

        FQuest_CraftingIngredient Vine;
        Vine.ResourceType = EQuest_ResourceType::Vine;
        Vine.Quantity = 1;
        FlintKnife.Ingredients.Add(Vine);

        AvailableRecipes.Add(FlintKnife);
    }

    // Recipe 5: Bone Scraper — 1 Bone + 1 Stone
    {
        FQuest_CraftingRecipe BoneScraper;
        BoneScraper.OutputItem = EQuest_CraftedItem::BoneScraper;
        BoneScraper.DisplayName = TEXT("Bone Scraper");
        BoneScraper.Description = TEXT("A sharpened bone tool for scraping hides. Required to craft leather items.");
        BoneScraper.CraftTimeSeconds = 2.0f;

        FQuest_CraftingIngredient Bone;
        Bone.ResourceType = EQuest_ResourceType::Bone;
        Bone.Quantity = 1;
        BoneScraper.Ingredients.Add(Bone);

        FQuest_CraftingIngredient Stone3;
        Stone3.ResourceType = EQuest_ResourceType::Stone;
        Stone3.Quantity = 1;
        BoneScraper.Ingredients.Add(Stone3);

        AvailableRecipes.Add(BoneScraper);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingManager: Initialized %d recipes"), AvailableRecipes.Num());
}

bool AQuest_CraftingManager::AddResource(EQuest_ResourceType Type, int32 Amount)
{
    if (Type == EQuest_ResourceType::None || Amount <= 0) return false;

    // Find existing slot
    for (FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == Type && Slot.Count < Slot.MaxStack)
        {
            int32 SpaceAvailable = Slot.MaxStack - Slot.Count;
            int32 ToAdd = FMath::Min(Amount, SpaceAvailable);
            Slot.Count += ToAdd;
            Amount -= ToAdd;
            if (Amount <= 0) return true;
        }
    }

    // Find empty slot
    for (FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == EQuest_ResourceType::None)
        {
            Slot.ResourceType = Type;
            Slot.Count = FMath::Min(Amount, Slot.MaxStack);
            Amount -= Slot.Count;
            if (Amount <= 0) return true;
        }
    }

    // Inventory full
    return false;
}

bool AQuest_CraftingManager::RemoveResource(EQuest_ResourceType Type, int32 Amount)
{
    if (GetResourceCount(Type) < Amount) return false;

    int32 Remaining = Amount;
    for (FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == Type && Remaining > 0)
        {
            int32 ToRemove = FMath::Min(Remaining, Slot.Count);
            Slot.Count -= ToRemove;
            Remaining -= ToRemove;

            if (Slot.Count <= 0)
            {
                Slot.ResourceType = EQuest_ResourceType::None;
                Slot.Count = 0;
            }
        }
    }
    return true;
}

int32 AQuest_CraftingManager::GetResourceCount(EQuest_ResourceType Type) const
{
    int32 Total = 0;
    for (const FQuest_InventorySlot& Slot : Inventory)
    {
        if (Slot.ResourceType == Type)
        {
            Total += Slot.Count;
        }
    }
    return Total;
}

bool AQuest_CraftingManager::HasIngredientsForRecipe(const FQuest_CraftingRecipe& Recipe) const
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

TArray<FQuest_CraftingRecipe> AQuest_CraftingManager::GetCraftableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Craftable;
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (HasIngredientsForRecipe(Recipe))
        {
            Craftable.Add(Recipe);
        }
    }
    return Craftable;
}

FQuest_CraftingRecipe* AQuest_CraftingManager::FindRecipeForItem(EQuest_CraftedItem Item)
{
    for (FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.OutputItem == Item)
        {
            return &Recipe;
        }
    }
    return nullptr;
}

void AQuest_CraftingManager::ConsumeIngredients(const FQuest_CraftingRecipe& Recipe)
{
    for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
    {
        RemoveResource(Ingredient.ResourceType, Ingredient.Quantity);
    }
}

FQuest_CraftResultData AQuest_CraftingManager::TryCraft(EQuest_CraftedItem ItemToCraft)
{
    FQuest_CraftResultData Result;

    FQuest_CraftingRecipe* Recipe = FindRecipeForItem(ItemToCraft);
    if (!Recipe)
    {
        Result.Result = EQuest_CraftResult::UnknownRecipe;
        Result.Message = TEXT("Recipe not found.");
        return Result;
    }

    if (!HasIngredientsForRecipe(*Recipe))
    {
        Result.Result = EQuest_CraftResult::InsufficientMaterials;
        Result.Message = FString::Printf(TEXT("Not enough materials to craft %s."), *Recipe->DisplayName);
        return Result;
    }

    ConsumeIngredients(*Recipe);

    Result.Result = EQuest_CraftResult::Success;
    Result.CraftedItem = ItemToCraft;
    Result.Message = FString::Printf(TEXT("Crafted: %s"), *Recipe->DisplayName);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green, Result.Message);
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingManager: %s"), *Result.Message);
    return Result;
}

void AQuest_CraftingManager::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;

    if (GEngine)
    {
        FString Status = bCraftingMenuOpen ? TEXT("OPEN") : TEXT("CLOSED");
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan,
            FString::Printf(TEXT("Crafting Menu: %s"), *Status));
    }

    UE_LOG(LogTemp, Log, TEXT("CraftingManager: Menu %s"), bCraftingMenuOpen ? TEXT("opened") : TEXT("closed"));
}
