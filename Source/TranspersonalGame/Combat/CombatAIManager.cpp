#include "CombatAIManager.h"
#include "EnemyAIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

ACombatAIManager::ACombatAIManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    TacticalUpdateInterval = 2.0f;
    MaxEngagementRange = 2000.0f;
    bUsePackTactics = true;
    CurrentTarget = nullptr;
    
    CurrentFormation.FormationType = ECombat_FormationType::Circle;
    CurrentFormation.FormationRadius = 500.0f;
}

void ACombatAIManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Start tactical update timer
    GetWorldTimerManager().SetTimer(TacticalUpdateTimer, this, &ACombatAIManager::OnTacticalUpdate, TacticalUpdateInterval, true);
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Initialized with tactical update interval: %f"), TacticalUpdateInterval);
}

void ACombatAIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Clean up null references
    RegisteredEnemies.RemoveAll([](AEnemyAIController* Enemy) {
        return !IsValid(Enemy);
    });
    
    // Update formation center if we have a target
    if (CurrentTarget && RegisteredEnemies.Num() > 0)
    {
        CurrentFormation.CenterPoint = CurrentTarget->GetActorLocation();
    }
}

void ACombatAIManager::RegisterEnemy(AEnemyAIController* Enemy)
{
    if (IsValid(Enemy) && !RegisteredEnemies.Contains(Enemy))
    {
        RegisteredEnemies.Add(Enemy);
        UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Registered enemy %s. Total enemies: %d"), 
               *Enemy->GetName(), RegisteredEnemies.Num());
        
        // Update tactical positions when new enemy joins
        UpdateTacticalPositions();
    }
}

void ACombatAIManager::UnregisterEnemy(AEnemyAIController* Enemy)
{
    if (RegisteredEnemies.Contains(Enemy))
    {
        RegisteredEnemies.Remove(Enemy);
        UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Unregistered enemy %s. Remaining enemies: %d"), 
               *Enemy->GetName(), RegisteredEnemies.Num());
        
        // Update tactical positions when enemy leaves
        UpdateTacticalPositions();
    }
}

void ACombatAIManager::SetTarget(APawn* NewTarget)
{
    if (CurrentTarget != NewTarget)
    {
        CurrentTarget = NewTarget;
        
        if (CurrentTarget)
        {
            UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: New target acquired: %s"), *CurrentTarget->GetName());
            UpdateTacticalPositions();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Target cleared"));
        }
    }
}

void ACombatAIManager::UpdateTacticalPositions()
{
    if (!CurrentTarget || RegisteredEnemies.Num() == 0)
    {
        return;
    }
    
    // Generate formation positions based on current tactic
    TArray<FCombat_TacticalPosition> NewPositions = GenerateFormationPositions(
        CurrentTarget->GetActorLocation(), 
        CurrentFormation.FormationType, 
        RegisteredEnemies.Num()
    );
    
    CurrentFormation.Positions = NewPositions;
    
    // Assign positions to enemies
    for (int32 i = 0; i < RegisteredEnemies.Num() && i < NewPositions.Num(); i++)
    {
        if (IsValid(RegisteredEnemies[i]))
        {
            // TODO: Send tactical position to enemy AI controller
            // This would be implemented when EnemyAIController is created
            UE_LOG(LogTemp, Log, TEXT("CombatAIManager: Assigned tactical position to %s at %s"), 
                   *RegisteredEnemies[i]->GetName(), *NewPositions[i].Position.ToString());
        }
    }
}

FCombat_TacticalPosition ACombatAIManager::CalculateFlankingPosition(const FVector& TargetLocation, int32 FlankIndex)
{
    FCombat_TacticalPosition FlankPosition;
    
    // Calculate flanking angle based on index
    float AngleStep = 360.0f / FMath::Max(RegisteredEnemies.Num(), 1);
    float FlankAngle = FlankIndex * AngleStep;
    
    // Convert to radians
    float AngleRadians = FMath::DegreesToRadians(FlankAngle);
    
    // Calculate position around target
    FVector Offset = FVector(
        FMath::Cos(AngleRadians) * CurrentFormation.FormationRadius,
        FMath::Sin(AngleRadians) * CurrentFormation.FormationRadius,
        0.0f
    );
    
    FlankPosition.Position = TargetLocation + Offset;
    FlankPosition.bIsFlankingPosition = true;
    FlankPosition.ThreatLevel = CalculateThreatLevel(FlankPosition.Position, CurrentTarget);
    FlankPosition.bHasCover = false; // TODO: Implement cover detection
    
    return FlankPosition;
}

bool ACombatAIManager::ShouldEngageTarget(APawn* Target) const
{
    if (!Target)
    {
        return false;
    }
    
    // Check distance
    float DistanceToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistanceToTarget > MaxEngagementRange)
    {
        return false;
    }
    
    // Check if target is alive (has health component or similar)
    // TODO: Implement health check when health system is available
    
    return true;
}

void ACombatAIManager::ExecutePackTactic(ECombat_FormationType TacticType)
{
    CurrentFormation.FormationType = TacticType;
    UpdateTacticalPositions();
    
    UE_LOG(LogTemp, Warning, TEXT("CombatAIManager: Executing pack tactic: %d"), (int32)TacticType);
}

TArray<FCombat_TacticalPosition> ACombatAIManager::GenerateFormationPositions(const FVector& CenterPoint, ECombat_FormationType FormationType, int32 NumPositions)
{
    TArray<FCombat_TacticalPosition> Positions;
    
    if (NumPositions <= 0)
    {
        return Positions;
    }
    
    switch (FormationType)
    {
        case ECombat_FormationType::Circle:
        {
            float AngleStep = 360.0f / NumPositions;
            for (int32 i = 0; i < NumPositions; i++)
            {
                FCombat_TacticalPosition Position = CalculateFlankingPosition(CenterPoint, i);
                Positions.Add(Position);
            }
            break;
        }
        
        case ECombat_FormationType::Line:
        {
            FVector LineDirection = FVector::ForwardVector;
            float Spacing = CurrentFormation.FormationRadius / NumPositions;
            
            for (int32 i = 0; i < NumPositions; i++)
            {
                FCombat_TacticalPosition Position;
                Position.Position = CenterPoint + (LineDirection * (i - NumPositions/2) * Spacing);
                Position.ThreatLevel = CalculateThreatLevel(Position.Position, CurrentTarget);
                Position.bIsFlankingPosition = (i == 0 || i == NumPositions - 1);
                Positions.Add(Position);
            }
            break;
        }
        
        case ECombat_FormationType::Ambush:
        {
            // Create ambush positions with some enemies hidden
            for (int32 i = 0; i < NumPositions; i++)
            {
                FCombat_TacticalPosition Position = CalculateFlankingPosition(CenterPoint, i);
                Position.bHasCover = (i % 2 == 0); // Alternate cover positions
                Positions.Add(Position);
            }
            break;
        }
        
        default:
            // Default to circle formation
            return GenerateFormationPositions(CenterPoint, ECombat_FormationType::Circle, NumPositions);
    }
    
    return Positions;
}

void ACombatAIManager::OnTacticalUpdate()
{
    if (bUsePackTactics && RegisteredEnemies.Num() > 0)
    {
        UpdateTacticalPositions();
        
        // Debug visualization
        if (GEngine && CurrentTarget)
        {
            for (const FCombat_TacticalPosition& Position : CurrentFormation.Positions)
            {
                FColor DebugColor = Position.bIsFlankingPosition ? FColor::Red : FColor::Yellow;
                DrawDebugSphere(GetWorld(), Position.Position, 50.0f, 8, DebugColor, false, TacticalUpdateInterval);
            }
        }
    }
}

float ACombatAIManager::CalculateThreatLevel(const FVector& Position, APawn* Target) const
{
    if (!Target)
    {
        return 0.0f;
    }
    
    float Distance = FVector::Dist(Position, Target->GetActorLocation());
    float ThreatLevel = 1.0f - (Distance / MaxEngagementRange);
    
    // Higher threat for closer positions
    ThreatLevel = FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
    
    // Bonus threat for flanking positions
    FVector ToTarget = (Target->GetActorLocation() - Position).GetSafeNormal();
    FVector TargetForward = Target->GetActorForwardVector();
    float DotProduct = FVector::DotProduct(ToTarget, TargetForward);
    
    if (DotProduct < 0.0f) // Behind target
    {
        ThreatLevel += 0.3f;
    }
    
    return FMath::Clamp(ThreatLevel, 0.0f, 2.0f);
}

bool ACombatAIManager::HasLineOfSight(const FVector& FromPosition, const FVector& ToPosition) const
{
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        FromPosition,
        ToPosition,
        ECC_Visibility,
        QueryParams
    );
    
    return !bHit;
}