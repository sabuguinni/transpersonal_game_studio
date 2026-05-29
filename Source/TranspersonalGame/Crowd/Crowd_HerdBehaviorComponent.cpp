#include "Crowd_HerdBehaviorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowd_HerdBehaviorComponent::UCrowd_HerdBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Default herd configuration
    MaxHerdSize = 12;
    HerdRadius = 2000.0f;
    CohesionStrength = 1.0f;
    SeparationStrength = 2.0f;
    AlignmentStrength = 1.5f;
    bIsAlpha = false;

    // Behavior settings
    GrazingTime = 5.0f;
    AlertRadius = 3000.0f;
    FleeSpeed = 800.0f;
    NormalSpeed = 300.0f;

    // State initialization
    bIsFleeingFromThreat = false;
    LastThreatDetectionTime = 0.0f;
    ThreatLocation = FVector::ZeroVector;
}

void UCrowd_HerdBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // If this is an alpha, initialize as herd leader
    if (bIsAlpha)
    {
        AlphaPawn = Cast<APawn>(GetOwner());
        if (AlphaPawn.IsValid())
        {
            FCrowd_HerdMember AlphaMember;
            AlphaMember.MemberPawn = AlphaPawn;
            AlphaMember.Role = ECrowd_HerdRole::Alpha;
            AlphaMember.DistanceFromAlpha = 0.0f;
            AlphaMember.ThreatLevel = 0.0f;
            AlphaMember.bIsAlive = true;
            HerdMembers.Add(AlphaMember);
        }
    }
}

void UCrowd_HerdBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner())
        return;

    // Update herd member positions and states
    UpdateHerdPositions();

    // Check for nearby threats
    CheckForThreats();

    // Calculate and apply herd movement if we're not the alpha
    if (!bIsAlpha && AlphaPawn.IsValid())
    {
        FVector HerdMovement = CalculateHerdMovement();
        
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            if (UPawnMovementComponent* MovementComp = OwnerPawn->GetMovementComponent())
            {
                float CurrentSpeed = bIsFleeingFromThreat ? FleeSpeed : NormalSpeed;
                FVector NormalizedMovement = HerdMovement.GetSafeNormal() * CurrentSpeed * DeltaTime;
                MovementComp->AddInputVector(NormalizedMovement);
            }
        }
    }
}

void UCrowd_HerdBehaviorComponent::JoinHerd(APawn* NewMember, ECrowd_HerdRole Role)
{
    if (!NewMember || HerdMembers.Num() >= MaxHerdSize)
        return;

    // Check if already in herd
    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.MemberPawn == NewMember)
            return;
    }

    FCrowd_HerdMember NewHerdMember;
    NewHerdMember.MemberPawn = NewMember;
    NewHerdMember.Role = Role;
    NewHerdMember.bIsAlive = true;

    if (AlphaPawn.IsValid())
    {
        NewHerdMember.DistanceFromAlpha = FVector::Dist(NewMember->GetActorLocation(), AlphaPawn->GetActorLocation());
    }

    HerdMembers.Add(NewHerdMember);

    UE_LOG(LogTemp, Log, TEXT("Pawn %s joined herd with role %d"), *NewMember->GetName(), (int32)Role);
}

void UCrowd_HerdBehaviorComponent::LeaveHerd(APawn* LeavingMember)
{
    if (!LeavingMember)
        return;

    for (int32 i = HerdMembers.Num() - 1; i >= 0; i--)
    {
        if (HerdMembers[i].MemberPawn == LeavingMember)
        {
            HerdMembers.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Pawn %s left the herd"), *LeavingMember->GetName());
            break;
        }
    }
}

void UCrowd_HerdBehaviorComponent::SetAlpha(APawn* NewAlpha)
{
    if (!NewAlpha)
        return;

    AlphaPawn = NewAlpha;
    bIsAlpha = (GetOwner() == NewAlpha);

    // Update the alpha member in the herd list
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.MemberPawn == NewAlpha)
        {
            Member.Role = ECrowd_HerdRole::Alpha;
            Member.DistanceFromAlpha = 0.0f;
        }
        else if (Member.Role == ECrowd_HerdRole::Alpha)
        {
            Member.Role = ECrowd_HerdRole::Member;
        }
    }
}

FVector UCrowd_HerdBehaviorComponent::CalculateHerdMovement()
{
    if (!AlphaPawn.IsValid())
        return FVector::ZeroVector;

    FVector Cohesion = CalculateCohesion() * CohesionStrength;
    FVector Separation = CalculateSeparation() * SeparationStrength;
    FVector Alignment = CalculateAlignment() * AlignmentStrength;

    FVector TotalForce = Cohesion + Separation + Alignment;

    // If fleeing, add strong force away from threat
    if (bIsFleeingFromThreat && !ThreatLocation.IsZero())
    {
        FVector FleeDirection = (GetOwner()->GetActorLocation() - ThreatLocation).GetSafeNormal();
        TotalForce += FleeDirection * 3.0f; // Strong flee force
    }

    return TotalForce;
}

FVector UCrowd_HerdBehaviorComponent::CalculateCohesion()
{
    if (!AlphaPawn.IsValid())
        return FVector::ZeroVector;

    FVector CenterOfMass = FVector::ZeroVector;
    int32 ValidMembers = 0;

    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.MemberPawn.IsValid() && Member.bIsAlive)
        {
            CenterOfMass += Member.MemberPawn->GetActorLocation();
            ValidMembers++;
        }
    }

    if (ValidMembers > 0)
    {
        CenterOfMass /= ValidMembers;
        return (CenterOfMass - GetOwner()->GetActorLocation()).GetSafeNormal();
    }

    return FVector::ZeroVector;
}

FVector UCrowd_HerdBehaviorComponent::CalculateSeparation()
{
    FVector SeparationForce = FVector::ZeroVector;
    FVector OwnerLocation = GetOwner()->GetActorLocation();

    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.MemberPawn.IsValid() && Member.MemberPawn.Get() != GetOwner() && Member.bIsAlive)
        {
            FVector ToMember = Member.MemberPawn->GetActorLocation() - OwnerLocation;
            float Distance = ToMember.Size();

            if (Distance < 500.0f && Distance > 0.0f) // Too close
            {
                SeparationForce -= ToMember.GetSafeNormal() / Distance;
            }
        }
    }

    return SeparationForce.GetSafeNormal();
}

FVector UCrowd_HerdBehaviorComponent::CalculateAlignment()
{
    if (!AlphaPawn.IsValid())
        return FVector::ZeroVector;

    FVector AverageVelocity = FVector::ZeroVector;
    int32 ValidMembers = 0;

    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.MemberPawn.IsValid() && Member.bIsAlive)
        {
            if (UPawnMovementComponent* MovementComp = Member.MemberPawn->GetMovementComponent())
            {
                AverageVelocity += MovementComp->Velocity;
                ValidMembers++;
            }
        }
    }

    if (ValidMembers > 0)
    {
        AverageVelocity /= ValidMembers;
        return AverageVelocity.GetSafeNormal();
    }

    return FVector::ZeroVector;
}

void UCrowd_HerdBehaviorComponent::UpdateHerdPositions()
{
    if (!AlphaPawn.IsValid())
        return;

    FVector AlphaLocation = AlphaPawn->GetActorLocation();

    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.MemberPawn.IsValid())
        {
            Member.DistanceFromAlpha = FVector::Dist(Member.MemberPawn->GetActorLocation(), AlphaLocation);
        }
    }
}

void UCrowd_HerdBehaviorComponent::CheckForThreats()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Simple threat detection - look for players or predators nearby
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == GetOwner())
            continue;

        float Distance = FVector::Dist(Actor->GetActorLocation(), OwnerLocation);
        
        // Check if this is a threat (player or predator)
        if (Distance < AlertRadius)
        {
            // Simple threat classification - anything with "TRex" or "Player" in name
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Player")) || ActorName.Contains(TEXT("Raptor")))
            {
                DetectThreat(Actor);
                break;
            }
        }
    }
}

void UCrowd_HerdBehaviorComponent::DetectThreat(AActor* ThreatActor)
{
    if (!ThreatActor)
        return;

    ThreatLocation = ThreatActor->GetActorLocation();
    LastThreatDetectionTime = GetWorld()->GetTimeSeconds();
    
    if (!bIsFleeingFromThreat)
    {
        StartFleeingBehavior();
    }

    UE_LOG(LogTemp, Warning, TEXT("Threat detected: %s at distance %f"), *ThreatActor->GetName(), 
           FVector::Dist(GetOwner()->GetActorLocation(), ThreatLocation));
}

void UCrowd_HerdBehaviorComponent::StartFleeingBehavior()
{
    bIsFleeingFromThreat = true;
    
    // Alert all herd members
    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.MemberPawn.IsValid())
        {
            if (UCrowd_HerdBehaviorComponent* MemberHerdComp = Member.MemberPawn->FindComponentByClass<UCrowd_HerdBehaviorComponent>())
            {
                MemberHerdComp->bIsFleeingFromThreat = true;
                MemberHerdComp->ThreatLocation = ThreatLocation;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Herd starting flee behavior"));
}

void UCrowd_HerdBehaviorComponent::StopFleeingBehavior()
{
    bIsFleeingFromThreat = false;
    ThreatLocation = FVector::ZeroVector;

    // Calm all herd members
    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.MemberPawn.IsValid())
        {
            if (UCrowd_HerdBehaviorComponent* MemberHerdComp = Member.MemberPawn->FindComponentByClass<UCrowd_HerdBehaviorComponent>())
            {
                MemberHerdComp->bIsFleeingFromThreat = false;
                MemberHerdComp->ThreatLocation = FVector::ZeroVector;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Herd calming down from threat"));
}

bool UCrowd_HerdBehaviorComponent::IsInDanger() const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return bIsFleeingFromThreat && (CurrentTime - LastThreatDetectionTime < 10.0f);
}