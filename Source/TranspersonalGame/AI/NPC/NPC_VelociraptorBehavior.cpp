#include "NPC_VelociraptorBehavior.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPC_VelociraptorBehavior::UNPC_VelociraptorBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentState = ENPC_VelociraptorState::Idle;
    StateTimer = 0.0f;
    CurrentTarget = nullptr;
    
    // Pack behavior defaults
    bIsPackLeader = false;
    PackCommunicationRange = 3000.0f;
    
    // Hunting defaults
    HuntingRange = 2500.0f;
    AttackRange = 200.0f;
    FleeHealthThreshold = 0.3f;
    
    // Patrol defaults
    CurrentPatrolIndex = 0;
    PatrolSpeed = 400.0f;
    PatrolWaitTime = 3.0f;
    
    // Social defaults
    SocialInteractionRange = 500.0f;
    SocialInteractionCooldown = 10.0f;
    LastSocialInteraction = 0.0f;
}

void UNPC_VelociraptorBehavior::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize patrol route
    InitializePatrolRoute();
    
    // Set up behavior update timer
    GetWorld()->GetTimerManager().SetTimer(
        BehaviorUpdateTimer,
        this,
        &UNPC_VelociraptorBehavior::OnBehaviorUpdate,
        0.5f,
        true
    );
    
    // Set up pack communication timer
    GetWorld()->GetTimerManager().SetTimer(
        PackCommunicationTimer,
        this,
        &UNPC_VelociraptorBehavior::OnPackCommunication,
        2.0f,
        true
    );
    
    // Start with idle state
    SetBehaviorState(ENPC_VelociraptorState::Idle);
}

void UNPC_VelociraptorBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateBehavior(DeltaTime);
    
    if (PackData.PackMembers.Num() > 1)
    {
        UpdatePackBehavior(DeltaTime);
    }
}

void UNPC_VelociraptorBehavior::SetBehaviorState(ENPC_VelociraptorState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        
        // Log state change for debugging
        if (GetOwner())
        {
            UE_LOG(LogTemp, Log, TEXT("Velociraptor %s changed state to %d"), 
                   *GetOwner()->GetName(), (int32)NewState);
        }
    }
}

void UNPC_VelociraptorBehavior::UpdateBehavior(float DeltaTime)
{
    StateTimer += DeltaTime;
    
    switch (CurrentState)
    {
        case ENPC_VelociraptorState::Idle:
        {
            // Look for prey or pack members
            AActor* Prey = FindNearestPrey();
            if (Prey && IsTargetInRange(Prey, HuntingRange))
            {
                StartHunt(Prey);
            }
            else if (StateTimer > 5.0f)
            {
                SetBehaviorState(ENPC_VelociraptorState::Patrolling);
            }
            break;
        }
        
        case ENPC_VelociraptorState::Patrolling:
        {
            UpdatePatrol(DeltaTime);
            
            // Check for prey while patrolling
            AActor* Prey = FindNearestPrey();
            if (Prey && IsTargetInRange(Prey, HuntingRange))
            {
                StartHunt(Prey);
            }
            break;
        }
        
        case ENPC_VelociraptorState::Hunting:
        {
            if (CurrentTarget)
            {
                float DistanceToTarget = GetDistanceToTarget(CurrentTarget);
                
                if (DistanceToTarget <= AttackRange)
                {
                    SetBehaviorState(ENPC_VelociraptorState::Attacking);
                }
                else if (DistanceToTarget > HuntingRange * 1.5f)
                {
                    // Lost target
                    CurrentTarget = nullptr;
                    SetBehaviorState(ENPC_VelociraptorState::Idle);
                }
                else
                {
                    // Continue pursuit
                    MoveTowardsTarget(CurrentTarget, PatrolSpeed * 1.5f);
                }
            }
            else
            {
                SetBehaviorState(ENPC_VelociraptorState::Idle);
            }
            break;
        }
        
        case ENPC_VelociraptorState::Attacking:
        {
            if (CurrentTarget && IsTargetInRange(CurrentTarget, AttackRange))
            {
                // Perform attack logic here
                if (StateTimer > 2.0f)
                {
                    SetBehaviorState(ENPC_VelociraptorState::Hunting);
                }
            }
            else
            {
                SetBehaviorState(ENPC_VelociraptorState::Hunting);
            }
            break;
        }
        
        case ENPC_VelociraptorState::Socializing:
        {
            PerformSocialBehavior();
            
            if (StateTimer > 8.0f)
            {
                SetBehaviorState(ENPC_VelociraptorState::Idle);
            }
            break;
        }
    }
}

void UNPC_VelociraptorBehavior::JoinPack(UNPC_VelociraptorBehavior* OtherRaptor)
{
    if (!OtherRaptor || OtherRaptor == this) return;
    
    // Add to each other's pack
    PackData.PackMembers.AddUnique(OtherRaptor->GetOwner());
    OtherRaptor->PackData.PackMembers.AddUnique(GetOwner());
    
    // Determine pack leader (first to form pack or existing leader)
    if (!PackData.PackLeader)
    {
        if (OtherRaptor->bIsPackLeader)
        {
            PackData.PackLeader = OtherRaptor->GetOwner();
        }
        else
        {
            PackData.PackLeader = GetOwner();
            bIsPackLeader = true;
        }
    }
    
    // Update pack center
    FVector TotalPosition = FVector::ZeroVector;
    for (AActor* Member : PackData.PackMembers)
    {
        if (Member)
        {
            TotalPosition += Member->GetActorLocation();
        }
    }
    
    if (PackData.PackMembers.Num() > 0)
    {
        PackData.PackCenter = TotalPosition / PackData.PackMembers.Num();
    }
}

void UNPC_VelociraptorBehavior::LeavePack()
{
    // Remove from all pack members
    for (AActor* Member : PackData.PackMembers)
    {
        if (Member && Member != GetOwner())
        {
            UNPC_VelociraptorBehavior* MemberBehavior = Member->FindComponentByClass<UNPC_VelociraptorBehavior>();
            if (MemberBehavior)
            {
                MemberBehavior->PackData.PackMembers.Remove(GetOwner());
            }
        }
    }
    
    // Clear own pack data
    PackData.PackMembers.Empty();
    PackData.PackLeader = nullptr;
    bIsPackLeader = false;
}

void UNPC_VelociraptorBehavior::UpdatePackBehavior(float DeltaTime)
{
    // Clean up invalid pack members
    PackData.PackMembers.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
    
    if (PackData.PackMembers.Num() <= 1)
    {
        LeavePack();
        return;
    }
    
    // Update pack center
    FVector TotalPosition = FVector::ZeroVector;
    for (AActor* Member : PackData.PackMembers)
    {
        if (Member)
        {
            TotalPosition += Member->GetActorLocation();
        }
    }
    PackData.PackCenter = TotalPosition / PackData.PackMembers.Num();
    
    // Pack hunting coordination
    if (CurrentState == ENPC_VelociraptorState::Hunting && bIsPackLeader)
    {
        ExecutePackHunt();
    }
}

AActor* UNPC_VelociraptorBehavior::FindNearestPrey()
{
    if (!GetOwner()) return nullptr;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    AActor* NearestPrey = nullptr;
    float NearestDistance = HuntingRange;
    
    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == GetOwner()) continue;
        
        // Check if this is a valid prey type
        bool bIsValidPrey = false;
        for (TSubclassOf<AActor> PreyType : PreferredPreyTypes)
        {
            if (Actor->IsA(PreyType))
            {
                bIsValidPrey = true;
                break;
            }
        }
        
        // For now, consider any actor with "Character" in name as potential prey
        if (!bIsValidPrey && Actor->GetName().Contains(TEXT("Character")))
        {
            bIsValidPrey = true;
        }
        
        if (bIsValidPrey)
        {
            float Distance = GetDistanceToTarget(Actor);
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestPrey = Actor;
            }
        }
    }
    
    return NearestPrey;
}

void UNPC_VelociraptorBehavior::StartHunt(AActor* Target)
{
    if (!Target) return;
    
    CurrentTarget = Target;
    SetBehaviorState(ENPC_VelociraptorState::Hunting);
    
    // Alert pack members if pack leader
    if (bIsPackLeader)
    {
        for (AActor* Member : PackData.PackMembers)
        {
            if (Member && Member != GetOwner())
            {
                UNPC_VelociraptorBehavior* MemberBehavior = Member->FindComponentByClass<UNPC_VelociraptorBehavior>();
                if (MemberBehavior)
                {
                    MemberBehavior->StartHunt(Target);
                }
            }
        }
    }
}

void UNPC_VelociraptorBehavior::ExecutePackHunt()
{
    if (!CurrentTarget || PackData.PackMembers.Num() < 2) return;
    
    // Coordinate pack attack positions
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    float AngleStep = 360.0f / PackData.PackMembers.Num();
    
    for (int32 i = 0; i < PackData.PackMembers.Num(); ++i)
    {
        AActor* Member = PackData.PackMembers[i];
        if (!Member) continue;
        
        float Angle = AngleStep * i;
        FVector Offset = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * AttackRange * 0.8f,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * AttackRange * 0.8f,
            0.0f
        );
        
        FVector AttackPosition = TargetLocation + Offset;
        
        UNPC_VelociraptorBehavior* MemberBehavior = Member->FindComponentByClass<UNPC_VelociraptorBehavior>();
        if (MemberBehavior)
        {
            MemberBehavior->MoveToLocation(AttackPosition, PatrolSpeed * 1.2f);
        }
    }
}

void UNPC_VelociraptorBehavior::InitializePatrolRoute()
{
    if (!GetOwner()) return;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    PatrolPoints.Empty();
    
    // Create a circular patrol route around spawn point
    float PatrolRadius = 1500.0f;
    int32 NumPoints = 6;
    
    for (int32 i = 0; i < NumPoints; ++i)
    {
        float Angle = (360.0f / NumPoints) * i;
        FVector PatrolPoint = OwnerLocation + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * PatrolRadius,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * PatrolRadius,
            0.0f
        );
        
        PatrolPoints.Add(PatrolPoint);
    }
    
    CurrentPatrolIndex = 0;
}

void UNPC_VelociraptorBehavior::UpdatePatrol(float DeltaTime)
{
    if (PatrolPoints.Num() == 0) return;
    
    FVector CurrentPatrolPoint = PatrolPoints[CurrentPatrolIndex];
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    float DistanceToPatrolPoint = FVector::Dist(OwnerLocation, CurrentPatrolPoint);
    
    if (DistanceToPatrolPoint < 200.0f)
    {
        // Reached patrol point, wait then move to next
        if (StateTimer > PatrolWaitTime)
        {
            CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
            StateTimer = 0.0f;
        }
    }
    else
    {
        // Move towards current patrol point
        MoveToLocation(CurrentPatrolPoint, PatrolSpeed);
    }
}

void UNPC_VelociraptorBehavior::PerformSocialBehavior()
{
    // Simple social behavior - look for nearby pack members
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastSocialInteraction > SocialInteractionCooldown)
    {
        for (AActor* Member : PackData.PackMembers)
        {
            if (Member && Member != GetOwner())
            {
                float Distance = GetDistanceToTarget(Member);
                if (Distance <= SocialInteractionRange)
                {
                    // Perform social interaction (visual/audio cues would go here)
                    LastSocialInteraction = CurrentTime;
                    break;
                }
            }
        }
    }
}

void UNPC_VelociraptorBehavior::OnBehaviorUpdate()
{
    // Periodic behavior checks
    if (CurrentState == ENPC_VelociraptorState::Idle)
    {
        // Check for social opportunities
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastSocialInteraction > SocialInteractionCooldown * 0.5f)
        {
            for (AActor* Member : PackData.PackMembers)
            {
                if (Member && Member != GetOwner())
                {
                    float Distance = GetDistanceToTarget(Member);
                    if (Distance <= SocialInteractionRange)
                    {
                        SetBehaviorState(ENPC_VelociraptorState::Socializing);
                        break;
                    }
                }
            }
        }
    }
}

void UNPC_VelociraptorBehavior::OnPackCommunication()
{
    // Look for nearby velociraptors to form packs with
    if (PackData.PackMembers.Num() < 6) // Max pack size
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (!Actor || Actor == GetOwner()) continue;
            
            UNPC_VelociraptorBehavior* OtherBehavior = Actor->FindComponentByClass<UNPC_VelociraptorBehavior>();
            if (OtherBehavior && !PackData.PackMembers.Contains(Actor))
            {
                float Distance = GetDistanceToTarget(Actor);
                if (Distance <= PackCommunicationRange)
                {
                    JoinPack(OtherBehavior);
                    break;
                }
            }
        }
    }
}

float UNPC_VelociraptorBehavior::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetOwner()) return FLT_MAX;
    
    return FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
}

bool UNPC_VelociraptorBehavior::IsTargetInRange(AActor* Target, float Range) const
{
    return GetDistanceToTarget(Target) <= Range;
}

void UNPC_VelociraptorBehavior::MoveTowardsTarget(AActor* Target, float Speed)
{
    if (!Target || !GetOwner()) return;
    
    FVector TargetLocation = Target->GetActorLocation();
    MoveToLocation(TargetLocation, Speed);
}

void UNPC_VelociraptorBehavior::MoveToLocation(const FVector& Location, float Speed)
{
    if (!GetOwner()) return;
    
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector Direction = (Location - CurrentLocation).GetSafeNormal();
    
    // Simple movement - in a real implementation this would use AI movement components
    FVector NewLocation = CurrentLocation + (Direction * Speed * GetWorld()->GetDeltaSeconds());
    GetOwner()->SetActorLocation(NewLocation);
    
    // Face movement direction
    if (!Direction.IsZero())
    {
        FRotator NewRotation = Direction.Rotation();
        GetOwner()->SetActorRotation(NewRotation);
    }
}