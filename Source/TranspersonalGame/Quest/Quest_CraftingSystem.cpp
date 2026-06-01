#include "Quest_CraftingSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

UQuest_CraftingSystem::UQuest_CraftingSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    bCraftingUIOpen = false;
    CraftingRange = 500.0f;
}

void UQuest_CraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultRecipes();
    
    // Give player some starting resources for testing
    PlayerInventory.Resources[EQuest_ResourceType::Rock] = 3;
    PlayerInventory.Resources[EQuest_ResourceType::Stick] = 2;
    PlayerInventory.Resources[EQuest_ResourceType::Leaf] = 1;
}

void UQuest_CraftingSystem::InitializeDefaultRecipes()
{
    AvailableRecipes.Empty();
    
    CreateStoneAxeRecipe();
    CreateCampfireRecipe();
    CreateWaterContainerRecipe();
    CreateSpearRecipe();
    CreateShelterRecipe();
    
    UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Initialized %d recipes"), AvailableRecipes.Num());
}

void UQuest_CraftingSystem::CreateStoneAxeRecipe()
{
    FQuest_CraftingRecipe StoneAxeRecipe;
    StoneAxeRecipe.RecipeName = TEXT("Stone Axe");
    StoneAxeRecipe.RequiredResources.Add(EQuest_ResourceType::Rock, 2);
    StoneAxeRecipe.RequiredResources.Add(EQuest_ResourceType::Stick, 1);
    StoneAxeRecipe.ResultItemName = TEXT("Stone Axe");
    StoneAxeRecipe.CraftingTime = 8;
    AvailableRecipes.Add(StoneAxeRecipe);
}

void UQuest_CraftingSystem::CreateCampfireRecipe()
{
    FQuest_CraftingRecipe CampfireRecipe;
    CampfireRecipe.RecipeName = TEXT("Campfire");
    CampfireRecipe.RequiredResources.Add(EQuest_ResourceType::Stick, 3);
    CampfireRecipe.RequiredResources.Add(EQuest_ResourceType::Rock, 5);
    CampfireRecipe.ResultItemName = TEXT("Campfire");
    CampfireRecipe.CraftingTime = 12;
    AvailableRecipes.Add(CampfireRecipe);
}

void UQuest_CraftingSystem::CreateWaterContainerRecipe()
{
    FQuest_CraftingRecipe WaterContainerRecipe;
    WaterContainerRecipe.RecipeName = TEXT("Water Container");
    WaterContainerRecipe.RequiredResources.Add(EQuest_ResourceType::Rock, 1);
    WaterContainerRecipe.RequiredResources.Add(EQuest_ResourceType::Leaf, 3);
    WaterContainerRecipe.ResultItemName = TEXT("Water Container");
    WaterContainerRecipe.CraftingTime = 6;
    AvailableRecipes.Add(WaterContainerRecipe);
}

void UQuest_CraftingSystem::CreateSpearRecipe()
{
    FQuest_CraftingRecipe SpearRecipe;
    SpearRecipe.RecipeName = TEXT("Wooden Spear");
    SpearRecipe.RequiredResources.Add(EQuest_ResourceType::Stick, 1);
    SpearRecipe.RequiredResources.Add(EQuest_ResourceType::Stone, 1);
    SpearRecipe.RequiredResources.Add(EQuest_ResourceType::Fiber, 2);
    SpearRecipe.ResultItemName = TEXT("Wooden Spear");
    SpearRecipe.CraftingTime = 10;
    AvailableRecipes.Add(SpearRecipe);
}

void UQuest_CraftingSystem::CreateShelterRecipe()
{
    FQuest_CraftingRecipe ShelterRecipe;
    ShelterRecipe.RecipeName = TEXT("Basic Shelter");
    ShelterRecipe.RequiredResources.Add(EQuest_ResourceType::Stick, 8);
    ShelterRecipe.RequiredResources.Add(EQuest_ResourceType::Leaf, 12);
    ShelterRecipe.RequiredResources.Add(EQuest_ResourceType::Fiber, 4);
    ShelterRecipe.ResultItemName = TEXT("Basic Shelter");
    ShelterRecipe.CraftingTime = 25;
    AvailableRecipes.Add(ShelterRecipe);
}

bool UQuest_CraftingSystem::CanCraftItem(const FString& RecipeName)
{
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.RecipeName == RecipeName)
        {
            for (const auto& RequiredResource : Recipe.RequiredResources)
            {
                EQuest_ResourceType ResourceType = RequiredResource.Key;
                int32 RequiredAmount = RequiredResource.Value;
                
                if (GetResourceCount(ResourceType) < RequiredAmount)
                {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

bool UQuest_CraftingSystem::CraftItem(const FString& RecipeName)
{
    if (!CanCraftItem(RecipeName))
    {
        UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Cannot craft %s - insufficient resources"), *RecipeName);
        return false;
    }
    
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.RecipeName == RecipeName)
        {
            // Consume resources
            for (const auto& RequiredResource : Recipe.RequiredResources)
            {
                EQuest_ResourceType ResourceType = RequiredResource.Key;
                int32 RequiredAmount = RequiredResource.Value;
                PlayerInventory.Resources[ResourceType] -= RequiredAmount;
            }
            
            // Add crafted item to inventory
            PlayerInventory.CraftedItems.Add(Recipe.ResultItemName);
            
            UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Successfully crafted %s"), *Recipe.ResultItemName);
            return true;
        }
    }
    return false;
}

void UQuest_CraftingSystem::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    if (PlayerInventory.Resources.Contains(ResourceType))
    {
        PlayerInventory.Resources[ResourceType] += Amount;
    }
    else
    {
        PlayerInventory.Resources.Add(ResourceType, Amount);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Added %d %s"), Amount, *UEnum::GetValueAsString(ResourceType));
}

int32 UQuest_CraftingSystem::GetResourceCount(EQuest_ResourceType ResourceType)
{
    if (PlayerInventory.Resources.Contains(ResourceType))
    {
        return PlayerInventory.Resources[ResourceType];
    }
    return 0;
}

void UQuest_CraftingSystem::ToggleCraftingUI()
{
    bCraftingUIOpen = !bCraftingUIOpen;
    UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Crafting UI %s"), bCraftingUIOpen ? TEXT("OPENED") : TEXT("CLOSED"));
}

TArray<FQuest_CraftingRecipe> UQuest_CraftingSystem::GetAvailableRecipes()
{
    return AvailableRecipes;
}

FQuest_PlayerInventory UQuest_CraftingSystem::GetPlayerInventory()
{
    return PlayerInventory;
}

void UQuest_CraftingSystem::SpawnResourcePickups()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Biome coordinates from memory
    TArray<FVector> BiomeLocations = {
        FVector(0, 0, 100),           // Savana
        FVector(-50000, -45000, 100), // Pantano
        FVector(-45000, 40000, 100),  // Floresta
        FVector(55000, 0, 100),       // Deserto
        FVector(40000, 50000, 100)    // Montanha
    };
    
    // Spawn resources across biomes
    for (const FVector& BiomeCenter : BiomeLocations)
    {
        // Spawn rocks (5 per biome)
        for (int32 i = 0; i < 5; i++)
        {
            FVector SpawnLocation = BiomeCenter + FVector(
                FMath::RandRange(-10000, 10000),
                FMath::RandRange(-10000, 10000),
                0
            );
            
            AStaticMeshActor* RockActor = World->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator);
            if (RockActor)
            {
                RockActor->SetActorLabel(TEXT("ResourceRock"));
                RockActor->Tags.Add(TEXT("Resource"));
                RockActor->Tags.Add(TEXT("Rock"));
            }
        }
        
        // Spawn sticks (3 per biome)
        for (int32 i = 0; i < 3; i++)
        {
            FVector SpawnLocation = BiomeCenter + FVector(
                FMath::RandRange(-10000, 10000),
                FMath::RandRange(-10000, 10000),
                0
            );
            
            AStaticMeshActor* StickActor = World->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator);
            if (StickActor)
            {
                StickActor->SetActorLabel(TEXT("ResourceStick"));
                StickActor->Tags.Add(TEXT("Resource"));
                StickActor->Tags.Add(TEXT("Stick"));
            }
        }
        
        // Spawn leaves (2 per biome)
        for (int32 i = 0; i < 2; i++)
        {
            FVector SpawnLocation = BiomeCenter + FVector(
                FMath::RandRange(-10000, 10000),
                FMath::RandRange(-10000, 10000),
                0
            );
            
            AStaticMeshActor* LeafActor = World->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator);
            if (LeafActor)
            {
                LeafActor->SetActorLabel(TEXT("ResourceLeaf"));
                LeafActor->Tags.Add(TEXT("Resource"));
                LeafActor->Tags.Add(TEXT("Leaf"));
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CraftingSystem: Spawned resource pickups across all biomes"));
}