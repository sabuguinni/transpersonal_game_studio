// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "CombatAISystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Engine/Engine.h"

// UCombatAIComponent Implementation
UCombatAIComponent::UCombatAIComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    CombatArchetype = ECombatArchetype::ApexPredator;
    CurrentCombatState = ECombatState::Passive;
    CurrentTactic = ECombatTactic::DirectAssault;
    
    // Combat parameters
    AggressionLevel = 0.5f;
    FearLevel = 0.3f;
    HealthThreshold = 0.3f;
    RetreatThreshold = 0.2f;
    
    // Timing
    DecisionInterval = 2.0f;
    LastDecisionTime = 0.0f;
    StateChangeTime = 0.0f;
    
    // Flags
    bIsInCombat = false;
    bCanSeeTarget = false;
    bIsWounded = false;
    bIsRetreating = false;
    
    // Initialize capabilities with default values
    CombatCapabilities = FCombatCapabilities();
    
    // Initialize threat assessment
    PrimaryThreat = FThreatAssessment();
    
    // Combat tags
    CombatTags.AddTag(FGameplayTag::RequestGameplayTag(FName(\"Combat.State.Passive\")));\n}

void UCombatAIComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get references to required components
    OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        AIController = Cast<AAIController>(OwnerPawn->GetController());
        if (AIController)
        {
            BlackboardComponent = AIController->GetBlackboardComponent();
            BehaviorTreeComponent = AIController->GetBrainComponent();
            PerceptionComponent = AIController->GetAIPerceptionComponent();
        }
    }
    
    // Initialize combat system
    InitializeCombatSystem();
    
    // Set initial blackboard values
    UpdateBlackboardValues();
    
    UE_LOG(LogTemp, Warning, TEXT(\"CombatAI Component initialized for %s\"), 
           OwnerPawn ? *OwnerPawn->GetName() : TEXT(\"Unknown\"));
}

void UCombatAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerPawn || !AIController)
        return;
    
    // Update threat assessment
    UpdateThreatAssessment(DeltaTime);
    
    // Update combat state
    UpdateCombatState(DeltaTime);
    
    // Make tactical decisions
    if (ShouldMakeDecision())
    {
        MakeTacticalDecision();
        LastDecisionTime = GetWorld()->GetTimeSeconds();
    }
    
    // Update blackboard with current state
    UpdateBlackboardValues();
}

void UCombatAIComponent::InitializeCombatSystem()
{
    // Configure combat capabilities based on archetype
    ConfigureCombatCapabilities();
    
    // Set up perception if available
    if (PerceptionComponent)
    {
        ConfigurePerception();
    }
    
    // Initialize behavior tree if available
    if (BehaviorTreeAsset && AIController)
    {
        AIController->RunBehaviorTree(BehaviorTreeAsset);
    }
}

void UCombatAIComponent::ConfigureCombatCapabilities()
{
    switch (CombatArchetype)
    {
        case ECombatArchetype::ApexPredator:\n            CombatCapabilities.AttackDamage = 100.0f;\n            CombatCapabilities.AttackRange = 300.0f;\n            CombatCapabilities.MaxSpeed = 800.0f;\n            CombatCapabilities.SightRange = 3000.0f;\n            CombatCapabilities.CanCoordinate = false;\n            break;\n            \n        case ECombatArchetype::PackHunter:\n            CombatCapabilities.AttackDamage = 60.0f;\n            CombatCapabilities.AttackRange = 200.0f;\n            CombatCapabilities.MaxSpeed = 1000.0f;\n            CombatCapabilities.SightRange = 2500.0f;\n            CombatCapabilities.CanCoordinate = true;\n            break;\n            \n        case ECombatArchetype::AmbushPredator:\n            CombatCapabilities.AttackDamage = 80.0f;\n            CombatCapabilities.AttackRange = 250.0f;\n            CombatCapabilities.MaxSpeed = 600.0f;\n            CombatCapabilities.SightRange = 2000.0f;\n            CombatCapabilities.CanCoordinate = false;\n            break;\n            \n        case ECombatArchetype::TankDefender:\n            CombatCapabilities.AttackDamage = 70.0f;\n            CombatCapabilities.AttackRange = 150.0f;\n            CombatCapabilities.MaxSpeed = 400.0f;\n            CombatCapabilities.SightRange = 1500.0f;\n            CombatCapabilities.KnockbackForce = 1000.0f;\n            break;\n            \n        case ECombatArchetype::FlightResponse:\n            CombatCapabilities.AttackDamage = 20.0f;\n            CombatCapabilities.AttackRange = 100.0f;\n            CombatCapabilities.MaxSpeed = 1200.0f;\n            CombatCapabilities.SightRange = 2500.0f;\n            break;\n            \n        default:\n            // Use default capabilities\n            break;\n    }\n}

void UCombatAIComponent::ConfigurePerception()
{
    if (!PerceptionComponent)\n        return;\n        \n    // Configure sight sense\n    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT(\"SightConfig\"));\n    if (SightConfig)\n    {\n        SightConfig->SightRadius = CombatCapabilities.SightRange;\n        SightConfig->LoseSightRadius = CombatCapabilities.SightRange + 200.0f;\n        SightConfig->PeripheralVisionAngleDegrees = CombatCapabilities.SightAngle;\n        SightConfig->SetMaxAge(5.0f);\n        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;\n        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;\n        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;\n        SightConfig->DetectionByAffiliation.bDetectEnemies = true;\n        \n        PerceptionComponent->ConfigureSense(*SightConfig);\n    }\n    \n    // Configure hearing sense\n    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT(\"HearingConfig\"));\n    if (HearingConfig)\n    {\n        HearingConfig->HearingRange = CombatCapabilities.HearingRange;\n        HearingConfig->SetMaxAge(3.0f);\n        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;\n        HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;\n        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;\n        \n        PerceptionComponent->ConfigureSense(*HearingConfig);\n    }\n    \n    // Set dominant sense\n    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());\n    \n    // Bind perception events\n    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &UCombatAIComponent::OnPerceptionUpdated);\n}

void UCombatAIComponent::UpdateThreatAssessment(float DeltaTime)
{
    if (!PerceptionComponent)\n        return;\n        \n    TArray<AActor*> PerceivedActors;\n    PerceptionComponent->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);\n    \n    AActor* HighestThreat = nullptr;\n    float HighestThreatScore = 0.0f;\n    \n    for (AActor* Actor : PerceivedActors)\n    {\n        if (!Actor || Actor == OwnerPawn)\n            continue;\n            \n        float ThreatScore = CalculateThreatScore(Actor);\n        if (ThreatScore > HighestThreatScore)\n        {\n            HighestThreatScore = ThreatScore;\n            HighestThreat = Actor;\n        }\n    }\n    \n    // Update primary threat\n    if (HighestThreat)\n    {\n        UpdatePrimaryThreat(HighestThreat, HighestThreatScore);\n    }\n    else\n    {\n        // No threats detected\n        PrimaryThreat.ThreatActor = nullptr;\n        PrimaryThreat.ThreatLevel = EThreatLevel::None;\n        PrimaryThreat.ThreatScore = 0.0f;\n    }\n}

float UCombatAIComponent::CalculateThreatScore(AActor* Actor)
{
    if (!Actor || !OwnerPawn)\n        return 0.0f;\n        \n    float ThreatScore = 0.0f;\n    \n    // Distance factor (closer = more threatening)\n    float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), Actor->GetActorLocation());\n    float DistanceFactor = FMath::Clamp(1.0f - (Distance / CombatCapabilities.SightRange), 0.0f, 1.0f);\n    ThreatScore += DistanceFactor * 30.0f;\n    \n    // Size factor (larger = more threatening)\n    if (APawn* TargetPawn = Cast<APawn>(Actor))\n    {\n        FVector TargetScale = TargetPawn->GetActorScale3D();\n        FVector OwnerScale = OwnerPawn->GetActorScale3D();\n        float SizeFactor = (TargetScale.X + TargetScale.Y + TargetScale.Z) / (OwnerScale.X + OwnerScale.Y + OwnerScale.Z);\n        ThreatScore += FMath::Clamp(SizeFactor, 0.0f, 2.0f) * 20.0f;\n    }\n    \n    // Movement factor (moving towards = more threatening)\n    FVector ToTarget = (Actor->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal();\n    FVector TargetVelocity = Actor->GetVelocity().GetSafeNormal();\n    float MovementDot = FVector::DotProduct(TargetVelocity, ToTarget);\n    if (MovementDot > 0.0f)\n    {\n        ThreatScore += MovementDot * 25.0f;\n    }\n    \n    // Player factor (player is always high threat)\n    if (Cast<APawn>(Actor) && Cast<APawn>(Actor)->IsPlayerControlled())\n    {\n        ThreatScore += 50.0f;\n    }\n    \n    return ThreatScore;\n}

void UCombatAIComponent::UpdatePrimaryThreat(AActor* ThreatActor, float ThreatScore)
{
    PrimaryThreat.ThreatActor = ThreatActor;\n    PrimaryThreat.ThreatScore = ThreatScore;\n    PrimaryThreat.LastKnownLocation = ThreatActor->GetActorLocation();\n    PrimaryThreat.LastSeenTime = GetWorld()->GetTimeSeconds();\n    PrimaryThreat.DistanceToThreat = FVector::Dist(OwnerPawn->GetActorLocation(), ThreatActor->GetActorLocation());\n    \n    // Determine threat level based on score\n    if (ThreatScore >= 80.0f)\n        PrimaryThreat.ThreatLevel = EThreatLevel::Lethal;\n    else if (ThreatScore >= 60.0f)\n        PrimaryThreat.ThreatLevel = EThreatLevel::Extreme;\n    else if (ThreatScore >= 40.0f)\n        PrimaryThreat.ThreatLevel = EThreatLevel::High;\n    else if (ThreatScore >= 25.0f)\n        PrimaryThreat.ThreatLevel = EThreatLevel::Moderate;\n    else if (ThreatScore >= 15.0f)\n        PrimaryThreat.ThreatLevel = EThreatLevel::Low;\n    else\n        PrimaryThreat.ThreatLevel = EThreatLevel::Minimal;\n        \n    // Check if threat is visible\n    FHitResult HitResult;\n    FVector Start = OwnerPawn->GetActorLocation() + FVector(0, 0, 50);\n    FVector End = ThreatActor->GetActorLocation() + FVector(0, 0, 50);\n    \n    PrimaryThreat.bIsVisible = !GetWorld()->LineTraceSingleByChannel(\n        HitResult, Start, End, ECC_Visibility);\n        \n    // Check if threat is moving\n    FVector Velocity = ThreatActor->GetVelocity();\n    PrimaryThreat.bIsMoving = Velocity.Size() > 10.0f;\n    PrimaryThreat.MovementDirection = Velocity.GetSafeNormal();\n}

void UCombatAIComponent::UpdateCombatState(float DeltaTime)
{
    ECombatState NewState = DetermineCombatState();\n    \n    if (NewState != CurrentCombatState)\n    {\n        OnCombatStateChanged(CurrentCombatState, NewState);\n        CurrentCombatState = NewState;\n        StateChangeTime = GetWorld()->GetTimeSeconds();\n        \n        // Update combat tags\n        UpdateCombatTags();\n    }\n}

ECombatState UCombatAIComponent::DetermineCombatState()
{
    // Check for retreat conditions\n    if (ShouldRetreat())\n    {\n        return ECombatState::Retreating;\n    }\n    \n    // Check for wounded state\n    if (IsWounded())\n    {\n        return ECombatState::Wounded;\n    }\n    \n    // State transitions based on threat level\n    if (PrimaryThreat.ThreatActor)\n    {\n        switch (PrimaryThreat.ThreatLevel)\n        {\n            case EThreatLevel::Lethal:\n            case EThreatLevel::Extreme:\n                if (PrimaryThreat.DistanceToThreat < CombatCapabilities.AttackRange)\n                    return ECombatState::Attacking;\n                else\n                    return ECombatState::Engaging;\n                    \n            case EThreatLevel::High:\n                if (PrimaryThreat.bIsVisible)\n                    return ECombatState::Engaging;\n                else\n                    return ECombatState::Stalking;\n                    \n            case EThreatLevel::Moderate:\n                return ECombatState::Alerted;\n                \n            case EThreatLevel::Low:\n            case EThreatLevel::Minimal:\n                return ECombatState::Investigating;\n                \n            default:\n                break;\n        }\n    }\n    \n    return ECombatState::Passive;\n}

bool UCombatAIComponent::ShouldRetreat()
{
    // Check health threshold\n    if (OwnerPawn)\n    {\n        // Assuming health component exists\n        float CurrentHealth = 100.0f; // TODO: Get from health component\n        float MaxHealth = 100.0f;\n        float HealthRatio = CurrentHealth / MaxHealth;\n        \n        if (HealthRatio < RetreatThreshold)\n            return true;\n    }\n    \n    // Check if overwhelmed\n    if (PrimaryThreat.ThreatLevel >= EThreatLevel::Extreme && \n        PrimaryThreat.DistanceToThreat < CombatCapabilities.AttackRange * 0.5f)\n    {\n        return true;\n    }\n    \n    return false;\n}

bool UCombatAIComponent::IsWounded()
{
    if (OwnerPawn)\n    {\n        // TODO: Implement health check\n        float CurrentHealth = 100.0f; // Get from health component\n        float MaxHealth = 100.0f;\n        float HealthRatio = CurrentHealth / MaxHealth;\n        \n        return HealthRatio < HealthThreshold;\n    }\n    \n    return false;\n}

void UCombatAIComponent::MakeTacticalDecision()
{
    ECombatTactic NewTactic = DetermineBestTactic();\n    \n    if (NewTactic != CurrentTactic)\n    {\n        CurrentTactic = NewTactic;\n        OnTacticChanged(NewTactic);\n    }\n}

ECombatTactic UCombatAIComponent::DetermineBestTactic()
{
    if (!PrimaryThreat.ThreatActor)\n        return ECombatTactic::StandGround;\n        \n    // Tactic selection based on archetype and situation\n    switch (CombatArchetype)\n    {\n        case ECombatArchetype::ApexPredator:\n            return DetermineApexPredatorTactic();\n            \n        case ECombatArchetype::PackHunter:\n            return DeterminePackHunterTactic();\n            \n        case ECombatArchetype::AmbushPredator:\n            return DetermineAmbushPredatorTactic();\n            \n        case ECombatArchetype::TankDefender:\n            return DetermineTankDefenderTactic();\n            \n        case ECombatArchetype::FlightResponse:\n            return DetermineFlightResponseTactic();\n            \n        default:\n            return ECombatTactic::DirectAssault;\n    }\n}

ECombatTactic UCombatAIComponent::DetermineApexPredatorTactic()
{
    if (PrimaryThreat.DistanceToThreat > CombatCapabilities.AttackRange * 2.0f)\n        return ECombatTactic::ChargeAttack;\n    else if (PrimaryThreat.DistanceToThreat < CombatCapabilities.AttackRange)\n        return ECombatTactic::DirectAssault;\n    else\n        return ECombatTactic::CircleAndStrike;\n}

ECombatTactic UCombatAIComponent::DeterminePackHunterTactic()
{
    // TODO: Check for nearby pack members\n    bool HasPackSupport = false; // Implement pack detection\n    \n    if (HasPackSupport)\n        return ECombatTactic::CoordinatedAttack;\n    else if (PrimaryThreat.bIsVisible)\n        return ECombatTactic::CircleAndStrike;\n    else\n        return ECombatTactic::AmbushStrike;\n}

ECombatTactic UCombatAIComponent::DetermineAmbushPredatorTactic()
{
    if (!PrimaryThreat.bIsVisible)\n        return ECombatTactic::AmbushStrike;\n    else if (PrimaryThreat.DistanceToThreat < CombatCapabilities.AttackRange)\n        return ECombatTactic::PounceAttack;\n    else\n        return ECombatTactic::UseEnvironment;\n}

ECombatTactic UCombatAIComponent::DetermineTankDefenderTactic()
{
    if (PrimaryThreat.DistanceToThreat < CombatCapabilities.AttackRange * 1.5f)\n        return ECombatTactic::StandGround;\n    else\n        return ECombatTactic::ChargeAttack;\n}

ECombatTactic UCombatAIComponent::DetermineFlightResponseTactic()
{
    if (PrimaryThreat.ThreatLevel >= EThreatLevel::High)\n        return ECombatTactic::ZigzagEscape;\n    else\n        return ECombatTactic::CreateDistance;\n}

bool UCombatAIComponent::ShouldMakeDecision()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();\n    return (CurrentTime - LastDecisionTime) >= DecisionInterval;\n}

void UCombatAIComponent::UpdateBlackboardValues()
{
    if (!BlackboardComponent)\n        return;\n        \n    // Update combat state\n    BlackboardComponent->SetValueAsEnum(TEXT(\"CombatState\"), (uint8)CurrentCombatState);\n    BlackboardComponent->SetValueAsEnum(TEXT(\"CombatTactic\"), (uint8)CurrentTactic);\n    \n    // Update threat information\n    if (PrimaryThreat.ThreatActor)\n    {\n        BlackboardComponent->SetValueAsObject(TEXT(\"TargetActor\"), PrimaryThreat.ThreatActor);\n        BlackboardComponent->SetValueAsVector(TEXT(\"TargetLocation\"), PrimaryThreat.LastKnownLocation);\n        BlackboardComponent->SetValueAsFloat(TEXT(\"ThreatLevel\"), (float)PrimaryThreat.ThreatLevel);\n        BlackboardComponent->SetValueAsBool(TEXT(\"CanSeeTarget\"), PrimaryThreat.bIsVisible);\n    }\n    else\n    {\n        BlackboardComponent->ClearValue(TEXT(\"TargetActor\"));\n        BlackboardComponent->ClearValue(TEXT(\"TargetLocation\"));\n        BlackboardComponent->SetValueAsFloat(TEXT(\"ThreatLevel\"), 0.0f);\n        BlackboardComponent->SetValueAsBool(TEXT(\"CanSeeTarget\"), false);\n    }\n    \n    // Update flags\n    BlackboardComponent->SetValueAsBool(TEXT(\"IsInCombat\"), bIsInCombat);\n    BlackboardComponent->SetValueAsBool(TEXT(\"IsWounded\"), bIsWounded);\n    BlackboardComponent->SetValueAsBool(TEXT(\"IsRetreating\"), bIsRetreating);\n}

void UCombatAIComponent::UpdateCombatTags()
{
    // Clear existing combat state tags\n    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag(FName(\"Combat.State.Passive\")));\n    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag(FName(\"Combat.State.Alerted\")));\n    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag(FName(\"Combat.State.Engaging\")));\n    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag(FName(\"Combat.State.Attacking\")));\n    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag(FName(\"Combat.State.Retreating\")));\n    \n    // Add current state tag\n    switch (CurrentCombatState)\n    {\n        case ECombatState::Passive:\n            CombatTags.AddTag(FGameplayTag::RequestGameplayTag(FName(\"Combat.State.Passive\")));\n            break;\n        case ECombatState::Alerted:\n            CombatTags.AddTag(FGameplayTag::RequestGameplayTag(FName(\"Combat.State.Alerted\")));\n            break;\n        case ECombatState::Engaging:\n            CombatTags.AddTag(FGameplayTag::RequestGameplayTag(FName(\"Combat.State.Engaging\")));\n            break;\n        case ECombatState::Attacking:\n            CombatTags.AddTag(FGameplayTag::RequestGameplayTag(FName(\"Combat.State.Attacking\")));\n            break;\n        case ECombatState::Retreating:\n            CombatTags.AddTag(FGameplayTag::RequestGameplayTag(FName(\"Combat.State.Retreating\")));\n            break;\n        default:\n            break;\n    }\n}

void UCombatAIComponent::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)\n        return;\n        \n    if (Stimulus.WasSuccessfullySensed())\n    {\n        // Actor was detected\n        UE_LOG(LogTemp, Log, TEXT(\"CombatAI: Detected %s\"), *Actor->GetName());\n        \n        // Update threat assessment immediately\n        float ThreatScore = CalculateThreatScore(Actor);\n        if (ThreatScore > PrimaryThreat.ThreatScore)\n        {\n            UpdatePrimaryThreat(Actor, ThreatScore);\n        }\n    }\n    else\n    {\n        // Actor was lost\n        UE_LOG(LogTemp, Log, TEXT(\"CombatAI: Lost sight of %s\"), *Actor->GetName());\n        \n        if (PrimaryThreat.ThreatActor == Actor)\n        {\n            // Lost primary threat\n            PrimaryThreat.bIsVisible = false;\n        }\n    }\n}

void UCombatAIComponent::OnCombatStateChanged(ECombatState OldState, ECombatState NewState)
{
    UE_LOG(LogTemp, Warning, TEXT(\"CombatAI: State changed from %d to %d\"), (int32)OldState, (int32)NewState);\n    \n    // Update combat flag\n    bIsInCombat = (NewState == ECombatState::Engaging || \n                   NewState == ECombatState::Attacking || \n                   NewState == ECombatState::Defending);\n                   \n    // Update other flags\n    bIsRetreating = (NewState == ECombatState::Retreating);\n    bIsWounded = (NewState == ECombatState::Wounded);\n    \n    // Broadcast state change\n    OnCombatStateChangedDelegate.Broadcast(OldState, NewState);\n}

void UCombatAIComponent::OnTacticChanged(ECombatTactic NewTactic)
{
    UE_LOG(LogTemp, Log, TEXT(\"CombatAI: Tactic changed to %d\"), (int32)NewTactic);\n    \n    // Broadcast tactic change\n    OnTacticChangedDelegate.Broadcast(NewTactic);\n}

// Blueprint callable functions
void UCombatAIComponent::SetCombatArchetype(ECombatArchetype NewArchetype)
{
    CombatArchetype = NewArchetype;\n    ConfigureCombatCapabilities();\n}

void UCombatAIComponent::SetAggressionLevel(float NewAggression)
{
    AggressionLevel = FMath::Clamp(NewAggression, 0.0f, 1.0f);\n}

void UCombatAIComponent::SetFearLevel(float NewFear)
{
    FearLevel = FMath::Clamp(NewFear, 0.0f, 1.0f);\n}

void UCombatAIComponent::ForceCombatState(ECombatState NewState)
{
    if (NewState != CurrentCombatState)\n    {\n        OnCombatStateChanged(CurrentCombatState, NewState);\n        CurrentCombatState = NewState;\n        StateChangeTime = GetWorld()->GetTimeSeconds();\n        UpdateCombatTags();\n    }\n}

void UCombatAIComponent::AddThreatActor(AActor* Actor, float ThreatScore)
{
    if (Actor && ThreatScore > 0.0f)\n    {\n        if (ThreatScore > PrimaryThreat.ThreatScore)\n        {\n            UpdatePrimaryThreat(Actor, ThreatScore);\n        }\n    }\n}

void UCombatAIComponent::RemoveThreatActor(AActor* Actor)
{
    if (PrimaryThreat.ThreatActor == Actor)\n    {\n        PrimaryThreat.ThreatActor = nullptr;\n        PrimaryThreat.ThreatLevel = EThreatLevel::None;\n        PrimaryThreat.ThreatScore = 0.0f;\n    }\n}

bool UCombatAIComponent::IsInCombat() const
{
    return bIsInCombat;\n}

ECombatState UCombatAIComponent::GetCurrentCombatState() const
{
    return CurrentCombatState;\n}

ECombatTactic UCombatAIComponent::GetCurrentTactic() const
{
    return CurrentTactic;\n}

FThreatAssessment UCombatAIComponent::GetPrimaryThreat() const
{
    return PrimaryThreat;\n}

FCombatCapabilities UCombatAIComponent::GetCombatCapabilities() const
{
    return CombatCapabilities;\n}