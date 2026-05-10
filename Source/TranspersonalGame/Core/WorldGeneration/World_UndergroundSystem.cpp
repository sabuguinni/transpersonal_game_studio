#include "World_UndergroundSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Landscape/Landscape.h"
#include "LandscapeComponent.h"

UWorld_UndergroundSystem::UWorld_UndergroundSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Cave generation parameters - Cretaceous period realistic caves
    CaveDepthRange = FVector2D(50.0f, 300.0f);
    CaveTunnelWidth = FVector2D(200.0f, 800.0f);
    CaveComplexity = 0.7f;
    MaxCavesPerChunk = 3;
    CaveSpawnChance = 0.3f;
    
    // Underground water parameters
    UndergroundWaterLevel = -150.0f;
    WaterFlowRate = 2.0f;
    
    // Mineral deposit parameters - realistic Cretaceous minerals
    MineralDensity = 0.15f;
    MaxMineralsPerCave = 8;
    
    // Audio parameters for underground ambience
    CaveAmbienceVolume = 0.6f;
    WaterDripVolume = 0.4f;
    EchoIntensity = 0.8f;
    
    // Performance optimization
    MaxActiveUndergroundChunks = 4;
    UndergroundLODDistance = 2000.0f;
    
    bIsInitialized = false;
}

void UWorld_UndergroundSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeUndergroundSystem();
}

void UWorld_UndergroundSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsInitialized)
    {
        UpdateUndergroundChunks(DeltaTime);
        UpdateUndergroundAudio(DeltaTime);
        UpdateWaterFlow(DeltaTime);
    }
}

void UWorld_UndergroundSystem::InitializeUndergroundSystem()
{
    if (bIsInitialized)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World_UndergroundSystem: Failed to get world reference"));
        return;
    }
    
    // Initialize cave generation noise
    CaveNoiseGenerator.SetSeed(FMath::RandRange(1000, 9999));
    CaveNoiseGenerator.SetFrequency(0.02f);
    CaveNoiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    
    // Initialize mineral generation noise
    MineralNoiseGenerator.SetSeed(FMath::RandRange(1000, 9999));
    MineralNoiseGenerator.SetFrequency(0.05f);
    MineralNoiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    
    // Initialize water flow noise
    WaterNoiseGenerator.SetSeed(FMath::RandRange(1000, 9999));
    WaterNoiseGenerator.SetFrequency(0.03f);
    WaterNoiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    
    // Load cave materials and sounds
    LoadUndergroundAssets();
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("World_UndergroundSystem: Underground system initialized successfully"));
}

void UWorld_UndergroundSystem::GenerateUndergroundChunk(FVector ChunkLocation, int32 ChunkSize)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("World_UndergroundSystem: System not initialized, cannot generate chunk"));
        return;
    }
    
    // Check if we should generate caves in this chunk
    float CaveChance = FMath::FRand();
    if (CaveChance > CaveSpawnChance)
    {
        return;
    }
    
    // Generate cave systems
    int32 NumCaves = FMath::RandRange(1, MaxCavesPerChunk);
    for (int32 i = 0; i < NumCaves; i++)
    {
        FVector CaveLocation = ChunkLocation + FVector(
            FMath::RandRange(-ChunkSize/2, ChunkSize/2),
            FMath::RandRange(-ChunkSize/2, ChunkSize/2),
            FMath::RandRange(CaveDepthRange.X, CaveDepthRange.Y) * -1.0f
        );
        
        GenerateCaveSystem(CaveLocation);
    }
    
    // Generate underground water bodies
    GenerateUndergroundWater(ChunkLocation, ChunkSize);
    
    // Generate mineral deposits
    GenerateMineralDeposits(ChunkLocation, ChunkSize);
    
    // Add to active chunks
    FWorld_UndergroundChunk NewChunk;
    NewChunk.ChunkLocation = ChunkLocation;
    NewChunk.ChunkSize = ChunkSize;
    NewChunk.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    ActiveUndergroundChunks.Add(NewChunk);
    
    UE_LOG(LogTemp, Log, TEXT("World_UndergroundSystem: Generated underground chunk at %s"), *ChunkLocation.ToString());
}

void UWorld_UndergroundSystem::GenerateCaveSystem(FVector CaveEntrance)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create cave system data
    FWorld_CaveSystem CaveSystem;
    CaveSystem.EntranceLocation = CaveEntrance;
    CaveSystem.CaveDepth = FMath::RandRange(CaveDepthRange.X, CaveDepthRange.Y);
    CaveSystem.TunnelWidth = FMath::RandRange(CaveTunnelWidth.X, CaveTunnelWidth.Y);
    CaveSystem.bHasWater = FMath::RandBool();
    CaveSystem.bHasMinerals = FMath::RandBool();
    
    // Generate cave entrance
    AActor* CaveEntrance = CreateCaveEntrance(CaveSystem.EntranceLocation);
    if (CaveEntrance)
    {
        CaveSystem.CaveActors.Add(CaveEntrance);
    }
    
    // Generate cave tunnels using noise
    GenerateCaveTunnels(CaveSystem);
    
    // Add water if needed
    if (CaveSystem.bHasWater)
    {
        GenerateCaveWater(CaveSystem);
    }
    
    // Add minerals if needed
    if (CaveSystem.bHasMinerals)
    {
        GenerateCaveMinerals(CaveSystem);
    }
    
    // Add cave audio
    AddCaveAmbience(CaveSystem);
    
    // Store cave system
    CaveSystems.Add(CaveSystem);
}

AActor* UWorld_UndergroundSystem::CreateCaveEntrance(FVector Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Spawn cave entrance actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* CaveActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator, SpawnParams);
    if (!CaveActor)
    {
        return nullptr;
    }
    
    // Set cave entrance mesh (using basic geometry for now)
    UStaticMeshComponent* MeshComp = CaveActor->GetStaticMeshComponent();
    if (MeshComp)
    {
        // Load cave entrance mesh
        UStaticMesh* CaveMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
        if (CaveMesh)
        {
            MeshComp->SetStaticMesh(CaveMesh);
            MeshComp->SetWorldScale3D(FVector(3.0f, 3.0f, 1.5f));
            
            // Apply cave material
            if (CaveMaterial)
            {
                MeshComp->SetMaterial(0, CaveMaterial);
            }
        }
    }
    
    CaveActor->SetActorLabel(TEXT("CaveEntrance"));
    
    return CaveActor;
}

void UWorld_UndergroundSystem::GenerateCaveTunnels(FWorld_CaveSystem& CaveSystem)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Generate tunnel segments using noise-based paths
    int32 NumTunnelSegments = FMath::RandRange(3, 8);
    FVector CurrentLocation = CaveSystem.EntranceLocation;
    
    for (int32 i = 0; i < NumTunnelSegments; i++)
    {
        // Use noise to determine tunnel direction
        float NoiseX = CaveNoiseGenerator.GetNoise(CurrentLocation.X * 0.01f, CurrentLocation.Y * 0.01f);
        float NoiseY = CaveNoiseGenerator.GetNoise(CurrentLocation.X * 0.01f + 100.0f, CurrentLocation.Y * 0.01f + 100.0f);
        
        FVector TunnelDirection = FVector(NoiseX, NoiseY, -0.3f).GetSafeNormal();
        FVector NextLocation = CurrentLocation + TunnelDirection * FMath::RandRange(200.0f, 500.0f);
        
        // Create tunnel segment
        AActor* TunnelSegment = CreateTunnelSegment(CurrentLocation, NextLocation, CaveSystem.TunnelWidth);
        if (TunnelSegment)
        {
            CaveSystem.CaveActors.Add(TunnelSegment);
        }
        
        CurrentLocation = NextLocation;
    }
}

AActor* UWorld_UndergroundSystem::CreateTunnelSegment(FVector StartLocation, FVector EndLocation, float Width)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    FVector MidPoint = (StartLocation + EndLocation) * 0.5f;
    FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
    FRotator Rotation = Direction.Rotation();
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* TunnelActor = World->SpawnActor<AStaticMeshActor>(MidPoint, Rotation, SpawnParams);
    if (!TunnelActor)
    {
        return nullptr;
    }
    
    UStaticMeshComponent* MeshComp = TunnelActor->GetStaticMeshComponent();
    if (MeshComp)
    {
        // Use cylinder for tunnel segment
        UStaticMesh* TunnelMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
        if (TunnelMesh)
        {
            MeshComp->SetStaticMesh(TunnelMesh);
            
            float TunnelLength = FVector::Dist(StartLocation, EndLocation);
            MeshComp->SetWorldScale3D(FVector(Width / 100.0f, Width / 100.0f, TunnelLength / 100.0f));
            
            if (CaveMaterial)
            {
                MeshComp->SetMaterial(0, CaveMaterial);
            }
        }
    }
    
    TunnelActor->SetActorLabel(TEXT("CaveTunnel"));
    
    return TunnelActor;
}

void UWorld_UndergroundSystem::GenerateUndergroundWater(FVector ChunkLocation, int32 ChunkSize)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Generate underground water bodies using noise
    int32 NumWaterBodies = FMath::RandRange(1, 3);
    
    for (int32 i = 0; i < NumWaterBodies; i++)
    {
        FVector WaterLocation = ChunkLocation + FVector(
            FMath::RandRange(-ChunkSize/3, ChunkSize/3),
            FMath::RandRange(-ChunkSize/3, ChunkSize/3),
            UndergroundWaterLevel
        );
        
        // Check noise value for water placement
        float WaterNoise = WaterNoiseGenerator.GetNoise(WaterLocation.X * 0.01f, WaterLocation.Y * 0.01f);
        if (WaterNoise > 0.3f)
        {
            CreateUndergroundWaterBody(WaterLocation);
        }
    }
}

void UWorld_UndergroundSystem::CreateUndergroundWaterBody(FVector Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* WaterActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator, SpawnParams);
    if (!WaterActor)
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = WaterActor->GetStaticMeshComponent();
    if (MeshComp)
    {
        UStaticMesh* WaterMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane"));
        if (WaterMesh)
        {
            MeshComp->SetStaticMesh(WaterMesh);
            MeshComp->SetWorldScale3D(FVector(5.0f, 5.0f, 1.0f));
            
            // Apply water material
            if (WaterMaterial)
            {
                MeshComp->SetMaterial(0, WaterMaterial);
            }
        }
    }
    
    WaterActor->SetActorLabel(TEXT("UndergroundWater"));
    
    // Add water flow audio
    UAudioComponent* AudioComp = NewObject<UAudioComponent>(WaterActor);
    if (AudioComp && WaterFlowSound)
    {
        AudioComp->SetSound(WaterFlowSound);
        AudioComp->SetVolumeMultiplier(WaterDripVolume);
        AudioComp->bAutoActivate = true;
        AudioComp->AttachToComponent(WaterActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
        AudioComp->RegisterComponent();
    }
}

void UWorld_UndergroundSystem::GenerateMineralDeposits(FVector ChunkLocation, int32 ChunkSize)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Generate mineral deposits using cellular noise
    int32 NumMinerals = FMath::RandRange(2, 6);
    
    for (int32 i = 0; i < NumMinerals; i++)
    {
        FVector MineralLocation = ChunkLocation + FVector(
            FMath::RandRange(-ChunkSize/2, ChunkSize/2),
            FMath::RandRange(-ChunkSize/2, ChunkSize/2),
            FMath::RandRange(-50.0f, -200.0f)
        );
        
        // Check noise value for mineral placement
        float MineralNoise = MineralNoiseGenerator.GetNoise(MineralLocation.X * 0.01f, MineralLocation.Y * 0.01f);
        if (MineralNoise > 0.4f)
        {
            EWorld_MineralType MineralType = static_cast<EWorld_MineralType>(FMath::RandRange(0, 3));
            CreateMineralDeposit(MineralLocation, MineralType);
        }
    }
}

void UWorld_UndergroundSystem::CreateMineralDeposit(FVector Location, EWorld_MineralType MineralType)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* MineralActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator, SpawnParams);
    if (!MineralActor)
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = MineralActor->GetStaticMeshComponent();
    if (MeshComp)
    {
        UStaticMesh* MineralMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
        if (MineralMesh)
        {
            MeshComp->SetStaticMesh(MineralMesh);
            MeshComp->SetWorldScale3D(FVector(0.5f, 0.5f, 0.8f));
            
            // Apply mineral-specific material based on type
            UMaterialInterface* MineralMaterial = GetMineralMaterial(MineralType);
            if (MineralMaterial)
            {
                MeshComp->SetMaterial(0, MineralMaterial);
            }
        }
    }
    
    FString MineralName = GetMineralTypeName(MineralType);
    MineralActor->SetActorLabel(FString::Printf(TEXT("Mineral_%s"), *MineralName));
    
    // Store mineral data
    FWorld_MineralDeposit MineralData;
    MineralData.Location = Location;
    MineralData.MineralType = MineralType;
    MineralData.Quantity = FMath::RandRange(10, 50);
    MineralData.Quality = FMath::RandRange(0.3f, 1.0f);
    
    MineralDeposits.Add(MineralData);
}

void UWorld_UndergroundSystem::GenerateCaveWater(FWorld_CaveSystem& CaveSystem)
{
    if (!CaveSystem.bHasWater)
    {
        return;
    }
    
    // Add water to random tunnel segments
    int32 NumWaterSections = FMath::RandRange(1, 3);
    
    for (int32 i = 0; i < NumWaterSections && i < CaveSystem.CaveActors.Num(); i++)
    {
        if (CaveSystem.CaveActors[i])
        {
            FVector WaterLocation = CaveSystem.CaveActors[i]->GetActorLocation() + FVector(0, 0, -50.0f);
            CreateUndergroundWaterBody(WaterLocation);
        }
    }
}

void UWorld_UndergroundSystem::GenerateCaveMinerals(FWorld_CaveSystem& CaveSystem)
{
    if (!CaveSystem.bHasMinerals)
    {
        return;
    }
    
    // Add minerals to cave walls
    int32 NumMinerals = FMath::RandRange(2, MaxMineralsPerCave);
    
    for (int32 i = 0; i < NumMinerals; i++)
    {
        if (CaveSystem.CaveActors.Num() > 0)
        {
            int32 RandomActorIndex = FMath::RandRange(0, CaveSystem.CaveActors.Num() - 1);
            AActor* CaveActor = CaveSystem.CaveActors[RandomActorIndex];
            
            if (CaveActor)
            {
                FVector MineralLocation = CaveActor->GetActorLocation() + FVector(
                    FMath::RandRange(-100.0f, 100.0f),
                    FMath::RandRange(-100.0f, 100.0f),
                    FMath::RandRange(-20.0f, 20.0f)
                );
                
                EWorld_MineralType MineralType = static_cast<EWorld_MineralType>(FMath::RandRange(0, 3));
                CreateMineralDeposit(MineralLocation, MineralType);
            }
        }
    }
}

void UWorld_UndergroundSystem::AddCaveAmbience(FWorld_CaveSystem& CaveSystem)
{
    if (CaveSystem.CaveActors.Num() == 0)
    {
        return;
    }
    
    // Add ambient sound to cave entrance
    AActor* EntranceActor = CaveSystem.CaveActors[0];
    if (EntranceActor && CaveAmbienceSound)
    {
        UAudioComponent* AudioComp = NewObject<UAudioComponent>(EntranceActor);
        if (AudioComp)
        {
            AudioComp->SetSound(CaveAmbienceSound);
            AudioComp->SetVolumeMultiplier(CaveAmbienceVolume);
            AudioComp->bAutoActivate = true;
            AudioComp->AttachToComponent(EntranceActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
            AudioComp->RegisterComponent();
        }
    }
}

void UWorld_UndergroundSystem::UpdateUndergroundChunks(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update active chunks and remove old ones
    for (int32 i = ActiveUndergroundChunks.Num() - 1; i >= 0; i--)
    {
        FWorld_UndergroundChunk& Chunk = ActiveUndergroundChunks[i];
        
        // Check if chunk is too old or too far from player
        if (CurrentTime - Chunk.LastUpdateTime > 300.0f) // 5 minutes
        {
            ActiveUndergroundChunks.RemoveAt(i);
            continue;
        }
        
        // Update chunk if needed
        Chunk.LastUpdateTime = CurrentTime;
    }
    
    // Limit number of active chunks for performance
    while (ActiveUndergroundChunks.Num() > MaxActiveUndergroundChunks)
    {
        ActiveUndergroundChunks.RemoveAt(0);
    }
}

void UWorld_UndergroundSystem::UpdateUndergroundAudio(float DeltaTime)
{
    // Update cave ambience based on player proximity
    // This would typically check player location and adjust audio accordingly
    // For now, we'll just update the echo intensity based on time
    
    float TimeVariation = FMath::Sin(GetWorld()->GetTimeSeconds() * 0.1f) * 0.2f;
    float CurrentEcho = EchoIntensity + TimeVariation;
    
    // Apply echo to all cave audio components
    // This would require tracking audio components, which we'll implement in a future iteration
}

void UWorld_UndergroundSystem::UpdateWaterFlow(float DeltaTime)
{
    // Update underground water flow animation
    // This would animate water materials and update flow sounds
    // For now, we'll just track the flow rate
    
    float TimeVariation = FMath::Sin(GetWorld()->GetTimeSeconds() * WaterFlowRate) * 0.5f;
    float CurrentFlowRate = WaterFlowRate + TimeVariation;
    
    // Apply flow rate to water materials and sounds
    // Implementation would require material parameter collection
}

void UWorld_UndergroundSystem::LoadUndergroundAssets()
{
    // Load cave materials
    CaveMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    WaterMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    
    // Load mineral materials (using basic materials for now)
    IronMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    CopperMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    StoneMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    CrystalMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    
    // Load audio assets (would be replaced with actual cave sounds)
    CaveAmbienceSound = LoadObject<USoundCue>(nullptr, TEXT("/Engine/VRTemplate/Sounds/StartGameCue"));
    WaterFlowSound = LoadObject<USoundCue>(nullptr, TEXT("/Engine/VRTemplate/Sounds/StartGameCue"));
    
    UE_LOG(LogTemp, Log, TEXT("World_UndergroundSystem: Underground assets loaded"));
}

UMaterialInterface* UWorld_UndergroundSystem::GetMineralMaterial(EWorld_MineralType MineralType)
{
    switch (MineralType)
    {
        case EWorld_MineralType::Iron:
            return IronMaterial;
        case EWorld_MineralType::Copper:
            return CopperMaterial;
        case EWorld_MineralType::Stone:
            return StoneMaterial;
        case EWorld_MineralType::Crystal:
            return CrystalMaterial;
        default:
            return StoneMaterial;
    }
}

FString UWorld_UndergroundSystem::GetMineralTypeName(EWorld_MineralType MineralType)
{
    switch (MineralType)
    {
        case EWorld_MineralType::Iron:
            return TEXT("Iron");
        case EWorld_MineralType::Copper:
            return TEXT("Copper");
        case EWorld_MineralType::Stone:
            return TEXT("Stone");
        case EWorld_MineralType::Crystal:
            return TEXT("Crystal");
        default:
            return TEXT("Unknown");
    }
}

TArray<FWorld_CaveSystem> UWorld_UndergroundSystem::GetNearbyCaves(FVector Location, float Radius)
{
    TArray<FWorld_CaveSystem> NearbyCaves;
    
    for (const FWorld_CaveSystem& Cave : CaveSystems)
    {
        float Distance = FVector::Dist(Cave.EntranceLocation, Location);
        if (Distance <= Radius)
        {
            NearbyCaves.Add(Cave);
        }
    }
    
    return NearbyCaves;
}

TArray<FWorld_MineralDeposit> UWorld_UndergroundSystem::GetNearbyMinerals(FVector Location, float Radius)
{
    TArray<FWorld_MineralDeposit> NearbyMinerals;
    
    for (const FWorld_MineralDeposit& Mineral : MineralDeposits)
    {
        float Distance = FVector::Dist(Mineral.Location, Location);
        if (Distance <= Radius)
        {
            NearbyMinerals.Add(Mineral);
        }
    }
    
    return NearbyMinerals;
}

bool UWorld_UndergroundSystem::IsLocationUnderground(FVector Location)
{
    return Location.Z < -50.0f; // Simple underground check
}

float UWorld_UndergroundSystem::GetUndergroundDepth(FVector Location)
{
    return FMath::Abs(FMath::Min(Location.Z, 0.0f));
}