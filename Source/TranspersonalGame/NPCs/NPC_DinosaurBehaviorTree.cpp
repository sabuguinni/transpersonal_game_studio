#include "NPC_DinosaurBehaviorTree.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPC_DinosaurBehaviorTree::UNPC_DinosaurBehaviorTree()
{
    DinosaurState = FNPC_DinosaurState();
    LastBehaviorUpdate = 0.0f;
    BehaviorUpdateInterval = 0.5f;
    PatrolPointChangeTime = 10.0f;
    LastPatrolChange = 0.0f;
    bIsPackLeader = false;
}

void UNPC_DinosaurBehaviorTree::InitializeBehaviorTree(AAIController* AIController, EDinosaurSpecies Species)
{
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurBehaviorTree: AIController is null"));
        return;
    }

    OwnerController = AIController;
    DinosaurState.Species = Species;
    
    // Get blackboard component
    BlackboardComp = AIController->GetBlackboardComponent();
    BehaviorTreeComp = AIController->GetBehaviorTreeComponent();

    if (BlackboardComp.IsValid())
    {
        // Initialize blackboard keys based on species
        BlackboardComp->SetValueAsEnum(TEXT("Species"), static_cast<uint8>(Species));
        BlackboardComp->SetValueAsFloat(TEXT("Health"), DinosaurState.Health);
        BlackboardComp->SetValueAsFloat(TEXT("Hunger"), DinosaurState.Hunger);
        BlackboardComp->SetValueAsFloat(TEXT("Aggression"), DinosaurState.Aggression);
        BlackboardComp->SetValueAsBool(TEXT("IsHunting"), DinosaurState.bIsHunting);
        BlackboardComp->SetValueAsBool(TEXT("IsPatrolling"), DinosaurState.bIsPatrolling);
        BlackboardComp->SetValueAsVector(TEXT("TerritoryCenter"), DinosaurState.TerritoryCenter);
        BlackboardComp->SetValueAsFloat(TEXT("TerritoryRadius"), DinosaurState.TerritoryRadius);
    }

    // Set species-specific parameters
    switch (Species)
    {
        case EDinosaurSpecies::TRex:
            DinosaurState.Health = 150.0f;
            DinosaurState.Aggression = 80.0f;
            DinosaurState.TerritoryRadius = 8000.0f;
            break;
        case EDinosaurSpecies::Raptor:
            DinosaurState.Health = 60.0f;
            DinosaurState.Aggression = 70.0f;
            DinosaurState.TerritoryRadius = 6000.0f;
            break;
        case EDinosaurSpecies::Brachiosaurus:
            DinosaurState.Health = 200.0f;
            DinosaurState.Aggression = 10.0f;
            DinosaurState.TerritoryRadius = 12000.0f;
            break;
        default:
            break;
    }

    // Set territory center to current location
    if (APawn* OwnerPawn = AIController->GetPawn())
    {
        DinosaurState.TerritoryCenter = OwnerPawn->GetActorLocation();
        if (BlackboardComp.IsValid())
        {
            BlackboardComp->SetValueAsVector(TEXT("TerritoryCenter"), DinosaurState.TerritoryCenter);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorTree initialized for species %d"), static_cast<int32>(Species));
}

void UNPC_DinosaurBehaviorTree::UpdateDinosaurState(float DeltaTime)
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    if (CurrentTime - LastBehaviorUpdate < BehaviorUpdateInterval)
    {
        return;
    }
    
    LastBehaviorUpdate = CurrentTime;

    // Update hunger and aggression over time
    UpdateHungerAndAggression(DeltaTime);

    // Execute species-specific behavior
    switch (DinosaurState.Species)
    {
        case EDinosaurSpecies::TRex:
            ExecuteTRexBehavior(DeltaTime);
            break;
        case EDinosaurSpecies::Raptor:
            ExecuteRaptorBehavior(DeltaTime);
            break;
        case EDinosaurSpecies::Brachiosaurus:
            ExecuteBrachiosaurusBehavior(DeltaTime);
            break;
        default:
            break;
    }

    // Update blackboard with current state
    if (BlackboardComp.IsValid())
    {
        BlackboardComp->SetValueAsFloat(TEXT("Health"), DinosaurState.Health);
        BlackboardComp->SetValueAsFloat(TEXT("Hunger"), DinosaurState.Hunger);
        BlackboardComp->SetValueAsFloat(TEXT("Aggression"), DinosaurState.Aggression);
        BlackboardComp->SetValueAsBool(TEXT("IsHunting"), DinosaurState.bIsHunting);
        BlackboardComp->SetValueAsBool(TEXT("IsPatrolling"), DinosaurState.bIsPatrolling);
    }
}

bool UNPC_DinosaurBehaviorTree::CanSeeTarget(AActor* Target) const
{
    if (!Target || !OwnerController.IsValid())
    {
        return false;
    }

    APawn* OwnerPawn = OwnerController->GetPawn();
    if (!OwnerPawn)
    {
        return false;
    }

    FVector Start = OwnerPawn->GetActorLocation();
    FVector End = Target->GetActorLocation();
    float Distance = FVector::Dist(Start, End);

    // Species-specific sight range
    float SightRange = 3000.0f;
    switch (DinosaurState.Species)
    {
        case EDinosaurSpecies::TRex:
            SightRange = 5000.0f;
            break;
        case EDinosaurSpecies::Raptor:
            SightRange = 4000.0f;
            break;
        case EDinosaurSpecies::Brachiosaurus:
            SightRange = 2000.0f;
            break;
    }

    if (Distance > SightRange)
    {
        return false;
    }

    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerPawn);
    QueryParams.AddIgnoredActor(Target);

    UWorld* World = OwnerPawn->GetWorld();
    if (!World)
    {
        return false;
    }

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    return !bHit; // Can see if no obstruction
}

void UNPC_DinosaurBehaviorTree::SetHuntingTarget(AActor* Target)
{
    if (!Target)
    {
        return;
    }

    DinosaurState.CurrentTarget = Target;
    DinosaurState.bIsHunting = true;
    DinosaurState.bIsPatrolling = false;
    DinosaurState.LastSeenTargetTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    if (BlackboardComp.IsValid())
    {
        BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Target);
        BlackboardComp->SetValueAsBool(TEXT("IsHunting"), true);
        BlackboardComp->SetValueAsBool(TEXT("IsPatrolling"), false);
    }

    // If this is a raptor, coordinate pack hunt
    if (DinosaurState.Species == EDinosaurSpecies::Raptor)
    {
        CoordinatePackHunt(Target);
    }

    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s now hunting target %s"), 
        OwnerController.IsValid() && OwnerController->GetPawn() ? 
        *OwnerController->GetPawn()->GetName() : TEXT("Unknown"),
        *Target->GetName());
}

void UNPC_DinosaurBehaviorTree::ClearHuntingTarget()
{
    DinosaurState.CurrentTarget = nullptr;
    DinosaurState.bIsHunting = false;
    DinosaurState.bIsPatrolling = true;

    if (BlackboardComp.IsValid())
    {
        BlackboardComp->ClearValue(TEXT("TargetActor"));
        BlackboardComp->SetValueAsBool(TEXT("IsHunting"), false);
        BlackboardComp->SetValueAsBool(TEXT("IsPatrolling"), true);
    }
}

FVector UNPC_DinosaurBehaviorTree::GetNextPatrolPoint() const
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    if (CurrentTime - LastPatrolChange > PatrolPointChangeTime)
    {
        return GenerateRandomPatrolPoint();
    }

    // Return current patrol point from blackboard
    if (BlackboardComp.IsValid())
    {
        return BlackboardComp->GetValueAsVector(TEXT("PatrolPoint"));
    }

    return DinosaurState.TerritoryCenter;
}

bool UNPC_DinosaurBehaviorTree::IsInTerritory(const FVector& Location) const
{
    float Distance = FVector::Dist(Location, DinosaurState.TerritoryCenter);
    return Distance <= DinosaurState.TerritoryRadius;
}

void UNPC_DinosaurBehaviorTree::OnTargetLost()
{
    UE_LOG(LogTemp, Log, TEXT("Dinosaur lost target, returning to patrol"));
    ClearHuntingTarget();
}

void UNPC_DinosaurBehaviorTree::OnTargetSpotted(AActor* Target)
{
    if (Target && CanSeeTarget(Target))
    {
        SetHuntingTarget(Target);
    }
}

void UNPC_DinosaurBehaviorTree::ExecuteTRexBehavior(float DeltaTime)
{
    // T-Rex: Aggressive territorial predator
    if (DinosaurState.bIsHunting && DinosaurState.CurrentTarget)
    {
        // Continue hunting if target is still visible
        if (!CanSeeTarget(DinosaurState.CurrentTarget))
        {
            float TimeSinceLastSeen = GetWorld()->GetTimeSeconds() - DinosaurState.LastSeenTargetTime;
            if (TimeSinceLastSeen > 5.0f) // Give up after 5 seconds
            {
                OnTargetLost();
            }
        }
    }
    else if (DinosaurState.bIsPatrolling)
    {
        // Look for targets while patrolling
        if (OwnerController.IsValid() && OwnerController->GetPawn())
        {
            // Find nearby players
            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);
            
            for (AActor* Actor : FoundActors)
            {
                if (Actor && Actor->GetClass()->GetName().Contains(TEXT("TranspersonalCharacter")))
                {
                    float Distance = FVector::Dist(OwnerController->GetPawn()->GetActorLocation(), Actor->GetActorLocation());
                    if (Distance < 3000.0f && CanSeeTarget(Actor))
                    {
                        OnTargetSpotted(Actor);
                        break;
                    }
                }
            }
        }
    }
}

void UNPC_DinosaurBehaviorTree::ExecuteRaptorBehavior(float DeltaTime)
{
    // Raptor: Pack hunter, fast and coordinated
    if (DinosaurState.bIsHunting && DinosaurState.CurrentTarget)
    {
        // Raptors are persistent hunters
        if (!CanSeeTarget(DinosaurState.CurrentTarget))
        {
            float TimeSinceLastSeen = GetWorld()->GetTimeSeconds() - DinosaurState.LastSeenTargetTime;
            if (TimeSinceLastSeen > 8.0f) // Raptors hunt longer than T-Rex
            {
                OnTargetLost();
            }
        }
    }
    else if (DinosaurState.bIsPatrolling)
    {
        // Look for targets and coordinate with pack
        if (OwnerController.IsValid() && OwnerController->GetPawn())
        {
            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);
            
            for (AActor* Actor : FoundActors)
            {
                if (Actor && Actor->GetClass()->GetName().Contains(TEXT("TranspersonalCharacter")))
                {
                    float Distance = FVector::Dist(OwnerController->GetPawn()->GetActorLocation(), Actor->GetActorLocation());
                    if (Distance < 4000.0f && CanSeeTarget(Actor))
                    {
                        OnTargetSpotted(Actor);
                        break;
                    }
                }
            }
        }
    }
}

void UNPC_DinosaurBehaviorTree::ExecuteBrachiosaurusBehavior(float DeltaTime)
{
    // Brachiosaurus: Peaceful herbivore, only aggressive when threatened
    if (DinosaurState.Health < 50.0f && !DinosaurState.bIsHunting)
    {
        // Become defensive when injured
        DinosaurState.Aggression = FMath::Min(DinosaurState.Aggression + 10.0f, 50.0f);
    }
    
    // Brachiosaurus rarely hunts, mostly just patrols and feeds
    if (DinosaurState.bIsPatrolling)
    {
        // Very slow patrol pattern
        float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        if (CurrentTime - LastPatrolChange > 30.0f) // Change patrol point every 30 seconds
        {
            FVector NewPatrolPoint = GenerateRandomPatrolPoint();
            if (BlackboardComp.IsValid())
            {
                BlackboardComp->SetValueAsVector(TEXT("PatrolPoint"), NewPatrolPoint);
            }
            const_cast<UNPC_DinosaurBehaviorTree*>(this)->LastPatrolChange = CurrentTime;
        }
    }
}

void UNPC_DinosaurBehaviorTree::JoinPack(UNPC_DinosaurBehaviorTree* PackLeaderRef)
{
    if (!PackLeaderRef || DinosaurState.Species != EDinosaurSpecies::Raptor)
    {
        return;
    }

    PackLeader = PackLeaderRef;
    bIsPackLeader = false;
    
    // Add this raptor to the pack leader's member list
    PackLeaderRef->PackMembers.AddUnique(this);
    
    UE_LOG(LogTemp, Log, TEXT("Raptor joined pack"));
}

void UNPC_DinosaurBehaviorTree::LeavePack()
{
    if (PackLeader.IsValid())
    {
        PackLeader->PackMembers.Remove(this);
    }
    
    PackLeader = nullptr;
    bIsPackLeader = false;
    PackMembers.Empty();
}

void UNPC_DinosaurBehaviorTree::CoordinatePackHunt(AActor* Target)
{
    if (DinosaurState.Species != EDinosaurSpecies::Raptor || !Target)
    {
        return;
    }

    // If this raptor is pack leader, coordinate the hunt
    if (bIsPackLeader || !PackLeader.IsValid())
    {
        bIsPackLeader = true;
        
        // Signal all pack members to hunt the same target
        for (auto& MemberPtr : PackMembers)
        {
            if (MemberPtr.IsValid())
            {
                MemberPtr->SetHuntingTarget(Target);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Pack leader coordinating hunt with %d members"), PackMembers.Num());
    }
}

void UNPC_DinosaurBehaviorTree::UpdateHungerAndAggression(float DeltaTime)
{
    // Hunger increases over time
    DinosaurState.Hunger += DeltaTime * 0.5f;
    DinosaurState.Hunger = FMath::Clamp(DinosaurState.Hunger, 0.0f, 100.0f);

    // Aggression increases with hunger
    if (DinosaurState.Hunger > 70.0f)
    {
        DinosaurState.Aggression += DeltaTime * 2.0f;
    }
    else if (DinosaurState.Hunger < 30.0f)
    {
        DinosaurState.Aggression -= DeltaTime * 1.0f;
    }

    DinosaurState.Aggression = FMath::Clamp(DinosaurState.Aggression, 0.0f, 100.0f);
}

void UNPC_DinosaurBehaviorTree::UpdatePatrolBehavior(float DeltaTime)
{
    if (!DinosaurState.bIsPatrolling)
    {
        return;
    }

    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    if (CurrentTime - LastPatrolChange > PatrolPointChangeTime)
    {
        FVector NewPatrolPoint = GenerateRandomPatrolPoint();
        if (BlackboardComp.IsValid())
        {
            BlackboardComp->SetValueAsVector(TEXT("PatrolPoint"), NewPatrolPoint);
        }
        LastPatrolChange = CurrentTime;
    }
}

void UNPC_DinosaurBehaviorTree::UpdateHuntingBehavior(float DeltaTime)
{
    if (!DinosaurState.bIsHunting || !DinosaurState.CurrentTarget)
    {
        return;
    }

    // Check if target is still valid and visible
    if (!IsValid(DinosaurState.CurrentTarget) || !CanSeeTarget(DinosaurState.CurrentTarget))
    {
        float TimeSinceLastSeen = GetWorld()->GetTimeSeconds() - DinosaurState.LastSeenTargetTime;
        float GiveUpTime = (DinosaurState.Species == EDinosaurSpecies::Raptor) ? 8.0f : 5.0f;
        
        if (TimeSinceLastSeen > GiveUpTime)
        {
            OnTargetLost();
        }
    }
    else
    {
        DinosaurState.LastSeenTargetTime = GetWorld()->GetTimeSeconds();
    }
}

FVector UNPC_DinosaurBehaviorTree::GenerateRandomPatrolPoint() const
{
    FVector RandomDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ).GetSafeNormal();

    float RandomDistance = FMath::RandRange(DinosaurState.TerritoryRadius * 0.3f, DinosaurState.TerritoryRadius * 0.8f);
    FVector PatrolPoint = DinosaurState.TerritoryCenter + (RandomDirection * RandomDistance);

    // Ensure patrol point is valid
    if (!IsValidPatrolPoint(PatrolPoint))
    {
        return DinosaurState.TerritoryCenter;
    }

    return PatrolPoint;
}

bool UNPC_DinosaurBehaviorTree::IsValidPatrolPoint(const FVector& Point) const
{
    // Check if point is within territory
    if (!IsInTerritory(Point))
    {
        return false;
    }

    // Additional validation could be added here (terrain checks, etc.)
    return true;
}