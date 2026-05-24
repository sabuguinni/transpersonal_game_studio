#include "Arch_ShelterManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

AArch_ShelterManager::AArch_ShelterManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Create main structure mesh component
    MainStructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainStructureMesh"));
    MainStructureMesh->SetupAttachment(RootComponent);
    MainStructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MainStructureMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

    // Create support structure mesh component
    SupportStructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SupportStructureMesh"));
    SupportStructureMesh->SetupAttachment(RootComponent);
    SupportStructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SupportStructureMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

    // Initialize default shelter data
    ShelterData.ShelterType = EArch_ShelterType::Cave;
    ShelterData.ProtectionValue = 0.5f;
    ShelterData.MaxOccupants = 2;
    ShelterData.bHasFirePit = false;
    ShelterData.bHasStorage = false;
    ShelterData.TemperatureBonus = 5.0f;

    AssignedBiome = EBiomeType::Savanna;
    ShelterRadius = 500.0f;

    // Try to load default meshes (will be null if not found, that's OK)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        MainStructureMesh->SetStaticMesh(CubeMeshAsset.Object);
        SupportStructureMesh->SetStaticMesh(CubeMeshAsset.Object);
    }
}

void AArch_ShelterManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial configuration
    UpdateMeshBasedOnType();
    ApplyBiomeSpecificMaterials();
    SetupCollisionAndPhysics();

    UE_LOG(LogTemp, Log, TEXT("Arch_ShelterManager: Initialized shelter type %d in biome %d"), 
           (int32)ShelterData.ShelterType, (int32)AssignedBiome);
}

void AArch_ShelterManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Shelter systems can be updated here if needed
    // For now, shelters are static structures
}

void AArch_ShelterManager::InitializeShelter(EArch_ShelterType Type, EBiomeType Biome)
{
    ShelterData.ShelterType = Type;
    AssignedBiome = Biome;

    // Configure shelter properties based on type
    switch (Type)
    {
        case EArch_ShelterType::Cave:
            ShelterData.ProtectionValue = 0.8f;
            ShelterData.MaxOccupants = 4;
            ShelterData.TemperatureBonus = 10.0f;
            break;
            
        case EArch_ShelterType::Platform:
            ShelterData.ProtectionValue = 0.6f;
            ShelterData.MaxOccupants = 3;
            ShelterData.TemperatureBonus = 3.0f;
            break;
            
        case EArch_ShelterType::LeanTo:
            ShelterData.ProtectionValue = 0.4f;
            ShelterData.MaxOccupants = 2;
            ShelterData.TemperatureBonus = 2.0f;
            break;
            
        case EArch_ShelterType::StoneCircle:
            ShelterData.ProtectionValue = 0.3f;
            ShelterData.MaxOccupants = 6;
            ShelterData.TemperatureBonus = 1.0f;
            break;
            
        case EArch_ShelterType::Underground:
            ShelterData.ProtectionValue = 0.9f;
            ShelterData.MaxOccupants = 2;
            ShelterData.TemperatureBonus = 15.0f;
            break;
    }

    UpdateMeshBasedOnType();
    ApplyBiomeSpecificMaterials();

    UE_LOG(LogTemp, Log, TEXT("Arch_ShelterManager: Initialized shelter type %d with protection %f"), 
           (int32)Type, ShelterData.ProtectionValue);
}

void AArch_ShelterManager::SetShelterMesh(UStaticMesh* NewMesh)
{
    if (NewMesh && MainStructureMesh)
    {
        MainStructureMesh->SetStaticMesh(NewMesh);
        UE_LOG(LogTemp, Log, TEXT("Arch_ShelterManager: Set new shelter mesh"));
    }
}

float AArch_ShelterManager::GetProtectionValue() const
{
    return ShelterData.ProtectionValue;
}

bool AArch_ShelterManager::CanAccommodateOccupants(int32 RequestedOccupants) const
{
    return RequestedOccupants <= ShelterData.MaxOccupants;
}

void AArch_ShelterManager::AddFirePit()
{
    if (!ShelterData.bHasFirePit)
    {
        ShelterData.bHasFirePit = true;
        ShelterData.TemperatureBonus += 5.0f;
        ShelterData.ProtectionValue += 0.1f;
        
        UE_LOG(LogTemp, Log, TEXT("Arch_ShelterManager: Added fire pit, new temp bonus: %f"), 
               ShelterData.TemperatureBonus);
    }
}

void AArch_ShelterManager::AddStorageArea()
{
    if (!ShelterData.bHasStorage)
    {
        ShelterData.bHasStorage = true;
        ShelterData.ProtectionValue += 0.05f;
        
        UE_LOG(LogTemp, Log, TEXT("Arch_ShelterManager: Added storage area"));
    }
}

FVector AArch_ShelterManager::GetShelterCenter() const
{
    return GetActorLocation();
}

void AArch_ShelterManager::UpdateMeshBasedOnType()
{
    if (!MainStructureMesh)
        return;

    // Scale and position based on shelter type
    FVector Scale = FVector(1.0f);
    FVector Offset = FVector::ZeroVector;

    switch (ShelterData.ShelterType)
    {
        case EArch_ShelterType::Cave:
            Scale = FVector(3.0f, 2.0f, 2.5f);
            Offset = FVector(0.0f, 0.0f, 100.0f);
            break;
            
        case EArch_ShelterType::Platform:
            Scale = FVector(2.0f, 2.0f, 0.2f);
            Offset = FVector(0.0f, 0.0f, 300.0f);
            break;
            
        case EArch_ShelterType::LeanTo:
            Scale = FVector(1.5f, 1.0f, 1.5f);
            Offset = FVector(0.0f, 0.0f, 50.0f);
            break;
            
        case EArch_ShelterType::StoneCircle:
            Scale = FVector(4.0f, 4.0f, 0.3f);
            Offset = FVector(0.0f, 0.0f, 0.0f);
            break;
            
        case EArch_ShelterType::Underground:
            Scale = FVector(2.0f, 2.0f, 1.0f);
            Offset = FVector(0.0f, 0.0f, -100.0f);
            break;
    }

    MainStructureMesh->SetWorldScale3D(Scale);
    MainStructureMesh->SetRelativeLocation(Offset);
}

void AArch_ShelterManager::ApplyBiomeSpecificMaterials()
{
    // Materials will be applied based on biome when available
    // For now, this is a placeholder for future material assignment
    
    UE_LOG(LogTemp, Log, TEXT("Arch_ShelterManager: Applied biome-specific materials for biome %d"), 
           (int32)AssignedBiome);
}

void AArch_ShelterManager::SetupCollisionAndPhysics()
{
    if (MainStructureMesh)
    {
        MainStructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MainStructureMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
        MainStructureMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        MainStructureMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    }

    if (SupportStructureMesh)
    {
        SupportStructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SupportStructureMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
        SupportStructureMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    }
}