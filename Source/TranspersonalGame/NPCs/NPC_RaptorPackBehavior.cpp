#include "NPC_RaptorPackBehavior.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UNPC_RaptorPackBehavior::UNPC_RaptorPackBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize pack behavior parameters
    PackCohesionRadius = 2000.0f;
    HuntingRadius = 5000.0f;
    FleeDistance = 8000.0f;
    HuntingCooldown = 30.0f;
    CallRadius = 3000.0f;
    CallCooldown = 5.0f;

    CurrentPackState = ENPC_RaptorPackState::Patrolling;
    PackCenterLocation = FVector::ZeroVector;
    AlphaRaptor = nullptr;
    CurrentTarget = nullptr;
    LastHuntTime = 0.0f;
    LastCallTime = 0.0f;
    bIsHunting = false;
}

void UNPC_RaptorPackBehavior::BeginPlay()
{
    Super::BeginPlay();

    // Initialize pack center to owner's location
    if (GetOwner())
    {
        PackCenterLocation = GetOwner()->GetActorLocation();
    }

    // Set up timers for pack behavior
    GetWorld()->GetTimerManager().SetTimer(PatrolTimer, [this]()
    {
        if (CurrentPackState == ENPC_RaptorPackState::Patrolling)
        {
            UpdatePatrolBehavior();
        }
    }, 2.0f, true);

    GetWorld()->GetTimerManager().SetTimer(CommunicationTimer, [this]()
    {
        if (FMath::RandRange(0.0f, 1.0f) < 0.3f) // 30% chance to call
        {
            EmitPackCall();
        }
    }, 8.0f, true);
}

void UNPC_RaptorPackBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner() || !GetWorld())
    {
        return;
    }

    // Update pack cohesion
    UpdatePackCohesion();

    // Check for threats
    CheckForThreats();

    // Update behavior based on current state
    switch (CurrentPackState)
    {
        case ENPC_RaptorPackState::Hunting:
            UpdateHuntingBehavior();
            break;
        case ENPC_RaptorPackState::Patrolling:
            UpdatePatrolBehavior();
            break;
        case ENPC_RaptorPackState::Fleeing:
            UpdateFleeingBehavior();
            break;
        case ENPC_RaptorPackState::Feeding:
            // Feeding behavior - stay near kill site
            if (FMath::RandRange(0.0f, 1.0f) < 0.01f) // 1% chance per tick to finish feeding
            {
                SetPackState(ENPC_RaptorPackState::Patrolling);
            }
            break;
        case ENPC_RaptorPackState::Resting:
            // Resting behavior - minimal movement
            if (FMath::RandRange(0.0f, 1.0f) < 0.005f) // 0.5% chance per tick to wake up
            {
                SetPackState(ENPC_RaptorPackState::Patrolling);
            }
            break;
    }

    // Update pack formation
    UpdatePackFormation();
}

void UNPC_RaptorPackBehavior::InitializePack(const TArray<APawn*>& Raptors)
{
    PackMembers.Empty();
    
    for (APawn* Raptor : Raptors)
    {
        if (Raptor)
        {
            FNPC_RaptorPackMember NewMember;
            NewMember.RaptorPawn = Raptor;
            NewMember.LastKnownPosition = Raptor->GetActorLocation();
            NewMember.bIsAlive = true;
            PackMembers.Add(NewMember);
        }
    }

    // Set the first raptor as alpha if no alpha is set
    if (!AlphaRaptor && PackMembers.Num() > 0)
    {
        AlphaRaptor = PackMembers[0].RaptorPawn;
    }

    AssignRoles();
    UE_LOG(LogTemp, Warning, TEXT("Raptor pack initialized with %d members"), PackMembers.Num());
}

void UNPC_RaptorPackBehavior::SetPackState(ENPC_RaptorPackState NewState)
{
    if (CurrentPackState != NewState)
    {
        CurrentPackState = NewState;
        
        switch (NewState)
        {
            case ENPC_RaptorPackState::Hunting:
                bIsHunting = true;
                LastHuntTime = GetWorld()->GetTimeSeconds();
                break;
            case ENPC_RaptorPackState::Patrolling:
                bIsHunting = false;
                CurrentTarget = nullptr;
                break;
            case ENPC_RaptorPackState::Fleeing:
                bIsHunting = false;
                CurrentTarget = nullptr;
                break;
        }

        UE_LOG(LogTemp, Log, TEXT("Raptor pack state changed to: %d"), (int32)NewState);
    }
}

void UNPC_RaptorPackBehavior::UpdatePackCohesion()
{
    if (PackMembers.Num() == 0)
    {
        return;
    }

    // Calculate pack center
    FVector CenterSum = FVector::ZeroVector;
    int32 AliveMembers = 0;

    for (FNPC_RaptorPackMember& Member : PackMembers)
    {
        if (Member.bIsAlive && Member.RaptorPawn)
        {
            CenterSum += Member.RaptorPawn->GetActorLocation();
            Member.LastKnownPosition = Member.RaptorPawn->GetActorLocation();
            AliveMembers++;
        }
    }

    if (AliveMembers > 0)
    {
        PackCenterLocation = CenterSum / AliveMembers;
    }

    // Update distances from alpha
    if (AlphaRaptor)
    {
        FVector AlphaLocation = AlphaRaptor->GetActorLocation();
        for (FNPC_RaptorPackMember& Member : PackMembers)
        {
            if (Member.bIsAlive && Member.RaptorPawn)
            {
                Member.DistanceFromAlpha = FVector::Dist(Member.RaptorPawn->GetActorLocation(), AlphaLocation);
            }
        }
    }
}

void UNPC_RaptorPackBehavior::StartHunt(APawn* Target)
{
    if (!Target)
    {
        return;
    }

    CurrentTarget = Target;
    SetPackState(ENPC_RaptorPackState::Hunting);
    
    // Emit hunting call to coordinate pack
    EmitPackCall();
    
    // Execute flanking maneuver
    ExecuteFlankingManeuver();

    UE_LOG(LogTemp, Warning, TEXT("Raptor pack starting hunt on target: %s"), *Target->GetName());
}

void UNPC_RaptorPackBehavior::EndHunt()
{
    if (CurrentTarget)
    {
        UE_LOG(LogTemp, Log, TEXT("Raptor pack ending hunt"));
        
        // Check if target was eliminated
        if (CurrentTarget->GetActorLocation().Z < -1000.0f) // Simple "death" check
        {
            SetPackState(ENPC_RaptorPackState::Feeding);
        }
        else
        {
            SetPackState(ENPC_RaptorPackState::Patrolling);
        }
        
        CurrentTarget = nullptr;
    }
}

void UNPC_RaptorPackBehavior::ExecuteFlankingManeuver()
{
    if (!CurrentTarget || PackMembers.Num() < 2)
    {
        return;
    }

    FVector TargetLocation = CurrentTarget->GetActorLocation();
    float FlankingRadius = 1500.0f;

    // Assign flanking positions to pack members
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (PackMembers[i].bIsAlive && PackMembers[i].RaptorPawn)
        {
            float Angle = (360.0f / PackMembers.Num()) * i;
            FVector FlankPosition = TargetLocation + FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)) * FlankingRadius,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * FlankingRadius,
                0.0f
            );

            // Move raptor to flanking position (simplified - in real implementation would use AI movement)
            if (PackMembers[i].RaptorPawn->GetActorLocation().Z > -500.0f) // Basic "alive" check
            {
                FVector Direction = (FlankPosition - PackMembers[i].RaptorPawn->GetActorLocation()).GetSafeNormal();
                FVector NewLocation = PackMembers[i].RaptorPawn->GetActorLocation() + Direction * 100.0f;
                PackMembers[i].RaptorPawn->SetActorLocation(NewLocation);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Raptor pack executing flanking maneuver"));
}

void UNPC_RaptorPackBehavior::EmitPackCall()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastCallTime < CallCooldown)
    {
        return;
    }

    LastCallTime = CurrentTime;

    // Notify all pack members within call radius
    FVector CallLocation = GetOwner()->GetActorLocation();
    
    for (FNPC_RaptorPackMember& Member : PackMembers)
    {
        if (Member.bIsAlive && Member.RaptorPawn)
        {
            float Distance = FVector::Dist(Member.RaptorPawn->GetActorLocation(), CallLocation);
            if (Distance <= CallRadius)
            {
                // In a full implementation, this would trigger AI behavior tree responses
                UE_LOG(LogTemp, Log, TEXT("Raptor %s responding to pack call"), *Member.RaptorPawn->GetName());
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Raptor pack call emitted"));
}

void UNPC_RaptorPackBehavior::RespondToPackCall(const FVector& CallLocation)
{
    // Move towards call location if not currently hunting
    if (CurrentPackState != ENPC_RaptorPackState::Hunting)
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        FVector Direction = (CallLocation - OwnerLocation).GetSafeNormal();
        FVector NewLocation = OwnerLocation + Direction * 200.0f;
        GetOwner()->SetActorLocation(NewLocation);
        
        UE_LOG(LogTemp, Log, TEXT("Raptor responding to pack call"));
    }
}

void UNPC_RaptorPackBehavior::UpdateHuntingBehavior()
{
    if (!CurrentTarget)
    {
        EndHunt();
        return;
    }

    // Check if target is still alive and within hunting range
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    if (DistanceToTarget > HuntingRadius * 2.0f)
    {
        // Target too far, abandon hunt
        EndHunt();
        return;
    }

    // Continue hunting behavior
    float HuntDuration = GetWorld()->GetTimeSeconds() - LastHuntTime;
    if (HuntDuration > 60.0f) // Hunt for maximum 60 seconds
    {
        EndHunt();
    }
}

void UNPC_RaptorPackBehavior::UpdatePatrolBehavior()
{
    if (!GetOwner())
    {
        return;
    }

    // Simple patrol behavior - move in random directions within territory
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector PatrolDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ).GetSafeNormal();

    FVector NewLocation = CurrentLocation + PatrolDirection * 300.0f;
    GetOwner()->SetActorLocation(NewLocation);
}

void UNPC_RaptorPackBehavior::UpdateFleeingBehavior()
{
    // Move away from threats
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector FleeDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ).GetSafeNormal();

    FVector FleeLocation = CurrentLocation + FleeDirection * 500.0f;
    GetOwner()->SetActorLocation(FleeLocation);

    // Check if safe to stop fleeing
    if (FMath::RandRange(0.0f, 1.0f) < 0.02f) // 2% chance per tick to stop fleeing
    {
        SetPackState(ENPC_RaptorPackState::Patrolling);
    }
}

void UNPC_RaptorPackBehavior::UpdatePackFormation()
{
    if (!AlphaRaptor || PackMembers.Num() < 2)
    {
        return;
    }

    FVector AlphaLocation = AlphaRaptor->GetActorLocation();
    
    // Keep pack members within cohesion radius of alpha
    for (FNPC_RaptorPackMember& Member : PackMembers)
    {
        if (Member.bIsAlive && Member.RaptorPawn && Member.RaptorPawn != AlphaRaptor)
        {
            float DistanceFromAlpha = FVector::Dist(Member.RaptorPawn->GetActorLocation(), AlphaLocation);
            
            if (DistanceFromAlpha > PackCohesionRadius)
            {
                // Move towards alpha
                FVector Direction = (AlphaLocation - Member.RaptorPawn->GetActorLocation()).GetSafeNormal();
                FVector NewLocation = Member.RaptorPawn->GetActorLocation() + Direction * 150.0f;
                Member.RaptorPawn->SetActorLocation(NewLocation);
            }
        }
    }
}

void UNPC_RaptorPackBehavior::CheckForThreats()
{
    // Look for player or large dinosaurs nearby
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (PlayerPawn)
    {
        float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
        
        if (DistanceToPlayer < HuntingRadius && CurrentPackState == ENPC_RaptorPackState::Patrolling)
        {
            // Consider hunting player if pack is strong enough
            if (PackMembers.Num() >= 2 && FMath::RandRange(0.0f, 1.0f) < 0.3f)
            {
                StartHunt(PlayerPawn);
            }
        }
        else if (DistanceToPlayer < FleeDistance * 0.3f && PackMembers.Num() < 2)
        {
            // Flee if pack is too small
            SetPackState(ENPC_RaptorPackState::Fleeing);
        }
    }
}

void UNPC_RaptorPackBehavior::AssignRoles()
{
    // Simple role assignment - largest raptor becomes alpha
    APawn* LargestRaptor = nullptr;
    float LargestSize = 0.0f;

    for (const FNPC_RaptorPackMember& Member : PackMembers)
    {
        if (Member.bIsAlive && Member.RaptorPawn)
        {
            FVector Scale = Member.RaptorPawn->GetActorScale3D();
            float Size = Scale.X * Scale.Y * Scale.Z;
            
            if (Size > LargestSize)
            {
                LargestSize = Size;
                LargestRaptor = Member.RaptorPawn;
            }
        }
    }

    if (LargestRaptor)
    {
        AlphaRaptor = LargestRaptor;
        UE_LOG(LogTemp, Log, TEXT("Alpha raptor assigned: %s"), *AlphaRaptor->GetName());
    }
}