#include "World_BiomeSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Math/UnrealMathUtility.h"

UWorld_BiomeSystem::UWorld_BiomeSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = BiomeUpdateInterval;
    
    // Initialize default biome data
    InitializeBiomeDatabase();
}

void UWorld_BiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache initial biome actors
    UpdateBiomeCache();
    
    // Set initial biome based on owner location
    if (AActor* Owner = GetOwner())
    {
        UpdateCurrentBiome(Owner->GetActorLocation());
    }
}

void UWorld_BiomeSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update biome cache periodically
    LastBiomeUpdateTime += DeltaTime;
    if (LastBiomeUpdateTime >= BiomeUpdateInterval)
    {
        UpdateBiomeCache();
        LastBiomeUpdateTime = 0.0f;
    }
    
    // Update current biome if we have an owner
    if (AActor* Owner = GetOwner())
    {
        UpdateCurrentBiome(Owner->GetActorLocation());
    }
}

void UWorld_BiomeSystem::InitializeBiomeDatabase()
{
    // Forest biome
    FWorld_BiomeData ForestBiome;
    ForestBiome.BiomeType = EBiomeType::Forest;
    ForestBiome.BiomeName = TEXT("Dense Forest");
    ForestBiome.Temperature = 18.0f;
    ForestBiome.Humidity = 0.8f;
    ForestBiome.Elevation = 150.0f;
    ForestBiome.VegetationDensity = 1.5f;
    ForestBiome.FogColor = FLinearColor(0.7f, 0.9f, 0.8f, 1.0f);
    ForestBiome.FogDensity = 0.03f;
    BiomeDatabase.Add(EBiomeType::Forest, ForestBiome);
    
    // Plains biome
    FWorld_BiomeData PlainsBiome;
    PlainsBiome.BiomeType = EBiomeType::Plains;
    PlainsBiome.BiomeName = TEXT("Grasslands");
    PlainsBiome.Temperature = 22.0f;
    PlainsBiome.Humidity = 0.4f;
    PlainsBiome.Elevation = 50.0f;
    PlainsBiome.VegetationDensity = 0.6f;
    PlainsBiome.FogColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    PlainsBiome.FogDensity = 0.01f;
    BiomeDatabase.Add(EBiomeType::Plains, PlainsBiome);
    
    // Desert biome
    FWorld_BiomeData DesertBiome;
    DesertBiome.BiomeType = EBiomeType::Desert;
    DesertBiome.BiomeName = TEXT("Arid Desert");
    DesertBiome.Temperature = 35.0f;
    DesertBiome.Humidity = 0.1f;
    DesertBiome.Elevation = 200.0f;
    DesertBiome.VegetationDensity = 0.2f;
    DesertBiome.FogColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    DesertBiome.FogDensity = 0.005f;
    BiomeDatabase.Add(EBiomeType::Desert, DesertBiome);
    
    // Mountain biome
    FWorld_BiomeData MountainBiome;
    MountainBiome.BiomeType = EBiomeType::Mountain;
    MountainBiome.BiomeName = TEXT("Rocky Mountains");
    MountainBiome.Temperature = 8.0f;
    MountainBiome.Humidity = 0.3f;
    MountainBiome.Elevation = 800.0f;
    MountainBiome.VegetationDensity = 0.4f;
    MountainBiome.FogColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
    MountainBiome.FogDensity = 0.04f;
    BiomeDatabase.Add(EBiomeType::Mountain, MountainBiome);
    
    // Swamp biome
    FWorld_BiomeData SwampBiome;
    SwampBiome.BiomeType = EBiomeType::Swamp;
    SwampBiome.BiomeName = TEXT("Murky Swampland");
    SwampBiome.Temperature = 24.0f;
    SwampBiome.Humidity = 0.9f;
    SwampBiome.Elevation = 10.0f;
    SwampBiome.VegetationDensity = 1.2f;
    SwampBiome.FogColor = FLinearColor(0.6f, 0.7f, 0.5f, 1.0f);
    SwampBiome.FogDensity = 0.06f;
    BiomeDatabase.Add(EBiomeType::Swamp, SwampBiome);
}

EBiomeType UWorld_BiomeSystem::GetBiomeAtLocation(const FVector& WorldLocation)
{
    // Use noise-based biome determination
    float NoiseValue = CalculateNoiseValue(WorldLocation, 0.001f, 3);
    float ElevationFactor = WorldLocation.Z / 1000.0f; // Normalize elevation
    
    // Combine noise and elevation to determine biome
    float BiomeValue = NoiseValue + ElevationFactor * 0.3f;
    
    if (BiomeValue > 0.6f)
    {
        return EBiomeType::Mountain;
    }
    else if (BiomeValue > 0.3f)
    {
        return EBiomeType::Forest;
    }
    else if (BiomeValue > 0.0f)
    {
        return EBiomeType::Plains;
    }
    else if (BiomeValue > -0.3f)
    {
        return EBiomeType::Desert;
    }
    else
    {
        return EBiomeType::Swamp;
    }
}

FWorld_BiomeData UWorld_BiomeSystem::GetBiomeData(EBiomeType BiomeType)
{
    if (FWorld_BiomeData* FoundData = BiomeDatabase.Find(BiomeType))
    {
        return *FoundData;
    }
    
    // Return default forest biome if not found
    return BiomeDatabase.FindRef(EBiomeType::Forest);
}

void UWorld_BiomeSystem::UpdateCurrentBiome(const FVector& PlayerLocation)
{
    EBiomeType NewBiome = GetBiomeAtLocation(PlayerLocation);
    
    if (NewBiome != CurrentBiome)
    {
        CurrentBiome = NewBiome;
        CurrentBiomeCenter = PlayerLocation;
        
        // Apply biome atmosphere effects
        ApplyBiomeAtmosphere(CurrentBiome);
        
        // Log biome change
        FWorld_BiomeData BiomeData = GetBiomeData(CurrentBiome);
        UE_LOG(LogTemp, Log, TEXT("Biome changed to: %s"), *BiomeData.BiomeName);
    }
    
    // Update biome influence strength based on distance from biome center
    float DistanceFromCenter = FVector::Dist(PlayerLocation, CurrentBiomeCenter);
    BiomeInfluenceStrength = FMath::Clamp(1.0f - (DistanceFromCenter / BiomeCheckRadius), 0.1f, 1.0f);
}

float UWorld_BiomeSystem::CalculateBiomeInfluence(const FVector& Location, EBiomeType BiomeType)
{
    EBiomeType LocationBiome = GetBiomeAtLocation(Location);
    
    if (LocationBiome == BiomeType)
    {
        return 1.0f;
    }
    
    // Calculate influence based on distance to biome boundary
    float InfluenceRadius = 2000.0f; // 20 meters
    float MinInfluence = 0.0f;
    
    // Simple distance-based falloff
    return FMath::Lerp(1.0f, MinInfluence, FMath::Clamp(0.0f, 1.0f, 0.5f));
}

TArray<AActor*> UWorld_BiomeSystem::GetBiomeActorsInRadius(const FVector& Center, float Radius, EBiomeType BiomeType)
{
    TArray<AActor*> Result;
    
    if (TArray<AActor*>* CachedActors = CachedBiomeActors.Find(BiomeType))
    {
        for (AActor* Actor : *CachedActors)
        {
            if (IsValid(Actor))
            {
                float Distance = FVector::Dist(Actor->GetActorLocation(), Center);
                if (Distance <= Radius)
                {
                    Result.Add(Actor);
                }
            }
        }
    }
    
    return Result;
}

void UWorld_BiomeSystem::ApplyBiomeEffectsToActor(AActor* TargetActor, EBiomeType BiomeType, float Intensity)
{
    if (!IsValid(TargetActor))
    {
        return;
    }
    
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    // Apply visual effects based on biome
    if (UStaticMeshComponent* MeshComp = TargetActor->FindComponentByClass<UStaticMeshComponent>())
    {
        // Could apply material changes, scaling, etc.
        FVector CurrentScale = TargetActor->GetActorScale3D();
        float ScaleFactor = FMath::Lerp(1.0f, BiomeData.VegetationDensity, Intensity);
        TargetActor->SetActorScale3D(CurrentScale * ScaleFactor);
    }
}

void UWorld_BiomeSystem::GenerateBiomeVegetation(const FVector& Center, float Radius, EBiomeType BiomeType)
{
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    
    if (BiomeData.VegetationMeshes.Num() == 0)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 VegetationCount = FMath::RoundToInt(BiomeData.VegetationDensity * 10.0f);
    
    for (int32 i = 0; i < VegetationCount; i++)
    {
        // Generate random position within radius
        FVector RandomOffset = FMath::VRand() * FMath::RandRange(0.0f, Radius);
        FVector SpawnLocation = Center + RandomOffset;
        
        // Trace to ground
        FHitResult HitResult;
        FVector TraceStart = SpawnLocation + FVector(0, 0, 1000);
        FVector TraceEnd = SpawnLocation - FVector(0, 0, 1000);
        
        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            SpawnLocation = HitResult.Location;
            
            // Spawn vegetation actor
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
            
            if (AStaticMeshActor* VegetationActor = World->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator, SpawnParams))
            {
                // Set random mesh from biome vegetation
                if (BiomeData.VegetationMeshes.Num() > 0)
                {
                    int32 RandomIndex = FMath::RandRange(0, BiomeData.VegetationMeshes.Num() - 1);
                    TSoftObjectPtr<UStaticMesh> MeshPtr = BiomeData.VegetationMeshes[RandomIndex];
                    
                    if (UStaticMesh* Mesh = MeshPtr.LoadSynchronous())
                    {
                        VegetationActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
                    }
                }
                
                // Apply random rotation and scale
                FRotator RandomRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
                float RandomScale = FMath::RandRange(0.8f, 1.2f);
                
                VegetationActor->SetActorRotation(RandomRotation);
                VegetationActor->SetActorScale3D(FVector(RandomScale));
                
                // Tag for biome system
                VegetationActor->Tags.Add(FName(*FString::Printf(TEXT("Biome_%s"), *UEnum::GetValueAsString(BiomeType))));
            }
        }
    }
}

void UWorld_BiomeSystem::ApplyBiomeAtmosphere(EBiomeType BiomeType)
{
    FWorld_BiomeData BiomeData = GetBiomeData(BiomeType);
    UWorld* World = GetWorld();
    
    if (!World)
    {
        return;
    }
    
    // Find and update exponential height fog
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    
    for (AActor* FogActor : FogActors)
    {
        if (AExponentialHeightFog* HeightFog = Cast<AExponentialHeightFog>(FogActor))
        {
            if (UExponentialHeightFogComponent* FogComponent = HeightFog->GetComponent())
            {
                FogComponent->SetFogInscatteringColor(BiomeData.FogColor);
                FogComponent->SetFogDensity(BiomeData.FogDensity);
            }
        }
    }
}

void UWorld_BiomeSystem::CreateBiomeTransitionZone(const FVector& Location, EBiomeType FromBiome, EBiomeType ToBiome)
{
    // Create a transition zone between two biomes
    FWorld_BiomeTransition Transition;
    Transition.FromBiome = FromBiome;
    Transition.ToBiome = ToBiome;
    Transition.TransitionDistance = 1000.0f;
    Transition.BlendStrength = 0.5f;
    
    BiomeTransitions.Add(Transition);
    
    // Generate blended vegetation in transition zone
    float TransitionRadius = Transition.TransitionDistance * 0.5f;
    
    // Generate vegetation from both biomes with reduced density
    GenerateBiomeVegetation(Location, TransitionRadius, FromBiome);
    GenerateBiomeVegetation(Location, TransitionRadius, ToBiome);
}

void UWorld_BiomeSystem::UpdateBiomeCache()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Clear existing cache
    CachedBiomeActors.Empty();
    
    // Get all actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    // Categorize actors by biome based on their tags
    for (AActor* Actor : AllActors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }
        
        // Check if actor has biome tags
        for (const FName& Tag : Actor->Tags)
        {
            FString TagString = Tag.ToString();
            if (TagString.StartsWith(TEXT("Biome_")))
            {
                FString BiomeString = TagString.RightChop(6); // Remove "Biome_" prefix
                
                // Convert string to biome type
                for (int32 i = 0; i < (int32)EBiomeType::Count; i++)
                {
                    EBiomeType BiomeType = (EBiomeType)i;
                    FString EnumString = UEnum::GetValueAsString(BiomeType);
                    EnumString = EnumString.RightChop(EnumString.Find(TEXT("::")) + 2); // Remove enum prefix
                    
                    if (EnumString == BiomeString)
                    {
                        if (!CachedBiomeActors.Contains(BiomeType))
                        {
                            CachedBiomeActors.Add(BiomeType, TArray<AActor*>());
                        }
                        CachedBiomeActors[BiomeType].Add(Actor);
                        break;
                    }
                }
            }
        }
    }
}

float UWorld_BiomeSystem::CalculateNoiseValue(const FVector& Location, float Scale, int32 Octaves)
{
    float NoiseValue = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = Scale;
    float MaxValue = 0.0f;
    
    for (int32 i = 0; i < Octaves; i++)
    {
        // Simple pseudo-noise using sine waves
        float X = Location.X * Frequency;
        float Y = Location.Y * Frequency;
        
        float SampleValue = FMath::Sin(X * 0.01f) * FMath::Cos(Y * 0.01f);
        SampleValue += FMath::Sin(X * 0.02f) * FMath::Cos(Y * 0.02f) * 0.5f;
        
        NoiseValue += SampleValue * Amplitude;
        MaxValue += Amplitude;
        
        Amplitude *= 0.5f;
        Frequency *= 2.0f;
    }
    
    return NoiseValue / MaxValue;
}

FVector UWorld_BiomeSystem::GetBiomeNoiseOffset(EBiomeType BiomeType)
{
    // Return different noise offsets for each biome type
    switch (BiomeType)
    {
        case EBiomeType::Forest:
            return FVector(0, 0, 0);
        case EBiomeType::Plains:
            return FVector(1000, 0, 0);
        case EBiomeType::Desert:
            return FVector(0, 1000, 0);
        case EBiomeType::Mountain:
            return FVector(1000, 1000, 0);
        case EBiomeType::Swamp:
            return FVector(500, 500, 0);
        default:
            return FVector::ZeroVector;
    }
}