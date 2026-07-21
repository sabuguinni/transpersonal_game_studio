#include "DinosaurPackBehavior.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UDinosaurPackBehavior::UDinosaurPackBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update twice per second

    // Initialize pack behavior settings
    PackCohesionRadius = 1000.0f;
    HuntingRadius = 2000.0f;
    FleeRadius = 1500.0f;
    MaxPackSize = 6;
    PackMoraleLevel = 75.0f;
    PackFormationSpacing = 300.0f;

    CurrentPackState = EPackBehaviorState::Idle;
    PackLeader = nullptr;
    CurrentTarget = nullptr;
    LastCohesionUpdate = 0.0f;
    LastMoraleUpdate = 0.0f;
}

void UDinosaurPackBehavior::BeginPlay()
{
    Super::BeginPlay();

    // Initialize pack with owner as potential leader
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        AddPackMember(OwnerPawn, EDinosaurPackRole::Alpha);
        PackLeader = OwnerPawn;
    }

    // Start with idle patrol behavior
    SetPackState(EPackBehaviorState::Patrolling);
    
    UE_LOG(LogTemp, Warning, TEXT("DinosaurPackBehavior initialized for: %s"), GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"));
}

void UDinosaurPackBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Update pack cohesion every 2 seconds
    if (CurrentTime - LastCohesionUpdate > 2.0f)
    {
        UpdatePackCohesion();
        LastCohesionUpdate = CurrentTime;
    }

    // Update pack morale every 5 seconds
    if (CurrentTime - LastMoraleUpdate > 5.0f)
    {
        UpdatePackMorale();
        LastMoraleUpdate = CurrentTime;
    }

    // Update pack member status
    UpdatePackMemberStatus();

    // Handle pack behavior based on current state
    switch (CurrentPackState)
    {
        case EPackBehaviorState::Hunting:
            HandlePackCommunication();
            if (CurrentTarget)
            {
                CoordinateAttack(CurrentTarget);
            }
            break;

        case EPackBehaviorState::Patrolling:
            ExecutePackFormation();
            break;

        case EPackBehaviorState::Fleeing:
            // Scatter formation when fleeing
            break;

        default:
            break;
    }
}

void UDinosaurPackBehavior::AddPackMember(APawn* NewMember, EDinosaurPackRole Role)
{
    if (!NewMember || PackMembers.Num() >= MaxPackSize)
    {
        return;
    }

    // Check if already in pack
    for (const FPackMember& Member : PackMembers)
    {
        if (Member.DinosaurPawn == NewMember)
        {
            return; // Already in pack
        }
    }

    FPackMember NewPackMember;
    NewPackMember.DinosaurPawn = NewMember;
    NewPackMember.Role = Role;
    NewPackMember.HealthPercentage = 100.0f;
    NewPackMember.HungerLevel = 50.0f;
    NewPackMember.bIsAlive = true;
    NewPackMember.LastKnownPosition = NewMember->GetActorLocation();

    PackMembers.Add(NewPackMember);

    // Assign as leader if Alpha role
    if (Role == EDinosaurPackRole::Alpha)
    {
        PackLeader = NewMember;
    }

    UE_LOG(LogTemp, Log, TEXT("Added pack member: %s with role: %d"), *NewMember->GetName(), (int32)Role);
}

void UDinosaurPackBehavior::RemovePackMember(APawn* MemberToRemove)
{
    if (!MemberToRemove) return;

    for (int32 i = PackMembers.Num() - 1; i >= 0; i--)
    {
        if (PackMembers[i].DinosaurPawn == MemberToRemove)
        {
            PackMembers.RemoveAt(i);
            
            // Reassign leader if removed member was leader
            if (PackLeader == MemberToRemove)
            {
                AssignPackRoles();
            }
            
            UE_LOG(LogTemp, Log, TEXT("Removed pack member: %s"), *MemberToRemove->GetName());
            break;
        }
    }

    // Update pack morale after member loss
    PackMoraleLevel = FMath::Max(0.0f, PackMoraleLevel - 15.0f);
}

void UDinosaurPackBehavior::SetPackState(EPackBehaviorState NewState)
{
    if (CurrentPackState != NewState)
    {
        CurrentPackState = NewState;
        
        UE_LOG(LogTemp, Warning, TEXT("Pack state changed to: %d"), (int32)NewState);

        // Notify all pack members of state change
        for (FPackMember& Member : PackMembers)
        {
            if (Member.bIsAlive && Member.DinosaurPawn)
            {
                // Could trigger behavior tree state changes here
            }
        }
    }
}

void UDinosaurPackBehavior::InitiateHunt(AActor* Target)
{
    if (!Target) return;

    CurrentTarget = Target;
    SetPackState(EPackBehaviorState::Hunting);

    UE_LOG(LogTemp, Warning, TEXT("Pack initiating hunt on target: %s"), *Target->GetName());

    // Increase pack morale when hunting
    PackMoraleLevel = FMath::Min(100.0f, PackMoraleLevel + 10.0f);
}

void UDinosaurPackBehavior::InitiateFlee(AActor* ThreatSource)
{
    if (!ThreatSource) return;

    SetPackState(EPackBehaviorState::Fleeing);
    CurrentTarget = nullptr;

    UE_LOG(LogTemp, Warning, TEXT("Pack fleeing from threat: %s"), *ThreatSource->GetName());

    // Decrease pack morale when fleeing
    PackMoraleLevel = FMath::Max(0.0f, PackMoraleLevel - 20.0f);
}

void UDinosaurPackBehavior::CoordinateAttack(AActor* Target)
{
    if (!Target || PackMembers.Num() == 0) return;

    FVector TargetLocation = Target->GetActorLocation();
    TArray<APawn*> LivingMembers = GetLivingPackMembers();

    if (LivingMembers.Num() == 0) return;

    // Coordinate pack attack formation
    for (int32 i = 0; i < LivingMembers.Num(); i++)
    {
        APawn* Member = LivingMembers[i];
        if (!Member) continue;

        // Calculate attack position around target
        float AngleStep = 360.0f / LivingMembers.Num();
        float Angle = i * AngleStep;
        float RadianAngle = FMath::DegreesToRadians(Angle);
        
        FVector AttackOffset = FVector(
            FMath::Cos(RadianAngle) * 400.0f,
            FMath::Sin(RadianAngle) * 400.0f,
            0.0f
        );

        FVector AttackPosition = TargetLocation + AttackOffset;

        // Move member to attack position (would integrate with AI controller)
        UE_LOG(LogTemp, Log, TEXT("Pack member %s attacking from position: %s"), 
               *Member->GetName(), *AttackPosition.ToString());
    }
}

FVector UDinosaurPackBehavior::GetPackCenterPosition()
{
    if (PackMembers.Num() == 0) return FVector::ZeroVector;

    FVector CenterPosition = FVector::ZeroVector;
    int32 ValidMembers = 0;

    for (const FPackMember& Member : PackMembers)
    {
        if (Member.bIsAlive && Member.DinosaurPawn)
        {
            CenterPosition += Member.DinosaurPawn->GetActorLocation();
            ValidMembers++;
        }
    }

    return ValidMembers > 0 ? CenterPosition / ValidMembers : FVector::ZeroVector;
}

TArray<APawn*> UDinosaurPackBehavior::GetLivingPackMembers()
{
    TArray<APawn*> LivingMembers;
    
    for (const FPackMember& Member : PackMembers)
    {
        if (Member.bIsAlive && Member.DinosaurPawn)
        {
            LivingMembers.Add(Member.DinosaurPawn);
        }
    }

    return LivingMembers;
}

bool UDinosaurPackBehavior::IsPackIntact()
{
    int32 LivingMembers = 0;
    
    for (const FPackMember& Member : PackMembers)
    {
        if (Member.bIsAlive)
        {
            LivingMembers++;
        }
    }

    return LivingMembers >= 2; // Pack needs at least 2 members to be intact
}

void UDinosaurPackBehavior::UpdatePackMorale()
{
    float MoraleChange = 0.0f;

    // Morale factors
    int32 LivingMembers = GetLivingPackMembers().Num();
    
    if (LivingMembers >= 4)
    {
        MoraleChange += 2.0f; // Strong pack
    }
    else if (LivingMembers <= 1)
    {
        MoraleChange -= 5.0f; // Isolated
    }

    // Successful hunt increases morale
    if (CurrentPackState == EPackBehaviorState::Feeding)
    {
        MoraleChange += 3.0f;
    }

    // Apply morale change
    PackMoraleLevel = FMath::Clamp(PackMoraleLevel + MoraleChange, 0.0f, 100.0f);
}

void UDinosaurPackBehavior::AssignPackRoles()
{
    if (PackMembers.Num() == 0) return;

    // Find strongest member for Alpha role
    FPackMember* StrongestMember = nullptr;
    float HighestHealth = 0.0f;

    for (FPackMember& Member : PackMembers)
    {
        if (Member.bIsAlive && Member.HealthPercentage > HighestHealth)
        {
            HighestHealth = Member.HealthPercentage;
            StrongestMember = &Member;
        }
    }

    if (StrongestMember)
    {
        StrongestMember->Role = EDinosaurPackRole::Alpha;
        PackLeader = StrongestMember->DinosaurPawn;
    }

    // Assign other roles based on pack size
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (PackMembers[i].Role != EDinosaurPackRole::Alpha)
        {
            if (i == 1)
            {
                PackMembers[i].Role = EDinosaurPackRole::Beta;
            }
            else if (i % 2 == 0)
            {
                PackMembers[i].Role = EDinosaurPackRole::Hunter;
            }
            else
            {
                PackMembers[i].Role = EDinosaurPackRole::Scout;
            }
        }
    }
}

void UDinosaurPackBehavior::UpdatePackCohesion()
{
    if (PackMembers.Num() <= 1) return;

    FVector PackCenter = GetPackCenterPosition();
    
    for (FPackMember& Member : PackMembers)
    {
        if (!Member.bIsAlive || !Member.DinosaurPawn) continue;

        float DistanceFromCenter = FVector::Dist(Member.DinosaurPawn->GetActorLocation(), PackCenter);
        
        // If member is too far from pack, reduce morale
        if (DistanceFromCenter > PackCohesionRadius)
        {
            PackMoraleLevel = FMath::Max(0.0f, PackMoraleLevel - 1.0f);
        }
    }
}

void UDinosaurPackBehavior::UpdatePackMemberStatus()
{
    for (FPackMember& Member : PackMembers)
    {
        if (Member.DinosaurPawn)
        {
            Member.LastKnownPosition = Member.DinosaurPawn->GetActorLocation();
            
            // Simple health check (would integrate with actual health system)
            if (Member.DinosaurPawn->IsValidLowLevel())
            {
                Member.bIsAlive = true;
                // Member.HealthPercentage would be updated from actual health component
            }
        }
        else
        {
            Member.bIsAlive = false;
        }
    }
}

void UDinosaurPackBehavior::HandlePackCommunication()
{
    // Simple pack communication - could be expanded with audio cues
    if (PackLeader && CurrentTarget)
    {
        UE_LOG(LogTemp, Log, TEXT("Pack leader %s coordinating attack on %s"), 
               *PackLeader->GetName(), *CurrentTarget->GetName());
    }
}

void UDinosaurPackBehavior::ExecutePackFormation()
{
    if (!PackLeader || PackMembers.Num() <= 1) return;

    FVector LeaderPosition = PackLeader->GetActorLocation();
    
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        FPackMember& Member = PackMembers[i];
        if (!Member.bIsAlive || !Member.DinosaurPawn || Member.DinosaurPawn == PackLeader) continue;

        // Calculate formation position relative to leader
        float AngleStep = 360.0f / (PackMembers.Num() - 1);
        float Angle = i * AngleStep;
        float RadianAngle = FMath::DegreesToRadians(Angle);
        
        FVector FormationOffset = FVector(
            FMath::Cos(RadianAngle) * PackFormationSpacing,
            FMath::Sin(RadianAngle) * PackFormationSpacing,
            0.0f
        );

        FVector TargetPosition = LeaderPosition + FormationOffset;
        
        // This would integrate with AI movement system
        UE_LOG(LogTemp, VeryVerbose, TEXT("Pack member %s formation position: %s"), 
               *Member.DinosaurPawn->GetName(), *TargetPosition.ToString());
    }
}