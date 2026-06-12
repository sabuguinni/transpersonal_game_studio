#include "Combat_PackBehavior.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"

UCombat_PackBehavior::UCombat_PackBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Default settings
    AttackCoordinationRadius = 500.0f;
    FlankingDistance = 400.0f;
    RetreatThreshold = 0.3f; // 30% pack members alive
    PackState = ECombat_PackState::Idle;
    LastFormationUpdate = 0.0f;

    // Default formation
    CurrentFormation.FormationType = ECombat_FormationType::Circle;
    CurrentFormation.FormationRadius = 300.0f;
}

void UCombat_PackBehavior::BeginPlay()
{
    Super::BeginPlay();
    
    PackState = ECombat_PackState::Idle;
    LastFormationUpdate = GetWorld()->GetTimeSeconds();
}

void UCombat_PackBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsPackLeader())
    {
        return;
    }

    UpdatePackState();
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastFormationUpdate > 1.0f)
    {
        UpdatePackFormation();
        LastFormationUpdate = CurrentTime;
    }
}

void UCombat_PackBehavior::InitializePack(AActor* Leader, const TArray<AActor*>& Members)
{
    if (!Leader)
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat_PackBehavior: Cannot initialize pack without leader"));
        return;
    }

    PackLeader = Leader;
    PackMembers.Empty();

    // Add leader as first member
    FCombat_PackMember LeaderMember;
    LeaderMember.MemberActor = Leader;
    LeaderMember.Role = ECombat_PackRole::Leader;
    LeaderMember.DistanceFromLeader = 0.0f;
    LeaderMember.bIsAlive = true;
    PackMembers.Add(LeaderMember);

    // Add other members
    for (AActor* Member : Members)
    {
        if (Member && Member != Leader)
        {
            AddPackMember(Member, ECombat_PackRole::Follower);
        }
    }

    PackState = ECombat_PackState::Idle;
    SetPackFormation(ECombat_FormationType::Circle);

    UE_LOG(LogTemp, Log, TEXT("Combat_PackBehavior: Pack initialized with %d members"), PackMembers.Num());
}

void UCombat_PackBehavior::AddPackMember(AActor* NewMember, ECombat_PackRole Role)
{
    if (!NewMember)
    {
        return;
    }

    // Check if already in pack
    for (const FCombat_PackMember& Member : PackMembers)
    {
        if (Member.MemberActor == NewMember)
        {
            return;
        }
    }

    FCombat_PackMember NewPackMember;
    NewPackMember.MemberActor = NewMember;
    NewPackMember.Role = Role;
    NewPackMember.bIsAlive = true;

    if (PackLeader.IsValid())
    {
        float Distance = FVector::Dist(NewMember->GetActorLocation(), PackLeader->GetActorLocation());
        NewPackMember.DistanceFromLeader = Distance;
    }

    PackMembers.Add(NewPackMember);
    UpdatePackFormation();
}

void UCombat_PackBehavior::RemovePackMember(AActor* Member)
{
    if (!Member)
    {
        return;
    }

    for (int32 i = PackMembers.Num() - 1; i >= 0; i--)
    {
        if (PackMembers[i].MemberActor == Member)
        {
            PackMembers.RemoveAt(i);
            break;
        }
    }

    UpdatePackFormation();
}

void UCombat_PackBehavior::SetPackFormation(ECombat_FormationType NewFormation)
{
    CurrentFormation.FormationType = NewFormation;
    CurrentFormation.RelativePositions.Empty();

    int32 MemberCount = PackMembers.Num();
    if (MemberCount <= 1)
    {
        return;
    }

    switch (NewFormation)
    {
        case ECombat_FormationType::Circle:
        {
            float AngleStep = 360.0f / (MemberCount - 1); // Exclude leader
            for (int32 i = 1; i < MemberCount; i++) // Start from 1 to skip leader
            {
                float Angle = (i - 1) * AngleStep;
                float RadianAngle = FMath::DegreesToRadians(Angle);
                FVector Position = FVector(
                    FMath::Cos(RadianAngle) * CurrentFormation.FormationRadius,
                    FMath::Sin(RadianAngle) * CurrentFormation.FormationRadius,
                    0.0f
                );
                CurrentFormation.RelativePositions.Add(Position);
            }
            break;
        }
        case ECombat_FormationType::Line:
        {
            float Spacing = CurrentFormation.FormationRadius / FMath::Max(1, MemberCount - 1);
            for (int32 i = 1; i < MemberCount; i++)
            {
                FVector Position = FVector((i - 1) * Spacing - CurrentFormation.FormationRadius * 0.5f, 0.0f, 0.0f);
                CurrentFormation.RelativePositions.Add(Position);
            }
            break;
        }
        case ECombat_FormationType::Wedge:
        {
            for (int32 i = 1; i < MemberCount; i++)
            {
                float Side = (i % 2 == 0) ? 1.0f : -1.0f;
                float Distance = (i / 2 + 1) * 100.0f;
                FVector Position = FVector(-Distance, Side * Distance * 0.5f, 0.0f);
                CurrentFormation.RelativePositions.Add(Position);
            }
            break;
        }
    }

    UpdatePackFormation();
}

void UCombat_PackBehavior::InitiatePackAttack(AActor* Target)
{
    if (!Target || !IsPackLeader())
    {
        return;
    }

    CurrentTarget = Target;
    PackState = ECombat_PackState::Attacking;
    
    BroadcastPackCommand(ECombat_PackCommand::Attack, Target);
    
    UE_LOG(LogTemp, Log, TEXT("Combat_PackBehavior: Pack attack initiated on %s"), *Target->GetName());
}

void UCombat_PackBehavior::ExecuteFlankingManeuver(AActor* Target)
{
    if (!Target || !IsPackLeader())
    {
        return;
    }

    CurrentTarget = Target;
    PackState = ECombat_PackState::Flanking;

    // Calculate flanking positions
    FVector TargetLocation = Target->GetActorLocation();
    FVector LeaderLocation = PackLeader->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - LeaderLocation).GetSafeNormal();
    FVector RightVector = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);

    // Assign flanking positions to pack members
    for (int32 i = 1; i < PackMembers.Num(); i++)
    {
        if (PackMembers[i].bIsAlive && PackMembers[i].MemberActor.IsValid())
        {
            float Side = (i % 2 == 0) ? 1.0f : -1.0f;
            FVector FlankPosition = TargetLocation + (RightVector * Side * FlankingDistance);
            
            // Here you would typically send movement commands to the AI
            // This is a simplified version for demonstration
            PackMembers[i].MemberActor->SetActorLocation(FlankPosition);
        }
    }

    BroadcastPackCommand(ECombat_PackCommand::Flank, Target);
    
    UE_LOG(LogTemp, Log, TEXT("Combat_PackBehavior: Flanking maneuver executed"));
}

void UCombat_PackBehavior::RetreatFormation()
{
    if (!IsPackLeader())
    {
        return;
    }

    PackState = ECombat_PackState::Retreating;
    CurrentTarget = nullptr;
    
    BroadcastPackCommand(ECombat_PackCommand::Retreat);
    
    UE_LOG(LogTemp, Log, TEXT("Combat_PackBehavior: Pack retreat initiated"));
}

bool UCombat_PackBehavior::IsPackLeader() const
{
    return PackLeader.IsValid() && PackLeader.Get() == GetOwner();
}

int32 UCombat_PackBehavior::GetPackSize() const
{
    int32 LivingMembers = 0;
    for (const FCombat_PackMember& Member : PackMembers)
    {
        if (Member.bIsAlive)
        {
            LivingMembers++;
        }
    }
    return LivingMembers;
}

AActor* UCombat_PackBehavior::GetPackLeader() const
{
    return PackLeader.Get();
}

TArray<AActor*> UCombat_PackBehavior::GetLivingMembers() const
{
    TArray<AActor*> LivingMembers;
    for (const FCombat_PackMember& Member : PackMembers)
    {
        if (Member.bIsAlive && Member.MemberActor.IsValid())
        {
            LivingMembers.Add(Member.MemberActor.Get());
        }
    }
    return LivingMembers;
}

void UCombat_PackBehavior::UpdatePackFormation()
{
    if (!IsPackLeader() || !PackLeader.IsValid())
    {
        return;
    }

    FVector LeaderLocation = PackLeader->GetActorLocation();
    FRotator LeaderRotation = PackLeader->GetActorRotation();

    for (int32 i = 1; i < PackMembers.Num(); i++)
    {
        if (PackMembers[i].bIsAlive && PackMembers[i].MemberActor.IsValid())
        {
            if (i - 1 < CurrentFormation.RelativePositions.Num())
            {
                FVector RelativePos = CurrentFormation.RelativePositions[i - 1];
                FVector WorldPos = LeaderLocation + LeaderRotation.RotateVector(RelativePos);
                
                // Update distance from leader
                PackMembers[i].DistanceFromLeader = FVector::Dist(WorldPos, LeaderLocation);
                
                // Here you would typically send movement commands to the AI
                // This is a simplified version for demonstration
                if (PackState == ECombat_PackState::Idle || PackState == ECombat_PackState::Moving)
                {
                    PackMembers[i].MemberActor->SetActorLocation(WorldPos);
                }
            }
        }
    }
}

void UCombat_PackBehavior::UpdatePackState()
{
    int32 LivingCount = GetPackSize();
    int32 TotalCount = PackMembers.Num();
    
    // Check for retreat condition
    if (TotalCount > 0 && (float)LivingCount / (float)TotalCount < RetreatThreshold)
    {
        if (PackState == ECombat_PackState::Attacking || PackState == ECombat_PackState::Flanking)
        {
            RetreatFormation();
        }
    }

    // Update member alive status
    for (FCombat_PackMember& Member : PackMembers)
    {
        if (Member.MemberActor.IsValid())
        {
            // Here you would check actual health/alive status
            // This is a placeholder implementation
            Member.bIsAlive = IsValid(Member.MemberActor.Get());
        }
        else
        {
            Member.bIsAlive = false;
        }
    }
}

FVector UCombat_PackBehavior::CalculateFormationPosition(int32 MemberIndex)
{
    if (!PackLeader.IsValid() || MemberIndex <= 0 || MemberIndex - 1 >= CurrentFormation.RelativePositions.Num())
    {
        return FVector::ZeroVector;
    }

    FVector LeaderLocation = PackLeader->GetActorLocation();
    FRotator LeaderRotation = PackLeader->GetActorRotation();
    FVector RelativePosition = CurrentFormation.RelativePositions[MemberIndex - 1];
    
    return LeaderLocation + LeaderRotation.RotateVector(RelativePosition);
}

void UCombat_PackBehavior::BroadcastPackCommand(ECombat_PackCommand Command, AActor* Target)
{
    // This would typically interface with the AI behavior tree system
    // For now, we'll just log the command
    FString CommandStr;
    switch (Command)
    {
        case ECombat_PackCommand::Attack:
            CommandStr = TEXT("Attack");
            break;
        case ECombat_PackCommand::Flank:
            CommandStr = TEXT("Flank");
            break;
        case ECombat_PackCommand::Retreat:
            CommandStr = TEXT("Retreat");
            break;
        case ECombat_PackCommand::Hold:
            CommandStr = TEXT("Hold");
            break;
        default:
            CommandStr = TEXT("Unknown");
            break;
    }
    
    FString TargetName = Target ? Target->GetName() : TEXT("None");
    UE_LOG(LogTemp, Log, TEXT("Combat_PackBehavior: Broadcasting command %s to pack (Target: %s)"), *CommandStr, *TargetName);
}