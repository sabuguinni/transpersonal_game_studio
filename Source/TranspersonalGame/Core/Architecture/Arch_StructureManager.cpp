#include "Arch_StructureManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"

AArch_StructureManager::AArch_StructureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);

    // Initialize default structure data
    StructureData.StructureType = EArch_StructureType::StonePillar;
    StructureData.WeatheringLevel = 0.5f;
    StructureData.bHasMossGrowth = true;
}

void AArch_StructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize structure with default data
    InitializeStructure(StructureData);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Architecture Structure Manager initialized at %s"), 
            *GetActorLocation().ToString()));
    }
}

void AArch_StructureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AArch_StructureManager::InitializeStructure(const FArch_StructureData& InStructureData)
{
    StructureData = InStructureData;
    
    // Set actor transform
    SetActorLocation(StructureData.SpawnLocation);
    SetActorRotation(StructureData.SpawnRotation);
    
    // Update mesh based on structure type
    UpdateStructureMesh();
    
    // Apply weathering and moss effects
    ApplyMaterialEffects();
}

void AArch_StructureManager::ApplyWeathering(float WeatheringAmount)
{
    StructureData.WeatheringLevel = FMath::Clamp(WeatheringAmount, 0.0f, 1.0f);
    ApplyMaterialEffects();
}

void AArch_StructureManager::SetMossGrowth(bool bEnableMoss)
{
    StructureData.bHasMossGrowth = bEnableMoss;
    ApplyMaterialEffects();
}

void AArch_StructureManager::SpawnStructureAtLocation(EArch_StructureType Type, FVector Location, FRotator Rotation)
{
    FArch_StructureData NewStructureData;
    NewStructureData.StructureType = Type;
    NewStructureData.SpawnLocation = Location;
    NewStructureData.SpawnRotation = Rotation;
    NewStructureData.WeatheringLevel = UKismetMathLibrary::RandomFloatInRange(0.3f, 0.8f);
    NewStructureData.bHasMossGrowth = UKismetMathLibrary::RandomBool();
    
    InitializeStructure(NewStructureData);
}

void AArch_StructureManager::GenerateRandomStructures()
{
    if (!GetWorld()) return;
    
    // Generate structures in different biomes
    TArray<FVector> BiomeLocations = {
        FVector(0, 0, 100),           // Savana
        FVector(-50000, -45000, 100), // Pantano
        FVector(-45000, 40000, 100),  // Floresta
        FVector(55000, 0, 100),       // Deserto
        FVector(40000, 50000, 200)    // Montanha
    };
    
    for (const FVector& Location : BiomeLocations)
    {
        // Spawn 3-5 structures per biome
        int32 StructureCount = UKismetMathLibrary::RandomIntegerInRange(3, 5);
        
        for (int32 i = 0; i < StructureCount; i++)
        {
            FVector RandomOffset = FVector(
                UKismetMathLibrary::RandomFloatInRange(-1000.0f, 1000.0f),
                UKismetMathLibrary::RandomFloatInRange(-1000.0f, 1000.0f),
                0.0f
            );
            
            FVector SpawnLoc = Location + RandomOffset;
            FRotator SpawnRot = FRotator(0, UKismetMathLibrary::RandomFloatInRange(0.0f, 360.0f), 0);
            
            EArch_StructureType RandomType = static_cast<EArch_StructureType>(
                UKismetMathLibrary::RandomIntegerInRange(0, 4)
            );
            
            // Spawn new structure actor
            if (AArch_StructureManager* NewStructure = GetWorld()->SpawnActor<AArch_StructureManager>(
                AArch_StructureManager::StaticClass(), SpawnLoc, SpawnRot))
            {
                NewStructure->SpawnStructureAtLocation(RandomType, SpawnLoc, SpawnRot);
            }
        }
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            TEXT("Random structures generated across all biomes"));
    }
}

void AArch_StructureManager::UpdateStructureMesh()
{
    if (!StructureMesh) return;
    
    UStaticMesh* SelectedMesh = GetMeshForStructureType(StructureData.StructureType);
    if (SelectedMesh)
    {
        StructureMesh->SetStaticMesh(SelectedMesh);
    }
    else
    {
        // Use engine default cube as fallback
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
        if (CubeMesh)
        {
            StructureMesh->SetStaticMesh(CubeMesh);
            StructureMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 3.0f)); // Make it pillar-like
        }
    }
}

void AArch_StructureManager::ApplyMaterialEffects()
{
    if (!StructureMesh) return;
    
    // Apply weathered materials if available
    if (WeatheredMaterials.Num() > 0)
    {
        int32 MaterialIndex = FMath::FloorToInt(StructureData.WeatheringLevel * (WeatheredMaterials.Num() - 1));
        MaterialIndex = FMath::Clamp(MaterialIndex, 0, WeatheredMaterials.Num() - 1);
        
        if (WeatheredMaterials[MaterialIndex])
        {
            StructureMesh->SetMaterial(0, WeatheredMaterials[MaterialIndex]);
        }
    }
    
    // Create dynamic material instance for moss effect
    if (UMaterialInterface* CurrentMaterial = StructureMesh->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynMaterial = StructureMesh->CreateDynamicMaterialInstance(0, CurrentMaterial);
        if (DynMaterial)
        {
            DynMaterial->SetScalarParameterValue(TEXT("MossAmount"), StructureData.bHasMossGrowth ? 1.0f : 0.0f);
            DynMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), StructureData.WeatheringLevel);
        }
    }
}

UStaticMesh* AArch_StructureManager::GetMeshForStructureType(EArch_StructureType Type)
{
    switch (Type)
    {
        case EArch_StructureType::StonePillar:
            return PillarMeshes.Num() > 0 ? PillarMeshes[0] : nullptr;
            
        case EArch_StructureType::RockFormation:
            return RockMeshes.Num() > 0 ? RockMeshes[0] : nullptr;
            
        case EArch_StructureType::CaveEntrance:
        case EArch_StructureType::AncientRuin:
        case EArch_StructureType::TribalMarker:
        default:
            // Use pillar mesh as default
            return PillarMeshes.Num() > 0 ? PillarMeshes[0] : nullptr;
    }
}