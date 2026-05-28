#include "ArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"

UArchitectureManager::UArchitectureManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f;
    
    StructureSpawnRadius = 5000.0f;
    MaxStructuresPerBiome = 8;
    bAutoGenerateStructures = false;
}

void UArchitectureManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Initialized for prehistoric structure management"));
    
    if (bAutoGenerateStructures)
    {
        GenerateTestStructures();
    }
}

void UArchitectureManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update structural integrity over time
    for (FArch_StructureData& Structure : RegisteredStructures)
    {
        if (Structure.StructuralIntegrity > 0.0f)
        {
            // Natural decay - structures deteriorate slowly over time
            Structure.StructuralIntegrity -= DeltaTime * 0.1f;
            Structure.StructuralIntegrity = FMath::Max(0.0f, Structure.StructuralIntegrity);
        }
    }
}

void UArchitectureManager::RegisterStructure(const FArch_StructureData& StructureData)
{
    if (RegisteredStructures.Num() >= MaxStructuresPerBiome * 5) // 5 biomes
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Maximum structure limit reached"));
        return;
    }
    
    RegisteredStructures.Add(StructureData);
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Registered structure at location %s"), 
           *StructureData.Location.ToString());
}

void UArchitectureManager::RemoveStructure(const FVector& Location, float Radius)
{
    for (int32 i = RegisteredStructures.Num() - 1; i >= 0; i--)
    {
        if (FVector::Dist(RegisteredStructures[i].Location, Location) <= Radius)
        {
            RegisteredStructures.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Removed structure near %s"), *Location.ToString());
        }
    }
}

TArray<FArch_StructureData> UArchitectureManager::GetStructuresInRadius(const FVector& Center, float Radius) const
{
    TArray<FArch_StructureData> NearbyStructures;
    
    for (const FArch_StructureData& Structure : RegisteredStructures)
    {
        if (FVector::Dist(Structure.Location, Center) <= Radius)
        {
            NearbyStructures.Add(Structure);
        }
    }
    
    return NearbyStructures;
}

void UArchitectureManager::GenerateCaveDwelling(const FVector& Location, const FRotator& Rotation)
{
    if (!ValidateStructurePlacement(Location, EArch_StructureType::CaveDwelling))
    {
        return;
    }
    
    FArch_StructureData CaveData;
    CaveData.StructureType = EArch_StructureType::CaveDwelling;
    CaveData.Location = Location;
    CaveData.Rotation = Rotation;
    CaveData.StructuralIntegrity = 95.0f;
    CaveData.Materials = {EArch_ConstructionMaterial::Stone};
    CaveData.bIsHabitable = true;
    CaveData.MaxOccupants = 3;
    CaveData.DefenseRating = 8.0f;
    
    RegisterStructure(CaveData);
    
    // Spawn visual representation
    UWorld* World = GetWorld();
    if (World)
    {
        AStaticMeshActor* CaveActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
        if (CaveActor)
        {
            CaveActor->SetActorLabel(TEXT("CaveDwelling"));
            UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Generated cave dwelling at %s"), *Location.ToString());
        }
    }
}

void UArchitectureManager::GenerateWoodenShelter(const FVector& Location, const FRotator& Rotation)
{
    if (!ValidateStructurePlacement(Location, EArch_StructureType::WoodenShelter))
    {
        return;
    }
    
    FArch_StructureData ShelterData;
    ShelterData.StructureType = EArch_StructureType::WoodenShelter;
    ShelterData.Location = Location;
    ShelterData.Rotation = Rotation;
    ShelterData.StructuralIntegrity = 70.0f;
    ShelterData.Materials = {EArch_ConstructionMaterial::Wood, EArch_ConstructionMaterial::Vine};
    ShelterData.bIsHabitable = true;
    ShelterData.MaxOccupants = 2;
    ShelterData.DefenseRating = 4.0f;
    
    RegisterStructure(ShelterData);
    
    // Spawn visual representation
    UWorld* World = GetWorld();
    if (World)
    {
        AStaticMeshActor* ShelterActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
        if (ShelterActor)
        {
            ShelterActor->SetActorLabel(TEXT("WoodenShelter"));
            UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Generated wooden shelter at %s"), *Location.ToString());
        }
    }
}

void UArchitectureManager::GenerateStoneSite(const FVector& Location, const FRotator& Rotation)
{
    if (!ValidateStructurePlacement(Location, EArch_StructureType::StoneSite))
    {
        return;
    }
    
    FArch_StructureData SiteData;
    SiteData.StructureType = EArch_StructureType::StoneSite;
    SiteData.Location = Location;
    SiteData.Rotation = Rotation;
    SiteData.StructuralIntegrity = 100.0f;
    SiteData.Materials = {EArch_ConstructionMaterial::Stone};
    SiteData.bIsHabitable = false;
    SiteData.MaxOccupants = 0;
    SiteData.DefenseRating = 2.0f;
    
    RegisterStructure(SiteData);
    
    // Spawn visual representation
    UWorld* World = GetWorld();
    if (World)
    {
        AStaticMeshActor* SiteActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
        if (SiteActor)
        {
            SiteActor->SetActorLabel(TEXT("StoneSite"));
            UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Generated stone site at %s"), *Location.ToString());
        }
    }
}

void UArchitectureManager::GenerateCliffDwelling(const FVector& Location, const FRotator& Rotation)
{
    if (!ValidateStructurePlacement(Location, EArch_StructureType::CliffDwelling))
    {
        return;
    }
    
    FArch_StructureData CliffData;
    CliffData.StructureType = EArch_StructureType::CliffDwelling;
    CliffData.Location = Location;
    CliffData.Rotation = Rotation;
    CliffData.StructuralIntegrity = 85.0f;
    CliffData.Materials = {EArch_ConstructionMaterial::Stone, EArch_ConstructionMaterial::Wood};
    CliffData.bIsHabitable = true;
    CliffData.MaxOccupants = 4;
    CliffData.DefenseRating = 9.0f;
    
    RegisterStructure(CliffData);
    
    // Spawn visual representation
    UWorld* World = GetWorld();
    if (World)
    {
        AStaticMeshActor* CliffActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
        if (CliffActor)
        {
            CliffActor->SetActorLabel(TEXT("CliffDwelling"));
            UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Generated cliff dwelling at %s"), *Location.ToString());
        }
    }
}

bool UArchitectureManager::ValidateStructurePlacement(const FVector& Location, EArch_StructureType StructureType) const
{
    // Check minimum distance from existing structures
    float MinDistance = 500.0f;
    for (const FArch_StructureData& ExistingStructure : RegisteredStructures)
    {
        if (FVector::Dist(ExistingStructure.Location, Location) < MinDistance)
        {
            UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Structure too close to existing structure"));
            return false;
        }
    }
    
    // Validate terrain suitability based on structure type
    switch (StructureType)
    {
        case EArch_StructureType::CaveDwelling:
            // Caves need elevated terrain or cliff faces
            if (Location.Z < 200.0f)
            {
                UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Cave dwelling needs elevated terrain"));
                return false;
            }
            break;
            
        case EArch_StructureType::CliffDwelling:
            // Cliff dwellings need very high elevation
            if (Location.Z < 400.0f)
            {
                UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Cliff dwelling needs high elevation"));
                return false;
            }
            break;
            
        case EArch_StructureType::WoodenShelter:
        case EArch_StructureType::StoneSite:
            // These can be placed on most terrain
            break;
    }
    
    return true;
}

void UArchitectureManager::UpdateStructuralIntegrity(const FVector& Location, float IntegrityChange)
{
    for (FArch_StructureData& Structure : RegisteredStructures)
    {
        if (FVector::Dist(Structure.Location, Location) <= 100.0f)
        {
            Structure.StructuralIntegrity += IntegrityChange;
            Structure.StructuralIntegrity = FMath::Clamp(Structure.StructuralIntegrity, 0.0f, 100.0f);
            
            UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Updated structure integrity to %f"), 
                   Structure.StructuralIntegrity);
            break;
        }
    }
}

void UArchitectureManager::GenerateTestStructures()
{
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Generating test prehistoric structures"));
    
    // Generate cave dwelling in mountainous area
    GenerateCaveDwelling(FVector(2000.0f, 1000.0f, 500.0f));
    
    // Generate wooden shelter in forest area
    GenerateWoodenShelter(FVector(-1500.0f, 2000.0f, 150.0f));
    
    // Generate stone site in open area
    GenerateStoneSite(FVector(0.0f, -2000.0f, 100.0f));
    
    // Generate cliff dwelling on high terrain
    GenerateCliffDwelling(FVector(3000.0f, -1000.0f, 800.0f));
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Generated %d test structures"), RegisteredStructures.Num());
}

void UArchitectureManager::ClearAllStructures()
{
    RegisteredStructures.Empty();
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Cleared all registered structures"));
}