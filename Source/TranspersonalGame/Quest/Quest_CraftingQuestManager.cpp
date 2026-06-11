#include "Quest_CraftingQuestManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AQuest_CraftingQuestManager::AQuest_CraftingQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    bCraftingMenuOpen = false;
    CraftingProgress = 0.0f;
    CurrentlyCrafting = TEXT("");
    
    // Initialize with empty inventory
    PlayerInventory.Empty();
}

void AQuest_CraftingQuestManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBasicRecipes();
    
    // Add some starting materials for testing
    AddItemToInventory(TEXT("Rock"), 5);
    AddItemToInventory(TEXT("Stick"), 3);
    AddItemToInventory(TEXT("Leaf"), 2);
    
    // Create initial crafting objectives
    FQuest_CraftingObjective StoneAxeObjective;
    StoneAxeObjective.ObjectiveID = TEXT("craft_stone_axe");
    StoneAxeObjective.Description = TEXT("Craft a Stone Axe to help with survival");
    StoneAxeObjective.RequiredItem = TEXT("Stone Axe");
    StoneAxeObjective.RequiredQuantity = 1;
    StoneAxeObjective.bIsCompleted = false;
    ActiveObjectives.Add(StoneAxeObjective);
    
    FQuest_CraftingObjective CampfireObjective;
    CampfireObjective.ObjectiveID = TEXT("craft_campfire");
    CampfireObjective.Description = TEXT("Build a Campfire for warmth and cooking");
    CampfireObjective.RequiredItem = TEXT("Campfire");
    CampfireObjective.RequiredQuantity = 1;
    CampfireObjective.bIsCompleted = false;
    ActiveObjectives.Add(CampfireObjective);
    
    UE_LOG(LogTemp, Warning, TEXT("CraftingQuestManager: Initialized with %d recipes and %d objectives"), 
           AvailableRecipes.Num(), ActiveObjectives.Num());
}

void AQuest_CraftingQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!CurrentlyCrafting.IsEmpty())
    {
        UpdateCraftingProgress(DeltaTime);
    }
}

void AQuest_CraftingQuestManager::InitializeBasicRecipes()
{
    AvailableRecipes.Empty();
    
    // Stone Axe Recipe
    FQuest_CraftingRecipe StoneAxeRecipe;
    StoneAxeRecipe.RecipeName = TEXT("Stone Axe");
    StoneAxeRecipe.RequiredMaterials.Add(TEXT("Rock"));
    StoneAxeRecipe.RequiredMaterials.Add(TEXT("Stick"));
    StoneAxeRecipe.RequiredQuantities.Add(2);
    StoneAxeRecipe.RequiredQuantities.Add(1);
    StoneAxeRecipe.CraftedItem = TEXT("Stone Axe");
    StoneAxeRecipe.CraftingTime = 8.0f;
    AvailableRecipes.Add(StoneAxeRecipe);
    
    // Campfire Recipe
    FQuest_CraftingRecipe CampfireRecipe;
    CampfireRecipe.RecipeName = TEXT("Campfire");
    CampfireRecipe.RequiredMaterials.Add(TEXT("Stick"));
    CampfireRecipe.RequiredQuantities.Add(3);
    CampfireRecipe.CraftedItem = TEXT("Campfire");
    CampfireRecipe.CraftingTime = 5.0f;
    AvailableRecipes.Add(CampfireRecipe);
    
    // Water Container Recipe
    FQuest_CraftingRecipe WaterContainerRecipe;
    WaterContainerRecipe.RecipeName = TEXT("Water Container");
    WaterContainerRecipe.RequiredMaterials.Add(TEXT("Rock"));
    WaterContainerRecipe.RequiredMaterials.Add(TEXT("Leaf"));
    WaterContainerRecipe.RequiredQuantities.Add(1);
    WaterContainerRecipe.RequiredQuantities.Add(1);
    WaterContainerRecipe.CraftedItem = TEXT("Water Container");
    WaterContainerRecipe.CraftingTime = 6.0f;
    AvailableRecipes.Add(WaterContainerRecipe);
    
    UE_LOG(LogTemp, Warning, TEXT("CraftingQuestManager: Initialized %d basic recipes"), AvailableRecipes.Num());
}

bool AQuest_CraftingQuestManager::CanCraftItem(const FString& ItemName)
{
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.CraftedItem == ItemName)
        {
            // Check if player has all required materials
            for (int32 i = 0; i < Recipe.RequiredMaterials.Num(); i++)
            {
                const FString& Material = Recipe.RequiredMaterials[i];
                int32 RequiredQty = Recipe.RequiredQuantities.IsValidIndex(i) ? Recipe.RequiredQuantities[i] : 1;
                int32 PlayerQty = GetItemQuantity(Material);
                
                if (PlayerQty < RequiredQty)
                {
                    UE_LOG(LogTemp, Warning, TEXT("CraftingQuestManager: Cannot craft %s - need %d %s, have %d"), 
                           *ItemName, RequiredQty, *Material, PlayerQty);
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

bool AQuest_CraftingQuestManager::StartCrafting(const FString& ItemName)
{
    if (!CanCraftItem(ItemName) || !CurrentlyCrafting.IsEmpty())
    {
        return false;
    }
    
    // Find the recipe
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.CraftedItem == ItemName)
        {
            // Consume materials
            for (int32 i = 0; i < Recipe.RequiredMaterials.Num(); i++)
            {
                const FString& Material = Recipe.RequiredMaterials[i];
                int32 RequiredQty = Recipe.RequiredQuantities.IsValidIndex(i) ? Recipe.RequiredQuantities[i] : 1;
                RemoveItemFromInventory(Material, RequiredQty);
            }
            
            CurrentlyCrafting = ItemName;
            CraftingProgress = 0.0f;
            
            UE_LOG(LogTemp, Warning, TEXT("CraftingQuestManager: Started crafting %s"), *ItemName);
            return true;
        }
    }
    return false;
}

void AQuest_CraftingQuestManager::UpdateCraftingProgress(float DeltaTime)
{
    if (CurrentlyCrafting.IsEmpty())
    {
        return;
    }
    
    // Find crafting time for current item
    float CraftingTime = 5.0f;
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.CraftedItem == CurrentlyCrafting)
        {
            CraftingTime = Recipe.CraftingTime;
            break;
        }
    }
    
    CraftingProgress += DeltaTime;
    
    if (CraftingProgress >= CraftingTime)
    {
        CompleteCrafting();
    }
}

void AQuest_CraftingQuestManager::CompleteCrafting()
{
    if (CurrentlyCrafting.IsEmpty())
    {
        return;
    }
    
    // Add crafted item to inventory
    AddItemToInventory(CurrentlyCrafting, 1);
    
    UE_LOG(LogTemp, Warning, TEXT("CraftingQuestManager: Completed crafting %s"), *CurrentlyCrafting);
    
    // Check if this completes any objectives
    CheckCraftingObjectives();
    
    CurrentlyCrafting = TEXT("");
    CraftingProgress = 0.0f;
}

void AQuest_CraftingQuestManager::AddItemToInventory(const FString& ItemName, int32 Quantity)
{
    if (PlayerInventory.Contains(ItemName))
    {
        PlayerInventory[ItemName] += Quantity;
    }
    else
    {
        PlayerInventory.Add(ItemName, Quantity);
    }
    
    UE_LOG(LogTemp, Log, TEXT("CraftingQuestManager: Added %d %s to inventory"), Quantity, *ItemName);
}

int32 AQuest_CraftingQuestManager::GetItemQuantity(const FString& ItemName)
{
    if (PlayerInventory.Contains(ItemName))
    {
        return PlayerInventory[ItemName];
    }
    return 0;
}

bool AQuest_CraftingQuestManager::RemoveItemFromInventory(const FString& ItemName, int32 Quantity)
{
    if (!PlayerInventory.Contains(ItemName) || PlayerInventory[ItemName] < Quantity)
    {
        return false;
    }
    
    PlayerInventory[ItemName] -= Quantity;
    if (PlayerInventory[ItemName] <= 0)
    {
        PlayerInventory.Remove(ItemName);
    }
    
    UE_LOG(LogTemp, Log, TEXT("CraftingQuestManager: Removed %d %s from inventory"), Quantity, *ItemName);
    return true;
}

void AQuest_CraftingQuestManager::CheckCraftingObjectives()
{
    for (FQuest_CraftingObjective& Objective : ActiveObjectives)
    {
        if (!Objective.bIsCompleted)
        {
            int32 PlayerQty = GetItemQuantity(Objective.RequiredItem);
            if (PlayerQty >= Objective.RequiredQuantity)
            {
                CompleteObjective(Objective.ObjectiveID);
            }
        }
    }
}

void AQuest_CraftingQuestManager::CompleteObjective(const FString& ObjectiveID)
{
    for (FQuest_CraftingObjective& Objective : ActiveObjectives)
    {
        if (Objective.ObjectiveID == ObjectiveID && !Objective.bIsCompleted)
        {
            Objective.bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("CraftingQuestManager: Completed objective %s - %s"), 
                   *ObjectiveID, *Objective.Description);
            
            // TODO: Trigger quest completion events, rewards, etc.
            break;
        }
    }
}

void AQuest_CraftingQuestManager::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    UE_LOG(LogTemp, Warning, TEXT("CraftingQuestManager: Crafting menu %s"), 
           bCraftingMenuOpen ? TEXT("opened") : TEXT("closed"));
}

void AQuest_CraftingQuestManager::DebugPrintInventory()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PLAYER INVENTORY ==="));
    for (const auto& Item : PlayerInventory)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: %d"), *Item.Key, Item.Value);
    }
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void AQuest_CraftingQuestManager::DebugPrintRecipes()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AVAILABLE RECIPES ==="));
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        UE_LOG(LogTemp, Warning, TEXT("Recipe: %s -> %s (%.1fs)"), 
               *Recipe.RecipeName, *Recipe.CraftedItem, Recipe.CraftingTime);
        for (int32 i = 0; i < Recipe.RequiredMaterials.Num(); i++)
        {
            int32 Qty = Recipe.RequiredQuantities.IsValidIndex(i) ? Recipe.RequiredQuantities[i] : 1;
            UE_LOG(LogTemp, Warning, TEXT("  - %d %s"), Qty, *Recipe.RequiredMaterials[i]);
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("========================="));
}