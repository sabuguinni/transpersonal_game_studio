#include "Combat_AIManager.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

ACombat_AIManager::ACombat_AIManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create detection sphere
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    RootComponent = DetectionSphere;
    DetectionSphere->SetSphereRadius(1000.0f);

    // Initialize default values
    CurrentThreatLevel = ECombat_ThreatLevel::None;
    LastThreatUpdate = 0.0f;
    ThreatUpdateInterval = 1.0f;

    // Set default tactical settings
    TacticalSettings.AttackRange = 300.0f;
    TacticalSettings.DetectionRadius = 800.0f;
    TacticalSettings.FlankingDistance = 500.0f;
    TacticalSettings.PackSize = 3;
    TacticalSettings.bCanUsePackTactics = true;
}

void ACombat_AIManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCombatZones();
    
    UE_LOG(LogTemp, Warning, TEXT("Combat AI Manager initialized with %d combat zones"), CombatZones.Num());
}

void ACombat_AIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update threat assessment periodically
    if (GetWorld()->GetTimeSeconds() - LastThreatUpdate > ThreatUpdateInterval)
    {
        UpdateThreatAssessment();
        UpdateCombatStates();
        ManagePackCoordination();
        LastThreatUpdate = GetWorld()->GetTimeSeconds();
    }
}

void ACombat_AIManager::RegisterCombatant(AActor* Combatant)
{
    if (Combatant && !RegisteredCombatants.Contains(Combatant))
    {
        RegisteredCombatants.Add(Combatant);
        UE_LOG(LogTemp, Log, TEXT("Registered combatant: %s"), *Combatant->GetName());
    }
}

void ACombat_AIManager::UnregisterCombatant(AActor* Combatant)
{
    if (Combatant)
    {
        RegisteredCombatants.Remove(Combatant);
        UE_LOG(LogTemp, Log, TEXT("Unregistered combatant: %s"), *Combatant->GetName());
    }
}

ECombat_ThreatLevel ACombat_AIManager::CalculateThreatLevel(AActor* Target)
{
    if (!Target)
    {
        return ECombat_ThreatLevel::None;
    }

    float DistanceToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    
    if (DistanceToTarget > TacticalSettings.DetectionRadius)
    {
        return ECombat_ThreatLevel::None;
    }
    else if (DistanceToTarget > TacticalSettings.AttackRange * 2.0f)
    {
        return ECombat_ThreatLevel::Low;
    }
    else if (DistanceToTarget > TacticalSettings.AttackRange)
    {
        return ECombat_ThreatLevel::Medium;
    }
    else if (DistanceToTarget > TacticalSettings.AttackRange * 0.5f)
    {
        return ECombat_ThreatLevel::High;
    }
    else
    {
        return ECombat_ThreatLevel::Critical;
    }
}

void ACombat_AIManager::InitializeCombatZones()
{
    // Find all trigger boxes in the level that could be combat zones
    TArray<AActor*> FoundTriggers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATriggerBox::StaticClass(), FoundTriggers);

    for (AActor* Actor : FoundTriggers)
    {
        ATriggerBox* TriggerBox = Cast<ATriggerBox>(Actor);
        if (TriggerBox && TriggerBox->GetName().Contains(TEXT("Combat")))
        {
            CombatZones.Add(TriggerBox);
            UE_LOG(LogTemp, Log, TEXT("Added combat zone: %s"), *TriggerBox->GetName());
        }
    }
}

TArray<FVector> ACombat_AIManager::GenerateFlankingPositions(FVector TargetLocation, int32 NumPositions)
{
    TArray<FVector> FlankingPositions;
    
    if (NumPositions <= 0)
    {
        return FlankingPositions;
    }

    float AngleStep = 360.0f / NumPositions;
    
    for (int32 i = 0; i < NumPositions; i++)
    {
        float Angle = i * AngleStep;
        float RadianAngle = FMath::DegreesToRadians(Angle);
        
        FVector FlankPosition = TargetLocation + FVector(
            FMath::Cos(RadianAngle) * TacticalSettings.FlankingDistance,
            FMath::Sin(RadianAngle) * TacticalSettings.FlankingDistance,
            0.0f
        );
        
        FlankingPositions.Add(FlankPosition);
    }
    
    return FlankingPositions;
}

void ACombat_AIManager::UpdateCombatStates()
{
    // Update combat states for all registered combatants
    for (AActor* Combatant : RegisteredCombatants)
    {
        if (!Combatant)
        {
            continue;
        }

        // Find player character for threat assessment
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            ECombat_ThreatLevel ThreatLevel = CalculateThreatLevel(PlayerPawn);
            
            // Log threat level changes
            if (ThreatLevel != CurrentThreatLevel)
            {
                CurrentThreatLevel = ThreatLevel;
                UE_LOG(LogTemp, Warning, TEXT("Threat level changed to: %d for combatant: %s"), 
                       (int32)ThreatLevel, *Combatant->GetName());
            }
        }
    }
}

void ACombat_AIManager::UpdateThreatAssessment()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        CurrentThreatLevel = ECombat_ThreatLevel::None;
        return;
    }

    ECombat_ThreatLevel NewThreatLevel = CalculateThreatLevel(PlayerPawn);
    
    if (NewThreatLevel != CurrentThreatLevel)
    {
        CurrentThreatLevel = NewThreatLevel;
        UE_LOG(LogTemp, Warning, TEXT("Global threat level updated to: %d"), (int32)CurrentThreatLevel);
    }
}

void ACombat_AIManager::ManagePackCoordination()
{
    if (!TacticalSettings.bCanUsePackTactics || RegisteredCombatants.Num() < 2)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    // Generate flanking positions for pack coordination
    TArray<FVector> FlankingPositions = GenerateFlankingPositions(
        PlayerPawn->GetActorLocation(), 
        FMath::Min(RegisteredCombatants.Num(), TacticalSettings.PackSize)
    );

    // Assign flanking positions to combatants
    for (int32 i = 0; i < FMath::Min(RegisteredCombatants.Num(), FlankingPositions.Num()); i++)
    {
        if (RegisteredCombatants[i])
        {
            UE_LOG(LogTemp, Log, TEXT("Assigning flanking position to %s: %s"), 
                   *RegisteredCombatants[i]->GetName(), 
                   *FlankingPositions[i].ToString());
        }
    }
}