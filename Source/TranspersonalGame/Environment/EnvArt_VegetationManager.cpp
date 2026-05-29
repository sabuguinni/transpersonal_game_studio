#include "EnvArt_VegetationManager.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMesh.h"

UEnvArt_VegetationManager::UEnvArt_VegetationManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CurrentBiome = EEnvArt_BiomeType::Savana;
    SpawnRadius = 5000.0f;
    MaxVegetationCount = 100;
    bAutoSpawnOnBeginPlay = true;
    MinDistanceBetweenVegetation = 200.0f;
}

void UEnvArt_VegetationManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoSpawnOnBeginPlay && GetOwner())
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        SpawnVegetationInRadius(OwnerLocation, SpawnRadius);
    }
}

void UEnvArt_VegetationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UEnvArt_VegetationManager::SpawnVegetationInRadius(const FVector& CenterLocation, float Radius)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("VegetationManager: No valid world found"));
        return;
    }

    int32 SpawnedCount = 0;
    int32 AttemptCount = 0;
    const int32 MaxAttempts = MaxVegetationCount * 3;

    while (SpawnedCount < MaxVegetationCount && AttemptCount < MaxAttempts)
    {
        AttemptCount++;
        
        FVector RandomLocation = GetRandomLocationInRadius(CenterLocation, Radius);
        
        if (IsLocationValid(RandomLocation))
        {
            // Choose random vegetation type based on biome
            EEnvArt_VegetationType VegType = EEnvArt_VegetationType::Fern;
            
            switch (CurrentBiome)
            {
                case EEnvArt_BiomeType::Forest:
                    VegType = static_cast<EEnvArt_VegetationType>(FMath::RandRange(0, 2)); // Fern, Cycad, Conifer
                    break;
                case EEnvArt_BiomeType::Savana:
                    VegType = static_cast<EEnvArt_VegetationType>(FMath::RandRange(4, 5)); // Grass, Vine
                    break;
                case EEnvArt_BiomeType::Swamp:
                    VegType = static_cast<EEnvArt_VegetationType>(FMath::RandRange(0, 1)); // Fern, Cycad
                    break;
                case EEnvArt_BiomeType::Desert:
                    VegType = EEnvArt_VegetationType::DeadTree;
                    break;
                default:
                    VegType = EEnvArt_VegetationType::Fern;
                    break;
            }
            
            SpawnVegetationAtLocation(RandomLocation, VegType);
            SpawnedCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("VegetationManager: Spawned %d vegetation actors in %d attempts"), SpawnedCount, AttemptCount);
}

void UEnvArt_VegetationManager::SpawnVegetationAtLocation(const FVector& Location, EEnvArt_VegetationType Type)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UStaticMesh* VegMesh = GetMeshForVegetationType(Type);
    if (!VegMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("VegetationManager: No mesh found for vegetation type %d"), (int32)Type);
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* VegActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    
    if (VegActor)
    {
        UStaticMeshComponent* MeshComp = VegActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            MeshComp->SetStaticMesh(VegMesh);
            
            // Random rotation for natural look
            FRotator RandomRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
            VegActor->SetActorRotation(RandomRotation);
            
            // Random scale variation
            float ScaleVariation = FMath::RandRange(0.8f, 1.2f);
            VegActor->SetActorScale3D(FVector(ScaleVariation));
            
            // Set label for identification
            FString VegTypeName = UEnum::GetValueAsString(Type);
            VegActor->SetActorLabel(FString::Printf(TEXT("Vegetation_%s_%d"), *VegTypeName, SpawnedVegetationActors.Num()));
        }
        
        SpawnedVegetationActors.Add(VegActor);
    }
}

void UEnvArt_VegetationManager::ClearAllVegetation()
{
    for (AActor* VegActor : SpawnedVegetationActors)
    {
        if (IsValid(VegActor))
        {
            VegActor->Destroy();
        }
    }
    
    SpawnedVegetationActors.Empty();
    UE_LOG(LogTemp, Log, TEXT("VegetationManager: Cleared all vegetation"));
}

void UEnvArt_VegetationManager::SetBiomeType(EEnvArt_BiomeType NewBiome)
{
    CurrentBiome = NewBiome;
    UE_LOG(LogTemp, Log, TEXT("VegetationManager: Biome changed to %d"), (int32)NewBiome);
}

TArray<AActor*> UEnvArt_VegetationManager::GetSpawnedVegetation() const
{
    return SpawnedVegetationActors;
}

FVector UEnvArt_VegetationManager::GetRandomLocationInRadius(const FVector& Center, float Radius) const
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(0.0f, Radius);
    
    FVector RandomOffset;
    RandomOffset.X = FMath::Cos(RandomAngle) * RandomDistance;
    RandomOffset.Y = FMath::Sin(RandomAngle) * RandomDistance;
    RandomOffset.Z = 0.0f;
    
    return Center + RandomOffset;
}

bool UEnvArt_VegetationManager::IsLocationValid(const FVector& Location) const
{
    // Check minimum distance from existing vegetation
    for (const AActor* ExistingVeg : SpawnedVegetationActors)
    {
        if (IsValid(ExistingVeg))
        {
            float Distance = FVector::Dist(Location, ExistingVeg->GetActorLocation());
            if (Distance < MinDistanceBetweenVegetation)
            {
                return false;
            }
        }
    }
    
    // Additional validation could include terrain height, slope, etc.
    return true;
}

UStaticMesh* UEnvArt_VegetationManager::GetMeshForVegetationType(EEnvArt_VegetationType Type) const
{
    // This would normally load actual mesh assets from content browser
    // For now, return nullptr - actual meshes would be set via Blueprint or data assets
    
    switch (Type)
    {
        case EEnvArt_VegetationType::Fern:
            // Load fern mesh from /Game/Tropical_Jungle_Pack/
            break;
        case EEnvArt_VegetationType::Cycad:
            // Load cycad mesh
            break;
        case EEnvArt_VegetationType::Conifer:
            // Load conifer mesh
            break;
        case EEnvArt_VegetationType::Grass:
            // Load grass mesh
            break;
        default:
            break;
    }
    
    // Return nullptr for now - meshes should be assigned via Blueprint
    return nullptr;
}