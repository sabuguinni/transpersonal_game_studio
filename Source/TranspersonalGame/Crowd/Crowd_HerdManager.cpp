#include "Crowd_HerdManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"

ACrowd_HerdManager::ACrowd_HerdManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create herd bounds sphere
    HerdBounds = CreateDefaultSubobject<USphereComponent>(TEXT("HerdBounds"));
    HerdBounds->SetupAttachment(RootComponent);
    HerdBounds->SetSphereRadius(2000.0f);
    HerdBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create visual marker
    HerdMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HerdMarker"));
    HerdMarker->SetupAttachment(RootComponent);
    HerdMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize default values
    HerdUpdateInterval = 5.0f;
    MaxHerdsPerBiome = 3;
    HerdSpawnRadius = 1500.0f;
    LastUpdateTime = 0.0f;

    // Set actor label
    SetActorLabel(TEXT("HerdManager"));
}

void ACrowd_HerdManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("HerdManager: BeginPlay - Initializing biome herds"));
    InitializeBiomeHerds();
}

void ACrowd_HerdManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= HerdUpdateInterval)
    {
        UpdateHerdPositions();
        LastUpdateTime = 0.0f;
    }
}

void ACrowd_HerdManager::CreateHerd(const FString& HerdID, const FVector& Location, const FString& Species, int32 Count)
{
    // Check if herd already exists
    for (const FCrowd_HerdData& ExistingHerd : ActiveHerds)
    {
        if (ExistingHerd.HerdID == HerdID)
        {
            UE_LOG(LogTemp, Warning, TEXT("HerdManager: Herd %s already exists"), *HerdID);
            return;
        }
    }

    // Create new herd
    FCrowd_HerdData NewHerd;
    NewHerd.HerdID = HerdID;
    NewHerd.CenterLocation = Location;
    NewHerd.SpeciesType = Species;
    NewHerd.MemberCount = Count;
    NewHerd.HerdRadius = HerdSpawnRadius;

    ActiveHerds.Add(NewHerd);

    UE_LOG(LogTemp, Warning, TEXT("HerdManager: Created herd %s at location (%f, %f, %f) with %d %s"), 
           *HerdID, Location.X, Location.Y, Location.Z, Count, *Species);

    // Spawn initial members
    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLocation = GetRandomLocationInHerd(NewHerd);
        SpawnHerdMember(HerdID, SpawnLocation);
    }
}

void ACrowd_HerdManager::UpdateHerdPositions()
{
    for (FCrowd_HerdData& HerdData : ActiveHerds)
    {
        UpdateHerdBehavior(HerdData);
    }
}

void ACrowd_HerdManager::SpawnHerdMember(const FString& HerdID, const FVector& Location)
{
    // Find the herd
    FCrowd_HerdData* TargetHerd = nullptr;
    for (FCrowd_HerdData& HerdData : ActiveHerds)
    {
        if (HerdData.HerdID == HerdID)
        {
            TargetHerd = &HerdData;
            break;
        }
    }

    if (!TargetHerd)
    {
        UE_LOG(LogTemp, Error, TEXT("HerdManager: Cannot spawn member - Herd %s not found"), *HerdID);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("HerdManager: Spawning %s member at (%f, %f, %f)"), 
           *TargetHerd->SpeciesType, Location.X, Location.Y, Location.Z);
}

FCrowd_HerdData ACrowd_HerdManager::GetHerdData(const FString& HerdID)
{
    for (const FCrowd_HerdData& HerdData : ActiveHerds)
    {
        if (HerdData.HerdID == HerdID)
        {
            return HerdData;
        }
    }

    // Return default if not found
    return FCrowd_HerdData();
}

void ACrowd_HerdManager::RemoveHerd(const FString& HerdID)
{
    for (int32 i = ActiveHerds.Num() - 1; i >= 0; i--)
    {
        if (ActiveHerds[i].HerdID == HerdID)
        {
            UE_LOG(LogTemp, Warning, TEXT("HerdManager: Removing herd %s"), *HerdID);
            ActiveHerds.RemoveAt(i);
            break;
        }
    }
}

void ACrowd_HerdManager::InitializeBiomeHerds()
{
    // Biome coordinates from memory
    TArray<TPair<FString, FVector>> BiomeLocations = {
        {TEXT("Savana"), FVector(0, 0, 100)},
        {TEXT("Pantano"), FVector(-50000, -45000, 100)},
        {TEXT("Floresta"), FVector(-45000, 40000, 100)},
        {TEXT("Deserto"), FVector(55000, 0, 100)},
        {TEXT("Montanha"), FVector(40000, 50000, 100)}
    };

    // Species distribution per biome
    TMap<FString, TArray<FString>> BiomeSpecies;
    BiomeSpecies.Add(TEXT("Savana"), {TEXT("Triceratops"), TEXT("Parasaurolophus")});
    BiomeSpecies.Add(TEXT("Pantano"), {TEXT("Ankylosaurus"), TEXT("Protoceratops")});
    BiomeSpecies.Add(TEXT("Floresta"), {TEXT("Brachiosaurus"), TEXT("Pachycephalo")});
    BiomeSpecies.Add(TEXT("Deserto"), {TEXT("Tsintaosaurus"), TEXT("Triceratops")});
    BiomeSpecies.Add(TEXT("Montanha"), {TEXT("Ankylosaurus"), TEXT("Parasaurolophus")});

    // Create herds for each biome
    for (const auto& BiomePair : BiomeLocations)
    {
        const FString& BiomeName = BiomePair.Key;
        const FVector& BiomeCenter = BiomePair.Value;

        if (BiomeSpecies.Contains(BiomeName))
        {
            const TArray<FString>& SpeciesList = BiomeSpecies[BiomeName];
            
            for (int32 HerdIndex = 0; HerdIndex < MaxHerdsPerBiome && HerdIndex < SpeciesList.Num(); HerdIndex++)
            {
                FString HerdID = FString::Printf(TEXT("%s_Herd_%d"), *BiomeName, HerdIndex + 1);
                FVector HerdLocation = BiomeCenter + FVector(
                    FMath::RandRange(-10000, 10000),
                    FMath::RandRange(-10000, 10000),
                    0
                );
                
                int32 HerdSize = FMath::RandRange(5, 12);
                CreateHerd(HerdID, HerdLocation, SpeciesList[HerdIndex], HerdSize);
            }
        }
    }
}

void ACrowd_HerdManager::UpdateHerdBehavior(FCrowd_HerdData& HerdData)
{
    // Simple herd movement - slowly drift in random direction
    FVector RandomOffset = FVector(
        FMath::RandRange(-100.0f, 100.0f),
        FMath::RandRange(-100.0f, 100.0f),
        0.0f
    );
    
    HerdData.CenterLocation += RandomOffset;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("HerdManager: Updated herd %s position to (%f, %f, %f)"), 
           *HerdData.HerdID, HerdData.CenterLocation.X, HerdData.CenterLocation.Y, HerdData.CenterLocation.Z);
}

FVector ACrowd_HerdManager::GetRandomLocationInHerd(const FCrowd_HerdData& HerdData)
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(0.0f, HerdData.HerdRadius);
    
    FVector RandomOffset = FVector(
        FMath::Cos(RandomAngle) * RandomRadius,
        FMath::Sin(RandomAngle) * RandomRadius,
        0.0f
    );
    
    return HerdData.CenterLocation + RandomOffset;
}