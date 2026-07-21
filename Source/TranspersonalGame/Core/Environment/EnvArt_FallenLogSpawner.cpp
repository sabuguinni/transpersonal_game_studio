#include "EnvArt_FallenLogSpawner.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/CollisionProfile.h"

UEnvArt_FallenLogSpawner::UEnvArt_FallenLogSpawner()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;

    SpawnRadius = 2000.0f;
    MaxLogsPerCluster = 8;
    MinDistanceBetweenLogs = 300.0f;
    bAlignToTerrain = true;
    bRandomRotation = true;
    CurrentBiome = TEXT("Forest");

    InitializeLogVariations();
}

void UEnvArt_FallenLogSpawner::BeginPlay()
{
    Super::BeginPlay();
    
    LoadLogAssets();
    
    // Auto-spawn initial log clusters based on owner location
    if (AActor* Owner = GetOwner())
    {
        FVector OwnerLocation = Owner->GetActorLocation();
        SpawnLogCluster(OwnerLocation, CurrentBiome);
    }
}

void UEnvArt_FallenLogSpawner::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateLogDecay(DeltaTime);
}

void UEnvArt_FallenLogSpawner::InitializeLogVariations()
{
    AvailableLogTypes.Empty();

    // Small fallen branch
    FEnvArt_LogVariation SmallBranch;
    SmallBranch.VariationName = TEXT("SmallBranch");
    SmallBranch.Length = 200.0f;
    SmallBranch.Diameter = 15.0f;
    SmallBranch.DecayLevel = 0.3f;
    SmallBranch.bHasMoss = false;
    SmallBranch.bHasLichen = true;
    SmallBranch.BiomeCompatibility = {TEXT("Forest"), TEXT("Plains"), TEXT("Mountain")};
    AvailableLogTypes.Add(SmallBranch);

    // Medium fallen log
    FEnvArt_LogVariation MediumLog;
    MediumLog.VariationName = TEXT("MediumLog");
    MediumLog.Length = 400.0f;
    MediumLog.Diameter = 40.0f;
    MediumLog.DecayLevel = 0.5f;
    MediumLog.bHasMoss = true;
    MediumLog.bHasLichen = true;
    MediumLog.BiomeCompatibility = {TEXT("Forest"), TEXT("Swamp")};
    AvailableLogTypes.Add(MediumLog);

    // Large fallen trunk
    FEnvArt_LogVariation LargeTrunk;
    LargeTrunk.VariationName = TEXT("LargeTrunk");
    LargeTrunk.Length = 800.0f;
    LargeTrunk.Diameter = 80.0f;
    LargeTrunk.DecayLevel = 0.7f;
    LargeTrunk.bHasMoss = true;
    LargeTrunk.bHasLichen = false;
    LargeTrunk.BiomeCompatibility = {TEXT("Forest"), TEXT("Swamp"), TEXT("Mountain")};
    AvailableLogTypes.Add(LargeTrunk);

    // Ancient fallen giant
    FEnvArt_LogVariation AncientGiant;
    AncientGiant.VariationName = TEXT("AncientGiant");
    AncientGiant.Length = 1200.0f;
    AncientGiant.Diameter = 120.0f;
    AncientGiant.DecayLevel = 0.9f;
    AncientGiant.bHasMoss = true;
    AncientGiant.bHasLichen = true;
    AncientGiant.BiomeCompatibility = {TEXT("Forest"), TEXT("Swamp")};
    AvailableLogTypes.Add(AncientGiant);
}

void UEnvArt_FallenLogSpawner::SpawnLogCluster(const FVector& Location, const FString& BiomeType)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("EnvArt_FallenLogSpawner: No valid world for spawning"));
        return;
    }

    FEnvArt_LogCluster NewCluster;
    NewCluster.CenterLocation = Location;
    NewCluster.ClusterRadius = SpawnRadius;
    NewCluster.LogCount = FMath::RandRange(3, MaxLogsPerCluster);

    for (int32 i = 0; i < NewCluster.LogCount; i++)
    {
        FVector LogLocation = CalculateLogPlacement(Location, i, NewCluster.LogCount);
        
        if (IsValidSpawnLocation(LogLocation))
        {
            FEnvArt_LogVariation SelectedLog = GetRandomLogForBiome(BiomeType);
            SpawnSingleLog(LogLocation, SelectedLog);
            NewCluster.LogVariations.Add(SelectedLog);
        }
    }

    SpawnedClusters.Add(NewCluster);
    
    UE_LOG(LogTemp, Log, TEXT("EnvArt_FallenLogSpawner: Spawned log cluster with %d logs at %s"), 
           NewCluster.LogCount, *Location.ToString());
}

void UEnvArt_FallenLogSpawner::SpawnSingleLog(const FVector& Location, const FEnvArt_LogVariation& LogType)
{
    if (!GetWorld() || !DefaultLogMesh)
    {
        return;
    }

    FVector SpawnLocation = bAlignToTerrain ? GetTerrainAlignedPosition(Location) : Location;
    FRotator SpawnRotation = CalculateLogRotation(SpawnLocation, bAlignToTerrain);

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AStaticMeshActor* LogActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), 
                                                                         SpawnLocation, SpawnRotation, SpawnParams);
    
    if (LogActor)
    {
        UStaticMeshComponent* MeshComp = LogActor->GetStaticMeshComponent();
        if (MeshComp)
        {
            MeshComp->SetStaticMesh(DefaultLogMesh);
            
            // Scale based on log variation
            FVector Scale = FVector(LogType.Length / 400.0f, LogType.Diameter / 40.0f, LogType.Diameter / 40.0f);
            LogActor->SetActorScale3D(Scale);
            
            ApplyLogMaterial(MeshComp, LogType);
            
            // Set collision for gameplay interaction
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
        }

        LogActor->SetActorLabel(FString::Printf(TEXT("FallenLog_%s"), *LogType.VariationName));
        SpawnedLogActors.Add(LogActor);
    }
}

void UEnvArt_FallenLogSpawner::ClearAllLogs()
{
    for (AActor* LogActor : SpawnedLogActors)
    {
        if (IsValid(LogActor))
        {
            LogActor->Destroy();
        }
    }
    
    SpawnedLogActors.Empty();
    SpawnedClusters.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("EnvArt_FallenLogSpawner: Cleared all spawned logs"));
}

FEnvArt_LogVariation UEnvArt_FallenLogSpawner::GetRandomLogForBiome(const FString& BiomeType)
{
    TArray<FEnvArt_LogVariation> CompatibleLogs;
    
    for (const FEnvArt_LogVariation& LogType : AvailableLogTypes)
    {
        if (LogType.BiomeCompatibility.Contains(BiomeType))
        {
            CompatibleLogs.Add(LogType);
        }
    }
    
    if (CompatibleLogs.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, CompatibleLogs.Num() - 1);
        return CompatibleLogs[RandomIndex];
    }
    
    // Fallback to first available log type
    return AvailableLogTypes.Num() > 0 ? AvailableLogTypes[0] : FEnvArt_LogVariation();
}

bool UEnvArt_FallenLogSpawner::IsValidSpawnLocation(const FVector& Location)
{
    if (!GetWorld())
    {
        return false;
    }

    // Check minimum distance from existing logs
    for (AActor* ExistingLog : SpawnedLogActors)
    {
        if (IsValid(ExistingLog))
        {
            float Distance = FVector::Dist(Location, ExistingLog->GetActorLocation());
            if (Distance < MinDistanceBetweenLogs)
            {
                return false;
            }
        }
    }

    // Perform ground trace to ensure valid terrain
    FHitResult HitResult;
    FVector TraceStart = Location + FVector(0, 0, 500);
    FVector TraceEnd = Location - FVector(0, 0, 1000);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, 
                                                    ECollisionChannel::ECC_WorldStatic, QueryParams);
    
    return bHit && HitResult.bBlockingHit;
}

FVector UEnvArt_FallenLogSpawner::GetTerrainAlignedPosition(const FVector& BaseLocation)
{
    if (!GetWorld())
    {
        return BaseLocation;
    }

    FHitResult HitResult;
    FVector TraceStart = BaseLocation + FVector(0, 0, 1000);
    FVector TraceEnd = BaseLocation - FVector(0, 0, 1000);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, 
                                                    ECollisionChannel::ECC_WorldStatic, QueryParams);
    
    if (bHit && HitResult.bBlockingHit)
    {
        return HitResult.Location + FVector(0, 0, 10); // Slight offset above ground
    }
    
    return BaseLocation;
}

void UEnvArt_FallenLogSpawner::UpdateLogDecay(float DeltaTime)
{
    // Placeholder for future decay system
    // Could gradually change materials, add more moss, etc.
}

void UEnvArt_FallenLogSpawner::LoadLogAssets()
{
    // Load default cylinder mesh as placeholder
    DefaultLogMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
    
    // Load basic materials
    LogMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    MossyLogMaterial = LogMaterial; // Placeholder - would be custom mossy material
    LichenLogMaterial = LogMaterial; // Placeholder - would be custom lichen material
    
    if (!DefaultLogMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("EnvArt_FallenLogSpawner: Failed to load default log mesh"));
    }
}

FVector UEnvArt_FallenLogSpawner::CalculateLogPlacement(const FVector& ClusterCenter, int32 LogIndex, int32 TotalLogs)
{
    float AngleStep = 360.0f / TotalLogs;
    float CurrentAngle = AngleStep * LogIndex;
    
    // Add some randomness to avoid perfect circles
    CurrentAngle += FMath::RandRange(-30.0f, 30.0f);
    float Distance = FMath::RandRange(SpawnRadius * 0.3f, SpawnRadius);
    
    float RadianAngle = FMath::DegreesToRadians(CurrentAngle);
    FVector Offset = FVector(
        FMath::Cos(RadianAngle) * Distance,
        FMath::Sin(RadianAngle) * Distance,
        0
    );
    
    return ClusterCenter + Offset;
}

FRotator UEnvArt_FallenLogSpawner::CalculateLogRotation(const FVector& Location, bool bAlignToSlope)
{
    FRotator BaseRotation = FRotator::ZeroRotator;
    
    if (bRandomRotation)
    {
        BaseRotation.Yaw = FMath::RandRange(0.0f, 360.0f);
        BaseRotation.Roll = FMath::RandRange(-15.0f, 15.0f);
    }
    
    if (bAlignToSlope && GetWorld())
    {
        // Sample terrain normal for slope alignment
        FHitResult HitResult;
        FVector TraceStart = Location + FVector(0, 0, 100);
        FVector TraceEnd = Location - FVector(0, 0, 200);
        
        FCollisionQueryParams QueryParams;
        QueryParams.bTraceComplex = true;
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, 
                                                ECollisionChannel::ECC_WorldStatic, QueryParams))
        {
            FVector SurfaceNormal = HitResult.Normal;
            FRotator SlopeRotation = FRotationMatrix::MakeFromZ(SurfaceNormal).Rotator();
            BaseRotation.Pitch += SlopeRotation.Pitch * 0.5f; // Partial alignment
        }
    }
    
    return BaseRotation;
}

void UEnvArt_FallenLogSpawner::ApplyLogMaterial(UStaticMeshComponent* MeshComp, const FEnvArt_LogVariation& LogType)
{
    if (!MeshComp)
    {
        return;
    }
    
    UMaterialInterface* MaterialToApply = LogMaterial;
    
    if (LogType.bHasMoss && MossyLogMaterial)
    {
        MaterialToApply = MossyLogMaterial;
    }
    else if (LogType.bHasLichen && LichenLogMaterial)
    {
        MaterialToApply = LichenLogMaterial;
    }
    
    if (MaterialToApply)
    {
        MeshComp->SetMaterial(0, MaterialToApply);
    }
}