#include "RaptorPackManager.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

URaptorPackManager::URaptorPackManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.25f; // 4Hz — enough for coordination
}

void URaptorPackManager::BeginPlay()
{
    Super::BeginPlay();
    PackData.PackState = ECombat_RaptorPackState::Idle;
    PackData.PackAlertLevel = 0.0f;
    PackData.bPlayerLocationKnown = false;
}

void URaptorPackManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Alert level decays over time when player not spotted
    if (!PackData.bPlayerLocationKnown)
    {
        PackData.PackAlertLevel = FMath::Max(0.0f, PackData.PackAlertLevel - AlertDecayRate * DeltaTime);
    }

    TimeSinceLastCoordination += DeltaTime;
    if (TimeSinceLastCoordination >= CoordinationInterval)
    {
        TimeSinceLastCoordination = 0.0f;
        CoordinatePackBehavior(DeltaTime);
    }

    UpdatePackState();
}

// ── Pack Registration ────────────────────────────────────────────────────────

void URaptorPackManager::RegisterPackMember(AActor* Raptor, ECombat_RaptorRole Role)
{
    if (!Raptor) return;

    if (!PackData.PackMembers.Contains(Raptor))
    {
        PackData.PackMembers.Add(Raptor);
        MemberRoles.Add(Raptor, Role);
        UE_LOG(LogTemp, Log, TEXT("RaptorPackManager: Registered %s as %s"),
            *Raptor->GetName(),
            Role == ECombat_RaptorRole::Alpha ? TEXT("Alpha") :
            Role == ECombat_RaptorRole::Flanker ? TEXT("Flanker") : TEXT("Ambush"));
    }
}

void URaptorPackManager::UnregisterPackMember(AActor* Raptor)
{
    if (!Raptor) return;
    PackData.PackMembers.Remove(Raptor);
    MemberRoles.Remove(Raptor);
    ReleaseFlankPosition(Raptor);
}

// ── Player Detection Sharing ─────────────────────────────────────────────────

void URaptorPackManager::ReportPlayerSighting(FVector PlayerLocation, AActor* ReportingRaptor)
{
    PackData.LastKnownPlayerLocation = PlayerLocation;
    PackData.bPlayerLocationKnown = true;
    PackData.PackAlertLevel = FMath::Min(1.0f, PackData.PackAlertLevel + 0.4f);

    UE_LOG(LogTemp, Log, TEXT("RaptorPackManager: Player sighting reported at (%.0f, %.0f, %.0f) by %s"),
        PlayerLocation.X, PlayerLocation.Y, PlayerLocation.Z,
        ReportingRaptor ? *ReportingRaptor->GetName() : TEXT("Unknown"));

    // Immediately broadcast hunt command to all pack members
    if (PackData.PackAlertLevel >= 0.6f)
    {
        IssueHuntCommand(PlayerLocation);
    }
}

void URaptorPackManager::ReportPlayerLost()
{
    PackData.bPlayerLocationKnown = false;
    // Keep last known location for investigation — do not clear it
    UE_LOG(LogTemp, Log, TEXT("RaptorPackManager: Player lost — pack investigating last known position"));
}

bool URaptorPackManager::GetLastKnownPlayerLocation(FVector& OutLocation) const
{
    if (PackData.bPlayerLocationKnown || PackData.PackAlertLevel > 0.1f)
    {
        OutLocation = PackData.LastKnownPlayerLocation;
        return true;
    }
    return false;
}

ECombat_RaptorPackState URaptorPackManager::GetPackState() const
{
    return PackData.PackState;
}

float URaptorPackManager::GetPackAlertLevel() const
{
    return PackData.PackAlertLevel;
}

// ── Flanking Coordination ────────────────────────────────────────────────────

TArray<FCombat_FlankPosition> URaptorPackManager::CalculateFlankPositions(FVector PlayerLocation)
{
    TArray<FCombat_FlankPosition> Positions;
    int32 MemberCount = FMath::Max(1, PackData.PackMembers.Num());

    for (int32 i = 0; i < MemberCount; ++i)
    {
        FCombat_FlankPosition FlankPos;
        FlankPos.WorldPosition = CalculateFlankOffset(i, PlayerLocation, MemberCount);
        FlankPos.AssignedRole = (i == 0) ? ECombat_RaptorRole::Alpha :
                                (i == MemberCount - 1) ? ECombat_RaptorRole::Ambush :
                                ECombat_RaptorRole::Flanker;
        FlankPos.bOccupied = false;
        FlankPos.AssignedRaptor = nullptr;
        Positions.Add(FlankPos);
    }

    return Positions;
}

bool URaptorPackManager::AssignFlankPosition(AActor* Raptor, FVector& OutAssignedPosition)
{
    if (!Raptor || !PackData.bPlayerLocationKnown) return false;

    // Find unoccupied flank position
    for (FCombat_FlankPosition& FlankPos : ActiveFlankPositions)
    {
        if (!FlankPos.bOccupied)
        {
            FlankPos.bOccupied = true;
            FlankPos.AssignedRaptor = Raptor;
            OutAssignedPosition = FlankPos.WorldPosition;
            return true;
        }
    }

    // No pre-calculated position available — generate one
    FVector NewPos = CalculateFlankOffset(
        ActiveFlankPositions.Num(),
        PackData.LastKnownPlayerLocation,
        FMath::Max(3, PackData.PackMembers.Num())
    );

    FCombat_FlankPosition NewFlankPos;
    NewFlankPos.WorldPosition = NewPos;
    NewFlankPos.bOccupied = true;
    NewFlankPos.AssignedRaptor = Raptor;
    ActiveFlankPositions.Add(NewFlankPos);
    OutAssignedPosition = NewPos;
    return true;
}

void URaptorPackManager::ReleaseFlankPosition(AActor* Raptor)
{
    for (FCombat_FlankPosition& FlankPos : ActiveFlankPositions)
    {
        if (FlankPos.AssignedRaptor == Raptor)
        {
            FlankPos.bOccupied = false;
            FlankPos.AssignedRaptor = nullptr;
            return;
        }
    }
}

// ── Pack Commands ────────────────────────────────────────────────────────────

void URaptorPackManager::IssueHuntCommand(FVector TargetLocation)
{
    PackData.PackState = ECombat_RaptorPackState::Flanking;
    PackData.LastHuntTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    // Recalculate flank positions around the new target
    ActiveFlankPositions = CalculateFlankPositions(TargetLocation);

    BroadcastPackCommand(ECombat_RaptorPackState::Flanking);
    UE_LOG(LogTemp, Log, TEXT("RaptorPackManager: HUNT command issued — %d flanking positions calculated"),
        ActiveFlankPositions.Num());
}

void URaptorPackManager::IssueRetreatCommand()
{
    PackData.PackState = ECombat_RaptorPackState::Retreating;
    PackData.PackAlertLevel = 0.2f;
    ActiveFlankPositions.Empty();
    BroadcastPackCommand(ECombat_RaptorPackState::Retreating);
    UE_LOG(LogTemp, Log, TEXT("RaptorPackManager: RETREAT command issued"));
}

void URaptorPackManager::IssueAttackCommand(AActor* Target)
{
    if (!Target) return;
    PackData.PackState = ECombat_RaptorPackState::Attacking;
    BroadcastPackCommand(ECombat_RaptorPackState::Attacking);
    UE_LOG(LogTemp, Log, TEXT("RaptorPackManager: ATTACK command issued on %s"), *Target->GetName());
}

// ── Private ──────────────────────────────────────────────────────────────────

void URaptorPackManager::CoordinatePackBehavior(float DeltaTime)
{
    if (PackData.PackMembers.Num() == 0) return;

    // Remove dead/invalid members
    PackData.PackMembers.RemoveAll([](AActor* A) { return !IsValid(A); });

    // If hunting, refresh flank positions toward current player location
    if (PackData.PackState == ECombat_RaptorPackState::Flanking && PackData.bPlayerLocationKnown)
    {
        ActiveFlankPositions = CalculateFlankPositions(PackData.LastKnownPlayerLocation);
    }
}

void URaptorPackManager::UpdatePackState()
{
    if (PackData.PackMembers.Num() == 0)
    {
        PackData.PackState = ECombat_RaptorPackState::Idle;
        return;
    }

    if (PackData.PackAlertLevel <= 0.0f && !PackData.bPlayerLocationKnown)
    {
        PackData.PackState = ECombat_RaptorPackState::Patrolling;
    }
    else if (PackData.PackAlertLevel > 0.0f && PackData.PackAlertLevel < 0.6f)
    {
        PackData.PackState = ECombat_RaptorPackState::Hunting;
    }
    // Flanking/Attacking states are set by explicit commands
}

void URaptorPackManager::BroadcastPackCommand(ECombat_RaptorPackState NewState)
{
    // In a full implementation, this would call an interface method on each pack member.
    // For now, log the broadcast — AIController on each raptor polls GetPackState() each tick.
    UE_LOG(LogTemp, Verbose, TEXT("RaptorPackManager: Broadcasting state %d to %d members"),
        (int32)NewState, PackData.PackMembers.Num());
}

FVector URaptorPackManager::CalculateFlankOffset(int32 FlankIndex, FVector PlayerLocation, int32 TotalFlankers)
{
    // Distribute raptors evenly in a circle around the player
    // with slight randomness to feel organic
    float BaseAngle = (360.0f / FMath::Max(1, TotalFlankers)) * FlankIndex;
    float RandomOffset = FMath::RandRange(-25.0f, 25.0f);
    float AngleRad = FMath::DegreesToRadians(BaseAngle + RandomOffset);

    float RadiusVariance = FMath::RandRange(0.8f, 1.2f);
    float EffectiveRadius = FlankRadius * RadiusVariance;

    FVector Offset(
        FMath::Cos(AngleRad) * EffectiveRadius,
        FMath::Sin(AngleRad) * EffectiveRadius,
        0.0f
    );

    return PlayerLocation + Offset;
}
