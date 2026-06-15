#include "Crowd_GroupFormationSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "MassEntitySubsystem.h"

UCrowd_GroupFormationSystem::UCrowd_GroupFormationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    CohesionWeight = 1.0f;
    SeparationWeight = 2.0f;
    AlignmentWeight = 1.0f;
    FormationTolerance = 100.0f;
    ReorganizationCooldown = 5.0f;
    NextFormationID = 1;
    LastReorganizationTime = 0.0f;
}

void UCrowd_GroupFormationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Crowd Group Formation System initialized"));
}

void UCrowd_GroupFormationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update all active formations
    for (auto& FormationPair : ActiveFormations)
    {
        MaintainFormationIntegrity(FormationPair.Key, DeltaTime);
    }
}

int32 UCrowd_GroupFormationSystem::CreateFormation(ECrowd_FormationType FormationType, FVector Center, float Radius, int32 MaxMembers)
{
    FCrowd_GroupFormation NewFormation;
    NewFormation.FormationType = FormationType;
    NewFormation.FormationCenter = Center;
    NewFormation.FormationRadius = Radius;
    NewFormation.MaxMembers = MaxMembers;

    // Generate formation slots based on type
    switch (FormationType)
    {
        case ECrowd_FormationType::Circle:
            GenerateCircleFormation(NewFormation);
            break;
        case ECrowd_FormationType::Line:
            GenerateLineFormation(NewFormation);
            break;
        case ECrowd_FormationType::Wedge:
            GenerateWedgeFormation(NewFormation);
            break;
        case ECrowd_FormationType::Column:
            GenerateColumnFormation(NewFormation);
            break;
    }

    int32 FormationID = NextFormationID++;
    ActiveFormations.Add(FormationID, NewFormation);

    UE_LOG(LogTemp, Log, TEXT("Created formation %d with %d slots"), FormationID, NewFormation.FormationSlots.Num());
    return FormationID;
}

bool UCrowd_GroupFormationSystem::AddEntityToFormation(int32 FormationID, FMassEntityHandle Entity)
{
    FCrowd_GroupFormation* Formation = ActiveFormations.Find(FormationID);
    if (!Formation || Formation->GroupMembers.Num() >= Formation->MaxMembers)
    {
        return false;
    }

    // Find best available slot
    FCrowd_FormationSlot* BestSlot = FindBestAvailableSlot(*Formation, FVector::ZeroVector);
    if (!BestSlot)
    {
        return false;
    }

    BestSlot->bIsOccupied = true;
    BestSlot->OccupyingEntity = Entity;
    Formation->GroupMembers.Add(Entity);

    UE_LOG(LogTemp, Log, TEXT("Added entity to formation %d, now has %d members"), FormationID, Formation->GroupMembers.Num());
    return true;
}

bool UCrowd_GroupFormationSystem::RemoveEntityFromFormation(int32 FormationID, FMassEntityHandle Entity)
{
    FCrowd_GroupFormation* Formation = ActiveFormations.Find(FormationID);
    if (!Formation)
    {
        return false;
    }

    // Find and free the slot
    for (FCrowd_FormationSlot& Slot : Formation->FormationSlots)
    {
        if (Slot.OccupyingEntity == Entity)
        {
            Slot.bIsOccupied = false;
            Slot.OccupyingEntity = FMassEntityHandle();
            break;
        }
    }

    Formation->GroupMembers.Remove(Entity);
    return true;
}

void UCrowd_GroupFormationSystem::UpdateFormationCenter(int32 FormationID, FVector NewCenter)
{
    FCrowd_GroupFormation* Formation = ActiveFormations.Find(FormationID);
    if (Formation)
    {
        Formation->FormationCenter = NewCenter;
        UpdateFormationSlotPositions(*Formation);
    }
}

FVector UCrowd_GroupFormationSystem::GetFormationSlotPosition(int32 FormationID, FMassEntityHandle Entity)
{
    FCrowd_GroupFormation* Formation = ActiveFormations.Find(FormationID);
    if (!Formation)
    {
        return FVector::ZeroVector;
    }

    for (const FCrowd_FormationSlot& Slot : Formation->FormationSlots)
    {
        if (Slot.OccupyingEntity == Entity)
        {
            return Formation->FormationCenter + Slot.LocalPosition;
        }
    }

    return Formation->FormationCenter;
}

void UCrowd_GroupFormationSystem::GenerateCircleFormation(FCrowd_GroupFormation& Formation)
{
    Formation.FormationSlots.Empty();
    
    int32 NumSlots = Formation.MaxMembers;
    float AngleStep = 360.0f / NumSlots;

    for (int32 i = 0; i < NumSlots; i++)
    {
        FCrowd_FormationSlot Slot;
        float Angle = FMath::DegreesToRadians(i * AngleStep);
        Slot.LocalPosition = FVector(
            FMath::Cos(Angle) * Formation.FormationRadius,
            FMath::Sin(Angle) * Formation.FormationRadius,
            0.0f
        );
        Slot.Priority = 1.0f;
        Formation.FormationSlots.Add(Slot);
    }
}

void UCrowd_GroupFormationSystem::GenerateLineFormation(FCrowd_GroupFormation& Formation)
{
    Formation.FormationSlots.Empty();
    
    int32 NumSlots = Formation.MaxMembers;
    float Spacing = Formation.FormationRadius * 2.0f / FMath::Max(1, NumSlots - 1);

    for (int32 i = 0; i < NumSlots; i++)
    {
        FCrowd_FormationSlot Slot;
        Slot.LocalPosition = FVector(
            (i - NumSlots / 2) * Spacing,
            0.0f,
            0.0f
        );
        Slot.Priority = 1.0f;
        Formation.FormationSlots.Add(Slot);
    }
}

void UCrowd_GroupFormationSystem::GenerateWedgeFormation(FCrowd_GroupFormation& Formation)
{
    Formation.FormationSlots.Empty();
    
    int32 NumSlots = Formation.MaxMembers;
    int32 RowsNeeded = FMath::CeilToInt(FMath::Sqrt(NumSlots));
    
    int32 SlotIndex = 0;
    for (int32 Row = 0; Row < RowsNeeded && SlotIndex < NumSlots; Row++)
    {
        int32 SlotsInRow = FMath::Min(Row + 1, NumSlots - SlotIndex);
        float RowSpacing = Formation.FormationRadius / RowsNeeded;
        
        for (int32 Col = 0; Col < SlotsInRow && SlotIndex < NumSlots; Col++)
        {
            FCrowd_FormationSlot Slot;
            Slot.LocalPosition = FVector(
                Row * RowSpacing,
                (Col - SlotsInRow / 2.0f) * 150.0f,
                0.0f
            );
            Slot.Priority = Row == 0 ? 2.0f : 1.0f; // Front row has higher priority
            Formation.FormationSlots.Add(Slot);
            SlotIndex++;
        }
    }
}

void UCrowd_GroupFormationSystem::GenerateColumnFormation(FCrowd_GroupFormation& Formation)
{
    Formation.FormationSlots.Empty();
    
    int32 NumSlots = Formation.MaxMembers;
    float Spacing = Formation.FormationRadius * 2.0f / FMath::Max(1, NumSlots - 1);

    for (int32 i = 0; i < NumSlots; i++)
    {
        FCrowd_FormationSlot Slot;
        Slot.LocalPosition = FVector(
            i * Spacing,
            0.0f,
            0.0f
        );
        Slot.Priority = i == 0 ? 2.0f : 1.0f; // Leader has higher priority
        Formation.FormationSlots.Add(Slot);
    }
}

FVector UCrowd_GroupFormationSystem::CalculateCohesionForce(FMassEntityHandle Entity, const TArray<FMassEntityHandle>& Neighbors)
{
    if (Neighbors.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    FVector CenterOfMass = FVector::ZeroVector;
    int32 ValidNeighbors = 0;

    // Calculate center of mass of neighbors
    for (const FMassEntityHandle& Neighbor : Neighbors)
    {
        if (Neighbor != Entity)
        {
            // In a real implementation, we'd get the actual position from Mass Entity
            // For now, use placeholder logic
            CenterOfMass += FVector(FMath::RandRange(-1000, 1000), FMath::RandRange(-1000, 1000), 0);
            ValidNeighbors++;
        }
    }

    if (ValidNeighbors > 0)
    {
        CenterOfMass /= ValidNeighbors;
        return CenterOfMass.GetSafeNormal() * CohesionWeight;
    }

    return FVector::ZeroVector;
}

FVector UCrowd_GroupFormationSystem::CalculateSeparationForce(FMassEntityHandle Entity, const TArray<FMassEntityHandle>& Neighbors)
{
    FVector SeparationForce = FVector::ZeroVector;
    const float MinSeparationDistance = 200.0f;

    for (const FMassEntityHandle& Neighbor : Neighbors)
    {
        if (Neighbor != Entity)
        {
            // Placeholder distance calculation
            FVector ToNeighbor = FVector(FMath::RandRange(-200, 200), FMath::RandRange(-200, 200), 0);
            float Distance = ToNeighbor.Size();
            
            if (Distance < MinSeparationDistance && Distance > 0.0f)
            {
                SeparationForce -= ToNeighbor.GetSafeNormal() * (MinSeparationDistance - Distance);
            }
        }
    }

    return SeparationForce.GetSafeNormal() * SeparationWeight;
}

FVector UCrowd_GroupFormationSystem::CalculateAlignmentForce(FMassEntityHandle Entity, const TArray<FMassEntityHandle>& Neighbors)
{
    if (Neighbors.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    FVector AverageVelocity = FVector::ZeroVector;
    int32 ValidNeighbors = 0;

    for (const FMassEntityHandle& Neighbor : Neighbors)
    {
        if (Neighbor != Entity)
        {
            // Placeholder velocity - in real implementation would get from Mass Entity
            AverageVelocity += FVector(FMath::RandRange(-1, 1), FMath::RandRange(-1, 1), 0).GetSafeNormal() * 300.0f;
            ValidNeighbors++;
        }
    }

    if (ValidNeighbors > 0)
    {
        AverageVelocity /= ValidNeighbors;
        return AverageVelocity.GetSafeNormal() * AlignmentWeight;
    }

    return FVector::ZeroVector;
}

void UCrowd_GroupFormationSystem::MaintainFormationIntegrity(int32 FormationID, float DeltaTime)
{
    FCrowd_GroupFormation* Formation = ActiveFormations.Find(FormationID);
    if (!Formation)
    {
        return;
    }

    // Check if formation needs reorganization
    if (!IsFormationIntact(FormationID, FormationTolerance))
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastReorganizationTime > ReorganizationCooldown)
        {
            ReorganizeFormation(FormationID);
            LastReorganizationTime = CurrentTime;
        }
    }
}

bool UCrowd_GroupFormationSystem::IsFormationIntact(int32 FormationID, float ToleranceRadius)
{
    FCrowd_GroupFormation* Formation = ActiveFormations.Find(FormationID);
    if (!Formation)
    {
        return false;
    }

    int32 EntitiesInPosition = 0;
    for (const FCrowd_FormationSlot& Slot : Formation->FormationSlots)
    {
        if (Slot.bIsOccupied)
        {
            // In real implementation, check actual entity position vs slot position
            // For now, assume 80% are in position
            if (FMath::RandRange(0.0f, 1.0f) < 0.8f)
            {
                EntitiesInPosition++;
            }
        }
    }

    float IntegrityRatio = Formation->GroupMembers.Num() > 0 ? 
        (float)EntitiesInPosition / Formation->GroupMembers.Num() : 0.0f;

    return IntegrityRatio > 0.7f; // 70% of entities must be in position
}

void UCrowd_GroupFormationSystem::ReorganizeFormation(int32 FormationID)
{
    FCrowd_GroupFormation* Formation = ActiveFormations.Find(FormationID);
    if (!Formation)
    {
        return;
    }

    // Clear all slot assignments
    for (FCrowd_FormationSlot& Slot : Formation->FormationSlots)
    {
        Slot.bIsOccupied = false;
        Slot.OccupyingEntity = FMassEntityHandle();
    }

    // Reassign entities to optimal slots
    for (const FMassEntityHandle& Entity : Formation->GroupMembers)
    {
        FCrowd_FormationSlot* BestSlot = FindBestAvailableSlot(*Formation, FVector::ZeroVector);
        if (BestSlot)
        {
            BestSlot->bIsOccupied = true;
            BestSlot->OccupyingEntity = Entity;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Reorganized formation %d"), FormationID);
}

FCrowd_FormationSlot* UCrowd_GroupFormationSystem::FindBestAvailableSlot(FCrowd_GroupFormation& Formation, FVector EntityPosition)
{
    FCrowd_FormationSlot* BestSlot = nullptr;
    float BestScore = -1.0f;

    for (FCrowd_FormationSlot& Slot : Formation.FormationSlots)
    {
        if (!Slot.bIsOccupied)
        {
            // Score based on priority and distance (if position provided)
            float Score = Slot.Priority;
            if (EntityPosition != FVector::ZeroVector)
            {
                float Distance = FVector::Dist(EntityPosition, Formation.FormationCenter + Slot.LocalPosition);
                Score += 1.0f / (Distance + 1.0f); // Closer is better
            }

            if (Score > BestScore)
            {
                BestScore = Score;
                BestSlot = &Slot;
            }
        }
    }

    return BestSlot;
}

void UCrowd_GroupFormationSystem::UpdateFormationSlotPositions(FCrowd_GroupFormation& Formation)
{
    // Regenerate slot positions based on current formation type
    switch (Formation.FormationType)
    {
        case ECrowd_FormationType::Circle:
            GenerateCircleFormation(Formation);
            break;
        case ECrowd_FormationType::Line:
            GenerateLineFormation(Formation);
            break;
        case ECrowd_FormationType::Wedge:
            GenerateWedgeFormation(Formation);
            break;
        case ECrowd_FormationType::Column:
            GenerateColumnFormation(Formation);
            break;
    }
}