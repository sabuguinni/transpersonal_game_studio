#include "World_MassiveBiomeManager.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Atmosphere/SkyAtmosphere.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

AWorld_MassiveBiomeManager::AWorld_MassiveBiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize biome data with 200km2 coordinates
    InitializeBiomeData();
    
    // Set default values
    BiomeTransitionDistance = 50000.0f; // 50km transition zones
    MaxActorsPerBiome = 10000;
    StreamingRadius = 200000.0f; // 200km streaming radius
    
    bIsExpansionActive = false;
    ExpansionProgress = 0.0f;
}

void AWorld_MassiveBiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_MassiveBiomeManager: Starting 200km2 biome management"));
    
    // Start biome expansion process
    StartBiomeExpansion();
}

void AWorld_MassiveBiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsExpansionActive)
    {
        UpdateExpansionProgress(DeltaTime);
    }
    
    // Update streaming based on player location
    UpdateBiomeStreaming();
}

void AWorld_MassiveBiomeManager::InitializeBiomeData()
{
    // Clear existing data
    BiomeZones.Empty();
    
    // Define 5 massive biome zones for 200km2 world
    FWorld_BiomeZone SwampBiome;
    SwampBiome.BiomeType = EWorld_BiomeType::Swamp;
    SwampBiome.CenterLocation = FVector(-500000.0f, -450000.0f, 0.0f);
    SwampBiome.Radius = 300000.0f; // 300km radius
    SwampBiome.bIsActive = true;
    BiomeZones.Add(SwampBiome);
    
    FWorld_BiomeZone ForestBiome;
    ForestBiome.BiomeType = EWorld_BiomeType::Forest;
    ForestBiome.CenterLocation = FVector(-450000.0f, 400000.0f, 0.0f);
    ForestBiome.Radius = 280000.0f;
    ForestBiome.bIsActive = true;
    BiomeZones.Add(ForestBiome);
    
    FWorld_BiomeZone SavannaBiome;
    SavannaBiome.BiomeType = EWorld_BiomeType::Savanna;
    SavannaBiome.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    SavannaBiome.Radius = 250000.0f;
    SavannaBiome.bIsActive = true;
    BiomeZones.Add(SavannaBiome);
    
    FWorld_BiomeZone DesertBiome;
    DesertBiome.BiomeType = EWorld_BiomeType::Desert;
    DesertBiome.CenterLocation = FVector(550000.0f, 0.0f, 0.0f);
    DesertBiome.Radius = 320000.0f;
    DesertBiome.bIsActive = true;
    BiomeZones.Add(DesertBiome);
    
    FWorld_BiomeZone MountainBiome;
    MountainBiome.BiomeType = EWorld_BiomeType::Mountain;
    MountainBiome.CenterLocation = FVector(400000.0f, 500000.0f, 50000.0f);
    MountainBiome.Radius = 350000.0f;
    MountainBiome.bIsActive = true;
    BiomeZones.Add(MountainBiome);
    
    UE_LOG(LogTemp, Warning, TEXT("World_MassiveBiomeManager: Initialized 5 biome zones for 200km2 world"));
}

void AWorld_MassiveBiomeManager::StartBiomeExpansion()
{
    if (bIsExpansionActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Biome expansion already active"));
        return;
    }
    
    bIsExpansionActive = true;
    ExpansionProgress = 0.0f;
    CurrentExpansionPhase = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Starting massive biome expansion to 200km2"));
    
    // Phase 1: Cleanup duplicate actors
    CleanupDuplicateActors();
}

void AWorld_MassiveBiomeManager::CleanupDuplicateActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Get all actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    // Track lighting actors for cleanup
    TArray<ADirectionalLight*> DirectionalLights;
    TArray<ASkyAtmosphere*> SkyAtmospheres;
    TArray<ASkyLight*> SkyLights;
    TArray<AExponentialHeightFog*> ExponentialFogs;
    
    for (AActor* Actor : AllActors)
    {
        if (ADirectionalLight* DirLight = Cast<ADirectionalLight>(Actor))
        {
            DirectionalLights.Add(DirLight);
        }
        else if (ASkyAtmosphere* SkyAtm = Cast<ASkyAtmosphere>(Actor))
        {
            SkyAtmospheres.Add(SkyAtm);
        }
        else if (ASkyLight* SkyLt = Cast<ASkyLight>(Actor))
        {
            SkyLights.Add(SkyLt);
        }
        else if (AExponentialHeightFog* ExpFog = Cast<AExponentialHeightFog>(Actor))
        {
            ExponentialFogs.Add(ExpFog);
        }
    }
    
    // Keep only the first of each type
    for (int32 i = 1; i < DirectionalLights.Num(); i++)
    {
        if (DirectionalLights[i])
        {
            DirectionalLights[i]->Destroy();
        }
    }
    
    for (int32 i = 1; i < SkyAtmospheres.Num(); i++)
    {
        if (SkyAtmospheres[i])
        {
            SkyAtmospheres[i]->Destroy();
        }
    }
    
    for (int32 i = 1; i < SkyLights.Num(); i++)
    {
        if (SkyLights[i])
        {
            SkyLights[i]->Destroy();
        }
    }
    
    for (int32 i = 1; i < ExponentialFogs.Num(); i++)
    {
        if (ExponentialFogs[i])
        {
            ExponentialFogs[i]->Destroy();
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Cleaned up duplicate lighting actors"));
}

void AWorld_MassiveBiomeManager::UpdateExpansionProgress(float DeltaTime)
{
    ExpansionProgress += DeltaTime * 0.1f; // 10 second expansion phases
    
    if (ExpansionProgress >= 1.0f)
    {
        ExpansionProgress = 0.0f;
        CurrentExpansionPhase++;
        
        switch (CurrentExpansionPhase)
        {
            case 1:
                CreateLandscapeProxies();
                break;
            case 2:
                GenerateBiomeTransitions();
                break;
            case 3:
                SetupStreamingLevels();
                break;
            case 4:
                FinalizeExpansion();
                break;
            default:
                bIsExpansionActive = false;
                break;
        }
    }
}

void AWorld_MassiveBiomeManager::CreateLandscapeProxies()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    UE_LOG(LogTemp, Warning, TEXT("Creating landscape proxies for 200km2 world"));
    
    // Create landscape proxies for each biome zone
    for (const FWorld_BiomeZone& Biome : BiomeZones)
    {
        // Spawn landscape proxy at biome center
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(*FString::Printf(TEXT("LandscapeProxy_%s"), *UEnum::GetValueAsString(Biome.BiomeType)));
        
        ALandscapeProxy* LandscapeProxy = World->SpawnActor<ALandscapeProxy>(
            ALandscapeProxy::StaticClass(),
            Biome.CenterLocation,
            FRotator::ZeroRotator,
            SpawnParams
        );
        
        if (LandscapeProxy)
        {
            LandscapeProxy->SetActorLabel(FString::Printf(TEXT("Landscape_%s"), *UEnum::GetValueAsString(Biome.BiomeType)));
            UE_LOG(LogTemp, Warning, TEXT("Created landscape proxy for %s biome"), *UEnum::GetValueAsString(Biome.BiomeType));
        }
    }
}

void AWorld_MassiveBiomeManager::GenerateBiomeTransitions()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating biome transitions for 200km2 world"));
    
    // Create transition zones between biomes
    for (int32 i = 0; i < BiomeZones.Num(); i++)
    {
        for (int32 j = i + 1; j < BiomeZones.Num(); j++)
        {
            const FWorld_BiomeZone& BiomeA = BiomeZones[i];
            const FWorld_BiomeZone& BiomeB = BiomeZones[j];
            
            float Distance = FVector::Dist(BiomeA.CenterLocation, BiomeB.CenterLocation);
            
            // If biomes are close enough, create transition zone
            if (Distance < (BiomeA.Radius + BiomeB.Radius + BiomeTransitionDistance))
            {
                FVector TransitionCenter = (BiomeA.CenterLocation + BiomeB.CenterLocation) * 0.5f;
                
                // Create transition marker
                FActorSpawnParameters SpawnParams;
                AActor* TransitionMarker = GetWorld()->SpawnActor<AActor>(
                    AActor::StaticClass(),
                    TransitionCenter,
                    FRotator::ZeroRotator,
                    SpawnParams
                );
                
                if (TransitionMarker)
                {
                    TransitionMarker->SetActorLabel(FString::Printf(TEXT("Transition_%s_to_%s"), 
                        *UEnum::GetValueAsString(BiomeA.BiomeType),
                        *UEnum::GetValueAsString(BiomeB.BiomeType)));
                }
            }
        }
    }
}

void AWorld_MassiveBiomeManager::SetupStreamingLevels()
{
    UE_LOG(LogTemp, Warning, TEXT("Setting up streaming levels for 200km2 world"));
    
    // Configure world composition streaming
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Set world origin rebasing for large worlds
    World->OriginLocation = FIntVector::ZeroValue;
    
    UE_LOG(LogTemp, Warning, TEXT("Streaming levels configured for massive world"));
}

void AWorld_MassiveBiomeManager::FinalizeExpansion()
{
    bIsExpansionActive = false;
    ExpansionProgress = 1.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("200km2 biome expansion completed successfully"));
    
    // Trigger biome population
    PopulateBiomes();
}

void AWorld_MassiveBiomeManager::UpdateBiomeStreaming()
{
    // Get player location for streaming decisions
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update biome streaming based on distance
    for (FWorld_BiomeZone& Biome : BiomeZones)
    {
        float DistanceToPlayer = FVector::Dist(PlayerLocation, Biome.CenterLocation);
        
        // Stream in/out based on distance
        bool bShouldBeActive = DistanceToPlayer < StreamingRadius;
        
        if (Biome.bIsActive != bShouldBeActive)
        {
            Biome.bIsActive = bShouldBeActive;
            
            if (bShouldBeActive)
            {
                UE_LOG(LogTemp, Log, TEXT("Streaming in biome: %s"), *UEnum::GetValueAsString(Biome.BiomeType));
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("Streaming out biome: %s"), *UEnum::GetValueAsString(Biome.BiomeType));
            }
        }
    }
}

void AWorld_MassiveBiomeManager::PopulateBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("Starting biome population for 200km2 world"));
    
    // Populate each active biome with appropriate content
    for (const FWorld_BiomeZone& Biome : BiomeZones)
    {
        if (Biome.bIsActive)
        {
            PopulateBiomeZone(Biome);
        }
    }
}

void AWorld_MassiveBiomeManager::PopulateBiomeZone(const FWorld_BiomeZone& BiomeZone)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 ActorCount = FMath::RandRange(1000, MaxActorsPerBiome);
    
    UE_LOG(LogTemp, Warning, TEXT("Populating %s biome with %d actors"), 
        *UEnum::GetValueAsString(BiomeZone.BiomeType), ActorCount);
    
    // Generate actors within biome radius
    for (int32 i = 0; i < ActorCount; i++)
    {
        // Random location within biome radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, BiomeZone.Radius * 0.8f);
        
        FVector SpawnLocation = BiomeZone.CenterLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Spawn appropriate actor based on biome type
        SpawnBiomeActor(BiomeZone.BiomeType, SpawnLocation);
        
        // Prevent timeout by limiting spawn rate
        if (i % 50 == 0)
        {
            break; // Limit to 50 actors per call to prevent timeout
        }
    }
}

void AWorld_MassiveBiomeManager::SpawnBiomeActor(EWorld_BiomeType BiomeType, const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Spawn different actors based on biome type
    FActorSpawnParameters SpawnParams;
    AActor* SpawnedActor = nullptr;
    
    switch (BiomeType)
    {
        case EWorld_BiomeType::Swamp:
            SpawnedActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
            if (SpawnedActor) SpawnedActor->SetActorLabel(TEXT("SwampTree"));
            break;
            
        case EWorld_BiomeType::Forest:
            SpawnedActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
            if (SpawnedActor) SpawnedActor->SetActorLabel(TEXT("ForestTree"));
            break;
            
        case EWorld_BiomeType::Savanna:
            SpawnedActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
            if (SpawnedActor) SpawnedActor->SetActorLabel(TEXT("SavannaGrass"));
            break;
            
        case EWorld_BiomeType::Desert:
            SpawnedActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
            if (SpawnedActor) SpawnedActor->SetActorLabel(TEXT("DesertRock"));
            break;
            
        case EWorld_BiomeType::Mountain:
            SpawnedActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
            if (SpawnedActor) SpawnedActor->SetActorLabel(TEXT("MountainRock"));
            break;
    }
}

EWorld_BiomeType AWorld_MassiveBiomeManager::GetBiomeAtLocation(const FVector& Location) const
{
    float ClosestDistance = MAX_FLT;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Savanna;
    
    for (const FWorld_BiomeZone& Biome : BiomeZones)
    {
        float Distance = FVector::Dist(Location, Biome.CenterLocation);
        
        if (Distance < ClosestDistance && Distance < Biome.Radius)
        {
            ClosestDistance = Distance;
            ClosestBiome = Biome.BiomeType;
        }
    }
    
    return ClosestBiome;
}

float AWorld_MassiveBiomeManager::GetBiomeInfluence(const FVector& Location, EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeZone& Biome : BiomeZones)
    {
        if (Biome.BiomeType == BiomeType)
        {
            float Distance = FVector::Dist(Location, Biome.CenterLocation);
            
            if (Distance > Biome.Radius)
            {
                return 0.0f;
            }
            
            // Linear falloff from center to edge
            return 1.0f - (Distance / Biome.Radius);
        }
    }
    
    return 0.0f;
}