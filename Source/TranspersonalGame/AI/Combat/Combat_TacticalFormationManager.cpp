#include "Combat_TacticalFormationManager.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UCombat_TacticalFormationManager::UCombat_TacticalFormationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    MaxFormationDistance = 1000.0f;
    FormationUpdateInterval = 0.5f;
    bAutoUpdateFormations = true;
    LastUpdateTime = 0.0f;
    FormationCenter = FVector::ZeroVector;
    ActiveFormationName = TEXT("Default");
}

void UCombat_TacticalFormationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default formations
    TArray<FVector> PackHuntSlots;
    PackHuntSlots.Add(FVector(0, 0, 0));        // Leader position
    PackHuntSlots.Add(FVector(-200, -300, 0));  // Left flank
    PackHuntSlots.Add(FVector(200, -300, 0));   // Right flank
    PackHuntSlots.Add(FVector(0, -500, 0));     // Rear support
    CreateFormation(TEXT("PackHunt"), PackHuntSlots, 600.0f);
    
    TArray<FVector> DefensiveSlots;
    DefensiveSlots.Add(FVector(0, 200, 0));     // Front guard
    DefensiveSlots.Add(FVector(-150, 0, 0));    // Left guard
    DefensiveSlots.Add(FVector(150, 0, 0));     // Right guard
    DefensiveSlots.Add(FVector(0, -200, 0));    // Rear guard
    CreateFormation(TEXT("Defensive"), DefensiveSlots, 400.0f);
    
    TArray<FVector> FlankingSlots;
    FlankingSlots.Add(FVector(0, 0, 0));        // Distraction
    FlankingSlots.Add(FVector(-400, 200, 0));   // Left flanker
    FlankingSlots.Add(FVector(400, 200, 0));    // Right flanker
    CreateFormation(TEXT("Flanking"), FlankingSlots, 800.0f);
}

void UCombat_TacticalFormationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoUpdateFormations)
    {
        LastUpdateTime += DeltaTime;
        if (LastUpdateTime >= FormationUpdateInterval)
        {
            UpdateFormationPositions();
            ValidateFormationIntegrity();
            LastUpdateTime = 0.0f;
        }
    }
}

bool UCombat_TacticalFormationManager::CreateFormation(const FString& FormationName, const TArray<FVector>& SlotPositions, float Radius)
{
    if (FormationName.IsEmpty() || SlotPositions.Num() == 0)
    {
        return false;
    }
    
    FCombat_TacticalFormation NewFormation;
    NewFormation.FormationName = FormationName;
    NewFormation.FormationRadius = Radius;
    NewFormation.RequiredState = ECombat_TacticalState::Hunting;
    
    for (const FVector& Position : SlotPositions)
    {
        FCombat_FormationSlot Slot;
        Slot.RelativePosition = Position;
        Slot.Priority = 1.0f;
        Slot.bIsOccupied = false;
        NewFormation.Slots.Add(Slot);
    }
    
    AvailableFormations.Add(FormationName, NewFormation);
    return true;
}

bool UCombat_TacticalFormationManager::AssignPawnToFormation(APawn* Pawn, const FString& FormationName)
{
    if (!Pawn || !AvailableFormations.Contains(FormationName))
    {
        return false;
    }
    
    FCombat_TacticalFormation* Formation = AvailableFormations.Find(FormationName);
    if (!Formation)
    {
        return false;
    }
    
    // Find first available slot
    for (FCombat_FormationSlot& Slot : Formation->Slots)
    {
        if (!Slot.bIsOccupied)
        {
            Slot.bIsOccupied = true;
            Slot.AssignedPawn = Pawn;
            
            // Update AI controller blackboard if available
            if (AAIController* AIController = Cast<AAIController>(Pawn->GetController()))
            {
                if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
                {
                    FVector WorldPosition = FormationCenter + Slot.RelativePosition;
                    Blackboard->SetValueAsVector(TEXT("FormationPosition"), WorldPosition);
                    Blackboard->SetValueAsString(TEXT("FormationName"), FormationName);
                }
            }
            
            return true;
        }
    }
    
    return false;
}

bool UCombat_TacticalFormationManager::RemovePawnFromFormation(APawn* Pawn)
{
    if (!Pawn)
    {
        return false;
    }
    
    for (auto& FormationPair : AvailableFormations)
    {
        FCombat_TacticalFormation& Formation = FormationPair.Value;
        for (FCombat_FormationSlot& Slot : Formation.Slots)
        {
            if (Slot.AssignedPawn == Pawn)
            {
                Slot.bIsOccupied = false;
                Slot.AssignedPawn = nullptr;
                
                // Clear AI controller blackboard
                if (AAIController* AIController = Cast<AAIController>(Pawn->GetController()))
                {
                    if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
                    {
                        Blackboard->ClearValue(TEXT("FormationPosition"));
                        Blackboard->ClearValue(TEXT("FormationName"));
                    }
                }
                
                return true;
            }
        }
    }
    
    return false;
}

FVector UCombat_TacticalFormationManager::GetAssignedPosition(APawn* Pawn) const
{
    if (!Pawn)
    {
        return FVector::ZeroVector;
    }
    
    for (const auto& FormationPair : AvailableFormations)
    {
        const FCombat_TacticalFormation& Formation = FormationPair.Value;
        for (const FCombat_FormationSlot& Slot : Formation.Slots)
        {
            if (Slot.AssignedPawn == Pawn)
            {
                return FormationCenter + Slot.RelativePosition;
            }
        }
    }
    
    return FVector::ZeroVector;
}

bool UCombat_TacticalFormationManager::UpdateFormationCenter(const FString& FormationName, const FVector& NewCenter)
{
    if (!AvailableFormations.Contains(FormationName))
    {
        return false;
    }
    
    FormationCenter = NewCenter;
    ActiveFormationName = FormationName;
    
    // Update all assigned pawns' target positions
    FCombat_TacticalFormation* Formation = AvailableFormations.Find(FormationName);
    if (Formation)
    {
        for (const FCombat_FormationSlot& Slot : Formation->Slots)
        {
            if (Slot.bIsOccupied && Slot.AssignedPawn.IsValid())
            {
                if (AAIController* AIController = Cast<AAIController>(Slot.AssignedPawn->GetController()))
                {
                    if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
                    {
                        FVector WorldPosition = FormationCenter + Slot.RelativePosition;
                        Blackboard->SetValueAsVector(TEXT("FormationPosition"), WorldPosition);
                    }
                }
            }
        }
    }
    
    return true;
}

float UCombat_TacticalFormationManager::CalculateFormationCohesion(const FString& FormationName) const
{
    if (!AvailableFormations.Contains(FormationName))
    {
        return 0.0f;
    }
    
    const FCombat_TacticalFormation* Formation = AvailableFormations.Find(FormationName);
    if (!Formation)
    {
        return 0.0f;
    }
    
    float TotalCohesion = 0.0f;
    int32 ValidSlots = 0;
    
    for (const FCombat_FormationSlot& Slot : Formation->Slots)
    {
        if (Slot.bIsOccupied && Slot.AssignedPawn.IsValid())
        {
            FVector ExpectedPosition = FormationCenter + Slot.RelativePosition;
            FVector ActualPosition = Slot.AssignedPawn->GetActorLocation();
            float Distance = FVector::Dist(ExpectedPosition, ActualPosition);
            
            // Cohesion decreases with distance from expected position
            float SlotCohesion = FMath::Clamp(1.0f - (Distance / Formation->FormationRadius), 0.0f, 1.0f);
            TotalCohesion += SlotCohesion;
            ValidSlots++;
        }
    }
    
    return ValidSlots > 0 ? TotalCohesion / ValidSlots : 0.0f;
}

TArray<APawn*> UCombat_TacticalFormationManager::GetFormationMembers(const FString& FormationName) const
{
    TArray<APawn*> Members;
    
    if (!AvailableFormations.Contains(FormationName))
    {
        return Members;
    }
    
    const FCombat_TacticalFormation* Formation = AvailableFormations.Find(FormationName);
    if (Formation)
    {
        for (const FCombat_FormationSlot& Slot : Formation->Slots)
        {
            if (Slot.bIsOccupied && Slot.AssignedPawn.IsValid())
            {
                Members.Add(Slot.AssignedPawn.Get());
            }
        }
    }
    
    return Members;
}

bool UCombat_TacticalFormationManager::IsFormationComplete(const FString& FormationName) const
{
    if (!AvailableFormations.Contains(FormationName))
    {
        return false;
    }
    
    const FCombat_TacticalFormation* Formation = AvailableFormations.Find(FormationName);
    if (!Formation)
    {
        return false;
    }
    
    for (const FCombat_FormationSlot& Slot : Formation->Slots)
    {
        if (!Slot.bIsOccupied)
        {
            return false;
        }
    }
    
    return true;
}

void UCombat_TacticalFormationManager::ExecuteCoordinatedAttack(const FString& FormationName, AActor* Target)
{
    if (!Target || !AvailableFormations.Contains(FormationName))
    {
        return;
    }
    
    TArray<APawn*> Members = GetFormationMembers(FormationName);
    
    for (APawn* Member : Members)
    {
        if (AAIController* AIController = Cast<AAIController>(Member->GetController()))
        {
            if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
            {
                Blackboard->SetValueAsObject(TEXT("AttackTarget"), Target);
                Blackboard->SetValueAsBool(TEXT("CoordinatedAttack"), true);
            }
        }
    }
}

void UCombat_TacticalFormationManager::ExecuteDefensiveManeuver(const FString& FormationName, const FVector& ThreatDirection)
{
    if (!AvailableFormations.Contains(FormationName))
    {
        return;
    }
    
    // Switch to defensive formation and orient towards threat
    UpdateFormationCenter(TEXT("Defensive"), FormationCenter);
    
    TArray<APawn*> Members = GetFormationMembers(FormationName);
    
    for (APawn* Member : Members)
    {
        if (AAIController* AIController = Cast<AAIController>(Member->GetController()))
        {
            if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
            {
                Blackboard->SetValueAsVector(TEXT("ThreatDirection"), ThreatDirection);
                Blackboard->SetValueAsBool(TEXT("DefensiveMode"), true);
            }
        }
    }
}

void UCombat_TacticalFormationManager::ExecuteFlankingManeuver(const FString& FormationName, AActor* Target)
{
    if (!Target || !AvailableFormations.Contains(FormationName))
    {
        return;
    }
    
    // Switch to flanking formation
    UpdateFormationCenter(TEXT("Flanking"), Target->GetActorLocation());
    
    TArray<APawn*> Members = GetFormationMembers(FormationName);
    
    for (int32 i = 0; i < Members.Num(); i++)
    {
        if (AAIController* AIController = Cast<AAIController>(Members[i]->GetController()))
        {
            if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
            {
                Blackboard->SetValueAsObject(TEXT("FlankTarget"), Target);
                
                // Assign roles: 0=distraction, 1=left flank, 2=right flank
                FString Role = (i == 0) ? TEXT("Distraction") : 
                              (i == 1) ? TEXT("LeftFlank") : TEXT("RightFlank");
                Blackboard->SetValueAsString(TEXT("FlankRole"), Role);
            }
        }
    }
}

void UCombat_TacticalFormationManager::UpdateFormationPositions()
{
    if (ActiveFormationName.IsEmpty() || !AvailableFormations.Contains(ActiveFormationName))
    {
        return;
    }
    
    FCombat_TacticalFormation* Formation = AvailableFormations.Find(ActiveFormationName);
    if (!Formation)
    {
        return;
    }
    
    for (const FCombat_FormationSlot& Slot : Formation->Slots)
    {
        if (Slot.bIsOccupied && Slot.AssignedPawn.IsValid())
        {
            if (AAIController* AIController = Cast<AAIController>(Slot.AssignedPawn->GetController()))
            {
                if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
                {
                    FVector WorldPosition = CalculateOptimalSlotPosition(Slot, FormationCenter);
                    Blackboard->SetValueAsVector(TEXT("FormationPosition"), WorldPosition);
                }
            }
        }
    }
}

void UCombat_TacticalFormationManager::ValidateFormationIntegrity()
{
    for (auto& FormationPair : AvailableFormations)
    {
        FCombat_TacticalFormation& Formation = FormationPair.Value;
        for (FCombat_FormationSlot& Slot : Formation.Slots)
        {
            if (Slot.bIsOccupied && !Slot.AssignedPawn.IsValid())
            {
                // Clean up invalid references
                Slot.bIsOccupied = false;
                Slot.AssignedPawn = nullptr;
            }
        }
    }
}

FVector UCombat_TacticalFormationManager::CalculateOptimalSlotPosition(const FCombat_FormationSlot& Slot, const FVector& Center) const
{
    FVector BasePosition = Center + Slot.RelativePosition;
    
    // Check if position is navigable
    if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(BasePosition, NavLocation, FVector(200.0f, 200.0f, 500.0f)))
        {
            return NavLocation.Location;
        }
    }
    
    return BasePosition;
}

bool UCombat_TacticalFormationManager::IsSlotPositionValid(const FVector& Position) const
{
    if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
    {
        FNavLocation NavLocation;
        return NavSys->ProjectPointToNavigation(Position, NavLocation, FVector(100.0f, 100.0f, 200.0f));
    }
    
    return true;
}