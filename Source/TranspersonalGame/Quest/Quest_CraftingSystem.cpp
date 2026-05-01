#include "Quest_CraftingSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

// ========== AQuest_ResourcePickup Implementation ==========

AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    // Criar componente de mesh
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

    // Criar componente de colisão
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetupAttachment(RootComponent);
    CollisionComponent->SetSphereRadius(150.0f);
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Configurar propriedades padrão
    ResourceItem.ItemName = TEXT("Unknown Resource");
    ResourceItem.ItemType = EQuest_ResourceType::Rock;
    ResourceItem.Quantity = 1;
    ResourceItem.Weight = 1.0f;
    
    RespawnTime = 60.0f;
    bCanRespawn = true;
    bIsCollected = false;
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();
    
    // Configurar overlap events
    if (CollisionComponent)
    {
        CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AQuest_ResourcePickup::OnOverlapBegin);
    }
}

void AQuest_ResourcePickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bIsCollected || !OtherActor)
    {
        return;
    }

    // Verificar se é um character (jogador)
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        CollectResource(Character);
    }
}

void AQuest_ResourcePickup::CollectResource(AActor* Collector)
{
    if (bIsCollected || !Collector)
    {
        return;
    }

    // Tentar encontrar componente de inventário no collector
    UQuest_CraftingInventoryComponent* Inventory = Collector->FindComponentByClass<UQuest_CraftingInventoryComponent>();
    if (!Inventory)
    {
        UE_LOG(LogTemp, Warning, TEXT("Collector does not have CraftingInventoryComponent"));
        return;
    }

    // Tentar adicionar item ao inventário
    if (Inventory->AddItem(ResourceItem))
    {
        bIsCollected = true;
        
        // Esconder o actor
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
        
        UE_LOG(LogTemp, Log, TEXT("Resource collected: %s"), *ResourceItem.ItemName);
        
        // Configurar respawn se permitido
        if (bCanRespawn && RespawnTime > 0.0f)
        {
            GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AQuest_ResourcePickup::RespawnResource, RespawnTime, false);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot collect resource: inventory full or too heavy"));
    }
}

void AQuest_ResourcePickup::RespawnResource()
{
    bIsCollected = false;
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    
    UE_LOG(LogTemp, Log, TEXT("Resource respawned: %s"), *ResourceItem.ItemName);
}

// ========== UQuest_CraftingInventoryComponent Implementation ==========

UQuest_CraftingInventoryComponent::UQuest_CraftingInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    MaxInventorySlots = 20;
    MaxCarryWeight = 50.0f;
}

void UQuest_CraftingInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
}

bool UQuest_CraftingInventoryComponent::AddItem(const FQuest_CraftingItem& Item)
{
    if (!CanAddItem(Item))
    {
        return false;
    }

    // Verificar se já existe um item do mesmo tipo para empilhar
    for (FQuest_CraftingItem& ExistingItem : InventoryItems)
    {
        if (ExistingItem.ItemType == Item.ItemType)
        {
            ExistingItem.Quantity += Item.Quantity;
            return true;
        }
    }

    // Adicionar como novo item se não existe
    if (InventoryItems.Num() < MaxInventorySlots)
    {
        InventoryItems.Add(Item);
        return true;
    }

    return false;
}

bool UQuest_CraftingInventoryComponent::RemoveItem(EQuest_ResourceType ItemType, int32 Quantity)
{
    for (int32 i = InventoryItems.Num() - 1; i >= 0; i--)
    {
        FQuest_CraftingItem& Item = InventoryItems[i];
        if (Item.ItemType == ItemType)
        {
            if (Item.Quantity >= Quantity)
            {
                Item.Quantity -= Quantity;
                if (Item.Quantity <= 0)
                {
                    InventoryItems.RemoveAt(i);
                }
                return true;
            }
        }
    }
    return false;
}

int32 UQuest_CraftingInventoryComponent::GetItemCount(EQuest_ResourceType ItemType) const
{
    for (const FQuest_CraftingItem& Item : InventoryItems)
    {
        if (Item.ItemType == ItemType)
        {
            return Item.Quantity;
        }
    }
    return 0;
}

float UQuest_CraftingInventoryComponent::GetCurrentWeight() const
{
    float TotalWeight = 0.0f;
    for (const FQuest_CraftingItem& Item : InventoryItems)
    {
        TotalWeight += Item.Weight * Item.Quantity;
    }
    return TotalWeight;
}

bool UQuest_CraftingInventoryComponent::CanAddItem(const FQuest_CraftingItem& Item) const
{
    // Verificar peso
    float NewWeight = GetCurrentWeight() + (Item.Weight * Item.Quantity);
    if (NewWeight > MaxCarryWeight)
    {
        return false;
    }

    // Verificar se já existe um item do mesmo tipo (pode empilhar)
    for (const FQuest_CraftingItem& ExistingItem : InventoryItems)
    {
        if (ExistingItem.ItemType == Item.ItemType)
        {
            return true; // Pode empilhar
        }
    }

    // Verificar slots disponíveis
    return InventoryItems.Num() < MaxInventorySlots;
}

// ========== AQuest_CraftingSystem Implementation ==========

AQuest_CraftingSystem::AQuest_CraftingSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    bIsCraftingMenuOpen = false;
    CurrentCrafter = nullptr;
}

void AQuest_CraftingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeRecipes();
}

void AQuest_CraftingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    HandleCraftingInput();
}

void AQuest_CraftingSystem::InitializeRecipes()
{
    CreateBasicRecipes();
}

void AQuest_CraftingSystem::CreateBasicRecipes()
{
    AvailableRecipes.Empty();

    // Receita: Stone Axe (2 rocks + 1 stick)
    FQuest_CraftingRecipe StoneAxeRecipe;
    StoneAxeRecipe.RecipeName = TEXT("Stone Axe");
    StoneAxeRecipe.RecipeDescription = TEXT("A primitive axe made from stone and wood. Useful for cutting and chopping.");
    StoneAxeRecipe.CraftingTime = 10.0f;
    StoneAxeRecipe.bRequiresCampfire = false;

    FQuest_CraftingItem RockRequirement;
    RockRequirement.ItemName = TEXT("Rock");
    RockRequirement.ItemType = EQuest_ResourceType::Rock;
    RockRequirement.Quantity = 2;
    RockRequirement.Weight = 2.0f;

    FQuest_CraftingItem StickRequirement;
    StickRequirement.ItemName = TEXT("Stick");
    StickRequirement.ItemType = EQuest_ResourceType::Stick;
    StickRequirement.Quantity = 1;
    StickRequirement.Weight = 0.5f;

    StoneAxeRecipe.RequiredItems.Add(RockRequirement);
    StoneAxeRecipe.RequiredItems.Add(StickRequirement);

    FQuest_CraftingItem StoneAxeResult;
    StoneAxeResult.ItemName = TEXT("Stone Axe");
    StoneAxeResult.ItemType = EQuest_ResourceType::Bone; // Usando Bone como placeholder para ferramentas
    StoneAxeResult.Quantity = 1;
    StoneAxeResult.Weight = 3.0f;
    StoneAxeRecipe.ResultItem = StoneAxeResult;

    AvailableRecipes.Add(StoneAxeRecipe);

    // Receita: Campfire (3 sticks)
    FQuest_CraftingRecipe CampfireRecipe;
    CampfireRecipe.RecipeName = TEXT("Campfire");
    CampfireRecipe.RecipeDescription = TEXT("A basic campfire for warmth, light, and cooking food.");
    CampfireRecipe.CraftingTime = 15.0f;
    CampfireRecipe.bRequiresCampfire = false;

    FQuest_CraftingItem StickRequirement3;
    StickRequirement3.ItemName = TEXT("Stick");
    StickRequirement3.ItemType = EQuest_ResourceType::Stick;
    StickRequirement3.Quantity = 3;
    StickRequirement3.Weight = 0.5f;

    CampfireRecipe.RequiredItems.Add(StickRequirement3);

    FQuest_CraftingItem CampfireResult;
    CampfireResult.ItemName = TEXT("Campfire");
    CampfireResult.ItemType = EQuest_ResourceType::Fiber; // Usando Fiber como placeholder para estruturas
    CampfireResult.Quantity = 1;
    CampfireResult.Weight = 5.0f;
    CampfireRecipe.ResultItem = CampfireResult;

    AvailableRecipes.Add(CampfireRecipe);

    // Receita: Water Container (1 rock + 1 leaf)
    FQuest_CraftingRecipe WaterContainerRecipe;
    WaterContainerRecipe.RecipeName = TEXT("Water Container");
    WaterContainerRecipe.RecipeDescription = TEXT("A primitive container for carrying water. Essential for survival.");
    WaterContainerRecipe.CraftingTime = 8.0f;
    WaterContainerRecipe.bRequiresCampfire = false;

    FQuest_CraftingItem RockRequirement1;
    RockRequirement1.ItemName = TEXT("Rock");
    RockRequirement1.ItemType = EQuest_ResourceType::Rock;
    RockRequirement1.Quantity = 1;
    RockRequirement1.Weight = 2.0f;

    FQuest_CraftingItem LeafRequirement;
    LeafRequirement.ItemName = TEXT("Leaf");
    LeafRequirement.ItemType = EQuest_ResourceType::Leaf;
    LeafRequirement.Quantity = 1;
    LeafRequirement.Weight = 0.1f;

    WaterContainerRecipe.RequiredItems.Add(RockRequirement1);
    WaterContainerRecipe.RequiredItems.Add(LeafRequirement);

    FQuest_CraftingItem WaterContainerResult;
    WaterContainerResult.ItemName = TEXT("Water Container");
    WaterContainerResult.ItemType = EQuest_ResourceType::Water; // Usando Water como placeholder para containers
    WaterContainerResult.Quantity = 1;
    WaterContainerResult.Weight = 1.5f;
    WaterContainerRecipe.ResultItem = WaterContainerResult;

    AvailableRecipes.Add(WaterContainerRecipe);

    UE_LOG(LogTemp, Log, TEXT("Crafting System: %d recipes initialized"), AvailableRecipes.Num());
}

bool AQuest_CraftingSystem::CanCraftRecipe(const FQuest_CraftingRecipe& Recipe, UQuest_CraftingInventoryComponent* Inventory) const
{
    if (!Inventory)
    {
        return false;
    }

    // Verificar se tem todos os materiais necessários
    for (const FQuest_CraftingItem& RequiredItem : Recipe.RequiredItems)
    {
        int32 AvailableCount = Inventory->GetItemCount(RequiredItem.ItemType);
        if (AvailableCount < RequiredItem.Quantity)
        {
            return false;
        }
    }

    // Verificar se pode adicionar o resultado ao inventário
    return Inventory->CanAddItem(Recipe.ResultItem);
}

bool AQuest_CraftingSystem::CraftItem(const FQuest_CraftingRecipe& Recipe, UQuest_CraftingInventoryComponent* Inventory)
{
    if (!CanCraftRecipe(Recipe, Inventory))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot craft %s: missing materials or inventory full"), *Recipe.RecipeName);
        return false;
    }

    // Remover materiais necessários
    for (const FQuest_CraftingItem& RequiredItem : Recipe.RequiredItems)
    {
        if (!Inventory->RemoveItem(RequiredItem.ItemType, RequiredItem.Quantity))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to remove required item during crafting"));
            return false;
        }
    }

    // Adicionar item resultante
    if (Inventory->AddItem(Recipe.ResultItem))
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully crafted: %s"), *Recipe.RecipeName);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to add crafted item to inventory"));
        return false;
    }
}

void AQuest_CraftingSystem::OpenCraftingMenu(AActor* Crafter)
{
    CurrentCrafter = Crafter;
    bIsCraftingMenuOpen = true;
    
    UE_LOG(LogTemp, Log, TEXT("Crafting menu opened for %s"), Crafter ? *Crafter->GetName() : TEXT("Unknown"));
}

void AQuest_CraftingSystem::CloseCraftingMenu()
{
    CurrentCrafter = nullptr;
    bIsCraftingMenuOpen = false;
    
    UE_LOG(LogTemp, Log, TEXT("Crafting menu closed"));
}

FQuest_CraftingRecipe AQuest_CraftingSystem::GetRecipeByName(const FString& RecipeName) const
{
    for (const FQuest_CraftingRecipe& Recipe : AvailableRecipes)
    {
        if (Recipe.RecipeName == RecipeName)
        {
            return Recipe;
        }
    }
    
    return FQuest_CraftingRecipe(); // Retorna receita vazia se não encontrar
}

void AQuest_CraftingSystem::HandleCraftingInput()
{
    // Verificar input para abrir/fechar menu de crafting
    if (CurrentCrafter && CurrentCrafter->IsA<ACharacter>())
    {
        // Input handling seria implementado aqui
        // Por agora, apenas log para debug
        if (bIsCraftingMenuOpen)
        {
            // Menu está aberto - processar inputs de crafting
        }
    }
}