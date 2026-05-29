#include "Quest_CraftingSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

// UQuest_CraftingInventory Implementation
UQuest_CraftingInventory::UQuest_CraftingInventory()
{
    PrimaryComponentTick.bCanEverTick = false;
    Materials.Empty();
}

void UQuest_CraftingInventory::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize with some basic materials for testing
    Materials.Add("Rock", 0);
    Materials.Add("Stick", 0);
    Materials.Add("Leaf", 0);
    Materials.Add("Vine", 0);
    Materials.Add("Flint", 0);
}

bool UQuest_CraftingInventory::AddMaterial(const FString& MaterialID, int32 Quantity)
{
    if (Quantity <= 0) return false;
    
    if (Materials.Contains(MaterialID))
    {
        Materials[MaterialID] += Quantity;
    }
    else
    {
        Materials.Add(MaterialID, Quantity);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Added %d %s to inventory. Total: %d"), Quantity, *MaterialID, Materials[MaterialID]);
    return true;
}

bool UQuest_CraftingInventory::RemoveMaterial(const FString& MaterialID, int32 Quantity)
{
    if (Quantity <= 0) return false;
    
    if (!Materials.Contains(MaterialID) || Materials[MaterialID] < Quantity)
    {
        return false;
    }
    
    Materials[MaterialID] -= Quantity;
    if (Materials[MaterialID] <= 0)
    {
        Materials.Remove(MaterialID);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Removed %d %s from inventory"), Quantity, *MaterialID);
    return true;
}

bool UQuest_CraftingInventory::HasMaterialsForRecipe(const FQuest_CraftingRecipe& Recipe)
{
    for (const auto& RequiredMaterial : Recipe.RequiredMaterials)
    {
        const FString& MaterialID = RequiredMaterial.Key;
        int32 RequiredQuantity = RequiredMaterial.Value;
        
        if (!Materials.Contains(MaterialID) || Materials[MaterialID] < RequiredQuantity)
        {
            return false;
        }
    }
    return true;
}

int32 UQuest_CraftingInventory::GetMaterialCount(const FString& MaterialID)
{
    if (Materials.Contains(MaterialID))
    {
        return Materials[MaterialID];
    }
    return 0;
}

// UQuest_CraftingSystem Implementation
UQuest_CraftingSystem::UQuest_CraftingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    bIsCrafting = false;
    CurrentRecipeID = "";
    CraftingProgress = 0.0f;
    CraftingTimeRemaining = 0.0f;
    PlayerInventory = nullptr;
}

void UQuest_CraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCraftingSystem();
    FindPlayerInventory();
}

void UQuest_CraftingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsCrafting)
    {
        UpdateCrafting(DeltaTime);
    }
}

void UQuest_CraftingSystem::InitializeCraftingSystem()
{
    AvailableRecipes.Empty();
    SetupBasicRecipes();
    
    UE_LOG(LogTemp, Log, TEXT("Crafting System initialized with %d recipes"), AvailableRecipes.Num());
}

void UQuest_CraftingSystem::SetupBasicRecipes()
{
    // Stone Axe Recipe
    FQuest_CraftingRecipe StoneAxeRecipe;
    StoneAxeRecipe.RecipeID = "StoneAxe";
    StoneAxeRecipe.DisplayName = "Stone Axe";
    StoneAxeRecipe.Description = "A primitive axe made from rocks and sticks. Essential for chopping wood and basic combat.";
    StoneAxeRecipe.RequiredMaterials.Add("Rock", 2);
    StoneAxeRecipe.RequiredMaterials.Add("Stick", 1);
    StoneAxeRecipe.RequiredMaterials.Add("Vine", 1);
    StoneAxeRecipe.ResultItem = "StoneAxe";
    StoneAxeRecipe.ResultQuantity = 1;
    StoneAxeRecipe.CraftingTime = 8.0f;
    StoneAxeRecipe.ExperienceGained = 25;
    AvailableRecipes.Add(StoneAxeRecipe);
    
    // Campfire Recipe
    FQuest_CraftingRecipe CampfireRecipe;
    CampfireRecipe.RecipeID = "Campfire";
    CampfireRecipe.DisplayName = "Campfire";
    CampfireRecipe.Description = "A basic campfire for cooking food, warmth, and protection from predators at night.";
    CampfireRecipe.RequiredMaterials.Add("Stick", 5);
    CampfireRecipe.RequiredMaterials.Add("Rock", 3);
    CampfireRecipe.RequiredMaterials.Add("Leaf", 3);
    CampfireRecipe.ResultItem = "Campfire";
    CampfireRecipe.ResultQuantity = 1;
    CampfireRecipe.CraftingTime = 12.0f;
    CampfireRecipe.ExperienceGained = 40;
    AvailableRecipes.Add(CampfireRecipe);
    
    // Water Container Recipe
    FQuest_CraftingRecipe WaterContainerRecipe;
    WaterContainerRecipe.RecipeID = "WaterContainer";
    WaterContainerRecipe.DisplayName = "Water Container";
    WaterContainerRecipe.Description = "A primitive water container made from large leaves and vines. Stores water for survival.";
    WaterContainerRecipe.RequiredMaterials.Add("Leaf", 4);
    WaterContainerRecipe.RequiredMaterials.Add("Vine", 2);
    WaterContainerRecipe.ResultItem = "WaterContainer";
    WaterContainerRecipe.ResultQuantity = 1;
    WaterContainerRecipe.CraftingTime = 6.0f;
    WaterContainerRecipe.ExperienceGained = 20;
    AvailableRecipes.Add(WaterContainerRecipe);
    
    // Stone Spear Recipe
    FQuest_CraftingRecipe StoneSpearRecipe;
    StoneSpearRecipe.RecipeID = "StoneSpear";
    StoneSpearRecipe.DisplayName = "Stone Spear";
    StoneSpearRecipe.Description = "A hunting spear with a sharp stone tip. Effective for hunting small dinosaurs and defense.";
    StoneSpearRecipe.RequiredMaterials.Add("Stick", 2);
    StoneSpearRecipe.RequiredMaterials.Add("Flint", 1);
    StoneSpearRecipe.RequiredMaterials.Add("Vine", 1);
    StoneSpearRecipe.ResultItem = "StoneSpear";
    StoneSpearRecipe.ResultQuantity = 1;
    StoneSpearRecipe.CraftingTime = 10.0f;
    StoneSpearRecipe.ExperienceGained = 30;
    AvailableRecipes.Add(StoneSpearRecipe);
    
    // Shelter Frame Recipe
    FQuest_CraftingRecipe ShelterRecipe;
    ShelterRecipe.RecipeID = "ShelterFrame";
    ShelterRecipe.DisplayName = "Shelter Frame";
    ShelterRecipe.Description = "A basic shelter frame for protection from weather and predators.";
    ShelterRecipe.RequiredMaterials.Add("Stick", 8);
    ShelterRecipe.RequiredMaterials.Add("Vine", 4);
    ShelterRecipe.RequiredMaterials.Add("Leaf", 6);
    ShelterRecipe.ResultItem = "ShelterFrame";
    ShelterRecipe.ResultQuantity = 1;
    ShelterRecipe.CraftingTime = 20.0f;
    ShelterRecipe.ExperienceGained = 60;
    AvailableRecipes.Add(ShelterRecipe);
}

bool UQuest_CraftingSystem::StartCrafting(const FString& RecipeID)
{
    if (bIsCrafting)
    {
        UE_LOG(LogTemp, Warning, TEXT("Already crafting. Cancel current crafting first."));
        return false;
    }
    
    if (!PlayerInventory)
    {
        UE_LOG(LogTemp, Error, TEXT("No player inventory found"));
        return false;
    }
    
    FQuest_CraftingRecipe Recipe = GetRecipeByID(RecipeID);
    if (Recipe.RecipeID.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Recipe not found: %s"), *RecipeID);
        return false;
    }
    
    if (!PlayerInventory->HasMaterialsForRecipe(Recipe))
    {
        UE_LOG(LogTemp, Warning, TEXT("Insufficient materials for recipe: %s"), *RecipeID);
        return false;
    }
    
    // Consume materials
    for (const auto& RequiredMaterial : Recipe.RequiredMaterials)
    {
        PlayerInventory->RemoveMaterial(RequiredMaterial.Key, RequiredMaterial.Value);
    }
    
    // Start crafting
    bIsCrafting = true;
    CurrentRecipeID = RecipeID;
    CraftingProgress = 0.0f;
    CraftingTimeRemaining = Recipe.CraftingTime;
    
    OnCraftingStarted(RecipeID);
    UE_LOG(LogTemp, Log, TEXT("Started crafting: %s (%.1fs)"), *Recipe.DisplayName, Recipe.CraftingTime);
    
    return true;
}

void UQuest_CraftingSystem::CancelCrafting()
{
    if (!bIsCrafting) return;
    
    // Return materials to inventory (partial refund)
    FQuest_CraftingRecipe Recipe = GetRecipeByID(CurrentRecipeID);
    if (!Recipe.RecipeID.IsEmpty() && PlayerInventory)
    {
        float RefundPercentage = 0.5f; // Return 50% of materials
        for (const auto& RequiredMaterial : Recipe.RequiredMaterials)
        {
            int32 RefundAmount = FMath::FloorToInt(RequiredMaterial.Value * RefundPercentage);
            if (RefundAmount > 0)
            {
                PlayerInventory->AddMaterial(RequiredMaterial.Key, RefundAmount);
            }
        }
    }
    
    bIsCrafting = false;
    CurrentRecipeID = "";
    CraftingProgress = 0.0f;
    CraftingTimeRemaining = 0.0f;
    
    OnCraftingCancelled();
    UE_LOG(LogTemp, Log, TEXT("Crafting cancelled"));
}

void UQuest_CraftingSystem::UpdateCrafting(float DeltaTime)
{
    if (!bIsCrafting) return;
    
    CraftingTimeRemaining -= DeltaTime;
    
    FQuest_CraftingRecipe Recipe = GetRecipeByID(CurrentRecipeID);
    if (!Recipe.RecipeID.IsEmpty())
    {
        CraftingProgress = 1.0f - (CraftingTimeRemaining / Recipe.CraftingTime);
        CraftingProgress = FMath::Clamp(CraftingProgress, 0.0f, 1.0f);
        
        OnCraftingProgress(CraftingProgress);
    }
    
    if (CraftingTimeRemaining <= 0.0f)
    {
        CompleteCrafting();
    }
}

void UQuest_CraftingSystem::CompleteCrafting()
{
    if (!bIsCrafting) return;
    
    FQuest_CraftingRecipe Recipe = GetRecipeByID(CurrentRecipeID);
    if (Recipe.RecipeID.IsEmpty()) return;
    
    // Add result item to inventory
    if (PlayerInventory)
    {
        PlayerInventory->AddMaterial(Recipe.ResultItem, Recipe.ResultQuantity);
    }
    
    // Reset crafting state
    bIsCrafting = false;
    FString CompletedItem = Recipe.DisplayName;
    CurrentRecipeID = "";
    CraftingProgress = 1.0f;
    CraftingTimeRemaining = 0.0f;
    
    OnCraftingCompleted(CompletedItem);
    UE_LOG(LogTemp, Log, TEXT("Crafting completed: %s (+%d XP)"), *CompletedItem, Recipe.ExperienceGained);
}

FQuest_CraftingRecipe UQuest_CraftingSystem::GetRecipeByID(const FString& RecipeID)
{
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.RecipeID == RecipeID)
        {
            return Recipe;
        }
    }
    return FQuest_CraftingRecipe(); // Empty recipe if not found
}

bool UQuest_CraftingSystem::CanCraftRecipe(const FString& RecipeID)
{
    if (!PlayerInventory) return false;
    
    FQuest_CraftingRecipe Recipe = GetRecipeByID(RecipeID);
    if (Recipe.RecipeID.IsEmpty()) return false;
    
    return PlayerInventory->HasMaterialsForRecipe(Recipe);
}

TArray<FQuest_CraftingRecipe> UQuest_CraftingSystem::GetCraftableRecipes()
{
    TArray<FQuest_CraftingRecipe> CraftableRecipes;
    
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (CanCraftRecipe(Recipe.RecipeID))
        {
            CraftableRecipes.Add(Recipe);
        }
    }
    
    return CraftableRecipes;
}

void UQuest_CraftingSystem::FindPlayerInventory()
{
    AActor* Owner = GetOwner();
    if (Owner)
    {
        PlayerInventory = Owner->FindComponentByClass<UQuest_CraftingInventory>();
        if (!PlayerInventory)
        {
            // Create inventory component if not found
            PlayerInventory = NewObject<UQuest_CraftingInventory>(Owner);
            Owner->AddInstanceComponent(PlayerInventory);
            PlayerInventory->RegisterComponent();
        }
    }
}

// AQuest_ResourcePickup Implementation
AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    
    // Create collision component
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetupAttachment(RootComponent);
    CollisionComponent->SetSphereRadius(100.0f);
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    // Default values
    MaterialID = "Rock";
    DisplayName = "Rock";
    Quantity = 1;
    RespawnTime = 60.0f;
    bRespawns = true;
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind overlap event
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AQuest_ResourcePickup::OnOverlapBegin);
}

void AQuest_ResourcePickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        OnPickup(OtherActor);
    }
}

void AQuest_ResourcePickup::OnPickup(AActor* Player)
{
    if (!CanPickup(Player)) return;
    
    // Find player's crafting inventory
    UQuest_CraftingInventory* PlayerInventory = Player->FindComponentByClass<UQuest_CraftingInventory>();
    if (PlayerInventory)
    {
        PlayerInventory->AddMaterial(MaterialID, Quantity);
        
        UE_LOG(LogTemp, Log, TEXT("Player picked up %d %s"), Quantity, *DisplayName);
        
        // Hide the pickup
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
        
        // Schedule respawn if enabled
        if (bRespawns && RespawnTime > 0.0f)
        {
            GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &AQuest_ResourcePickup::RespawnResource, RespawnTime, false);
        }
        else
        {
            // Destroy if no respawn
            Destroy();
        }
    }
}

bool AQuest_ResourcePickup::CanPickup(AActor* Player)
{
    return Player && Player->IsA<ACharacter>() && !IsActorHiddenInGame();
}

void AQuest_ResourcePickup::RespawnResource()
{
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    UE_LOG(LogTemp, Log, TEXT("Resource respawned: %s"), *DisplayName);
}