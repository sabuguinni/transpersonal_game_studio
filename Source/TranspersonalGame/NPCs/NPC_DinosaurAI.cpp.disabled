#include "NPC_DinosaurAI.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"

UNPC_DinosaurAI::UNPC_DinosaurAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
}

void UNPC_DinosaurAI::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeSpeciesStats();
    
    // Set territory center to current location
    if (AActor* Owner = GetOwner())
    {
        TerritoryCenter = Owner->GetActorLocation();
        GeneratePatrolPoints();
    }
    
    // Start AI update timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(StateUpdateTimer, this, &UNPC_DinosaurAI::UpdateAIState, 1.0f, true);
        World->GetTimerManager().SetTimer(PackUpdateTimer, this, &UNPC_DinosaurAI::UpdatePackBehavior, 2.0f, true);
        World->GetTimerManager().SetTimer(HungerTimer, this, &UNPC_DinosaurAI::UpdateHunger, 30.0f, true);
    }
    
    // Start with idle state
    SetState(ENPC_DinosaurState::Idle);
}

void UNPC_DinosaurAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Handle state-specific behavior
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Patrolling:
            MoveToNextPatrolPoint();
            break;
            
        case ENPC_DinosaurState::Hunting:
            HandleCombatBehavior();
            break;
            
        case ENPC_DinosaurState::PackHunting:
            HandlePackCommunication();
            break;
            
        default:
            break;
    }
}

void UNPC_DinosaurAI::SetState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        
        // Log state change for debugging
        if (AActor* Owner = GetOwner())
        {
            UE_LOG(LogTemp, Log, TEXT("Dinosaur %s changed state to %d"), 
                   *Owner->GetName(), (int32)CurrentState);
        }
    }
}

void UNPC_DinosaurAI::StartPatrol()
{
    SetState(ENPC_DinosaurState::Patrolling);
    CurrentPatrolIndex = 0;
}

void UNPC_DinosaurAI::StartHunting(AActor* Target)
{
    if (Target)
    {
        CurrentTarget = Target;
        SetState(ENPC_DinosaurState::Hunting);
    }
}

void UNPC_DinosaurAI::StartFleeing(AActor* Threat)
{
    if (Threat)
    {
        CurrentTarget = Threat;
        SetState(ENPC_DinosaurState::Fleeing);
        
        // Move away from threat
        if (AActor* Owner = GetOwner())
        {
            FVector FleeDirection = (Owner->GetActorLocation() - Threat->GetActorLocation()).GetSafeNormal();
            FVector FleeLocation = Owner->GetActorLocation() + (FleeDirection * DinosaurStats.TerritoryRadius);
            
            // Use AI controller to move
            if (APawn* OwnerPawn = Cast<APawn>(Owner))
            {
                if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
                {
                    AIController->MoveToLocation(FleeLocation);
                }
            }
        }
    }
}

AActor* UNPC_DinosaurAI::FindNearestPlayer()
{
    if (UWorld* World = GetWorld())
    {
        APlayerController* PC = World->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            return PC->GetPawn();
        }
    }
    return nullptr;
}

AActor* UNPC_DinosaurAI::FindNearestPrey()
{
    // Find smaller dinosaurs or other prey
    if (UWorld* World = GetWorld())
    {
        AActor* NearestPrey = nullptr;
        float NearestDistance = DinosaurStats.DetectionRange;
        
        for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
        {
            APawn* Pawn = *ActorItr;
            if (Pawn && Pawn != GetOwner())
            {
                // Check if it's a smaller dinosaur or player
                if (UNPC_DinosaurAI* OtherAI = Pawn->FindComponentByClass<UNPC_DinosaurAI>())
                {
                    // Predator logic: T-Rex hunts smaller dinosaurs
                    if (Species == ENPC_DinosaurSpecies::TRex && 
                        OtherAI->Species != ENPC_DinosaurSpecies::TRex)
                    {
                        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Pawn->GetActorLocation());
                        if (Distance < NearestDistance)
                        {
                            NearestDistance = Distance;
                            NearestPrey = Pawn;
                        }
                    }
                }
            }
        }
        
        return NearestPrey;
    }
    return nullptr;
}

bool UNPC_DinosaurAI::IsPlayerInRange(float Range)
{
    if (AActor* Player = FindNearestPlayer())
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
        return Distance <= Range;
    }
    return false;
}

void UNPC_DinosaurAI::UpdatePackBehavior()
{
    // Pack behavior for species like Velociraptors
    if (Species == ENPC_DinosaurSpecies::Velociraptor)
    {
        // Find other raptors nearby
        PackData.PackMembers.Empty();
        
        if (UWorld* World = GetWorld())
        {
            for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
            {
                APawn* Pawn = *ActorItr;
                if (Pawn && Pawn != GetOwner())
                {
                    if (UNPC_DinosaurAI* OtherAI = Pawn->FindComponentByClass<UNPC_DinosaurAI>())
                    {
                        if (OtherAI->Species == ENPC_DinosaurSpecies::Velociraptor)
                        {
                            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Pawn->GetActorLocation());
                            if (Distance <= PackData.PackCohesionRadius)
                            {
                                PackData.PackMembers.Add(Pawn);
                            }
                        }
                    }
                }
            }
            
            // Set pack leader (first found or self)
            if (PackData.PackMembers.Num() > 0 && !PackData.PackLeader)
            {
                PackData.PackLeader = GetOwner();
            }
            
            // Calculate pack center
            if (PackData.PackMembers.Num() > 0)
            {
                FVector CenterSum = GetOwner()->GetActorLocation();
                for (AActor* Member : PackData.PackMembers)
                {
                    CenterSum += Member->GetActorLocation();
                }
                PackData.PackCenterLocation = CenterSum / (PackData.PackMembers.Num() + 1);
            }
        }
    }
}

void UNPC_DinosaurAI::UpdateAIState()
{
    if (!GetOwner()) return;
    
    // Check for player proximity
    bool PlayerNearby = IsPlayerInRange(DinosaurStats.DetectionRange);
    AActor* Player = FindNearestPlayer();
    
    // State machine logic
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            if (PlayerNearby && DinosaurStats.Aggression > 30.0f)
            {
                StartHunting(Player);
            }
            else if (FMath::RandRange(0.0f, 100.0f) < 20.0f) // 20% chance to start patrolling
            {
                StartPatrol();
            }
            break;
            
        case ENPC_DinosaurState::Patrolling:
            if (PlayerNearby && DinosaurStats.Aggression > 50.0f)
            {
                StartHunting(Player);
            }
            else if (!IsInTerritory(GetOwner()->GetActorLocation()))
            {
                SetState(ENPC_DinosaurState::Idle);
            }
            break;
            
        case ENPC_DinosaurState::Hunting:
            if (!CurrentTarget || !PlayerNearby)
            {
                SetState(ENPC_DinosaurState::Patrolling);
                CurrentTarget = nullptr;
            }
            else if (IsPlayerInRange(DinosaurStats.AttackRange))
            {
                // Attack logic would go here
                UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s is attacking!"), *GetOwner()->GetName());
            }
            break;
            
        case ENPC_DinosaurState::Fleeing:
            if (!CurrentTarget || !IsPlayerInRange(DinosaurStats.DetectionRange * 1.5f))
            {
                SetState(ENPC_DinosaurState::Idle);
                CurrentTarget = nullptr;
            }
            break;
            
        default:
            break;
    }
    
    // Update fear and aggression based on health
    if (DinosaurStats.Health < DinosaurStats.MaxHealth * 0.3f)
    {
        DinosaurStats.Fear = FMath::Min(100.0f, DinosaurStats.Fear + 10.0f);
        DinosaurStats.Aggression = FMath::Max(0.0f, DinosaurStats.Aggression - 5.0f);
    }
}

void UNPC_DinosaurAI::UpdateHunger()
{
    // Increase hunger over time
    DinosaurStats.Hunger = FMath::Min(100.0f, DinosaurStats.Hunger + 5.0f);
    
    // High hunger increases aggression
    if (DinosaurStats.Hunger > 70.0f)
    {
        DinosaurStats.Aggression = FMath::Min(100.0f, DinosaurStats.Aggression + 10.0f);
    }
}

void UNPC_DinosaurAI::InitializeSpeciesStats()
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            DinosaurStats.MaxHealth = 500.0f;
            DinosaurStats.Health = 500.0f;
            DinosaurStats.Aggression = 80.0f;
            DinosaurStats.TerritoryRadius = 8000.0f;
            DinosaurStats.DetectionRange = 5000.0f;
            DinosaurStats.AttackRange = 800.0f;
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            DinosaurStats.MaxHealth = 150.0f;
            DinosaurStats.Health = 150.0f;
            DinosaurStats.Aggression = 90.0f;
            DinosaurStats.TerritoryRadius = 5000.0f;
            DinosaurStats.DetectionRange = 4000.0f;
            DinosaurStats.AttackRange = 400.0f;
            PackData.PackCohesionRadius = 3000.0f;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            DinosaurStats.MaxHealth = 400.0f;
            DinosaurStats.Health = 400.0f;
            DinosaurStats.Aggression = 30.0f;
            DinosaurStats.TerritoryRadius = 6000.0f;
            DinosaurStats.DetectionRange = 3000.0f;
            DinosaurStats.AttackRange = 600.0f;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            DinosaurStats.MaxHealth = 800.0f;
            DinosaurStats.Health = 800.0f;
            DinosaurStats.Aggression = 10.0f;
            DinosaurStats.TerritoryRadius = 10000.0f;
            DinosaurStats.DetectionRange = 2000.0f;
            DinosaurStats.AttackRange = 1000.0f;
            break;
            
        default:
            // Default stats
            DinosaurStats.MaxHealth = 200.0f;
            DinosaurStats.Health = 200.0f;
            DinosaurStats.Aggression = 50.0f;
            DinosaurStats.TerritoryRadius = 5000.0f;
            DinosaurStats.DetectionRange = 3000.0f;
            DinosaurStats.AttackRange = 500.0f;
            break;
    }
    
    DinosaurStats.MaxStamina = 100.0f;
    DinosaurStats.Stamina = 100.0f;
    DinosaurStats.Hunger = 0.0f;
    DinosaurStats.Fear = 0.0f;
}

void UNPC_DinosaurAI::GeneratePatrolPoints()
{
    PatrolPoints.Empty();
    
    // Generate 4-6 patrol points around territory center
    int32 NumPoints = FMath::RandRange(4, 6);
    float AngleStep = 360.0f / NumPoints;
    
    for (int32 i = 0; i < NumPoints; i++)
    {
        float Angle = i * AngleStep + FMath::RandRange(-30.0f, 30.0f);
        float Distance = FMath::RandRange(DinosaurStats.TerritoryRadius * 0.3f, DinosaurStats.TerritoryRadius * 0.8f);
        
        FVector Offset = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            0.0f
        );
        
        PatrolPoints.Add(TerritoryCenter + Offset);
    }
}

void UNPC_DinosaurAI::MoveToNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0) return;
    
    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    FVector CurrentLocation = Owner->GetActorLocation();
    FVector TargetPoint = PatrolPoints[CurrentPatrolIndex];
    
    // Check if we're close to the current patrol point
    if (FVector::Dist(CurrentLocation, TargetPoint) < 500.0f)
    {
        // Move to next patrol point
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    }
    
    // Move towards current patrol point
    if (APawn* OwnerPawn = Cast<APawn>(Owner))
    {
        if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
        {
            AIController->MoveToLocation(TargetPoint);
        }
    }
}

bool UNPC_DinosaurAI::IsInTerritory(const FVector& Location) const
{
    return FVector::Dist(Location, TerritoryCenter) <= DinosaurStats.TerritoryRadius;
}

float UNPC_DinosaurAI::GetDistanceToPlayer() const
{
    if (AActor* Player = FindNearestPlayer())
    {
        return FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    }
    return -1.0f;
}

void UNPC_DinosaurAI::HandleCombatBehavior()
{
    if (!CurrentTarget || !GetOwner()) return;
    
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    if (DistanceToTarget <= DinosaurStats.AttackRange)
    {
        // Attack target
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s attacks target!"), *GetOwner()->GetName());
        
        // Deal damage logic would go here
        // For now, just log the attack
    }
    else if (DistanceToTarget <= DinosaurStats.DetectionRange)
    {
        // Chase target
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
            {
                AIController->MoveToLocation(CurrentTarget->GetActorLocation());
            }
        }
    }
}

void UNPC_DinosaurAI::HandlePackCommunication()
{
    // Pack hunting coordination for Velociraptors
    if (Species == ENPC_DinosaurSpecies::Velociraptor && PackData.PackMembers.Num() > 0)
    {
        // If this is the pack leader, coordinate the hunt
        if (PackData.PackLeader == GetOwner() && CurrentTarget)
        {
            // Signal other pack members to hunt the same target
            for (AActor* Member : PackData.PackMembers)
            {
                if (UNPC_DinosaurAI* MemberAI = Member->FindComponentByClass<UNPC_DinosaurAI>())
                {
                    MemberAI->StartHunting(CurrentTarget);
                }
            }
        }
    }
}