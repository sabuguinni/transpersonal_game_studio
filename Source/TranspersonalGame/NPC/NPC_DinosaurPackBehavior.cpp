#include "NPC_DinosaurPackBehavior.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

UNPC_DinosaurPackBehavior::UNPC_DinosaurPackBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    PackLeader = nullptr;
    CurrentTarget = nullptr;
    CurrentPackState = ENPC_PackState::Idle;
    PackCohesionRadius = 1500.0f;
    HuntingRange = 3000.0f;
    AttackCoordinationDelay = 2.0f;
    MinPackSize = 2;
    MaxPackSize = 6;
    LastStateUpdateTime = 0.0f;
    LastCommunicationTime = 0.0f;
}

void UNPC_DinosaurPackBehavior::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize formation positions for pack coordination
    FormationPositions.Empty();
    FormationPositions.Add(FVector(0, 0, 0));      // Leader position
    FormationPositions.Add(FVector(-200, -300, 0)); // Left flank
    FormationPositions.Add(FVector(-200, 300, 0));  // Right flank
    FormationPositions.Add(FVector(-400, 0, 0));    // Rear guard
    FormationPositions.Add(FVector(-300, -200, 0)); // Left rear
    FormationPositions.Add(FVector(-300, 200, 0));  // Right rear
}

void UNPC_DinosaurPackBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!PackLeader || PackMembers.Num() < MinPackSize)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update pack state every 2 seconds
    if (CurrentTime - LastStateUpdateTime > 2.0f)
    {
        UpdatePackState();
        LastStateUpdateTime = CurrentTime;
    }
    
    // Execute current behavior
    ExecutePackBehavior(DeltaTime);
    
    // Update formation every frame for smooth movement
    UpdatePackFormation();
}

void UNPC_DinosaurPackBehavior::InitializePack(APawn* Leader, const TArray<APawn*>& Members)
{
    if (!Leader)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot initialize pack without a leader"));
        return;
    }
    
    PackLeader = Leader;
    PackMembers.Empty();
    
    // Add leader as first member
    FNPC_PackMember LeaderMember;
    LeaderMember.DinosaurPawn = Leader;
    LeaderMember.Role = ENPC_PackRole::Leader;
    PackMembers.Add(LeaderMember);
    
    // Add other members
    for (int32 i = 0; i < Members.Num() && PackMembers.Num() < MaxPackSize; i++)
    {
        if (Members[i] && Members[i] != Leader)
        {
            AddPackMember(Members[i], ENPC_PackRole::Hunter);
        }
    }
    
    AssignPackRoles();
    CurrentPackState = ENPC_PackState::Idle;
    
    UE_LOG(LogTemp, Log, TEXT("Pack initialized with %d members"), PackMembers.Num());
}

void UNPC_DinosaurPackBehavior::AddPackMember(APawn* NewMember, ENPC_PackRole Role)
{
    if (!NewMember || PackMembers.Num() >= MaxPackSize)
    {
        return;
    }
    
    // Check if already in pack
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.DinosaurPawn == NewMember)
        {
            return;
        }
    }
    
    FNPC_PackMember NewPackMember;
    NewPackMember.DinosaurPawn = NewMember;
    NewPackMember.Role = Role;
    NewPackMember.LastKnownPosition = NewMember->GetActorLocation();
    PackMembers.Add(NewPackMember);
    
    UE_LOG(LogTemp, Log, TEXT("Added pack member: %s"), *NewMember->GetName());
}

void UNPC_DinosaurPackBehavior::RemovePackMember(APawn* Member)
{
    for (int32 i = PackMembers.Num() - 1; i >= 0; i--)
    {
        if (PackMembers[i].DinosaurPawn == Member)
        {
            PackMembers.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Removed pack member: %s"), *Member->GetName());
            break;
        }
    }
    
    // Reassign roles if pack is still viable
    if (PackMembers.Num() >= MinPackSize)
    {
        AssignPackRoles();
    }
}

void UNPC_DinosaurPackBehavior::SetPackTarget(APawn* Target)
{
    CurrentTarget = Target;
    if (Target)
    {
        CurrentPackState = ENPC_PackState::Hunting;
        CommunicateWithPack(FString::Printf(TEXT("Target acquired: %s"), *Target->GetName()));
    }
    else
    {
        CurrentPackState = ENPC_PackState::Idle;
    }
}

void UNPC_DinosaurPackBehavior::ExecutePackHunt()
{
    if (!CurrentTarget || !IsTargetWithinHuntingRange(CurrentTarget))
    {
        CurrentPackState = ENPC_PackState::Idle;
        return;
    }
    
    CurrentPackState = ENPC_PackState::Hunting;
    
    // Coordinate pack movement towards target
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (PackMembers[i].bIsAlive && PackMembers[i].DinosaurPawn)
        {
            FVector FlankingPos = CalculateFlankingPosition(CurrentTarget, i);
            
            // Move pack member to flanking position
            // This would typically interface with AI movement components
            PackMembers[i].LastKnownPosition = FlankingPos;
        }
    }
}

void UNPC_DinosaurPackBehavior::ExecuteCoordinatedAttack()
{
    if (!CurrentTarget)
    {
        return;
    }
    
    CurrentPackState = ENPC_PackState::Attacking;
    
    // Stagger attacks for realistic pack behavior
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (PackMembers[i].bIsAlive && PackMembers[i].DinosaurPawn)
        {
            float AttackDelay = i * AttackCoordinationDelay;
            
            // Schedule attack with delay
            FTimerHandle AttackTimer;
            GetWorld()->GetTimerManager().SetTimer(AttackTimer, [this, i]()
            {
                if (PackMembers.IsValidIndex(i) && PackMembers[i].DinosaurPawn)
                {
                    // Execute attack behavior
                    UE_LOG(LogTemp, Log, TEXT("Pack member %d attacking"), i);
                }
            }, AttackDelay, false);
        }
    }
}

void UNPC_DinosaurPackBehavior::RetreatPack()
{
    CurrentPackState = ENPC_PackState::Retreating;
    CurrentTarget = nullptr;
    
    // Move pack away from danger
    if (PackLeader)
    {
        FVector RetreatDirection = -PackLeader->GetActorForwardVector();
        FVector RetreatPosition = PackLeader->GetActorLocation() + (RetreatDirection * 2000.0f);
        
        for (FNPC_PackMember& Member : PackMembers)
        {
            if (Member.bIsAlive && Member.DinosaurPawn)
            {
                Member.LastKnownPosition = RetreatPosition + FMath::VRand() * 500.0f;
            }
        }
    }
    
    CommunicateWithPack(TEXT("Pack retreating"));
}

bool UNPC_DinosaurPackBehavior::IsPackIntact() const
{
    int32 AliveMembers = 0;
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.bIsAlive)
        {
            AliveMembers++;
        }
    }
    
    return AliveMembers >= MinPackSize;
}

FVector UNPC_DinosaurPackBehavior::GetPackCenterPosition() const
{
    if (PackMembers.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    
    FVector CenterPos = FVector::ZeroVector;
    int32 ValidMembers = 0;
    
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.bIsAlive && Member.DinosaurPawn)
        {
            CenterPos += Member.DinosaurPawn->GetActorLocation();
            ValidMembers++;
        }
    }
    
    return ValidMembers > 0 ? CenterPos / ValidMembers : FVector::ZeroVector;
}

APawn* UNPC_DinosaurPackBehavior::GetNearestPackMember(FVector Position) const
{
    APawn* NearestMember = nullptr;
    float MinDistance = FLT_MAX;
    
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.bIsAlive && Member.DinosaurPawn)
        {
            float Distance = FVector::Dist(Position, Member.DinosaurPawn->GetActorLocation());
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                NearestMember = Member.DinosaurPawn;
            }
        }
    }
    
    return NearestMember;
}

void UNPC_DinosaurPackBehavior::UpdatePackFormation()
{
    if (!PackLeader || PackMembers.Num() == 0)
    {
        return;
    }
    
    FVector LeaderPos = PackLeader->GetActorLocation();
    FVector LeaderForward = PackLeader->GetActorForwardVector();
    FVector LeaderRight = PackLeader->GetActorRightVector();
    
    for (int32 i = 0; i < PackMembers.Num() && i < FormationPositions.Num(); i++)
    {
        if (PackMembers[i].bIsAlive && PackMembers[i].DinosaurPawn)
        {
            FVector FormationOffset = FormationPositions[i];
            FVector WorldFormationPos = LeaderPos + 
                (LeaderForward * FormationOffset.X) + 
                (LeaderRight * FormationOffset.Y);
            
            PackMembers[i].LastKnownPosition = WorldFormationPos;
        }
    }
}

void UNPC_DinosaurPackBehavior::UpdatePackState()
{
    if (!IsPackIntact())
    {
        CurrentPackState = ENPC_PackState::Retreating;
        return;
    }
    
    // Update member health and stamina
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.DinosaurPawn)
        {
            // This would interface with health/stamina components
            Member.HealthPercentage = 100.0f; // Placeholder
            Member.StaminaPercentage = 100.0f; // Placeholder
        }
    }
    
    // State transition logic
    switch (CurrentPackState)
    {
        case ENPC_PackState::Idle:
            if (CurrentTarget && IsTargetWithinHuntingRange(CurrentTarget))
            {
                ExecutePackHunt();
            }
            break;
            
        case ENPC_PackState::Hunting:
            if (!CurrentTarget || !IsTargetWithinHuntingRange(CurrentTarget))
            {
                CurrentPackState = ENPC_PackState::Idle;
            }
            else
            {
                float DistanceToTarget = FVector::Dist(GetPackCenterPosition(), CurrentTarget->GetActorLocation());
                if (DistanceToTarget < 500.0f) // Attack range
                {
                    ExecuteCoordinatedAttack();
                }
            }
            break;
            
        case ENPC_PackState::Attacking:
            // Attack duration logic would go here
            break;
            
        case ENPC_PackState::Retreating:
            // Check if safe to return to idle
            break;
    }
}

void UNPC_DinosaurPackBehavior::ExecutePackBehavior(float DeltaTime)
{
    // Implement frame-by-frame pack behavior execution
    switch (CurrentPackState)
    {
        case ENPC_PackState::Idle:
            // Patrol behavior, foraging, etc.
            break;
            
        case ENPC_PackState::Hunting:
            // Coordinate movement towards target
            break;
            
        case ENPC_PackState::Attacking:
            // Execute attack patterns
            break;
            
        case ENPC_PackState::Retreating:
            // Move away from danger
            break;
    }
}

void UNPC_DinosaurPackBehavior::AssignPackRoles()
{
    if (PackMembers.Num() == 0)
    {
        return;
    }
    
    // Ensure first member is leader
    if (PackMembers.Num() > 0)
    {
        PackMembers[0].Role = ENPC_PackRole::Leader;
    }
    
    // Assign roles based on pack size
    for (int32 i = 1; i < PackMembers.Num(); i++)
    {
        switch (i % 3)
        {
            case 0:
                PackMembers[i].Role = ENPC_PackRole::Hunter;
                break;
            case 1:
                PackMembers[i].Role = ENPC_PackRole::Scout;
                break;
            case 2:
                PackMembers[i].Role = ENPC_PackRole::Guardian;
                break;
        }
    }
}

FVector UNPC_DinosaurPackBehavior::CalculateFlankingPosition(APawn* Target, int32 MemberIndex)
{
    if (!Target || !PackLeader)
    {
        return FVector::ZeroVector;
    }
    
    FVector TargetPos = Target->GetActorLocation();
    FVector LeaderPos = PackLeader->GetActorLocation();
    FVector ToTarget = (TargetPos - LeaderPos).GetSafeNormal();
    FVector RightVector = FVector::CrossProduct(ToTarget, FVector::UpVector);
    
    // Calculate flanking positions around target
    float AngleOffset = (MemberIndex * 60.0f) - 180.0f; // Spread around target
    float RadianOffset = FMath::DegreesToRadians(AngleOffset);
    
    FVector FlankDirection = ToTarget.RotateAngleAxis(AngleOffset, FVector::UpVector);
    FVector FlankPosition = TargetPos + (FlankDirection * 800.0f);
    
    return FlankPosition;
}

bool UNPC_DinosaurPackBehavior::IsTargetWithinHuntingRange(APawn* Target) const
{
    if (!Target || !PackLeader)
    {
        return false;
    }
    
    float Distance = FVector::Dist(PackLeader->GetActorLocation(), Target->GetActorLocation());
    return Distance <= HuntingRange;
}

void UNPC_DinosaurPackBehavior::CommunicateWithPack(const FString& Message)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastCommunicationTime > 1.0f) // Limit communication frequency
    {
        UE_LOG(LogTemp, Log, TEXT("Pack Communication: %s"), *Message);
        LastCommunicationTime = CurrentTime;
        
        // This could trigger audio cues, visual effects, etc.
    }
}