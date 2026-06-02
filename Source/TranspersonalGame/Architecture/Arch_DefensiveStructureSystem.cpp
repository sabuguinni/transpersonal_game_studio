#include "Arch_DefensiveStructureSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UArch_DefensiveStructureSystem::UArch_DefensiveStructureSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;

    // Initialize default configuration
    DefensiveConfig = FArch_DefensiveStructureConfig();
    ThreatScanInterval = 2.0f;
    MaintenanceCheckInterval = 30.0f;
    bAutoRepairEnabled = false;
    AutoRepairRate = 1.0f;

    // Initialize runtime data
    DefensiveStructures.Empty();
    StructureStatuses.Empty();
    CurrentThreats.Empty();
    LastThreatScan = 0.0f;
    LastMaintenanceCheck = 0.0f;
    bSystemInitialized = false;
}

void UArch_DefensiveStructureSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefensiveSystem();
}

void UArch_DefensiveStructureSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bSystemInitialized)
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Periodic threat scanning
    if (CurrentTime - LastThreatScan >= ThreatScanInterval)
    {
        ScanForThreats();
        LastThreatScan = CurrentTime;
    }

    // Periodic maintenance checks
    if (CurrentTime - LastMaintenanceCheck >= MaintenanceCheckInterval)
    {
        PerformMaintenanceCheck();
        LastMaintenanceCheck = CurrentTime;
    }

    // Auto-repair if enabled
    if (bAutoRepairEnabled && AutoRepairRate > 0.0f)
    {
        for (int32 i = 0; i < StructureStatuses.Num(); i++)
        {
            if (StructureStatuses[i].bNeedsRepair && StructureStatuses[i].CurrentIntegrity < 100.0f)
            {
                RepairStructure(i, AutoRepairRate * DeltaTime);
            }
        }
    }
}

void UArch_DefensiveStructureSystem::InitializeDefensiveSystem()
{
    if (bSystemInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Initializing Defensive Structure System"));

    // Clear existing data
    DefensiveStructures.Empty();
    StructureStatuses.Empty();
    CurrentThreats.Empty();

    // Reset timers
    LastThreatScan = GetWorld()->GetTimeSeconds();
    LastMaintenanceCheck = GetWorld()->GetTimeSeconds();

    bSystemInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("Defensive Structure System initialized successfully"));
}

bool UArch_DefensiveStructureSystem::CreateDefensiveStructure(EArch_DefensiveStructureType StructureType, const FVector& Location, const FRotator& Rotation)
{
    if (!bSystemInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create defensive structure - system not initialized"));
        return false;
    }

    if (!IsValidDefensiveLocation(Location))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid location for defensive structure"));
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("Creating defensive structure of type %d at location %s"), 
           static_cast<int32>(StructureType), *Location.ToString());

    switch (StructureType)
    {
        case EArch_DefensiveStructureType::Watchtower:
            CreateWatchtower(Location, Rotation);
            break;
        case EArch_DefensiveStructureType::Barricade:
            CreateBarricade(Location, Rotation);
            break;
        case EArch_DefensiveStructureType::PitTrap:
            CreatePitTrap(Location, Rotation);
            break;
        case EArch_DefensiveStructureType::RockWall:
            CreateRockWall(Location, Rotation);
            break;
        case EArch_DefensiveStructureType::SpikePalisade:
            CreateSpikePalisade(Location, Rotation);
            break;
        case EArch_DefensiveStructureType::HidingSpot:
            CreateHidingSpot(Location, Rotation);
            break;
        case EArch_DefensiveStructureType::EscapeRoute:
            CreateEscapeRoute(Location, Rotation);
            break;
        case EArch_DefensiveStructureType::LookoutPost:
            CreateLookoutPost(Location, Rotation);
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("Unknown defensive structure type"));
            return false;
    }

    // Initialize structure status
    FArch_DefensivePositionStatus NewStatus;
    NewStatus.CurrentIntegrity = DefensiveConfig.StructuralIntegrity;
    NewStatus.LastMaintenanceTime = GetWorld()->GetTimeSeconds();
    StructureStatuses.Add(NewStatus);

    return true;
}

void UArch_DefensiveStructureSystem::CreateWatchtower(const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Create watchtower actor
    AStaticMeshActor* WatchtowerActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (WatchtowerActor)
    {
        WatchtowerActor->SetActorLabel(TEXT("DefensiveWatchtower"));
        
        // Add collision component for interaction
        UBoxComponent* CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WatchtowerCollision"));
        if (CollisionBox)
        {
            CollisionBox->SetBoxExtent(DefensiveConfig.Dimensions);
            WatchtowerActor->SetRootComponent(CollisionBox);
        }

        DefensiveStructures.Add(WatchtowerActor);
        UE_LOG(LogTemp, Log, TEXT("Created watchtower at %s"), *Location.ToString());
    }
}

void UArch_DefensiveStructureSystem::CreateBarricade(const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    AStaticMeshActor* BarricadeActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (BarricadeActor)
    {
        BarricadeActor->SetActorLabel(TEXT("DefensiveBarricade"));
        DefensiveStructures.Add(BarricadeActor);
        UE_LOG(LogTemp, Log, TEXT("Created barricade at %s"), *Location.ToString());
    }
}

void UArch_DefensiveStructureSystem::CreatePitTrap(const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    AStaticMeshActor* PitTrapActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (PitTrapActor)
    {
        PitTrapActor->SetActorLabel(TEXT("DefensivePitTrap"));
        DefensiveStructures.Add(PitTrapActor);
        UE_LOG(LogTemp, Log, TEXT("Created pit trap at %s"), *Location.ToString());
    }
}

void UArch_DefensiveStructureSystem::CreateRockWall(const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    AStaticMeshActor* RockWallActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (RockWallActor)
    {
        RockWallActor->SetActorLabel(TEXT("DefensiveRockWall"));
        DefensiveStructures.Add(RockWallActor);
        UE_LOG(LogTemp, Log, TEXT("Created rock wall at %s"), *Location.ToString());
    }
}

void UArch_DefensiveStructureSystem::CreateSpikePalisade(const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    AStaticMeshActor* PalisadeActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (PalisadeActor)
    {
        PalisadeActor->SetActorLabel(TEXT("DefensiveSpikePalisade"));
        DefensiveStructures.Add(PalisadeActor);
        UE_LOG(LogTemp, Log, TEXT("Created spike palisade at %s"), *Location.ToString());
    }
}

void UArch_DefensiveStructureSystem::CreateHidingSpot(const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    AStaticMeshActor* HidingSpotActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (HidingSpotActor)
    {
        HidingSpotActor->SetActorLabel(TEXT("DefensiveHidingSpot"));
        DefensiveStructures.Add(HidingSpotActor);
        UE_LOG(LogTemp, Log, TEXT("Created hiding spot at %s"), *Location.ToString());
    }
}

void UArch_DefensiveStructureSystem::CreateEscapeRoute(const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    AStaticMeshActor* EscapeRouteActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (EscapeRouteActor)
    {
        EscapeRouteActor->SetActorLabel(TEXT("DefensiveEscapeRoute"));
        DefensiveStructures.Add(EscapeRouteActor);
        UE_LOG(LogTemp, Log, TEXT("Created escape route at %s"), *Location.ToString());
    }
}

void UArch_DefensiveStructureSystem::CreateLookoutPost(const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    AStaticMeshActor* LookoutPostActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
    if (LookoutPostActor)
    {
        LookoutPostActor->SetActorLabel(TEXT("DefensiveLookoutPost"));
        DefensiveStructures.Add(LookoutPostActor);
        UE_LOG(LogTemp, Log, TEXT("Created lookout post at %s"), *Location.ToString());
    }
}

void UArch_DefensiveStructureSystem::ScanForThreats()
{
    if (!bSystemInitialized)
    {
        return;
    }

    CurrentThreats.Empty();

    for (AActor* Structure : DefensiveStructures)
    {
        if (Structure && IsValid(Structure))
        {
            DetectThreatsInRange(Structure, DefensiveConfig.VisibilityRange);
        }
    }

    UpdateThreatLevels();
    ProcessThreatAlerts();
}

void UArch_DefensiveStructureSystem::DetectThreatsInRange(AActor* DefensiveStructure, float Range)
{
    if (!DefensiveStructure || !IsValid(DefensiveStructure))
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FVector StructureLocation = DefensiveStructure->GetActorLocation();
    
    // Get all actors in range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (!Actor || !IsValid(Actor) || Actor == DefensiveStructure)
        {
            continue;
        }

        float Distance = FVector::Dist(StructureLocation, Actor->GetActorLocation());
        if (Distance <= Range)
        {
            // Check if this actor represents a threat
            FString ActorName = Actor->GetName().ToLower();
            bool bIsThreat = ActorName.Contains(TEXT("trex")) || 
                           ActorName.Contains(TEXT("raptor")) || 
                           ActorName.Contains(TEXT("carno")) ||
                           ActorName.Contains(TEXT("dino"));

            if (bIsThreat)
            {
                FArch_ThreatDetectionData ThreatData;
                ThreatData.ThreatActor = Actor;
                ThreatData.ThreatLocation = Actor->GetActorLocation();
                ThreatData.ThreatDistance = Distance;
                ThreatData.ThreatLevel = FMath::Clamp(1.0f - (Distance / Range), 0.1f, 1.0f);
                ThreatData.DetectionTime = GetWorld()->GetTimeSeconds();
                ThreatData.bIsHostile = true;
                ThreatData.bRequiresAlert = ThreatData.ThreatLevel > 0.7f;

                CurrentThreats.Add(ThreatData);
            }
        }
    }
}

void UArch_DefensiveStructureSystem::UpdateThreatLevels()
{
    for (FArch_ThreatDetectionData& Threat : CurrentThreats)
    {
        if (Threat.ThreatActor && IsValid(Threat.ThreatActor))
        {
            // Update threat location and distance
            Threat.ThreatLocation = Threat.ThreatActor->GetActorLocation();
            
            // Find nearest defensive structure to this threat
            float MinDistance = FLT_MAX;
            for (AActor* Structure : DefensiveStructures)
            {
                if (Structure && IsValid(Structure))
                {
                    float Distance = FVector::Dist(Structure->GetActorLocation(), Threat.ThreatLocation);
                    if (Distance < MinDistance)
                    {
                        MinDistance = Distance;
                    }
                }
            }
            
            Threat.ThreatDistance = MinDistance;
            Threat.ThreatLevel = FMath::Clamp(1.0f - (MinDistance / DefensiveConfig.VisibilityRange), 0.1f, 1.0f);
            Threat.bRequiresAlert = Threat.ThreatLevel > 0.7f;
        }
    }
}

void UArch_DefensiveStructureSystem::ProcessThreatAlerts()
{
    for (const FArch_ThreatDetectionData& Threat : CurrentThreats)
    {
        if (Threat.bRequiresAlert)
        {
            UE_LOG(LogTemp, Warning, TEXT("HIGH THREAT DETECTED: %s at distance %.2f"), 
                   Threat.ThreatActor ? *Threat.ThreatActor->GetName() : TEXT("Unknown"), 
                   Threat.ThreatDistance);
        }
    }
}

bool UArch_DefensiveStructureSystem::IsValidDefensiveLocation(const FVector& Location) const
{
    // Check if location is too close to existing structures
    for (AActor* Structure : DefensiveStructures)
    {
        if (Structure && IsValid(Structure))
        {
            float Distance = FVector::Dist(Structure->GetActorLocation(), Location);
            if (Distance < 500.0f) // Minimum distance between structures
            {
                return false;
            }
        }
    }

    return true;
}

float UArch_DefensiveStructureSystem::CalculateDefensiveValue(const FVector& Location) const
{
    float DefensiveValue = 1.0f;

    // Higher elevation provides better defensive value
    DefensiveValue += Location.Z * 0.001f;

    // Distance from existing structures affects value
    for (AActor* Structure : DefensiveStructures)
    {
        if (Structure && IsValid(Structure))
        {
            float Distance = FVector::Dist(Structure->GetActorLocation(), Location);
            if (Distance < 1000.0f)
            {
                DefensiveValue += (1000.0f - Distance) * 0.0005f;
            }
        }
    }

    return FMath::Clamp(DefensiveValue, 0.1f, 2.0f);
}

TArray<FArch_ThreatDetectionData> UArch_DefensiveStructureSystem::GetDetectedThreats() const
{
    return CurrentThreats;
}

bool UArch_DefensiveStructureSystem::IsLocationDefended(const FVector& Location) const
{
    for (AActor* Structure : DefensiveStructures)
    {
        if (Structure && IsValid(Structure))
        {
            float Distance = FVector::Dist(Structure->GetActorLocation(), Location);
            if (Distance <= DefensiveConfig.DefensiveRadius)
            {
                return true;
            }
        }
    }
    return false;
}

AActor* UArch_DefensiveStructureSystem::GetNearestDefensiveStructure(const FVector& Location) const
{
    AActor* NearestStructure = nullptr;
    float MinDistance = FLT_MAX;

    for (AActor* Structure : DefensiveStructures)
    {
        if (Structure && IsValid(Structure))
        {
            float Distance = FVector::Dist(Structure->GetActorLocation(), Location);
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                NearestStructure = Structure;
            }
        }
    }

    return NearestStructure;
}

void UArch_DefensiveStructureSystem::PerformMaintenanceCheck()
{
    for (int32 i = 0; i < StructureStatuses.Num(); i++)
    {
        if (StructureStatuses[i].CurrentIntegrity < 75.0f)
        {
            StructureStatuses[i].bNeedsRepair = true;
        }

        // Natural degradation over time
        float TimeSinceLastMaintenance = GetWorld()->GetTimeSeconds() - StructureStatuses[i].LastMaintenanceTime;
        if (TimeSinceLastMaintenance > 60.0f) // 1 minute
        {
            StructureStatuses[i].CurrentIntegrity = FMath::Max(0.0f, StructureStatuses[i].CurrentIntegrity - 1.0f);
            StructureStatuses[i].LastMaintenanceTime = GetWorld()->GetTimeSeconds();
        }
    }
}

void UArch_DefensiveStructureSystem::RepairStructure(int32 StructureIndex, float RepairAmount)
{
    if (StructureIndex >= 0 && StructureIndex < StructureStatuses.Num())
    {
        StructureStatuses[StructureIndex].CurrentIntegrity = FMath::Min(100.0f, 
            StructureStatuses[StructureIndex].CurrentIntegrity + RepairAmount);
        
        if (StructureStatuses[StructureIndex].CurrentIntegrity >= 75.0f)
        {
            StructureStatuses[StructureIndex].bNeedsRepair = false;
        }

        UE_LOG(LogTemp, Log, TEXT("Repaired structure %d - Integrity: %.2f"), 
               StructureIndex, StructureStatuses[StructureIndex].CurrentIntegrity);
    }
}

int32 UArch_DefensiveStructureSystem::GetTotalDefensiveStructures() const
{
    return DefensiveStructures.Num();
}