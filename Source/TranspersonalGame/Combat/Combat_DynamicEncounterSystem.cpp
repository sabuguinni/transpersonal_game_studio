#include "Combat_DynamicEncounterSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"

UCombat_DynamicEncounterComponent::UCombat_DynamicEncounterComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    EncounterState = ECombat_EncounterState::Inactive;
    EncounterTimer = 0.0f;
    bAutoEscalate = true;
    PlayerProximityThreshold = 500.0f;
    
    // Default escalation rules
    FCombat_EscalationRule DefaultRule;
    DefaultRule.TriggerTime = 30.0f;
    DefaultRule.ThreatIncrease = 1;
    DefaultRule.bSpawnReinforcements = true;
    DefaultRule.ReinforcementCount = 1;
    EscalationRules.Add(DefaultRule);
    
    FCombat_EscalationRule SecondRule;
    SecondRule.TriggerTime = 60.0f;
    SecondRule.ThreatIncrease = 2;
    SecondRule.bSpawnReinforcements = true;
    SecondRule.ReinforcementCount = 2;
    EscalationRules.Add(SecondRule);
}

void UCombat_DynamicEncounterComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (GetWorld())
    {
        UE_LOG(LogTemp, Log, TEXT("Combat Dynamic Encounter Component initialized"));
    }
}

void UCombat_DynamicEncounterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (EncounterState != ECombat_EncounterState::Inactive)
    {
        UpdateEncounterState(DeltaTime);
    }
}

void UCombat_DynamicEncounterComponent::StartEncounter(ECombat_EncounterType Type, int32 InitialThreatLevel)
{
    if (EncounterState != ECombat_EncounterState::Inactive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start encounter - already active"));
        return;
    }
    
    CurrentEncounter.EncounterType = Type;
    CurrentEncounter.ThreatLevel = InitialThreatLevel;
    CurrentEncounter.EncounterCenter = GetOwner()->GetActorLocation();
    
    EncounterState = ECombat_EncounterState::Preparing;
    EncounterTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Starting encounter of type %d with threat level %d"), 
           (int32)Type, InitialThreatLevel);
    
    // Transition to active after brief preparation
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        [this]() { EncounterState = ECombat_EncounterState::Active; },
        2.0f,
        false
    );
}

void UCombat_DynamicEncounterComponent::EscalateEncounter()
{
    if (EncounterState != ECombat_EncounterState::Active)
    {
        return;
    }
    
    CurrentEncounter.ThreatLevel = FMath::Min(CurrentEncounter.ThreatLevel + 1, 5);
    EncounterState = ECombat_EncounterState::Escalating;
    
    UE_LOG(LogTemp, Log, TEXT("Escalating encounter to threat level %d"), CurrentEncounter.ThreatLevel);
    
    // Return to active state after escalation
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        [this]() { EncounterState = ECombat_EncounterState::Active; },
        3.0f,
        false
    );
}

void UCombat_DynamicEncounterComponent::EndEncounter()
{
    EncounterState = ECombat_EncounterState::Resolving;
    
    UE_LOG(LogTemp, Log, TEXT("Ending encounter"));
    
    // Clear participants
    EncounterParticipants.Empty();
    
    // Transition to completed then inactive
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        [this]() { 
            EncounterState = ECombat_EncounterState::Completed;
            GetWorld()->GetTimerManager().SetTimer(
                FTimerHandle(),
                [this]() { EncounterState = ECombat_EncounterState::Inactive; },
                2.0f,
                false
            );
        },
        1.0f,
        false
    );
}

void UCombat_DynamicEncounterComponent::AddParticipant(AActor* Participant)
{
    if (Participant && !EncounterParticipants.Contains(Participant))
    {
        EncounterParticipants.Add(Participant);
        CurrentEncounter.ParticipantCount = EncounterParticipants.Num();
        
        UE_LOG(LogTemp, Log, TEXT("Added participant to encounter: %s"), *Participant->GetName());
    }
}

void UCombat_DynamicEncounterComponent::RemoveParticipant(AActor* Participant)
{
    if (EncounterParticipants.Remove(Participant) > 0)
    {
        CurrentEncounter.ParticipantCount = EncounterParticipants.Num();
        
        UE_LOG(LogTemp, Log, TEXT("Removed participant from encounter: %s"), *Participant->GetName());
        
        // End encounter if no participants remain
        if (EncounterParticipants.Num() == 0)
        {
            EndEncounter();
        }
    }
}

bool UCombat_DynamicEncounterComponent::IsPlayerInEncounterRange() const
{
    AActor* Player = GetPlayerActor();
    if (!Player)
    {
        return false;
    }
    
    float Distance = FVector::Dist(Player->GetActorLocation(), CurrentEncounter.EncounterCenter);
    return Distance <= CurrentEncounter.EncounterRadius;
}

float UCombat_DynamicEncounterComponent::GetEncounterIntensity() const
{
    if (EncounterState == ECombat_EncounterState::Inactive)
    {
        return 0.0f;
    }
    
    float BaseIntensity = CurrentEncounter.ThreatLevel * 0.2f;
    float TimeMultiplier = FMath::Min(EncounterTimer / 60.0f, 1.0f);
    float ParticipantMultiplier = FMath::Min(CurrentEncounter.ParticipantCount * 0.1f, 0.5f);
    
    return FMath::Clamp(BaseIntensity + TimeMultiplier + ParticipantMultiplier, 0.0f, 1.0f);
}

void UCombat_DynamicEncounterComponent::UpdateEncounterState(float DeltaTime)
{
    EncounterTimer += DeltaTime;
    
    if (EncounterState == ECombat_EncounterState::Active && bAutoEscalate)
    {
        CheckEscalationRules();
    }
    
    // Auto-end encounter after duration
    if (EncounterTimer >= CurrentEncounter.Duration)
    {
        EndEncounter();
    }
}

void UCombat_DynamicEncounterComponent::CheckEscalationRules()
{
    for (const FCombat_EscalationRule& Rule : EscalationRules)
    {
        if (EncounterTimer >= Rule.TriggerTime && CurrentEncounter.ThreatLevel < 5)
        {
            CurrentEncounter.ThreatLevel += Rule.ThreatIncrease;
            CurrentEncounter.ThreatLevel = FMath::Min(CurrentEncounter.ThreatLevel, 5);
            
            if (Rule.bSpawnReinforcements)
            {
                SpawnReinforcements(Rule.ReinforcementCount);
            }
            
            UE_LOG(LogTemp, Log, TEXT("Escalation rule triggered - new threat level: %d"), 
                   CurrentEncounter.ThreatLevel);
            break;
        }
    }
}

void UCombat_DynamicEncounterComponent::SpawnReinforcements(int32 Count)
{
    // This would spawn additional AI actors based on encounter type
    UE_LOG(LogTemp, Log, TEXT("Spawning %d reinforcements"), Count);
    
    // Implementation would depend on available AI actor classes
    // For now, just log the action
}

AActor* UCombat_DynamicEncounterComponent::GetPlayerActor() const
{
    if (UWorld* World = GetWorld())
    {
        return UGameplayStatics::GetPlayerCharacter(World, 0);
    }
    return nullptr;
}

// ACombat_DynamicEncounterManager Implementation

ACombat_DynamicEncounterManager::ACombat_DynamicEncounterManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;
    
    EncounterSpawnRadius = 2000.0f;
    MinEncounterInterval = 60.0f;
    MaxEncounterInterval = 180.0f;
    MaxSimultaneousEncounters = 3;
    NextEncounterTime = 0.0f;
    
    // Setup default encounters
    FCombat_EncounterData AmbushEncounter;
    AmbushEncounter.EncounterType = ECombat_EncounterType::Ambush;
    AmbushEncounter.ThreatLevel = 2;
    AmbushEncounter.Duration = 90.0f;
    AmbushEncounter.ParticipantCount = 2;
    AvailableEncounters.Add(AmbushEncounter);
    
    FCombat_EncounterData PackHuntEncounter;
    PackHuntEncounter.EncounterType = ECombat_EncounterType::PackHunt;
    PackHuntEncounter.ThreatLevel = 3;
    PackHuntEncounter.Duration = 120.0f;
    PackHuntEncounter.ParticipantCount = 4;
    AvailableEncounters.Add(PackHuntEncounter);
    
    FCombat_EncounterData TerritorialEncounter;
    TerritorialEncounter.EncounterType = ECombat_EncounterType::Territorial;
    TerritorialEncounter.ThreatLevel = 4;
    TerritorialEncounter.Duration = 150.0f;
    TerritorialEncounter.ParticipantCount = 1;
    AvailableEncounters.Add(TerritorialEncounter);
}

void ACombat_DynamicEncounterManager::BeginPlay()
{
    Super::BeginPlay();
    
    NextEncounterTime = GetWorld()->GetTimeSeconds() + 
                       FMath::RandRange(MinEncounterInterval, MaxEncounterInterval);
    
    UE_LOG(LogTemp, Log, TEXT("Combat Dynamic Encounter Manager initialized"));
}

void ACombat_DynamicEncounterManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateEncounterSpawning(DeltaTime);
}

void ACombat_DynamicEncounterManager::TriggerRandomEncounter()
{
    if (!CanSpawnNewEncounter())
    {
        return;
    }
    
    FVector Location = FindValidEncounterLocation();
    ECombat_EncounterType Type = SelectEncounterType();
    
    TriggerSpecificEncounter(Type, Location);
}

void ACombat_DynamicEncounterManager::TriggerSpecificEncounter(ECombat_EncounterType Type, FVector Location)
{
    if (!CanSpawnNewEncounter())
    {
        return;
    }
    
    // Find encounter data for this type
    FCombat_EncounterData* EncounterData = AvailableEncounters.FindByPredicate(
        [Type](const FCombat_EncounterData& Data) { return Data.EncounterType == Type; }
    );
    
    if (!EncounterData)
    {
        UE_LOG(LogTemp, Warning, TEXT("No encounter data found for type %d"), (int32)Type);
        return;
    }
    
    // Spawn encounter actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AActor* EncounterActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    if (EncounterActor)
    {
        UCombat_DynamicEncounterComponent* EncounterComponent = 
            EncounterActor->CreateDefaultSubobject<UCombat_DynamicEncounterComponent>(TEXT("EncounterComponent"));
        
        if (EncounterComponent)
        {
            EncounterComponent->StartEncounter(Type, EncounterData->ThreatLevel);
            RegisterEncounter(EncounterComponent);
            
            UE_LOG(LogTemp, Log, TEXT("Triggered encounter of type %d at location %s"), 
                   (int32)Type, *Location.ToString());
        }
    }
}

void ACombat_DynamicEncounterManager::RegisterEncounter(UCombat_DynamicEncounterComponent* Encounter)
{
    if (Encounter && !ActiveEncounters.Contains(Encounter))
    {
        ActiveEncounters.Add(Encounter);
        UE_LOG(LogTemp, Log, TEXT("Registered new encounter - total active: %d"), ActiveEncounters.Num());
    }
}

void ACombat_DynamicEncounterManager::UnregisterEncounter(UCombat_DynamicEncounterComponent* Encounter)
{
    if (ActiveEncounters.Remove(Encounter) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Unregistered encounter - total active: %d"), ActiveEncounters.Num());
    }
}

int32 ACombat_DynamicEncounterManager::GetActiveEncounterCount() const
{
    return ActiveEncounters.Num();
}

bool ACombat_DynamicEncounterManager::CanSpawnNewEncounter() const
{
    return ActiveEncounters.Num() < MaxSimultaneousEncounters;
}

void ACombat_DynamicEncounterManager::UpdateEncounterSpawning(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime >= NextEncounterTime && CanSpawnNewEncounter())
    {
        TriggerRandomEncounter();
        NextEncounterTime = CurrentTime + FMath::RandRange(MinEncounterInterval, MaxEncounterInterval);
    }
    
    // Clean up completed encounters
    ActiveEncounters.RemoveAll([](UCombat_DynamicEncounterComponent* Encounter) {
        return !Encounter || !IsValid(Encounter) || 
               Encounter->EncounterState == ECombat_EncounterState::Inactive;
    });
}

FVector ACombat_DynamicEncounterManager::FindValidEncounterLocation() const
{
    AActor* Player = GetPlayerActor();
    if (!Player)
    {
        return GetActorLocation();
    }
    
    FVector PlayerLocation = Player->GetActorLocation();
    
    // Find location within spawn radius but not too close to player
    float MinDistance = 800.0f;
    float MaxDistance = EncounterSpawnRadius;
    
    float Distance = FMath::RandRange(MinDistance, MaxDistance);
    float Angle = FMath::RandRange(0.0f, 360.0f);
    
    FVector Offset = FVector(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
        0.0f
    );
    
    return PlayerLocation + Offset;
}

ECombat_EncounterType ACombat_DynamicEncounterManager::SelectEncounterType() const
{
    if (AvailableEncounters.Num() == 0)
    {
        return ECombat_EncounterType::Territorial;
    }
    
    int32 RandomIndex = FMath::RandRange(0, AvailableEncounters.Num() - 1);
    return AvailableEncounters[RandomIndex].EncounterType;
}

AActor* ACombat_DynamicEncounterManager::GetPlayerActor() const
{
    if (UWorld* World = GetWorld())
    {
        return UGameplayStatics::GetPlayerCharacter(World, 0);
    }
    return nullptr;
}