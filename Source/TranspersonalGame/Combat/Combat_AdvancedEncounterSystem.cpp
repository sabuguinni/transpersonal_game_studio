#include "Combat_AdvancedEncounterSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "TranspersonalGame/Shared/SharedTypes.h"

ACombat_AdvancedEncounterSystem::ACombat_AdvancedEncounterSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create detection sphere
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    RootComponent = DetectionSphere;
    DetectionSphere->SetSphereRadius(2000.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    // Initialize encounter settings
    MaxActiveEncounters = 3;
    EncounterCooldownTime = 30.0f;
    ThreatEscalationRate = 0.1f;
    BaseThreatLevel = 0.3f;
    CurrentThreatLevel = BaseThreatLevel;
    
    // Initialize encounter types
    InitializeEncounterTypes();
    
    bIsEncounterActive = false;
    LastEncounterTime = 0.0f;
}

void ACombat_AdvancedEncounterSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind overlap events
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACombat_AdvancedEncounterSystem::OnPlayerEnterZone);
    DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ACombat_AdvancedEncounterSystem::OnPlayerExitZone);
    
    // Start threat monitoring
    GetWorldTimerManager().SetTimer(
        ThreatMonitoringTimer,
        this,
        &ACombat_AdvancedEncounterSystem::UpdateThreatLevel,
        1.0f,
        true
    );
    
    UE_LOG(LogTemp, Warning, TEXT("Advanced Encounter System initialized"));
}

void ACombat_AdvancedEncounterSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsEncounterActive)
    {
        UpdateActiveEncounters(DeltaTime);
        MonitorPlayerBehavior(DeltaTime);
    }
}

void ACombat_AdvancedEncounterSystem::InitializeEncounterTypes()
{
    // Raptor Pack Ambush
    FCombat_EncounterData RaptorAmbush;
    RaptorAmbush.EncounterType = ECombat_EncounterType::PackAmbush;
    RaptorAmbush.RequiredThreatLevel = 0.4f;
    RaptorAmbush.EnemyCount = 3;
    RaptorAmbush.SpawnDistance = 1500.0f;
    RaptorAmbush.EncounterDuration = 120.0f;
    RaptorAmbush.RewardMultiplier = 1.5f;
    EncounterTypes.Add(RaptorAmbush);
    
    // Territorial Dispute
    FCombat_EncounterData TerritorialFight;
    TerritorialFight.EncounterType = ECombat_EncounterType::TerritorialDispute;
    TerritorialFight.RequiredThreatLevel = 0.6f;
    TerritorialFight.EnemyCount = 2;
    TerritorialFight.SpawnDistance = 2000.0f;
    TerritorialFight.EncounterDuration = 180.0f;
    TerritorialFight.RewardMultiplier = 2.0f;
    EncounterTypes.Add(TerritorialFight);
    
    // Alpha Predator Hunt
    FCombat_EncounterData AlphaHunt;
    AlphaHunt.EncounterType = ECombat_EncounterType::AlphaPredatorHunt;
    AlphaHunt.RequiredThreatLevel = 0.8f;
    AlphaHunt.EnemyCount = 1;
    AlphaHunt.SpawnDistance = 2500.0f;
    AlphaHunt.EncounterDuration = 240.0f;
    AlphaHunt.RewardMultiplier = 3.0f;
    EncounterTypes.Add(AlphaHunt);
}

void ACombat_AdvancedEncounterSystem::OnPlayerEnterZone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    APawn* PlayerPawn = Cast<APawn>(OtherActor);
    if (PlayerPawn && PlayerPawn->IsPlayerControlled())
    {
        CurrentPlayer = PlayerPawn;
        EvaluateEncounterTrigger();
    }
}

void ACombat_AdvancedEncounterSystem::OnPlayerExitZone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    APawn* PlayerPawn = Cast<APawn>(OtherActor);
    if (PlayerPawn && PlayerPawn == CurrentPlayer)
    {
        CurrentPlayer = nullptr;
        if (bIsEncounterActive)
        {
            EndCurrentEncounter();
        }
    }
}

void ACombat_AdvancedEncounterSystem::EvaluateEncounterTrigger()
{
    if (bIsEncounterActive || !CurrentPlayer)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastEncounterTime < EncounterCooldownTime)
    {
        return;
    }
    
    // Find suitable encounter based on threat level
    for (const FCombat_EncounterData& EncounterData : EncounterTypes)
    {
        if (CurrentThreatLevel >= EncounterData.RequiredThreatLevel)
        {
            float TriggerChance = CalculateEncounterProbability(EncounterData);
            if (FMath::RandRange(0.0f, 1.0f) <= TriggerChance)
            {
                StartEncounter(EncounterData);
                break;
            }
        }
    }
}

float ACombat_AdvancedEncounterSystem::CalculateEncounterProbability(const FCombat_EncounterData& EncounterData)
{
    float BaseProbability = 0.3f;
    float ThreatModifier = (CurrentThreatLevel - EncounterData.RequiredThreatLevel) * 2.0f;
    float TimeModifier = FMath::Min((GetWorld()->GetTimeSeconds() - LastEncounterTime) / EncounterCooldownTime, 2.0f) * 0.2f;
    
    return FMath::Clamp(BaseProbability + ThreatModifier + TimeModifier, 0.1f, 0.8f);
}

void ACombat_AdvancedEncounterSystem::StartEncounter(const FCombat_EncounterData& EncounterData)
{
    if (!CurrentPlayer)
    {
        return;
    }
    
    bIsEncounterActive = true;
    CurrentEncounter = EncounterData;
    LastEncounterTime = GetWorld()->GetTimeSeconds();
    
    // Spawn enemies based on encounter type
    SpawnEncounterEnemies(EncounterData);
    
    // Set encounter timer
    GetWorldTimerManager().SetTimer(
        EncounterTimer,
        this,
        &ACombat_AdvancedEncounterSystem::EndCurrentEncounter,
        EncounterData.EncounterDuration,
        false
    );
    
    // Increase threat level
    CurrentThreatLevel = FMath::Min(CurrentThreatLevel + ThreatEscalationRate, 1.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Started encounter: %s with %d enemies"), 
           *UEnum::GetValueAsString(EncounterData.EncounterType), 
           EncounterData.EnemyCount);
}

void ACombat_AdvancedEncounterSystem::SpawnEncounterEnemies(const FCombat_EncounterData& EncounterData)
{
    if (!CurrentPlayer)
    {
        return;
    }
    
    FVector PlayerLocation = CurrentPlayer->GetActorLocation();
    
    for (int32 i = 0; i < EncounterData.EnemyCount; i++)
    {
        // Calculate spawn position around player
        float Angle = (360.0f / EncounterData.EnemyCount) * i;
        FVector SpawnOffset = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * EncounterData.SpawnDistance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * EncounterData.SpawnDistance,
            100.0f
        );
        
        FVector SpawnLocation = PlayerLocation + SpawnOffset;
        
        // Try to spawn enemy (placeholder - would use actual dinosaur classes)
        UClass* EnemyClass = GetEnemyClassForEncounter(EncounterData.EncounterType);
        if (EnemyClass)
        {
            AActor* SpawnedEnemy = GetWorld()->SpawnActor<AActor>(
                EnemyClass,
                SpawnLocation,
                FRotator::ZeroRotator
            );
            
            if (SpawnedEnemy)
            {
                ActiveEnemies.Add(SpawnedEnemy);
                UE_LOG(LogTemp, Log, TEXT("Spawned enemy %d at location %s"), i, *SpawnLocation.ToString());
            }
        }
    }
}

UClass* ACombat_AdvancedEncounterSystem::GetEnemyClassForEncounter(ECombat_EncounterType EncounterType)
{
    // Return appropriate enemy class based on encounter type
    // For now, return base Actor class as placeholder
    return AActor::StaticClass();
}

void ACombat_AdvancedEncounterSystem::UpdateActiveEncounters(float DeltaTime)
{
    // Clean up destroyed enemies
    ActiveEnemies.RemoveAll([](AActor* Enemy) {
        return !IsValid(Enemy);
    });
    
    // Check if all enemies are defeated
    if (ActiveEnemies.Num() == 0 && bIsEncounterActive)
    {
        EndCurrentEncounter();
    }
}

void ACombat_AdvancedEncounterSystem::MonitorPlayerBehavior(float DeltaTime)
{
    if (!CurrentPlayer)
    {
        return;
    }
    
    // Monitor player actions and adjust threat accordingly
    FVector PlayerVelocity = CurrentPlayer->GetVelocity();
    float Speed = PlayerVelocity.Size();
    
    // Running increases threat slightly
    if (Speed > 600.0f)
    {
        CurrentThreatLevel += ThreatEscalationRate * 0.1f * DeltaTime;
    }
    
    // Clamp threat level
    CurrentThreatLevel = FMath::Clamp(CurrentThreatLevel, 0.0f, 1.0f);
}

void ACombat_AdvancedEncounterSystem::EndCurrentEncounter()
{
    if (!bIsEncounterActive)
    {
        return;
    }
    
    bIsEncounterActive = false;
    
    // Clean up remaining enemies
    for (AActor* Enemy : ActiveEnemies)
    {
        if (IsValid(Enemy))
        {
            Enemy->Destroy();
        }
    }
    ActiveEnemies.Empty();
    
    // Clear timers
    GetWorldTimerManager().ClearTimer(EncounterTimer);
    
    // Reduce threat level after encounter
    CurrentThreatLevel = FMath::Max(CurrentThreatLevel - (ThreatEscalationRate * 2.0f), BaseThreatLevel);
    
    UE_LOG(LogTemp, Warning, TEXT("Encounter ended. New threat level: %f"), CurrentThreatLevel);
}

void ACombat_AdvancedEncounterSystem::UpdateThreatLevel()
{
    if (!bIsEncounterActive)
    {
        // Gradually reduce threat when not in combat
        CurrentThreatLevel = FMath::Max(CurrentThreatLevel - (ThreatEscalationRate * 0.5f), BaseThreatLevel);
    }
}

float ACombat_AdvancedEncounterSystem::GetCurrentThreatLevel() const
{
    return CurrentThreatLevel;
}

bool ACombat_AdvancedEncounterSystem::IsEncounterActive() const
{
    return bIsEncounterActive;
}

int32 ACombat_AdvancedEncounterSystem::GetActiveEnemyCount() const
{
    return ActiveEnemies.Num();
}