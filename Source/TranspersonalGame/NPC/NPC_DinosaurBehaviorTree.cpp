#include "NPC_DinosaurBehaviorTree.h"
#include "NPC_DinosaurAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"
#include "Math/UnrealMathUtility.h"

// Blackboard key definitions
const FName UNPC_DinosaurBlackboardKeys::TargetActorKey = TEXT("TargetActor");
const FName UNPC_DinosaurBlackboardKeys::TargetLocationKey = TEXT("TargetLocation");
const FName UNPC_DinosaurBlackboardKeys::PatrolPointKey = TEXT("PatrolPoint");
const FName UNPC_DinosaurBlackboardKeys::HomeLocationKey = TEXT("HomeLocation");
const FName UNPC_DinosaurBlackboardKeys::LastKnownPlayerLocationKey = TEXT("LastKnownPlayerLocation");
const FName UNPC_DinosaurBlackboardKeys::CurrentStateKey = TEXT("CurrentState");
const FName UNPC_DinosaurBlackboardKeys::PreviousStateKey = TEXT("PreviousState");
const FName UNPC_DinosaurBlackboardKeys::AlertLevelKey = TEXT("AlertLevel");
const FName UNPC_DinosaurBlackboardKeys::HealthPercentageKey = TEXT("HealthPercentage");
const FName UNPC_DinosaurBlackboardKeys::StaminaPercentageKey = TEXT("StaminaPercentage");
const FName UNPC_DinosaurBlackboardKeys::PackMembersKey = TEXT("PackMembers");
const FName UNPC_DinosaurBlackboardKeys::PackLeaderKey = TEXT("PackLeader");
const FName UNPC_DinosaurBlackboardKeys::IsPackLeaderKey = TEXT("IsPackLeader");
const FName UNPC_DinosaurBlackboardKeys::PackTargetKey = TEXT("PackTarget");
const FName UNPC_DinosaurBlackboardKeys::TerritoryRadiusKey = TEXT("TerritoryRadius");
const FName UNPC_DinosaurBlackboardKeys::CanSeePlayerKey = TEXT("CanSeePlayer");
const FName UNPC_DinosaurBlackboardKeys::CanHearPlayerKey = TEXT("CanHearPlayer");
const FName UNPC_DinosaurBlackboardKeys::LastSeenPlayerTimeKey = TEXT("LastSeenPlayerTime");
const FName UNPC_DinosaurBlackboardKeys::LastHeardPlayerTimeKey = TEXT("LastHeardPlayerTime");
const FName UNPC_DinosaurBlackboardKeys::AggressionLevelKey = TEXT("AggressionLevel");
const FName UNPC_DinosaurBlackboardKeys::FearLevelKey = TEXT("FearLevel");
const FName UNPC_DinosaurBlackboardKeys::HungerLevelKey = TEXT("HungerLevel");
const FName UNPC_DinosaurBlackboardKeys::IsRestingKey = TEXT("IsResting");
const FName UNPC_DinosaurBlackboardKeys::IsFeedingKey = TEXT("IsFeeding");

// ============================================================================
// UNPC_BTTask_DinosaurHunt Implementation
// ============================================================================

UNPC_BTTask_DinosaurHunt::UNPC_BTTask_DinosaurHunt()
{
    NodeName = TEXT("Dinosaur Hunt");
    bNotifyTick = true;
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UNPC_BTTask_DinosaurHunt::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AActor* HuntTarget = nullptr;
    if (FindHuntTarget(OwnerComp, HuntTarget))
    {
        SetHuntBlackboardKeys(OwnerComp, HuntTarget);
        
        // Set hunt speed
        if (APawn* OwnerPawn = OwnerComp.GetAIOwner()->GetPawn())
        {
            if (ACharacter* Character = Cast<ACharacter>(OwnerPawn))
            {
                if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
                {
                    float OriginalSpeed = MovementComp->MaxWalkSpeed;
                    MovementComp->MaxWalkSpeed = OriginalSpeed * HuntSpeedMultiplier;
                }
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s starting hunt for target %s"), 
               *OwnerComp.GetAIOwner()->GetPawn()->GetName(), 
               *HuntTarget->GetName());
        
        return EBTNodeResult::InProgress;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s could not find hunt target"), 
           *OwnerComp.GetAIOwner()->GetPawn()->GetName());
    return EBTNodeResult::Failed;
}

void UNPC_BTTask_DinosaurHunt::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    // Check if target is still valid
    AActor* CurrentTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(UNPC_DinosaurBlackboardKeys::TargetActorKey));
    if (!CurrentTarget || !IsValidHuntTarget(CurrentTarget, OwnerComp.GetAIOwner()->GetPawn()))
    {
        UE_LOG(LogTemp, Log, TEXT("Hunt target lost or invalid, ending hunt"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    // Check hunt duration
    static float HuntStartTime = GetWorld()->GetTimeSeconds();
    if (GetWorld()->GetTimeSeconds() - HuntStartTime > HuntDuration)
    {
        UE_LOG(LogTemp, Log, TEXT("Hunt duration exceeded, ending hunt"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    // Update target location
    BlackboardComp->SetValueAsVector(UNPC_DinosaurBlackboardKeys::TargetLocationKey, CurrentTarget->GetActorLocation());
    
    // Check if we're close enough to attack
    APawn* OwnerPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (OwnerPawn)
    {
        float DistanceToTarget = FVector::Dist(OwnerPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
        if (DistanceToTarget < 300.0f) // Attack range
        {
            UE_LOG(LogTemp, Log, TEXT("Hunt successful - close enough to attack"));
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
            return;
        }
    }
}

FString UNPC_BTTask_DinosaurHunt::GetStaticDescription() const
{
    return FString::Printf(TEXT("Hunt target within %0.1f units for %0.1f seconds"), MaxHuntDistance, HuntDuration);
}

bool UNPC_BTTask_DinosaurHunt::FindHuntTarget(UBehaviorTreeComponent& OwnerComp, AActor*& OutTarget)
{
    APawn* OwnerPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (!OwnerPawn)
    {
        return false;
    }
    
    UWorld* World = OwnerPawn->GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Look for player character first
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (PlayerPawn && IsValidHuntTarget(PlayerPawn, OwnerPawn))
    {
        float DistanceToPlayer = FVector::Dist(OwnerPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
        if (DistanceToPlayer <= MaxHuntDistance)
        {
            OutTarget = PlayerPawn;
            return true;
        }
    }
    
    // Look for other potential prey
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), AllActors);
    
    float ClosestDistance = MaxHuntDistance;
    AActor* ClosestTarget = nullptr;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor == OwnerPawn) continue;
        
        APawn* PotentialPrey = Cast<APawn>(Actor);
        if (PotentialPrey && IsValidHuntTarget(PotentialPrey, OwnerPawn))
        {
            float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), PotentialPrey->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestTarget = PotentialPrey;
            }
        }
    }
    
    if (ClosestTarget)
    {
        OutTarget = ClosestTarget;
        return true;
    }
    
    return false;
}

bool UNPC_BTTask_DinosaurHunt::IsValidHuntTarget(AActor* Target, APawn* Hunter)
{
    if (!Target || !Hunter) return false;
    
    // Don't hunt same species (basic check)
    if (Target->GetClass() == Hunter->GetClass()) return false;
    
    // Check if target is alive (has health component or is not destroyed)
    if (Target->IsPendingKill()) return false;
    
    return true;
}

void UNPC_BTTask_DinosaurHunt::SetHuntBlackboardKeys(UBehaviorTreeComponent& OwnerComp, AActor* Target)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (BlackboardComp && Target)
    {
        BlackboardComp->SetValueAsObject(UNPC_DinosaurBlackboardKeys::TargetActorKey, Target);
        BlackboardComp->SetValueAsVector(UNPC_DinosaurBlackboardKeys::TargetLocationKey, Target->GetActorLocation());
        BlackboardComp->SetValueAsFloat(UNPC_DinosaurBlackboardKeys::AggressionLevelKey, 0.8f);
    }
}

// ============================================================================
// UNPC_BTTask_DinosaurPatrol Implementation
// ============================================================================

UNPC_BTTask_DinosaurPatrol::UNPC_BTTask_DinosaurPatrol()
{
    NodeName = TEXT("Dinosaur Patrol");
    bNotifyTick = true;
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UNPC_BTTask_DinosaurPatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    APawn* OwnerPawn = OwnerComp.GetAIOwner()->GetPawn();
    
    if (!BlackboardComp || !OwnerPawn)
    {
        return EBTNodeResult::Failed;
    }
    
    // Get territory center (home location)
    FVector TerritoryCenter = BlackboardComp->GetValueAsVector(UNPC_DinosaurBlackboardKeys::HomeLocationKey);
    if (TerritoryCenter.IsZero())
    {
        TerritoryCenter = OwnerPawn->GetActorLocation();
        BlackboardComp->SetValueAsVector(UNPC_DinosaurBlackboardKeys::HomeLocationKey, TerritoryCenter);
    }
    
    // Generate new patrol point
    FVector PatrolPoint = GeneratePatrolPoint(TerritoryCenter, PatrolRadius);
    BlackboardComp->SetValueAsVector(UNPC_DinosaurBlackboardKeys::PatrolPointKey, PatrolPoint);
    
    // Set patrol speed
    if (ACharacter* Character = Cast<ACharacter>(OwnerPawn))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            MovementComp->MaxWalkSpeed = PatrolSpeed;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s starting patrol to %s"), 
           *OwnerPawn->GetName(), *PatrolPoint.ToString());
    
    return EBTNodeResult::InProgress;
}

void UNPC_BTTask_DinosaurPatrol::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    APawn* OwnerPawn = OwnerComp.GetAIOwner()->GetPawn();
    
    if (!BlackboardComp || !OwnerPawn)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    FVector PatrolPoint = BlackboardComp->GetValueAsVector(UNPC_DinosaurBlackboardKeys::PatrolPointKey);
    
    if (HasReachedPatrolPoint(OwnerPawn, PatrolPoint))
    {
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s reached patrol point"), *OwnerPawn->GetName());
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

FString UNPC_BTTask_DinosaurPatrol::GetStaticDescription() const
{
    return FString::Printf(TEXT("Patrol within %0.1f units radius"), PatrolRadius);
}

FVector UNPC_BTTask_DinosaurPatrol::GeneratePatrolPoint(const FVector& TerritoryCenter, float Radius)
{
    // Generate random point within patrol radius
    float RandomAngle = FMath::RandRange(0.0f, 360.0f);
    float RandomDistance = FMath::RandRange(Radius * 0.3f, Radius);
    
    FVector RandomDirection = FVector(
        FMath::Cos(FMath::DegreesToRadians(RandomAngle)),
        FMath::Sin(FMath::DegreesToRadians(RandomAngle)),
        0.0f
    );
    
    FVector PatrolPoint = TerritoryCenter + (RandomDirection * RandomDistance);
    
    // Project to ground (simple Z adjustment)
    PatrolPoint.Z = TerritoryCenter.Z;
    
    return PatrolPoint;
}

bool UNPC_BTTask_DinosaurPatrol::HasReachedPatrolPoint(APawn* Pawn, const FVector& TargetLocation)
{
    if (!Pawn) return false;
    
    float Distance = FVector::Dist2D(Pawn->GetActorLocation(), TargetLocation);
    return Distance < 200.0f; // Reached threshold
}

// ============================================================================
// UNPC_BTService_PackCoordination Implementation
// ============================================================================

UNPC_BTService_PackCoordination::UNPC_BTService_PackCoordination()
{
    NodeName = TEXT("Pack Coordination");
    Interval = CoordinationUpdateInterval;
    RandomDeviation = 0.5f;
}

void UNPC_BTService_PackCoordination::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    UpdatePackMembers(OwnerComp);
    ShareTargetInformation(OwnerComp);
}

FString UNPC_BTService_PackCoordination::GetStaticDescription() const
{
    return FString::Printf(TEXT("Coordinate with pack members within %0.1f units"), PackDetectionRange);
}

void UNPC_BTService_PackCoordination::UpdatePackMembers(UBehaviorTreeComponent& OwnerComp)
{
    APawn* OwnerPawn = OwnerComp.GetAIOwner()->GetPawn();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    
    if (!OwnerPawn || !BlackboardComp) return;
    
    TArray<APawn*> PackMembers = FindNearbyPackMembers(OwnerPawn);
    
    // Update pack size in blackboard
    BlackboardComp->SetValueAsInt(TEXT("PackSize"), PackMembers.Num());
    
    // Determine pack leader (for now, just the one with lowest ID)
    APawn* PackLeader = nullptr;
    for (APawn* Member : PackMembers)
    {
        if (!PackLeader || Member->GetUniqueID() < PackLeader->GetUniqueID())
        {
            PackLeader = Member;
        }
    }
    
    if (PackLeader)
    {
        BlackboardComp->SetValueAsObject(UNPC_DinosaurBlackboardKeys::PackLeaderKey, PackLeader);
        BlackboardComp->SetValueAsBool(UNPC_DinosaurBlackboardKeys::IsPackLeaderKey, PackLeader == OwnerPawn);
    }
}

void UNPC_BTService_PackCoordination::ShareTargetInformation(UBehaviorTreeComponent& OwnerComp)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp) return;
    
    AActor* MyTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(UNPC_DinosaurBlackboardKeys::TargetActorKey));
    if (MyTarget)
    {
        // Share target with pack members
        BlackboardComp->SetValueAsObject(UNPC_DinosaurBlackboardKeys::PackTargetKey, MyTarget);
    }
}

TArray<APawn*> UNPC_BTService_PackCoordination::FindNearbyPackMembers(APawn* SelfPawn)
{
    TArray<APawn*> PackMembers;
    if (!SelfPawn) return PackMembers;
    
    UWorld* World = SelfPawn->GetWorld();
    if (!World) return PackMembers;
    
    TArray<AActor*> AllPawns;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), AllPawns);
    
    for (AActor* Actor : AllPawns)
    {
        APawn* OtherPawn = Cast<APawn>(Actor);
        if (!OtherPawn || OtherPawn == SelfPawn) continue;
        
        // Check if same species (simple class check)
        if (OtherPawn->GetClass() == SelfPawn->GetClass())
        {
            float Distance = FVector::Dist(SelfPawn->GetActorLocation(), OtherPawn->GetActorLocation());
            if (Distance <= PackDetectionRange)
            {
                PackMembers.Add(OtherPawn);
                if (PackMembers.Num() >= MaxPackSize) break;
            }
        }
    }
    
    return PackMembers;
}

// ============================================================================
// UNPC_BTDecorator_FearResponse Implementation
// ============================================================================

UNPC_BTDecorator_FearResponse::UNPC_BTDecorator_FearResponse()
{
    NodeName = TEXT("Fear Response");
}

bool UNPC_BTDecorator_FearResponse::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    float CurrentFearLevel = CalculateFearLevel(OwnerComp);
    return CurrentFearLevel >= FearThreshold;
}

FString UNPC_BTDecorator_FearResponse::GetStaticDescription() const
{
    return FString::Printf(TEXT("Fear level >= %0.2f"), FearThreshold);
}

float UNPC_BTDecorator_FearResponse::CalculateFearLevel(UBehaviorTreeComponent& OwnerComp) const
{
    APawn* OwnerPawn = OwnerComp.GetAIOwner()->GetPawn();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    
    if (!OwnerPawn || !BlackboardComp) return 0.0f;
    
    float FearLevel = 0.0f;
    
    // Check for large predators nearby
    if (IsLargePredatorNearby(OwnerPawn))
    {
        FearLevel += 0.4f;
    }
    
    // Check health level
    float HealthPercentage = BlackboardComp->GetValueAsFloat(UNPC_DinosaurBlackboardKeys::HealthPercentageKey);
    if (HealthPercentage < LowHealthThreshold)
    {
        FearLevel += (LowHealthThreshold - HealthPercentage) * 0.5f;
    }
    
    // Check if outnumbered
    int32 PackSize = BlackboardComp->GetValueAsInt(TEXT("PackSize"));
    if (PackSize < 2)
    {
        FearLevel += 0.2f;
    }
    
    return FMath::Clamp(FearLevel, 0.0f, 1.0f);
}

bool UNPC_BTDecorator_FearResponse::IsLargePredatorNearby(APawn* SelfPawn) const
{
    if (!SelfPawn) return false;
    
    UWorld* World = SelfPawn->GetWorld();
    if (!World) return false;
    
    // Simple check for larger pawns nearby (representing predators)
    TArray<AActor*> AllPawns;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), AllPawns);
    
    for (AActor* Actor : AllPawns)
    {
        APawn* OtherPawn = Cast<APawn>(Actor);
        if (!OtherPawn || OtherPawn == SelfPawn) continue;
        
        float Distance = FVector::Dist(SelfPawn->GetActorLocation(), OtherPawn->GetActorLocation());
        if (Distance <= PredatorFearDistance)
        {
            // Check if it's a larger predator (simple size check)
            if (UCapsuleComponent* SelfCapsule = SelfPawn->FindComponentByClass<UCapsuleComponent>())
            {
                if (UCapsuleComponent* OtherCapsule = OtherPawn->FindComponentByClass<UCapsuleComponent>())
                {
                    if (OtherCapsule->GetScaledCapsuleRadius() > SelfCapsule->GetScaledCapsuleRadius() * 1.5f)
                    {
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

// ============================================================================
// UNPC_DinosaurBehaviorTreeManager Implementation
// ============================================================================

UNPC_DinosaurBehaviorTreeManager::UNPC_DinosaurBehaviorTreeManager()
{
    InitializeDefaultBehaviorTrees();
}

UBehaviorTree* UNPC_DinosaurBehaviorTreeManager::GetBehaviorTreeForSpecies(ENPC_DinosaurSpecies Species, ENPC_DinosaurRole Role)
{
    // First check for role-specific behavior tree
    if (UBehaviorTree** RoleTree = RoleBehaviorTrees.Find(Role))
    {
        if (*RoleTree) return *RoleTree;
    }
    
    // Then check for species-specific behavior tree
    if (UBehaviorTree** SpeciesTree = SpeciesBehaviorTrees.Find(Species))
    {
        if (*SpeciesTree) return *SpeciesTree;
    }
    
    // Fall back to default
    return DefaultBehaviorTree;
}

void UNPC_DinosaurBehaviorTreeManager::ConfigureBlackboardForDinosaur(UBlackboardComponent* Blackboard, ENPC_DinosaurSpecies Species)
{
    if (!Blackboard) return;
    
    // Set default values based on species
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            Blackboard->SetValueAsFloat(UNPC_DinosaurBlackboardKeys::AggressionLevelKey, 0.8f);
            Blackboard->SetValueAsFloat(UNPC_DinosaurBlackboardKeys::FearLevelKey, 0.1f);
            Blackboard->SetValueAsFloat(UNPC_DinosaurBlackboardKeys::TerritoryRadiusKey, 5000.0f);
            break;
            
        case ENPC_DinosaurSpecies::Raptor:
            Blackboard->SetValueAsFloat(UNPC_DinosaurBlackboardKeys::AggressionLevelKey, 0.7f);
            Blackboard->SetValueAsFloat(UNPC_DinosaurBlackboardKeys::FearLevelKey, 0.3f);
            Blackboard->SetValueAsFloat(UNPC_DinosaurBlackboardKeys::TerritoryRadiusKey, 3000.0f);
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            Blackboard->SetValueAsFloat(UNPC_DinosaurBlackboardKeys::AggressionLevelKey, 0.4f);
            Blackboard->SetValueAsFloat(UNPC_DinosaurBlackboardKeys::FearLevelKey, 0.2f);
            Blackboard->SetValueAsFloat(UNPC_DinosaurBlackboardKeys::TerritoryRadiusKey, 2000.0f);
            break;
            
        default:
            Blackboard->SetValueAsFloat(UNPC_DinosaurBlackboardKeys::AggressionLevelKey, 0.5f);
            Blackboard->SetValueAsFloat(UNPC_DinosaurBlackboardKeys::FearLevelKey, 0.3f);
            Blackboard->SetValueAsFloat(UNPC_DinosaurBlackboardKeys::TerritoryRadiusKey, 2500.0f);
            break;
    }
    
    // Set common default values
    Blackboard->SetValueAsFloat(UNPC_DinosaurBlackboardKeys::HealthPercentageKey, 1.0f);
    Blackboard->SetValueAsFloat(UNPC_DinosaurBlackboardKeys::StaminaPercentageKey, 1.0f);
    Blackboard->SetValueAsBool(UNPC_DinosaurBlackboardKeys::IsRestingKey, false);
    Blackboard->SetValueAsBool(UNPC_DinosaurBlackboardKeys::IsFeedingKey, false);
}

void UNPC_DinosaurBehaviorTreeManager::UpdateBehaviorTreeForState(ANPC_DinosaurAIController* Controller, ENPC_DinosaurBehaviorState NewState)
{
    if (!Controller) return;
    
    UBlackboardComponent* Blackboard = Controller->GetBlackboardComponent();
    if (Blackboard)
    {
        // Store previous state
        uint8 CurrentState = Blackboard->GetValueAsEnum(UNPC_DinosaurBlackboardKeys::CurrentStateKey);
        Blackboard->SetValueAsEnum(UNPC_DinosaurBlackboardKeys::PreviousStateKey, CurrentState);
        
        // Set new state
        Blackboard->SetValueAsEnum(UNPC_DinosaurBlackboardKeys::CurrentStateKey, static_cast<uint8>(NewState));
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s state changed to %d"), 
               *Controller->GetPawn()->GetName(), static_cast<int32>(NewState));
    }
}

void UNPC_DinosaurBehaviorTreeManager::InitializeDefaultBehaviorTrees()
{
    // This would normally load behavior trees from assets
    // For now, we'll set up the structure for future asset loading
    DefaultBehaviorTree = nullptr;
}

UBehaviorTree* UNPC_DinosaurBehaviorTreeManager::CreateBehaviorTreeForSpecies(ENPC_DinosaurSpecies Species)
{
    // This would create or load species-specific behavior trees
    // Implementation depends on asset management system
    return nullptr;
}