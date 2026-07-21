#include "Quest_CraftingMissionController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"

UQuest_CraftingMissionController::UQuest_CraftingMissionController()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize crafting recipes
    InitializeCraftingRecipes();
    
    // Initialize resource tracking
    CollectedStones = 0;
    CollectedSticks = 0;
    CollectedLeaves = 0;
    CollectedFlint = 0;
    CollectedLogs = 0;
    
    RequiredStones = 5;
    RequiredSticks = 3;
    RequiredLeaves = 2;
    RequiredFlint = 1;
    RequiredLogs = 2;
    
    bStoneAxeUnlocked = false;
    bCampfireUnlocked = false;
    bWaterContainerUnlocked = false;
    bShelterUnlocked = false;
    bHuntingSpearUnlocked = false;
    
    CurrentMissionStage = EQuest_CraftingStage::GatherBasicMaterials;
}

void UQuest_CraftingMissionController::BeginPlay()
{
    Super::BeginPlay();
    
    // Start the first crafting mission
    StartGatheringMission();
}

void UQuest_CraftingMissionController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Check mission progress
    CheckMissionProgress();
    
    // Update UI feedback
    UpdateCraftingUI();
}

void UQuest_CraftingMissionController::InitializeCraftingRecipes()
{
    // Stone Axe Recipe
    FQuest_CraftingRecipe StoneAxeRecipe;
    StoneAxeRecipe.RecipeName = "Stone Axe";
    StoneAxeRecipe.Description = "A basic tool for cutting wood and hunting small prey";
    StoneAxeRecipe.RequiredStones = 2;
    StoneAxeRecipe.RequiredSticks = 1;
    StoneAxeRecipe.RequiredLeaves = 0;
    StoneAxeRecipe.RequiredFlint = 1;
    StoneAxeRecipe.RequiredLogs = 0;
    StoneAxeRecipe.CraftingTime = 5.0f;
    StoneAxeRecipe.bIsUnlocked = false;
    CraftingRecipes.Add("StoneAxe", StoneAxeRecipe);
    
    // Campfire Recipe
    FQuest_CraftingRecipe CampfireRecipe;
    CampfireRecipe.RecipeName = "Campfire";
    CampfireRecipe.Description = "Essential for cooking food and staying warm at night";
    CampfireRecipe.RequiredStones = 3;
    CampfireRecipe.RequiredSticks = 5;
    CampfireRecipe.RequiredLeaves = 3;
    CampfireRecipe.RequiredFlint = 1;
    CampfireRecipe.RequiredLogs = 2;
    CampfireRecipe.CraftingTime = 10.0f;
    CampfireRecipe.bIsUnlocked = false;
    CraftingRecipes.Add("Campfire", CampfireRecipe);
    
    // Water Container Recipe
    FQuest_CraftingRecipe WaterContainerRecipe;
    WaterContainerRecipe.RecipeName = "Water Container";
    WaterContainerRecipe.Description = "Store and purify water for survival";
    WaterContainerRecipe.RequiredStones = 1;
    WaterContainerRecipe.RequiredSticks = 2;
    WaterContainerRecipe.RequiredLeaves = 5;
    WaterContainerRecipe.RequiredFlint = 0;
    WaterContainerRecipe.RequiredLogs = 0;
    WaterContainerRecipe.CraftingTime = 7.0f;
    WaterContainerRecipe.bIsUnlocked = false;
    CraftingRecipes.Add("WaterContainer", WaterContainerRecipe);
    
    // Basic Shelter Recipe
    FQuest_CraftingRecipe ShelterRecipe;
    ShelterRecipe.RecipeName = "Basic Shelter";
    ShelterRecipe.Description = "Protection from weather and predators";
    ShelterRecipe.RequiredStones = 4;
    ShelterRecipe.RequiredSticks = 8;
    ShelterRecipe.RequiredLeaves = 10;
    ShelterRecipe.RequiredFlint = 0;
    ShelterRecipe.RequiredLogs = 4;
    ShelterRecipe.CraftingTime = 20.0f;
    ShelterRecipe.bIsUnlocked = false;
    CraftingRecipes.Add("BasicShelter", ShelterRecipe);
    
    // Hunting Spear Recipe
    FQuest_CraftingRecipe SpearRecipe;
    SpearRecipe.RecipeName = "Hunting Spear";
    SpearRecipe.Description = "Advanced weapon for hunting larger prey";
    SpearRecipe.RequiredStones = 1;
    SpearRecipe.RequiredSticks = 1;
    SpearRecipe.RequiredLeaves = 1;
    SpearRecipe.RequiredFlint = 2;
    SpearRecipe.RequiredLogs = 1;
    SpearRecipe.CraftingTime = 15.0f;
    SpearRecipe.bIsUnlocked = false;
    CraftingRecipes.Add("HuntingSpear", SpearRecipe);
}

void UQuest_CraftingMissionController::StartGatheringMission()
{
    CurrentMissionStage = EQuest_CraftingStage::GatherBasicMaterials;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
            TEXT("MISSION: Gather basic crafting materials"));
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow, 
            TEXT("Collect: 5 Stones, 3 Sticks, 2 Leaves"));
    }
    
    // Broadcast mission start event
    OnMissionStarted.Broadcast("Gather Basic Materials", "Collect stones, sticks, and leaves for crafting");
}

void UQuest_CraftingMissionController::CollectResource(const FString& ResourceType, int32 Amount)
{
    if (ResourceType == "Stone")
    {
        CollectedStones += Amount;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, 
                FString::Printf(TEXT("Collected Stone! (%d/%d)"), CollectedStones, RequiredStones));
        }
    }
    else if (ResourceType == "Stick")
    {
        CollectedSticks += Amount;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Brown, 
                FString::Printf(TEXT("Collected Stick! (%d/%d)"), CollectedSticks, RequiredSticks));
        }
    }
    else if (ResourceType == "Leaf")
    {
        CollectedLeaves += Amount;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("Collected Leaf! (%d/%d)"), CollectedLeaves, RequiredLeaves));
        }
    }
    else if (ResourceType == "Flint")
    {
        CollectedFlint += Amount;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                FString::Printf(TEXT("Collected Flint! (%d/%d)"), CollectedFlint, RequiredFlint));
        }
    }
    else if (ResourceType == "Log")
    {
        CollectedLogs += Amount;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, 
                FString::Printf(TEXT("Collected Log! (%d/%d)"), CollectedLogs, RequiredLogs));
        }
    }
    
    // Broadcast resource collection event
    OnResourceCollected.Broadcast(ResourceType, Amount, GetResourceCount(ResourceType));
}

int32 UQuest_CraftingMissionController::GetResourceCount(const FString& ResourceType) const
{
    if (ResourceType == "Stone") return CollectedStones;
    if (ResourceType == "Stick") return CollectedSticks;
    if (ResourceType == "Leaf") return CollectedLeaves;
    if (ResourceType == "Flint") return CollectedFlint;
    if (ResourceType == "Log") return CollectedLogs;
    
    return 0;
}

bool UQuest_CraftingMissionController::CanCraftItem(const FString& RecipeName) const
{
    if (!CraftingRecipes.Contains(RecipeName))
    {
        return false;
    }
    
    const FQuest_CraftingRecipe& Recipe = CraftingRecipes[RecipeName];
    
    if (!Recipe.bIsUnlocked)
    {
        return false;
    }
    
    return (CollectedStones >= Recipe.RequiredStones &&
            CollectedSticks >= Recipe.RequiredSticks &&
            CollectedLeaves >= Recipe.RequiredLeaves &&
            CollectedFlint >= Recipe.RequiredFlint &&
            CollectedLogs >= Recipe.RequiredLogs);
}

bool UQuest_CraftingMissionController::CraftItem(const FString& RecipeName)
{
    if (!CanCraftItem(RecipeName))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                TEXT("Cannot craft item - insufficient resources or recipe locked"));
        }
        return false;
    }
    
    const FQuest_CraftingRecipe& Recipe = CraftingRecipes[RecipeName];
    
    // Consume resources
    CollectedStones -= Recipe.RequiredStones;
    CollectedSticks -= Recipe.RequiredSticks;
    CollectedLeaves -= Recipe.RequiredLeaves;
    CollectedFlint -= Recipe.RequiredFlint;
    CollectedLogs -= Recipe.RequiredLogs;
    
    // Mark as crafted
    if (RecipeName == "StoneAxe")
    {
        bStoneAxeUnlocked = true;
        CurrentMissionStage = EQuest_CraftingStage::CraftStoneAxe;
    }
    else if (RecipeName == "Campfire")
    {
        bCampfireUnlocked = true;
        CurrentMissionStage = EQuest_CraftingStage::BuildCampfire;
    }
    else if (RecipeName == "WaterContainer")
    {
        bWaterContainerUnlocked = true;
        CurrentMissionStage = EQuest_CraftingStage::CraftWaterContainer;
    }
    else if (RecipeName == "BasicShelter")
    {
        bShelterUnlocked = true;
        CurrentMissionStage = EQuest_CraftingStage::BuildShelter;
    }
    else if (RecipeName == "HuntingSpear")
    {
        bHuntingSpearUnlocked = true;
        CurrentMissionStage = EQuest_CraftingStage::CraftHuntingSpear;
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green, 
            FString::Printf(TEXT("Successfully crafted: %s"), *Recipe.RecipeName));
    }
    
    // Broadcast crafting success event
    OnItemCrafted.Broadcast(RecipeName, Recipe.RecipeName);
    
    return true;
}

void UQuest_CraftingMissionController::CheckMissionProgress()
{
    switch (CurrentMissionStage)
    {
        case EQuest_CraftingStage::GatherBasicMaterials:
            if (CollectedStones >= RequiredStones && 
                CollectedSticks >= RequiredSticks && 
                CollectedLeaves >= RequiredLeaves)
            {
                UnlockBasicRecipes();
                CurrentMissionStage = EQuest_CraftingStage::CraftStoneAxe;
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                        TEXT("MISSION COMPLETE: Basic materials gathered!"));
                    GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow, 
                        TEXT("NEW MISSION: Craft a Stone Axe"));
                }
                
                OnMissionCompleted.Broadcast("Gather Basic Materials");
                OnMissionStarted.Broadcast("Craft Stone Axe", "Use your materials to craft a stone axe");
            }
            break;
            
        case EQuest_CraftingStage::CraftStoneAxe:
            if (bStoneAxeUnlocked)
            {
                CurrentMissionStage = EQuest_CraftingStage::GatherAdvancedMaterials;
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                        TEXT("MISSION COMPLETE: Stone Axe crafted!"));
                    GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow, 
                        TEXT("NEW MISSION: Gather materials for advanced crafting"));
                }
                
                OnMissionCompleted.Broadcast("Craft Stone Axe");
                OnMissionStarted.Broadcast("Gather Advanced Materials", "Collect flint and logs for advanced recipes");
            }
            break;
            
        case EQuest_CraftingStage::GatherAdvancedMaterials:
            if (CollectedFlint >= RequiredFlint && CollectedLogs >= RequiredLogs)
            {
                UnlockAdvancedRecipes();
                CurrentMissionStage = EQuest_CraftingStage::BuildCampfire;
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                        TEXT("MISSION COMPLETE: Advanced materials gathered!"));
                    GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow, 
                        TEXT("NEW MISSION: Build a Campfire"));
                }
                
                OnMissionCompleted.Broadcast("Gather Advanced Materials");
                OnMissionStarted.Broadcast("Build Campfire", "Construct a campfire for cooking and warmth");
            }
            break;
            
        case EQuest_CraftingStage::BuildCampfire:
            if (bCampfireUnlocked)
            {
                CurrentMissionStage = EQuest_CraftingStage::CraftWaterContainer;
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                        TEXT("MISSION COMPLETE: Campfire built!"));
                    GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow, 
                        TEXT("NEW MISSION: Craft a Water Container"));
                }
                
                OnMissionCompleted.Broadcast("Build Campfire");
                OnMissionStarted.Broadcast("Craft Water Container", "Create a container for storing clean water");
            }
            break;
            
        case EQuest_CraftingStage::CraftWaterContainer:
            if (bWaterContainerUnlocked)
            {
                CurrentMissionStage = EQuest_CraftingStage::BuildShelter;
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                        TEXT("MISSION COMPLETE: Water Container crafted!"));
                    GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow, 
                        TEXT("NEW MISSION: Build a Basic Shelter"));
                }
                
                OnMissionCompleted.Broadcast("Craft Water Container");
                OnMissionStarted.Broadcast("Build Basic Shelter", "Construct shelter for protection");
            }
            break;
            
        case EQuest_CraftingStage::BuildShelter:
            if (bShelterUnlocked)
            {
                CurrentMissionStage = EQuest_CraftingStage::CraftHuntingSpear;
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                        TEXT("MISSION COMPLETE: Basic Shelter built!"));
                    GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow, 
                        TEXT("NEW MISSION: Craft a Hunting Spear"));
                }
                
                OnMissionCompleted.Broadcast("Build Basic Shelter");
                OnMissionStarted.Broadcast("Craft Hunting Spear", "Create an advanced weapon for hunting");
            }
            break;
            
        case EQuest_CraftingStage::CraftHuntingSpear:
            if (bHuntingSpearUnlocked)
            {
                CurrentMissionStage = EQuest_CraftingStage::MasterCrafter;
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                        TEXT("MISSION COMPLETE: Hunting Spear crafted!"));
                    GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Gold, 
                        TEXT("CONGRATULATIONS: You are now a Master Crafter!"));
                }
                
                OnMissionCompleted.Broadcast("Craft Hunting Spear");
                OnMissionCompleted.Broadcast("Master Crafter Achievement");
            }
            break;
            
        case EQuest_CraftingStage::MasterCrafter:
            // All crafting missions completed
            break;
    }
}

void UQuest_CraftingMissionController::UnlockBasicRecipes()
{
    if (CraftingRecipes.Contains("StoneAxe"))
    {
        CraftingRecipes["StoneAxe"].bIsUnlocked = true;
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            TEXT("RECIPE UNLOCKED: Stone Axe"));
    }
}

void UQuest_CraftingMissionController::UnlockAdvancedRecipes()
{
    if (CraftingRecipes.Contains("Campfire"))
    {
        CraftingRecipes["Campfire"].bIsUnlocked = true;
    }
    
    if (CraftingRecipes.Contains("WaterContainer"))
    {
        CraftingRecipes["WaterContainer"].bIsUnlocked = true;
    }
    
    if (CraftingRecipes.Contains("BasicShelter"))
    {
        CraftingRecipes["BasicShelter"].bIsUnlocked = true;
    }
    
    if (CraftingRecipes.Contains("HuntingSpear"))
    {
        CraftingRecipes["HuntingSpear"].bIsUnlocked = true;
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            TEXT("RECIPES UNLOCKED: Campfire, Water Container, Shelter, Hunting Spear"));
    }
}

void UQuest_CraftingMissionController::UpdateCraftingUI()
{
    // This would typically update UI elements
    // For now, we'll just provide debug information
    static float UIUpdateTimer = 0.0f;
    UIUpdateTimer += GetWorld()->GetDeltaSeconds();
    
    if (UIUpdateTimer >= 5.0f)  // Update every 5 seconds
    {
        UIUpdateTimer = 0.0f;
        
        if (GEngine && CurrentMissionStage != EQuest_CraftingStage::MasterCrafter)
        {
            FString StatusMessage = FString::Printf(
                TEXT("Resources: Stones %d/%d, Sticks %d/%d, Leaves %d/%d, Flint %d/%d, Logs %d/%d"),
                CollectedStones, RequiredStones,
                CollectedSticks, RequiredSticks,
                CollectedLeaves, RequiredLeaves,
                CollectedFlint, RequiredFlint,
                CollectedLogs, RequiredLogs
            );
            
            GEngine->AddOnScreenDebugMessage(1000, 5.0f, FColor::White, StatusMessage);
        }
    }
}

TArray<FString> UQuest_CraftingMissionController::GetAvailableRecipes() const
{
    TArray<FString> AvailableRecipes;
    
    for (const auto& RecipePair : CraftingRecipes)
    {
        if (RecipePair.Value.bIsUnlocked)
        {
            AvailableRecipes.Add(RecipePair.Key);
        }
    }
    
    return AvailableRecipes;
}

FQuest_CraftingRecipe UQuest_CraftingMissionController::GetRecipeInfo(const FString& RecipeName) const
{
    if (CraftingRecipes.Contains(RecipeName))
    {
        return CraftingRecipes[RecipeName];
    }
    
    return FQuest_CraftingRecipe();
}