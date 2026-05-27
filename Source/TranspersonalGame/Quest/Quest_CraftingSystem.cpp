#include "Quest_CraftingSystem.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "TranspersonalGame/TranspersonalCharacter.h"

// Resource Pickup Implementation
AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    // Create collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetupAttachment(RootComponent);
    CollisionSphere->SetSphereRadius(150.0f);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Bind overlap event
    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_ResourcePickup::OnPlayerOverlap);

    // Default values
    ResourceName = TEXT("Stone");
    ResourceQuantity = 1;
    ResourceType = TEXT("Material");
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
    
    // Set random scale variation
    float ScaleVariation = FMath::RandRange(0.8f, 1.2f);
    SetActorScale3D(FVector(ScaleVariation));
}

void AQuest_ResourcePickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Simple floating animation
    float FloatOffset = FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f) * 10.0f;
    FVector CurrentLocation = GetActorLocation();
    CurrentLocation.Z += FloatOffset * DeltaTime;
}

void AQuest_ResourcePickup::OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                           UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, 
                                           bool bFromSweep, const FHitResult& SweepResult)
{
    if (ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(OtherActor))
    {
        // Add resource to player inventory (would need inventory component)
        UE_LOG(LogTemp, Warning, TEXT("Player collected %s x%d"), *ResourceName, ResourceQuantity);
        
        // Destroy the pickup
        Destroy();
    }
}

// Crafting System Implementation
UQuest_CraftingSystem::UQuest_CraftingSystem()
{
    RecipeDataTable = nullptr;
    InitializeDefaultRecipes();
}

void UQuest_CraftingSystem::InitializeDefaultRecipes()
{
    // Initialize default inventory with some starting items
    PlayerInventory.Empty();
    
    // Add some starting resources
    AddItemToInventory(TEXT("Stone"), 5, TEXT("Material"));
    AddItemToInventory(TEXT("Stick"), 3, TEXT("Material"));
    AddItemToInventory(TEXT("Leaf"), 2, TEXT("Material"));
}

bool UQuest_CraftingSystem::CanCraftItem(const FString& RecipeName)
{
    FQuest_CraftingRecipe* Recipe = FindRecipe(RecipeName);
    if (!Recipe)
    {
        return false;
    }

    // Check if player has all required items
    for (int32 i = 0; i < Recipe->RequiredItems.Num(); i++)
    {
        FString RequiredItem = Recipe->RequiredItems[i];
        int32 RequiredQuantity = Recipe->RequiredQuantities.IsValidIndex(i) ? Recipe->RequiredQuantities[i] : 1;
        
        if (GetItemQuantity(RequiredItem) < RequiredQuantity)
        {
            return false;
        }
    }

    return true;
}

bool UQuest_CraftingSystem::CraftItem(const FString& RecipeName)
{
    if (!CanCraftItem(RecipeName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot craft %s - missing materials"), *RecipeName);
        return false;
    }

    FQuest_CraftingRecipe* Recipe = FindRecipe(RecipeName);
    if (!Recipe)
    {
        return false;
    }

    // Remove required items from inventory
    for (int32 i = 0; i < Recipe->RequiredItems.Num(); i++)
    {
        FString RequiredItem = Recipe->RequiredItems[i];
        int32 RequiredQuantity = Recipe->RequiredQuantities.IsValidIndex(i) ? Recipe->RequiredQuantities[i] : 1;
        
        RemoveItemFromInventory(RequiredItem, RequiredQuantity);
    }

    // Add result item to inventory
    AddItemToInventory(Recipe->ResultItem, Recipe->ResultQuantity, TEXT("Crafted"));

    UE_LOG(LogTemp, Warning, TEXT("Successfully crafted %s x%d"), *Recipe->ResultItem, Recipe->ResultQuantity);
    return true;
}

void UQuest_CraftingSystem::AddItemToInventory(const FString& ItemName, int32 Quantity, const FString& ItemType)
{
    // Check if item already exists in inventory
    for (FQuest_InventoryItem& Item : PlayerInventory)
    {
        if (Item.ItemName == ItemName)
        {
            Item.Quantity += Quantity;
            return;
        }
    }

    // Add new item to inventory
    FQuest_InventoryItem NewItem;
    NewItem.ItemName = ItemName;
    NewItem.Quantity = Quantity;
    NewItem.ItemType = ItemType;
    PlayerInventory.Add(NewItem);
}

int32 UQuest_CraftingSystem::GetItemQuantity(const FString& ItemName)
{
    for (const FQuest_InventoryItem& Item : PlayerInventory)
    {
        if (Item.ItemName == ItemName)
        {
            return Item.Quantity;
        }
    }
    return 0;
}

bool UQuest_CraftingSystem::RemoveItemFromInventory(const FString& ItemName, int32 Quantity)
{
    for (int32 i = 0; i < PlayerInventory.Num(); i++)
    {
        if (PlayerInventory[i].ItemName == ItemName)
        {
            if (PlayerInventory[i].Quantity >= Quantity)
            {
                PlayerInventory[i].Quantity -= Quantity;
                
                // Remove item if quantity reaches 0
                if (PlayerInventory[i].Quantity <= 0)
                {
                    PlayerInventory.RemoveAt(i);
                }
                return true;
            }
            break;
        }
    }
    return false;
}

TArray<FQuest_CraftingRecipe> UQuest_CraftingSystem::GetAvailableRecipes()
{
    TArray<FQuest_CraftingRecipe> AvailableRecipes;
    
    // Hardcoded basic recipes for now
    FQuest_CraftingRecipe StoneAxeRecipe;
    StoneAxeRecipe.RecipeName = TEXT("Stone Axe");
    StoneAxeRecipe.RequiredItems = {TEXT("Stone"), TEXT("Stick")};
    StoneAxeRecipe.RequiredQuantities = {2, 1};
    StoneAxeRecipe.ResultItem = TEXT("Stone Axe");
    StoneAxeRecipe.ResultQuantity = 1;
    StoneAxeRecipe.CraftingTime = 5.0f;
    AvailableRecipes.Add(StoneAxeRecipe);

    FQuest_CraftingRecipe CampfireRecipe;
    CampfireRecipe.RecipeName = TEXT("Campfire");
    CampfireRecipe.RequiredItems = {TEXT("Stick"), TEXT("Stone")};
    CampfireRecipe.RequiredQuantities = {3, 1};
    CampfireRecipe.ResultItem = TEXT("Campfire");
    CampfireRecipe.ResultQuantity = 1;
    CampfireRecipe.CraftingTime = 8.0f;
    AvailableRecipes.Add(CampfireRecipe);

    FQuest_CraftingRecipe WaterContainerRecipe;
    WaterContainerRecipe.RecipeName = TEXT("Water Container");
    WaterContainerRecipe.RequiredItems = {TEXT("Stone"), TEXT("Leaf")};
    WaterContainerRecipe.RequiredQuantities = {1, 1};
    WaterContainerRecipe.ResultItem = TEXT("Water Container");
    WaterContainerRecipe.ResultQuantity = 1;
    WaterContainerRecipe.CraftingTime = 3.0f;
    AvailableRecipes.Add(WaterContainerRecipe);

    return AvailableRecipes;
}

FQuest_CraftingRecipe* UQuest_CraftingSystem::FindRecipe(const FString& RecipeName)
{
    TArray<FQuest_CraftingRecipe> Recipes = GetAvailableRecipes();
    
    for (FQuest_CraftingRecipe& Recipe : Recipes)
    {
        if (Recipe.RecipeName == RecipeName)
        {
            return &Recipe;
        }
    }
    
    return nullptr;
}