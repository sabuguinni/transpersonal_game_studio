#include "ArchRuinSpawner.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

AArchRuinSpawner::AArchRuinSpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default ruin configs for a Cretaceous ruin cluster
    FArch_RuinConfig PillarConfig;
    PillarConfig.RuinType = EArch_RuinType::Pillar;
    PillarConfig.SpawnScale = FVector(0.5f, 0.5f, 3.0f);
    PillarConfig.MossIntensity = 0.8f;
    PillarConfig.WeatheringAmount = 0.9f;
    PillarConfig.bHasVines = true;
    RuinConfigs.Add(PillarConfig);

    FArch_RuinConfig WallConfig;
    WallConfig.RuinType = EArch_RuinType::WallSection;
    WallConfig.SpawnScale = FVector(3.0f, 0.3f, 1.5f);
    WallConfig.MossIntensity = 0.6f;
    WallConfig.WeatheringAmount = 0.85f;
    WallConfig.bHasVines = true;
    RuinConfigs.Add(WallConfig);

    FArch_RuinConfig BoulderConfig;
    BoulderConfig.RuinType = EArch_RuinType::Boulder;
    BoulderConfig.SpawnScale = FVector(1.5f, 1.5f, 1.2f);
    BoulderConfig.MossIntensity = 0.95f;
    BoulderConfig.WeatheringAmount = 1.0f;
    BoulderConfig.bHasVines = false;
    RuinConfigs.Add(BoulderConfig);
}

void AArchRuinSpawner::BeginPlay()
{
    Super::BeginPlay();

    if (bScatterRandomly && RuinConfigs.Num() > 0)
    {
        SpawnRuinCluster();
    }
}

void AArchRuinSpawner::SpawnRuinCluster()
{
    UWorld* World = GetWorld();
    if (!World || RuinConfigs.Num() == 0)
    {
        return;
    }

    ClearAllRuins();

    const FVector Origin = GetActorLocation();
    const int32 SpawnCount = FMath::Min(MaxRuinCount, 20);

    for (int32 i = 0; i < SpawnCount; ++i)
    {
        // Pick a random ruin config
        const int32 ConfigIndex = FMath::RandRange(0, RuinConfigs.Num() - 1);
        const FArch_RuinConfig& Config = RuinConfigs[ConfigIndex];

        // Random position within radius
        const float Angle = FMath::FRandRange(0.0f, 360.0f);
        const float Distance = FMath::FRandRange(50.0f, SpawnRadius);
        const FVector Offset(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            0.0f
        );

        FVector SpawnLocation = Origin + Offset;

        // Trace to ground
        FHitResult HitResult;
        FVector TraceStart = SpawnLocation + FVector(0.0f, 0.0f, 1000.0f);
        FVector TraceEnd = SpawnLocation - FVector(0.0f, 0.0f, 1000.0f);
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);

        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
        {
            SpawnLocation = HitResult.Location;
        }

        FRotator SpawnRotation(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AStaticMeshActor* RuinActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            SpawnLocation,
            SpawnRotation,
            SpawnParams
        );

        if (RuinActor)
        {
            RuinActor->SetActorScale3D(Config.SpawnScale);

            // Label by type for easy identification
            FString TypeName;
            switch (Config.RuinType)
            {
                case EArch_RuinType::Pillar:      TypeName = TEXT("Pillar"); break;
                case EArch_RuinType::WallSection: TypeName = TEXT("Wall"); break;
                case EArch_RuinType::Archway:     TypeName = TEXT("Arch"); break;
                case EArch_RuinType::Boulder:     TypeName = TEXT("Boulder"); break;
                case EArch_RuinType::Foundation:  TypeName = TEXT("Foundation"); break;
                default:                          TypeName = TEXT("Ruin"); break;
            }
            RuinActor->SetActorLabel(FString::Printf(TEXT("Ruin_%s_%02d"), *TypeName, i));

            SpawnedRuins.Add(RuinActor);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AArchRuinSpawner: Spawned %d ruin actors"), SpawnedRuins.Num());
}

void AArchRuinSpawner::ClearAllRuins()
{
    for (AActor* Ruin : SpawnedRuins)
    {
        if (IsValid(Ruin))
        {
            Ruin->Destroy();
        }
    }
    SpawnedRuins.Empty();
}

int32 AArchRuinSpawner::GetActiveRuinCount() const
{
    int32 Count = 0;
    for (const AActor* Ruin : SpawnedRuins)
    {
        if (IsValid(Ruin))
        {
            ++Count;
        }
    }
    return Count;
}
