#include "EnvArt_VegetationSystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

AEnvArt_VegetationSystem::AEnvArt_VegetationSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Create instanced mesh components for different vegetation types
    FernInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FernInstances"));
    FernInstances->SetupAttachment(RootComponent);
    FernInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    TreeInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TreeInstances"));
    TreeInstances->SetupAttachment(RootComponent);
    TreeInstances->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    CycadInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CycadInstances"));
    CycadInstances->SetupAttachment(RootComponent);
    CycadInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize default values
    SpawnRadius = 5000.0f;
    MaxInstancesPerType = 500;
    WindStrength = 1.0f;
    WindDirection = FVector(1.0f, 0.0f, 0.0f);
    LODDistance = 10000.0f;
    bEnableCulling = true;
    WindTimer = 0.0f;
    CurrentWindOffset = FVector::ZeroVector;

    InitializeVegetationData();
}

void AEnvArt_VegetationSystem::BeginPlay()
{
    Super::BeginPlay();

    // Set LOD distances for performance
    if (FernInstances)
    {
        FernInstances->SetCullDistance(LODDistance * 0.5f);
    }
    if (TreeInstances)
    {
        TreeInstances->SetCullDistance(LODDistance);
    }
    if (CycadInstances)
    {
        CycadInstances->SetCullDistance(LODDistance * 0.7f);
    }
}

void AEnvArt_VegetationSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (WindStrength > 0.0f)
    {
        UpdateWindEffect(DeltaTime);
    }
}

void AEnvArt_VegetationSystem::InitializeVegetationData()
{
    VegetationTypes.Empty();

    // Fern vegetation
    FEnvArt_VegetationData FernData;
    FernData.VegetationType = EEnvArt_VegetationType::Fern;
    FernData.ScaleRange = FVector(0.8f, 1.5f, 1.0f);
    FernData.SpawnDensity = 2.0f;
    FernData.WindIntensity = 0.8f;
    VegetationTypes.Add(FernData);

    // Conifer tree vegetation
    FEnvArt_VegetationData TreeData;
    TreeData.VegetationType = EEnvArt_VegetationType::ConiferTree;
    TreeData.ScaleRange = FVector(0.9f, 1.3f, 1.0f);
    TreeData.SpawnDensity = 0.3f;
    TreeData.WindIntensity = 0.2f;
    VegetationTypes.Add(TreeData);

    // Cycad plant vegetation
    FEnvArt_VegetationData CycadData;
    CycadData.VegetationType = EEnvArt_VegetationType::CycadPlant;
    CycadData.ScaleRange = FVector(0.7f, 1.2f, 1.0f);
    CycadData.SpawnDensity = 1.5f;
    CycadData.WindIntensity = 0.6f;
    VegetationTypes.Add(CycadData);
}

void AEnvArt_VegetationSystem::SpawnVegetationInRadius(const FVector& Center, float Radius, EEnvArt_VegetationType Type, int32 Count)
{
    UInstancedStaticMeshComponent* TargetComponent = GetInstancedMeshForType(Type);
    if (!TargetComponent)
    {
        return;
    }

    for (int32 i = 0; i < Count; ++i)
    {
        FVector SpawnLocation = GetRandomSpawnLocation(Center, Radius);
        FRotator SpawnRotation = GetRandomRotation();
        
        // Find vegetation data for scaling
        FVector Scale = FVector::OneVector;
        for (const FEnvArt_VegetationData& VegData : VegetationTypes)
        {
            if (VegData.VegetationType == Type)
            {
                Scale = GetRandomScale(VegData.ScaleRange);
                break;
            }
        }

        FTransform InstanceTransform(SpawnRotation, SpawnLocation, Scale);
        TargetComponent->AddInstance(InstanceTransform);
    }

    UE_LOG(LogTemp, Log, TEXT("Spawned %d vegetation instances of type %d at location %s"), 
           Count, (int32)Type, *Center.ToString());
}

void AEnvArt_VegetationSystem::ClearAllVegetation()
{
    if (FernInstances)
    {
        FernInstances->ClearInstances();
    }
    if (TreeInstances)
    {
        TreeInstances->ClearInstances();
    }
    if (CycadInstances)
    {
        CycadInstances->ClearInstances();
    }

    UE_LOG(LogTemp, Log, TEXT("Cleared all vegetation instances"));
}

void AEnvArt_VegetationSystem::UpdateWindParameters(float NewStrength, const FVector& NewDirection)
{
    WindStrength = NewStrength;
    WindDirection = NewDirection.GetSafeNormal();
    
    UE_LOG(LogTemp, Log, TEXT("Updated wind: Strength=%.2f, Direction=%s"), 
           WindStrength, *WindDirection.ToString());
}

int32 AEnvArt_VegetationSystem::GetTotalInstanceCount() const
{
    int32 Total = 0;
    
    if (FernInstances)
    {
        Total += FernInstances->GetInstanceCount();
    }
    if (TreeInstances)
    {
        Total += TreeInstances->GetInstanceCount();
    }
    if (CycadInstances)
    {
        Total += CycadInstances->GetInstanceCount();
    }
    
    return Total;
}

void AEnvArt_VegetationSystem::GenerateRandomVegetation()
{
    ClearAllVegetation();

    FVector ActorLocation = GetActorLocation();
    
    // Spawn different vegetation types
    SpawnVegetationInRadius(ActorLocation, SpawnRadius, EEnvArt_VegetationType::Fern, 200);
    SpawnVegetationInRadius(ActorLocation, SpawnRadius, EEnvArt_VegetationType::ConiferTree, 50);
    SpawnVegetationInRadius(ActorLocation, SpawnRadius, EEnvArt_VegetationType::CycadPlant, 100);

    UE_LOG(LogTemp, Log, TEXT("Generated random vegetation. Total instances: %d"), GetTotalInstanceCount());
}

void AEnvArt_VegetationSystem::UpdateWindEffect(float DeltaTime)
{
    WindTimer += DeltaTime;
    
    // Create sinusoidal wind movement
    float WindX = FMath::Sin(WindTimer * 2.0f) * WindStrength * WindDirection.X;
    float WindY = FMath::Cos(WindTimer * 1.5f) * WindStrength * WindDirection.Y;
    float WindZ = FMath::Sin(WindTimer * 3.0f) * WindStrength * 0.1f; // Minimal vertical movement
    
    CurrentWindOffset = FVector(WindX, WindY, WindZ);
    
    // Apply wind effect to material parameters if available
    // This would typically be done through material parameter collections
    // For now, we log the wind state for debugging
    if (FMath::Fmod(WindTimer, 5.0f) < DeltaTime)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Wind effect: Offset=%s, Strength=%.2f"), 
               *CurrentWindOffset.ToString(), WindStrength);
    }
}

FVector AEnvArt_VegetationSystem::GetRandomSpawnLocation(const FVector& Center, float Radius) const
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(0.0f, Radius);
    
    float X = Center.X + RandomDistance * FMath::Cos(RandomAngle);
    float Y = Center.Y + RandomDistance * FMath::Sin(RandomAngle);
    float Z = Center.Z + FMath::RandRange(-50.0f, 50.0f); // Small height variation
    
    return FVector(X, Y, Z);
}

FRotator AEnvArt_VegetationSystem::GetRandomRotation() const
{
    float RandomYaw = FMath::RandRange(0.0f, 360.0f);
    float RandomPitch = FMath::RandRange(-5.0f, 5.0f); // Slight pitch variation
    float RandomRoll = FMath::RandRange(-5.0f, 5.0f);  // Slight roll variation
    
    return FRotator(RandomPitch, RandomYaw, RandomRoll);
}

FVector AEnvArt_VegetationSystem::GetRandomScale(const FVector& ScaleRange) const
{
    float UniformScale = FMath::RandRange(ScaleRange.X, ScaleRange.Y);
    float HeightVariation = FMath::RandRange(0.9f, 1.1f) * ScaleRange.Z;
    
    return FVector(UniformScale, UniformScale, UniformScale * HeightVariation);
}

UInstancedStaticMeshComponent* AEnvArt_VegetationSystem::GetInstancedMeshForType(EEnvArt_VegetationType Type) const
{
    switch (Type)
    {
        case EEnvArt_VegetationType::Fern:
        case EEnvArt_VegetationType::Moss:
        case EEnvArt_VegetationType::Mushroom:
            return FernInstances;
            
        case EEnvArt_VegetationType::ConiferTree:
        case EEnvArt_VegetationType::DeadLog:
            return TreeInstances;
            
        case EEnvArt_VegetationType::CycadPlant:
            return CycadInstances;
            
        default:
            return FernInstances;
    }
}