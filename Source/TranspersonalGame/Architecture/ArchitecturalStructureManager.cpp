#include "ArchitecturalStructureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AArch_StructureActor::AArch_StructureActor()
{
    PrimaryActorTick.bCanEverTick = true;

    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    RootComponent = StructureMesh;

    StructureType = EArch_StructureType::Dwelling;
    StructureData.StructureHealth = 100.0f;
    StructureData.bIsHabitable = true;
}

void AArch_StructureActor::BeginPlay()
{
    Super::BeginPlay();
    
    if (UArchitecturalStructureManager* ArchManager = GetWorld()->GetSubsystem<UArchitecturalStructureManager>())
    {
        ArchManager->RegisterStructure(this);
    }
}

void AArch_StructureActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Weather damage simulation
    if (StructureData.StructureHealth > 0.0f)
    {
        float WeatherDamage = DeltaTime * 0.001f; // Very slow degradation
        StructureData.StructureHealth = FMath::Max(0.0f, StructureData.StructureHealth - WeatherDamage);
    }
}

void AArch_StructureActor::InitializeStructure(const FArch_StructureData& InStructureData, EArch_StructureType InType)
{
    StructureData = InStructureData;
    StructureType = InType;
    
    SetActorLocation(StructureData.Location);
    SetActorRotation(StructureData.Rotation);
    
    if (StructureMesh)
    {
        // Set material based on structure type
        switch (StructureType)
        {
        case EArch_StructureType::Dwelling:
            StructureData.bIsHabitable = true;
            break;
        case EArch_StructureType::Ruins:
            StructureData.StructureHealth *= 0.3f; // Ruins are damaged
            StructureData.bIsHabitable = false;
            break;
        default:
            break;
        }
    }
}

bool AArch_StructureActor::CanPlayerEnter() const
{
    return StructureData.bIsHabitable && StructureData.StructureHealth > 25.0f;
}

void AArch_StructureActor::DamageStructure(float DamageAmount)
{
    StructureData.StructureHealth = FMath::Max(0.0f, StructureData.StructureHealth - DamageAmount);
    
    if (StructureData.StructureHealth <= 0.0f)
    {
        StructureData.bIsHabitable = false;
        // Could trigger collapse effects here
    }
}

void UArchitecturalStructureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    ManagedStructures.Empty();
    BiomeStructures.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalStructureManager initialized"));
}

void UArchitecturalStructureManager::Deinitialize()
{
    ManagedStructures.Empty();
    BiomeStructures.Empty();
    
    Super::Deinitialize();
}

AArch_StructureActor* UArchitecturalStructureManager::SpawnStructure(const FArch_StructureData& StructureData, EArch_StructureType StructureType)
{
    if (!GetWorld())
    {
        return nullptr;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AArch_StructureActor* NewStructure = GetWorld()->SpawnActor<AArch_StructureActor>(
        AArch_StructureActor::StaticClass(),
        StructureData.Location,
        StructureData.Rotation,
        SpawnParams
    );
    
    if (NewStructure)
    {
        NewStructure->InitializeStructure(StructureData, StructureType);
        RegisterStructure(NewStructure);
        
        UE_LOG(LogTemp, Warning, TEXT("Spawned structure: %s at %s"), 
               *StructureData.StructureName, 
               *StructureData.Location.ToString());
    }
    
    return NewStructure;
}

void UArchitecturalStructureManager::PopulateBiomeWithStructures(EBiomeType BiomeType, int32 StructureCount)
{
    if (!GetWorld())
    {
        return;
    }
    
    FVector BiomeCenter;
    switch (BiomeType)
    {
    case EBiomeType::Savana:
        BiomeCenter = FVector(0, 0, 100);
        break;
    case EBiomeType::Pantano:
        BiomeCenter = FVector(-50000, -45000, 100);
        break;
    case EBiomeType::Floresta:
        BiomeCenter = FVector(-45000, 40000, 100);
        break;
    case EBiomeType::Deserto:
        BiomeCenter = FVector(55000, 0, 100);
        break;
    case EBiomeType::Montanha:
        BiomeCenter = FVector(40000, 50000, 100);
        break;
    default:
        BiomeCenter = FVector::ZeroVector;
        break;
    }
    
    for (int32 i = 0; i < StructureCount; ++i)
    {
        FArch_StructureData NewStructureData;
        
        // Random position within biome radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(500.0f, 5000.0f);
        
        NewStructureData.Location = BiomeCenter + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            FMath::RandRange(-50.0f, 200.0f)
        );
        
        NewStructureData.Rotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
        NewStructureData.BiomeType = BiomeType;
        NewStructureData.StructureName = FString::Printf(TEXT("Structure_%s_%d"), 
                                                        *UEnum::GetValueAsString(BiomeType), i);
        
        // Vary structure types based on biome
        EArch_StructureType StructureType = EArch_StructureType::Dwelling;
        if (BiomeType == EBiomeType::Deserto)
        {
            StructureType = (FMath::RandRange(0, 100) < 30) ? EArch_StructureType::Ruins : EArch_StructureType::Storage;
        }
        else if (BiomeType == EBiomeType::Montanha)
        {
            StructureType = EArch_StructureType::Defensive;
        }
        
        SpawnStructure(NewStructureData, StructureType);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Populated %s biome with %d structures"), 
           *UEnum::GetValueAsString(BiomeType), StructureCount);
}

TArray<AArch_StructureActor*> UArchitecturalStructureManager::GetStructuresInRadius(FVector Location, float Radius)
{
    TArray<AArch_StructureActor*> NearbyStructures;
    
    for (AArch_StructureActor* Structure : ManagedStructures)
    {
        if (IsValid(Structure))
        {
            float Distance = FVector::Dist(Structure->GetActorLocation(), Location);
            if (Distance <= Radius)
            {
                NearbyStructures.Add(Structure);
            }
        }
    }
    
    return NearbyStructures;
}

void UArchitecturalStructureManager::RegisterStructure(AArch_StructureActor* Structure)
{
    if (IsValid(Structure) && !ManagedStructures.Contains(Structure))
    {
        ManagedStructures.Add(Structure);
        
        FArch_StructureData StructureData = Structure->GetStructureData();
        if (!BiomeStructures.Contains(StructureData.BiomeType))
        {
            BiomeStructures.Add(StructureData.BiomeType, TArray<FArch_StructureData>());
        }
        BiomeStructures[StructureData.BiomeType].Add(StructureData);
    }
}

void UArchitecturalStructureManager::UnregisterStructure(AArch_StructureActor* Structure)
{
    if (IsValid(Structure))
    {
        ManagedStructures.Remove(Structure);
        
        FArch_StructureData StructureData = Structure->GetStructureData();
        if (BiomeStructures.Contains(StructureData.BiomeType))
        {
            BiomeStructures[StructureData.BiomeType].RemoveAll([&](const FArch_StructureData& Data)
            {
                return Data.StructureName == StructureData.StructureName;
            });
        }
    }
}

void UArchitecturalStructureManager::GenerateArchitecturalLayout()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating architectural layout for all biomes"));
    
    // Clear existing structures
    for (AArch_StructureActor* Structure : ManagedStructures)
    {
        if (IsValid(Structure))
        {
            Structure->Destroy();
        }
    }
    ManagedStructures.Empty();
    BiomeStructures.Empty();
    
    // Populate each biome with structures
    PopulateBiomeWithStructures(EBiomeType::Savana, 15);
    PopulateBiomeWithStructures(EBiomeType::Pantano, 8);
    PopulateBiomeWithStructures(EBiomeType::Floresta, 12);
    PopulateBiomeWithStructures(EBiomeType::Deserto, 6);
    PopulateBiomeWithStructures(EBiomeType::Montanha, 10);
    
    UE_LOG(LogTemp, Warning, TEXT("Architectural layout generation complete. Total structures: %d"), 
           ManagedStructures.Num());
}