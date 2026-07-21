#include "Arch_MountainStructureSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/KismetMathLibrary.h"

UArch_MountainStructureSystem::UArch_MountainStructureSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second for weather effects

    WeatheringRate = 0.01f;
    MossGrowthRate = 0.005f;
    LichenGrowthRate = 0.003f;
}

void UArch_MountainStructureSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Arch_MountainStructureSystem: Initialized for mountain biome architecture"));
}

void UArch_MountainStructureSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Apply gradual weather effects to all structures
    ApplyWeatherEffects(DeltaTime);
}

void UArch_MountainStructureSystem::CreateMountainStructure(const FArch_MountainStructureData& StructureData)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Arch_MountainStructureSystem: No valid world context"));
        return;
    }

    // Add structure data to array
    int32 NewIndex = MountainStructures.Add(StructureData);

    // Create the physical actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AStaticMeshActor* NewStructureActor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        StructureData.Location,
        StructureData.Rotation,
        SpawnParams
    );

    if (NewStructureActor)
    {
        // Initialize the structure actor
        InitializeStructureActor(NewStructureActor, StructureData);
        
        // Add to our tracking array
        StructureActors.Add(NewStructureActor);

        // Set appropriate label based on structure type
        FString StructureLabel;
        switch (StructureData.StructureType)
        {
            case EArch_MountainStructureType::StoneArch:
                StructureLabel = FString::Printf(TEXT("MountainStoneArch_%d"), NewIndex);
                break;
            case EArch_MountainStructureType::CaveEntrance:
                StructureLabel = FString::Printf(TEXT("MountainCaveEntrance_%d"), NewIndex);
                break;
            case EArch_MountainStructureType::RockFormation:
                StructureLabel = FString::Printf(TEXT("MountainRockFormation_%d"), NewIndex);
                break;
            case EArch_MountainStructureType::CliffDwelling:
                StructureLabel = FString::Printf(TEXT("MountainCliffDwelling_%d"), NewIndex);
                break;
            case EArch_MountainStructureType::StonePlatform:
                StructureLabel = FString::Printf(TEXT("MountainStonePlatform_%d"), NewIndex);
                break;
        }
        
        NewStructureActor->SetActorLabel(StructureLabel);

        UE_LOG(LogTemp, Warning, TEXT("Arch_MountainStructureSystem: Created %s at location %s"), 
               *StructureLabel, *StructureData.Location.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Arch_MountainStructureSystem: Failed to spawn structure actor"));
        MountainStructures.RemoveAt(NewIndex);
    }
}

void UArch_MountainStructureSystem::RemoveMountainStructure(int32 StructureIndex)
{
    if (!MountainStructures.IsValidIndex(StructureIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Arch_MountainStructureSystem: Invalid structure index %d"), StructureIndex);
        return;
    }

    // Destroy the actor if it exists
    if (StructureActors.IsValidIndex(StructureIndex) && StructureActors[StructureIndex])
    {
        StructureActors[StructureIndex]->Destroy();
    }

    // Remove from arrays
    MountainStructures.RemoveAt(StructureIndex);
    StructureActors.RemoveAt(StructureIndex);

    UE_LOG(LogTemp, Warning, TEXT("Arch_MountainStructureSystem: Removed structure at index %d"), StructureIndex);
}

void UArch_MountainStructureSystem::UpdateStructureWeathering(int32 StructureIndex, float NewWeatheringLevel)
{
    if (!MountainStructures.IsValidIndex(StructureIndex))
    {
        return;
    }

    MountainStructures[StructureIndex].WeatheringLevel = FMath::Clamp(NewWeatheringLevel, 0.0f, 1.0f);
    UpdateStructureAppearance(StructureIndex);

    UE_LOG(LogTemp, Log, TEXT("Arch_MountainStructureSystem: Updated weathering level to %f for structure %d"), 
           NewWeatheringLevel, StructureIndex);
}

void UArch_MountainStructureSystem::ApplyMossGrowth(int32 StructureIndex, bool bEnableMoss)
{
    if (!MountainStructures.IsValidIndex(StructureIndex))
    {
        return;
    }

    MountainStructures[StructureIndex].bHasMossGrowth = bEnableMoss;
    UpdateStructureAppearance(StructureIndex);

    UE_LOG(LogTemp, Log, TEXT("Arch_MountainStructureSystem: %s moss growth for structure %d"), 
           bEnableMoss ? TEXT("Enabled") : TEXT("Disabled"), StructureIndex);
}

void UArch_MountainStructureSystem::ApplyLichenCoverage(int32 StructureIndex, bool bEnableLichen)
{
    if (!MountainStructures.IsValidIndex(StructureIndex))
    {
        return;
    }

    MountainStructures[StructureIndex].bHasLichenCoverage = bEnableLichen;
    UpdateStructureAppearance(StructureIndex);

    UE_LOG(LogTemp, Log, TEXT("Arch_MountainStructureSystem: %s lichen coverage for structure %d"), 
           bEnableLichen ? TEXT("Enabled") : TEXT("Disabled"), StructureIndex);
}

int32 UArch_MountainStructureSystem::GetStructureCount() const
{
    return MountainStructures.Num();
}

FArch_MountainStructureData UArch_MountainStructureSystem::GetStructureData(int32 StructureIndex) const
{
    if (MountainStructures.IsValidIndex(StructureIndex))
    {
        return MountainStructures[StructureIndex];
    }
    
    return FArch_MountainStructureData();
}

TArray<int32> UArch_MountainStructureSystem::FindStructuresInRadius(FVector Center, float Radius) const
{
    TArray<int32> FoundStructures;
    
    for (int32 i = 0; i < MountainStructures.Num(); i++)
    {
        float Distance = FVector::Dist(MountainStructures[i].Location, Center);
        if (Distance <= Radius)
        {
            FoundStructures.Add(i);
        }
    }
    
    return FoundStructures;
}

void UArch_MountainStructureSystem::IntegrateWithTerrain(int32 StructureIndex)
{
    if (!MountainStructures.IsValidIndex(StructureIndex) || !StructureActors.IsValidIndex(StructureIndex))
    {
        return;
    }

    AStaticMeshActor* StructureActor = StructureActors[StructureIndex];
    if (!StructureActor)
    {
        return;
    }

    // Adjust structure position to match terrain height
    FVector CurrentLocation = StructureActor->GetActorLocation();
    
    // Simple terrain integration - in a full implementation, this would use landscape queries
    // For now, we'll just ensure the structure is properly grounded
    FVector AdjustedLocation = CurrentLocation;
    AdjustedLocation.Z = CurrentLocation.Z; // Keep current Z for now
    
    StructureActor->SetActorLocation(AdjustedLocation);
    MountainStructures[StructureIndex].Location = AdjustedLocation;

    UE_LOG(LogTemp, Log, TEXT("Arch_MountainStructureSystem: Integrated structure %d with terrain"), StructureIndex);
}

void UArch_MountainStructureSystem::ApplyWeatherEffects(float DeltaTime)
{
    for (int32 i = 0; i < MountainStructures.Num(); i++)
    {
        ApplyEnvironmentalEffects(i, DeltaTime);
    }
}

void UArch_MountainStructureSystem::UpdateStructuralIntegrity(int32 StructureIndex, float IntegrityChange)
{
    if (!MountainStructures.IsValidIndex(StructureIndex))
    {
        return;
    }

    float& Integrity = MountainStructures[StructureIndex].StructuralIntegrity;
    Integrity = FMath::Clamp(Integrity + IntegrityChange, 0.0f, 1.0f);

    // If integrity is very low, increase weathering
    if (Integrity < 0.3f)
    {
        MountainStructures[StructureIndex].WeatheringLevel = FMath::Min(
            MountainStructures[StructureIndex].WeatheringLevel + 0.1f, 1.0f);
        UpdateStructureAppearance(StructureIndex);
    }

    UE_LOG(LogTemp, Log, TEXT("Arch_MountainStructureSystem: Structure %d integrity now %f"), 
           StructureIndex, Integrity);
}

void UArch_MountainStructureSystem::SetStructureMaterial(int32 StructureIndex, UMaterialInterface* NewMaterial)
{
    if (!StructureActors.IsValidIndex(StructureIndex) || !StructureActors[StructureIndex] || !NewMaterial)
    {
        return;
    }

    UStaticMeshComponent* MeshComp = StructureActors[StructureIndex]->GetStaticMeshComponent();
    if (MeshComp)
    {
        MeshComp->SetMaterial(0, NewMaterial);
        UE_LOG(LogTemp, Log, TEXT("Arch_MountainStructureSystem: Applied material to structure %d"), StructureIndex);
    }
}

void UArch_MountainStructureSystem::ApplyWeatheringMaterial(int32 StructureIndex)
{
    if (!MountainStructures.IsValidIndex(StructureIndex))
    {
        return;
    }

    UMaterialInterface* MaterialToApply = GetAppropriateStructureMaterial(MountainStructures[StructureIndex]);
    if (MaterialToApply)
    {
        SetStructureMaterial(StructureIndex, MaterialToApply);
    }
}

void UArch_MountainStructureSystem::InitializeStructureActor(AStaticMeshActor* StructureActor, const FArch_MountainStructureData& StructureData)
{
    if (!StructureActor)
    {
        return;
    }

    // Set transform
    StructureActor->SetActorTransform(FTransform(StructureData.Rotation, StructureData.Location, StructureData.Scale));

    // Apply appropriate material
    UMaterialInterface* Material = GetAppropriateStructureMaterial(StructureData);
    if (Material)
    {
        UStaticMeshComponent* MeshComp = StructureActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            MeshComp->SetMaterial(0, Material);
        }
    }

    // Set collision properties for mountain structures
    UStaticMeshComponent* MeshComp = StructureActor->GetStaticMeshComponent();
    if (MeshComp)
    {
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    }
}

void UArch_MountainStructureSystem::UpdateStructureAppearance(int32 StructureIndex)
{
    if (!MountainStructures.IsValidIndex(StructureIndex) || !StructureActors.IsValidIndex(StructureIndex))
    {
        return;
    }

    UMaterialInterface* NewMaterial = GetAppropriateStructureMaterial(MountainStructures[StructureIndex]);
    if (NewMaterial)
    {
        SetStructureMaterial(StructureIndex, NewMaterial);
    }
}

UMaterialInterface* UArch_MountainStructureSystem::GetAppropriateStructureMaterial(const FArch_MountainStructureData& StructureData) const
{
    // Determine material based on weathering, moss, and lichen
    if (StructureData.bHasMossGrowth && StructureData.WeatheringLevel > 0.6f)
    {
        return MossyCoverMaterial;
    }
    else if (StructureData.bHasLichenCoverage && StructureData.WeatheringLevel > 0.4f)
    {
        return LichenCoverMaterial;
    }
    else if (StructureData.WeatheringLevel > 0.3f)
    {
        return WeatheredStoneMaterial;
    }
    else
    {
        return BaseStoneMaterial;
    }
}

void UArch_MountainStructureSystem::ApplyEnvironmentalEffects(int32 StructureIndex, float DeltaTime)
{
    if (!MountainStructures.IsValidIndex(StructureIndex))
    {
        return;
    }

    FArch_MountainStructureData& StructureData = MountainStructures[StructureIndex];
    bool bNeedsUpdate = false;

    // Gradual weathering over time
    if (StructureData.WeatheringLevel < 1.0f)
    {
        StructureData.WeatheringLevel += WeatheringRate * DeltaTime;
        StructureData.WeatheringLevel = FMath::Min(StructureData.WeatheringLevel, 1.0f);
        bNeedsUpdate = true;
    }

    // Moss growth in humid conditions
    if (StructureData.bHasMossGrowth && StructureData.WeatheringLevel > 0.3f)
    {
        // Moss grows faster on weathered surfaces
        float MossGrowthFactor = StructureData.WeatheringLevel * MossGrowthRate * DeltaTime;
        bNeedsUpdate = true;
    }

    // Lichen growth on exposed surfaces
    if (StructureData.bHasLichenCoverage && StructureData.WeatheringLevel > 0.2f)
    {
        // Lichen grows slower but is more persistent
        float LichenGrowthFactor = StructureData.WeatheringLevel * LichenGrowthRate * DeltaTime;
        bNeedsUpdate = true;
    }

    // Structural degradation over time
    if (StructureData.StructuralIntegrity > 0.0f)
    {
        float DegradationRate = StructureData.WeatheringLevel * 0.001f * DeltaTime;
        StructureData.StructuralIntegrity -= DegradationRate;
        StructureData.StructuralIntegrity = FMath::Max(StructureData.StructuralIntegrity, 0.0f);
    }

    // Update appearance if needed
    if (bNeedsUpdate)
    {
        UpdateStructureAppearance(StructureIndex);
    }
}