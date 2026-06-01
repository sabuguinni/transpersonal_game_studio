#include "Arch_StructureManager.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AArch_StructureManager::AArch_StructureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create main structure mesh component
    MainStructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainStructureMesh"));
    MainStructureMesh->SetupAttachment(RootComponent);
    MainStructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MainStructureMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

    // Initialize structure data
    StructureData.StructureType = EArch_StructureType::Ruins;
    StructureData.StructureName = "Ancient Structure";
    StructureData.Durability = 75.0f;
    StructureData.WeatheringLevel = 25.0f;
    StructureData.bIsRuined = false;
    StructureData.BiomeType = EBiomeType::Savanna;

    // Initialize settings
    WeatheringRate = 0.1f;
    bAutoWeathering = true;
    MaxViewDistance = 10000.0f;
    WeatheringUpdateInterval = 5.0f;
    LastWeatheringUpdate = 0.0f;
    bIsInitialized = false;
}

void AArch_StructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (!bIsInitialized)
    {
        UpdateStructureAppearance();
        bIsInitialized = true;
    }
}

void AArch_StructureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAutoWeathering)
    {
        ProcessWeathering(DeltaTime);
    }

    UpdateLOD();
}

void AArch_StructureManager::InitializeStructure(EArch_StructureType Type, const FString& Name, EBiomeType Biome)
{
    StructureData.StructureType = Type;
    StructureData.StructureName = Name;
    StructureData.BiomeType = Biome;

    // Set default properties based on structure type
    switch (Type)
    {
        case EArch_StructureType::Temple:
            StructureData.Durability = 90.0f;
            StructureData.WeatheringLevel = 10.0f;
            StructureData.bIsRuined = false;
            break;
        case EArch_StructureType::Ruins:
            StructureData.Durability = 30.0f;
            StructureData.WeatheringLevel = 70.0f;
            StructureData.bIsRuined = true;
            break;
        case EArch_StructureType::Pillar:
            StructureData.Durability = 80.0f;
            StructureData.WeatheringLevel = 20.0f;
            StructureData.bIsRuined = false;
            break;
        case EArch_StructureType::Archway:
            StructureData.Durability = 70.0f;
            StructureData.WeatheringLevel = 30.0f;
            StructureData.bIsRuined = false;
            break;
        default:
            StructureData.Durability = 50.0f;
            StructureData.WeatheringLevel = 50.0f;
            StructureData.bIsRuined = false;
            break;
    }

    UpdateStructureAppearance();
    bIsInitialized = true;

    UE_LOG(LogTemp, Log, TEXT("Architecture: Initialized %s structure '%s' in %s biome"), 
           *UEnum::GetValueAsString(Type), *Name, *UEnum::GetValueAsString(Biome));
}

void AArch_StructureManager::SetStructureMesh(UStaticMesh* NewMesh)
{
    if (NewMesh && MainStructureMesh)
    {
        MainStructureMesh->SetStaticMesh(NewMesh);
        UpdateStructureAppearance();
        UE_LOG(LogTemp, Log, TEXT("Architecture: Set new mesh for structure %s"), *StructureData.StructureName);
    }
}

void AArch_StructureManager::ApplyWeathering(float WeatheringAmount)
{
    StructureData.WeatheringLevel = FMath::Clamp(StructureData.WeatheringLevel + WeatheringAmount, 0.0f, 100.0f);
    StructureData.Durability = FMath::Clamp(StructureData.Durability - WeatheringAmount * 0.5f, 0.0f, 100.0f);

    if (StructureData.Durability <= 20.0f && !StructureData.bIsRuined)
    {
        SetRuinedState(true);
    }

    UpdateStructureAppearance();
}

void AArch_StructureManager::SetRuinedState(bool bRuined)
{
    StructureData.bIsRuined = bRuined;
    
    if (bRuined)
    {
        StructureData.WeatheringLevel = FMath::Max(StructureData.WeatheringLevel, 60.0f);
        StructureData.Durability = FMath::Min(StructureData.Durability, 30.0f);
    }

    UpdateStructureAppearance();
    UE_LOG(LogTemp, Log, TEXT("Architecture: Structure %s ruined state set to %s"), 
           *StructureData.StructureName, bRuined ? TEXT("true") : TEXT("false"));
}

void AArch_StructureManager::RepairStructure(float RepairAmount)
{
    if (!StructureData.bIsRuined)
    {
        StructureData.Durability = FMath::Clamp(StructureData.Durability + RepairAmount, 0.0f, 100.0f);
        StructureData.WeatheringLevel = FMath::Clamp(StructureData.WeatheringLevel - RepairAmount * 0.3f, 0.0f, 100.0f);
        UpdateStructureAppearance();
        UE_LOG(LogTemp, Log, TEXT("Architecture: Repaired structure %s by %.1f points"), 
               *StructureData.StructureName, RepairAmount);
    }
}

void AArch_StructureManager::RandomizeWeathering()
{
    StructureData.WeatheringLevel = FMath::RandRange(0.0f, 100.0f);
    StructureData.Durability = FMath::RandRange(20.0f, 100.0f);
    StructureData.bIsRuined = StructureData.Durability < 30.0f;
    UpdateStructureAppearance();
    UE_LOG(LogTemp, Log, TEXT("Architecture: Randomized weathering for structure %s"), *StructureData.StructureName);
}

void AArch_StructureManager::ResetStructure()
{
    StructureData.Durability = 100.0f;
    StructureData.WeatheringLevel = 0.0f;
    StructureData.bIsRuined = false;
    UpdateStructureAppearance();
    UE_LOG(LogTemp, Log, TEXT("Architecture: Reset structure %s to pristine condition"), *StructureData.StructureName);
}

void AArch_StructureManager::UpdateStructureAppearance()
{
    if (!MainStructureMesh)
    {
        return;
    }

    // Apply weathering effects based on weathering level
    float WeatheringAlpha = StructureData.WeatheringLevel / 100.0f;
    
    // Scale structure slightly based on durability (ruins appear smaller/collapsed)
    if (StructureData.bIsRuined)
    {
        float ScaleFactor = FMath::Lerp(0.7f, 1.0f, StructureData.Durability / 100.0f);
        SetActorScale3D(FVector(ScaleFactor, ScaleFactor, ScaleFactor * 0.8f));
    }
    else
    {
        SetActorScale3D(FVector(1.0f, 1.0f, 1.0f));
    }

    // Apply rotation for ruined structures (slight tilt)
    if (StructureData.bIsRuined && StructureData.WeatheringLevel > 50.0f)
    {
        float TiltAngle = FMath::Lerp(0.0f, 15.0f, WeatheringAlpha);
        FRotator CurrentRotation = GetActorRotation();
        CurrentRotation.Roll = FMath::RandRange(-TiltAngle, TiltAngle);
        SetActorRotation(CurrentRotation);
    }
}

void AArch_StructureManager::ProcessWeathering(float DeltaTime)
{
    LastWeatheringUpdate += DeltaTime;
    
    if (LastWeatheringUpdate >= WeatheringUpdateInterval)
    {
        LastWeatheringUpdate = 0.0f;
        
        // Apply gradual weathering based on biome
        float BiomeWeatheringMultiplier = 1.0f;
        switch (StructureData.BiomeType)
        {
            case EBiomeType::Swamp:
                BiomeWeatheringMultiplier = 2.0f; // High humidity accelerates decay
                break;
            case EBiomeType::Desert:
                BiomeWeatheringMultiplier = 1.5f; // Sand erosion
                break;
            case EBiomeType::Mountain:
                BiomeWeatheringMultiplier = 0.7f; // Cold preserves structures
                break;
            case EBiomeType::Forest:
                BiomeWeatheringMultiplier = 1.3f; // Vegetation overgrowth
                break;
            default:
                BiomeWeatheringMultiplier = 1.0f;
                break;
        }

        float WeatheringIncrement = WeatheringRate * BiomeWeatheringMultiplier * WeatheringUpdateInterval;
        ApplyWeathering(WeatheringIncrement);
    }
}

void AArch_StructureManager::UpdateLOD()
{
    if (!MainStructureMesh)
    {
        return;
    }

    // Simple distance-based LOD
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
        
        if (Distance > MaxViewDistance)
        {
            MainStructureMesh->SetVisibility(false);
        }
        else
        {
            MainStructureMesh->SetVisibility(true);
            
            // Adjust detail level based on distance
            if (Distance > MaxViewDistance * 0.5f)
            {
                MainStructureMesh->SetCastShadow(false);
            }
            else
            {
                MainStructureMesh->SetCastShadow(true);
            }
        }
    }
}