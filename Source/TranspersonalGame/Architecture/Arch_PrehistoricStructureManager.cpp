#include "Arch_PrehistoricStructureManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"

AArch_PrehistoricStructureManager::AArch_PrehistoricStructureManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    MaxStructuresPerBiome = 10;
    MinDistanceBetweenStructures = 5000.0f;
    WeatheredStoneMaterial = nullptr;
    MossyRockMaterial = nullptr;
    StoneCircleMesh = nullptr;
    CaveEntranceMesh = nullptr;
    RockFormationMesh = nullptr;
}

void AArch_PrehistoricStructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultMeshes();
    InitializeDefaultMaterials();
    
    UE_LOG(LogTemp, Warning, TEXT("Arch_PrehistoricStructureManager: Initialized with %d predefined structures"), StructureDataArray.Num());
}

void AArch_PrehistoricStructureManager::SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, FRotator Rotation)
{
    if (!IsValidStructureLocation(Location))
    {
        UE_LOG(LogTemp, Warning, TEXT("Arch_PrehistoricStructureManager: Invalid location for structure spawn"));
        return;
    }

    UStaticMeshComponent* StructureComponent = CreateStructureComponent(StructureType);
    if (!StructureComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Arch_PrehistoricStructureManager: Failed to create structure component"));
        return;
    }

    // Set transform
    StructureComponent->SetWorldLocation(Location);
    StructureComponent->SetWorldRotation(Rotation);
    
    // Apply random scale variation
    float ScaleVariation = FMath::RandRange(0.8f, 1.2f);
    StructureComponent->SetWorldScale3D(FVector(ScaleVariation));

    // Apply weathering and moss
    float WeatheringLevel = FMath::RandRange(0.3f, 0.9f);
    bool bHasMoss = FMath::RandBool();
    ApplyMaterialToStructure(StructureComponent, WeatheringLevel, bHasMoss);

    // Store structure data
    FArch_StructureData NewStructureData;
    NewStructureData.StructureType = StructureType;
    NewStructureData.Location = Location;
    NewStructureData.Rotation = Rotation;
    NewStructureData.Scale = FVector(ScaleVariation);
    NewStructureData.WeatheringLevel = WeatheringLevel;
    NewStructureData.bHasMossGrowth = bHasMoss;
    StructureDataArray.Add(NewStructureData);

    SpawnedStructures.Add(StructureComponent);
    
    UE_LOG(LogTemp, Log, TEXT("Arch_PrehistoricStructureManager: Spawned structure type %d at location %s"), 
           (int32)StructureType, *Location.ToString());
}

void AArch_PrehistoricStructureManager::GenerateRandomStructures(int32 Count, float Radius)
{
    FVector CenterLocation = GetActorLocation();
    
    for (int32 i = 0; i < Count; i++)
    {
        // Generate random location within radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );
        
        FVector SpawnLocation = CenterLocation + RandomOffset;
        
        // Random structure type
        EArch_StructureType RandomType = static_cast<EArch_StructureType>(
            FMath::RandRange(0, static_cast<int32>(EArch_StructureType::NaturalArch))
        );
        
        // Random rotation
        FRotator RandomRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        
        SpawnStructureAtLocation(RandomType, SpawnLocation, RandomRotation);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Arch_PrehistoricStructureManager: Generated %d random structures"), Count);
}

void AArch_PrehistoricStructureManager::ApplyWeathering(float WeatheringIntensity)
{
    for (UStaticMeshComponent* StructureComponent : SpawnedStructures)
    {
        if (StructureComponent)
        {
            ApplyMaterialToStructure(StructureComponent, WeatheringIntensity, FMath::RandBool());
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Arch_PrehistoricStructureManager: Applied weathering intensity %f to %d structures"), 
           WeatheringIntensity, SpawnedStructures.Num());
}

void AArch_PrehistoricStructureManager::ClearAllStructures()
{
    for (UStaticMeshComponent* StructureComponent : SpawnedStructures)
    {
        if (StructureComponent)
        {
            StructureComponent->DestroyComponent();
        }
    }
    
    SpawnedStructures.Empty();
    StructureDataArray.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Arch_PrehistoricStructureManager: Cleared all structures"));
}

TArray<FArch_StructureData> AArch_PrehistoricStructureManager::GetNearbyStructures(FVector CenterLocation, float SearchRadius)
{
    TArray<FArch_StructureData> NearbyStructures;
    
    for (const FArch_StructureData& StructureData : StructureDataArray)
    {
        float Distance = FVector::Dist(CenterLocation, StructureData.Location);
        if (Distance <= SearchRadius)
        {
            NearbyStructures.Add(StructureData);
        }
    }
    
    return NearbyStructures;
}

UStaticMeshComponent* AArch_PrehistoricStructureManager::CreateStructureComponent(EArch_StructureType StructureType)
{
    UStaticMeshComponent* NewComponent = CreateDefaultSubobject<UStaticMeshComponent>(
        *FString::Printf(TEXT("Structure_%d"), SpawnedStructures.Num())
    );
    
    if (!NewComponent)
    {
        return nullptr;
    }
    
    NewComponent->SetupAttachment(RootComponent);
    
    // Set appropriate mesh based on structure type
    UStaticMesh* MeshToUse = nullptr;
    switch (StructureType)
    {
        case EArch_StructureType::StoneCircle:
            MeshToUse = StoneCircleMesh;
            break;
        case EArch_StructureType::CaveEntrance:
            MeshToUse = CaveEntranceMesh;
            break;
        case EArch_StructureType::RockFormation:
        case EArch_StructureType::AncientRuin:
        case EArch_StructureType::NaturalArch:
            MeshToUse = RockFormationMesh;
            break;
    }
    
    if (MeshToUse)
    {
        NewComponent->SetStaticMesh(MeshToUse);
    }
    else
    {
        // Use engine default cube as fallback
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
        if (CubeMesh)
        {
            NewComponent->SetStaticMesh(CubeMesh);
        }
    }
    
    return NewComponent;
}

void AArch_PrehistoricStructureManager::ApplyMaterialToStructure(UStaticMeshComponent* MeshComponent, float WeatheringLevel, bool bHasMoss)
{
    if (!MeshComponent)
    {
        return;
    }
    
    UMaterialInterface* MaterialToApply = nullptr;
    
    if (bHasMoss && MossyRockMaterial)
    {
        MaterialToApply = MossyRockMaterial;
    }
    else if (WeatheredStoneMaterial)
    {
        MaterialToApply = WeatheredStoneMaterial;
    }
    
    if (MaterialToApply)
    {
        // Create dynamic material instance to adjust weathering parameters
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(MaterialToApply, this);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringIntensity"), WeatheringLevel);
            DynamicMaterial->SetScalarParameterValue(TEXT("MossAmount"), bHasMoss ? 1.0f : 0.0f);
            MeshComponent->SetMaterial(0, DynamicMaterial);
        }
    }
}

bool AArch_PrehistoricStructureManager::IsValidStructureLocation(FVector Location)
{
    // Check minimum distance from existing structures
    for (const FArch_StructureData& ExistingStructure : StructureDataArray)
    {
        float Distance = FVector::Dist(Location, ExistingStructure.Location);
        if (Distance < MinDistanceBetweenStructures)
        {
            return false;
        }
    }
    
    // Check if location is within reasonable bounds
    float MaxDistance = 100000.0f; // 1km from origin
    if (FVector::Dist(Location, FVector::ZeroVector) > MaxDistance)
    {
        return false;
    }
    
    return true;
}

void AArch_PrehistoricStructureManager::InitializeDefaultMeshes()
{
    // Try to load default meshes from engine content
    if (!StoneCircleMesh)
    {
        StoneCircleMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
    }
    
    if (!CaveEntranceMesh)
    {
        CaveEntranceMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
    }
    
    if (!RockFormationMesh)
    {
        RockFormationMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
    }
}

void AArch_PrehistoricStructureManager::InitializeDefaultMaterials()
{
    // Try to load default materials from engine content
    if (!WeatheredStoneMaterial)
    {
        WeatheredStoneMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    }
    
    if (!MossyRockMaterial)
    {
        MossyRockMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    }
}