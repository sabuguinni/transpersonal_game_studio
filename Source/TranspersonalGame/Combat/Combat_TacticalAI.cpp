#include "Combat_TacticalAI.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    TacticalUpdateInterval = 0.5f;
    LastTacticalUpdate = 0.0f;
    AttackRange = 300.0f;
    FlankingAngle = 45.0f;
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize tactical state
    CurrentTacticalState = FCombat_TacticalState();
    PackData = FCombat_PackCoordination();
    
    UE_LOG(LogTemp, Log, TEXT("Combat_TacticalAI: Component initialized for %s"), 
           *GetOwner()->GetName());
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update tactical analysis periodically
    if (CurrentTime - LastTacticalUpdate >= TacticalUpdateInterval)
    {
        if (PackData.PrimaryTarget)
        {
            CurrentTacticalState = AnalyzeTacticalSituation(PackData.PrimaryTarget);
            LastTacticalUpdate = CurrentTime;
        }
    }
    
    // Update pack coordination if we're part of a pack
    if (PackData.PackMembers.Num() > 1)
    {
        // Clean up invalid pack members
        PackData.PackMembers.RemoveAll([](AActor* Actor) {
            return !IsValid(Actor);
        });
        
        // Update pack member count in tactical state
        CurrentTacticalState.PackMembersNearby = PackData.PackMembers.Num();
    }
}

FCombat_TacticalState UCombat_TacticalAI::AnalyzeTacticalSituation(AActor* Target)
{
    FCombat_TacticalState NewState;
    
    if (!IsValid(Target) || !IsValid(GetOwner()))
    {
        return NewState;
    }
    
    // Calculate distance to target
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    NewState.DistanceToTarget = FVector::Dist(OwnerLocation, TargetLocation);
    
    // Check line of sight
    NewState.bHasLineOfSight = IsLineOfSightClear(Target);
    
    // Assess threat level
    NewState.ThreatLevel = AssessThreatLevel(Target);
    
    // Check flanking position
    NewState.bIsFlankingTarget = CalculateFlankingAdvantage(Target) > 0.7f;
    
    // Count nearby pack members
    TArray<AActor*> NearbyPack = FindNearbyPackMembers();
    NewState.PackMembersNearby = NearbyPack.Num();
    
    // Store last attack time
    NewState.LastAttackTime = CurrentTacticalState.LastAttackTime;
    
    return NewState;
}

ECombat_AttackPattern UCombat_TacticalAI::SelectOptimalAttackPattern(const FCombat_TacticalState& TacticalState)
{
    // Solo combat patterns
    if (TacticalState.PackMembersNearby <= 1)
    {
        if (TacticalState.DistanceToTarget > AttackRange * 2.0f)
        {
            return ECombat_AttackPattern::Stalk;
        }
        else if (TacticalState.bHasLineOfSight && TacticalState.DistanceToTarget <= AttackRange)
        {
            return ECombat_AttackPattern::DirectAssault;
        }
        else
        {
            return ECombat_AttackPattern::Ambush;
        }
    }
    
    // Pack combat patterns
    if (TacticalState.PackMembersNearby >= 3)
    {
        return ECombat_AttackPattern::PackHunt;
    }
    else if (TacticalState.bIsFlankingTarget)
    {
        return ECombat_AttackPattern::Flank;
    }
    
    return ECombat_AttackPattern::DirectAssault;
}

FVector UCombat_TacticalAI::CalculateOptimalPosition(AActor* Target, ECombat_AttackPattern Pattern)
{
    if (!IsValid(Target) || !IsValid(GetOwner()))
    {
        return GetOwner()->GetActorLocation();
    }
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    switch (Pattern)
    {
        case ECombat_AttackPattern::Stalk:
        {
            // Stay at medium distance, slightly behind cover
            FVector StalkPosition = TargetLocation - (DirectionToTarget * AttackRange * 1.5f);
            return StalkPosition;
        }
        
        case ECombat_AttackPattern::Flank:
        {
            // Move to the side of the target
            FVector RightVector = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
            FVector FlankPosition = TargetLocation + (RightVector * AttackRange * 0.8f);
            return FlankPosition;
        }
        
        case ECombat_AttackPattern::Ambush:
        {
            // Position behind target
            FVector AmbushPosition = TargetLocation - (DirectionToTarget * AttackRange * 0.5f);
            return AmbushPosition;
        }
        
        case ECombat_AttackPattern::DirectAssault:
        {
            // Move directly toward target at attack range
            FVector AssaultPosition = TargetLocation - (DirectionToTarget * AttackRange * 0.7f);
            return AssaultPosition;
        }
        
        case ECombat_AttackPattern::PackHunt:
        {
            // Use pack coordination to determine position
            int32 MemberIndex = PackData.PackMembers.Find(GetOwner());
            return GetPackPosition(MemberIndex, PackData.CurrentTactic);
        }
        
        default:
            return OwnerLocation;
    }
}

void UCombat_TacticalAI::InitializePackCoordination(const TArray<AActor*>& PackMembers, AActor* Leader)
{
    PackData.PackMembers = PackMembers;
    PackData.PackLeader = Leader;
    PackData.CurrentTactic = ECombat_PackTactic::Surround;
    PackData.TacticStartTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Combat_TacticalAI: Pack initialized with %d members, leader: %s"), 
           PackMembers.Num(), Leader ? *Leader->GetName() : TEXT("None"));
}

void UCombat_TacticalAI::UpdatePackTactic(ECombat_PackTactic NewTactic)
{
    if (PackData.CurrentTactic != NewTactic)
    {
        PackData.CurrentTactic = NewTactic;
        PackData.TacticStartTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("Combat_TacticalAI: Pack tactic changed to %d"), (int32)NewTactic);
    }
}

FVector UCombat_TacticalAI::GetPackPosition(int32 MemberIndex, ECombat_PackTactic Tactic)
{
    if (!IsValid(PackData.PrimaryTarget) || MemberIndex < 0)
    {
        return GetOwner()->GetActorLocation();
    }
    
    FVector TargetLocation = PackData.PrimaryTarget->GetActorLocation();
    float Radius = AttackRange * 1.2f;
    
    switch (Tactic)
    {
        case ECombat_PackTactic::Surround:
        {
            float AngleStep = 360.0f / FMath::Max(PackData.PackMembers.Num(), 1);
            float Angle = MemberIndex * AngleStep;
            FVector Offset = FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
                0.0f
            );
            return TargetLocation + Offset;
        }
        
        case ECombat_PackTactic::Pincer:
        {
            bool bLeftSide = (MemberIndex % 2 == 0);
            FVector SideOffset = FVector(0.0f, bLeftSide ? -Radius : Radius, 0.0f);
            return TargetLocation + SideOffset;
        }
        
        case ECombat_PackTactic::LineFormation:
        {
            FVector LineOffset = FVector(MemberIndex * 200.0f - (PackData.PackMembers.Num() * 100.0f), -Radius, 0.0f);
            return TargetLocation + LineOffset;
        }
        
        default:
            return TargetLocation + FVector(MemberIndex * 300.0f, 0.0f, 0.0f);
    }
}

ECombat_ThreatLevel UCombat_TacticalAI::AssessThreatLevel(AActor* Target)
{
    if (!IsValid(Target))
    {
        return ECombat_ThreatLevel::None;
    }
    
    float Distance = CurrentTacticalState.DistanceToTarget;
    
    // Distance-based threat assessment
    if (Distance > AttackRange * 3.0f)
    {
        return ECombat_ThreatLevel::Low;
    }
    else if (Distance > AttackRange * 1.5f)
    {
        return ECombat_ThreatLevel::Medium;
    }
    else if (Distance > AttackRange * 0.5f)
    {
        return ECombat_ThreatLevel::High;
    }
    else
    {
        return ECombat_ThreatLevel::Critical;
    }
}

bool UCombat_TacticalAI::ShouldRetreat(const FCombat_TacticalState& TacticalState)
{
    // Retreat if critically threatened and outnumbered
    if (TacticalState.ThreatLevel == ECombat_ThreatLevel::Critical && 
        TacticalState.PackMembersNearby < 2)
    {
        return true;
    }
    
    // Retreat if no line of sight and threat is high
    if (!TacticalState.bHasLineOfSight && 
        TacticalState.ThreatLevel >= ECombat_ThreatLevel::High)
    {
        return true;
    }
    
    return false;
}

bool UCombat_TacticalAI::ShouldCallForBackup(const FCombat_TacticalState& TacticalState)
{
    // Call for backup if facing high threat alone
    return (TacticalState.ThreatLevel >= ECombat_ThreatLevel::High && 
            TacticalState.PackMembersNearby < 2);
}

bool UCombat_TacticalAI::CanAttackNow(float CooldownTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - CurrentTacticalState.LastAttackTime) >= CooldownTime;
}

void UCombat_TacticalAI::RegisterAttack()
{
    CurrentTacticalState.LastAttackTime = GetWorld()->GetTimeSeconds();
}

bool UCombat_TacticalAI::IsLineOfSightClear(AActor* Target)
{
    if (!IsValid(Target) || !IsValid(GetOwner()))
    {
        return false;
    }
    
    FVector Start = GetOwner()->GetActorLocation();
    FVector End = Target->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Target);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return !bHit;
}

float UCombat_TacticalAI::CalculateFlankingAdvantage(AActor* Target)
{
    if (!IsValid(Target) || !IsValid(GetOwner()))
    {
        return 0.0f;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    FVector TargetForward = Target->GetActorForwardVector();
    
    FVector DirectionToOwner = (OwnerLocation - TargetLocation).GetSafeNormal();
    float DotProduct = FVector::DotProduct(TargetForward, DirectionToOwner);
    
    // Convert dot product to flanking advantage (1.0 = perfect flank, 0.0 = head-on)
    return FMath::Clamp((1.0f - DotProduct) * 0.5f, 0.0f, 1.0f);
}

TArray<AActor*> UCombat_TacticalAI::FindNearbyPackMembers(float SearchRadius)
{
    TArray<AActor*> NearbyMembers;
    
    if (!IsValid(GetOwner()) || PackData.PackMembers.Num() <= 1)
    {
        return NearbyMembers;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Member : PackData.PackMembers)
    {
        if (IsValid(Member) && Member != GetOwner())
        {
            float Distance = FVector::Dist(OwnerLocation, Member->GetActorLocation());
            if (Distance <= SearchRadius)
            {
                NearbyMembers.Add(Member);
            }
        }
    }
    
    return NearbyMembers;
}