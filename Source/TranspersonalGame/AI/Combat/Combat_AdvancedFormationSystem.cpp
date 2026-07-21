#include "Combat_AdvancedFormationSystem.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"

UCombat_AdvancedFormationSystem::UCombat_AdvancedFormationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    FormationUpdateInterval = 0.5f;
    MaxFormationDistance = 1000.0f;
    bFormationActive = false;
    FormationLeader = nullptr;

    // Initialize default formation
    CurrentFormation.FormationRadius = 300.0f;
    CurrentFormation.FormationType = ECombat_FormationType::PackHunt;
}

void UCombat_AdvancedFormationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Create default pack hunting formation
    CreateFormationSlots(ECombat_FormationType::PackHunt);
}

void UCombat_AdvancedFormationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bFormationActive && FormationLeader)
    {
        UpdateFormationPositions();
        
        // Check formation integrity
        if (!IsFormationIntact())
        {
            HandleFormationBreakdown();
        }
    }
}

void UCombat_AdvancedFormationSystem::InitializeFormation(ECombat_FormationType InFormationType, APawn* Leader)
{
    if (!Leader)
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat Formation: Cannot initialize without leader"));
        return;
    }

    FormationLeader = Leader;
    CurrentFormation.FormationType = InFormationType;
    CurrentFormation.CenterPosition = Leader->GetActorLocation();

    CreateFormationSlots(InFormationType);
    
    // Add leader to formation
    if (!FormationMembers.Contains(Leader))
    {
        FormationMembers.Add(Leader);
    }

    bFormationActive = true;
    
    UE_LOG(LogTemp, Log, TEXT("Combat Formation: Initialized %s formation with leader %s"), 
        *UEnum::GetValueAsString(InFormationType), *Leader->GetName());
}

bool UCombat_AdvancedFormationSystem::AddMemberToFormation(APawn* NewMember)
{
    if (!NewMember || FormationMembers.Contains(NewMember))
    {
        return false;
    }

    // Check if we have available slots
    bool bHasAvailableSlot = false;
    for (const FCombat_FormationSlot& Slot : CurrentFormation.FormationSlots)
    {
        if (!Slot.bIsOccupied)
        {
            bHasAvailableSlot = true;
            break;
        }
    }

    if (!bHasAvailableSlot)
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat Formation: No available slots for new member"));
        return false;
    }

    FormationMembers.Add(NewMember);
    AssignMembersToSlots();

    UE_LOG(LogTemp, Log, TEXT("Combat Formation: Added member %s to formation"), *NewMember->GetName());
    return true;
}

void UCombat_AdvancedFormationSystem::RemoveMemberFromFormation(APawn* Member)
{
    if (FormationMembers.Contains(Member))
    {
        FormationMembers.Remove(Member);
        
        // If removing the leader, assign new leader
        if (Member == FormationLeader && FormationMembers.Num() > 0)
        {
            FormationLeader = FormationMembers[0];
            UE_LOG(LogTemp, Log, TEXT("Combat Formation: New leader assigned: %s"), *FormationLeader->GetName());
        }

        AssignMembersToSlots();
    }
}

void UCombat_AdvancedFormationSystem::UpdateFormationPositions()
{
    if (!FormationLeader)
    {
        return;
    }

    // Update center position based on leader
    CurrentFormation.CenterPosition = FormationLeader->GetActorLocation();

    // Update formation slots world positions
    for (int32 i = 0; i < FormationMembers.Num() && i < CurrentFormation.FormationSlots.Num(); i++)
    {
        if (FormationMembers[i] && FormationMembers[i] != FormationLeader)
        {
            FVector TargetPosition = CalculateSlotWorldPosition(CurrentFormation.FormationSlots[i]);
            
            // Apply movement towards formation position
            FVector CurrentPos = FormationMembers[i]->GetActorLocation();
            FVector Direction = (TargetPosition - CurrentPos).GetSafeNormal();
            
            // This would typically be handled by AI movement component
            // For now, we log the intended movement
            UE_LOG(LogTemp, VeryVerbose, TEXT("Formation Member %s should move to %s"), 
                *FormationMembers[i]->GetName(), *TargetPosition.ToString());
        }
    }
}

FVector UCombat_AdvancedFormationSystem::GetAssignedPosition(APawn* Member)
{
    int32 MemberIndex = FormationMembers.Find(Member);
    if (MemberIndex != INDEX_NONE && MemberIndex < CurrentFormation.FormationSlots.Num())
    {
        return CalculateSlotWorldPosition(CurrentFormation.FormationSlots[MemberIndex]);
    }
    
    return FVector::ZeroVector;
}

void UCombat_AdvancedFormationSystem::SetFormationType(ECombat_FormationType NewType)
{
    if (NewType != CurrentFormation.FormationType)
    {
        CurrentFormation.FormationType = NewType;
        CreateFormationSlots(NewType);
        AssignMembersToSlots();
        
        UE_LOG(LogTemp, Log, TEXT("Combat Formation: Changed to %s formation"), *UEnum::GetValueAsString(NewType));
    }
}

void UCombat_AdvancedFormationSystem::ActivateFormation(bool bActivate)
{
    bFormationActive = bActivate;
    UE_LOG(LogTemp, Log, TEXT("Combat Formation: %s"), bActivate ? TEXT("Activated") : TEXT("Deactivated"));
}

bool UCombat_AdvancedFormationSystem::IsFormationIntact()
{
    if (!FormationLeader || FormationMembers.Num() < 2)
    {
        return false;
    }

    // Check if members are within formation distance
    for (APawn* Member : FormationMembers)
    {
        if (Member && Member != FormationLeader)
        {
            float Distance = FVector::Dist(Member->GetActorLocation(), FormationLeader->GetActorLocation());
            if (Distance > MaxFormationDistance)
            {
                return false;
            }
        }
    }

    return true;
}

void UCombat_AdvancedFormationSystem::ExecuteTacticalManeuver(ECombat_TacticalManeuver Maneuver)
{
    switch (Maneuver)
    {
        case ECombat_TacticalManeuver::FlankLeft:
            SetFormationType(ECombat_FormationType::FlankingLeft);
            break;
        case ECombat_TacticalManeuver::FlankRight:
            SetFormationType(ECombat_FormationType::FlankingRight);
            break;
        case ECombat_TacticalManeuver::Surround:
            SetFormationType(ECombat_FormationType::Encirclement);
            break;
        case ECombat_TacticalManeuver::Retreat:
            SetFormationType(ECombat_FormationType::Retreat);
            break;
        default:
            SetFormationType(ECombat_FormationType::PackHunt);
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("Combat Formation: Executing tactical maneuver %s"), *UEnum::GetValueAsString(Maneuver));
}

void UCombat_AdvancedFormationSystem::CreateFormationSlots(ECombat_FormationType FormationType)
{
    CurrentFormation.FormationSlots.Empty();

    switch (FormationType)
    {
        case ECombat_FormationType::PackHunt:
        {
            // Leader at center, flankers at sides
            CurrentFormation.FormationSlots.Add(FCombat_FormationSlot()); // Leader slot
            
            FCombat_FormationSlot LeftFlank;
            LeftFlank.RelativePosition = FVector(-100, -200, 0);
            LeftFlank.Role = ECombat_FormationRole::Flanker;
            LeftFlank.Priority = 2.0f;
            CurrentFormation.FormationSlots.Add(LeftFlank);
            
            FCombat_FormationSlot RightFlank;
            RightFlank.RelativePosition = FVector(-100, 200, 0);
            RightFlank.Role = ECombat_FormationRole::Flanker;
            RightFlank.Priority = 2.0f;
            CurrentFormation.FormationSlots.Add(RightFlank);
            break;
        }
        case ECombat_FormationType::Encirclement:
        {
            // Create circular formation
            int32 NumSlots = 6;
            float AngleStep = 360.0f / NumSlots;
            
            for (int32 i = 0; i < NumSlots; i++)
            {
                FCombat_FormationSlot Slot;
                float Angle = AngleStep * i;
                Slot.RelativePosition = FVector(
                    FMath::Cos(FMath::DegreesToRadians(Angle)) * CurrentFormation.FormationRadius,
                    FMath::Sin(FMath::DegreesToRadians(Angle)) * CurrentFormation.FormationRadius,
                    0
                );
                Slot.Role = ECombat_FormationRole::Encircler;
                Slot.Priority = 1.0f;
                CurrentFormation.FormationSlots.Add(Slot);
            }
            break;
        }
        case ECombat_FormationType::FlankingLeft:
        case ECombat_FormationType::FlankingRight:
        {
            float FlankDirection = (FormationType == ECombat_FormationType::FlankingLeft) ? -1.0f : 1.0f;
            
            FCombat_FormationSlot Leader;
            CurrentFormation.FormationSlots.Add(Leader);
            
            FCombat_FormationSlot Flanker1;
            Flanker1.RelativePosition = FVector(0, 300 * FlankDirection, 0);
            Flanker1.Role = ECombat_FormationRole::Flanker;
            CurrentFormation.FormationSlots.Add(Flanker1);
            
            FCombat_FormationSlot Flanker2;
            Flanker2.RelativePosition = FVector(-150, 200 * FlankDirection, 0);
            Flanker2.Role = ECombat_FormationRole::Flanker;
            CurrentFormation.FormationSlots.Add(Flanker2);
            break;
        }
        default:
            // Default to pack hunt formation
            CreateFormationSlots(ECombat_FormationType::PackHunt);
            break;
    }
}

void UCombat_AdvancedFormationSystem::AssignMembersToSlots()
{
    // Reset slot occupancy
    for (FCombat_FormationSlot& Slot : CurrentFormation.FormationSlots)
    {
        Slot.bIsOccupied = false;
    }

    // Assign leader to first slot
    if (FormationLeader && CurrentFormation.FormationSlots.Num() > 0)
    {
        CurrentFormation.FormationSlots[0].bIsOccupied = true;
    }

    // Assign other members to remaining slots
    int32 SlotIndex = 1;
    for (APawn* Member : FormationMembers)
    {
        if (Member && Member != FormationLeader && SlotIndex < CurrentFormation.FormationSlots.Num())
        {
            CurrentFormation.FormationSlots[SlotIndex].bIsOccupied = true;
            SlotIndex++;
        }
    }
}

FVector UCombat_AdvancedFormationSystem::CalculateSlotWorldPosition(const FCombat_FormationSlot& Slot)
{
    if (!FormationLeader)
    {
        return FVector::ZeroVector;
    }

    // Transform relative position to world space based on leader's rotation
    FRotator LeaderRotation = FormationLeader->GetActorRotation();
    FVector WorldOffset = LeaderRotation.RotateVector(Slot.RelativePosition);
    
    return CurrentFormation.CenterPosition + WorldOffset;
}

void UCombat_AdvancedFormationSystem::HandleFormationBreakdown()
{
    UE_LOG(LogTemp, Warning, TEXT("Combat Formation: Formation integrity compromised - attempting to reform"));
    
    // Remove members that are too far away
    TArray<APawn*> MembersToRemove;
    for (APawn* Member : FormationMembers)
    {
        if (Member && Member != FormationLeader)
        {
            float Distance = FVector::Dist(Member->GetActorLocation(), FormationLeader->GetActorLocation());
            if (Distance > MaxFormationDistance * 1.5f)
            {
                MembersToRemove.Add(Member);
            }
        }
    }

    for (APawn* Member : MembersToRemove)
    {
        RemoveMemberFromFormation(Member);
    }

    // If too few members remain, deactivate formation
    if (FormationMembers.Num() < 2)
    {
        ActivateFormation(false);
        UE_LOG(LogTemp, Log, TEXT("Combat Formation: Insufficient members - formation deactivated"));
    }
}