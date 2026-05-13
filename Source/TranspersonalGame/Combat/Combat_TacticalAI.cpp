#include "Combat_TacticalAI.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    CurrentTacticalState = ECombat_TacticalState::Idle;
    PrimaryTarget = nullptr;
    AssignedPosition = FVector::ZeroVector;
    FormationIndex = -1;

    // Default tactical parameters
    FlankingRange = 800.0f;
    RetreatHealthThreshold = 0.3f;
    CommunicationRange = 1500.0f;
    TacticalUpdateInterval = 1.0f;

    LastTacticalUpdate = 0.0f;
    StateChangeTimer = 0.0f;
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Combat_TacticalAI: Component initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner() || !PrimaryTarget)
    {
        return;
    }

    StateChangeTimer += DeltaTime;
    
    // Update tactical situation periodically
    if (StateChangeTimer >= TacticalUpdateInterval)
    {
        EvaluateTacticalSituation();
        UpdateFormationPositions();
        StateChangeTimer = 0.0f;
    }

    // Execute current tactical behavior
    switch (CurrentTacticalState)
    {
        case ECombat_TacticalState::Scouting:
            // Move to assigned scouting position
            break;
        case ECombat_TacticalState::Stalking:
            // Maintain distance and track target
            break;
        case ECombat_TacticalState::Flanking:
            // Move to flanking position
            break;
        case ECombat_TacticalState::Attacking:
            // Execute attack behavior
            break;
        case ECombat_TacticalState::Retreating:
            // Move away from danger
            break;
        case ECombat_TacticalState::Regrouping:
            // Return to formation
            break;
    }
}

void UCombat_TacticalAI::InitializeTacticalAI(AActor* InTarget, TArray<AActor*> InAllies)
{
    PrimaryTarget = InTarget;
    AlliedActors = InAllies;
    
    if (PrimaryTarget)
    {
        SetTacticalState(ECombat_TacticalState::Scouting);
        UE_LOG(LogTemp, Log, TEXT("Combat_TacticalAI: Initialized with target %s and %d allies"), 
               *PrimaryTarget->GetName(), AlliedActors.Num());
    }
}

void UCombat_TacticalAI::SetTacticalState(ECombat_TacticalState NewState)
{
    if (CurrentTacticalState != NewState)
    {
        ECombat_TacticalState PreviousState = CurrentTacticalState;
        CurrentTacticalState = NewState;
        StateChangeTimer = 0.0f;

        UE_LOG(LogTemp, Log, TEXT("Combat_TacticalAI: State changed from %d to %d"), 
               (int32)PreviousState, (int32)NewState);

        // Broadcast state change to allies
        BroadcastTacticalOrder(NewState);
    }
}

void UCombat_TacticalAI::CreateFormation(ECombat_FormationType FormationType, AActor* Target)
{
    if (!Target || AlliedActors.Num() == 0)
    {
        return;
    }

    CurrentFormation.FormationType = FormationType;
    CurrentFormation.Target = Target;
    CurrentFormation.CenterPoint = Target->GetActorLocation();
    CurrentFormation.Positions.Empty();

    TArray<FVector> FormationPositions;
    FVector TargetLocation = Target->GetActorLocation();
    FVector Direction = (GetOwner()->GetActorLocation() - TargetLocation).GetSafeNormal();

    switch (FormationType)
    {
        case ECombat_FormationType::Line:
            FormationPositions = GenerateLineFormation(TargetLocation, Direction, AlliedActors.Num());
            break;
        case ECombat_FormationType::Circle:
            FormationPositions = GenerateCircleFormation(TargetLocation, CurrentFormation.FormationRadius, AlliedActors.Num());
            break;
        case ECombat_FormationType::Wedge:
            FormationPositions = GenerateWedgeFormation(TargetLocation, Direction, AlliedActors.Num());
            break;
        case ECombat_FormationType::Pincer:
            FormationPositions = GeneratePincerFormation(TargetLocation, Direction, AlliedActors.Num());
            break;
        case ECombat_FormationType::Ambush:
            FormationPositions = GenerateAmbushFormation(TargetLocation, Direction, AlliedActors.Num());
            break;
    }

    // Convert positions to tactical positions
    for (const FVector& Position : FormationPositions)
    {
        FCombat_TacticalPosition TacticalPos;
        TacticalPos.Position = Position;
        TacticalPos.Priority = FMath::RandRange(0.5f, 1.0f);
        TacticalPos.bIsOccupied = false;
        TacticalPos.OccupyingActor = nullptr;
        CurrentFormation.Positions.Add(TacticalPos);
    }

    UE_LOG(LogTemp, Log, TEXT("Combat_TacticalAI: Created %s formation with %d positions"), 
           *UEnum::GetValueAsString(FormationType), FormationPositions.Num());
}

FVector UCombat_TacticalAI::GetAssignedPosition() const
{
    if (FormationIndex >= 0 && FormationIndex < CurrentFormation.Positions.Num())
    {
        return CurrentFormation.Positions[FormationIndex].Position;
    }
    return AssignedPosition;
}

bool UCombat_TacticalAI::AssignPositionInFormation(AActor* Actor)
{
    if (!Actor)
    {
        return false;
    }

    // Find best available position
    int32 BestIndex = -1;
    float BestDistance = FLT_MAX;

    for (int32 i = 0; i < CurrentFormation.Positions.Num(); i++)
    {
        if (!CurrentFormation.Positions[i].bIsOccupied)
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), CurrentFormation.Positions[i].Position);
            if (Distance < BestDistance)
            {
                BestDistance = Distance;
                BestIndex = i;
            }
        }
    }

    if (BestIndex >= 0)
    {
        CurrentFormation.Positions[BestIndex].bIsOccupied = true;
        CurrentFormation.Positions[BestIndex].OccupyingActor = Actor;
        
        if (Actor == GetOwner())
        {
            FormationIndex = BestIndex;
            AssignedPosition = CurrentFormation.Positions[BestIndex].Position;
        }
        
        return true;
    }

    return false;
}

void UCombat_TacticalAI::UpdateFormationPositions()
{
    if (CurrentFormation.FormationType == ECombat_FormationType::None || !CurrentFormation.Target)
    {
        return;
    }

    // Update formation center based on target movement
    FVector NewCenter = CurrentFormation.Target->GetActorLocation();
    FVector Offset = NewCenter - CurrentFormation.CenterPoint;
    
    if (Offset.Size() > 50.0f) // Only update if target moved significantly
    {
        CurrentFormation.CenterPoint = NewCenter;
        
        // Update all position offsets
        for (FCombat_TacticalPosition& Position : CurrentFormation.Positions)
        {
            Position.Position += Offset;
        }
        
        AssignedPosition += Offset;
    }
}

void UCombat_TacticalAI::EvaluateTacticalSituation()
{
    if (!PrimaryTarget || !GetOwner())
    {
        return;
    }

    float ThreatLevel = CalculateThreatLevel();
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), PrimaryTarget->GetActorLocation());

    // Decision making based on threat level and situation
    if (ThreatLevel > 0.8f)
    {
        if (ShouldRetreat())
        {
            SetTacticalState(ECombat_TacticalState::Retreating);
        }
        else if (CurrentTacticalState != ECombat_TacticalState::Attacking)
        {
            SetTacticalState(ECombat_TacticalState::Attacking);
        }
    }
    else if (ThreatLevel > 0.5f)
    {
        if (ShouldFlank() && CurrentTacticalState != ECombat_TacticalState::Flanking)
        {
            SetTacticalState(ECombat_TacticalState::Flanking);
            CreateFormation(ECombat_FormationType::Pincer, PrimaryTarget);
        }
        else if (CurrentTacticalState == ECombat_TacticalState::Idle)
        {
            SetTacticalState(ECombat_TacticalState::Stalking);
        }
    }
    else if (DistanceToTarget > 1000.0f)
    {
        SetTacticalState(ECombat_TacticalState::Scouting);
    }
}

bool UCombat_TacticalAI::ShouldFlank() const
{
    if (!PrimaryTarget || !GetOwner())
    {
        return false;
    }

    // Check if we have enough allies for flanking
    if (AlliedActors.Num() < 2)
    {
        return false;
    }

    // Check if target is in a position that can be flanked
    FVector FlankPosition = FindFlankingPosition();
    return IsPositionSafe(FlankPosition);
}

bool UCombat_TacticalAI::ShouldRetreat() const
{
    if (!GetOwner())
    {
        return false;
    }

    // Check health threshold (assuming pawn has health component)
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        // Simplified health check - in real implementation would check actual health component
        float HealthRatio = 1.0f; // Placeholder
        if (HealthRatio < RetreatHealthThreshold)
        {
            return true;
        }
    }

    // Check if outnumbered significantly
    float ThreatLevel = CalculateThreatLevel();
    return ThreatLevel > 1.5f;
}

FVector UCombat_TacticalAI::FindFlankingPosition() const
{
    if (!PrimaryTarget || !GetOwner())
    {
        return GetOwner()->GetActorLocation();
    }

    FVector TargetLocation = PrimaryTarget->GetActorLocation();
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector ToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();
    
    // Calculate flanking position 90 degrees to the side
    FVector RightVector = FVector::CrossProduct(ToTarget, FVector::UpVector);
    FVector FlankPosition = TargetLocation + (RightVector * FlankingRange);
    
    return FlankPosition;
}

void UCombat_TacticalAI::BroadcastTacticalOrder(ECombat_TacticalState OrderType)
{
    if (!GetOwner())
    {
        return;
    }

    for (AActor* Ally : AlliedActors)
    {
        if (Ally && FVector::Dist(GetOwner()->GetActorLocation(), Ally->GetActorLocation()) <= CommunicationRange)
        {
            if (UCombat_TacticalAI* AllyTacticalAI = Ally->FindComponentByClass<UCombat_TacticalAI>())
            {
                AllyTacticalAI->ReceiveTacticalOrder(OrderType, GetOwner());
            }
        }
    }
}

void UCombat_TacticalAI::ReceiveTacticalOrder(ECombat_TacticalState OrderType, AActor* Sender)
{
    if (!Sender)
    {
        return;
    }

    // Process received order based on current state and sender authority
    switch (OrderType)
    {
        case ECombat_TacticalState::Retreating:
            // Always respond to retreat orders
            SetTacticalState(ECombat_TacticalState::Retreating);
            break;
        case ECombat_TacticalState::Attacking:
            // Join attack if not already retreating
            if (CurrentTacticalState != ECombat_TacticalState::Retreating)
            {
                SetTacticalState(ECombat_TacticalState::Attacking);
            }
            break;
        case ECombat_TacticalState::Regrouping:
            SetTacticalState(ECombat_TacticalState::Regrouping);
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("Combat_TacticalAI: Received tactical order %d from %s"), 
           (int32)OrderType, *Sender->GetName());
}

// Private helper functions
TArray<FVector> UCombat_TacticalAI::GenerateLineFormation(const FVector& Center, const FVector& Direction, int32 NumPositions)
{
    TArray<FVector> Positions;
    FVector RightVector = FVector::CrossProduct(Direction, FVector::UpVector);
    float Spacing = 200.0f;
    float StartOffset = -(NumPositions - 1) * Spacing * 0.5f;

    for (int32 i = 0; i < NumPositions; i++)
    {
        FVector Position = Center + (RightVector * (StartOffset + i * Spacing)) + (Direction * -300.0f);
        Positions.Add(Position);
    }
    return Positions;
}

TArray<FVector> UCombat_TacticalAI::GenerateCircleFormation(const FVector& Center, float Radius, int32 NumPositions)
{
    TArray<FVector> Positions;
    float AngleStep = 360.0f / NumPositions;

    for (int32 i = 0; i < NumPositions; i++)
    {
        float Angle = i * AngleStep;
        FVector Offset = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
            0.0f
        );
        Positions.Add(Center + Offset);
    }
    return Positions;
}

TArray<FVector> UCombat_TacticalAI::GenerateWedgeFormation(const FVector& Center, const FVector& Direction, int32 NumPositions)
{
    TArray<FVector> Positions;
    FVector RightVector = FVector::CrossProduct(Direction, FVector::UpVector);
    
    // Leader at the front
    Positions.Add(Center + Direction * 300.0f);
    
    // Wings spreading back
    for (int32 i = 1; i < NumPositions; i++)
    {
        float Side = (i % 2 == 0) ? 1.0f : -1.0f;
        float Row = (i + 1) / 2;
        FVector Position = Center + (RightVector * Side * Row * 150.0f) + (Direction * -Row * 100.0f);
        Positions.Add(Position);
    }
    return Positions;
}

TArray<FVector> UCombat_TacticalAI::GeneratePincerFormation(const FVector& Center, const FVector& Direction, int32 NumPositions)
{
    TArray<FVector> Positions;
    FVector RightVector = FVector::CrossProduct(Direction, FVector::UpVector);
    
    int32 LeftWing = NumPositions / 2;
    int32 RightWing = NumPositions - LeftWing;
    
    // Left wing
    for (int32 i = 0; i < LeftWing; i++)
    {
        FVector Position = Center + (RightVector * -400.0f) + (Direction * (i * 200.0f - 200.0f));
        Positions.Add(Position);
    }
    
    // Right wing
    for (int32 i = 0; i < RightWing; i++)
    {
        FVector Position = Center + (RightVector * 400.0f) + (Direction * (i * 200.0f - 200.0f));
        Positions.Add(Position);
    }
    
    return Positions;
}

TArray<FVector> UCombat_TacticalAI::GenerateAmbushFormation(const FVector& Center, const FVector& Direction, int32 NumPositions)
{
    TArray<FVector> Positions;
    
    // Scatter positions around target for ambush
    for (int32 i = 0; i < NumPositions; i++)
    {
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(300.0f, 600.0f);
        FVector Offset = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            0.0f
        );
        Positions.Add(Center + Offset);
    }
    
    return Positions;
}

float UCombat_TacticalAI::CalculateThreatLevel() const
{
    if (!PrimaryTarget || !GetOwner())
    {
        return 0.0f;
    }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PrimaryTarget->GetActorLocation());
    float ThreatLevel = FMath::Clamp(1000.0f / Distance, 0.0f, 2.0f);
    
    // Modify based on ally count
    float AllyModifier = FMath::Clamp(AlliedActors.Num() / 3.0f, 0.5f, 1.5f);
    ThreatLevel /= AllyModifier;
    
    return ThreatLevel;
}

bool UCombat_TacticalAI::HasClearLineOfSight(const FVector& From, const FVector& To) const
{
    if (!GetWorld())
    {
        return false;
    }

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        From,
        To,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return !bHit;
}

bool UCombat_TacticalAI::IsPositionSafe(const FVector& Position) const
{
    if (!GetWorld())
    {
        return false;
    }

    // Check for ground
    FHitResult GroundHit;
    FVector TraceStart = Position + FVector(0, 0, 100);
    FVector TraceEnd = Position - FVector(0, 0, 1000);
    
    bool bHasGround = GetWorld()->LineTraceSingleByChannel(
        GroundHit,
        TraceStart,
        TraceEnd,
        ECollisionChannel::ECC_WorldStatic
    );
    
    return bHasGround && GroundHit.Distance < 1000.0f;
}

FVector UCombat_TacticalAI::GetOptimalAttackVector() const
{
    if (!PrimaryTarget || !GetOwner())
    {
        return FVector::ZeroVector;
    }

    FVector TargetLocation = PrimaryTarget->GetActorLocation();
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    
    return (TargetLocation - CurrentLocation).GetSafeNormal();
}