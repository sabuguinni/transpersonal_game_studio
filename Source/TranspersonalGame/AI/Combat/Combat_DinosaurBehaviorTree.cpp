#include "Combat_DinosaurBehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UCombat_DinosaurBehaviorTree::UCombat_DinosaurBehaviorTree()
{
    PrimaryComponentTick.bCanEverTick = true;

    CurrentBehaviorState = ECombat_DinosaurBehaviorState::Idle;
    StateChangeInterval = 2.0f;
    LastStateChange = 0.0f;
    bIsPackLeader = false;
    PackLeader = nullptr;
}

void UCombat_DinosaurBehaviorTree::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBehaviorTree();
    
    // Set initial patrol points around current location
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    PatrolPoints.Add(OwnerLocation + FVector(500, 0, 0));
    PatrolPoints.Add(OwnerLocation + FVector(0, 500, 0));
    PatrolPoints.Add(OwnerLocation + FVector(-500, 0, 0));
    PatrolPoints.Add(OwnerLocation + FVector(0, -500, 0));
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Behavior Tree initialized for: %s"), *GetOwner()->GetName());
}

void UCombat_DinosaurBehaviorTree::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (GetWorld()->GetTimeSeconds() - LastStateChange > StateChangeInterval)
    {
        UpdateBehaviorState();
        UpdatePackBehavior();
        UpdateBlackboardValues();
        LastStateChange = GetWorld()->GetTimeSeconds();
    }
}

void UCombat_DinosaurBehaviorTree::InitializeBehaviorTree()
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("Behavior Tree component owner is not a Pawn"));
        return;
    }

    AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController());
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("No AI Controller found for: %s"), *GetOwner()->GetName());
        return;
    }

    BlackboardComponent = AIController->GetBlackboardComponent();
    if (!BlackboardComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Blackboard Component found for: %s"), *GetOwner()->GetName());
        return;
    }

    // Initialize blackboard keys
    BlackboardComponent->SetValueAsEnum(BlackboardKeys.BehaviorStateKey, (uint8)CurrentBehaviorState);
    BlackboardComponent->SetValueAsFloat(BlackboardKeys.AlertLevelKey, 0.0f);
    
    if (PatrolPoints.Num() > 0)
    {
        BlackboardComponent->SetValueAsVector(BlackboardKeys.PatrolLocationKey, PatrolPoints[0]);
    }

    UE_LOG(LogTemp, Log, TEXT("Behavior Tree initialized for: %s"), *GetOwner()->GetName());
}

void UCombat_DinosaurBehaviorTree::SetBehaviorState(ECombat_DinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(BlackboardKeys.BehaviorStateKey, (uint8)NewState);
        }
        
        UE_LOG(LogTemp, Log, TEXT("%s behavior state changed to: %d"), *GetOwner()->GetName(), (int32)NewState);
    }
}

void UCombat_DinosaurBehaviorTree::SetTargetActor(AActor* Target)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(BlackboardKeys.TargetActorKey, Target);
        
        if (Target)
        {
            BlackboardComponent->SetValueAsVector(BlackboardKeys.LastKnownLocationKey, Target->GetActorLocation());
            UE_LOG(LogTemp, Log, TEXT("%s target set to: %s"), *GetOwner()->GetName(), *Target->GetName());
        }
    }
}

void UCombat_DinosaurBehaviorTree::SetPatrolLocation(FVector Location)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(BlackboardKeys.PatrolLocationKey, Location);
        UE_LOG(LogTemp, Log, TEXT("%s patrol location set to: %s"), *GetOwner()->GetName(), *Location.ToString());
    }
}

void UCombat_DinosaurBehaviorTree::SetAlertLevel(float AlertLevel)
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(BlackboardKeys.AlertLevelKey, AlertLevel);
        
        // Change behavior based on alert level
        if (AlertLevel > 0.8f)
        {
            SetBehaviorState(ECombat_DinosaurBehaviorState::Attacking);
        }
        else if (AlertLevel > 0.5f)
        {
            SetBehaviorState(ECombat_DinosaurBehaviorState::Stalking);
        }
        else if (AlertLevel > 0.2f)
        {
            SetBehaviorState(ECombat_DinosaurBehaviorState::Investigating);
        }
        else
        {
            SetBehaviorState(ECombat_DinosaurBehaviorState::Patrolling);
        }
    }
}

void UCombat_DinosaurBehaviorTree::JoinPack(AActor* Leader)
{
    if (Leader && Leader != GetOwner())
    {
        PackLeader = Leader;
        bIsPackLeader = false;
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsObject(BlackboardKeys.PackLeaderKey, Leader);
        }
        
        UE_LOG(LogTemp, Log, TEXT("%s joined pack under leader: %s"), *GetOwner()->GetName(), *Leader->GetName());
    }
}

void UCombat_DinosaurBehaviorTree::LeavePack()
{
    PackLeader = nullptr;
    bIsPackLeader = false;
    PackMembers.Empty();
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(BlackboardKeys.PackLeaderKey, nullptr);
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s left the pack"), *GetOwner()->GetName());
}

void UCombat_DinosaurBehaviorTree::BecomePackLeader()
{
    bIsPackLeader = true;
    PackLeader = GetOwner();
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(BlackboardKeys.PackLeaderKey, GetOwner());
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s became pack leader"), *GetOwner()->GetName());
}

void UCombat_DinosaurBehaviorTree::AddPackMember(AActor* Member)
{
    if (Member && !PackMembers.Contains(Member) && bIsPackLeader)
    {
        PackMembers.Add(Member);
        UE_LOG(LogTemp, Log, TEXT("%s added to pack led by: %s"), *Member->GetName(), *GetOwner()->GetName());
    }
}

void UCombat_DinosaurBehaviorTree::RemovePackMember(AActor* Member)
{
    if (Member && bIsPackLeader)
    {
        PackMembers.Remove(Member);
        UE_LOG(LogTemp, Log, TEXT("%s removed from pack led by: %s"), *Member->GetName(), *GetOwner()->GetName());
    }
}

void UCombat_DinosaurBehaviorTree::UpdateBehaviorState()
{
    // Simple state machine logic
    switch (CurrentBehaviorState)
    {
        case ECombat_DinosaurBehaviorState::Idle:
            if (FMath::RandRange(0.0f, 1.0f) > 0.7f)
            {
                SetBehaviorState(ECombat_DinosaurBehaviorState::Patrolling);
            }
            break;
            
        case ECombat_DinosaurBehaviorState::Patrolling:
            if (FMath::RandRange(0.0f, 1.0f) > 0.9f)
            {
                SetBehaviorState(ECombat_DinosaurBehaviorState::Resting);
            }
            break;
            
        case ECombat_DinosaurBehaviorState::Resting:
            if (FMath::RandRange(0.0f, 1.0f) > 0.8f)
            {
                SetBehaviorState(ECombat_DinosaurBehaviorState::Idle);
            }
            break;
            
        default:
            break;
    }
}

void UCombat_DinosaurBehaviorTree::UpdatePackBehavior()
{
    if (bIsPackLeader && PackMembers.Num() > 0)
    {
        // Pack leader behavior - coordinate pack members
        for (AActor* Member : PackMembers)
        {
            if (Member)
            {
                UCombat_DinosaurBehaviorTree* MemberBehavior = Member->FindComponentByClass<UCombat_DinosaurBehaviorTree>();
                if (MemberBehavior)
                {
                    // Synchronize behavior states within the pack
                    if (CurrentBehaviorState == ECombat_DinosaurBehaviorState::Attacking)
                    {
                        MemberBehavior->SetBehaviorState(ECombat_DinosaurBehaviorState::Attacking);
                    }
                }
            }
        }
    }
    else if (PackLeader)
    {
        // Pack member behavior - follow leader
        UCombat_DinosaurBehaviorTree* LeaderBehavior = PackLeader->FindComponentByClass<UCombat_DinosaurBehaviorTree>();
        if (LeaderBehavior)
        {
            ECombat_DinosaurBehaviorState LeaderState = LeaderBehavior->GetCurrentBehaviorState();
            if (LeaderState == ECombat_DinosaurBehaviorState::Attacking || LeaderState == ECombat_DinosaurBehaviorState::Stalking)
            {
                SetBehaviorState(LeaderState);
            }
        }
    }
}

void UCombat_DinosaurBehaviorTree::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
    {
        return;
    }

    // Update current behavior state
    BlackboardComponent->SetValueAsEnum(BlackboardKeys.BehaviorStateKey, (uint8)CurrentBehaviorState);
    
    // Update pack leader reference
    if (PackLeader)
    {
        BlackboardComponent->SetValueAsObject(BlackboardKeys.PackLeaderKey, PackLeader);
    }
    
    // Cycle through patrol points
    if (PatrolPoints.Num() > 0 && CurrentBehaviorState == ECombat_DinosaurBehaviorState::Patrolling)
    {
        static int32 CurrentPatrolIndex = 0;
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
        BlackboardComponent->SetValueAsVector(BlackboardKeys.PatrolLocationKey, PatrolPoints[CurrentPatrolIndex]);
    }
}