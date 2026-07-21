#include "Combat_TacticalFormation.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"

UCombat_TacticalFormation::UCombat_TacticalFormation()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    FormationType = ECombat_FormationType::Line;
    FormationCenter = FVector::ZeroVector;
    FormationRotation = 0.0f;
    FormationScale = 1.0f;
    MaxFormationDistance = 500.0f;
    FormationTightness = 1.0f;
}

void UCombat_TacticalFormation::BeginPlay()
{
    Super::BeginPlay();
    GenerateFormationSlots();
}

void UCombat_TacticalFormation::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update formation positions and maintain cohesion
    AssignUnitsToSlots();

    // Check formation integrity
    float CurrentCohesion = GetFormationCohesion();
    if (CurrentCohesion < 0.5f)
    {
        // Formation is breaking - trigger reformation
        for (auto& Slot : FormationSlots)
        {
            if (Slot.bIsOccupied && Slot.AssignedUnit.IsValid())
            {
                FVector TargetPosition = CalculateSlotWorldPosition(Slot);
                // Signal unit to move to formation position
                // This would typically interface with the AI behavior tree
            }
        }
    }
}

void UCombat_TacticalFormation::SetFormationType(ECombat_FormationType NewFormationType)
{
    if (FormationType != NewFormationType)
    {
        FormationType = NewFormationType;
        GenerateFormationSlots();
        AssignUnitsToSlots();
    }
}

void UCombat_TacticalFormation::AddUnitToFormation(APawn* Unit)
{
    if (!Unit)
    {
        return;
    }

    // Find an unoccupied slot
    for (auto& Slot : FormationSlots)
    {
        if (!Slot.bIsOccupied)
        {
            Slot.bIsOccupied = true;
            Slot.AssignedUnit = Unit;
            
            // Move unit to formation position
            FVector TargetPosition = CalculateSlotWorldPosition(Slot);
            // Interface with AI movement system here
            
            break;
        }
    }
}

void UCombat_TacticalFormation::RemoveUnitFromFormation(APawn* Unit)
{
    if (!Unit)
    {
        return;
    }

    for (auto& Slot : FormationSlots)
    {
        if (Slot.AssignedUnit == Unit)
        {
            Slot.bIsOccupied = false;
            Slot.AssignedUnit = nullptr;
            break;
        }
    }
}

FVector UCombat_TacticalFormation::GetFormationPosition(APawn* Unit) const
{
    if (!Unit)
    {
        return FVector::ZeroVector;
    }

    for (const auto& Slot : FormationSlots)
    {
        if (Slot.AssignedUnit == Unit)
        {
            return CalculateSlotWorldPosition(Slot);
        }
    }

    return FVector::ZeroVector;
}

void UCombat_TacticalFormation::UpdateFormationCenter(const FVector& NewCenter)
{
    FormationCenter = NewCenter;
}

void UCombat_TacticalFormation::RotateFormation(float RotationAngle)
{
    FormationRotation += RotationAngle;
    
    // Normalize rotation
    while (FormationRotation > 360.0f)
    {
        FormationRotation -= 360.0f;
    }
    while (FormationRotation < 0.0f)
    {
        FormationRotation += 360.0f;
    }
}

bool UCombat_TacticalFormation::IsFormationComplete() const
{
    int32 OccupiedSlots = 0;
    for (const auto& Slot : FormationSlots)
    {
        if (Slot.bIsOccupied && Slot.AssignedUnit.IsValid())
        {
            OccupiedSlots++;
        }
    }
    
    return OccupiedSlots >= FMath::Max(1, FormationSlots.Num() * 0.8f);
}

int32 UCombat_TacticalFormation::GetFormationSize() const
{
    return FormationSlots.Num();
}

float UCombat_TacticalFormation::GetFormationCohesion() const
{
    if (FormationSlots.Num() == 0)
    {
        return 0.0f;
    }

    float TotalCohesion = 0.0f;
    int32 ValidUnits = 0;

    for (const auto& Slot : FormationSlots)
    {
        if (Slot.bIsOccupied && Slot.AssignedUnit.IsValid())
        {
            FVector TargetPosition = CalculateSlotWorldPosition(Slot);
            FVector UnitPosition = Slot.AssignedUnit->GetActorLocation();
            float Distance = FVector::Dist(UnitPosition, TargetPosition);
            
            // Cohesion decreases with distance from formation position
            float UnitCohesion = FMath::Clamp(1.0f - (Distance / MaxFormationDistance), 0.0f, 1.0f);
            TotalCohesion += UnitCohesion;
            ValidUnits++;
        }
    }

    return ValidUnits > 0 ? TotalCohesion / ValidUnits : 0.0f;
}

void UCombat_TacticalFormation::GenerateFormationSlots()
{
    FormationSlots.Empty();

    switch (FormationType)
    {
        case ECombat_FormationType::Line:
            GenerateLineFormation();
            break;
        case ECombat_FormationType::Wedge:
            GenerateWedgeFormation();
            break;
        case ECombat_FormationType::Circle:
            GenerateCircleFormation();
            break;
        case ECombat_FormationType::Ambush:
            GenerateAmbushFormation();
            break;
        case ECombat_FormationType::Scatter:
            GenerateScatterFormation();
            break;
    }
}

void UCombat_TacticalFormation::GenerateLineFormation()
{
    // Create a line of 5 positions
    for (int32 i = 0; i < 5; i++)
    {
        FCombat_FormationSlot Slot;
        Slot.RelativePosition = FVector(0, (i - 2) * 200.0f * FormationScale, 0);
        Slot.Priority = 1.0f;
        FormationSlots.Add(Slot);
    }
}

void UCombat_TacticalFormation::GenerateWedgeFormation()
{
    // Create a wedge formation with leader at front
    FCombat_FormationSlot LeaderSlot;
    LeaderSlot.RelativePosition = FVector(200.0f * FormationScale, 0, 0);
    LeaderSlot.Priority = 2.0f;
    FormationSlots.Add(LeaderSlot);

    // Add flanking positions
    for (int32 i = 1; i <= 2; i++)
    {
        FCombat_FormationSlot LeftSlot;
        LeftSlot.RelativePosition = FVector(0, -i * 150.0f * FormationScale, 0);
        LeftSlot.Priority = 1.0f;
        FormationSlots.Add(LeftSlot);

        FCombat_FormationSlot RightSlot;
        RightSlot.RelativePosition = FVector(0, i * 150.0f * FormationScale, 0);
        RightSlot.Priority = 1.0f;
        FormationSlots.Add(RightSlot);
    }
}

void UCombat_TacticalFormation::GenerateCircleFormation()
{
    // Create circular formation
    int32 NumPositions = 6;
    float AngleStep = 360.0f / NumPositions;
    float Radius = 250.0f * FormationScale;

    for (int32 i = 0; i < NumPositions; i++)
    {
        float Angle = i * AngleStep;
        FCombat_FormationSlot Slot;
        Slot.RelativePosition = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
            0
        );
        Slot.Priority = 1.0f;
        FormationSlots.Add(Slot);
    }
}

void UCombat_TacticalFormation::GenerateAmbushFormation()
{
    // Create concealed positions for ambush
    TArray<FVector> AmbushPositions = {
        FVector(-300, -400, 0),  // Hidden left
        FVector(-300, 400, 0),   // Hidden right
        FVector(100, -600, 0),   // Flanking left
        FVector(100, 600, 0),    // Flanking right
        FVector(-600, 0, 0)      // Rear support
    };

    for (const FVector& Position : AmbushPositions)
    {
        FCombat_FormationSlot Slot;
        Slot.RelativePosition = Position * FormationScale;
        Slot.Priority = 1.5f;
        FormationSlots.Add(Slot);
    }
}

void UCombat_TacticalFormation::GenerateScatterFormation()
{
    // Create scattered positions to avoid area damage
    TArray<FVector> ScatterPositions = {
        FVector(200, 300, 0),
        FVector(-150, 450, 0),
        FVector(400, -200, 0),
        FVector(-300, -100, 0),
        FVector(100, -500, 0)
    };

    for (const FVector& Position : ScatterPositions)
    {
        FCombat_FormationSlot Slot;
        Slot.RelativePosition = Position * FormationScale;
        Slot.Priority = 1.0f;
        FormationSlots.Add(Slot);
    }
}

FVector UCombat_TacticalFormation::CalculateSlotWorldPosition(const FCombat_FormationSlot& Slot) const
{
    // Apply rotation to relative position
    FVector RotatedPosition = UKismetMathLibrary::RotateAngleAxis(
        Slot.RelativePosition, 
        FormationRotation, 
        FVector::UpVector
    );
    
    return FormationCenter + RotatedPosition;
}

void UCombat_TacticalFormation::AssignUnitsToSlots()
{
    // Sort slots by priority and reassign units as needed
    FormationSlots.Sort([](const FCombat_FormationSlot& A, const FCombat_FormationSlot& B) {
        return A.Priority > B.Priority;
    });

    // Update formation tightness based on threat level
    if (GetWorld())
    {
        // Check for nearby threats and adjust formation accordingly
        // This would interface with the threat detection system
        FormationTightness = FMath::Clamp(FormationTightness, 0.5f, 2.0f);
    }
}

float UCombat_TacticalFormation::CalculateUnitDistance(APawn* Unit, const FVector& TargetPosition) const
{
    if (!Unit)
    {
        return MAX_FLT;
    }

    return FVector::Dist(Unit->GetActorLocation(), TargetPosition);
}