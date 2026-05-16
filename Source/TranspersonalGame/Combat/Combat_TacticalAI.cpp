#include "Combat_TacticalAI.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACombat_TacticalAI::ACombat_TacticalAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create tactical radius component
    TacticalRadius = CreateDefaultSubobject<USphereComponent>(TEXT("TacticalRadius"));
    RootComponent = TacticalRadius;
    TacticalRadius->SetSphereRadius(2000.0f);

    // Initialize tactical parameters
    CurrentTacticalState = ECombat_TacticalState::Patrol;
    ThreatAssessmentRadius = 1500.0f;
    FlankingDistance = 800.0f;
    RetreatThreshold = 0.3f; // 30% health
    bIsPackLeader = false;
    CurrentTarget = nullptr;

    // Initialize formation
    CurrentFormation.FormationRadius = 500.0f;
    CurrentFormation.FormationType = ECombat_TacticalState::Patrol;
}

void ACombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    CalculateTacticalPositions();
    UpdateTacticalAssessment();
    
    UE_LOG(LogTemp, Warning, TEXT("Combat Tactical AI initialized at %s"), 
           *GetActorLocation().ToString());
}

void ACombat_TacticalAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update tactical assessment every frame
    UpdateTacticalAssessment();
    
    // Update pack formation if we're the leader
    if (bIsPackLeader)
    {
        UpdatePackFormation();
    }

    // Execute current tactical state
    switch (CurrentTacticalState)
    {
        case ECombat_TacticalState::Hunt:
            if (CurrentTarget)
            {
                // Move towards target with tactical awareness
                FVector TargetDirection = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                SetActorLocation(GetActorLocation() + TargetDirection * 100.0f * DeltaTime);
            }
            break;
            
        case ECombat_TacticalState::Flank:
            if (CurrentTarget)
            {
                FVector FlankPosition = GetFlankingPosition(CurrentTarget);
                FVector FlankDirection = (FlankPosition - GetActorLocation()).GetSafeNormal();
                SetActorLocation(GetActorLocation() + FlankDirection * 150.0f * DeltaTime);
            }
            break;
            
        case ECombat_TacticalState::Retreat:
            if (CurrentTarget)
            {
                FVector RetreatDirection = (GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
                SetActorLocation(GetActorLocation() + RetreatDirection * 200.0f * DeltaTime);
            }
            break;
    }
}

void ACombat_TacticalAI::SetTacticalState(ECombat_TacticalState NewState)
{
    if (CurrentTacticalState != NewState)
    {
        CurrentTacticalState = NewState;
        BroadcastTacticalOrder(NewState);
        
        UE_LOG(LogTemp, Warning, TEXT("Tactical state changed to: %d"), (int32)NewState);
    }
}

void ACombat_TacticalAI::AssessThreat(AActor* PotentialThreat)
{
    if (!PotentialThreat) return;

    float ThreatLevel = CalculateThreatLevel(PotentialThreat);
    float Distance = FVector::Dist(GetActorLocation(), PotentialThreat->GetActorLocation());

    if (ThreatLevel > 0.7f && Distance < ThreatAssessmentRadius)
    {
        CurrentTarget = PotentialThreat;
        
        if (PackMembers.Num() > 1)
        {
            SetTacticalState(ECombat_TacticalState::Hunt);
            CoordinatePackAttack(PotentialThreat);
        }
        else
        {
            SetTacticalState(ECombat_TacticalState::Flank);
        }
    }
    else if (ThreatLevel > 0.9f)
    {
        SetTacticalState(ECombat_TacticalState::Retreat);
    }
}

void ACombat_TacticalAI::ExecuteFlankingManeuver(AActor* Target)
{
    if (!Target) return;

    FVector FlankPosition = GetFlankingPosition(Target);
    FCombat_TacticalPosition OptimalPosition = FindOptimalPosition(FlankPosition);
    
    // Move to flanking position
    SetActorLocation(OptimalPosition.Position);
    SetTacticalState(ECombat_TacticalState::Flank);
    
    UE_LOG(LogTemp, Warning, TEXT("Executing flanking maneuver to position: %s"), 
           *OptimalPosition.Position.ToString());
}

void ACombat_TacticalAI::InitiatePackFormation(ECombat_TacticalState FormationType)
{
    CurrentFormation.FormationType = FormationType;
    CurrentFormation.LeaderPosition = GetActorLocation();
    
    // Calculate member positions based on formation type
    CurrentFormation.MemberPositions.Empty();
    
    for (int32 i = 0; i < PackMembers.Num(); ++i)
    {
        float Angle = (2.0f * PI * i) / PackMembers.Num();
        FVector Offset = FVector(
            FMath::Cos(Angle) * CurrentFormation.FormationRadius,
            FMath::Sin(Angle) * CurrentFormation.FormationRadius,
            0.0f
        );
        
        CurrentFormation.MemberPositions.Add(GetActorLocation() + Offset);
    }
    
    BroadcastTacticalOrder(FormationType);
}

FCombat_TacticalPosition ACombat_TacticalAI::FindOptimalPosition(FVector TargetLocation)
{
    FCombat_TacticalPosition BestPosition;
    float BestScore = -1.0f;

    for (const FCombat_TacticalPosition& Position : TacticalPositions)
    {
        float Distance = FVector::Dist(Position.Position, TargetLocation);
        float Score = Position.CoverValue - (Position.ThreatLevel * 0.5f) - (Distance * 0.001f);
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestPosition = Position;
        }
    }

    return BestPosition;
}

void ACombat_TacticalAI::CoordinatePackAttack(AActor* Target)
{
    if (!Target || PackMembers.Num() == 0) return;

    // Assign roles to pack members
    for (int32 i = 0; i < PackMembers.Num(); ++i)
    {
        if (i % 2 == 0)
        {
            // Even members flank left
            FVector FlankLeft = GetFlankingPosition(Target) + FVector(0, -FlankingDistance, 0);
            // Signal pack member to move to flank position
        }
        else
        {
            // Odd members flank right
            FVector FlankRight = GetFlankingPosition(Target) + FVector(0, FlankingDistance, 0);
            // Signal pack member to move to flank position
        }
    }
    
    SetTacticalState(ECombat_TacticalState::Hunt);
    UE_LOG(LogTemp, Warning, TEXT("Coordinating pack attack on target: %s"), 
           *Target->GetName());
}

void ACombat_TacticalAI::ExecuteRetreatProtocol()
{
    SetTacticalState(ECombat_TacticalState::Retreat);
    
    // Find safest tactical position
    FCombat_TacticalPosition SafestPosition;
    float LowestThreat = 1.0f;
    
    for (const FCombat_TacticalPosition& Position : TacticalPositions)
    {
        if (Position.ThreatLevel < LowestThreat)
        {
            LowestThreat = Position.ThreatLevel;
            SafestPosition = Position;
        }
    }
    
    // Move to safest position
    SetActorLocation(SafestPosition.Position);
    
    UE_LOG(LogTemp, Warning, TEXT("Executing retreat protocol to safe position"));
}

bool ACombat_TacticalAI::IsPositionSafe(FVector Position)
{
    // Check if position is within safe distance from threats
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (Actor && Actor != this)
        {
            float Distance = FVector::Dist(Position, Actor->GetActorLocation());
            float ThreatLevel = CalculateThreatLevel(Actor);
            
            if (ThreatLevel > 0.5f && Distance < 500.0f)
            {
                return false;
            }
        }
    }
    
    return true;
}

void ACombat_TacticalAI::UpdateTacticalAssessment()
{
    // Recalculate threat levels for all tactical positions
    for (FCombat_TacticalPosition& Position : TacticalPositions)
    {
        Position.ThreatLevel = 0.0f;
        
        TArray<AActor*> NearbyActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);
        
        for (AActor* Actor : NearbyActors)
        {
            if (Actor && Actor != this)
            {
                float Distance = FVector::Dist(Position.Position, Actor->GetActorLocation());
                if (Distance < ThreatAssessmentRadius)
                {
                    Position.ThreatLevel += CalculateThreatLevel(Actor) * (1.0f - Distance / ThreatAssessmentRadius);
                }
            }
        }
    }
}

void ACombat_TacticalAI::AddPackMember(AActor* NewMember)
{
    if (NewMember && !PackMembers.Contains(NewMember))
    {
        PackMembers.Add(NewMember);
        UpdatePackFormation();
        
        UE_LOG(LogTemp, Warning, TEXT("Added pack member: %s. Pack size: %d"), 
               *NewMember->GetName(), PackMembers.Num());
    }
}

void ACombat_TacticalAI::RemovePackMember(AActor* Member)
{
    if (Member && PackMembers.Contains(Member))
    {
        PackMembers.Remove(Member);
        UpdatePackFormation();
        
        UE_LOG(LogTemp, Warning, TEXT("Removed pack member: %s. Pack size: %d"), 
               *Member->GetName(), PackMembers.Num());
    }
}

void ACombat_TacticalAI::CalculateTacticalPositions()
{
    TacticalPositions.Empty();
    
    // Generate tactical positions in a grid around the AI
    for (int32 x = -2; x <= 2; ++x)
    {
        for (int32 y = -2; y <= 2; ++y)
        {
            if (x == 0 && y == 0) continue; // Skip center position
            
            FCombat_TacticalPosition NewPosition;
            NewPosition.Position = GetActorLocation() + FVector(x * 500.0f, y * 500.0f, 0.0f);
            NewPosition.CoverValue = FMath::RandRange(0.2f, 0.8f);
            NewPosition.ThreatLevel = 0.0f;
            NewPosition.bIsFlankingPosition = (FMath::Abs(x) > 1 || FMath::Abs(y) > 1);
            
            TacticalPositions.Add(NewPosition);
        }
    }
}

void ACombat_TacticalAI::UpdatePackFormation()
{
    if (!bIsPackLeader || PackMembers.Num() == 0) return;
    
    // Update formation based on current tactical state
    switch (CurrentTacticalState)
    {
        case ECombat_TacticalState::Hunt:
            CurrentFormation.FormationRadius = 300.0f;
            break;
        case ECombat_TacticalState::Defend:
            CurrentFormation.FormationRadius = 150.0f;
            break;
        default:
            CurrentFormation.FormationRadius = 500.0f;
            break;
    }
    
    InitiatePackFormation(CurrentTacticalState);
}

float ACombat_TacticalAI::CalculateThreatLevel(AActor* Actor)
{
    if (!Actor) return 0.0f;
    
    // Calculate threat based on actor type and distance
    float BaseThreat = 0.1f;
    
    if (Actor->GetName().Contains(TEXT("TRex")))
    {
        BaseThreat = 0.9f;
    }
    else if (Actor->GetName().Contains(TEXT("Raptor")))
    {
        BaseThreat = 0.7f;
    }
    else if (Actor->GetName().Contains(TEXT("Player")) || Actor->GetName().Contains(TEXT("Character")))
    {
        BaseThreat = 0.8f;
    }
    
    return BaseThreat;
}

FVector ACombat_TacticalAI::GetFlankingPosition(AActor* Target)
{
    if (!Target) return GetActorLocation();
    
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector RightVector = FVector::CrossProduct(ToTarget, FVector::UpVector);
    
    // Choose left or right flank randomly
    float FlankDirection = FMath::RandBool() ? 1.0f : -1.0f;
    
    return Target->GetActorLocation() + (RightVector * FlankDirection * FlankingDistance);
}

void ACombat_TacticalAI::BroadcastTacticalOrder(ECombat_TacticalState Order)
{
    // Broadcast tactical order to all pack members
    for (AActor* Member : PackMembers)
    {
        if (Member)
        {
            // In a real implementation, this would call a function on the pack member
            // to coordinate their behavior with the tactical order
            UE_LOG(LogTemp, Warning, TEXT("Broadcasting tactical order %d to %s"), 
                   (int32)Order, *Member->GetName());
        }
    }
}