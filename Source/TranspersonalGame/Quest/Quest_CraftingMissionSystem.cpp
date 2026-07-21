#include "Quest_CraftingMissionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h"

UQuest_CraftingMissionSystem::UQuest_CraftingMissionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    bCraftingMenuOpen = false;
    CraftingRange = 500.0f;
    PlayerCraftingLevel = 1;
}

void UQuest_CraftingMissionSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeCraftingSystem();
}

void UQuest_CraftingMissionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    ProcessCraftingInput();
    CheckMissionProgress();
    UpdateCraftingUI();
}

void UQuest_CraftingMissionSystem::InitializeCraftingSystem()
{
    AddBasicRecipes();
    
    // Initialize player inventory with some starting items
    AddItemToInventory(TEXT("Rock"), 3);
    AddItemToInventory(TEXT("Stick"), 5);
    AddItemToInventory(TEXT("Leaf"), 2);
    
    // Add initial crafting missions
    FQuest_CraftingMission FirstToolMission;
    FirstToolMission.MissionName = TEXT("First Tool");
    FirstToolMission.Description = TEXT("Craft your first stone axe to survive in the wilderness");
    FirstToolMission.RequiredCraftedItems.Add(TEXT("Stone Axe"));
    FirstToolMission.RequiredQuantities.Add(1);
    FirstToolMission.RewardExperience = 100;
    FirstToolMission.RewardItems.Add(TEXT("Stick"));
    AddCraftingMission(FirstToolMission);
    
    FQuest_CraftingMission ShelterMission;
    ShelterMission.MissionName = TEXT("Basic Shelter");
    ShelterMission.Description = TEXT("Build a campfire to keep warm and cook food");
    ShelterMission.RequiredCraftedItems.Add(TEXT("Campfire"));
    ShelterMission.RequiredQuantities.Add(1);
    ShelterMission.RewardExperience = 150;
    ShelterMission.RewardItems.Add(TEXT("Cooked Meat"));
    AddCraftingMission(ShelterMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Crafting System Initialized with %d recipes and %d missions"), 
           AvailableRecipes.Num(), ActiveMissions.Num());
}

void UQuest_CraftingMissionSystem::AddBasicRecipes()
{
    CreateStoneAxeRecipe();
    CreateCampfireRecipe();
    CreateWaterContainerRecipe();
    CreateShelterRecipe();
    CreateSpearRecipe();
}

void UQuest_CraftingMissionSystem::CreateStoneAxeRecipe()
{
    FQuest_CraftingRecipe StoneAxeRecipe;
    StoneAxeRecipe.RecipeName = TEXT("Stone Axe");
    StoneAxeRecipe.RequiredItems.Add(TEXT("Rock"));
    StoneAxeRecipe.RequiredItems.Add(TEXT("Stick"));
    StoneAxeRecipe.RequiredQuantities.Add(2);
    StoneAxeRecipe.RequiredQuantities.Add(1);
    StoneAxeRecipe.ResultItem = TEXT("Stone Axe");
    StoneAxeRecipe.CraftingTime = 8.0f;
    StoneAxeRecipe.ExperienceReward = 25;
    AvailableRecipes.Add(StoneAxeRecipe);
}

void UQuest_CraftingMissionSystem::CreateCampfireRecipe()
{
    FQuest_CraftingRecipe CampfireRecipe;
    CampfireRecipe.RecipeName = TEXT("Campfire");
    CampfireRecipe.RequiredItems.Add(TEXT("Stick"));
    CampfireRecipe.RequiredItems.Add(TEXT("Rock"));
    CampfireRecipe.RequiredQuantities.Add(3);
    CampfireRecipe.RequiredQuantities.Add(1);
    CampfireRecipe.ResultItem = TEXT("Campfire");
    CampfireRecipe.CraftingTime = 12.0f;
    CampfireRecipe.ExperienceReward = 40;
    AvailableRecipes.Add(CampfireRecipe);
}

void UQuest_CraftingMissionSystem::CreateWaterContainerRecipe()
{
    FQuest_CraftingRecipe WaterContainerRecipe;
    WaterContainerRecipe.RecipeName = TEXT("Water Container");
    WaterContainerRecipe.RequiredItems.Add(TEXT("Rock"));
    WaterContainerRecipe.RequiredItems.Add(TEXT("Leaf"));
    WaterContainerRecipe.RequiredQuantities.Add(1);
    WaterContainerRecipe.RequiredQuantities.Add(1);
    WaterContainerRecipe.ResultItem = TEXT("Water Container");
    WaterContainerRecipe.CraftingTime = 6.0f;
    WaterContainerRecipe.ExperienceReward = 20;
    AvailableRecipes.Add(WaterContainerRecipe);
}

void UQuest_CraftingMissionSystem::CreateShelterRecipe()
{
    FQuest_CraftingRecipe ShelterRecipe;
    ShelterRecipe.RecipeName = TEXT("Basic Shelter");
    ShelterRecipe.RequiredItems.Add(TEXT("Stick"));
    ShelterRecipe.RequiredItems.Add(TEXT("Leaf"));
    ShelterRecipe.RequiredItems.Add(TEXT("Rock"));
    ShelterRecipe.RequiredQuantities.Add(8);
    ShelterRecipe.RequiredQuantities.Add(6);
    ShelterRecipe.RequiredQuantities.Add(4);
    ShelterRecipe.ResultItem = TEXT("Basic Shelter");
    ShelterRecipe.CraftingTime = 30.0f;
    ShelterRecipe.ExperienceReward = 100;
    AvailableRecipes.Add(ShelterRecipe);
}

void UQuest_CraftingMissionSystem::CreateSpearRecipe()
{
    FQuest_CraftingRecipe SpearRecipe;
    SpearRecipe.RecipeName = TEXT("Stone Spear");
    SpearRecipe.RequiredItems.Add(TEXT("Stick"));
    SpearRecipe.RequiredItems.Add(TEXT("Rock"));
    SpearRecipe.RequiredQuantities.Add(1);
    SpearRecipe.RequiredQuantities.Add(1);
    SpearRecipe.ResultItem = TEXT("Stone Spear");
    SpearRecipe.CraftingTime = 10.0f;
    SpearRecipe.ExperienceReward = 30;
    AvailableRecipes.Add(SpearRecipe);
}

bool UQuest_CraftingMissionSystem::CanCraftItem(const FString& RecipeName)
{
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.RecipeName == RecipeName)
        {
            for (int32 i = 0; i < Recipe.RequiredItems.Num(); i++)
            {
                int32 RequiredQuantity = Recipe.RequiredQuantities.IsValidIndex(i) ? Recipe.RequiredQuantities[i] : 1;
                if (GetItemQuantity(Recipe.RequiredItems[i]) < RequiredQuantity)
                {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

bool UQuest_CraftingMissionSystem::CraftItem(const FString& RecipeName)
{
    if (!CanCraftItem(RecipeName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot craft %s - insufficient materials"), *RecipeName);
        return false;
    }
    
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.RecipeName == RecipeName)
        {
            // Remove required materials
            for (int32 i = 0; i < Recipe.RequiredItems.Num(); i++)
            {
                int32 RequiredQuantity = Recipe.RequiredQuantities.IsValidIndex(i) ? Recipe.RequiredQuantities[i] : 1;
                int32 CurrentQuantity = GetItemQuantity(Recipe.RequiredItems[i]);
                AddItemToInventory(Recipe.RequiredItems[i], -(RequiredQuantity));
            }
            
            // Add crafted item
            AddItemToInventory(Recipe.ResultItem, 1);
            
            // Gain experience
            GainCraftingExperience(Recipe.ExperienceReward);
            
            UE_LOG(LogTemp, Warning, TEXT("Successfully crafted %s"), *RecipeName);
            return true;
        }
    }
    return false;
}

void UQuest_CraftingMissionSystem::AddItemToInventory(const FString& ItemName, int32 Quantity)
{
    if (PlayerInventory.Contains(ItemName))
    {
        PlayerInventory[ItemName] = FMath::Max(0, PlayerInventory[ItemName] + Quantity);
    }
    else if (Quantity > 0)
    {
        PlayerInventory.Add(ItemName, Quantity);
    }
}

int32 UQuest_CraftingMissionSystem::GetItemQuantity(const FString& ItemName)
{
    if (PlayerInventory.Contains(ItemName))
    {
        return PlayerInventory[ItemName];
    }
    return 0;
}

void UQuest_CraftingMissionSystem::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    UE_LOG(LogTemp, Warning, TEXT("Crafting menu %s"), bCraftingMenuOpen ? TEXT("opened") : TEXT("closed"));
}

void UQuest_CraftingMissionSystem::AddCraftingMission(const FQuest_CraftingMission& NewMission)
{
    ActiveMissions.Add(NewMission);
    UE_LOG(LogTemp, Warning, TEXT("Added crafting mission: %s"), *NewMission.MissionName);
}

void UQuest_CraftingMissionSystem::CheckMissionProgress()
{
    for (FQuest_CraftingMission& Mission : ActiveMissions)
    {
        if (!Mission.bIsCompleted)
        {
            bool bAllItemsCrafted = true;
            for (int32 i = 0; i < Mission.RequiredCraftedItems.Num(); i++)
            {
                int32 RequiredQuantity = Mission.RequiredQuantities.IsValidIndex(i) ? Mission.RequiredQuantities[i] : 1;
                if (GetItemQuantity(Mission.RequiredCraftedItems[i]) < RequiredQuantity)
                {
                    bAllItemsCrafted = false;
                    break;
                }
            }
            
            if (bAllItemsCrafted)
            {
                CompleteMission(Mission.MissionName);
            }
        }
    }
}

void UQuest_CraftingMissionSystem::CompleteMission(const FString& MissionName)
{
    for (FQuest_CraftingMission& Mission : ActiveMissions)
    {
        if (Mission.MissionName == MissionName && !Mission.bIsCompleted)
        {
            Mission.bIsCompleted = true;
            
            // Award experience
            GainCraftingExperience(Mission.RewardExperience);
            
            // Award items
            for (const FString& RewardItem : Mission.RewardItems)
            {
                AddItemToInventory(RewardItem, 1);
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Mission completed: %s"), *MissionName);
            break;
        }
    }
}

TArray<FQuest_CraftingRecipe> UQuest_CraftingMissionSystem::GetAvailableRecipes() const
{
    return AvailableRecipes;
}

TArray<FQuest_CraftingMission> UQuest_CraftingMissionSystem::GetActiveMissions() const
{
    return ActiveMissions;
}

void UQuest_CraftingMissionSystem::SpawnResourceNodes()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // This would spawn resource nodes in the world
    // Implementation would depend on specific resource actor classes
    UE_LOG(LogTemp, Warning, TEXT("Resource nodes spawning system activated"));
}

void UQuest_CraftingMissionSystem::ValidateCraftingRequirements()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating crafting system requirements..."));
    UE_LOG(LogTemp, Warning, TEXT("Available recipes: %d"), AvailableRecipes.Num());
    UE_LOG(LogTemp, Warning, TEXT("Active missions: %d"), ActiveMissions.Num());
    UE_LOG(LogTemp, Warning, TEXT("Player crafting level: %d"), PlayerCraftingLevel);
}

void UQuest_CraftingMissionSystem::ProcessCraftingInput()
{
    // Handle input processing for crafting menu
    // This would be connected to input bindings
}

void UQuest_CraftingMissionSystem::UpdateCraftingUI()
{
    // Update UI elements based on current state
    // This would communicate with UMG widgets
}

void UQuest_CraftingMissionSystem::GainCraftingExperience(int32 Experience)
{
    // Simple leveling system
    int32 ExperienceNeeded = PlayerCraftingLevel * 100;
    if (Experience >= ExperienceNeeded)
    {
        PlayerCraftingLevel++;
        UE_LOG(LogTemp, Warning, TEXT("Crafting level increased to %d"), PlayerCraftingLevel);
    }
}