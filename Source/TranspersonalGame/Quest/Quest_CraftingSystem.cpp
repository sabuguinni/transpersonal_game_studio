#include "Quest_CraftingSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// AQuest_ResourcePickup Implementation
AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;
    
    ResourceName = TEXT("Stone");
    ResourceQuantity = 1;
    RespawnTime = 300.0f; // 5 minutes
    bCanRespawn = true;
    InteractionRange = 200.0f;
    bIsCollected = false;

    // Set up static mesh component
    if (GetStaticMeshComponent())
    {
        GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
    }
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
    
    // Set random resource type based on location
    FVector Location = GetActorLocation();
    if (Location.Z > 1000.0f) // Mountain biome
    {
        ResourceName = TEXT("Stone");
        ResourceQuantity = FMath::RandRange(1, 3);
    }
    else if (FMath::Abs(Location.X) < 10000.0f && FMath::Abs(Location.Y) < 10000.0f) // Savanna
    {
        ResourceName = TEXT("Stick");
        ResourceQuantity = FMath::RandRange(1, 2);
    }
    else
    {
        ResourceName = TEXT("Leaf");
        ResourceQuantity = 1;
    }
}

void AQuest_ResourcePickup::OnPlayerInteract(APawn* InteractingPawn)
{
    if (bIsCollected || !InteractingPawn)
    {
        return;
    }

    // Try to add item to player inventory
    UQuest_CraftingComponent* CraftingComp = InteractingPawn->FindComponentByClass<UQuest_CraftingComponent>();
    if (CraftingComp)
    {
        if (CraftingComp->AddItemToInventory(ResourceName, ResourceQuantity))
        {
            bIsCollected = true;
            SetActorHiddenInGame(true);
            SetActorEnableCollision(false);

            UE_LOG(LogTemp, Warning, TEXT("Player collected %d %s"), ResourceQuantity, *ResourceName);

            // Start respawn timer if enabled
            if (bCanRespawn && RespawnTime > 0.0f)
            {
                GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &AQuest_ResourcePickup::RespawnResource, RespawnTime, false);
            }
        }
    }
}

void AQuest_ResourcePickup::RespawnResource()
{
    bIsCollected = false;
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    
    UE_LOG(LogTemp, Warning, TEXT("Resource %s respawned at location %s"), *ResourceName, *GetActorLocation().ToString());
}

// UQuest_CraftingComponent Implementation
UQuest_CraftingComponent::UQuest_CraftingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CraftingSkillLevel = 1.0f;
    bIsCrafting = false;
    CurrentCraftingRecipe = TEXT("");
}

void UQuest_CraftingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBasicRecipes();
    
    // Register with crafting system
    if (UWorld* World = GetWorld())
    {
        if (UQuest_CraftingSystem* CraftingSystem = World->GetSubsystem<UQuest_CraftingSystem>())
        {
            CraftingSystem->RegisterCraftingComponent(this);
        }
    }
}

void UQuest_CraftingComponent::InitializeBasicRecipes()
{
    AvailableRecipes.Empty();

    // Stone Axe Recipe
    FQuest_CraftingRecipe StoneAxe;
    StoneAxe.RecipeName = TEXT("Stone Axe");
    StoneAxe.RequiredMaterials = {TEXT("Stone"), TEXT("Stick")};
    StoneAxe.MaterialQuantities = {2, 1};
    StoneAxe.ResultItem = TEXT("Stone Axe");
    StoneAxe.CraftingTime = 10.0f;
    StoneAxe.ExperienceGained = 25;
    AvailableRecipes.Add(StoneAxe);

    // Campfire Recipe
    FQuest_CraftingRecipe Campfire;
    Campfire.RecipeName = TEXT("Campfire");
    Campfire.RequiredMaterials = {TEXT("Stick"), TEXT("Stone")};
    Campfire.MaterialQuantities = {3, 1};
    Campfire.ResultItem = TEXT("Campfire");
    Campfire.CraftingTime = 15.0f;
    Campfire.ExperienceGained = 30;
    AvailableRecipes.Add(Campfire);

    // Water Container Recipe
    FQuest_CraftingRecipe WaterContainer;
    WaterContainer.RecipeName = TEXT("Water Container");
    WaterContainer.RequiredMaterials = {TEXT("Stone"), TEXT("Leaf")};
    WaterContainer.MaterialQuantities = {1, 1};
    WaterContainer.ResultItem = TEXT("Water Container");
    WaterContainer.CraftingTime = 8.0f;
    WaterContainer.ExperienceGained = 20;
    AvailableRecipes.Add(WaterContainer);

    // Spear Recipe
    FQuest_CraftingRecipe Spear;
    Spear.RecipeName = TEXT("Stone Spear");
    Spear.RequiredMaterials = {TEXT("Stick"), TEXT("Stone")};
    Spear.MaterialQuantities = {1, 1};
    Spear.ResultItem = TEXT("Stone Spear");
    Spear.CraftingTime = 12.0f;
    Spear.ExperienceGained = 35;
    AvailableRecipes.Add(Spear);

    // Shelter Recipe
    FQuest_CraftingRecipe Shelter;
    Shelter.RecipeName = TEXT("Basic Shelter");
    Shelter.RequiredMaterials = {TEXT("Stick"), TEXT("Leaf"), TEXT("Stone")};
    Shelter.MaterialQuantities = {5, 3, 2};
    Shelter.ResultItem = TEXT("Basic Shelter");
    Shelter.CraftingTime = 30.0f;
    Shelter.ExperienceGained = 50;
    AvailableRecipes.Add(Shelter);
}

bool UQuest_CraftingComponent::CanCraftItem(const FString& RecipeName)
{
    if (bIsCrafting)
    {
        return false;
    }

    // Find recipe
    FQuest_CraftingRecipe* Recipe = AvailableRecipes.FindByPredicate([&RecipeName](const FQuest_CraftingRecipe& R) {
        return R.RecipeName == RecipeName;
    });

    if (!Recipe)
    {
        return false;
    }

    // Check if player has required materials
    for (int32 i = 0; i < Recipe->RequiredMaterials.Num(); i++)
    {
        FString MaterialName = Recipe->RequiredMaterials[i];
        int32 RequiredQuantity = Recipe->MaterialQuantities.IsValidIndex(i) ? Recipe->MaterialQuantities[i] : 1;
        
        if (GetItemQuantity(MaterialName) < RequiredQuantity)
        {
            return false;
        }
    }

    return true;
}

bool UQuest_CraftingComponent::StartCrafting(const FString& RecipeName)
{
    if (!CanCraftItem(RecipeName))
    {
        return false;
    }

    // Find recipe
    FQuest_CraftingRecipe* Recipe = AvailableRecipes.FindByPredicate([&RecipeName](const FQuest_CraftingRecipe& R) {
        return R.RecipeName == RecipeName;
    });

    if (!Recipe)
    {
        return false;
    }

    // Consume materials
    for (int32 i = 0; i < Recipe->RequiredMaterials.Num(); i++)
    {
        FString MaterialName = Recipe->RequiredMaterials[i];
        int32 RequiredQuantity = Recipe->MaterialQuantities.IsValidIndex(i) ? Recipe->MaterialQuantities[i] : 1;
        
        if (!RemoveItemFromInventory(MaterialName, RequiredQuantity))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to remove material %s for crafting"), *MaterialName);
            return false;
        }
    }

    // Start crafting process
    bIsCrafting = true;
    CurrentCraftingRecipe = RecipeName;
    
    UE_LOG(LogTemp, Warning, TEXT("Started crafting %s (%.1f seconds)"), *RecipeName, Recipe->CraftingTime);

    // Set timer for completion
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(CraftingTimerHandle, this, &UQuest_CraftingComponent::CompleteCrafting, Recipe->CraftingTime, false);
    }

    return true;
}

void UQuest_CraftingComponent::CompleteCrafting()
{
    if (!bIsCrafting || CurrentCraftingRecipe.IsEmpty())
    {
        return;
    }

    // Find recipe
    FQuest_CraftingRecipe* Recipe = AvailableRecipes.FindByPredicate([this](const FQuest_CraftingRecipe& R) {
        return R.RecipeName == CurrentCraftingRecipe;
    });

    if (Recipe)
    {
        // Add crafted item to inventory
        AddItemToInventory(Recipe->ResultItem, 1);
        
        // Gain experience
        CraftingSkillLevel += Recipe->ExperienceGained * 0.01f;
        
        UE_LOG(LogTemp, Warning, TEXT("Completed crafting %s! Skill level now %.2f"), *Recipe->ResultItem, CraftingSkillLevel);
    }

    // Reset crafting state
    bIsCrafting = false;
    CurrentCraftingRecipe = TEXT("");
    GetWorld()->GetTimerManager().ClearTimer(CraftingTimerHandle);
}

bool UQuest_CraftingComponent::AddItemToInventory(const FString& ItemName, int32 Quantity)
{
    if (Quantity <= 0)
    {
        return false;
    }

    // Check if item already exists
    FQuest_InventoryItem* ExistingItem = PlayerInventory.FindByPredicate([&ItemName](const FQuest_InventoryItem& Item) {
        return Item.ItemName == ItemName;
    });

    if (ExistingItem)
    {
        ExistingItem->Quantity += Quantity;
    }
    else
    {
        // Create new inventory item
        FQuest_InventoryItem NewItem;
        NewItem.ItemName = ItemName;
        NewItem.Quantity = Quantity;
        
        // Set item type based on name
        if (ItemName.Contains(TEXT("Axe")) || ItemName.Contains(TEXT("Spear")))
        {
            NewItem.ItemType = TEXT("Tool");
            NewItem.Weight = 2.0f;
        }
        else if (ItemName.Contains(TEXT("Container")) || ItemName.Contains(TEXT("Campfire")))
        {
            NewItem.ItemType = TEXT("Equipment");
            NewItem.Weight = 3.0f;
        }
        else if (ItemName.Contains(TEXT("Shelter")))
        {
            NewItem.ItemType = TEXT("Structure");
            NewItem.Weight = 10.0f;
        }
        else
        {
            NewItem.ItemType = TEXT("Material");
            NewItem.Weight = 1.0f;
        }
        
        PlayerInventory.Add(NewItem);
    }

    UE_LOG(LogTemp, Warning, TEXT("Added %d %s to inventory"), Quantity, *ItemName);
    return true;
}

bool UQuest_CraftingComponent::RemoveItemFromInventory(const FString& ItemName, int32 Quantity)
{
    if (Quantity <= 0)
    {
        return false;
    }

    FQuest_InventoryItem* Item = PlayerInventory.FindByPredicate([&ItemName](const FQuest_InventoryItem& InvItem) {
        return InvItem.ItemName == ItemName;
    });

    if (!Item || Item->Quantity < Quantity)
    {
        return false;
    }

    Item->Quantity -= Quantity;
    
    // Remove item if quantity reaches zero
    if (Item->Quantity <= 0)
    {
        PlayerInventory.RemoveAll([&ItemName](const FQuest_InventoryItem& InvItem) {
            return InvItem.ItemName == ItemName;
        });
    }

    UE_LOG(LogTemp, Warning, TEXT("Removed %d %s from inventory"), Quantity, *ItemName);
    return true;
}

int32 UQuest_CraftingComponent::GetItemQuantity(const FString& ItemName)
{
    FQuest_InventoryItem* Item = PlayerInventory.FindByPredicate([&ItemName](const FQuest_InventoryItem& InvItem) {
        return InvItem.ItemName == ItemName;
    });

    return Item ? Item->Quantity : 0;
}

void UQuest_CraftingComponent::OpenCraftingMenu()
{
    UE_LOG(LogTemp, Warning, TEXT("Opening crafting menu - %d recipes available"), AvailableRecipes.Num());
    
    // Log current inventory
    UE_LOG(LogTemp, Warning, TEXT("Current inventory:"));
    for (const FQuest_InventoryItem& Item : PlayerInventory)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s: %d"), *Item.ItemName, Item.Quantity);
    }
    
    // Log available recipes
    UE_LOG(LogTemp, Warning, TEXT("Available recipes:"));
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        bool bCanCraft = CanCraftItem(Recipe.RecipeName);
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s"), *Recipe.RecipeName, bCanCraft ? TEXT("CAN CRAFT") : TEXT("Missing materials"));
    }
}

// UQuest_CraftingSystem Implementation
void UQuest_CraftingSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CreateBasicSurvivalRecipes();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Crafting System initialized"));
}

void UQuest_CraftingSystem::Deinitialize()
{
    RegisteredCraftingComponents.Empty();
    SpawnedResources.Empty();
    
    Super::Deinitialize();
}

void UQuest_CraftingSystem::CreateBasicSurvivalRecipes()
{
    BasicRecipes.Empty();

    // Same recipes as in component for reference
    FQuest_CraftingRecipe StoneAxe;
    StoneAxe.RecipeName = TEXT("Stone Axe");
    StoneAxe.RequiredMaterials = {TEXT("Stone"), TEXT("Stick")};
    StoneAxe.MaterialQuantities = {2, 1};
    StoneAxe.ResultItem = TEXT("Stone Axe");
    StoneAxe.CraftingTime = 10.0f;
    StoneAxe.ExperienceGained = 25;
    BasicRecipes.Add(StoneAxe);

    FQuest_CraftingRecipe Campfire;
    Campfire.RecipeName = TEXT("Campfire");
    Campfire.RequiredMaterials = {TEXT("Stick"), TEXT("Stone")};
    Campfire.MaterialQuantities = {3, 1};
    Campfire.ResultItem = TEXT("Campfire");
    Campfire.CraftingTime = 15.0f;
    Campfire.ExperienceGained = 30;
    BasicRecipes.Add(Campfire);

    FQuest_CraftingRecipe WaterContainer;
    WaterContainer.RecipeName = TEXT("Water Container");
    WaterContainer.RequiredMaterials = {TEXT("Stone"), TEXT("Leaf")};
    WaterContainer.MaterialQuantities = {1, 1};
    WaterContainer.ResultItem = TEXT("Water Container");
    WaterContainer.CraftingTime = 8.0f;
    WaterContainer.ExperienceGained = 20;
    BasicRecipes.Add(WaterContainer);
}

TArray<FQuest_CraftingRecipe> UQuest_CraftingSystem::GetBasicSurvivalRecipes()
{
    return BasicRecipes;
}

void UQuest_CraftingSystem::RegisterCraftingComponent(UQuest_CraftingComponent* Component)
{
    if (Component && !RegisteredCraftingComponents.Contains(Component))
    {
        RegisteredCraftingComponents.Add(Component);
        UE_LOG(LogTemp, Warning, TEXT("Registered crafting component - Total: %d"), RegisteredCraftingComponents.Num());
    }
}

void UQuest_CraftingSystem::UnregisterCraftingComponent(UQuest_CraftingComponent* Component)
{
    if (Component)
    {
        RegisteredCraftingComponents.Remove(Component);
        UE_LOG(LogTemp, Warning, TEXT("Unregistered crafting component - Total: %d"), RegisteredCraftingComponents.Num());
    }
}

void UQuest_CraftingSystem::SpawnResourcesInBiome(EBiomeType BiomeType, int32 ResourceCount)
{
    if (ResourceCount <= 0)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (int32 i = 0; i < ResourceCount; i++)
    {
        FVector SpawnLocation = GetRandomLocationInBiome(BiomeType);
        
        // Determine resource type based on biome
        FString ResourceType;
        switch (BiomeType)
        {
            case EBiomeType::Mountain:
                ResourceType = TEXT("Stone");
                break;
            case EBiomeType::Forest:
                ResourceType = TEXT("Stick");
                break;
            case EBiomeType::Swamp:
                ResourceType = TEXT("Leaf");
                break;
            case EBiomeType::Desert:
                ResourceType = TEXT("Stone");
                break;
            default: // Savanna
                ResourceType = (FMath::RandBool() ? TEXT("Stick") : TEXT("Stone"));
                break;
        }
        
        SpawnResourceAtLocation(SpawnLocation, ResourceType);
    }

    UE_LOG(LogTemp, Warning, TEXT("Spawned %d resources in biome %d"), ResourceCount, (int32)BiomeType);
}

void UQuest_CraftingSystem::SpawnResourceAtLocation(const FVector& Location, const FString& ResourceType)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Spawn resource pickup actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AQuest_ResourcePickup* ResourcePickup = World->SpawnActor<AQuest_ResourcePickup>(AQuest_ResourcePickup::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    
    if (ResourcePickup)
    {
        ResourcePickup->ResourceName = ResourceType;
        ResourcePickup->SetActorLabel(FString::Printf(TEXT("Resource_%s_%d"), *ResourceType, SpawnedResources.Num()));
        
        SpawnedResources.Add(ResourcePickup);
        
        UE_LOG(LogTemp, Warning, TEXT("Spawned %s resource at %s"), *ResourceType, *Location.ToString());
    }
}

FVector UQuest_CraftingSystem::GetRandomLocationInBiome(EBiomeType BiomeType)
{
    FVector BaseLocation;
    float SpreadRadius = 5000.0f;
    
    // Use biome coordinates from memory ID 709
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            BaseLocation = FVector(0.0f, 0.0f, 100.0f);
            break;
        case EBiomeType::Swamp:
            BaseLocation = FVector(-50000.0f, -45000.0f, 50.0f);
            break;
        case EBiomeType::Forest:
            BaseLocation = FVector(-45000.0f, 40000.0f, 100.0f);
            break;
        case EBiomeType::Desert:
            BaseLocation = FVector(55000.0f, 0.0f, 100.0f);
            break;
        case EBiomeType::Mountain:
            BaseLocation = FVector(40000.0f, 50000.0f, 500.0f);
            break;
        default:
            BaseLocation = FVector(0.0f, 0.0f, 100.0f);
            break;
    }
    
    // Add random offset within biome
    FVector RandomOffset = FVector(
        FMath::RandRange(-SpreadRadius, SpreadRadius),
        FMath::RandRange(-SpreadRadius, SpreadRadius),
        FMath::RandRange(0.0f, 200.0f)
    );
    
    return BaseLocation + RandomOffset;
}