#include "NPC_PackDynamicsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"

UNPC_PackDynamicsManager::UNPC_PackDynamicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Default pack settings
    MaxPackSize = 8.0f;
    PackCohesionRadius = 2000.0f;
    PackSeparationDistance = 150.0f;
    LeadershipTransferCooldown = 5.0f;
    bAutoFormation = true;
    bEnablePackHunting = true;
    
    // Internal timers
    LastCohesionCheck = 0.0f;
    LastLeadershipChange = 0.0f;
    PackUpdateInterval = 0.1f;
    
    // Initialize formation
    CurrentFormation.FormationType = ENPC_PackFormationType::Line;
    CurrentFormation.FormationRadius = 500.0f;
    CurrentFormation.FormationSpacing = 200.0f;
}

void UNPC_PackDynamicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Pack Dynamics Manager initialized"));
}

void UNPC_PackDynamicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (PackLeader.IsValid() && PackMembers.Num() > 0)
    {
        UpdatePackMemberPositions(DeltaTime);
        UpdatePackCohesion(DeltaTime);
        CheckPackCohesion();
        ValidatePackMembers();
    }
}

void UNPC_PackDynamicsManager::InitializePack(APawn* LeaderPawn, const TArray<APawn*>& MemberPawns)
{
    if (!LeaderPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot initialize pack without a valid leader"));
        return;
    }
    
    PackLeader = LeaderPawn;
    PackMembers.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Initializing pack with leader: %s"), *LeaderPawn->GetName());
    
    // Add pack members
    for (APawn* Member : MemberPawns)
    {
        if (Member && Member != LeaderPawn)
        {
            AddPackMember(Member, ENPC_PackRole::Member);
        }
    }
    
    // Set initial formation
    if (bAutoFormation)
    {
        UpdateFormationPositions();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Pack initialized with %d members"), PackMembers.Num());
}

void UNPC_PackDynamicsManager::AddPackMember(APawn* NewMember, ENPC_PackRole Role)
{
    if (!NewMember || PackMembers.Num() >= MaxPackSize)
    {
        return;
    }
    
    // Check if already a member
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.DinosaurPawn == NewMember)
        {
            return; // Already in pack
        }
    }
    
    FNPC_PackMember NewPackMember;
    NewPackMember.DinosaurPawn = NewMember;
    NewPackMember.PackRole = Role;
    NewPackMember.LoyaltyLevel = 1.0f;
    NewPackMember.bIsActive = true;
    
    if (PackLeader.IsValid())
    {
        FVector LeaderLocation = PackLeader->GetActorLocation();
        FVector MemberLocation = NewMember->GetActorLocation();
        NewPackMember.DistanceFromLeader = FVector::Dist(LeaderLocation, MemberLocation);
    }
    
    PackMembers.Add(NewPackMember);
    
    UE_LOG(LogTemp, Log, TEXT("Added pack member: %s (Role: %d)"), *NewMember->GetName(), (int32)Role);
    
    if (bAutoFormation)
    {
        UpdateFormationPositions();
    }
}

void UNPC_PackDynamicsManager::RemovePackMember(APawn* MemberToRemove)
{
    if (!MemberToRemove)
    {
        return;
    }
    
    for (int32 i = PackMembers.Num() - 1; i >= 0; --i)
    {
        if (PackMembers[i].DinosaurPawn == MemberToRemove)
        {
            UE_LOG(LogTemp, Log, TEXT("Removing pack member: %s"), *MemberToRemove->GetName());
            PackMembers.RemoveAt(i);
            break;
        }
    }
    
    if (bAutoFormation)
    {
        UpdateFormationPositions();
    }
}

void UNPC_PackDynamicsManager::ChangePackLeader(APawn* NewLeader)
{
    if (!NewLeader || !IsPackMember(NewLeader))
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastLeadershipChange < LeadershipTransferCooldown)
    {
        return; // Cooldown not elapsed
    }
    
    APawn* OldLeader = PackLeader.Get();
    PackLeader = NewLeader;
    LastLeadershipChange = CurrentTime;
    
    // Convert old leader to pack member
    if (OldLeader)
    {
        AddPackMember(OldLeader, ENPC_PackRole::Member);
    }
    
    // Remove new leader from members list
    RemovePackMember(NewLeader);
    
    UE_LOG(LogTemp, Log, TEXT("Pack leadership changed from %s to %s"), 
           OldLeader ? *OldLeader->GetName() : TEXT("None"), *NewLeader->GetName());
    
    if (bAutoFormation)
    {
        UpdateFormationPositions();
    }
}

void UNPC_PackDynamicsManager::SetPackFormation(ENPC_PackFormationType NewFormation)
{
    CurrentFormation.FormationType = NewFormation;
    UpdateFormationPositions();
    
    UE_LOG(LogTemp, Log, TEXT("Pack formation changed to: %d"), (int32)NewFormation);
}

void UNPC_PackDynamicsManager::UpdateFormationPositions()
{
    if (!PackLeader.IsValid() || PackMembers.Num() == 0)
    {
        return;
    }
    
    CurrentFormation.MemberPositions.Empty();
    
    for (int32 i = 0; i < PackMembers.Num(); ++i)
    {
        FVector FormationPos = CalculateFormationPosition(i);
        CurrentFormation.MemberPositions.Add(FormationPos);
    }
}

FVector UNPC_PackDynamicsManager::GetMemberTargetPosition(APawn* Member) const
{
    if (!PackLeader.IsValid() || !Member)
    {
        return FVector::ZeroVector;
    }
    
    // Find member index
    for (int32 i = 0; i < PackMembers.Num(); ++i)
    {
        if (PackMembers[i].DinosaurPawn == Member)
        {
            if (CurrentFormation.MemberPositions.IsValidIndex(i))
            {
                FVector LeaderLocation = PackLeader->GetActorLocation();
                return LeaderLocation + CurrentFormation.MemberPositions[i];
            }
            break;
        }
    }
    
    return PackLeader->GetActorLocation();
}

void UNPC_PackDynamicsManager::IssuePackCommand(ENPC_PackCommand Command, const FVector& TargetLocation)
{
    UE_LOG(LogTemp, Log, TEXT("Issuing pack command: %d"), (int32)Command);
    
    // Implementation would depend on AI controller integration
    // For now, just log the command
    switch (Command)
    {
        case ENPC_PackCommand::Hunt:
            UE_LOG(LogTemp, Log, TEXT("Pack hunting command issued"));
            break;
        case ENPC_PackCommand::Patrol:
            UE_LOG(LogTemp, Log, TEXT("Pack patrol command issued"));
            break;
        case ENPC_PackCommand::Defend:
            UE_LOG(LogTemp, Log, TEXT("Pack defend command issued"));
            break;
        case ENPC_PackCommand::Retreat:
            UE_LOG(LogTemp, Log, TEXT("Pack retreat command issued"));
            break;
        case ENPC_PackCommand::Rest:
            UE_LOG(LogTemp, Log, TEXT("Pack rest command issued"));
            break;
    }
}

void UNPC_PackDynamicsManager::UpdatePackCohesion(float DeltaTime)
{
    if (!PackLeader.IsValid())
    {
        return;
    }
    
    FVector LeaderLocation = PackLeader->GetActorLocation();
    
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.DinosaurPawn.IsValid())
        {
            FVector MemberLocation = Member.DinosaurPawn->GetActorLocation();
            float Distance = FVector::Dist(LeaderLocation, MemberLocation);
            Member.DistanceFromLeader = Distance;
            
            // Adjust loyalty based on distance and time
            if (Distance > PackCohesionRadius)
            {
                Member.LoyaltyLevel = FMath::Max(0.0f, Member.LoyaltyLevel - DeltaTime * 0.1f);
            }
            else
            {
                Member.LoyaltyLevel = FMath::Min(1.0f, Member.LoyaltyLevel + DeltaTime * 0.05f);
            }
        }
    }
}

void UNPC_PackDynamicsManager::HandleMemberThreat(APawn* ThreatenedMember, AActor* ThreatSource)
{
    if (!ThreatenedMember || !ThreatSource || !IsPackMember(ThreatenedMember))
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Pack member %s is threatened by %s"), 
           *ThreatenedMember->GetName(), *ThreatSource->GetName());
    
    // Issue defend command to pack
    IssuePackCommand(ENPC_PackCommand::Defend, ThreatSource->GetActorLocation());
}

bool UNPC_PackDynamicsManager::IsPackLeader(APawn* Pawn) const
{
    return PackLeader.IsValid() && PackLeader.Get() == Pawn;
}

bool UNPC_PackDynamicsManager::IsPackMember(APawn* Pawn) const
{
    if (!Pawn)
    {
        return false;
    }
    
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.DinosaurPawn == Pawn)
        {
            return true;
        }
    }
    
    return false;
}

int32 UNPC_PackDynamicsManager::GetPackSize() const
{
    int32 ActiveMembers = 0;
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.bIsActive && Member.DinosaurPawn.IsValid())
        {
            ActiveMembers++;
        }
    }
    return ActiveMembers + (PackLeader.IsValid() ? 1 : 0);
}

float UNPC_PackDynamicsManager::GetPackCohesion() const
{
    if (PackMembers.Num() == 0)
    {
        return 1.0f;
    }
    
    float TotalLoyalty = 0.0f;
    int32 ValidMembers = 0;
    
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.bIsActive && Member.DinosaurPawn.IsValid())
        {
            TotalLoyalty += Member.LoyaltyLevel;
            ValidMembers++;
        }
    }
    
    return ValidMembers > 0 ? TotalLoyalty / ValidMembers : 0.0f;
}

APawn* UNPC_PackDynamicsManager::GetPackLeader() const
{
    return PackLeader.Get();
}

TArray<APawn*> UNPC_PackDynamicsManager::GetPackMembers() const
{
    TArray<APawn*> Members;
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.bIsActive && Member.DinosaurPawn.IsValid())
        {
            Members.Add(Member.DinosaurPawn.Get());
        }
    }
    return Members;
}

void UNPC_PackDynamicsManager::UpdatePackMemberPositions(float DeltaTime)
{
    // This would integrate with AI movement systems
    // For now, just update distances
    if (PackLeader.IsValid())
    {
        FVector LeaderLocation = PackLeader->GetActorLocation();
        
        for (FNPC_PackMember& Member : PackMembers)
        {
            if (Member.DinosaurPawn.IsValid())
            {
                FVector MemberLocation = Member.DinosaurPawn->GetActorLocation();
                Member.DistanceFromLeader = FVector::Dist(LeaderLocation, MemberLocation);
            }
        }
    }
}

void UNPC_PackDynamicsManager::CheckPackCohesion()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastCohesionCheck < 1.0f) // Check every second
    {
        return;
    }
    
    LastCohesionCheck = CurrentTime;
    
    // Remove members that are too far or have low loyalty
    for (int32 i = PackMembers.Num() - 1; i >= 0; --i)
    {
        FNPC_PackMember& Member = PackMembers[i];
        if (!Member.DinosaurPawn.IsValid() || 
            Member.DistanceFromLeader > PackCohesionRadius * 2.0f ||
            Member.LoyaltyLevel < 0.1f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Removing pack member due to low cohesion"));
            PackMembers.RemoveAt(i);
        }
    }
}

void UNPC_PackDynamicsManager::HandlePackCommunication()
{
    // Future implementation for pack communication
    // Could include howling, roaring, visual signals, etc.
}

FVector UNPC_PackDynamicsManager::CalculateFormationPosition(int32 MemberIndex) const
{
    if (!PackLeader.IsValid())
    {
        return FVector::ZeroVector;
    }
    
    FVector Offset = FVector::ZeroVector;
    float Angle = 0.0f;
    float Distance = CurrentFormation.FormationSpacing;
    
    switch (CurrentFormation.FormationType)
    {
        case ENPC_PackFormationType::Line:
            Offset = FVector(0, (MemberIndex - PackMembers.Num() / 2) * Distance, 0);
            break;
            
        case ENPC_PackFormationType::Circle:
            Angle = (2.0f * PI * MemberIndex) / FMath::Max(1, PackMembers.Num());
            Offset = FVector(
                FMath::Cos(Angle) * CurrentFormation.FormationRadius,
                FMath::Sin(Angle) * CurrentFormation.FormationRadius,
                0
            );
            break;
            
        case ENPC_PackFormationType::Wedge:
            {
                int32 Row = FMath::Sqrt(MemberIndex);
                int32 Col = MemberIndex - (Row * Row);
                Offset = FVector(
                    -Row * Distance,
                    (Col - Row / 2) * Distance,
                    0
                );
            }
            break;
            
        case ENPC_PackFormationType::Scattered:
            {
                // Pseudo-random positions within formation radius
                float RandomAngle = (MemberIndex * 137.5f) * PI / 180.0f; // Golden angle
                float RandomDistance = FMath::Sqrt(MemberIndex / float(PackMembers.Num())) * CurrentFormation.FormationRadius;
                Offset = FVector(
                    FMath::Cos(RandomAngle) * RandomDistance,
                    FMath::Sin(RandomAngle) * RandomDistance,
                    0
                );
            }
            break;
    }
    
    return Offset;
}

void UNPC_PackDynamicsManager::ValidatePackMembers()
{
    // Remove invalid members
    for (int32 i = PackMembers.Num() - 1; i >= 0; --i)
    {
        if (!PackMembers[i].DinosaurPawn.IsValid())
        {
            PackMembers.RemoveAt(i);
        }
    }
    
    // Validate leader
    if (!PackLeader.IsValid() && PackMembers.Num() > 0)
    {
        // Promote a member to leader
        if (PackMembers[0].DinosaurPawn.IsValid())
        {
            APawn* NewLeader = PackMembers[0].DinosaurPawn.Get();
            PackMembers.RemoveAt(0);
            PackLeader = NewLeader;
            UE_LOG(LogTemp, Log, TEXT("Auto-promoted new pack leader: %s"), *NewLeader->GetName());
        }
    }
}