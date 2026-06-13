#include "Arch_CretaceousStructureManager.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

AArch_CretaceousStructureManager::AArch_CretaceousStructureManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Create main structure mesh component
    MainStructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainStructureMesh"));
    MainStructureMesh->SetupAttachment(RootComponent);
    MainStructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MainStructureMesh->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Create interior mesh component
    InteriorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InteriorMesh"));
    InteriorMesh->SetupAttachment(RootComponent);
    InteriorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Initialize default configuration
    StructureConfig.StructureType = EArch_StructureType::CircularStoneDwelling;
    StructureConfig.WeatheringLevel = EArch_WeatheringLevel::Aged;
    StructureConfig.Scale = FVector(1.0f, 1.0f, 1.0f);
    StructureConfig.WeatherProtection = 0.7f;
    StructureConfig.StructuralIntegrity = 0.8f;
    StructureConfig.bHasInterior = true;
    
    // Initialize private variables
    LastWeatheringUpdate = 0.0f;
    bInteriorGenerated = false;
}

void AArch_CretaceousStructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Configure structure on begin play
    ConfigureStructureMesh();
    ApplyWeatheringMaterial();
    
    if (StructureConfig.bHasInterior)
    {
        SetupInteriorSpace();
    }
    
    UE_LOG(LogTemp, Log, TEXT("CretaceousStructureManager initialized: Type=%d, Weathering=%d"), 
           (int32)StructureConfig.StructureType, (int32)StructureConfig.WeatheringLevel);
}

void AArch_CretaceousStructureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update weathering over time (very slow process)
    LastWeatheringUpdate += DeltaTime;
    if (LastWeatheringUpdate > 300.0f) // Update every 5 minutes
    {
        // Gradually increase weathering level over very long periods
        if (FMath::RandRange(0.0f, 1.0f) < 0.001f) // 0.1% chance per update
        {
            int32 CurrentLevel = (int32)StructureConfig.WeatheringLevel;
            if (CurrentLevel < 4) // Don't exceed Ruined level
            {
                StructureConfig.WeatheringLevel = (EArch_WeatheringLevel)(CurrentLevel + 1);
                ApplyWeatheringMaterial();
                UE_LOG(LogTemp, Log, TEXT("Structure weathering increased to level %d"), CurrentLevel + 1);
            }
        }
        LastWeatheringUpdate = 0.0f;
    }
}

void AArch_CretaceousStructureManager::SetStructureType(EArch_StructureType NewType)
{
    if (StructureConfig.StructureType != NewType)
    {
        StructureConfig.StructureType = NewType;
        ConfigureStructureMesh();
        
        if (StructureConfig.bHasInterior)
        {
            SetupInteriorSpace();
        }
        
        UE_LOG(LogTemp, Log, TEXT("Structure type changed to: %d"), (int32)NewType);
    }
}

void AArch_CretaceousStructureManager::SetWeatheringLevel(EArch_WeatheringLevel NewLevel)
{
    if (StructureConfig.WeatheringLevel != NewLevel)
    {
        StructureConfig.WeatheringLevel = NewLevel;
        ApplyWeatheringMaterial();
        
        // Recalculate structural properties
        StructureConfig.StructuralIntegrity = CalculateStructuralIntegrity();
        StructureConfig.WeatherProtection = CalculateWeatherProtection();
        
        UE_LOG(LogTemp, Log, TEXT("Weathering level changed to: %d"), (int32)NewLevel);
    }
}

void AArch_CretaceousStructureManager::UpdateStructureAppearance()
{
    ConfigureStructureMesh();
    ApplyWeatheringMaterial();
    
    if (StructureConfig.bHasInterior && !bInteriorGenerated)
    {
        SetupInteriorSpace();
    }
}

void AArch_CretaceousStructureManager::SpawnInteriorProps()
{
    if (!StructureConfig.bHasInterior || InteriorPropMeshes.Num() == 0)
    {
        return;
    }
    
    TArray<FVector> SpawnPoints = GetInteriorSpawnPoints();
    
    for (int32 i = 0; i < FMath::Min(SpawnPoints.Num(), InteriorPropMeshes.Num()); i++)
    {
        if (InteriorPropMeshes[i])
        {
            FVector SpawnLocation = GetActorLocation() + SpawnPoints[i];
            FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
            
            // Create a new static mesh component for each prop
            FString ComponentName = FString::Printf(TEXT("InteriorProp_%d"), i);
            UStaticMeshComponent* PropComponent = CreateDefaultSubobject<UStaticMeshComponent>(*ComponentName);
            
            if (PropComponent)
            {
                PropComponent->SetupAttachment(RootComponent);
                PropComponent->SetStaticMesh(InteriorPropMeshes[i]);
                PropComponent->SetWorldLocation(SpawnLocation);
                PropComponent->SetWorldRotation(SpawnRotation);
                PropComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Spawned %d interior props"), SpawnPoints.Num());
}

float AArch_CretaceousStructureManager::GetWeatherProtection() const
{
    return StructureConfig.WeatherProtection;
}

float AArch_CretaceousStructureManager::GetStructuralIntegrity() const
{
    return StructureConfig.StructuralIntegrity;
}

bool AArch_CretaceousStructureManager::CanProvideWeatherProtection(const FVector& TestLocation) const
{
    // Simple distance-based check for weather protection
    float Distance = FVector::Dist(GetActorLocation(), TestLocation);
    float ProtectionRadius = 500.0f * StructureConfig.Scale.X; // Base radius scaled by structure size
    
    return Distance <= ProtectionRadius && StructureConfig.WeatherProtection > 0.3f;
}

TArray<FVector> AArch_CretaceousStructureManager::GetInteriorSpawnPoints() const
{
    if (CachedInteriorPoints.Num() > 0)
    {
        return CachedInteriorPoints;
    }
    
    TArray<FVector> Points;
    
    // Generate interior spawn points based on structure type
    switch (StructureConfig.StructureType)
    {
        case EArch_StructureType::CircularStoneDwelling:
        {
            // Circular arrangement of spawn points
            int32 NumPoints = 6;
            float Radius = 200.0f * StructureConfig.Scale.X;
            for (int32 i = 0; i < NumPoints; i++)
            {
                float Angle = (2.0f * PI * i) / NumPoints;
                FVector Point = FVector(
                    FMath::Cos(Angle) * Radius,
                    FMath::Sin(Angle) * Radius,
                    0.0f
                );
                Points.Add(Point);
            }
            break;
        }
        case EArch_StructureType::RectangularShelter:
        {
            // Rectangular grid of spawn points
            FVector BaseOffset = FVector(-150.0f, -100.0f, 0.0f) * StructureConfig.Scale;
            for (int32 x = 0; x < 3; x++)
            {
                for (int32 y = 0; y < 2; y++)
                {
                    FVector Point = BaseOffset + FVector(x * 150.0f, y * 100.0f, 0.0f) * StructureConfig.Scale;
                    Points.Add(Point);
                }
            }
            break;
        }
        case EArch_StructureType::CaveEntrance:
        {
            // Linear arrangement along cave depth
            FVector BaseOffset = FVector(0.0f, 0.0f, 0.0f);
            for (int32 i = 0; i < 4; i++)
            {
                FVector Point = BaseOffset + FVector(-i * 100.0f, 0.0f, 0.0f) * StructureConfig.Scale;
                Points.Add(Point);
            }
            break;
        }
        default:
        {
            // Default single central point
            Points.Add(FVector::ZeroVector);
            break;
        }
    }
    
    return Points;
}

void AArch_CretaceousStructureManager::RegenerateStructure()
{
    bInteriorGenerated = false;
    CachedInteriorPoints.Empty();
    UpdateStructureAppearance();
    
    UE_LOG(LogTemp, Log, TEXT("Structure regenerated"));
}

void AArch_CretaceousStructureManager::ApplyWeatheringMaterial()
{
    if (WeatheringMaterials.Num() == 0 || !MainStructureMesh)
    {
        return;
    }
    
    int32 MaterialIndex = FMath::Clamp((int32)StructureConfig.WeatheringLevel, 0, WeatheringMaterials.Num() - 1);
    
    if (WeatheringMaterials[MaterialIndex])
    {
        MainStructureMesh->SetMaterial(0, WeatheringMaterials[MaterialIndex]);
        UE_LOG(LogTemp, Log, TEXT("Applied weathering material index: %d"), MaterialIndex);
    }
}

void AArch_CretaceousStructureManager::ConfigureStructureMesh()
{
    if (!MainStructureMesh)
    {
        return;
    }
    
    // Set mesh based on structure type
    if (StructureMeshes.Contains(StructureConfig.StructureType))
    {
        UStaticMesh* SelectedMesh = StructureMeshes[StructureConfig.StructureType];
        if (SelectedMesh)
        {
            MainStructureMesh->SetStaticMesh(SelectedMesh);
        }
    }
    
    // Apply scale
    MainStructureMesh->SetWorldScale3D(StructureConfig.Scale);
    
    UE_LOG(LogTemp, Log, TEXT("Configured structure mesh for type: %d"), (int32)StructureConfig.StructureType);
}

void AArch_CretaceousStructureManager::SetupInteriorSpace()
{
    if (!StructureConfig.bHasInterior)
    {
        return;
    }
    
    // Cache interior spawn points
    CachedInteriorPoints = GetInteriorSpawnPoints();
    
    // Configure interior mesh if available
    if (InteriorMesh && InteriorPropMeshes.Num() > 0)
    {
        InteriorMesh->SetStaticMesh(InteriorPropMeshes[0]); // Use first mesh as base interior
        InteriorMesh->SetWorldScale3D(StructureConfig.Scale * 0.8f); // Slightly smaller than main structure
    }
    
    bInteriorGenerated = true;
    UE_LOG(LogTemp, Log, TEXT("Interior space setup complete with %d spawn points"), CachedInteriorPoints.Num());
}

float AArch_CretaceousStructureManager::CalculateWeatherProtection() const
{
    float BaseProtection = 0.5f;
    
    // Structure type affects protection
    switch (StructureConfig.StructureType)
    {
        case EArch_StructureType::CircularStoneDwelling:
            BaseProtection = 0.8f;
            break;
        case EArch_StructureType::RectangularShelter:
            BaseProtection = 0.7f;
            break;
        case EArch_StructureType::CaveEntrance:
            BaseProtection = 0.9f;
            break;
        case EArch_StructureType::StonePlatform:
            BaseProtection = 0.3f;
            break;
        default:
            BaseProtection = 0.5f;
            break;
    }
    
    // Weathering reduces protection
    float WeatheringPenalty = (int32)StructureConfig.WeatheringLevel * 0.1f;
    
    return FMath::Clamp(BaseProtection - WeatheringPenalty, 0.1f, 1.0f);
}

float AArch_CretaceousStructureManager::CalculateStructuralIntegrity() const
{
    float BaseIntegrity = 0.9f;
    
    // Weathering significantly affects integrity
    float WeatheringPenalty = (int32)StructureConfig.WeatheringLevel * 0.15f;
    
    // Structure type affects base integrity
    switch (StructureConfig.StructureType)
    {
        case EArch_StructureType::StonePlatform:
        case EArch_StructureType::WeatheredPillar:
            BaseIntegrity = 0.7f; // Less stable structures
            break;
        case EArch_StructureType::CaveEntrance:
            BaseIntegrity = 0.95f; // Most stable
            break;
        default:
            BaseIntegrity = 0.8f;
            break;
    }
    
    return FMath::Clamp(BaseIntegrity - WeatheringPenalty, 0.1f, 1.0f);
}