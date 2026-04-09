#include "TacticalCombatSystem.h"
#include "CombatAIManager.h"
#include "DinosaurCombatAI.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UTacticalCombatSystem::UTacticalCombatSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update tactical analysis 10 times per second
    
    // Initialize default tactical parameters
    TacticalUpdateInterval = 0.1f;
    MaxTacticalDistance = 2000.0f;
    MinFlankingAngle = 45.0f;
    MaxFlankingAngle = 135.0f;
    AmbushDetectionRadius = 800.0f;
    TerrainAnalysisRadius = 500.0f;
    
    LastTacticalUpdate = 0.0f;
    bIsInitialized = false;
}

void UTacticalCombatSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTacticalSystem();
}

void UTacticalCombatSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsInitialized)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastTacticalUpdate >= TacticalUpdateInterval)
    {
        UpdateTacticalAnalysis();
        LastTacticalUpdate = CurrentTime;
    }
}

void UTacticalCombatSystem::InitializeTacticalSystem()
{
    // Find the Combat AI Manager in the world
    CombatManager = Cast<ACombatAIManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ACombatAIManager::StaticClass()));
    
    if (!CombatManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("TacticalCombatSystem: No CombatAIManager found in world"));
        return;
    }
    
    // Initialize tactical data structures
    ActiveCombatSituations.Empty();
    FlankingPositions.Empty();
    AmbushPositions.Empty();
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("TacticalCombatSystem initialized successfully"));
}

void UTacticalCombatSystem::UpdateTacticalAnalysis()
{
    if (!CombatManager)
    {
        return;
    }
    
    // Update all active combat situations
    for (auto& Situation : ActiveCombatSituations)
    {
        if (IsValid(Situation.Attacker) && IsValid(Situation.Target))
        {
            UpdateCombatSituation(Situation);
        }
    }
    
    // Clean up invalid situations
    ActiveCombatSituations.RemoveAll([](const FCombatSituation& Situation)
    {
        return !IsValid(Situation.Attacker) || !IsValid(Situation.Target);
    });
}

void UTacticalCombatSystem::UpdateCombatSituation(FCombatSituation& Situation)
{
    // Update distance and positioning
    float CurrentDistance = FVector::Dist(Situation.Attacker->GetActorLocation(), Situation.Target->GetActorLocation());
    Situation.Distance = CurrentDistance;
    
    // Update tactical advantage
    Situation.TacticalAdvantage = CalculateTacticalAdvantage(Situation.Attacker, Situation.Target);
    
    // Update flanking opportunities
    TArray<FVector> FlankingPositions = CalculateFlankingPositions(Situation.Attacker, Situation.Target);
    Situation.FlankingPositions = FlankingPositions;
    
    // Update terrain advantage
    Situation.bHasTerrainAdvantage = HasTerrainAdvantage(Situation.Attacker, Situation.Target);
    
    // Update pack coordination if applicable
    if (Situation.PackMembers.Num() > 1)
    {
        UpdatePackCoordination(Situation);
    }
    
    // Determine optimal tactical action
    Situation.RecommendedAction = DetermineTacticalAction(Situation);
}

ETacticalAction UTacticalCombatSystem::DetermineTacticalAction(const FCombatSituation& Situation)
{
    // Priority-based tactical decision making
    
    // 1. Check if retreat is necessary (low health, overwhelming odds)
    if (ShouldRetreat(Situation))
    {
        return ETacticalAction::Retreat;
    }
    
    // 2. Check for ambush opportunities
    if (CanSetupAmbush(Situation))
    {
        return ETacticalAction::Ambush;
    }
    
    // 3. Check for flanking opportunities
    if (CanFlankTarget(Situation))
    {
        return ETacticalAction::Flank;
    }
    
    // 4. Check for pack coordination
    if (Situation.PackMembers.Num() > 1 && CanCoordinatePackAttack(Situation))
    {
        return ETacticalAction::PackAttack;
    }
    
    // 5. Check if we should circle/harass
    if (ShouldCircleTarget(Situation))
    {
        return ETacticalAction::Circle;
    }
    
    // 6. Check for direct attack
    if (CanDirectAttack(Situation))
    {
        return ETacticalAction::DirectAttack;
    }
    
    // 7. Default to stalking/positioning
    return ETacticalAction::Stalk;
}

bool UTacticalCombatSystem::ShouldRetreat(const FCombatSituation& Situation)
{
    // Get attacker's health percentage
    ACharacter* AttackerCharacter = Cast<ACharacter>(Situation.Attacker);
    if (!AttackerCharacter)
    {
        return false;
    }
    
    // Simple health-based retreat logic (would be enhanced with actual health component)
    float HealthPercentage = 1.0f; // Placeholder - would get from health component
    
    // Retreat if health is below threshold
    if (HealthPercentage < 0.3f)
    {
        return true;
    }
    
    // Retreat if significantly outnumbered
    int32 EnemyCount = 1; // Would count nearby enemies
    int32 AllyCount = Situation.PackMembers.Num();
    
    if (EnemyCount > AllyCount * 2)
    {
        return true;
    }
    
    return false;
}

bool UTacticalCombatSystem::CanSetupAmbush(const FCombatSituation& Situation)
{
    // Check if target is unaware and we have good cover
    bool bTargetUnaware = !IsTargetAwareOfAttacker(Situation.Target, Situation.Attacker);
    bool bHasCover = HasNearbyAmbushPositions(Situation.Attacker->GetActorLocation());
    
    return bTargetUnaware && bHasCover;
}

bool UTacticalCombatSystem::CanFlankTarget(const FCombatSituation& Situation)
{
    // Check if we have valid flanking positions
    return Situation.FlankingPositions.Num() > 0;
}

bool UTacticalCombatSystem::CanCoordinatePackAttack(const FCombatSituation& Situation)
{
    // Need at least 2 pack members for coordination
    if (Situation.PackMembers.Num() < 2)
    {
        return false;
    }
    
    // Check if pack members are in position
    for (AActor* PackMember : Situation.PackMembers)
    {
        if (!IsValid(PackMember))
        {
            continue;
        }
        
        float DistanceToTarget = FVector::Dist(PackMember->GetActorLocation(), Situation.Target->GetActorLocation());
        if (DistanceToTarget > MaxTacticalDistance)
        {
            return false;
        }
    }
    
    return true;
}

bool UTacticalCombatSystem::ShouldCircleTarget(const FCombatSituation& Situation)
{
    // Circle if target is larger/stronger and we need to wear them down
    return Situation.Distance > 200.0f && Situation.Distance < 800.0f;
}

bool UTacticalCombatSystem::CanDirectAttack(const FCombatSituation& Situation)
{
    // Can attack if in optimal range and have tactical advantage
    bool bInRange = Situation.Distance <= 300.0f;
    bool bHasAdvantage = Situation.TacticalAdvantage > 0.5f;
    
    return bInRange && bHasAdvantage;
}

float UTacticalCombatSystem::CalculateTacticalAdvantage(AActor* Attacker, AActor* Target)
{
    if (!IsValid(Attacker) || !IsValid(Target))
    {
        return 0.0f;
    }
    
    float Advantage = 0.5f; // Base neutral advantage
    
    // Factor 1: Terrain advantage
    if (HasTerrainAdvantage(Attacker, Target))
    {
        Advantage += 0.2f;
    }
    
    // Factor 2: Positioning advantage (behind or flanking)
    FVector AttackerToTarget = (Target->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal();\n    FVector TargetForward = Target->GetActorForwardVector();\n    float DotProduct = FVector::DotProduct(AttackerToTarget, TargetForward);\n    \n    if (DotProduct < -0.5f) // Behind target\n    {\n        Advantage += 0.3f;\n    }\n    else if (FMath::Abs(DotProduct) < 0.5f) // Flanking\n    {\n        Advantage += 0.15f;\n    }\n    \n    // Factor 3: Pack advantage\n    TArray<AActor*> NearbyAllies = GetNearbyAllies(Attacker, 500.0f);\n    if (NearbyAllies.Num() > 0)\n    {\n        Advantage += 0.1f * FMath::Min(NearbyAllies.Num(), 3); // Max bonus for 3 allies\n    }\n    \n    return FMath::Clamp(Advantage, 0.0f, 1.0f);\n}\n\nTArray<FVector> UTacticalCombatSystem::CalculateFlankingPositions(AActor* Attacker, AActor* Target)\n{\n    TArray<FVector> FlankingPositions;\n    \n    if (!IsValid(Attacker) || !IsValid(Target))\n    {\n        return FlankingPositions;\n    }\n    \n    FVector TargetLocation = Target->GetActorLocation();\n    FVector TargetForward = Target->GetActorForwardVector();\n    float FlankingDistance = 400.0f;\n    \n    // Calculate left and right flanking positions\n    FVector RightFlank = TargetLocation + TargetForward.RotateAngleAxis(90.0f, FVector::UpVector) * FlankingDistance;\n    FVector LeftFlank = TargetLocation + TargetForward.RotateAngleAxis(-90.0f, FVector::UpVector) * FlankingDistance;\n    \n    // Check if positions are navigable\n    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());\n    if (NavSystem)\n    {\n        FNavLocation NavLocation;\n        if (NavSystem->ProjectPointToNavigation(RightFlank, NavLocation, FVector(100.0f)))\n        {\n            FlankingPositions.Add(NavLocation.Location);\n        }\n        \n        if (NavSystem->ProjectPointToNavigation(LeftFlank, NavLocation, FVector(100.0f)))\n        {\n            FlankingPositions.Add(NavLocation.Location);\n        }\n    }\n    \n    return FlankingPositions;\n}\n\nbool UTacticalCombatSystem::HasTerrainAdvantage(AActor* Attacker, AActor* Target)\n{\n    if (!IsValid(Attacker) || !IsValid(Target))\n    {\n        return false;\n    }\n    \n    // Simple height advantage check\n    float HeightDifference = Attacker->GetActorLocation().Z - Target->GetActorLocation().Z;\n    \n    // Advantage if attacker is significantly higher\n    return HeightDifference > 100.0f;\n}\n\nTArray<AActor*> UTacticalCombatSystem::GetNearbyAllies(AActor* Actor, float Radius)\n{\n    TArray<AActor*> NearbyAllies;\n    \n    if (!IsValid(Actor) || !CombatManager)\n    {\n        return NearbyAllies;\n    }\n    \n    // This would be implemented with proper faction/team system\n    // For now, return empty array\n    \n    return NearbyAllies;\n}\n\nbool UTacticalCombatSystem::IsTargetAwareOfAttacker(AActor* Target, AActor* Attacker)\n{\n    // Simplified awareness check\n    // In a full implementation, this would check the target's perception system\n    \n    if (!IsValid(Target) || !IsValid(Attacker))\n    {\n        return false;\n    }\n    \n    // Check if target is facing attacker\n    FVector TargetToAttacker = (Attacker->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();\n    FVector TargetForward = Target->GetActorForwardVector();\n    float DotProduct = FVector::DotProduct(TargetToAttacker, TargetForward);\n    \n    // Target is aware if attacker is in front cone\n    return DotProduct > 0.3f;\n}\n\nbool UTacticalCombatSystem::HasNearbyAmbushPositions(const FVector& Location)\n{\n    // Check for cover positions within ambush radius\n    // This would use proper cover system in full implementation\n    \n    return true; // Simplified for now\n}\n\nvoid UTacticalCombatSystem::UpdatePackCoordination(FCombatSituation& Situation)\n{\n    if (Situation.PackMembers.Num() < 2)\n    {\n        return;\n    }\n    \n    // Calculate optimal pack positions\n    FVector TargetLocation = Situation.Target->GetActorLocation();\n    float PackRadius = 600.0f;\n    \n    // Distribute pack members around target\n    for (int32 i = 0; i < Situation.PackMembers.Num(); i++)\n    {\n        if (!IsValid(Situation.PackMembers[i]))\n        {\n            continue;\n        }\n        \n        float Angle = (360.0f / Situation.PackMembers.Num()) * i;\n        FVector PackPosition = TargetLocation + FVector(\n            FMath::Cos(FMath::DegreesToRadians(Angle)) * PackRadius,\n            FMath::Sin(FMath::DegreesToRadians(Angle)) * PackRadius,\n            0.0f\n        );\n        \n        // Store recommended position for this pack member\n        // This would be communicated to the individual AI controllers\n    }\n}\n\nFCombatSituation* UTacticalCombatSystem::RegisterCombatSituation(AActor* Attacker, AActor* Target)\n{\n    if (!IsValid(Attacker) || !IsValid(Target))\n    {\n        return nullptr;\n    }\n    \n    // Check if situation already exists\n    for (auto& Situation : ActiveCombatSituations)\n    {\n        if (Situation.Attacker == Attacker && Situation.Target == Target)\n        {\n            return &Situation;\n        }\n    }\n    \n    // Create new combat situation\n    FCombatSituation NewSituation;\n    NewSituation.Attacker = Attacker;\n    NewSituation.Target = Target;\n    NewSituation.StartTime = GetWorld()->GetTimeSeconds();\n    NewSituation.Distance = FVector::Dist(Attacker->GetActorLocation(), Target->GetActorLocation());\n    NewSituation.TacticalAdvantage = CalculateTacticalAdvantage(Attacker, Target);\n    NewSituation.RecommendedAction = ETacticalAction::Stalk;\n    NewSituation.bHasTerrainAdvantage = HasTerrainAdvantage(Attacker, Target);\n    \n    ActiveCombatSituations.Add(NewSituation);\n    \n    UE_LOG(LogTemp, Log, TEXT(\"Registered new combat situation: %s vs %s\"), \n           *Attacker->GetName(), *Target->GetName());\n    \n    return &ActiveCombatSituations.Last();\n}\n\nvoid UTacticalCombatSystem::UnregisterCombatSituation(AActor* Attacker, AActor* Target)\n{\n    ActiveCombatSituations.RemoveAll([Attacker, Target](const FCombatSituation& Situation)\n    {\n        return Situation.Attacker == Attacker && Situation.Target == Target;\n    });\n    \n    UE_LOG(LogTemp, Log, TEXT(\"Unregistered combat situation: %s vs %s\"), \n           *Attacker->GetName(), *Target->GetName());\n}\n\nETacticalAction UTacticalCombatSystem::GetRecommendedAction(AActor* Attacker, AActor* Target)\n{\n    for (const auto& Situation : ActiveCombatSituations)\n    {\n        if (Situation.Attacker == Attacker && Situation.Target == Target)\n        {\n            return Situation.RecommendedAction;\n        }\n    }\n    \n    return ETacticalAction::Stalk;\n}\n\nTArray<FVector> UTacticalCombatSystem::GetFlankingPositions(AActor* Attacker, AActor* Target)\n{\n    for (const auto& Situation : ActiveCombatSituations)\n    {\n        if (Situation.Attacker == Attacker && Situation.Target == Target)\n        {\n            return Situation.FlankingPositions;\n        }\n    }\n    \n    return TArray<FVector>();\n}\n\nfloat UTacticalCombatSystem::GetTacticalAdvantage(AActor* Attacker, AActor* Target)\n{\n    for (const auto& Situation : ActiveCombatSituations)\n    {\n        if (Situation.Attacker == Attacker && Situation.Target == Target)\n        {\n            return Situation.TacticalAdvantage;\n        }\n    }\n    \n    return 0.5f; // Neutral advantage\n}"