#include "Combat_TacticalFormationSystem.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/PrimitiveComponent.h"

UCombat_TacticalFormationSystem::UCombat_TacticalFormationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    bFormationActive = false;
    FormationTolerance = 150.0f;
    ReformationDelay = 2.0f;
}

void UCombat_TacticalFormationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default circle formation
    InitializeFormation(TEXT("Default Circle"), ECombat_FormationType::Circle, 6);
}

void UCombat_TacticalFormationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bFormationActive && UnitsInFormation.Num() > 0)
    {
        UpdateFormationPositions();
        
        // Check if formation needs reformation
        float cohesion = GetFormationCohesion();
        if (cohesion < 0.5f && !GetWorld()->GetTimerManager().IsTimerActive(ReformationTimer))
        {
            GetWorld()->GetTimerManager().SetTimer(ReformationTimer, this, &UCombat_TacticalFormationSystem::ReformFormation, ReformationDelay, false);
        }
    }
}

void UCombat_TacticalFormationSystem::InitializeFormation(const FString& FormationName, ECombat_FormationType Type, int32 MaxUnits)
{
    CurrentFormation.FormationName = FormationName;
    CurrentFormation.FormationType = Type;
    CurrentFormation.Slots.Empty();
    
    switch (Type)
    {
        case ECombat_FormationType::Circle:
            CreateCircleFormation(MaxUnits);
            break;
        case ECombat_FormationType::Line:
            CreateLineFormation(MaxUnits);
            break;
        case ECombat_FormationType::Wedge:
            CreateWedgeFormation(MaxUnits);
            break;
        case ECombat_FormationType::Ambush:
            CreateAmbushFormation(MaxUnits);
            break;
        default:
            CreateCircleFormation(MaxUnits);
            break;
    }
    
    bFormationActive = true;
}

bool UCombat_TacticalFormationSystem::AddUnitToFormation(APawn* Unit)
{
    if (!Unit || UnitsInFormation.Contains(Unit))
    {
        return false;
    }
    
    // Find available slot
    int32 BestSlot = FindBestSlotForUnit(Unit);
    if (BestSlot != INDEX_NONE && BestSlot < CurrentFormation.Slots.Num())
    {
        UnitsInFormation.Add(Unit);
        AssignUnitToSlot(Unit, BestSlot);
        return true;
    }
    
    return false;
}

void UCombat_TacticalFormationSystem::RemoveUnitFromFormation(APawn* Unit)
{
    if (!Unit)
    {
        return;
    }
    
    // Find and clear slot
    for (int32 i = 0; i < CurrentFormation.Slots.Num(); i++)
    {
        if (CurrentFormation.Slots[i].OccupyingUnit == Unit)
        {
            CurrentFormation.Slots[i].bIsOccupied = false;
            CurrentFormation.Slots[i].OccupyingUnit = nullptr;
            break;
        }
    }
    
    UnitsInFormation.Remove(Unit);
    
    // If formation becomes too small, break it
    if (UnitsInFormation.Num() < 2)
    {
        BreakFormation();
    }
}

void UCombat_TacticalFormationSystem::UpdateFormationCenter(const FVector& NewCenter)
{
    CurrentFormation.FormationCenter = NewCenter;
    UpdateFormationPositions();
}

FVector UCombat_TacticalFormationSystem::GetFormationPositionForUnit(APawn* Unit)
{
    if (!Unit)
    {
        return FVector::ZeroVector;
    }
    
    // Find unit's assigned slot
    for (const FCombat_FormationSlot& Slot : CurrentFormation.Slots)
    {
        if (Slot.OccupyingUnit == Unit)
        {
            return CurrentFormation.FormationCenter + Slot.RelativePosition;
        }
    }
    
    return CurrentFormation.FormationCenter;
}

void UCombat_TacticalFormationSystem::BreakFormation()
{
    bFormationActive = false;
    
    // Clear all slots
    for (FCombat_FormationSlot& Slot : CurrentFormation.Slots)
    {
        Slot.bIsOccupied = false;
        Slot.OccupyingUnit = nullptr;
    }
    
    GetWorld()->GetTimerManager().ClearTimer(ReformationTimer);
}

void UCombat_TacticalFormationSystem::ReformFormation()
{
    if (!bFormationActive)
    {
        return;
    }
    
    // Reassign all units to optimal positions
    TArray<TWeakObjectPtr<APawn>> ValidUnits;
    for (auto& UnitPtr : UnitsInFormation)
    {
        if (UnitPtr.IsValid())
        {
            ValidUnits.Add(UnitPtr);
        }
    }
    
    UnitsInFormation = ValidUnits;
    
    // Clear all slots
    for (FCombat_FormationSlot& Slot : CurrentFormation.Slots)
    {
        Slot.bIsOccupied = false;
        Slot.OccupyingUnit = nullptr;
    }
    
    // Reassign units
    for (auto& UnitPtr : UnitsInFormation)
    {
        if (UnitPtr.IsValid())
        {
            int32 BestSlot = FindBestSlotForUnit(UnitPtr.Get());
            if (BestSlot != INDEX_NONE)
            {
                AssignUnitToSlot(UnitPtr.Get(), BestSlot);
            }
        }
    }
}

float UCombat_TacticalFormationSystem::GetFormationCohesion() const
{
    if (UnitsInFormation.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalCohesion = 0.0f;
    int32 ValidUnits = 0;
    
    for (auto& UnitPtr : UnitsInFormation)
    {
        if (UnitPtr.IsValid())
        {
            FVector TargetPosition = GetFormationPositionForUnit(UnitPtr.Get());
            FVector UnitPosition = UnitPtr->GetActorLocation();
            float Distance = FVector::Dist(UnitPosition, TargetPosition);
            
            // Cohesion decreases with distance from formation position
            float UnitCohesion = FMath::Clamp(1.0f - (Distance / (CurrentFormation.FormationRadius * 2.0f)), 0.0f, 1.0f);
            TotalCohesion += UnitCohesion;
            ValidUnits++;
        }
    }
    
    return ValidUnits > 0 ? TotalCohesion / ValidUnits : 0.0f;
}

void UCombat_TacticalFormationSystem::ExecuteFlankingManeuver(const FVector& TargetPosition)
{
    if (!bFormationActive || UnitsInFormation.Num() < 3)
    {
        return;
    }
    
    // Split units into left and right flanking groups
    FVector FormationToTarget = (TargetPosition - CurrentFormation.FormationCenter).GetSafeNormal();
    FVector RightFlank = FVector::CrossProduct(FormationToTarget, FVector::UpVector).GetSafeNormal();
    FVector LeftFlank = -RightFlank;
    
    // Reposition formation center to flank target
    FVector NewCenter = TargetPosition + (RightFlank * CurrentFormation.FormationRadius * 1.5f);
    UpdateFormationCenter(NewCenter);
    
    // Switch to line formation for flanking
    InitializeFormation(TEXT("Flanking Line"), ECombat_FormationType::Line, UnitsInFormation.Num());
}

void UCombat_TacticalFormationSystem::ExecutePincerAttack(const FVector& TargetPosition)
{
    if (!bFormationActive || UnitsInFormation.Num() < 4)
    {
        return;
    }
    
    // Create pincer formation around target
    InitializeFormation(TEXT("Pincer"), ECombat_FormationType::Ambush, UnitsInFormation.Num());
    UpdateFormationCenter(TargetPosition);
}

void UCombat_TacticalFormationSystem::ExecuteRetreat(const FVector& RetreatDirection)
{
    if (!bFormationActive)
    {
        return;
    }
    
    // Move formation center in retreat direction
    FVector NewCenter = CurrentFormation.FormationCenter + (RetreatDirection.GetSafeNormal() * CurrentFormation.FormationRadius * 2.0f);
    UpdateFormationCenter(NewCenter);
    
    // Switch to line formation for organized retreat
    InitializeFormation(TEXT("Retreat Line"), ECombat_FormationType::Line, UnitsInFormation.Num());
}

void UCombat_TacticalFormationSystem::CreateCircleFormation(int32 NumSlots)
{
    CurrentFormation.Slots.Empty();
    
    for (int32 i = 0; i < NumSlots; i++)
    {
        FCombat_FormationSlot NewSlot;
        float Angle = (2.0f * PI * i) / NumSlots;
        NewSlot.RelativePosition = FVector(
            FMath::Cos(Angle) * CurrentFormation.FormationRadius,
            FMath::Sin(Angle) * CurrentFormation.FormationRadius,
            0.0f
        );
        NewSlot.Priority = 1.0f;
        CurrentFormation.Slots.Add(NewSlot);
    }
}

void UCombat_TacticalFormationSystem::CreateLineFormation(int32 NumSlots)
{
    CurrentFormation.Slots.Empty();
    
    float SlotSpacing = CurrentFormation.FormationRadius / NumSlots;
    float StartOffset = -(NumSlots - 1) * SlotSpacing * 0.5f;
    
    for (int32 i = 0; i < NumSlots; i++)
    {
        FCombat_FormationSlot NewSlot;
        NewSlot.RelativePosition = FVector(StartOffset + (i * SlotSpacing), 0.0f, 0.0f);
        NewSlot.Priority = 1.0f;
        CurrentFormation.Slots.Add(NewSlot);
    }
}

void UCombat_TacticalFormationSystem::CreateWedgeFormation(int32 NumSlots)
{
    CurrentFormation.Slots.Empty();
    
    // Create V-shaped wedge
    int32 SlotsPerSide = NumSlots / 2;
    float AngleStep = 30.0f; // degrees
    
    for (int32 i = 0; i < NumSlots; i++)
    {
        FCombat_FormationSlot NewSlot;
        
        if (i == 0)
        {
            // Point of the wedge
            NewSlot.RelativePosition = FVector(CurrentFormation.FormationRadius, 0.0f, 0.0f);
            NewSlot.Priority = 2.0f; // Higher priority for lead position
        }
        else
        {
            int32 Side = (i - 1) % 2; // 0 for left, 1 for right
            int32 Rank = (i - 1) / 2 + 1;
            float Angle = FMath::DegreesToRadians(AngleStep * Rank);
            float Distance = CurrentFormation.FormationRadius * 0.7f * Rank;
            
            if (Side == 0) // Left side
            {
                NewSlot.RelativePosition = FVector(
                    Distance * FMath::Cos(Angle),
                    -Distance * FMath::Sin(Angle),
                    0.0f
                );
            }
            else // Right side
            {
                NewSlot.RelativePosition = FVector(
                    Distance * FMath::Cos(Angle),
                    Distance * FMath::Sin(Angle),
                    0.0f
                );
            }
            NewSlot.Priority = 1.0f;
        }
        
        CurrentFormation.Slots.Add(NewSlot);
    }
}

void UCombat_TacticalFormationSystem::CreateAmbushFormation(int32 NumSlots)
{
    CurrentFormation.Slots.Empty();
    
    // Create scattered positions around target for ambush
    for (int32 i = 0; i < NumSlots; i++)
    {
        FCombat_FormationSlot NewSlot;
        
        // Random angle with some clustering
        float BaseAngle = (2.0f * PI * i) / NumSlots;
        float AngleVariation = FMath::RandRange(-0.3f, 0.3f);
        float Angle = BaseAngle + AngleVariation;
        
        // Variable distance for natural ambush positioning
        float Distance = FMath::RandRange(CurrentFormation.FormationRadius * 0.8f, CurrentFormation.FormationRadius * 1.2f);
        
        NewSlot.RelativePosition = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        NewSlot.Priority = 1.0f;
        CurrentFormation.Slots.Add(NewSlot);
    }
}

void UCombat_TacticalFormationSystem::UpdateFormationPositions()
{
    // This method is called each tick to update formation positions
    // Individual AI controllers should query GetFormationPositionForUnit() for their target positions
}

bool UCombat_TacticalFormationSystem::IsUnitInPosition(APawn* Unit, const FVector& TargetPosition) const
{
    if (!Unit)
    {
        return false;
    }
    
    float Distance = FVector::Dist(Unit->GetActorLocation(), TargetPosition);
    return Distance <= FormationTolerance;
}

void UCombat_TacticalFormationSystem::AssignUnitToSlot(APawn* Unit, int32 SlotIndex)
{
    if (SlotIndex >= 0 && SlotIndex < CurrentFormation.Slots.Num())
    {
        CurrentFormation.Slots[SlotIndex].bIsOccupied = true;
        CurrentFormation.Slots[SlotIndex].OccupyingUnit = Unit;
    }
}

int32 UCombat_TacticalFormationSystem::FindBestSlotForUnit(APawn* Unit) const
{
    if (!Unit)
    {
        return INDEX_NONE;
    }
    
    FVector UnitLocation = Unit->GetActorLocation();
    int32 BestSlot = INDEX_NONE;
    float BestDistance = FLT_MAX;
    
    for (int32 i = 0; i < CurrentFormation.Slots.Num(); i++)
    {
        if (!CurrentFormation.Slots[i].bIsOccupied)
        {
            FVector SlotWorldPosition = CurrentFormation.FormationCenter + CurrentFormation.Slots[i].RelativePosition;
            float Distance = FVector::Dist(UnitLocation, SlotWorldPosition);
            
            // Factor in priority
            float Score = Distance / CurrentFormation.Slots[i].Priority;
            
            if (Score < BestDistance)
            {
                BestDistance = Score;
                BestSlot = i;
            }
        }
    }
    
    return BestSlot;
}