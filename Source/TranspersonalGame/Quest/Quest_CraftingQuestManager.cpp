#include "Quest_CraftingQuestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UQuest_CraftingQuestManager::UQuest_CraftingQuestManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    bCraftingMenuOpen = false;
    CraftingExperience = 0;
    CraftingLevel = 1;
}

void UQuest_CraftingQuestManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeCraftingSystem();
    AddBasicRecipes();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CraftingQuestManager: System initialized with %d recipes"), AvailableRecipes.Num());
}

void UQuest_CraftingQuestManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Check for crafting menu input (C key)
    if (GetWorld())
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && PC->WasInputKeyJustPressed(FKey("C")))
        {
            ToggleCraftingMenu();
        }
    }
}

void UQuest_CraftingQuestManager::InitializeCraftingSystem()
{
    // Initialize basic inventory items
    PlayerInventory.Empty();
    PlayerInventory.Add(TEXT("Rock"), 5);
    PlayerInventory.Add(TEXT("Stick"), 3);
    PlayerInventory.Add(TEXT("Leaf"), 8);
    PlayerInventory.Add(TEXT("Fiber"), 2);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CraftingQuestManager: Inventory initialized"));
}

void UQuest_CraftingQuestManager::AddBasicRecipes()
{
    AvailableRecipes.Empty();
    
    // Stone Axe Recipe
    FQuest_CraftingRecipe StoneAxe;
    StoneAxe.RecipeName = TEXT("Stone Axe");
    StoneAxe.RequiredMaterials.Add(TEXT("Rock"));
    StoneAxe.RequiredMaterials.Add(TEXT("Stick"));
    StoneAxe.MaterialQuantities.Add(2);
    StoneAxe.MaterialQuantities.Add(1);
    StoneAxe.ResultItem = TEXT("Stone Axe");
    StoneAxe.CraftingTime = 8.0f;
    StoneAxe.ExperienceReward = 25;
    AvailableRecipes.Add(StoneAxe);
    
    // Campfire Recipe
    FQuest_CraftingRecipe Campfire;
    Campfire.RecipeName = TEXT("Campfire");
    Campfire.RequiredMaterials.Add(TEXT("Stick"));
    Campfire.RequiredMaterials.Add(TEXT("Rock"));
    Campfire.MaterialQuantities.Add(3);
    Campfire.MaterialQuantities.Add(5);
    Campfire.ResultItem = TEXT("Campfire");
    Campfire.CraftingTime = 12.0f;
    Campfire.ExperienceReward = 35;
    AvailableRecipes.Add(Campfire);
    
    // Water Container Recipe
    FQuest_CraftingRecipe WaterContainer;
    WaterContainer.RecipeName = TEXT("Water Container");
    WaterContainer.RequiredMaterials.Add(TEXT("Rock"));
    WaterContainer.RequiredMaterials.Add(TEXT("Leaf"));
    WaterContainer.RequiredMaterials.Add(TEXT("Fiber"));
    WaterContainer.MaterialQuantities.Add(1);
    WaterContainer.MaterialQuantities.Add(3);
    WaterContainer.MaterialQuantities.Add(2);
    WaterContainer.ResultItem = TEXT("Water Container");
    WaterContainer.CraftingTime = 6.0f;
    WaterContainer.ExperienceReward = 20;
    AvailableRecipes.Add(WaterContainer);
    
    // Spear Recipe
    FQuest_CraftingRecipe Spear;
    Spear.RecipeName = TEXT("Spear");
    Spear.RequiredMaterials.Add(TEXT("Stick"));
    Spear.RequiredMaterials.Add(TEXT("Rock"));
    Spear.RequiredMaterials.Add(TEXT("Fiber"));
    Spear.MaterialQuantities.Add(2);
    Spear.MaterialQuantities.Add(1);
    Spear.MaterialQuantities.Add(1);
    Spear.ResultItem = TEXT("Spear");
    Spear.CraftingTime = 10.0f;
    Spear.ExperienceReward = 30;
    AvailableRecipes.Add(Spear);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CraftingQuestManager: Added %d basic recipes"), AvailableRecipes.Num());
}

bool UQuest_CraftingQuestManager::CanCraftItem(const FString& ItemName)
{
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.RecipeName == ItemName)
        {
            // Check if we have all required materials
            for (int32 i = 0; i < Recipe.RequiredMaterials.Num(); i++)
            {
                FString Material = Recipe.RequiredMaterials[i];
                int32 RequiredQuantity = Recipe.MaterialQuantities[i];
                int32 CurrentQuantity = GetItemQuantity(Material);
                
                if (CurrentQuantity < RequiredQuantity)
                {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

bool UQuest_CraftingQuestManager::CraftItem(const FString& ItemName)
{
    if (!CanCraftItem(ItemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_CraftingQuestManager: Cannot craft %s - insufficient materials"), *ItemName);
        return false;
    }
    
    // Find the recipe
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.RecipeName == ItemName)
        {
            // Remove required materials
            for (int32 i = 0; i < Recipe.RequiredMaterials.Num(); i++)
            {
                FString Material = Recipe.RequiredMaterials[i];
                int32 RequiredQuantity = Recipe.MaterialQuantities[i];
                RemoveItemFromInventory(Material, RequiredQuantity);
            }
            
            // Add crafted item
            AddItemToInventory(Recipe.ResultItem, 1);
            
            // Add experience
            AddCraftingExperience(Recipe.ExperienceReward);
            
            // Update quest progress
            UpdateCraftingProgress(ItemName, 1);
            
            UE_LOG(LogTemp, Warning, TEXT("Quest_CraftingQuestManager: Successfully crafted %s"), *ItemName);
            return true;
        }
    }
    
    return false;
}

void UQuest_CraftingQuestManager::AddItemToInventory(const FString& ItemName, int32 Quantity)
{
    if (PlayerInventory.Contains(ItemName))
    {
        PlayerInventory[ItemName] += Quantity;
    }
    else
    {
        PlayerInventory.Add(ItemName, Quantity);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest_CraftingQuestManager: Added %d %s to inventory"), Quantity, *ItemName);
}

int32 UQuest_CraftingQuestManager::GetItemQuantity(const FString& ItemName)
{
    if (PlayerInventory.Contains(ItemName))
    {
        return PlayerInventory[ItemName];
    }
    return 0;
}

bool UQuest_CraftingQuestManager::RemoveItemFromInventory(const FString& ItemName, int32 Quantity)
{
    if (!PlayerInventory.Contains(ItemName))
    {
        return false;
    }
    
    int32 CurrentQuantity = PlayerInventory[ItemName];
    if (CurrentQuantity < Quantity)
    {
        return false;
    }
    
    PlayerInventory[ItemName] = CurrentQuantity - Quantity;
    if (PlayerInventory[ItemName] <= 0)
    {
        PlayerInventory.Remove(ItemName);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest_CraftingQuestManager: Removed %d %s from inventory"), Quantity, *ItemName);
    return true;
}

void UQuest_CraftingQuestManager::AddCraftingObjective(const FString& ObjectiveID, const FString& ItemName, int32 Quantity)
{
    FQuest_CraftingObjective NewObjective;
    NewObjective.ObjectiveID = ObjectiveID;
    NewObjective.ItemToCraft = ItemName;
    NewObjective.QuantityRequired = Quantity;
    NewObjective.CurrentProgress = 0;
    NewObjective.bIsCompleted = false;
    
    ActiveCraftingObjectives.Add(NewObjective);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CraftingQuestManager: Added crafting objective - Craft %d %s"), Quantity, *ItemName);
}

void UQuest_CraftingQuestManager::UpdateCraftingProgress(const FString& ItemName, int32 Quantity)
{
    for (FQuest_CraftingObjective& Objective : ActiveCraftingObjectives)
    {
        if (Objective.ItemToCraft == ItemName && !Objective.bIsCompleted)
        {
            Objective.CurrentProgress += Quantity;
            
            if (Objective.CurrentProgress >= Objective.QuantityRequired)
            {
                Objective.bIsCompleted = true;
                UE_LOG(LogTemp, Warning, TEXT("Quest_CraftingQuestManager: Objective completed - %s"), *Objective.ObjectiveID);
            }
        }
    }
}

bool UQuest_CraftingQuestManager::IsObjectiveCompleted(const FString& ObjectiveID)
{
    for (const FQuest_CraftingObjective& Objective : ActiveCraftingObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective.bIsCompleted;
        }
    }
    return false;
}

void UQuest_CraftingQuestManager::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    
    if (bCraftingMenuOpen)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_CraftingQuestManager: Crafting menu opened"));
        DebugPrintRecipes();
        DebugPrintInventory();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_CraftingQuestManager: Crafting menu closed"));
    }
}

void UQuest_CraftingQuestManager::AddCraftingExperience(int32 Experience)
{
    CraftingExperience += Experience;
    UE_LOG(LogTemp, Log, TEXT("Quest_CraftingQuestManager: Added %d crafting experience (Total: %d)"), Experience, CraftingExperience);
    CheckLevelUp();
}

void UQuest_CraftingQuestManager::CheckLevelUp()
{
    int32 ExperienceNeeded = CraftingLevel * 100; // 100 XP per level
    
    if (CraftingExperience >= ExperienceNeeded)
    {
        CraftingLevel++;
        UE_LOG(LogTemp, Warning, TEXT("Quest_CraftingQuestManager: Level up! New crafting level: %d"), CraftingLevel);
    }
}

void UQuest_CraftingQuestManager::DebugPrintInventory()
{
    UE_LOG(LogTemp, Warning, TEXT("=== INVENTORY ==="));
    for (const auto& Item : PlayerInventory)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: %d"), *Item.Key, Item.Value);
    }
    UE_LOG(LogTemp, Warning, TEXT("================"));
}

void UQuest_CraftingQuestManager::DebugPrintRecipes()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AVAILABLE RECIPES ==="));
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        FString Materials = TEXT("");
        for (int32 i = 0; i < Recipe.RequiredMaterials.Num(); i++)
        {
            Materials += FString::Printf(TEXT("%s(%d) "), *Recipe.RequiredMaterials[i], Recipe.MaterialQuantities[i]);
        }
        
        bool bCanCraft = CanCraftItem(Recipe.RecipeName);
        FString Status = bCanCraft ? TEXT("[CAN CRAFT]") : TEXT("[NEED MATERIALS]");
        
        UE_LOG(LogTemp, Warning, TEXT("%s %s - Requires: %s"), *Status, *Recipe.RecipeName, *Materials);
    }
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}