#include "Combat_PackHuntingAI.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"

UCombat_PackHuntingAI::UCombat_PackHuntingAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    PackRadius = 2000.0f;
    HuntingRange = 8000.0f;
    CurrentTarget = nullptr;
    bIsHunting = false;
    bIsAlpha = false;
    LastCoordinationUpdate = 0.0f;
    CoordinationInterval = 2.0f;
}

void UCombat_PackHuntingAI::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePack();
    
    if (GetOwner())
    {
        UE_LOG(LogTemp, Warning, TEXT("Pack Hunting AI initialized for: %s"), *GetOwner()->GetName());
    }
}

void UCombat_PackHuntingAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner()) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update pack coordination
    if (CurrentTime - LastCoordinationUpdate >= CoordinationInterval)
    {
        if (bIsAlpha)
        {
            if (!CurrentTarget)
            {
                CurrentTarget = FindNearestThreat();
            }
            
            if (CurrentTarget)
            {
                if (!bIsHunting)
                {
                    StartHunt(CurrentTarget);
                }
                ExecutePackTactics();
            }
        }
        
        UpdatePackFormation();
        LastCoordinationUpdate = CurrentTime;
    }
}

void UCombat_PackHuntingAI::InitializePack()
{
    // Check if this is the alpha (first spawned or designated leader)
    if (GetOwner() && GetOwner()->GetName().Contains("Pack_1"))
    {
        bIsAlpha = true;
        UE_LOG(LogTemp, Warning, TEXT("Pack Alpha initialized: %s"), *GetOwner()->GetName());
    }
    
    // Find nearby pack members
    if (bIsAlpha)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (Actor == GetOwner()) continue;
            
            if (Actor->GetName().Contains("Velociraptor_Pack"))
            {
                float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
                if (Distance <= PackRadius)
                {
                    ECombat_PackRole Role = ECombat_PackRole::Hunter;
                    if (Actor->GetName().Contains("Pack_2"))
                    {
                        Role = ECombat_PackRole::Flanker;
                    }
                    else if (Actor->GetName().Contains("Pack_3"))
                    {
                        Role = ECombat_PackRole::Scout;
                    }
                    
                    AddPackMember(Actor, Role);
                }
            }
        }
    }
}

void UCombat_PackHuntingAI::AddPackMember(AActor* NewMember, ECombat_PackRole Role)
{
    if (!NewMember) return;
    
    FCombat_PackMember Member;
    Member.Member = NewMember;
    Member.Role = Role;
    Member.bIsActive = true;
    Member.AssignedPosition = NewMember->GetActorLocation();
    
    PackMembers.Add(Member);
    
    UE_LOG(LogTemp, Log, TEXT("Pack member added: %s with role %d"), *NewMember->GetName(), (int32)Role);
}

void UCombat_PackHuntingAI::RemovePackMember(AActor* Member)
{
    PackMembers.RemoveAll([Member](const FCombat_PackMember& PackMember) {
        return PackMember.Member == Member;
    });
}

void UCombat_PackHuntingAI::StartHunt(AActor* Target)
{
    if (!Target || !bIsAlpha) return;
    
    CurrentTarget = Target;
    bIsHunting = true;
    
    AssignFlankingPositions();
    SendPackCommand("HUNT_START");
    
    UE_LOG(LogTemp, Warning, TEXT("Pack hunt started on target: %s"), *Target->GetName());
}

void UCombat_PackHuntingAI::ExecutePackTactics()
{
    if (!CurrentTarget || !bIsAlpha) return;
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector AlphaLocation = GetOwner()->GetActorLocation();
    
    // Alpha approaches directly
    FVector DirectionToTarget = (TargetLocation - AlphaLocation).GetSafeNormal();
    FVector NewAlphaPos = AlphaLocation + DirectionToTarget * 150.0f * GetWorld()->GetDeltaSeconds();
    GetOwner()->SetActorLocation(NewAlphaPos);
    
    // Coordinate pack member positions
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        FCombat_PackMember& Member = PackMembers[i];
        if (!Member.Member || !Member.bIsActive) continue;
        
        FVector MemberLocation = Member.Member->GetActorLocation();
        FVector TargetDirection = (TargetLocation - MemberLocation).GetSafeNormal();
        
        switch (Member.Role)
        {
            case ECombat_PackRole::Flanker:
            {
                // Circle around target
                float Angle = FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f) * 90.0f;
                FVector FlankDirection = TargetDirection.RotateAngleAxis(Angle, FVector::UpVector);
                FVector FlankPos = TargetLocation + FlankDirection * 1200.0f;
                Member.AssignedPosition = FlankPos;
                break;
            }
            case ECombat_PackRole::Scout:
            {
                // Maintain distance and observe
                FVector ScoutPos = TargetLocation + TargetDirection * -1500.0f;
                Member.AssignedPosition = ScoutPos;
                break;
            }
            default:
            {
                // Hunter - support alpha
                FVector HunterPos = TargetLocation + TargetDirection * -800.0f;
                Member.AssignedPosition = HunterPos;
                break;
            }
        }
        
        // Move towards assigned position
        FVector MoveDirection = (Member.AssignedPosition - MemberLocation).GetSafeNormal();
        FVector NewPos = MemberLocation + MoveDirection * 200.0f * GetWorld()->GetDeltaSeconds();
        Member.Member->SetActorLocation(NewPos);
    }
}

void UCombat_PackHuntingAI::AssignFlankingPositions()
{
    if (!CurrentTarget) return;
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    
    for (FCombat_PackMember& Member : PackMembers)
    {
        if (!Member.Member) continue;
        
        switch (Member.Role)
        {
            case ECombat_PackRole::Flanker:
                Member.AssignedPosition = TargetLocation + FVector(1000, 1000, 0);
                break;
            case ECombat_PackRole::Scout:
                Member.AssignedPosition = TargetLocation + FVector(-1500, 0, 0);
                break;
            default:
                Member.AssignedPosition = TargetLocation + FVector(500, -500, 0);
                break;
        }
    }
}

void UCombat_PackHuntingAI::CoordinateAttack()
{
    if (!bIsHunting || !CurrentTarget) return;
    
    // Simultaneous attack coordination
    SendPackCommand("ATTACK_NOW");
    
    for (FCombat_PackMember& Member : PackMembers)
    {
        if (Member.Member && Member.bIsActive)
        {
            // Execute attack behavior
            UE_LOG(LogTemp, Log, TEXT("Pack member %s executing coordinated attack"), *Member.Member->GetName());
        }
    }
}

AActor* UCombat_PackHuntingAI::FindNearestThreat()
{
    if (!GetOwner()) return nullptr;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    AActor* NearestThreat = nullptr;
    float NearestDistance = HuntingRange;
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner()) continue;
        
        // Look for player character
        if (Actor->GetName().Contains("Character"))
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestThreat = Actor;
            }
        }
    }
    
    return NearestThreat;
}

void UCombat_PackHuntingAI::UpdatePackFormation()
{
    // Remove inactive members
    PackMembers.RemoveAll([](const FCombat_PackMember& Member) {
        return !Member.Member || !IsValid(Member.Member);
    });
}

void UCombat_PackHuntingAI::SendPackCommand(const FString& Command)
{
    UE_LOG(LogTemp, Log, TEXT("Pack Command: %s"), *Command);
    
    // In a full implementation, this would send signals to other pack members
    // For now, it's logged for debugging
}