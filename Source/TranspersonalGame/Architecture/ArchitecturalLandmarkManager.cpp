#include "ArchitecturalLandmarkManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

AArchitecturalLandmarkManager::AArchitecturalLandmarkManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    bAutoSpawnLandmarks = true;
    MaxLandmarksPerBiome = 5;
    WeatheredStoneMaterial = nullptr;
    MossyStoneMaterial = nullptr;

    // Initialize default landmark registry
    LandmarkRegistry.Empty();
}

void AArchitecturalLandmarkManager::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoSpawnLandmarks)
    {
        GenerateLandmarksForAllBiomes();
    }
}

void AArchitecturalLandmarkManager::SpawnLandmarkAtLocation(EArch_LandmarkType LandmarkType, FVector Location, FRotator Rotation, const FString& BiomeName)
{
    switch (LandmarkType)
    {
    case EArch_LandmarkType::StoneArch:
        CreateStoneArchLandmark(Location, Rotation, BiomeName);
        break;
    case EArch_LandmarkType::StoneCircle:
        CreateStoneCircleLandmark(Location, Rotation, BiomeName);
        break;
    case EArch_LandmarkType::MegalithPillar:
        CreateMegalithPillarLandmark(Location, Rotation, BiomeName);
        break;
    case EArch_LandmarkType::RuinedWall:
        CreateRuinedWallLandmark(Location, Rotation, BiomeName);
        break;
    case EArch_LandmarkType::CaveEntrance:
        CreateCaveEntranceLandmark(Location, Rotation, BiomeName);
        break;
    }

    // Register the landmark
    FArch_LandmarkData NewLandmark;
    NewLandmark.LandmarkType = LandmarkType;
    NewLandmark.SpawnLocation = Location;
    NewLandmark.SpawnRotation = Rotation;
    NewLandmark.BiomeName = BiomeName;
    NewLandmark.WeatheringLevel = FMath::RandRange(0.3f, 0.8f);
    
    RegisterLandmark(NewLandmark);
}

void AArchitecturalLandmarkManager::SpawnLandmarksInBiome(const FString& BiomeName, int32 Count)
{
    for (int32 i = 0; i < Count; i++)
    {
        FVector RandomLocation = GetRandomLocationInBiome(BiomeName);
        FRotator RandomRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        
        // Randomly select landmark type
        EArch_LandmarkType RandomType = static_cast<EArch_LandmarkType>(FMath::RandRange(0, 4));
        
        SpawnLandmarkAtLocation(RandomType, RandomLocation, RandomRotation, BiomeName);
    }
}

void AArchitecturalLandmarkManager::RegisterLandmark(const FArch_LandmarkData& LandmarkData)
{
    LandmarkRegistry.Add(LandmarkData);
    
    UE_LOG(LogTemp, Log, TEXT("Registered landmark: %s at location %s in biome %s"), 
           *UEnum::GetValueAsString(LandmarkData.LandmarkType),
           *LandmarkData.SpawnLocation.ToString(),
           *LandmarkData.BiomeName);
}

TArray<FArch_LandmarkData> AArchitecturalLandmarkManager::GetLandmarksInBiome(const FString& BiomeName) const
{
    TArray<FArch_LandmarkData> BiomeLandmarks;
    
    for (const FArch_LandmarkData& Landmark : LandmarkRegistry)
    {
        if (Landmark.BiomeName == BiomeName)
        {
            BiomeLandmarks.Add(Landmark);
        }
    }
    
    return BiomeLandmarks;
}

void AArchitecturalLandmarkManager::GenerateLandmarksForAllBiomes()
{
    // Biome coordinates from memory
    TMap<FString, FVector> BiomeLocations;
    BiomeLocations.Add(TEXT("Savana"), FVector(0.0f, 0.0f, 100.0f));
    BiomeLocations.Add(TEXT("Pantano"), FVector(-50000.0f, -45000.0f, 100.0f));
    BiomeLocations.Add(TEXT("Floresta"), FVector(-45000.0f, 40000.0f, 100.0f));
    BiomeLocations.Add(TEXT("Deserto"), FVector(55000.0f, 0.0f, 100.0f));
    BiomeLocations.Add(TEXT("Montanha"), FVector(40000.0f, 50000.0f, 200.0f));

    for (const auto& BiomePair : BiomeLocations)
    {
        SpawnLandmarksInBiome(BiomePair.Key, MaxLandmarksPerBiome);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Generated landmarks for all biomes. Total landmarks: %d"), LandmarkRegistry.Num());
}

void AArchitecturalLandmarkManager::ApplyWeatheringToLandmark(AActor* LandmarkActor, float WeatheringLevel)
{
    if (!LandmarkActor)
    {
        return;
    }

    TArray<UStaticMeshComponent*> MeshComponents;
    LandmarkActor->GetComponents<UStaticMeshComponent>(MeshComponents);

    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp)
        {
            UMaterialInterface* WeatheredMaterial = GetWeatheredMaterial(WeatheringLevel);
            if (WeatheredMaterial)
            {
                MeshComp->SetMaterial(0, WeatheredMaterial);
            }
        }
    }
}

void AArchitecturalLandmarkManager::CreateStoneArchLandmark(FVector Location, FRotator Rotation, const FString& BiomeName)
{
    UStaticMesh* ArchMesh = GetLandmarkMesh(EArch_LandmarkType::StoneArch);
    if (!ArchMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("No mesh found for Stone Arch landmark"));
        return;
    }

    AActor* ArchActor = GetWorld()->SpawnActor<AActor>(Location, Rotation);
    if (ArchActor)
    {
        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(ArchActor);
        MeshComp->SetStaticMesh(ArchMesh);
        ArchActor->SetRootComponent(MeshComp);
        ArchActor->SetActorLabel(FString::Printf(TEXT("StoneArch_%s_%d"), *BiomeName, FMath::RandRange(1, 999)));
        
        ApplyWeatheringToLandmark(ArchActor, FMath::RandRange(0.4f, 0.7f));
    }
}

void AArchitecturalLandmarkManager::CreateStoneCircleLandmark(FVector Location, FRotator Rotation, const FString& BiomeName)
{
    UStaticMesh* PillarMesh = GetLandmarkMesh(EArch_LandmarkType::MegalithPillar);
    if (!PillarMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("No mesh found for Stone Circle landmark"));
        return;
    }

    // Create circle of stone pillars
    float CircleRadius = 800.0f;
    int32 PillarCount = 8;
    
    for (int32 i = 0; i < PillarCount; i++)
    {
        float Angle = (360.0f / PillarCount) * i;
        FVector PillarOffset = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * CircleRadius,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * CircleRadius,
            0.0f
        );
        
        FVector PillarLocation = Location + PillarOffset;
        
        AActor* PillarActor = GetWorld()->SpawnActor<AActor>(PillarLocation, Rotation);
        if (PillarActor)
        {
            UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(PillarActor);
            MeshComp->SetStaticMesh(PillarMesh);
            PillarActor->SetRootComponent(MeshComp);
            PillarActor->SetActorLabel(FString::Printf(TEXT("StoneCircle_%s_Pillar_%d"), *BiomeName, i));
            
            ApplyWeatheringToLandmark(PillarActor, FMath::RandRange(0.5f, 0.8f));
        }
    }
}

void AArchitecturalLandmarkManager::CreateMegalithPillarLandmark(FVector Location, FRotator Rotation, const FString& BiomeName)
{
    UStaticMesh* PillarMesh = GetLandmarkMesh(EArch_LandmarkType::MegalithPillar);
    if (!PillarMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("No mesh found for Megalith Pillar landmark"));
        return;
    }

    AActor* PillarActor = GetWorld()->SpawnActor<AActor>(Location, Rotation);
    if (PillarActor)
    {
        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(PillarActor);
        MeshComp->SetStaticMesh(PillarMesh);
        PillarActor->SetRootComponent(MeshComp);
        PillarActor->SetActorLabel(FString::Printf(TEXT("MegalithPillar_%s_%d"), *BiomeName, FMath::RandRange(1, 999)));
        
        ApplyWeatheringToLandmark(PillarActor, FMath::RandRange(0.3f, 0.6f));
    }
}

void AArchitecturalLandmarkManager::CreateRuinedWallLandmark(FVector Location, FRotator Rotation, const FString& BiomeName)
{
    UStaticMesh* WallMesh = GetLandmarkMesh(EArch_LandmarkType::RuinedWall);
    if (!WallMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("No mesh found for Ruined Wall landmark"));
        return;
    }

    AActor* WallActor = GetWorld()->SpawnActor<AActor>(Location, Rotation);
    if (WallActor)
    {
        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(WallActor);
        MeshComp->SetStaticMesh(WallMesh);
        WallActor->SetRootComponent(MeshComp);
        WallActor->SetActorLabel(FString::Printf(TEXT("RuinedWall_%s_%d"), *BiomeName, FMath::RandRange(1, 999)));
        
        ApplyWeatheringToLandmark(WallActor, FMath::RandRange(0.6f, 0.9f));
    }
}

void AArchitecturalLandmarkManager::CreateCaveEntranceLandmark(FVector Location, FRotator Rotation, const FString& BiomeName)
{
    UStaticMesh* CaveMesh = GetLandmarkMesh(EArch_LandmarkType::CaveEntrance);
    if (!CaveMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("No mesh found for Cave Entrance landmark"));
        return;
    }

    AActor* CaveActor = GetWorld()->SpawnActor<AActor>(Location, Rotation);
    if (CaveActor)
    {
        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(CaveActor);
        MeshComp->SetStaticMesh(CaveMesh);
        CaveActor->SetRootComponent(MeshComp);
        CaveActor->SetActorLabel(FString::Printf(TEXT("CaveEntrance_%s_%d"), *BiomeName, FMath::RandRange(1, 999)));
        
        ApplyWeatheringToLandmark(CaveActor, FMath::RandRange(0.2f, 0.5f));
    }
}

FVector AArchitecturalLandmarkManager::GetRandomLocationInBiome(const FString& BiomeName) const
{
    // Base biome locations with random offset
    FVector BaseLocation = FVector::ZeroVector;
    
    if (BiomeName == TEXT("Savana"))
    {
        BaseLocation = FVector(0.0f, 0.0f, 100.0f);
    }
    else if (BiomeName == TEXT("Pantano"))
    {
        BaseLocation = FVector(-50000.0f, -45000.0f, 100.0f);
    }
    else if (BiomeName == TEXT("Floresta"))
    {
        BaseLocation = FVector(-45000.0f, 40000.0f, 100.0f);
    }
    else if (BiomeName == TEXT("Deserto"))
    {
        BaseLocation = FVector(55000.0f, 0.0f, 100.0f);
    }
    else if (BiomeName == TEXT("Montanha"))
    {
        BaseLocation = FVector(40000.0f, 50000.0f, 200.0f);
    }
    
    // Add random offset within biome bounds
    FVector RandomOffset = FVector(
        FMath::RandRange(-5000.0f, 5000.0f),
        FMath::RandRange(-5000.0f, 5000.0f),
        FMath::RandRange(-100.0f, 200.0f)
    );
    
    return BaseLocation + RandomOffset;
}

UStaticMesh* AArchitecturalLandmarkManager::GetLandmarkMesh(EArch_LandmarkType LandmarkType) const
{
    // Try to load appropriate meshes from available asset packs
    FString MeshPath;
    
    switch (LandmarkType)
    {
    case EArch_LandmarkType::StoneArch:
        MeshPath = TEXT("/Game/LandscapePackOne/Rock_01");
        break;
    case EArch_LandmarkType::StoneCircle:
    case EArch_LandmarkType::MegalithPillar:
        MeshPath = TEXT("/Game/ANGRY_MESH/Rock_01");
        break;
    case EArch_LandmarkType::RuinedWall:
        MeshPath = TEXT("/Game/Desert_Oasis/Rock");
        break;
    case EArch_LandmarkType::CaveEntrance:
        MeshPath = TEXT("/Game/LandscapePackTwo/Rock_02");
        break;
    default:
        MeshPath = TEXT("/Engine/BasicShapes/Cube");
        break;
    }
    
    return LoadObject<UStaticMesh>(nullptr, *MeshPath);
}

UMaterialInterface* AArchitecturalLandmarkManager::GetWeatheredMaterial(float WeatheringLevel) const
{
    if (WeatheringLevel > 0.6f && MossyStoneMaterial)
    {
        return MossyStoneMaterial;
    }
    else if (WeatheredStoneMaterial)
    {
        return WeatheredStoneMaterial;
    }
    
    return nullptr;
}