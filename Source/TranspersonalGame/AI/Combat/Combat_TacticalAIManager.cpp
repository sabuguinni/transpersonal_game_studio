#include "Combat_TacticalAIManager.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"

ACombat_TacticalAIManager::ACombat_TacticalAIManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create detection sphere component
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    RootComponent = DetectionSphere;
    DetectionSphere->SetSphereRadius(1000.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize tactical parameters
    DetectionRadius = 1000.0f;
    FormationUpdateInterval = 2.0f;
    CurrentTacticalState = ECombat_TacticalState::Patrol;
    PrimaryTarget = nullptr;
    CombatCenter = FVector::ZeroVector;
    LastFormationUpdate = 0.0f;

    // Setup default formations
    FCombat_TacticalFormation CircleFormation;
    CircleFormation.FormationType = ECombat_FormationType::Circle;
    CircleFormation.FormationRadius = 400.0f;
    CircleFormation.MaxUnits = 6;
    AvailableFormations.Add(CircleFormation);

    FCombat_TacticalFormation LineFormation;
    LineFormation.FormationType = ECombat_FormationType::Line;
    LineFormation.FormationRadius = 300.0f;
    LineFormation.MaxUnits = 8;
    AvailableFormations.Add(LineFormation);

    FCombat_TacticalFormation WedgeFormation;
    WedgeFormation.FormationType = ECombat_FormationType::Wedge;
    WedgeFormation.FormationRadius = 350.0f;
    WedgeFormation.MaxUnits = 5;
    AvailableFormations.Add(WedgeFormation);
}

void ACombat_TacticalAIManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACombat_TacticalAIManager::OnDetectionSphereBeginOverlap);
    DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ACombat_TacticalAIManager::OnDetectionSphereEndOverlap);

    // Set initial combat center to actor location
    CombatCenter = GetActorLocation();

    UE_LOG(LogTemp, Warning, TEXT("Tactical AI Manager initialized at location: %s"), *GetActorLocation().ToString());
}

void ACombat_TacticalAIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update tactical analysis
    AnalyzeTacticalSituation();

    // Update formations periodically
    if (GetWorld()->GetTimeSeconds() - LastFormationUpdate > FormationUpdateInterval)
    {
        UpdateFormationPositions();
        IssueUnitOrders();
        LastFormationUpdate = GetWorld()->GetTimeSeconds();
    }

    // Calculate combat effectiveness
    CalculateCombatEffectiveness();
}

void ACombat_TacticalAIManager::RegisterCombatUnit(AActor* Unit, ECombat_UnitRole Role)
{
    if (!Unit)
    {
        return;
    }

    // Check if unit is already registered
    for (FCombat_TacticalUnit& ExistingUnit : ManagedUnits)
    {
        if (ExistingUnit.UnitActor == Unit)
        {
            ExistingUnit.Role = Role; // Update role if already registered
            return;
        }
    }

    // Create new tactical unit
    FCombat_TacticalUnit NewUnit;
    NewUnit.UnitActor = Unit;
    NewUnit.Role = Role;
    NewUnit.AssignedPosition = Unit->GetActorLocation();
    NewUnit.CombatEffectiveness = 1.0f;
    NewUnit.bIsInCombat = false;

    ManagedUnits.Add(NewUnit);

    UE_LOG(LogTemp, Log, TEXT("Registered combat unit: %s with role: %d"), *Unit->GetName(), (int32)Role);
}

void ACombat_TacticalAIManager::UnregisterCombatUnit(AActor* Unit)
{
    if (!Unit)
    {
        return;
    }

    for (int32 i = ManagedUnits.Num() - 1; i >= 0; i--)
    {
        if (ManagedUnits[i].UnitActor == Unit)
        {
            ManagedUnits.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Unregistered combat unit: %s"), *Unit->GetName());
            break;
        }
    }
}

void ACombat_TacticalAIManager::SetFormation(ECombat_FormationType FormationType)
{
    // Find the requested formation
    for (const FCombat_TacticalFormation& Formation : AvailableFormations)
    {
        if (Formation.FormationType == FormationType)
        {
            // Update formation positions immediately
            UpdateFormationPositions();
            UE_LOG(LogTemp, Log, TEXT("Formation changed to: %d"), (int32)FormationType);
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Formation type %d not found in available formations"), (int32)FormationType);
}

void ACombat_TacticalAIManager::UpdateTacticalState(ECombat_TacticalState NewState)
{
    if (CurrentTacticalState != NewState)
    {
        ECombat_TacticalState PreviousState = CurrentTacticalState;
        CurrentTacticalState = NewState;

        // Handle state transitions
        switch (NewState)
        {
        case ECombat_TacticalState::Engage:
            SetFormation(ECombat_FormationType::Line);
            break;
        case ECombat_TacticalState::Flank:
            SetFormation(ECombat_FormationType::Wedge);
            break;
        case ECombat_TacticalState::Retreat:
            SetFormation(ECombat_FormationType::Circle);
            break;
        default:
            break;
        }

        UE_LOG(LogTemp, Log, TEXT("Tactical state changed from %d to %d"), (int32)PreviousState, (int32)NewState);
    }
}

void ACombat_TacticalAIManager::AssignTarget(AActor* Target)
{
    PrimaryTarget = Target;
    if (Target)
    {
        CombatCenter = Target->GetActorLocation();
        UpdateTacticalState(ECombat_TacticalState::Engage);
        UE_LOG(LogTemp, Log, TEXT("Primary target assigned: %s"), *Target->GetName());
    }
    else
    {
        UpdateTacticalState(ECombat_TacticalState::Patrol);
        UE_LOG(LogTemp, Log, TEXT("Primary target cleared"));
    }
}

FVector ACombat_TacticalAIManager::GetOptimalAttackPosition(AActor* Unit)
{
    if (!Unit || !PrimaryTarget)
    {
        return Unit ? Unit->GetActorLocation() : FVector::ZeroVector;
    }

    // Find the unit in managed units
    for (const FCombat_TacticalUnit& TacticalUnit : ManagedUnits)
    {
        if (TacticalUnit.UnitActor == Unit)
        {
            return TacticalUnit.AssignedPosition;
        }
    }

    // Fallback: calculate position based on target location
    FVector TargetLocation = PrimaryTarget->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - Unit->GetActorLocation()).GetSafeNormal();
    return TargetLocation - (DirectionToTarget * 200.0f); // Position 200 units away from target
}

bool ACombat_TacticalAIManager::ShouldRetreat()
{
    // Calculate overall combat effectiveness
    float TotalEffectiveness = 0.0f;
    int32 ActiveUnits = 0;

    for (const FCombat_TacticalUnit& Unit : ManagedUnits)
    {
        if (Unit.UnitActor && Unit.bIsInCombat)
        {
            TotalEffectiveness += Unit.CombatEffectiveness;
            ActiveUnits++;
        }
    }

    if (ActiveUnits == 0)
    {
        return true; // No active units, should retreat
    }

    float AverageEffectiveness = TotalEffectiveness / ActiveUnits;
    
    // Retreat if average effectiveness is below 30% or we have fewer than 2 active units
    return AverageEffectiveness < 0.3f || ActiveUnits < 2;
}

void ACombat_TacticalAIManager::ExecuteTacticalManeuver(ECombat_TacticalManeuver Maneuver)
{
    switch (Maneuver)
    {
    case ECombat_TacticalManeuver::Surround:
        SetFormation(ECombat_FormationType::Circle);
        UpdateTacticalState(ECombat_TacticalState::Flank);
        break;
    case ECombat_TacticalManeuver::Charge:
        SetFormation(ECombat_FormationType::Wedge);
        UpdateTacticalState(ECombat_TacticalState::Engage);
        break;
    case ECombat_TacticalManeuver::Retreat:
        SetFormation(ECombat_FormationType::Line);
        UpdateTacticalState(ECombat_TacticalState::Retreat);
        break;
    case ECombat_TacticalManeuver::Ambush:
        // Spread units in ambush positions
        UpdateTacticalState(ECombat_TacticalState::Ambush);
        break;
    }

    UE_LOG(LogTemp, Log, TEXT("Executing tactical maneuver: %d"), (int32)Maneuver);
}

void ACombat_TacticalAIManager::OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    // Check if it's a character (potential enemy or ally)
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        // Simple enemy detection logic (can be expanded)
        if (Character->GetName().Contains(TEXT("Player")) || Character->GetName().Contains(TEXT("Enemy")))
        {
            DetectedEnemies.AddUnique(OtherActor);
            
            // If we don't have a primary target, assign this one
            if (!PrimaryTarget)
            {
                AssignTarget(OtherActor);
            }
            
            UE_LOG(LogTemp, Log, TEXT("Enemy detected: %s"), *OtherActor->GetName());
        }
        else
        {
            DetectedAllies.AddUnique(OtherActor);
            UE_LOG(LogTemp, Log, TEXT("Ally detected: %s"), *OtherActor->GetName());
        }
    }
}

void ACombat_TacticalAIManager::OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor)
    {
        return;
    }

    DetectedEnemies.Remove(OtherActor);
    DetectedAllies.Remove(OtherActor);

    // If this was our primary target, find a new one
    if (PrimaryTarget == OtherActor)
    {
        if (DetectedEnemies.Num() > 0)
        {
            AssignTarget(DetectedEnemies[0]);
        }
        else
        {
            AssignTarget(nullptr);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Actor left detection range: %s"), *OtherActor->GetName());
}

void ACombat_TacticalAIManager::UpdateFormationPositions()
{
    if (ManagedUnits.Num() == 0 || AvailableFormations.Num() == 0)
    {
        return;
    }

    // Use the first formation as default (can be made more sophisticated)
    const FCombat_TacticalFormation& CurrentFormation = AvailableFormations[0];
    
    // Calculate formation positions around combat center
    for (int32 i = 0; i < ManagedUnits.Num() && i < CurrentFormation.MaxUnits; i++)
    {
        FCombat_TacticalUnit& Unit = ManagedUnits[i];
        
        if (!Unit.UnitActor)
        {
            continue;
        }

        FVector NewPosition;
        
        switch (CurrentFormation.FormationType)
        {
        case ECombat_FormationType::Circle:
            {
                float Angle = (2.0f * PI * i) / FMath::Min(ManagedUnits.Num(), CurrentFormation.MaxUnits);
                NewPosition = CombatCenter + FVector(
                    FMath::Cos(Angle) * CurrentFormation.FormationRadius,
                    FMath::Sin(Angle) * CurrentFormation.FormationRadius,
                    0.0f
                );
            }
            break;
        case ECombat_FormationType::Line:
            {
                float Spacing = CurrentFormation.FormationRadius / FMath::Max(1, ManagedUnits.Num() - 1);
                NewPosition = CombatCenter + FVector(
                    (i - ManagedUnits.Num() / 2.0f) * Spacing,
                    -200.0f, // Line formation behind combat center
                    0.0f
                );
            }
            break;
        case ECombat_FormationType::Wedge:
            {
                float Row = FMath::Floor(FMath::Sqrt(i));
                float PositionInRow = i - (Row * Row);
                NewPosition = CombatCenter + FVector(
                    Row * -100.0f,
                    (PositionInRow - Row / 2.0f) * 150.0f,
                    0.0f
                );
            }
            break;
        }

        Unit.AssignedPosition = NewPosition;
    }
}

void ACombat_TacticalAIManager::AnalyzeTacticalSituation()
{
    // Analyze current tactical situation and update state accordingly
    int32 EnemyCount = DetectedEnemies.Num();
    int32 AllyCount = ManagedUnits.Num();

    if (EnemyCount == 0)
    {
        if (CurrentTacticalState != ECombat_TacticalState::Patrol)
        {
            UpdateTacticalState(ECombat_TacticalState::Patrol);
        }
    }
    else if (EnemyCount > AllyCount * 2)
    {
        // Outnumbered, consider retreat
        if (ShouldRetreat())
        {
            UpdateTacticalState(ECombat_TacticalState::Retreat);
        }
    }
    else if (EnemyCount == 1 && AllyCount >= 3)
    {
        // Good opportunity for flanking
        UpdateTacticalState(ECombat_TacticalState::Flank);
    }
    else
    {
        // Standard engagement
        if (CurrentTacticalState != ECombat_TacticalState::Engage)
        {
            UpdateTacticalState(ECombat_TacticalState::Engage);
        }
    }
}

void ACombat_TacticalAIManager::IssueUnitOrders()
{
    // Issue orders to managed units based on current tactical state
    for (FCombat_TacticalUnit& Unit : ManagedUnits)
    {
        if (!Unit.UnitActor)
        {
            continue;
        }

        // Update combat status based on proximity to enemies
        Unit.bIsInCombat = false;
        for (AActor* Enemy : DetectedEnemies)
        {
            if (Enemy && FVector::Dist(Unit.UnitActor->GetActorLocation(), Enemy->GetActorLocation()) < 500.0f)
            {
                Unit.bIsInCombat = true;
                break;
            }
        }

        // Here you would typically send movement orders to the unit's AI controller
        // For now, we just log the intended position
        UE_LOG(LogTemp, VeryVerbose, TEXT("Unit %s assigned position: %s"), 
               *Unit.UnitActor->GetName(), 
               *Unit.AssignedPosition.ToString());
    }
}

void ACombat_TacticalAIManager::CalculateCombatEffectiveness()
{
    for (FCombat_TacticalUnit& Unit : ManagedUnits)
    {
        if (!Unit.UnitActor)
        {
            Unit.CombatEffectiveness = 0.0f;
            continue;
        }

        // Base effectiveness
        float Effectiveness = 1.0f;

        // Reduce effectiveness based on distance from assigned position
        float DistanceFromPosition = FVector::Dist(Unit.UnitActor->GetActorLocation(), Unit.AssignedPosition);
        if (DistanceFromPosition > 100.0f)
        {
            Effectiveness *= FMath::Clamp(1.0f - (DistanceFromPosition / 1000.0f), 0.1f, 1.0f);
        }

        // Modify based on role and tactical state
        switch (Unit.Role)
        {
        case ECombat_UnitRole::Tank:
            if (CurrentTacticalState == ECombat_TacticalState::Engage)
            {
                Effectiveness *= 1.2f; // Tanks are more effective in direct engagement
            }
            break;
        case ECombat_UnitRole::Flanker:
            if (CurrentTacticalState == ECombat_TacticalState::Flank)
            {
                Effectiveness *= 1.3f; // Flankers excel at flanking maneuvers
            }
            break;
        case ECombat_UnitRole::Support:
            Effectiveness *= 0.8f; // Support units have lower direct combat effectiveness
            break;
        }

        Unit.CombatEffectiveness = FMath::Clamp(Effectiveness, 0.1f, 2.0f);
    }
}