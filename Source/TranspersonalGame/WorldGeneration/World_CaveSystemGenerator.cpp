#include "World_CaveSystemGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AWorld_CaveSystemGenerator::AWorld_CaveSystemGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    
    MaxCavesPerBiome = 8;
    MinDistanceBetweenCaves = 1500.0f;
    bGenerateUndergroundNetwork = true;
    UndergroundNetworkDepth = -300.0f;
    
    InitializeCaveConfigurations();
}

void AWorld_CaveSystemGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bGenerateUndergroundNetwork)
    {
        GenerateCaveSystem();
    }
}

void AWorld_CaveSystemGenerator::InitializeCaveConfigurations()
{
    CaveConfigurations.Empty();
    
    // Small exploration caves
    FWorld_CaveData SmallCave;
    SmallCave.CaveType = EWorld_CaveType::SmallCave;
    SmallCave.CaveDepth = 200.0f;
    SmallCave.CaveWidth = 150.0f;
    SmallCave.TunnelCount = 2;
    CaveConfigurations.Add(SmallCave);
    
    // Medium shelter caves
    FWorld_CaveData MediumCave;
    MediumCave.CaveType = EWorld_CaveType::MediumCave;
    MediumCave.CaveDepth = 400.0f;
    MediumCave.CaveWidth = 300.0f;
    MediumCave.TunnelCount = 4;
    CaveConfigurations.Add(MediumCave);
    
    // Large complex caves
    FWorld_CaveData LargeCave;
    LargeCave.CaveType = EWorld_CaveType::LargeCave;
    LargeCave.CaveDepth = 800.0f;
    LargeCave.CaveWidth = 600.0f;
    LargeCave.TunnelCount = 8;
    LargeCave.bHasCrystals = true;
    CaveConfigurations.Add(LargeCave);
    
    // Water caves with underground streams
    FWorld_CaveData WaterCave;
    WaterCave.CaveType = EWorld_CaveType::WaterCave;
    WaterCave.CaveDepth = 300.0f;
    WaterCave.CaveWidth = 250.0f;
    WaterCave.TunnelCount = 3;
    WaterCave.bHasWater = true;
    CaveConfigurations.Add(WaterCave);
}

void AWorld_CaveSystemGenerator::GenerateCaveSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 GeneratedCaves = 0;
    int32 MaxAttempts = 50;
    int32 Attempts = 0;
    
    while (GeneratedCaves < MaxCavesPerBiome && Attempts < MaxAttempts)
    {
        FVector CaveLocation = GetRandomCaveLocation();
        
        if (IsValidCaveLocation(CaveLocation))
        {
            int32 ConfigIndex = FMath::RandRange(0, CaveConfigurations.Num() - 1);
            FWorld_CaveData CaveData = CaveConfigurations[ConfigIndex];
            CaveData.EntranceLocation = CaveLocation;
            
            GenerateCaveEntrance(CaveData);
            GeneratedCaves++;
        }
        
        Attempts++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Cave System Generator: Created %d caves in %d attempts"), GeneratedCaves, Attempts);
}

void AWorld_CaveSystemGenerator::GenerateCaveEntrance(const FWorld_CaveData& CaveData)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create cave entrance marker
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AActor* CaveEntrance = World->SpawnActor<AActor>(AActor::StaticClass(), CaveData.EntranceLocation, FRotator::ZeroRotator, SpawnParams);
    if (CaveEntrance)
    {
        FString CaveName = FString::Printf(TEXT("CaveEntrance_%s"), *UEnum::GetValueAsString(CaveData.CaveType));
        CaveEntrance->SetActorLabel(CaveName);
        
        // Create lighting for the cave
        CreateCaveLighting(CaveData.EntranceLocation, CaveData.CaveType);
        
        // Generate underground tunnel network
        if (CaveData.TunnelCount > 0)
        {
            GenerateUndergroundTunnels(CaveData.EntranceLocation, CaveData.TunnelCount);
        }
        
        // Add special features
        if (CaveData.bHasWater)
        {
            AddCaveWaterFeatures(CaveData.EntranceLocation);
        }
        
        if (CaveData.bHasCrystals)
        {
            AddCaveCrystals(CaveData.EntranceLocation);
        }
    }
}

void AWorld_CaveSystemGenerator::GenerateUndergroundTunnels(const FVector& StartLocation, int32 TunnelCount)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    for (int32 i = 0; i < TunnelCount; i++)
    {
        FVector TunnelLocation = StartLocation;
        TunnelLocation.X += FMath::RandRange(-500.0f, 500.0f);
        TunnelLocation.Y += FMath::RandRange(-500.0f, 500.0f);
        TunnelLocation.Z = UndergroundNetworkDepth + FMath::RandRange(-100.0f, 50.0f);
        
        FActorSpawnParameters SpawnParams;
        AActor* TunnelMarker = World->SpawnActor<AActor>(AActor::StaticClass(), TunnelLocation, FRotator::ZeroRotator, SpawnParams);
        if (TunnelMarker)
        {
            FString TunnelName = FString::Printf(TEXT("UndergroundTunnel_%d"), i + 1);
            TunnelMarker->SetActorLabel(TunnelName);
        }
    }
}

void AWorld_CaveSystemGenerator::CreateCaveLighting(const FVector& Location, EWorld_CaveType CaveType)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    FActorSpawnParameters SpawnParams;
    AActor* LightActor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    if (LightActor)
    {
        UPointLightComponent* PointLight = NewObject<UPointLightComponent>(LightActor);
        LightActor->SetRootComponent(PointLight);
        
        // Configure lighting based on cave type
        switch (CaveType)
        {
            case EWorld_CaveType::SmallCave:
                PointLight->SetIntensity(30.0f);
                PointLight->SetLightColor(FLinearColor(1.0f, 0.8f, 0.6f));
                break;
            case EWorld_CaveType::MediumCave:
                PointLight->SetIntensity(50.0f);
                PointLight->SetLightColor(FLinearColor(0.9f, 0.7f, 0.5f));
                break;
            case EWorld_CaveType::LargeCave:
                PointLight->SetIntensity(80.0f);
                PointLight->SetLightColor(FLinearColor(0.8f, 0.6f, 0.4f));
                break;
            case EWorld_CaveType::CrystalCave:
                PointLight->SetIntensity(60.0f);
                PointLight->SetLightColor(FLinearColor(0.6f, 0.8f, 1.0f));
                break;
            case EWorld_CaveType::WaterCave:
                PointLight->SetIntensity(40.0f);
                PointLight->SetLightColor(FLinearColor(0.7f, 0.9f, 1.0f));
                break;
            default:
                PointLight->SetIntensity(40.0f);
                PointLight->SetLightColor(FLinearColor(0.8f, 0.7f, 0.5f));
                break;
        }
        
        PointLight->SetAttenuationRadius(800.0f);
        LightActor->SetActorLabel(TEXT("CaveLight"));
    }
}

void AWorld_CaveSystemGenerator::AddCaveWaterFeatures(const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create underground water pool marker
    FVector WaterLocation = Location;
    WaterLocation.Z -= 50.0f;
    
    FActorSpawnParameters SpawnParams;
    AActor* WaterFeature = World->SpawnActor<AActor>(AActor::StaticClass(), WaterLocation, FRotator::ZeroRotator, SpawnParams);
    if (WaterFeature)
    {
        WaterFeature->SetActorLabel(TEXT("CaveWaterPool"));
    }
}

void AWorld_CaveSystemGenerator::AddCaveCrystals(const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create crystal formation markers
    for (int32 i = 0; i < 5; i++)
    {
        FVector CrystalLocation = Location;
        CrystalLocation.X += FMath::RandRange(-200.0f, 200.0f);
        CrystalLocation.Y += FMath::RandRange(-200.0f, 200.0f);
        CrystalLocation.Z -= FMath::RandRange(20.0f, 80.0f);
        
        FActorSpawnParameters SpawnParams;
        AActor* Crystal = World->SpawnActor<AActor>(AActor::StaticClass(), CrystalLocation, FRotator::ZeroRotator, SpawnParams);
        if (Crystal)
        {
            FString CrystalName = FString::Printf(TEXT("CaveCrystal_%d"), i + 1);
            Crystal->SetActorLabel(CrystalName);
        }
    }
}

bool AWorld_CaveSystemGenerator::IsValidCaveLocation(const FVector& Location) const
{
    // Check minimum distance from other caves
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Simple validation - ensure not too close to world origin
    float DistanceFromOrigin = FVector::Dist(Location, FVector::ZeroVector);
    return DistanceFromOrigin > 500.0f && DistanceFromOrigin < 5000.0f;
}

TArray<FVector> AWorld_CaveSystemGenerator::GenerateTunnelPath(const FVector& Start, const FVector& End)
{
    TArray<FVector> PathPoints;
    
    int32 SegmentCount = FMath::RandRange(3, 8);
    for (int32 i = 0; i <= SegmentCount; i++)
    {
        float Alpha = static_cast<float>(i) / SegmentCount;
        FVector PathPoint = FMath::Lerp(Start, End, Alpha);
        
        // Add some randomness to create natural cave tunnels
        PathPoint.X += FMath::RandRange(-100.0f, 100.0f);
        PathPoint.Y += FMath::RandRange(-100.0f, 100.0f);
        PathPoint.Z += FMath::RandRange(-50.0f, 50.0f);
        
        PathPoints.Add(PathPoint);
    }
    
    return PathPoints;
}

FVector AWorld_CaveSystemGenerator::GetRandomCaveLocation() const
{
    return FVector(
        FMath::RandRange(-4000.0f, 4000.0f),
        FMath::RandRange(-4000.0f, 4000.0f),
        FMath::RandRange(50.0f, 200.0f)
    );
}

float AWorld_CaveSystemGenerator::CalculateCaveComplexity(EWorld_CaveType CaveType) const
{
    switch (CaveType)
    {
        case EWorld_CaveType::SmallCave: return 1.0f;
        case EWorld_CaveType::MediumCave: return 2.0f;
        case EWorld_CaveType::LargeCave: return 4.0f;
        case EWorld_CaveType::CrystalCave: return 3.0f;
        case EWorld_CaveType::WaterCave: return 2.5f;
        default: return 1.5f;
    }
}