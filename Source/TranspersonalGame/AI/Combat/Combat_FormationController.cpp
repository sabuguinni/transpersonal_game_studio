#include "Combat_FormationController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "AIModule/Classes/Blueprint/AIBlueprintHelperLibrary.h"

ACombat_FormationController::ACombat_FormationController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize formation parameters
    CurrentFormationType = ECombat_FormationType::None;
    FormationRadius = 800.0f;
    FormationUpdateInterval = 0.5f;
    FormationTarget = nullptr;
    AlphaLeader = nullptr;
    bIsAlphaLeader = false;
    CombatRange = 500.0f;
    FlankingDistance = 1200.0f;
    FormationCohesion = 0.8f;
    LastTargetPosition = FVector::ZeroVector;
    
    // Initialize formation positions for pack formation
    FormationPositions.Empty();
    
    // Alpha leader position (center)
    FCombat_FormationPosition AlphaPos;
    AlphaPos.RelativePosition = FVector(0, 0, 0);
    AlphaPos.RelativeRotation = FRotator(0, 0, 0);
    AlphaPos.Priority = 10.0f;
    AlphaPos.bIsLeaderPosition = true;
    FormationPositions.Add(AlphaPos);
    
    // Pack flanker positions
    FCombat_FormationPosition LeftFlank;
    LeftFlank.RelativePosition = FVector(-200, -400, 0);
    LeftFlank.Priority = 8.0f;
    FormationPositions.Add(LeftFlank);
    
    FCombat_FormationPosition RightFlank;
    RightFlank.RelativePosition = FVector(-200, 400, 0);
    RightFlank.Priority = 8.0f;
    FormationPositions.Add(RightFlank);
    
    FCombat_FormationPosition RearGuard;
    RearGuard.RelativePosition = FVector(-400, 0, 0);
    RearGuard.Priority = 6.0f;
    FormationPositions.Add(RearGuard);
}

void ACombat_FormationController::BeginPlay()
{
    Super::BeginPlay();
    
    // Start formation update timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            FormationUpdateTimer,
            this,
            &ACombat_FormationController::UpdateFormation,
            FormationUpdateInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Combat Formation Controller initialized for %s"), 
           GetPawn() ? *GetPawn()->GetName() : TEXT("No Pawn"));
}

void ACombat_FormationController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Handle real-time combat coordination
    if (FormationTarget && CurrentFormationType != ECombat_FormationType::None)
    {
        HandleCombatEngagement();
    }
}

void ACombat_FormationController::SetFormationType(ECombat_FormationType NewFormationType)
{
    CurrentFormationType = NewFormationType;
    CalculateFormationPositions();
    
    UE_LOG(LogTemp, Warning, TEXT("Formation type changed to: %d"), (int32)NewFormationType);
}

void ACombat_FormationController::SetFormationTarget(APawn* NewTarget)
{
    FormationTarget = NewTarget;
    if (FormationTarget)
    {
        LastTargetPosition = FormationTarget->GetActorLocation();
        UE_LOG(LogTemp, Warning, TEXT("Formation target set: %s"), *FormationTarget->GetName());
    }
}

void ACombat_FormationController::AddPackMember(ACombat_FormationController* NewMember)
{
    if (NewMember && !PackMembers.Contains(NewMember))
    {
        PackMembers.Add(NewMember);
        NewMember->AlphaLeader = this;
        UE_LOG(LogTemp, Warning, TEXT("Pack member added: %s"), 
               NewMember->GetPawn() ? *NewMember->GetPawn()->GetName() : TEXT("Unknown"));
    }
}

void ACombat_FormationController::RemovePackMember(ACombat_FormationController* MemberToRemove)
{
    if (MemberToRemove)
    {
        PackMembers.Remove(MemberToRemove);
        MemberToRemove->AlphaLeader = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("Pack member removed"));
    }
}

void ACombat_FormationController::SetAsAlphaLeader(bool bNewAlphaStatus)
{
    bIsAlphaLeader = bNewAlphaStatus;
    if (bIsAlphaLeader)
    {
        CurrentFormationType = ECombat_FormationType::AlphaLead;
        UE_LOG(LogTemp, Warning, TEXT("Set as Alpha Leader"));
    }
}

FVector ACombat_FormationController::GetFormationPosition() const
{
    if (!FormationTarget || FormationPositions.Num() == 0)
    {
        return GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
    }
    
    // Calculate position based on formation type and pack index
    int32 PackIndex = 0;
    if (AlphaLeader && AlphaLeader->PackMembers.Contains(const_cast<ACombat_FormationController*>(this)))
    {
        PackIndex = AlphaLeader->PackMembers.IndexOfByKey(const_cast<ACombat_FormationController*>(this));
    }
    
    if (bIsAlphaLeader)
    {
        PackIndex = 0; // Alpha gets first position
    }
    else
    {
        PackIndex = FMath::Clamp(PackIndex + 1, 1, FormationPositions.Num() - 1);
    }
    
    if (PackIndex < FormationPositions.Num())
    {
        FVector TargetLocation = FormationTarget->GetActorLocation();
        FVector FormationOffset = FormationPositions[PackIndex].RelativePosition;
        
        // Rotate offset based on target direction
        FVector TargetDirection = (TargetLocation - TargetLocation).GetSafeNormal();
        FRotator TargetRotation = TargetDirection.Rotation();
        FormationOffset = TargetRotation.RotateVector(FormationOffset);
        
        return TargetLocation + FormationOffset;
    }
    
    return FormationTarget->GetActorLocation();
}

bool ACombat_FormationController::IsInFormation() const
{
    if (!GetPawn() || CurrentFormationType == ECombat_FormationType::None)
    {
        return false;
    }
    
    FVector DesiredPosition = GetFormationPosition();
    FVector CurrentPosition = GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(CurrentPosition, DesiredPosition);
    
    return Distance < (FormationRadius * 0.5f);
}

void ACombat_FormationController::ExecuteFormationCommand(const FString& Command)
{
    if (Command == "ENGAGE")
    {
        SetFormationType(ECombat_FormationType::AlphaLead);
        CoordinatePackAttack();
    }
    else if (Command == "FLANK")
    {
        SetFormationType(ECombat_FormationType::PackFlank);
    }
    else if (Command == "CIRCLE")
    {
        SetFormationType(ECombat_FormationType::CircleHunt);
    }
    else if (Command == "AMBUSH")
    {
        SetFormationType(ECombat_FormationType::AmbushTrap);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Formation command executed: %s"), *Command);
}

void ACombat_FormationController::UpdateFormation()
{
    if (!GetPawn() || CurrentFormationType == ECombat_FormationType::None)
    {
        return;
    }
    
    // Validate formation integrity
    if (!ValidateFormationIntegrity())
    {
        return;
    }
    
    // Move to formation position
    MoveToFormationPosition();
    
    // Update pack coordination if alpha leader
    if (bIsAlphaLeader && PackMembers.Num() > 0)
    {
        for (ACombat_FormationController* Member : PackMembers)
        {
            if (Member && Member->GetPawn())
            {
                Member->UpdateFormation();
            }
        }
    }
}

void ACombat_FormationController::CalculateFormationPositions()
{
    // Formation positions are pre-calculated in constructor
    // This method can be expanded for dynamic formation calculation
    UE_LOG(LogTemp, Log, TEXT("Formation positions calculated"));
}

void ACombat_FormationController::MoveToFormationPosition()
{
    if (!GetPawn())
    {
        return;
    }
    
    FVector DesiredPosition = GetFormationPosition();
    FVector CurrentPosition = GetPawn()->GetActorLocation();
    
    float Distance = FVector::Dist(CurrentPosition, DesiredPosition);
    if (Distance > FormationRadius * 0.3f)
    {
        // Use AI movement to reach formation position
        UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, DesiredPosition);
    }
}

bool ACombat_FormationController::ValidateFormationIntegrity()
{
    // Check if formation target is still valid
    if (!FormationTarget || !IsValid(FormationTarget))
    {
        FormationTarget = nullptr;
        return false;
    }
    
    // Check if alpha leader is still valid (for pack members)
    if (!bIsAlphaLeader && (!AlphaLeader || !IsValid(AlphaLeader)))
    {
        AlphaLeader = nullptr;
        CurrentFormationType = ECombat_FormationType::None;
        return false;
    }
    
    return true;
}

void ACombat_FormationController::HandleCombatEngagement()
{
    if (!FormationTarget || !GetPawn())
    {
        return;
    }
    
    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), FormationTarget->GetActorLocation());
    
    // Engage if within combat range
    if (DistanceToTarget <= CombatRange)
    {
        if (bIsAlphaLeader)
        {
            ExecuteFormationCommand("ENGAGE");
        }
        
        // Direct attack logic would go here
        UE_LOG(LogTemp, Warning, TEXT("Combat engagement at range: %f"), DistanceToTarget);
    }
    else if (DistanceToTarget <= FlankingDistance)
    {
        // Maintain flanking position
        if (CurrentFormationType != ECombat_FormationType::PackFlank)
        {
            ExecuteFormationCommand("FLANK");
        }
    }
}

void ACombat_FormationController::CoordinatePackAttack()
{
    if (!bIsAlphaLeader || PackMembers.Num() == 0)
    {
        return;
    }
    
    // Coordinate simultaneous attack
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        ACombat_FormationController* Member = PackMembers[i];
        if (Member && Member->GetPawn())
        {
            // Stagger attack timing for realistic coordination
            FTimerHandle AttackTimer;
            GetWorld()->GetTimerManager().SetTimer(
                AttackTimer,
                [Member]()
                {
                    Member->ExecuteFormationCommand("ENGAGE");
                },
                i * 0.3f, // 0.3 second stagger between attacks
                false
            );
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Pack attack coordinated with %d members"), PackMembers.Num());
}