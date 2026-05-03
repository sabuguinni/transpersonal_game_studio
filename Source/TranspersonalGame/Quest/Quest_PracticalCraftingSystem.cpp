#include "Quest_PracticalCraftingSystem.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "UObject/ConstructorHelpers.h"

// AQuest_ResourcePickup Implementation
AQuest_ResourcePickup::AQuest_ResourcePickup()
{
    PrimaryActorTick.bCanEverTick = false;

    // Criar componente de mesh
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    // Criar componente de colisão
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetupAttachment(RootComponent);
    CollisionComponent->SetSphereRadius(150.0f);
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Configurações padrão
    ResourceType = EQuest_ResourceType::Stone;
    ResourceQuantity = 1;
    bCanBePickedUp = true;

    // Bind do evento de overlap
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AQuest_ResourcePickup::OnPlayerEnterRange);
}

void AQuest_ResourcePickup::BeginPlay()
{
    Super::BeginPlay();

    // Configurar mesh baseado no tipo de recurso
    UStaticMesh* ResourceMesh = nullptr;
    
    switch (ResourceType)
    {
        case EQuest_ResourceType::Stone:
            // Usar mesh básica de cubo para pedra
            ResourceMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
            if (MeshComponent && ResourceMesh)
            {
                MeshComponent->SetStaticMesh(ResourceMesh);
                MeshComponent->SetWorldScale3D(FVector(0.5f, 0.5f, 0.3f));
            }
            break;
            
        case EQuest_ResourceType::Stick:
            // Usar mesh básica de cilindro para pau
            ResourceMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
            if (MeshComponent && ResourceMesh)
            {
                MeshComponent->SetStaticMesh(ResourceMesh);
                MeshComponent->SetWorldScale3D(FVector(0.1f, 0.1f, 1.0f));
                MeshComponent->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
            }
            break;
            
        case EQuest_ResourceType::Leaf:
            // Usar mesh básica de plano para folha
            ResourceMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
            if (MeshComponent && ResourceMesh)
            {
                MeshComponent->SetStaticMesh(ResourceMesh);
                MeshComponent->SetWorldScale3D(FVector(0.3f, 0.3f, 0.1f));
            }
            break;
            
        default:
            // Usar esfera por padrão
            ResourceMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
            if (MeshComponent && ResourceMesh)
            {
                MeshComponent->SetStaticMesh(ResourceMesh);
                MeshComponent->SetWorldScale3D(FVector(0.4f, 0.4f, 0.4f));
            }
            break;
    }
}

void AQuest_ResourcePickup::OnPlayerEnterRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                              bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bCanBePickedUp || !OtherActor)
        return;

    // Verificar se é o jogador
    ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
    if (PlayerCharacter && PlayerCharacter->IsPlayerControlled())
    {
        // Mostrar mensagem de pickup
        FString ResourceName;
        switch (ResourceType)
        {
            case EQuest_ResourceType::Stone: ResourceName = TEXT("Pedra"); break;
            case EQuest_ResourceType::Stick: ResourceName = TEXT("Pau"); break;
            case EQuest_ResourceType::Leaf: ResourceName = TEXT("Folha"); break;
            case EQuest_ResourceType::Bone: ResourceName = TEXT("Osso"); break;
            case EQuest_ResourceType::Hide: ResourceName = TEXT("Pele"); break;
            default: ResourceName = TEXT("Recurso"); break;
        }
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("Pressiona E para apanhar %s (x%d)"), *ResourceName, ResourceQuantity));
        }
    }
}

bool AQuest_ResourcePickup::PickupResource(AActor* Player)
{
    if (!bCanBePickedUp || !Player)
        return false;

    // Aqui seria onde adicionaríamos o recurso ao inventário do jogador
    // Por agora, apenas destruir o actor e mostrar mensagem
    
    FString ResourceName;
    switch (ResourceType)
    {
        case EQuest_ResourceType::Stone: ResourceName = TEXT("Pedra"); break;
        case EQuest_ResourceType::Stick: ResourceName = TEXT("Pau"); break;
        case EQuest_ResourceType::Leaf: ResourceName = TEXT("Folha"); break;
        case EQuest_ResourceType::Bone: ResourceName = TEXT("Osso"); break;
        case EQuest_ResourceType::Hide: ResourceName = TEXT("Pele"); break;
        default: ResourceName = TEXT("Recurso"); break;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("Apanhaste %s x%d!"), *ResourceName, ResourceQuantity));
    }

    // Marcar para destruição
    Destroy();
    return true;
}

// UQuest_PracticalCraftingSystem Implementation
UQuest_PracticalCraftingSystem::UQuest_PracticalCraftingSystem()
{
    bCraftingMenuOpen = false;
    InitializeCraftingSystem();
}

void UQuest_PracticalCraftingSystem::InitializeCraftingSystem()
{
    // Limpar inventário e receitas
    PlayerInventory.Empty();
    AvailableRecipes.Empty();
    
    // Configurar receitas básicas
    SetupBasicRecipes();
    
    // Inicializar inventário vazio
    PlayerInventory.Add(EQuest_ResourceType::Stone, 0);
    PlayerInventory.Add(EQuest_ResourceType::Stick, 0);
    PlayerInventory.Add(EQuest_ResourceType::Leaf, 0);
    PlayerInventory.Add(EQuest_ResourceType::Bone, 0);
    PlayerInventory.Add(EQuest_ResourceType::Hide, 0);
}

void UQuest_PracticalCraftingSystem::SetupBasicRecipes()
{
    // Receita 1: Machado de Pedra (2 pedras + 1 pau)
    FQuest_CraftingRecipe StoneAxeRecipe;
    StoneAxeRecipe.RecipeName = TEXT("Machado de Pedra");
    StoneAxeRecipe.RequiredResources.Add(EQuest_ResourceType::Stone, 2);
    StoneAxeRecipe.RequiredResources.Add(EQuest_ResourceType::Stick, 1);
    StoneAxeRecipe.ResultItem = EQuest_ResourceType::Bone; // Representa ferramenta
    StoneAxeRecipe.ResultQuantity = 1;
    AvailableRecipes.Add(StoneAxeRecipe);

    // Receita 2: Fogueira (3 paus + 2 folhas)
    FQuest_CraftingRecipe CampfireRecipe;
    CampfireRecipe.RecipeName = TEXT("Fogueira");
    CampfireRecipe.RequiredResources.Add(EQuest_ResourceType::Stick, 3);
    CampfireRecipe.RequiredResources.Add(EQuest_ResourceType::Leaf, 2);
    CampfireRecipe.ResultItem = EQuest_ResourceType::Bone; // Representa estrutura
    CampfireRecipe.ResultQuantity = 1;
    AvailableRecipes.Add(CampfireRecipe);

    // Receita 3: Recipiente de Água (1 pedra + 1 folha)
    FQuest_CraftingRecipe WaterContainerRecipe;
    WaterContainerRecipe.RecipeName = TEXT("Recipiente de Água");
    WaterContainerRecipe.RequiredResources.Add(EQuest_ResourceType::Stone, 1);
    WaterContainerRecipe.RequiredResources.Add(EQuest_ResourceType::Leaf, 1);
    WaterContainerRecipe.ResultItem = EQuest_ResourceType::Hide; // Representa recipiente
    WaterContainerRecipe.ResultQuantity = 1;
    AvailableRecipes.Add(WaterContainerRecipe);

    // Receita 4: Lança Simples (2 paus + 1 pedra)
    FQuest_CraftingRecipe SimpleSpearRecipe;
    SimpleSpearRecipe.RecipeName = TEXT("Lança Simples");
    SimpleSpearRecipe.RequiredResources.Add(EQuest_ResourceType::Stick, 2);
    SimpleSpearRecipe.RequiredResources.Add(EQuest_ResourceType::Stone, 1);
    SimpleSpearRecipe.ResultItem = EQuest_ResourceType::Bone; // Representa arma
    SimpleSpearRecipe.ResultQuantity = 1;
    AvailableRecipes.Add(SimpleSpearRecipe);
}

bool UQuest_PracticalCraftingSystem::CanCraftItem(const FQuest_CraftingRecipe& Recipe) const
{
    // Verificar se temos todos os recursos necessários
    for (const auto& RequiredResource : Recipe.RequiredResources)
    {
        EQuest_ResourceType ResourceType = RequiredResource.Key;
        int32 RequiredQuantity = RequiredResource.Value;
        
        const int32* CurrentQuantity = PlayerInventory.Find(ResourceType);
        if (!CurrentQuantity || *CurrentQuantity < RequiredQuantity)
        {
            return false;
        }
    }
    
    return true;
}

bool UQuest_PracticalCraftingSystem::CraftItem(const FQuest_CraftingRecipe& Recipe)
{
    if (!CanCraftItem(Recipe))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                TEXT("Recursos insuficientes para criar este item!"));
        }
        return false;
    }
    
    // Consumir recursos
    ConsumeResources(Recipe.RequiredResources);
    
    // Adicionar item resultante ao inventário
    AddResourceToInventory(Recipe.ResultItem, Recipe.ResultQuantity);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Criaste: %s x%d!"), *Recipe.RecipeName, Recipe.ResultQuantity));
    }
    
    return true;
}

void UQuest_PracticalCraftingSystem::ConsumeResources(const TMap<EQuest_ResourceType, int32>& Resources)
{
    for (const auto& Resource : Resources)
    {
        EQuest_ResourceType ResourceType = Resource.Key;
        int32 ConsumeQuantity = Resource.Value;
        
        int32* CurrentQuantity = PlayerInventory.Find(ResourceType);
        if (CurrentQuantity)
        {
            *CurrentQuantity = FMath::Max(0, *CurrentQuantity - ConsumeQuantity);
        }
    }
}

void UQuest_PracticalCraftingSystem::AddResourceToInventory(EQuest_ResourceType ResourceType, int32 Quantity)
{
    int32* CurrentQuantity = PlayerInventory.Find(ResourceType);
    if (CurrentQuantity)
    {
        *CurrentQuantity += Quantity;
    }
    else
    {
        PlayerInventory.Add(ResourceType, Quantity);
    }
}

int32 UQuest_PracticalCraftingSystem::GetResourceCount(EQuest_ResourceType ResourceType) const
{
    const int32* CurrentQuantity = PlayerInventory.Find(ResourceType);
    return CurrentQuantity ? *CurrentQuantity : 0;
}

void UQuest_PracticalCraftingSystem::ToggleCraftingMenu()
{
    bCraftingMenuOpen = !bCraftingMenuOpen;
    
    if (GEngine)
    {
        if (bCraftingMenuOpen)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, 
                TEXT("Menu de Crafting Aberto! (Pressiona C novamente para fechar)"));
            
            // Mostrar receitas disponíveis
            for (const auto& Recipe : AvailableRecipes)
            {
                FString CanCraft = CanCraftItem(Recipe) ? TEXT("[DISPONÍVEL]") : TEXT("[BLOQUEADO]");
                GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::White, 
                    FString::Printf(TEXT("%s %s"), *CanCraft, *Recipe.RecipeName));
            }
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, 
                TEXT("Menu de Crafting Fechado"));
        }
    }
}