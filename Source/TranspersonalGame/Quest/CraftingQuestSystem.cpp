#include "Quest/CraftingQuestSystem.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

// ── FQuest_PlayerInventory ────────────────────────────────────────────────────

int32 FQuest_PlayerInventory::GetCount(EQuest_ResourceType Type) const
{
    switch (Type)
    {
    case EQuest_ResourceType::Stone:  return Stones;
    case EQuest_ResourceType::Stick:  return Sticks;
    case EQuest_ResourceType::Leaf:   return Leaves;
    case EQuest_ResourceType::Bone:   return Bones;
    case EQuest_ResourceType::Flint:  return Flints;
    default: return 0;
    }
}

bool FQuest_PlayerInventory::ConsumeIngredients(const TArray<FQuest_RecipeIngredient>& Ingredients)
{
    // First verify we have enough
    for (const FQuest_RecipeIngredient& Ing : Ingredients)
    {
        if (GetCount(Ing.ResourceType) < Ing.RequiredCount)
            return false;
    }
    // Then consume
    for (const FQuest_RecipeIngredient& Ing : Ingredients)
    {
        switch (Ing.ResourceType)
        {
        case EQuest_ResourceType::Stone:  Stones  -= Ing.RequiredCount; break;
        case EQuest_ResourceType::Stick:  Sticks  -= Ing.RequiredCount; break;
        case EQuest_ResourceType::Leaf:   Leaves  -= Ing.RequiredCount; break;
        case EQuest_ResourceType::Bone:   Bones   -= Ing.RequiredCount; break;
        case EQuest_ResourceType::Flint:  Flints  -= Ing.RequiredCount; break;
        default: break;
        }
    }
    return true;
}

// ── AQuest_ResourcePickup ─────────────────────────────────────────────────────

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
    if (!bAvailable) return;
    if (!OtherActor) return;

    // Only player characters pick up resources
    ACharacter* Char = Cast<ACharacter>(OtherActor);
    if (!Char) return;

    TryPickup(OtherActor);
}

bool AQuest_ResourcePickup::TryPickup(AActor* Collector)
{
    if (!bAvailable || !Collector) return false;

    bAvailable = false;
    MeshComponent->SetVisibility(false);
    PickupRadius->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    UE_LOG(LogTemp, Log, TEXT("ResourcePickup: %s collected %d x %s"),
        *Collector->GetName(),
        Quantity,
        *UEnum::GetValueAsString(ResourceType));

    if (bAutoRespawn)
    {
        GetWorldTimerManager().SetTimer(RespawnTimer, this,
            &AQuest_ResourcePickup::Respawn, RespawnDelaySeconds, false);
    }

    return true;
}

void AQuest_ResourcePickup::Respawn()
{
    bAvailable = true;
    MeshComponent->SetVisibility(true);
    PickupRadius->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    UE_LOG(LogTemp, Log, TEXT("ResourcePickup: respawned at %s"), *GetActorLocation().ToString());
}

// ── AQuest_CraftingManager ────────────────────────────────────────────────────

AQuest_CraftingManager::AQuest_CraftingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
}

void AQuest_CraftingManager::BeginPlay()
{
    Super::BeginPlay();
    InitDefaultRecipes();
    UE_LOG(LogTemp, Log, TEXT("CraftingQuestManager: initialized with %d recipes"), AvailableRecipes.Num());
}

void AQuest_CraftingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (CurrentPhase == EQuest_CraftingPhase::CraftItem)
    {
        TickCrafting(DeltaTime);
    }
}

void AQuest_CraftingManager::InitDefaultRecipes()
{
    AvailableRecipes.Empty();

    // ── Recipe 1: Stone Axe (2 Stones + 1 Stick) ─────────────────────────────
    {
        FQuest_CraftingRecipe Axe;
        Axe.OutputItem        = EQuest_CraftedItem::StoneAxe;
        Axe.RecipeName        = TEXT("Stone Axe");
        Axe.Description       = TEXT("A crude but effective chopping tool. Useful for cutting wood and defending against small predators.");
        Axe.CraftingTimeSeconds = 4.0f;
        Axe.bUnlocked         = true;

        FQuest_RecipeIngredient StoneIng;
        StoneIng.ResourceType  = EQuest_ResourceType::Stone;
        StoneIng.RequiredCount = 2;

        FQuest_RecipeIngredient StickIng;
        StickIng.ResourceType  = EQuest_ResourceType::Stick;
        StickIng.RequiredCount = 1;

        Axe.Ingredients.Add(StoneIng);
        Axe.Ingredients.Add(StickIng);
        AvailableRecipes.Add(Axe);
    }

    // ── Recipe 2: Campfire (3 Sticks) ────────────────────────────────────────
    {
        FQuest_CraftingRecipe Fire;
        Fire.OutputItem        = EQuest_CraftedItem::Campfire;
        Fire.RecipeName        = TEXT("Campfire");
        Fire.Description       = TEXT("A basic fire for warmth and cooking. Keeps predators at bay during the night.");
        Fire.CraftingTimeSeconds = 5.0f;
        Fire.bUnlocked         = true;

        FQuest_RecipeIngredient StickIng;
        StickIng.ResourceType  = EQuest_ResourceType::Stick;
        StickIng.RequiredCount = 3;

        Fire.Ingredients.Add(StickIng);
        AvailableRecipes.Add(Fire);
    }

    // ── Recipe 3: Water Container (1 Stone + 1 Leaf) ─────────────────────────
    {
        FQuest_CraftingRecipe Container;
        Container.OutputItem        = EQuest_CraftedItem::WaterContainer;
        Container.RecipeName        = TEXT("Water Container");
        Container.Description       = TEXT("A hollowed stone lined with large leaves. Holds enough water for one day of travel.");
        Container.CraftingTimeSeconds = 3.0f;
        Container.bUnlocked         = true;

        FQuest_RecipeIngredient StoneIng;
        StoneIng.ResourceType  = EQuest_ResourceType::Stone;
        StoneIng.RequiredCount = 1;

        FQuest_RecipeIngredient LeafIng;
        LeafIng.ResourceType  = EQuest_ResourceType::Leaf;
        LeafIng.RequiredCount = 1;

        Container.Ingredients.Add(StoneIng);
        Container.Ingredients.Add(LeafIng);
        AvailableRecipes.Add(Container);
    }

    // ── Recipe 4: Spear Tip (2 Flints) ───────────────────────────────────────
    {
        FQuest_CraftingRecipe Spear;
        Spear.OutputItem        = EQuest_CraftedItem::SpearTip;
        Spear.RecipeName        = TEXT("Spear Tip");
        Spear.Description       = TEXT("A razor-sharp flint point. Attach it to a long stick to make a hunting spear.");
        Spear.CraftingTimeSeconds = 6.0f;
        Spear.bUnlocked         = false; // Unlocked after crafting Stone Axe

        FQuest_RecipeIngredient FlintIng;
        FlintIng.ResourceType  = EQuest_ResourceType::Flint;
        FlintIng.RequiredCount = 2;

        Spear.Ingredients.Add(FlintIng);
        AvailableRecipes.Add(Spear);
    }
}

void AQuest_CraftingManager::OpenCraftingMenu()
{
    bCraftingMenuOpen = true;
    UE_LOG(LogTemp, Log, TEXT("CraftingMenu: opened — %d recipes available"), GetCraftableRecipes().Num());
}

void AQuest_CraftingManager::CloseCraftingMenu()
{
    bCraftingMenuOpen = false;
    if (CurrentPhase == EQuest_CraftingPhase::CraftItem)
    {
        CancelCrafting();
    }
    UE_LOG(LogTemp, Log, TEXT("CraftingMenu: closed"));
}

bool AQuest_CraftingManager::CanCraft(EQuest_CraftedItem Item) const
{
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.OutputItem != Item || !Recipe.bUnlocked) continue;

        for (const FQuest_RecipeIngredient& Ing : Recipe.Ingredients)
        {
            if (PlayerInventory.GetCount(Ing.ResourceType) < Ing.RequiredCount)
                return false;
        }
        return true;
    }
    return false;
}

bool AQuest_CraftingManager::StartCrafting(EQuest_CraftedItem Item)
{
    if (CurrentPhase == EQuest_CraftingPhase::CraftItem) return false;
    if (!CanCraft(Item)) return false;

    FQuest_CraftingRecipe* Recipe = FindRecipe(Item);
    if (!Recipe) return false;

    ActiveCraftTarget    = Item;
    ActiveCraftDuration  = Recipe->CraftingTimeSeconds;
    CraftingTimer        = 0.0f;
    CraftingProgress     = 0.0f;
    CurrentPhase         = EQuest_CraftingPhase::CraftItem;

    UE_LOG(LogTemp, Log, TEXT("CraftingManager: started crafting %s (%.1fs)"),
        *Recipe->RecipeName, ActiveCraftDuration);
    return true;
}

void AQuest_CraftingManager::CancelCrafting()
{
    ActiveCraftTarget   = EQuest_CraftedItem::None;
    CraftingTimer       = 0.0f;
    CraftingProgress    = 0.0f;
    CurrentPhase        = EQuest_CraftingPhase::Idle;
    UE_LOG(LogTemp, Log, TEXT("CraftingManager: crafting cancelled"));
}

void AQuest_CraftingManager::TickCrafting(float DeltaTime)
{
    CraftingTimer    += DeltaTime;
    CraftingProgress  = FMath::Clamp(CraftingTimer / ActiveCraftDuration, 0.0f, 1.0f);

    if (CraftingTimer >= ActiveCraftDuration)
    {
        CompleteCrafting();
    }
}

void AQuest_CraftingManager::CompleteCrafting()
{
    FQuest_CraftingRecipe* Recipe = FindRecipe(ActiveCraftTarget);
    if (!Recipe) { CancelCrafting(); return; }

    // Consume ingredients
    if (!PlayerInventory.ConsumeIngredients(Recipe->Ingredients))
    {
        UE_LOG(LogTemp, Warning, TEXT("CraftingManager: ingredient check failed at completion"));
        CancelCrafting();
        return;
    }

    EQuest_CraftedItem Crafted = ActiveCraftTarget;

    // Unlock spear tip after first stone axe
    if (Crafted == EQuest_CraftedItem::StoneAxe)
    {
        for (FQuest_CraftingRecipe& R : AvailableRecipes)
        {
            if (R.OutputItem == EQuest_CraftedItem::SpearTip)
            {
                R.bUnlocked = true;
                UE_LOG(LogTemp, Log, TEXT("CraftingManager: Spear Tip recipe unlocked"));
            }
        }
    }

    // First tool quest completion
    if (bFirstToolQuestActive && !bFirstToolQuestComplete)
    {
        bFirstToolQuestComplete = true;
        UE_LOG(LogTemp, Log, TEXT("CraftingManager: FIRST TOOL QUEST COMPLETE — crafted %s"), *Recipe->RecipeName);
    }

    CurrentPhase     = EQuest_CraftingPhase::Completed;
    CraftingProgress = 1.0f;

    OnItemCrafted.Broadcast(Crafted);
    UE_LOG(LogTemp, Log, TEXT("CraftingManager: crafted %s"), *Recipe->RecipeName);

    // Reset to idle after brief delay
    GetWorldTimerManager().SetTimer(
        FTimerHandle{},
        [this]() { CurrentPhase = EQuest_CraftingPhase::Idle; CraftingProgress = 0.0f; },
        1.5f, false);
}

void AQuest_CraftingManager::AddResource(EQuest_ResourceType Type, int32 Amount)
{
    switch (Type)
    {
    case EQuest_ResourceType::Stone:  PlayerInventory.Stones  += Amount; break;
    case EQuest_ResourceType::Stick:  PlayerInventory.Sticks  += Amount; break;
    case EQuest_ResourceType::Leaf:   PlayerInventory.Leaves  += Amount; break;
    case EQuest_ResourceType::Bone:   PlayerInventory.Bones   += Amount; break;
    case EQuest_ResourceType::Flint:  PlayerInventory.Flints  += Amount; break;
    default: break;
    }

    OnResourceCollected.Broadcast(Type, PlayerInventory.GetCount(Type));
    UE_LOG(LogTemp, Log, TEXT("Inventory: +%d %s (total: %d)"),
        Amount,
        *UEnum::GetValueAsString(Type),
        PlayerInventory.GetCount(Type));
}

FQuest_CraftingRecipe AQuest_CraftingManager::GetRecipe(EQuest_CraftedItem Item) const
{
    for (const FQuest_CraftingRecipe& R : AvailableRecipes)
    {
        if (R.OutputItem == Item) return R;
    }
    return FQuest_CraftingRecipe{};
}

TArray<FQuest_CraftingRecipe> AQuest_CraftingManager::GetCraftableRecipes() const
{
    TArray<FQuest_CraftingRecipe> Result;
    for (const FQuest_CraftingRecipe& R : AvailableRecipes)
    {
        if (R.bUnlocked) Result.Add(R);
    }
    return Result;
}

void AQuest_CraftingManager::StartFirstToolQuest()
{
    bFirstToolQuestActive   = true;
    bFirstToolQuestComplete = false;
    CurrentPhase            = EQuest_CraftingPhase::GatherMaterials;
    UE_LOG(LogTemp, Log, TEXT("CraftingManager: First Tool Quest started — gather 2 stones and 1 stick"));
}

FQuest_CraftingRecipe* AQuest_CraftingManager::FindRecipe(EQuest_CraftedItem Item)
{
    for (FQuest_CraftingRecipe& R : AvailableRecipes)
    {
        if (R.OutputItem == Item) return &R;
    }
    return nullptr;
}
