#include "Crowd/CrowdStampedeController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACrowd_StampedeController::ACrowd_StampedeController()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentHerdState = ECrowd_StampedeState::Grazing;
    StampedeTimer = 0.0f;
    MaxStampedeDuration = 30.0f;
    StampedeDirection = FVector::ForwardVector;
}

void ACrowd_StampedeController::BeginPlay()
{
    Super::BeginPlay();
    CurrentHerdState = ECrowd_StampedeState::Grazing;
    UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: Herd initialized with %d members"), HerdMembers.Num());
}

void ACrowd_StampedeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateHerdBehavior(DeltaTime);
    ApplyFlockingToMembers(DeltaTime);
    CheckPlayerProximity();
}

void ACrowd_StampedeController::TriggerStampede(FVector PanicOrigin, FVector FleeDirection)
{
    if (CurrentHerdState == ECrowd_StampedeState::Stampeding)
    {
        return;
    }

    StampedeDirection = FleeDirection.GetSafeNormal();
    StampedeTimer = 0.0f;
    TransitionToState(ECrowd_StampedeState::Stampeding);

    // Raise fear level on all members
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        Member.FearLevel = 1.0f;
        Member.State = ECrowd_StampedeState::Stampeding;
    }

    UE_LOG(LogTemp, Warning, TEXT("CrowdStampedeController: STAMPEDE TRIGGERED — %d animals fleeing!"), HerdMembers.Num());
}

void ACrowd_StampedeController::AddHerdMember(AActor* Member)
{
    if (!Member)
    {
        return;
    }

    FCrowd_HerdMember NewMember;
    NewMember.MemberActor = Member;
    NewMember.State = CurrentHerdState;
    NewMember.FearLevel = 0.0f;
    NewMember.CurrentVelocity = FVector::ZeroVector;
    HerdMembers.Add(NewMember);
}

void ACrowd_StampedeController::SetStampedeConfig(const FCrowd_StampedeConfig& Config)
{
    StampedeConfig = Config;
}

void ACrowd_StampedeController::DisbandHerd()
{
    TransitionToState(ECrowd_StampedeState::Dispersing);
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        Member.FearLevel = FMath::Max(0.0f, Member.FearLevel - 0.5f);
        Member.State = ECrowd_StampedeState::Dispersing;
    }
    UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: Herd dispersing"));
}

FVector ACrowd_StampedeController::ComputeFlockingForce(const FCrowd_HerdMember& Member)
{
    if (!Member.MemberActor)
    {
        return FVector::ZeroVector;
    }

    FVector SeparationForce = FVector::ZeroVector;
    FVector CohesionForce = FVector::ZeroVector;
    FVector AlignmentForce = FVector::ZeroVector;
    int32 NeighborCount = 0;
    FVector CenterOfMass = FVector::ZeroVector;

    FVector MyPos = Member.MemberActor->GetActorLocation();

    for (const FCrowd_HerdMember& Other : HerdMembers)
    {
        if (!Other.MemberActor || Other.MemberActor == Member.MemberActor)
        {
            continue;
        }

        FVector OtherPos = Other.MemberActor->GetActorLocation();
        float Distance = FVector::Dist(MyPos, OtherPos);

        if (Distance < 300.0f && Distance > 0.0f)
        {
            // Separation — push away from close neighbors
            FVector AwayDir = (MyPos - OtherPos).GetSafeNormal();
            SeparationForce += AwayDir * (300.0f - Distance) / 300.0f;
        }

        if (Distance < 1000.0f)
        {
            CenterOfMass += OtherPos;
            AlignmentForce += Other.CurrentVelocity;
            NeighborCount++;
        }
    }

    if (NeighborCount > 0)
    {
        CenterOfMass /= NeighborCount;
        CohesionForce = (CenterOfMass - MyPos).GetSafeNormal();
        AlignmentForce = (AlignmentForce / NeighborCount).GetSafeNormal();
    }

    FVector TotalForce =
        SeparationForce * StampedeConfig.SeparationForce +
        CohesionForce * StampedeConfig.CohesionForce +
        AlignmentForce * StampedeConfig.AlignmentForce;

    return TotalForce;
}

void ACrowd_StampedeController::UpdateHerdBehavior(float DeltaTime)
{
    if (CurrentHerdState == ECrowd_StampedeState::Stampeding)
    {
        StampedeTimer += DeltaTime;
        if (StampedeTimer >= MaxStampedeDuration)
        {
            DisbandHerd();
        }
    }

    // Decay fear over time
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.FearLevel > 0.0f)
        {
            Member.FearLevel = FMath::Max(0.0f, Member.FearLevel - DeltaTime * 0.1f);
        }
    }
}

void ACrowd_StampedeController::ApplyFlockingToMembers(float DeltaTime)
{
    if (CurrentHerdState != ECrowd_StampedeState::Stampeding &&
        CurrentHerdState != ECrowd_StampedeState::Fleeing)
    {
        return;
    }

    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (!Member.MemberActor)
        {
            continue;
        }

        FVector FlockForce = ComputeFlockingForce(Member);
        FVector DriveForce = StampedeDirection * StampedeConfig.StampedeSpeed;
        FVector TotalVelocity = DriveForce + FlockForce;

        // Clamp to max stampede speed
        if (TotalVelocity.SizeSquared() > FMath::Square(StampedeConfig.StampedeSpeed * 1.5f))
        {
            TotalVelocity = TotalVelocity.GetSafeNormal() * StampedeConfig.StampedeSpeed * 1.5f;
        }

        Member.CurrentVelocity = FMath::VInterpTo(Member.CurrentVelocity, TotalVelocity, DeltaTime, 3.0f);

        FVector NewLocation = Member.MemberActor->GetActorLocation() + Member.CurrentVelocity * DeltaTime;
        Member.MemberActor->SetActorLocation(NewLocation, true);

        // Face direction of movement
        if (!Member.CurrentVelocity.IsNearlyZero())
        {
            FRotator FaceDir = Member.CurrentVelocity.Rotation();
            Member.MemberActor->SetActorRotation(FaceDir);
        }
    }
}

void ACrowd_StampedeController::CheckPlayerProximity()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerPos = PlayerPawn->GetActorLocation();

    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        if (!Member.MemberActor)
        {
            continue;
        }

        float DistToPlayer = FVector::Dist(Member.MemberActor->GetActorLocation(), PlayerPos);
        if (DistToPlayer < StampedeConfig.PanicRadius && CurrentHerdState != ECrowd_StampedeState::Stampeding)
        {
            // Player too close — trigger alert then stampede
            TransitionToState(ECrowd_StampedeState::Alert);
            FVector FleeDir = (Member.MemberActor->GetActorLocation() - PlayerPos).GetSafeNormal();
            TriggerStampede(PlayerPos, FleeDir);
            break;
        }
    }
}

void ACrowd_StampedeController::TransitionToState(ECrowd_StampedeState NewState)
{
    if (CurrentHerdState == NewState)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: State %d -> %d"),
        (int32)CurrentHerdState, (int32)NewState);

    CurrentHerdState = NewState;

    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        Member.State = NewState;
    }
}
