#include "QuestCraftingSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

// ============================================================
// AQuest_ResourcePickup
// ============================================================

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ResourceMesh"));
    RootComponent = MeshComponent;

    PickupRadius = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRadius"));
    PickupRadius->SetupAttachment(RootComponent);
    PickupRadius->SetSphereRadius(120.0f);
    PickupRadius->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
}

bool AQuest_ResourcePickup::TryPickup(FQuest_PlayerInventory& OutInventory)
{
    if (bHasBeenPickedUp)
    {
        return false;
    }

    bHasBeenPickedUp = true;

    switch (ResourceType)
    {
        case EQuest_ResourceType::Flint:
            OutInventory.FlintCount += PickupCount;
            break;
        case EQuest_ResourceType::Stick:
            OutInventory.StickCount += PickupCount;
            break;
        case EQuest_ResourceType::Leaf:
            OutInventory.LeafCount += PickupCount;
            break;
        case EQuest_ResourceType::Bone:
            OutInventory.BoneCount += PickupCount;
            break;
        case EQuest_ResourceType::Vine:
            OutInventory.VineCount += PickupCount;
            break;
        case EQuest_ResourceType::Clay:
            OutInventory.ClayCount += PickupCount;
            break;
        case EQuest_ResourceType::Tinder:
            OutInventory.TinderCount += PickupCount;
            break;
        default:
            break;
    }

    // Hide the actor after pickup
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    return true;
}

FString AQuest_ResourcePickup::GetResourceDisplayName() const
{
    switch (ResourceType)
    {
        case EQuest_ResourceType::Flint:    return TEXT("Flint Rock");
        case EQuest_ResourceType::Stick:    return TEXT("Wooden Stick");
        case EQuest_ResourceType::Leaf:     return TEXT("Large Leaf");
        case EQuest_ResourceType::Bone:     return TEXT("Animal Bone");
        case EQuest_ResourceType::Vine:     return TEXT("Vine");
        case EQuest_ResourceType::Clay:     return TEXT("Clay");
        case EQuest_ResourceType::Tinder:   return TEXT("Dry Tinder");
        default:                            return TEXT("Unknown Resource");
    }
}

// ============================================================
// AQuest_CraftingSystemManager
// ============================================================

AQuest_CraftingSystemManager::AQuest_CraftingSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AQuest_CraftingSystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeRecipes();
}

void AQuest_CraftingSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CraftingState == EQuest_CraftingState::Crafting)
    {
        if (SelectedRecipeIndex >= 0 && SelectedRecipeIndex < Recipes.Num())
        {
            CraftingProgress += DeltaTime;
            float Required = Recipes[SelectedRecipeIndex].CraftingTimeSec;
            if (CraftingProgress >= Required)
            {
                OnCraftingComplete();
            }
        }
    }
}

void AQuest_CraftingSystemManager::InitializeRecipes()
{
    Recipes.Empty();

    // ---- Recipe 1: Stone Axe (2 Flint + 1 Stick) ----
    {
        FQuest_CraftingRecipe StoneAxe;
        StoneAxe.OutputItem = EQuest_CraftedItem::StoneAxe;
        StoneAxe.RecipeName = TEXT("Stone Axe");
        StoneAxe.Description = TEXT("A sharp flint head bound to a wooden handle. Essential for chopping wood and fighting off small predators.");
        StoneAxe.CraftingTimeSec = 3.0f;

        FQuest_CraftingIngredient Flint1;
        Flint1.ResourceType = EQuest_ResourceType::Flint;
        Flint1.RequiredCount = 2;
        StoneAxe.Ingredients.Add(Flint1);

        FQuest_CraftingIngredient Stick1;
        Stick1.ResourceType = EQuest_ResourceType::Stick;
        Stick1.RequiredCount = 1;
        StoneAxe.Ingredients.Add(Stick1);

        Recipes.Add(StoneAxe);
    }

    // ---- Recipe 2: Campfire (3 Sticks + 1 Tinder) ----
    {
        FQuest_CraftingRecipe Campfire;
        Campfire.OutputItem = EQuest_CraftedItem::Campfire;
        Campfire.RecipeName = TEXT("Campfire");
        Campfire.Description = TEXT("A small fire that keeps predators away at night and allows cooking of raw meat. Place it near your shelter.");
        Campfire.CraftingTimeSec = 4.0f;

        FQuest_CraftingIngredient Stick2;
        Stick2.ResourceType = EQuest_ResourceType::Stick;
        Stick2.RequiredCount = 3;
        Campfire.Ingredients.Add(Stick2);

        FQuest_CraftingIngredient Tinder1;
        Tinder1.ResourceType = EQuest_ResourceType::Tinder;
        Tinder1.RequiredCount = 1;
        Campfire.Ingredients.Add(Tinder1);

        Recipes.Add(Campfire);
    }

    // ---- Recipe 3: Water Container (1 Clay + 1 Leaf) ----
    {
        FQuest_CraftingRecipe WaterContainer;
        WaterContainer.OutputItem = EQuest_CraftedItem::WaterContainer;
        WaterContainer.RecipeName = TEXT("Water Container");
        WaterContainer.Description = TEXT("A clay bowl lined with large leaves. Holds enough water for one day of travel. Find a river to fill it.");
        WaterContainer.CraftingTimeSec = 5.0f;

        FQuest_CraftingIngredient Clay1;
        Clay1.ResourceType = EQuest_ResourceType::Clay;
        Clay1.RequiredCount = 1;
        WaterContainer.Ingredients.Add(Clay1);

        FQuest_CraftingIngredient Leaf1;
        Leaf1.ResourceType = EQuest_ResourceType::Leaf;
        Leaf1.RequiredCount = 1;
        WaterContainer.Ingredients.Add(Leaf1);

        Recipes.Add(WaterContainer);
    }

    // ---- Recipe 4: Bone-Tipped Spear (2 Sticks + 1 Bone + 1 Vine) ----
    {
        FQuest_CraftingRecipe BoneSpear;
        BoneSpear.OutputItem = EQuest_CraftedItem::BoneTip;
        BoneSpear.RecipeName = TEXT("Bone-Tipped Spear");
        BoneSpear.Description = TEXT("A long spear with a sharpened bone tip. More effective against large dinosaurs than a stone axe. Keep distance.");
        BoneSpear.CraftingTimeSec = 6.0f;

        FQuest_CraftingIngredient Stick3;
        Stick3.ResourceType = EQuest_ResourceType::Stick;
        Stick3.RequiredCount = 2;
        BoneSpear.Ingredients.Add(Stick3);

        FQuest_CraftingIngredient Bone1;
        Bone1.ResourceType = EQuest_ResourceType::Bone;
        Bone1.RequiredCount = 1;
        BoneSpear.Ingredients.Add(Bone1);

        FQuest_CraftingIngredient Vine1;
        Vine1.ResourceType = EQuest_ResourceType::Vine;
        Vine1.RequiredCount = 1;
        BoneSpear.Ingredients.Add(Vine1);

        Recipes.Add(BoneSpear);
    }

    // ---- Recipe 5: Torch (1 Stick + 1 Tinder + 1 Vine) ----
    {
        FQuest_CraftingRecipe Torch;
        Torch.OutputItem = EQuest_CraftedItem::Torch;
        Torch.RecipeName = TEXT("Torch");
        Torch.Description = TEXT("A burning torch that scares off predators and lights dark caves. Burns for 5 minutes before going out.");
        Torch.CraftingTimeSec = 2.5f;

        FQuest_CraftingIngredient Stick4;
        Stick4.ResourceType = EQuest_ResourceType::Stick;
        Stick4.RequiredCount = 1;
        Torch.Ingredients.Add(Stick4);

        FQuest_CraftingIngredient Tinder2;
        Tinder2.ResourceType = EQuest_ResourceType::Tinder;
        Tinder2.RequiredCount = 1;
        Torch.Ingredients.Add(Tinder2);

        FQuest_CraftingIngredient Vine2;
        Vine2.ResourceType = EQuest_ResourceType::Vine;
        Vine2.RequiredCount = 1;
        Torch.Ingredients.Add(Vine2);

        Recipes.Add(Torch);
    }
}

bool AQuest_CraftingSystemManager::CanCraft(int32 RecipeIndex) const
{
    if (!Recipes.IsValidIndex(RecipeIndex))
    {
        return false;
    }

    const FQuest_CraftingRecipe& Recipe = Recipes[RecipeIndex];

    for (const FQuest_CraftingIngredient& Ingredient : Recipe.Ingredients)
    {
        if (GetResourceCount(Ingredient.ResourceType) < Ingredient.RequiredCount)
        {
            return false;
        }
    }

    return true;
}

bool AQuest_CraftingSystemManager::TryCraft(int32 RecipeIndex)
{
    if (!CanCraft(RecipeIndex))
    {
        CraftingState = EQuest_CraftingState::Failed;
        return false;
    }

    SelectedRecipeIndex = RecipeIndex;
    CraftingState = EQuest_CraftingState::Crafting;
    CraftingProgress = 0.0f;

    return true;
}

void AQuest_CraftingSystemManager::OnCraftingComplete()
{
    if (!Recipes.IsValidIndex(SelectedRecipeIndex))
    {
        return;
    }

    ConsumeMaterials(SelectedRecipeIndex);
    PlayerInventory.CraftedItems.AddUnique(Recipes[SelectedRecipeIndex].OutputItem);

    CraftingState = EQuest_CraftingState::Success;
    CraftingProgress = 0.0f;
    SelectedRecipeIndex = -1;
}

void AQuest_CraftingSystemManager::ConsumeMaterials(int32 RecipeIndex)
{
    if (!Recipes.IsValidIndex(RecipeIndex))
    {
        return;
    }

    for (const FQuest_CraftingIngredient& Ingredient : Recipes[RecipeIndex].Ingredients)
    {
        switch (Ingredient.ResourceType)
        {
            case EQuest_ResourceType::Flint:
                PlayerInventory.FlintCount = FMath::Max(0, PlayerInventory.FlintCount - Ingredient.RequiredCount);
                break;
            case EQuest_ResourceType::Stick:
                PlayerInventory.StickCount = FMath::Max(0, PlayerInventory.StickCount - Ingredient.RequiredCount);
                break;
            case EQuest_ResourceType::Leaf:
                PlayerInventory.LeafCount = FMath::Max(0, PlayerInventory.LeafCount - Ingredient.RequiredCount);
                break;
            case EQuest_ResourceType::Bone:
                PlayerInventory.BoneCount = FMath::Max(0, PlayerInventory.BoneCount - Ingredient.RequiredCount);
                break;
            case EQuest_ResourceType::Vine:
                PlayerInventory.VineCount = FMath::Max(0, PlayerInventory.VineCount - Ingredient.RequiredCount);
                break;
            case EQuest_ResourceType::Clay:
                PlayerInventory.ClayCount = FMath::Max(0, PlayerInventory.ClayCount - Ingredient.RequiredCount);
                break;
            case EQuest_ResourceType::Tinder:
                PlayerInventory.TinderCount = FMath::Max(0, PlayerInventory.TinderCount - Ingredient.RequiredCount);
                break;
            default:
                break;
        }
    }
}

void AQuest_CraftingSystemManager::OpenCraftingMenu()
{
    CraftingState = EQuest_CraftingState::MenuOpen;
}

void AQuest_CraftingSystemManager::CloseCraftingMenu()
{
    CraftingState = EQuest_CraftingState::Idle;
    CraftingProgress = 0.0f;
    SelectedRecipeIndex = -1;
}

TArray<FQuest_CraftingRecipe> AQuest_CraftingSystemManager::GetAvailableRecipes() const
{
    return Recipes;
}

int32 AQuest_CraftingSystemManager::GetResourceCount(EQuest_ResourceType ResourceType) const
{
    switch (ResourceType)
    {
        case EQuest_ResourceType::Flint:    return PlayerInventory.FlintCount;
        case EQuest_ResourceType::Stick:    return PlayerInventory.StickCount;
        case EQuest_ResourceType::Leaf:     return PlayerInventory.LeafCount;
        case EQuest_ResourceType::Bone:     return PlayerInventory.BoneCount;
        case EQuest_ResourceType::Vine:     return PlayerInventory.VineCount;
        case EQuest_ResourceType::Clay:     return PlayerInventory.ClayCount;
        case EQuest_ResourceType::Tinder:   return PlayerInventory.TinderCount;
        default:                            return 0;
    }
}

void AQuest_CraftingSystemManager::AddResource(EQuest_ResourceType ResourceType, int32 Count)
{
    switch (ResourceType)
    {
        case EQuest_ResourceType::Flint:    PlayerInventory.FlintCount  += Count; break;
        case EQuest_ResourceType::Stick:    PlayerInventory.StickCount  += Count; break;
        case EQuest_ResourceType::Leaf:     PlayerInventory.LeafCount   += Count; break;
        case EQuest_ResourceType::Bone:     PlayerInventory.BoneCount   += Count; break;
        case EQuest_ResourceType::Vine:     PlayerInventory.VineCount   += Count; break;
        case EQuest_ResourceType::Clay:     PlayerInventory.ClayCount   += Count; break;
        case EQuest_ResourceType::Tinder:   PlayerInventory.TinderCount += Count; break;
        default: break;
    }
}

bool AQuest_CraftingSystemManager::HasCraftedItem(EQuest_CraftedItem Item) const
{
    return PlayerInventory.CraftedItems.Contains(Item);
}

bool AQuest_CraftingSystemManager::HasCraftedStoneAxe() const
{
    return HasCraftedItem(EQuest_CraftedItem::StoneAxe);
}

bool AQuest_CraftingSystemManager::HasCraftedCampfire() const
{
    return HasCraftedItem(EQuest_CraftedItem::Campfire);
}

bool AQuest_CraftingSystemManager::HasCraftedTorch() const
{
    return HasCraftedItem(EQuest_CraftedItem::Torch);
}
