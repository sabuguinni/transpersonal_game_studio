#include "Quest_CraftingStation.h"
#include "TranspersonalCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

AQuest_CraftingStation::AQuest_CraftingStation()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create station mesh
    StationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StationMesh"));
    StationMesh->SetupAttachment(RootComponent);

    // Try to load a basic mesh for the crafting station
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        StationMesh->SetStaticMesh(CubeMeshAsset.Object);
        StationMesh->SetWorldScale3D(FVector(2.0f, 1.0f, 0.5f)); // Table-like proportions
    }

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(300.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Bind overlap events
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_CraftingStation::OnInteractionOverlapBegin);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AQuest_CraftingStation::OnInteractionOverlapEnd);

    // Initialize crafting state
    bIsCrafting = false;
    CraftingTimer = 0.0f;
    TotalCraftingTime = 0.0f;
    CurrentRecipe = nullptr;
    CurrentCrafter = nullptr;
}

void AQuest_CraftingStation::BeginPlay()
{
    Super::BeginPlay();

    // Create default recipes if none are set
    if (AvailableRecipes.Num() == 0)
    {
        // Note: In a real implementation, these would be loaded from data assets
        // For now, we'll create them programmatically as placeholders
        UE_LOG(LogTemp, Warning, TEXT("Crafting Station initialized with default recipes"));
    }
}

void AQuest_CraftingStation::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle crafting timer
    if (bIsCrafting && CurrentRecipe && CurrentCrafter)
    {
        CraftingTimer -= DeltaTime;
        
        if (CraftingTimer <= 0.0f)
        {
            CompleteCrafting();
        }
    }
}

bool AQuest_CraftingStation::StartCrafting(UQuest_CraftingRecipe* Recipe, ATranspersonalCharacter* Crafter)
{
    if (!Recipe || !Crafter || bIsCrafting)
    {
        return false;
    }

    // Check if player has required resources (placeholder - needs inventory system)
    TMap<EResourceType, int32> PlayerResources;
    // TODO: Get actual player resources from inventory system
    PlayerResources.Add(EResourceType::Stone, 10);
    PlayerResources.Add(EResourceType::Wood, 10);
    PlayerResources.Add(EResourceType::Fiber, 10);

    if (!Recipe->CanCraft(PlayerResources))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                TEXT("Not enough resources to craft this item!"));
        }
        return false;
    }

    // Start crafting
    CurrentRecipe = Recipe;
    CurrentCrafter = Crafter;
    bIsCrafting = true;
    TotalCraftingTime = Recipe->CraftingResult.CraftingTime;
    CraftingTimer = TotalCraftingTime;

    // Consume ingredients (placeholder)
    Recipe->ConsumeIngredients(PlayerResources);

    // Broadcast crafting started
    OnCraftingStarted.Broadcast(TotalCraftingTime);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
            FString::Printf(TEXT("Started crafting %s (%.1fs)"), 
                           *Recipe->RecipeName, TotalCraftingTime));
    }

    UE_LOG(LogTemp, Log, TEXT("Started crafting %s for player %s"), 
           *Recipe->RecipeName, *Crafter->GetName());

    return true;
}

void AQuest_CraftingStation::CancelCrafting()
{
    if (!bIsCrafting)
    {
        return;
    }

    bIsCrafting = false;
    CraftingTimer = 0.0f;
    TotalCraftingTime = 0.0f;
    CurrentRecipe = nullptr;
    CurrentCrafter = nullptr;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Crafting cancelled"));
    }

    UE_LOG(LogTemp, Log, TEXT("Crafting cancelled"));
}

void AQuest_CraftingStation::CompleteCrafting()
{
    if (!bIsCrafting || !CurrentRecipe || !CurrentCrafter)
    {
        return;
    }

    // Add crafted item to player inventory (placeholder)
    EItemType CraftedItem = CurrentRecipe->CraftingResult.ItemType;
    int32 CraftedAmount = CurrentRecipe->CraftingResult.ResultAmount;

    // Broadcast completion
    OnCraftingComplete.Broadcast(CraftedItem, CraftedAmount);

    if (GEngine)
    {
        FString ItemName = TEXT("Unknown Item");
        switch (CraftedItem)
        {
            case EItemType::StoneAxe:
                ItemName = TEXT("Stone Axe");
                break;
            case EItemType::Campfire:
                ItemName = TEXT("Campfire");
                break;
            case EItemType::WaterContainer:
                ItemName = TEXT("Water Container");
                break;
        }

        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Crafted %d %s!"), CraftedAmount, *ItemName));
    }

    UE_LOG(LogTemp, Log, TEXT("Completed crafting %s"), *CurrentRecipe->RecipeName);

    // Reset crafting state
    bIsCrafting = false;
    CraftingTimer = 0.0f;
    TotalCraftingTime = 0.0f;
    CurrentRecipe = nullptr;
    CurrentCrafter = nullptr;
}

TArray<UQuest_CraftingRecipe*> AQuest_CraftingStation::GetAvailableRecipes() const
{
    return AvailableRecipes;
}

bool AQuest_CraftingStation::CanCraftRecipe(UQuest_CraftingRecipe* Recipe, const TMap<EResourceType, int32>& PlayerResources) const
{
    if (!Recipe)
    {
        return false;
    }

    return Recipe->CanCraft(PlayerResources);
}

float AQuest_CraftingStation::GetCraftingProgress() const
{
    if (!bIsCrafting || TotalCraftingTime <= 0.0f)
    {
        return 0.0f;
    }

    return 1.0f - (CraftingTimer / TotalCraftingTime);
}

void AQuest_CraftingStation::OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(OtherActor);
    if (Player)
    {
        OnPlayerEnterRange(Player);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
                TEXT("Press [C] to open crafting menu"));
        }
    }
}

void AQuest_CraftingStation::OnInteractionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(OtherActor);
    if (Player)
    {
        OnPlayerExitRange(Player);
    }
}