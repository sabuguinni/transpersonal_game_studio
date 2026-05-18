#include "ArchitecturalStructureManager.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"

AArchitecturalStructureManager::AArchitecturalStructureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create structure mesh component
    StructureMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMeshComponent"));
    StructureMeshComponent->SetupAttachment(RootComponent);

    // Initialize default structure data
    StructureData.StructureType = EArch_StructureType::StonePillar;
    StructureData.SpawnLocation = FVector::ZeroVector;
    StructureData.SpawnRotation = FRotator::ZeroRotator;
    StructureData.WeatheringLevel = 0.5f;
    StructureData.bHasTribalMarkings = false;
}

void AArchitecturalStructureManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize structure with default data
    InitializeStructure(StructureData);
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalStructureManager initialized at location: %s"), 
           *GetActorLocation().ToString());
}

void AArchitecturalStructureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AArchitecturalStructureManager::InitializeStructure(const FArch_StructureData& InStructureData)
{
    StructureData = InStructureData;
    
    // Set location and rotation
    SetActorLocation(StructureData.SpawnLocation);
    SetActorRotation(StructureData.SpawnRotation);
    
    // Update mesh based on structure type
    UpdateStructureMesh();
    
    // Apply weathering
    ApplyWeatheringMaterial();
    
    // Add tribal markings if needed
    if (StructureData.bHasTribalMarkings)
    {
        AddTribalMarkings(true);
    }
    
    // Trigger blueprint event
    OnStructureInitialized();
    
    UE_LOG(LogTemp, Log, TEXT("Structure initialized: Type=%d, Location=%s, Weathering=%f"), 
           (int32)StructureData.StructureType, 
           *StructureData.SpawnLocation.ToString(), 
           StructureData.WeatheringLevel);
}

void AArchitecturalStructureManager::SetStructureType(EArch_StructureType NewType)
{
    StructureData.StructureType = NewType;
    UpdateStructureMesh();
    
    UE_LOG(LogTemp, Log, TEXT("Structure type changed to: %d"), (int32)NewType);
}

void AArchitecturalStructureManager::ApplyWeathering(float WeatheringAmount)
{
    StructureData.WeatheringLevel = FMath::Clamp(WeatheringAmount, 0.0f, 1.0f);
    ApplyWeatheringMaterial();
    
    // Trigger blueprint event
    OnWeatheringApplied(StructureData.WeatheringLevel);
    
    UE_LOG(LogTemp, Log, TEXT("Weathering applied: %f"), StructureData.WeatheringLevel);
}

void AArchitecturalStructureManager::AddTribalMarkings(bool bEnable)
{
    StructureData.bHasTribalMarkings = bEnable;
    
    // This would typically modify material parameters or swap materials
    if (StructureMeshComponent && StructureMeshComponent->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = StructureMeshComponent->CreateDynamicMaterialInstance(0);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetScalarParameterValue(TEXT("TribalMarkings"), bEnable ? 1.0f : 0.0f);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Tribal markings %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

FVector AArchitecturalStructureManager::GetStructureLocation() const
{
    return StructureData.SpawnLocation;
}

EArch_StructureType AArchitecturalStructureManager::GetStructureType() const
{
    return StructureData.StructureType;
}

void AArchitecturalStructureManager::UpdateStructureMesh()
{
    if (!StructureMeshComponent)
    {
        return;
    }
    
    UStaticMesh* NewMesh = GetMeshForStructureType(StructureData.StructureType);
    if (NewMesh)
    {
        StructureMeshComponent->SetStaticMesh(NewMesh);
        UE_LOG(LogTemp, Log, TEXT("Updated structure mesh for type: %d"), (int32)StructureData.StructureType);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No mesh found for structure type: %d"), (int32)StructureData.StructureType);
    }
}

void AArchitecturalStructureManager::ApplyWeatheringMaterial()
{
    if (!StructureMeshComponent)
    {
        return;
    }
    
    UMaterialInterface* WeatheredMaterial = GetWeatheredMaterial(StructureData.WeatheringLevel);
    if (WeatheredMaterial)
    {
        StructureMeshComponent->SetMaterial(0, WeatheredMaterial);
        UE_LOG(LogTemp, Log, TEXT("Applied weathered material with level: %f"), StructureData.WeatheringLevel);
    }
    else
    {
        // Create dynamic material instance to control weathering
        UMaterialInstanceDynamic* DynamicMaterial = StructureMeshComponent->CreateDynamicMaterialInstance(0);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), StructureData.WeatheringLevel);
            DynamicMaterial->SetScalarParameterValue(TEXT("MossAmount"), StructureData.WeatheringLevel * 0.8f);
            DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.3f + (StructureData.WeatheringLevel * 0.5f));
        }
    }
}

UStaticMesh* AArchitecturalStructureManager::GetMeshForStructureType(EArch_StructureType Type)
{
    switch (Type)
    {
        case EArch_StructureType::StonePillar:
            return PillarMeshes.Num() > 0 ? PillarMeshes[0] : nullptr;
            
        case EArch_StructureType::RockFormation:
            return RockFormationMeshes.Num() > 0 ? RockFormationMeshes[0] : nullptr;
            
        case EArch_StructureType::CaveEntrance:
            return RockFormationMeshes.Num() > 1 ? RockFormationMeshes[1] : nullptr;
            
        case EArch_StructureType::AncientRuin:
            return PillarMeshes.Num() > 1 ? PillarMeshes[1] : nullptr;
            
        case EArch_StructureType::TribalMarker:
            return PillarMeshes.Num() > 2 ? PillarMeshes[2] : nullptr;
            
        default:
            return nullptr;
    }
}

UMaterialInterface* AArchitecturalStructureManager::GetWeatheredMaterial(float WeatheringLevel)
{
    if (WeatheredMaterials.Num() == 0)
    {
        return nullptr;
    }
    
    // Select material based on weathering level
    int32 MaterialIndex = FMath::FloorToInt(WeatheringLevel * WeatheredMaterials.Num());
    MaterialIndex = FMath::Clamp(MaterialIndex, 0, WeatheredMaterials.Num() - 1);
    
    return WeatheredMaterials[MaterialIndex];
}