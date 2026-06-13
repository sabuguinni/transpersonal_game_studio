#include "Arch_InteriorManager.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "UObject/ConstructorHelpers.h"

AArch_InteriorManager::AArch_InteriorManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create interior bounds
    InteriorBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("InteriorBounds"));
    InteriorBounds->SetupAttachment(RootComponent);
    InteriorBounds->SetBoxExtent(FVector(500.0f, 500.0f, 300.0f));

    // Initialize default values
    StructureType = EArch_StructureType::Dwelling;
    InteriorSize = 500.0f;
    bHasFirePit = true;
    bHasSleepingArea = true;
    bHasStorageArea = true;
}

void AArch_InteriorManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Arch_InteriorManager: BeginPlay - Setting up interior"));
    SetupInteriorForStructure(StructureType);
}

void AArch_InteriorManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AArch_InteriorManager::SetupInteriorForStructure(EArch_StructureType InStructureType)
{
    StructureType = InStructureType;
    ClearInterior();

    switch (StructureType)
    {
        case EArch_StructureType::Dwelling:
            SetupDwellingInterior();
            break;
        case EArch_StructureType::Shelter:
            SetupShelterInterior();
            break;
        case EArch_StructureType::Storage:
            SetupStorageInterior();
            break;
        case EArch_StructureType::Workshop:
            SetupWorkshopInterior();
            break;
        case EArch_StructureType::Ruins:
            SetupRuinInterior();
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Interior setup complete for structure type %d"), (int32)StructureType);
}

bool AArch_InteriorManager::AddInteriorItem(EArch_InteriorType ItemType, FVector Location, FRotator Rotation)
{
    // Check if location is within interior bounds
    if (!IsLocationInsideInterior(Location))
    {
        UE_LOG(LogTemp, Warning, TEXT("Arch_InteriorManager: Location outside interior bounds"));
        return false;
    }

    // Create new interior item
    FArch_InteriorItem NewItem;
    NewItem.ItemName = FString::Printf(TEXT("InteriorItem_%d"), InteriorItems.Num() + 1);
    NewItem.ItemType = ItemType;
    NewItem.RelativeLocation = Location - GetActorLocation();
    NewItem.RelativeRotation = Rotation;
    NewItem.bIsInteractable = (ItemType != EArch_InteriorType::Decoration);
    NewItem.WearLevel = FMath::RandRange(0.0f, 0.3f);

    // Add to interior items
    InteriorItems.Add(NewItem);

    // Spawn the visual representation
    SpawnInteriorItem(NewItem);

    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Interior item added: %s"), *NewItem.ItemName);
    return true;
}

void AArch_InteriorManager::RemoveInteriorItem(int32 ItemIndex)
{
    if (InteriorItems.IsValidIndex(ItemIndex))
    {
        InteriorItems.RemoveAt(ItemIndex);
        UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Interior item removed at index %d"), ItemIndex);
    }
}

TArray<FArch_InteriorItem> AArch_InteriorManager::GetInteractableItems()
{
    TArray<FArch_InteriorItem> InteractableItems;
    
    for (const FArch_InteriorItem& Item : InteriorItems)
    {
        if (Item.bIsInteractable)
        {
            InteractableItems.Add(Item);
        }
    }
    
    return InteractableItems;
}

void AArch_InteriorManager::GenerateRandomInterior()
{
    UE_LOG(LogTemp, Warning, TEXT("Arch_InteriorManager: Generating random interior"));
    
    ClearInterior();
    
    // Random structure type
    StructureType = static_cast<EArch_StructureType>(FMath::RandRange(0, 4));
    SetupInteriorForStructure(StructureType);
}

void AArch_InteriorManager::ClearInterior()
{
    InteriorItems.Empty();
    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Interior cleared"));
}

bool AArch_InteriorManager::IsLocationInsideInterior(FVector WorldLocation)
{
    if (InteriorBounds)
    {
        FVector LocalLocation = InteriorBounds->GetComponentTransform().InverseTransformPosition(WorldLocation);
        FVector BoxExtent = InteriorBounds->GetScaledBoxExtent();
        
        return (FMath::Abs(LocalLocation.X) <= BoxExtent.X &&
                FMath::Abs(LocalLocation.Y) <= BoxExtent.Y &&
                FMath::Abs(LocalLocation.Z) <= BoxExtent.Z);
    }
    
    return false;
}

void AArch_InteriorManager::SpawnInteriorItem(const FArch_InteriorItem& Item)
{
    // Create a static mesh component for the item
    UStaticMeshComponent* ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        *FString::Printf(TEXT("ItemMesh_%d"), InteriorItems.Num())
    );
    
    if (ItemMesh)
    {
        ItemMesh->SetupAttachment(RootComponent);
        ItemMesh->SetRelativeLocation(Item.RelativeLocation);
        ItemMesh->SetRelativeRotation(Item.RelativeRotation);
        
        // Apply wear-based scaling
        float Scale = 1.0f - (Item.WearLevel * 0.2f);
        ItemMesh->SetRelativeScale3D(FVector(Scale));
        
        UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Item mesh spawned for %s"), *Item.ItemName);
    }
}

void AArch_InteriorManager::SetupDwellingInterior()
{
    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Setting up dwelling interior"));
    
    // Central fire pit
    if (bHasFirePit)
    {
        AddInteriorItem(EArch_InteriorType::FirePit, FVector(0, 0, 0), FRotator::ZeroRotator);
    }
    
    // Sleeping areas around the edges
    if (bHasSleepingArea)
    {
        for (int32 i = 0; i < 3; i++)
        {
            float Angle = (i * 120.0f) * PI / 180.0f;
            FVector SleepLocation = FVector(FMath::Cos(Angle) * 300.0f, FMath::Sin(Angle) * 300.0f, 0.0f);
            AddInteriorItem(EArch_InteriorType::Furniture, SleepLocation, FRotator(0, i * 120.0f, 0));
        }
    }
    
    // Storage areas
    if (bHasStorageArea)
    {
        AddInteriorItem(EArch_InteriorType::Storage, FVector(200, 200, 0), FRotator::ZeroRotator);
        AddInteriorItem(EArch_InteriorType::Storage, FVector(-200, 200, 0), FRotator::ZeroRotator);
    }
    
    // Decorative elements
    AddInteriorItem(EArch_InteriorType::Decoration, FVector(100, -200, 50), FRotator::ZeroRotator);
    AddInteriorItem(EArch_InteriorType::Decoration, FVector(-100, -200, 50), FRotator::ZeroRotator);
}

void AArch_InteriorManager::SetupShelterInterior()
{
    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Setting up shelter interior"));
    
    // Simple fire pit
    AddInteriorItem(EArch_InteriorType::FirePit, FVector(0, 0, 0), FRotator::ZeroRotator);
    
    // Basic sleeping area
    AddInteriorItem(EArch_InteriorType::Furniture, FVector(150, 0, 0), FRotator::ZeroRotator);
    
    // Minimal storage
    AddInteriorItem(EArch_InteriorType::Storage, FVector(-150, 0, 0), FRotator::ZeroRotator);
}

void AArch_InteriorManager::SetupStorageInterior()
{
    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Setting up storage interior"));
    
    // Multiple storage containers
    for (int32 i = 0; i < 6; i++)
    {
        float Angle = (i * 60.0f) * PI / 180.0f;
        FVector StorageLocation = FVector(FMath::Cos(Angle) * 200.0f, FMath::Sin(Angle) * 200.0f, 0.0f);
        AddInteriorItem(EArch_InteriorType::Storage, StorageLocation, FRotator(0, i * 60.0f, 0));
    }
}

void AArch_InteriorManager::SetupWorkshopInterior()
{
    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Setting up workshop interior"));
    
    // Central work area
    AddInteriorItem(EArch_InteriorType::Furniture, FVector(0, 0, 0), FRotator::ZeroRotator);
    
    // Tool storage around edges
    for (int32 i = 0; i < 4; i++)
    {
        float Angle = (i * 90.0f) * PI / 180.0f;
        FVector ToolLocation = FVector(FMath::Cos(Angle) * 250.0f, FMath::Sin(Angle) * 250.0f, 0.0f);
        AddInteriorItem(EArch_InteriorType::Storage, ToolLocation, FRotator(0, i * 90.0f, 0));
    }
    
    // Fire pit for metalworking
    AddInteriorItem(EArch_InteriorType::FirePit, FVector(200, 0, 0), FRotator::ZeroRotator);
}

void AArch_InteriorManager::SetupRuinInterior()
{
    UE_LOG(LogTemp, Log, TEXT("Arch_InteriorManager: Setting up ruin interior"));
    
    // Scattered debris and remnants
    for (int32 i = 0; i < 5; i++)
    {
        FVector RandomLocation = FVector(
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(-200.0f, 200.0f),
            0.0f
        );
        
        EArch_InteriorType RandomType = static_cast<EArch_InteriorType>(FMath::RandRange(0, 3));
        FRotator RandomRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
        
        AddInteriorItem(RandomType, RandomLocation, RandomRotation);
    }
}