#include "DinosaurCombatAIComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDinosaurCombatAIComponent::UDinosaurCombatAIComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — sufficient for AI decisions
}

void UDinosaurCombatAIComponent::BeginPlay()
{
    Super::BeginPlay();
    InitSpeciesParams();
}

void UDinosaurCombatAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AttackCooldownTimer += DeltaTime;
    EncounterState.TimeSinceLastAttack += DeltaTime;

    if (CurrentTarget && IsValid(CurrentTarget))
    {
        EncounterState.DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
        TickCombatPhase(DeltaTime);
    }
    else if (EncounterState.CurrentPhase != ECombat_DinoPhase::Roaming &&
             EncounterState.CurrentPhase != ECombat_DinoPhase::Feeding)
    {
        // Target gone — return to roaming
        TransitionToPhase(ECombat_DinoPhase::Roaming);
    }
}

void UDinosaurCombatAIComponent::InitSpeciesParams()
{
    switch (Species)
    {
        case ECombat_DinoSpecies::TyrannosaurusRex:
            SpeciesParams.DetectionRadius = 2500.0f;
            SpeciesParams.ChargeRadius    = 800.0f;
            SpeciesParams.AttackRadius    = 300.0f;
            SpeciesParams.AttackDamage    = 80.0f;
            SpeciesParams.AttackCooldown  = 3.0f;
            SpeciesParams.MoveSpeed       = 700.0f;
            SpeciesParams.bUsesPackTactics = false;
            SpeciesParams.bCanFlanks       = false;
            break;

        case ECombat_DinoSpecies::Velociraptor:
            SpeciesParams.DetectionRadius = 1800.0f;
            SpeciesParams.ChargeRadius    = 500.0f;
            SpeciesParams.AttackRadius    = 150.0f;
            SpeciesParams.AttackDamage    = 25.0f;
            SpeciesParams.AttackCooldown  = 1.2f;
            SpeciesParams.MoveSpeed       = 900.0f;
            SpeciesParams.bUsesPackTactics = true;
            SpeciesParams.bCanFlanks       = true;
            SpeciesParams.FlankAngleDegrees = 90.0f;
            break;

        case ECombat_DinoSpecies::Brachiosaurus:
            SpeciesParams.DetectionRadius = 1200.0f;
            SpeciesParams.ChargeRadius    = 400.0f;
            SpeciesParams.AttackRadius    = 350.0f;
            SpeciesParams.AttackDamage    = 60.0f;
            SpeciesParams.AttackCooldown  = 4.0f;
            SpeciesParams.MoveSpeed       = 350.0f;
            SpeciesParams.bUsesPackTactics = false;
            SpeciesParams.bCanFlanks       = false;
            break;

        case ECombat_DinoSpecies::Triceratops:
            SpeciesParams.DetectionRadius = 1000.0f;
            SpeciesParams.ChargeRadius    = 600.0f;
            SpeciesParams.AttackRadius    = 250.0f;
            SpeciesParams.AttackDamage    = 50.0f;
            SpeciesParams.AttackCooldown  = 2.0f;
            SpeciesParams.MoveSpeed       = 550.0f;
            SpeciesParams.bUsesPackTactics = false;
            SpeciesParams.bCanFlanks       = false;
            break;

        case ECombat_DinoSpecies::Pterodactyl:
            SpeciesParams.DetectionRadius = 3000.0f;
            SpeciesParams.ChargeRadius    = 700.0f;
            SpeciesParams.AttackRadius    = 200.0f;
            SpeciesParams.AttackDamage    = 20.0f;
            SpeciesParams.AttackCooldown  = 1.8f;
            SpeciesParams.MoveSpeed       = 1200.0f;
            SpeciesParams.bUsesPackTactics = false;
            SpeciesParams.bCanFlanks       = true;
            SpeciesParams.FlankAngleDegrees = 45.0f;
            break;

        default:
            break;
    }
}

void UDinosaurCombatAIComponent::OnTargetDetected(AActor* Target, float InitialThreat)
{
    if (!Target || !IsValid(Target)) return;

    CurrentTarget = Target;
    EncounterState.ThreatLevel = FMath::Clamp(InitialThreat, 0.0f, 1.0f);
    EncounterState.TargetLocation = Target->GetActorLocation();

    // Immediately alert nearby NPCs — they should flee
    AlertNearbyNPCs(SpeciesParams.DetectionRadius * 0.8f);

    TransitionToPhase(ECombat_DinoPhase::Stalking);
}

void UDinosaurCombatAIComponent::OnTargetLost()
{
    CurrentTarget = nullptr;
    EncounterState.ThreatLevel = 0.0f;
    TransitionToPhase(ECombat_DinoPhase::Roaming);
}

void UDinosaurCombatAIComponent::TickCombatPhase(float DeltaTime)
{
    const float Dist = EncounterState.DistanceToTarget;

    // Phase transitions based on distance
    switch (EncounterState.CurrentPhase)
    {
        case ECombat_DinoPhase::Stalking:
            if (Dist <= SpeciesParams.ChargeRadius)
            {
                TransitionToPhase(ECombat_DinoPhase::Charging);
            }
            break;

        case ECombat_DinoPhase::Charging:
            if (Dist <= SpeciesParams.AttackRadius)
            {
                // In attack range — execute attack if cooldown ready
                if (AttackCooldownTimer >= SpeciesParams.AttackCooldown)
                {
                    ExecuteAttack(CurrentTarget);
                    AttackCooldownTimer = 0.0f;
                    EncounterState.TimeSinceLastAttack = 0.0f;
                }
            }
            else if (Dist > SpeciesParams.ChargeRadius * 1.5f)
            {
                // Target escaped charge range — stalk again
                TransitionToPhase(ECombat_DinoPhase::Stalking);
            }
            break;

        case ECombat_DinoPhase::Flanking:
            // Flanking transitions to charging once in position
            if (Dist <= SpeciesParams.ChargeRadius)
            {
                TransitionToPhase(ECombat_DinoPhase::Charging);
            }
            break;

        case ECombat_DinoPhase::Retreating:
            // Retreat until safe distance
            if (Dist > SpeciesParams.DetectionRadius * 0.6f)
            {
                TransitionToPhase(ECombat_DinoPhase::Roaming);
            }
            break;

        default:
            break;
    }

    // Evaluate and store pending action
    EncounterState.PendingAction = EvaluateTactics();

    // Update target location snapshot
    if (CurrentTarget && IsValid(CurrentTarget))
    {
        EncounterState.TargetLocation = CurrentTarget->GetActorLocation();
    }
}

void UDinosaurCombatAIComponent::TransitionToPhase(ECombat_DinoPhase NewPhase)
{
    if (EncounterState.CurrentPhase == NewPhase) return;
    EncounterState.CurrentPhase = NewPhase;

    // When entering Charging, alert NPCs again with higher urgency
    if (NewPhase == ECombat_DinoPhase::Charging)
    {
        AlertNearbyNPCs(SpeciesParams.ChargeRadius * 2.0f);
    }
}

ECombat_TacticalAction UDinosaurCombatAIComponent::EvaluateTactics()
{
    switch (Species)
    {
        case ECombat_DinoSpecies::TyrannosaurusRex:
            return DecideTacticsForTRex();

        case ECombat_DinoSpecies::Velociraptor:
            return DecideTacticsForRaptor();

        case ECombat_DinoSpecies::Brachiosaurus:
        case ECombat_DinoSpecies::Triceratops:
            return DecideTacticsForHerbivore();

        default:
            return ECombat_TacticalAction::ChargeTarget;
    }
}

ECombat_TacticalAction UDinosaurCombatAIComponent::DecideTacticsForTRex() const
{
    // T-Rex: direct, overwhelming force — no flanking, just charge and roar
    switch (EncounterState.CurrentPhase)
    {
        case ECombat_DinoPhase::Stalking:
            // Roar to intimidate before charging
            if (EncounterState.TimeSinceLastAttack > 5.0f)
                return ECombat_TacticalAction::Roar;
            return ECombat_TacticalAction::ChargeTarget;

        case ECombat_DinoPhase::Charging:
            return ECombat_TacticalAction::Bite;

        case ECombat_DinoPhase::Retreating:
            return ECombat_TacticalAction::Retreat;

        default:
            return ECombat_TacticalAction::None;
    }
}

ECombat_TacticalAction UDinosaurCombatAIComponent::DecideTacticsForRaptor() const
{
    // Raptors: pack tactics, flanking, coordinated strikes
    switch (EncounterState.CurrentPhase)
    {
        case ECombat_DinoPhase::Stalking:
            if (EncounterState.bPackMembersNearby && EncounterState.PackMemberCount >= 2)
                return ECombat_TacticalAction::MoveToFlank;
            return ECombat_TacticalAction::CircleTarget;

        case ECombat_DinoPhase::Flanking:
            return ECombat_TacticalAction::MoveToFlank;

        case ECombat_DinoPhase::Charging:
            if (EncounterState.bPackMembersNearby)
                return ECombat_TacticalAction::Bite; // Coordinated bite
            return ECombat_TacticalAction::CircleTarget; // Solo raptor circles

        case ECombat_DinoPhase::Retreating:
            return ECombat_TacticalAction::CallPack; // Call pack before retreating

        default:
            return ECombat_TacticalAction::None;
    }
}

ECombat_TacticalAction UDinosaurCombatAIComponent::DecideTacticsForHerbivore() const
{
    // Herbivores: defensive — charge only when cornered
    switch (EncounterState.CurrentPhase)
    {
        case ECombat_DinoPhase::Stalking:
            return ECombat_TacticalAction::Retreat; // Herbivores prefer to flee

        case ECombat_DinoPhase::Charging:
            // Cornered — fight back
            if (EncounterState.ThreatLevel > 0.8f)
                return ECombat_TacticalAction::Bite;
            return ECombat_TacticalAction::Retreat;

        default:
            return ECombat_TacticalAction::Retreat;
    }
}

void UDinosaurCombatAIComponent::ExecuteAttack(AActor* Target)
{
    if (!Target || !IsValid(Target)) return;

    const float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    if (Dist > SpeciesParams.AttackRadius) return;

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        Target,
        SpeciesParams.AttackDamage,
        nullptr, // Instigator controller — set by AIController in full implementation
        GetOwner(),
        UDamageType::StaticClass()
    );
}

void UDinosaurCombatAIComponent::AlertNearbyNPCs(float AlertRadius)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find all actors with UNPCBehaviorComponent within radius
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    const FVector DinoLocation = GetOwner()->GetActorLocation();

    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == GetOwner()) continue;

        const float Dist = FVector::Dist(DinoLocation, Actor->GetActorLocation());
        if (Dist > AlertRadius) continue;

        UNPCBehaviorComponent* NPCComp = Actor->FindComponentByClass<UNPCBehaviorComponent>();
        if (NPCComp)
        {
            // Threat level scales inversely with distance (closer = more threatening)
            const float ThreatLevel = FMath::Clamp(1.0f - (Dist / AlertRadius), 0.1f, 1.0f);
            NPCComp->RegisterThreat(DinoLocation, ThreatLevel);
        }
    }
}

FVector UDinosaurCombatAIComponent::ComputeFlankPosition(AActor* Target, bool bLeftSide) const
{
    if (!Target) return FVector::ZeroVector;

    const FVector OwnerLoc = GetOwner()->GetActorLocation();
    const FVector TargetLoc = Target->GetActorLocation();
    const FVector ToTarget = (TargetLoc - OwnerLoc).GetSafeNormal();

    // Perpendicular flank direction
    const FVector Right = FVector::CrossProduct(ToTarget, FVector::UpVector).GetSafeNormal();
    const FVector FlankDir = bLeftSide ? -Right : Right;

    const float FlankDist = SpeciesParams.ChargeRadius * 0.7f;
    return TargetLoc + FlankDir * FlankDist;
}

bool UDinosaurCombatAIComponent::IsInCombat() const
{
    return EncounterState.CurrentPhase != ECombat_DinoPhase::Roaming &&
           EncounterState.CurrentPhase != ECombat_DinoPhase::Feeding;
}
