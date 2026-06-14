#include "NPCBehaviorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    CurrentState = ENPC_BehaviorState::Idle;
    DinosaurType = ENPC_DinosaurType::TRex;
    PatrolCenter = FVector::ZeroVector;
    CurrentTarget = FVector::ZeroVector;
    PlayerTarget = nullptr;
    DistanceToPlayer = 0.0f;
    bIsPackHunter = false;
    StateChangeTimer = 0.0f;
    PatrolWaitTimer = 0.0f;
    bHasValidTarget = false;
    LastKnownPlayerLocation = FVector::ZeroVector;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize patrol center to current location
    if (AActor* Owner = GetOwner())
    {
        PatrolCenter = Owner->GetActorLocation();
        CurrentTarget = PatrolCenter;
    }
    
    // Setup dinosaur-specific stats
    SetupDinosaurStats(DinosaurType);
    
    // Start behavior update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(BehaviorUpdateTimer, 
            [this]() { UpdateBehavior(0.1f); }, 
            0.1f, true);
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update timers
    StateChangeTimer += DeltaTime;
    PatrolWaitTimer += DeltaTime;
    
    // Find nearest player
    FindNearestPlayer();
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateChangeTimer = 0.0f;
        
        // Debug logging
        if (AActor* Owner = GetOwner())
        {
            UE_LOG(LogTemp, Log, TEXT("%s changed state to %d"), 
                *Owner->GetName(), (int32)NewState);
        }
    }
}

void UNPCBehaviorComponent::InitializeBehavior(ENPC_DinosaurType Type)
{
    DinosaurType = Type;
    SetupDinosaurStats(Type);
    SetBehaviorState(ENPC_BehaviorState::Patrol);
}

void UNPCBehaviorComponent::UpdateBehavior(float DeltaTime)
{
    if (!GetOwner()) return;
    
    // Pack coordination for pack hunters
    if (bIsPackHunter && PackMembers.Num() > 0)
    {
        CoordinatePackBehavior();
    }
    
    // State machine
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
            if (StateChangeTimer > 2.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Patrol);
            }
            break;
            
        case ENPC_BehaviorState::Patrol:
            PatrolBehavior();
            if (PlayerTarget && DistanceToPlayer <= BehaviorStats.ChaseDistance)
            {
                SetBehaviorState(ENPC_BehaviorState::Chase);
            }
            break;
            
        case ENPC_BehaviorState::Chase:
            ChaseBehavior();
            if (DistanceToPlayer <= BehaviorStats.AttackDistance)
            {
                SetBehaviorState(ENPC_BehaviorState::Attack);
            }
            else if (DistanceToPlayer > BehaviorStats.ChaseDistance * 1.5f)
            {
                SetBehaviorState(ENPC_BehaviorState::Patrol);
            }
            break;
            
        case ENPC_BehaviorState::Attack:
            AttackBehavior();
            if (DistanceToPlayer > BehaviorStats.AttackDistance * 1.2f)
            {
                SetBehaviorState(ENPC_BehaviorState::Chase);
            }
            break;
            
        case ENPC_BehaviorState::Flee:
            FleeBehavior();
            if (DistanceToPlayer > BehaviorStats.FleeDistance * 2.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Patrol);
            }
            break;
            
        case ENPC_BehaviorState::Dead:
            // Do nothing when dead
            break;
    }
}

void UNPCBehaviorComponent::FindNearestPlayer()
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                PlayerTarget = PlayerPawn;
                DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), 
                    PlayerPawn->GetActorLocation());
                LastKnownPlayerLocation = PlayerPawn->GetActorLocation();
            }
        }
    }
}

void UNPCBehaviorComponent::PatrolBehavior()
{
    if (!GetOwner()) return;
    
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    float DistanceToTarget = FVector::Dist(CurrentLocation, CurrentTarget);
    
    // If reached patrol point or no target, find new patrol point
    if (DistanceToTarget < 200.0f || !bHasValidTarget)
    {
        if (PatrolWaitTimer > 3.0f)
        {
            // Generate random patrol point within radius
            FVector RandomDirection = FMath::VRand();
            RandomDirection.Z = 0.0f; // Keep on ground level
            RandomDirection.Normalize();
            
            float RandomDistance = FMath::RandRange(500.0f, BehaviorStats.PatrolRadius);
            CurrentTarget = PatrolCenter + (RandomDirection * RandomDistance);
            bHasValidTarget = true;
            PatrolWaitTimer = 0.0f;
        }
    }
}

void UNPCBehaviorComponent::ChaseBehavior()
{
    if (PlayerTarget)
    {
        CurrentTarget = PlayerTarget->GetActorLocation();
        bHasValidTarget = true;
        
        // Update last known location
        LastKnownPlayerLocation = CurrentTarget;
    }
    else if (LastKnownPlayerLocation != FVector::ZeroVector)
    {
        // Chase last known location
        CurrentTarget = LastKnownPlayerLocation;
        bHasValidTarget = true;
    }
}

void UNPCBehaviorComponent::AttackBehavior()
{
    if (PlayerTarget)
    {
        // Face the player
        if (AActor* Owner = GetOwner())
        {
            FVector Direction = (PlayerTarget->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
            FRotator NewRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
            Owner->SetActorRotation(NewRotation);
        }
        
        // Attack logic would go here (damage dealing, animation triggers, etc.)
        UE_LOG(LogTemp, Log, TEXT("%s is attacking player!"), 
            *GetOwner()->GetName());
    }
}

void UNPCBehaviorComponent::FleeBehavior()
{
    if (PlayerTarget && GetOwner())
    {
        // Run away from player
        FVector FleeDirection = (GetOwner()->GetActorLocation() - PlayerTarget->GetActorLocation()).GetSafeNormal();
        CurrentTarget = GetOwner()->GetActorLocation() + (FleeDirection * BehaviorStats.FleeDistance);
        bHasValidTarget = true;
    }
}

void UNPCBehaviorComponent::SetupDinosaurStats(ENPC_DinosaurType Type)
{
    switch (Type)
    {
        case ENPC_DinosaurType::TRex:
            BehaviorStats.PatrolRadius = 5000.0f;
            BehaviorStats.ChaseDistance = 3000.0f;
            BehaviorStats.AttackDistance = 400.0f;
            BehaviorStats.MovementSpeed = 800.0f;
            BehaviorStats.Aggression = 0.9f;
            BehaviorStats.Fear = 0.1f;
            bIsPackHunter = false;
            break;
            
        case ENPC_DinosaurType::Velociraptor:
            BehaviorStats.PatrolRadius = 2000.0f;
            BehaviorStats.ChaseDistance = 1500.0f;
            BehaviorStats.AttackDistance = 200.0f;
            BehaviorStats.MovementSpeed = 1200.0f;
            BehaviorStats.Aggression = 0.8f;
            BehaviorStats.Fear = 0.3f;
            bIsPackHunter = true;
            break;
            
        case ENPC_DinosaurType::Triceratops:
            BehaviorStats.PatrolRadius = 1500.0f;
            BehaviorStats.ChaseDistance = 1000.0f;
            BehaviorStats.AttackDistance = 300.0f;
            BehaviorStats.MovementSpeed = 400.0f;
            BehaviorStats.Aggression = 0.5f;
            BehaviorStats.Fear = 0.4f;
            bIsPackHunter = false;
            break;
            
        case ENPC_DinosaurType::Brachiosaurus:
            BehaviorStats.PatrolRadius = 3000.0f;
            BehaviorStats.ChaseDistance = 800.0f;
            BehaviorStats.AttackDistance = 500.0f;
            BehaviorStats.MovementSpeed = 300.0f;
            BehaviorStats.Aggression = 0.2f;
            BehaviorStats.Fear = 0.7f;
            bIsPackHunter = false;
            break;
            
        case ENPC_DinosaurType::Ankylosaurus:
            BehaviorStats.PatrolRadius = 1000.0f;
            BehaviorStats.ChaseDistance = 600.0f;
            BehaviorStats.AttackDistance = 250.0f;
            BehaviorStats.MovementSpeed = 200.0f;
            BehaviorStats.Aggression = 0.3f;
            BehaviorStats.Fear = 0.5f;
            bIsPackHunter = false;
            break;
    }
}

void UNPCBehaviorComponent::AddPackMember(AActor* Member)
{
    if (Member && !PackMembers.Contains(Member))
    {
        PackMembers.Add(Member);
    }
}

void UNPCBehaviorComponent::CoordinatePackBehavior()
{
    // Pack coordination logic
    if (CurrentState == ENPC_BehaviorState::Chase || CurrentState == ENPC_BehaviorState::Attack)
    {
        // Share target information with pack members
        for (AActor* PackMember : PackMembers)
        {
            if (PackMember && IsValid(PackMember))
            {
                if (UNPCBehaviorComponent* PackBehavior = PackMember->FindComponentByClass<UNPCBehaviorComponent>())
                {
                    if (PackBehavior->CurrentState == ENPC_BehaviorState::Patrol)
                    {
                        PackBehavior->SetBehaviorState(ENPC_BehaviorState::Chase);
                        PackBehavior->CurrentTarget = this->CurrentTarget;
                    }
                }
            }
        }
    }
}