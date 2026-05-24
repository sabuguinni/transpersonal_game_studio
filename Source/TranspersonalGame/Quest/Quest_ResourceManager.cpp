#include "Quest_ResourceManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

UQuest_ResourceManager::UQuest_ResourceManager()
{
    // Initialize default inventory
    PlayerInventory.Add(EQuest_ResourceType::Stone, 0);
    PlayerInventory.Add(EQuest_ResourceType::Wood, 0);
    PlayerInventory.Add(EQuest_ResourceType::Plant, 0);
    PlayerInventory.Add(EQuest_ResourceType::Water, 0);
    PlayerInventory.Add(EQuest_ResourceType::Meat, 0);
    PlayerInventory.Add(EQuest_ResourceType::Hide, 0);
    PlayerInventory.Add(EQuest_ResourceType::Bone, 0);
    PlayerInventory.Add(EQuest_ResourceType::Fire, 0);
}

void UQuest_ResourceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_ResourceManager: Initializing resource and crafting system"));
    
    InitializeCraftingRecipes();
    InitializeResourceSpawns();
    
    // Give player starting resources for testing
    AddResource(EQuest_ResourceType::Stone, 5);
    AddResource(EQuest_ResourceType::Wood, 3);
    AddResource(EQuest_ResourceType::Plant, 2);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_ResourceManager: Initialized with %d crafting recipes"), CraftingRecipes.Num());
}

void UQuest_ResourceManager::Deinitialize()
{
    PlayerInventory.Empty();
    CraftingRecipes.Empty();
    WorldResources.Empty();
    
    Super::Deinitialize();
}

void UQuest_ResourceManager::AddResource(EQuest_ResourceType Type, int32 Quantity, float Quality)
{
    if (Type == EQuest_ResourceType::None || Quantity <= 0)
    {
        return;
    }
    
    int32* CurrentQuantity = PlayerInventory.Find(Type);
    if (CurrentQuantity)
    {
        *CurrentQuantity += Quantity;
    }
    else
    {
        PlayerInventory.Add(Type, Quantity);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest_ResourceManager: Added %d %s (Total: %d)"), 
           Quantity, 
           *UEnum::GetValueAsString(Type),
           PlayerInventory[Type]);
}

bool UQuest_ResourceManager::ConsumeResource(EQuest_ResourceType Type, int32 Quantity)
{
    if (Type == EQuest_ResourceType::None || Quantity <= 0)
    {
        return false;
    }
    
    int32* CurrentQuantity = PlayerInventory.Find(Type);
    if (!CurrentQuantity || *CurrentQuantity < Quantity)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_ResourceManager: Insufficient %s (Need: %d, Have: %d)"), 
               *UEnum::GetValueAsString(Type), 
               Quantity, 
               CurrentQuantity ? *CurrentQuantity : 0);
        return false;
    }
    
    *CurrentQuantity -= Quantity;
    
    UE_LOG(LogTemp, Log, TEXT("Quest_ResourceManager: Consumed %d %s (Remaining: %d)"), 
           Quantity, 
           *UEnum::GetValueAsString(Type),
           *CurrentQuantity);
    
    return true;
}

int32 UQuest_ResourceManager::GetResourceQuantity(EQuest_ResourceType Type) const
{
    const int32* Quantity = PlayerInventory.Find(Type);
    return Quantity ? *Quantity : 0;
}

TArray<FQuest_ResourceData> UQuest_ResourceManager::GetAllResources() const
{
    TArray<FQuest_ResourceData> Resources;
    
    for (const auto& ResourcePair : PlayerInventory)
    {
        if (ResourcePair.Value > 0)
        {
            FQuest_ResourceData ResourceData = CreateResourceData(ResourcePair.Key, ResourcePair.Value);
            Resources.Add(ResourceData);
        }
    }
    
    return Resources;
}

bool UQuest_ResourceManager::CanCraftItem(const FString& RecipeName) const
{
    const FQuest_CraftingRecipe* Recipe = CraftingRecipes.FindByPredicate([&RecipeName](const FQuest_CraftingRecipe& R) {
        return R.RecipeName.Equals(RecipeName);
    });
    
    if (!Recipe)
    {
        return false;
    }
    
    // Check if player has all required resources
    for (const FQuest_ResourceData& RequiredResource : Recipe->RequiredResources)
    {
        int32 PlayerQuantity = GetResourceQuantity(RequiredResource.Type);
        if (PlayerQuantity < RequiredResource.Quantity)
        {
            return false;
        }
    }
    
    return true;
}

bool UQuest_ResourceManager::CraftItem(const FString& RecipeName)
{
    if (!CanCraftItem(RecipeName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_ResourceManager: Cannot craft %s - insufficient resources"), *RecipeName);
        return false;
    }
    
    const FQuest_CraftingRecipe* Recipe = CraftingRecipes.FindByPredicate([&RecipeName](const FQuest_CraftingRecipe& R) {
        return R.RecipeName.Equals(RecipeName);
    });
    
    if (!Recipe)
    {
        return false;
    }
    
    // Consume required resources
    for (const FQuest_ResourceData& RequiredResource : Recipe->RequiredResources)
    {
        ConsumeResource(RequiredResource.Type, RequiredResource.Quantity);
    }
    
    // Add crafted item
    AddResource(Recipe->Result.Type, Recipe->Result.Quantity, Recipe->Result.Quality);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_ResourceManager: Successfully crafted %s"), *RecipeName);
    
    return true;
}

TArray<FQuest_CraftingRecipe> UQuest_ResourceManager::GetAvailableRecipes() const
{
    return CraftingRecipes;
}

void UQuest_ResourceManager::SpawnResourceNode(EQuest_ResourceType Type, const FVector& Location, int32 Quantity)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create resource data
    FQuest_ResourceData ResourceData = CreateResourceData(Type, Quantity);
    ResourceData.Location = Location;
    ResourceData.bIsRenewable = true;
    
    WorldResources.Add(ResourceData);
    
    // Spawn visual representation (basic cube for now)
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* ResourceActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    if (ResourceActor)
    {
        // Set actor label based on resource type
        FString ResourceName = UEnum::GetValueAsString(Type);
        ResourceActor->SetActorLabel(FString::Printf(TEXT("Resource_%s_%d"), *ResourceName, Quantity));
        
        UE_LOG(LogTemp, Log, TEXT("Quest_ResourceManager: Spawned %s resource node at %s"), 
               *ResourceName, 
               *Location.ToString());
    }
}

void UQuest_ResourceManager::PopulateWorldWithResources()
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_ResourceManager: Populating world with resource nodes"));
    
    // Spawn stone resources
    for (int32 i = 0; i < 10; i++)
    {
        FVector StoneLocation = FVector(
            FMath::RandRange(-5000.0f, 5000.0f),
            FMath::RandRange(-5000.0f, 5000.0f),
            100.0f
        );
        SpawnResourceNode(EQuest_ResourceType::Stone, StoneLocation, FMath::RandRange(1, 3));
    }
    
    // Spawn wood resources
    for (int32 i = 0; i < 8; i++)
    {
        FVector WoodLocation = FVector(
            FMath::RandRange(-3000.0f, 3000.0f),
            FMath::RandRange(-3000.0f, 3000.0f),
            100.0f
        );
        SpawnResourceNode(EQuest_ResourceType::Wood, WoodLocation, FMath::RandRange(2, 5));
    }
    
    // Spawn plant resources
    for (int32 i = 0; i < 15; i++)
    {
        FVector PlantLocation = FVector(
            FMath::RandRange(-2000.0f, 2000.0f),
            FMath::RandRange(-2000.0f, 2000.0f),
            100.0f
        );
        SpawnResourceNode(EQuest_ResourceType::Plant, PlantLocation, 1);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_ResourceManager: Spawned %d resource nodes"), WorldResources.Num());
}

bool UQuest_ResourceManager::HasRequiredResourcesForMission(const FString& MissionType) const
{
    // Define resource requirements for different mission types
    if (MissionType == TEXT("HunterGatherer"))
    {
        return GetResourceQuantity(EQuest_ResourceType::Stone) >= 2 && 
               GetResourceQuantity(EQuest_ResourceType::Wood) >= 1;
    }
    else if (MissionType == TEXT("ShelterBuilder"))
    {
        return GetResourceQuantity(EQuest_ResourceType::Wood) >= 5 && 
               GetResourceQuantity(EQuest_ResourceType::Stone) >= 3;
    }
    else if (MissionType == TEXT("ToolCrafter"))
    {
        return GetResourceQuantity(EQuest_ResourceType::Stone) >= 3 && 
               GetResourceQuantity(EQuest_ResourceType::Wood) >= 2;
    }
    else if (MissionType == TEXT("FireKeeper"))
    {
        return GetResourceQuantity(EQuest_ResourceType::Wood) >= 3 && 
               GetResourceQuantity(EQuest_ResourceType::Plant) >= 2;
    }
    
    return true; // Default: no requirements
}

void UQuest_ResourceManager::ConsumeResourcesForMission(const FString& MissionType)
{
    if (MissionType == TEXT("HunterGatherer"))
    {
        ConsumeResource(EQuest_ResourceType::Stone, 2);
        ConsumeResource(EQuest_ResourceType::Wood, 1);
    }
    else if (MissionType == TEXT("ShelterBuilder"))
    {
        ConsumeResource(EQuest_ResourceType::Wood, 5);
        ConsumeResource(EQuest_ResourceType::Stone, 3);
    }
    else if (MissionType == TEXT("ToolCrafter"))
    {
        ConsumeResource(EQuest_ResourceType::Stone, 3);
        ConsumeResource(EQuest_ResourceType::Wood, 2);
    }
    else if (MissionType == TEXT("FireKeeper"))
    {
        ConsumeResource(EQuest_ResourceType::Wood, 3);
        ConsumeResource(EQuest_ResourceType::Plant, 2);
    }
}

void UQuest_ResourceManager::InitializeCraftingRecipes()
{
    CraftingRecipes.Empty();
    
    // Stone Axe Recipe
    {
        TArray<FQuest_ResourceData> Requirements;
        Requirements.Add(CreateResourceData(EQuest_ResourceType::Stone, 2));
        Requirements.Add(CreateResourceData(EQuest_ResourceType::Wood, 1));
        
        FQuest_ResourceData Result = CreateResourceData(EQuest_ResourceType::Bone, 1, 1.5f); // Represents tool
        
        FQuest_CraftingRecipe StoneAxeRecipe = CreateRecipe(TEXT("StoneAxe"), Requirements, Result);
        StoneAxeRecipe.CraftingTime = 10.0f;
        StoneAxeRecipe.ExperienceGained = 25;
        
        CraftingRecipes.Add(StoneAxeRecipe);
    }
    
    // Campfire Recipe
    {
        TArray<FQuest_ResourceData> Requirements;
        Requirements.Add(CreateResourceData(EQuest_ResourceType::Wood, 3));
        Requirements.Add(CreateResourceData(EQuest_ResourceType::Stone, 1));
        
        FQuest_ResourceData Result = CreateResourceData(EQuest_ResourceType::Fire, 1);
        
        FQuest_CraftingRecipe CampfireRecipe = CreateRecipe(TEXT("Campfire"), Requirements, Result);
        CampfireRecipe.CraftingTime = 15.0f;
        CampfireRecipe.ExperienceGained = 30;
        
        CraftingRecipes.Add(CampfireRecipe);
    }
    
    // Water Container Recipe
    {
        TArray<FQuest_ResourceData> Requirements;
        Requirements.Add(CreateResourceData(EQuest_ResourceType::Stone, 1));
        Requirements.Add(CreateResourceData(EQuest_ResourceType::Plant, 2));
        
        FQuest_ResourceData Result = CreateResourceData(EQuest_ResourceType::Water, 3);
        
        FQuest_CraftingRecipe WaterContainerRecipe = CreateRecipe(TEXT("WaterContainer"), Requirements, Result);
        WaterContainerRecipe.CraftingTime = 8.0f;
        WaterContainerRecipe.ExperienceGained = 20;
        
        CraftingRecipes.Add(WaterContainerRecipe);
    }
    
    // Spear Recipe
    {
        TArray<FQuest_ResourceData> Requirements;
        Requirements.Add(CreateResourceData(EQuest_ResourceType::Wood, 2));
        Requirements.Add(CreateResourceData(EQuest_ResourceType::Stone, 1));
        
        FQuest_ResourceData Result = CreateResourceData(EQuest_ResourceType::Bone, 1, 2.0f); // High quality tool
        
        FQuest_CraftingRecipe SpearRecipe = CreateRecipe(TEXT("Spear"), Requirements, Result);
        SpearRecipe.CraftingTime = 12.0f;
        SpearRecipe.ExperienceGained = 35;
        
        CraftingRecipes.Add(SpearRecipe);
    }
}

void UQuest_ResourceManager::InitializeResourceSpawns()
{
    WorldResources.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Quest_ResourceManager: Initialized resource spawn system"));
}

FQuest_ResourceData UQuest_ResourceManager::CreateResourceData(EQuest_ResourceType Type, int32 Quantity, float Quality) const
{
    FQuest_ResourceData ResourceData;
    ResourceData.Type = Type;
    ResourceData.Quantity = Quantity;
    ResourceData.Quality = Quality;
    ResourceData.Location = FVector::ZeroVector;
    ResourceData.bIsRenewable = false;
    ResourceData.RespawnTime = 300.0f;
    
    return ResourceData;
}

FQuest_CraftingRecipe UQuest_ResourceManager::CreateRecipe(const FString& Name, const TArray<FQuest_ResourceData>& Requirements, const FQuest_ResourceData& Result) const
{
    FQuest_CraftingRecipe Recipe;
    Recipe.RecipeName = Name;
    Recipe.RequiredResources = Requirements;
    Recipe.Result = Result;
    Recipe.CraftingTime = 5.0f;
    Recipe.ExperienceGained = 10;
    
    return Recipe;
}