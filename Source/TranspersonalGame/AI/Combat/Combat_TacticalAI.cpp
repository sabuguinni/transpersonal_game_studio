#include "Combat_TacticalAI.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize tactical parameters
    TacticalAnalysisRadius = 2000.0f;
    ThreatAssessmentInterval = 0.5f;
    PackCoordinationRadius = 1500.0f;
    FlankingDistance = 800.0f;
    RetreatThreshold = 0.3f;

    // Initialize state
    CurrentStance = ECombat_TacticalStance::Defensive;
    bIsPackLeader = false;
    LastThreatUpdate = 0.0f;
    LastTacticalAnalysis = 0.0f;
    CachedPrimaryThreat = nullptr;
    PackRallyPoint = FVector::ZeroVector;
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize pack coordination
    UpdatePackMembers();
    
    // Set initial tactical position
    if (AActor* Owner = GetOwner())
    {
        CurrentPosition.Position = Owner->GetActorLocation();
        CurrentPosition.AdvantageScore = 0.5f;
    }
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner())
        return;

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Update threat assessment periodically
    if (CurrentTime - LastThreatUpdate >= ThreatAssessmentInterval)
    {
        UpdateThreatAssessment();
        LastThreatUpdate = CurrentTime;
    }

    // Perform tactical analysis
    if (CurrentTime - LastTacticalAnalysis >= 1.0f)
    {
        AnalyzeTacticalSituation();
        LastTacticalAnalysis = CurrentTime;
    }

    // Coordinate with pack
    if (PackMembers.Num() > 0)
    {
        CoordinateWithPack();
    }
}

void UCombat_TacticalAI::AnalyzeTacticalSituation()
{
    if (!GetOwner())
        return;

    AActor* PrimaryThreat = GetPrimaryThreat();
    if (!PrimaryThreat)
    {
        SetTacticalStance(ECombat_TacticalStance::Defensive);
        return;
    }

    float ThreatDistance = FVector::Dist(GetOwner()->GetActorLocation(), PrimaryThreat->GetActorLocation());
    float ThreatLevel = CalculateThreatLevel(PrimaryThreat);

    // Determine tactical stance based on situation
    if (ThreatLevel > 0.8f && ThreatDistance < 500.0f)
    {
        // High threat, close distance - consider retreat or aggressive response
        if (PackMembers.Num() >= 2)
        {
            SetTacticalStance(ECombat_TacticalStance::Aggressive);
        }
        else if (ThreatLevel > RetreatThreshold)
        {
            SetTacticalStance(ECombat_TacticalStance::Retreating);
        }
    }
    else if (ThreatDistance > 1000.0f && PackMembers.Num() > 0)
    {
        // Distant threat with pack support - flanking opportunity
        SetTacticalStance(ECombat_TacticalStance::Flanking);
    }
    else if (ThreatDistance < 300.0f)
    {
        // Close threat - defensive stance
        SetTacticalStance(ECombat_TacticalStance::Defensive);
    }

    // Update current position advantage
    CurrentPosition.AdvantageScore = CalculatePositionAdvantage(GetOwner()->GetActorLocation(), PrimaryThreat);
}

FCombat_TacticalPosition UCombat_TacticalAI::FindOptimalPosition(const FVector& TargetLocation)
{
    FCombat_TacticalPosition BestPosition;
    float BestScore = -1.0f;

    if (!GetOwner())
        return BestPosition;

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Generate candidate positions in a circle around current location
    int32 NumCandidates = 8;
    float SearchRadius = 600.0f;

    for (int32 i = 0; i < NumCandidates; i++)
    {
        float Angle = (2.0f * PI * i) / NumCandidates;
        FVector CandidatePos = OwnerLocation + FVector(
            FMath::Cos(Angle) * SearchRadius,
            FMath::Sin(Angle) * SearchRadius,
            0.0f
        );

        // Calculate advantage score for this position
        float Score = CalculatePositionAdvantage(CandidatePos, nullptr);
        
        // Bonus for flanking positions
        FVector ToTarget = (TargetLocation - CandidatePos).GetSafeNormal();
        FVector TargetForward = FVector::ForwardVector; // Assume target faces forward
        float FlankingAngle = FVector::DotProduct(ToTarget, TargetForward);
        if (FMath::Abs(FlankingAngle) < 0.5f) // Side approach
        {
            Score += 0.3f;
        }

        // Check for line of sight
        if (HasLineOfSight(CandidatePos, TargetLocation))
        {
            Score += 0.2f;
        }

        if (Score > BestScore)
        {
            BestScore = Score;
            BestPosition.Position = CandidatePos;
            BestPosition.AdvantageScore = Score;
            BestPosition.bIsFlankingPosition = FMath::Abs(FlankingAngle) < 0.5f;
            BestPosition.bHasCover = false; // TODO: Implement cover detection
        }
    }

    return BestPosition;
}

void UCombat_TacticalAI::SetTacticalStance(ECombat_TacticalStance NewStance)
{
    if (CurrentStance != NewStance)
    {
        CurrentStance = NewStance;
        
        // Broadcast stance change to pack members
        BroadcastTacticalInfo();
        
        // Debug output
        if (GEngine && GetOwner())
        {
            FString StanceName = UEnum::GetValueAsString(NewStance);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
                FString::Printf(TEXT("%s: Tactical Stance -> %s"), 
                *GetOwner()->GetName(), *StanceName));
        }
    }
}

void UCombat_TacticalAI::UpdateThreatAssessment()
{
    if (!GetOwner() || !GetWorld())
        return;

    KnownThreats.Empty();
    
    // Find all potential threats in range
    TArray<AActor*> NearbyActors;
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (TActorIterator<APawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        APawn* Pawn = *ActorItr;
        if (!Pawn || Pawn == GetOwner())
            continue;

        float Distance = FVector::Dist(OwnerLocation, Pawn->GetActorLocation());
        if (Distance <= TacticalAnalysisRadius)
        {
            FCombat_ThreatAssessment Threat;
            Threat.ThreatTarget = Pawn;
            Threat.Distance = Distance;
            Threat.ThreatLevel = CalculateThreatLevel(Pawn);
            Threat.bIsDirectThreat = Threat.ThreatLevel > 0.3f;
            Threat.LastSeenTime = GetWorld()->GetTimeSeconds();
            
            KnownThreats.Add(Threat);
        }
    }

    // Sort threats by level (highest first)
    KnownThreats.Sort([](const FCombat_ThreatAssessment& A, const FCombat_ThreatAssessment& B) {
        return A.ThreatLevel > B.ThreatLevel;
    });

    // Update cached primary threat
    CachedPrimaryThreat = KnownThreats.Num() > 0 ? KnownThreats[0].ThreatTarget : nullptr;
}

AActor* UCombat_TacticalAI::GetPrimaryThreat() const
{
    return CachedPrimaryThreat;
}

float UCombat_TacticalAI::CalculateThreatLevel(AActor* Target) const
{
    if (!Target || !GetOwner())
        return 0.0f;

    float ThreatLevel = 0.0f;
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());

    // Base threat based on distance (closer = more threatening)
    ThreatLevel += FMath::Clamp(1.0f - (Distance / TacticalAnalysisRadius), 0.0f, 0.5f);

    // Check if target is player character
    if (Target->IsA<APawn>() && Cast<APawn>(Target)->IsPlayerControlled())
    {
        ThreatLevel += 0.7f; // Players are high priority threats
    }

    // Check if target is facing us (indicates awareness)
    FVector ToOwner = (GetOwner()->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
    FVector TargetForward = Target->GetActorForwardVector();
    float DotProduct = FVector::DotProduct(TargetForward, ToOwner);
    if (DotProduct > 0.7f)
    {
        ThreatLevel += 0.2f; // Target is aware and facing us
    }

    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

void UCombat_TacticalAI::CoordinateWithPack()
{
    UpdatePackMembers();
    
    if (bIsPackLeader && PackMembers.Num() > 0)
    {
        // As pack leader, coordinate group tactics
        AActor* PrimaryThreat = GetPrimaryThreat();
        if (PrimaryThreat)
        {
            // Assign roles to pack members
            for (int32 i = 0; i < PackMembers.Num(); i++)
            {
                if (UCombat_TacticalAI* Member = PackMembers[i])
                {
                    if (i % 2 == 0)
                    {
                        Member->SetTacticalStance(ECombat_TacticalStance::Flanking);
                    }
                    else
                    {
                        Member->SetTacticalStance(ECombat_TacticalStance::Aggressive);
                    }
                }
            }
        }
    }
}

void UCombat_TacticalAI::RequestPackSupport(const FVector& Location)
{
    PackRallyPoint = Location;
    
    // Notify pack members
    for (UCombat_TacticalAI* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            Member->RespondToPackCall(Location);
        }
    }
}

void UCombat_TacticalAI::RespondToPackCall(const FVector& RallyPoint)
{
    PackRallyPoint = RallyPoint;
    SetTacticalStance(ECombat_TacticalStance::Aggressive);
}

bool UCombat_TacticalAI::ShouldEngageTarget(AActor* Target) const
{
    if (!Target || !GetOwner())
        return false;

    float ThreatLevel = CalculateThreatLevel(Target);
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());

    // Engage if threat is high and we have pack support, or if cornered
    bool bHasPackSupport = PackMembers.Num() >= 2;
    bool bIsCornered = Distance < 300.0f;
    
    return (ThreatLevel > 0.6f && bHasPackSupport) || (ThreatLevel > 0.4f && bIsCornered);
}

bool UCombat_TacticalAI::ShouldRetreat() const
{
    AActor* PrimaryThreat = GetPrimaryThreat();
    if (!PrimaryThreat)
        return false;

    float ThreatLevel = CalculateThreatLevel(PrimaryThreat);
    bool bOutnumbered = PackMembers.Num() == 0 && KnownThreats.Num() > 1;
    
    return ThreatLevel > RetreatThreshold && bOutnumbered;
}

FVector UCombat_TacticalAI::CalculateFlankingPosition(AActor* Target) const
{
    if (!Target || !GetOwner())
        return GetOwner()->GetActorLocation();

    FVector TargetLocation = Target->GetActorLocation();
    FVector TargetForward = Target->GetActorForwardVector();
    FVector TargetRight = Target->GetActorRightVector();
    
    // Position to the side of the target
    FVector FlankingPos = TargetLocation + (TargetRight * FlankingDistance);
    
    return FlankingPos;
}

void UCombat_TacticalAI::UpdatePackMembers()
{
    if (!GetWorld())
        return;

    PackMembers.Empty();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Find other tactical AI components in range
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor == GetOwner())
            continue;

        if (UCombat_TacticalAI* OtherAI = Actor->FindComponentByClass<UCombat_TacticalAI>())
        {
            float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
            if (Distance <= PackCoordinationRadius)
            {
                PackMembers.Add(OtherAI);
            }
        }
    }

    // Determine pack leadership (first found becomes leader)
    bIsPackLeader = PackMembers.Num() > 0;
    for (UCombat_TacticalAI* Member : PackMembers)
    {
        if (Member && Member->bIsPackLeader && Member != this)
        {
            bIsPackLeader = false;
            break;
        }
    }
}

float UCombat_TacticalAI::CalculatePositionAdvantage(const FVector& Position, AActor* Target) const
{
    float Advantage = 0.5f; // Base advantage
    
    if (!GetWorld())
        return Advantage;

    // Higher ground bonus
    if (GetOwner())
    {
        float HeightDiff = Position.Z - GetOwner()->GetActorLocation().Z;
        Advantage += FMath::Clamp(HeightDiff / 500.0f, -0.2f, 0.3f);
    }

    // Cover bonus (simplified - check for nearby obstacles)
    FVector TraceStart = Position + FVector(0, 0, 100);
    FVector TraceEnd = TraceStart + FVector(500, 0, 0);
    
    FHitResult HitResult;
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        Advantage += 0.1f; // Some cover available
    }

    return FMath::Clamp(Advantage, 0.0f, 1.0f);
}

bool UCombat_TacticalAI::HasLineOfSight(const FVector& FromLocation, const FVector& ToLocation) const
{
    if (!GetWorld())
        return false;

    FHitResult HitResult;
    FVector Start = FromLocation + FVector(0, 0, 100); // Eye level
    FVector End = ToLocation + FVector(0, 0, 100);
    
    return !GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
}

void UCombat_TacticalAI::BroadcastTacticalInfo()
{
    // Broadcast tactical information to pack members
    for (UCombat_TacticalAI* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            // Share threat information
            for (const FCombat_ThreatAssessment& Threat : KnownThreats)
            {
                // Pack members can share threat data
                bool bAlreadyKnown = false;
                for (const FCombat_ThreatAssessment& MemberThreat : Member->KnownThreats)
                {
                    if (MemberThreat.ThreatTarget == Threat.ThreatTarget)
                    {
                        bAlreadyKnown = true;
                        break;
                    }
                }
                
                if (!bAlreadyKnown)
                {
                    Member->KnownThreats.Add(Threat);
                }
            }
        }
    }
}