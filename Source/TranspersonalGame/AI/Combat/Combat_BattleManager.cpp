#include "Combat_BattleManager.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ACombat_BattleManager::ACombat_BattleManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create combat detection sphere
    CombatDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatDetectionSphere"));
    CombatDetectionSphere->SetupAttachment(RootComponent);
    CombatDetectionSphere->SetSphereRadius(CombatDetectionRadius);
    CombatDetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CombatDetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    CombatDetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize combat state
    CurrentBattleState = FCombat_BattleState();
    PlayerCharacter = nullptr;
}

void ACombat_BattleManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (CombatDetectionSphere)
    {
        CombatDetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACombat_BattleManager::OnCombatDetectionBeginOverlap);
        CombatDetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ACombat_BattleManager::OnCombatDetectionEndOverlap);
    }

    // Find player character
    FindPlayerCharacter();

    UE_LOG(LogTemp, Warning, TEXT("Combat_BattleManager initialized at location: %s"), *GetActorLocation().ToString());
}

void ACombat_BattleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update combat state
    UpdateCombatIntensity(DeltaTime);
    UpdateEnemyStates(DeltaTime);

    // Check for combat timeout
    if (CurrentBattleState.bInCombat)
    {
        float TimeSinceLastCombat = GetWorld()->GetTimeSeconds() - CurrentBattleState.LastCombatTime;
        if (TimeSinceLastCombat > CombatTimeoutDuration && CurrentBattleState.ActiveEnemies == 0)
        {
            EndCombat();
        }
    }
}

void ACombat_BattleManager::StartCombat(AActor* Enemy, AActor* Player)
{
    if (!CurrentBattleState.bInCombat)
    {
        CurrentBattleState.bInCombat = true;
        CurrentBattleState.LastCombatTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Warning, TEXT("Combat started between %s and %s"), 
               Enemy ? *Enemy->GetName() : TEXT("Unknown"), 
               Player ? *Player->GetName() : TEXT("Unknown"));
    }

    RegisterEnemy(Enemy);
    CurrentBattleState.CombatIntensity = FMath::Min(CurrentBattleState.CombatIntensity + 0.3f, MaxCombatIntensity);
}

void ACombat_BattleManager::EndCombat()
{
    if (CurrentBattleState.bInCombat)
    {
        CurrentBattleState.bInCombat = false;
        CurrentBattleState.CombatIntensity = 0.0f;
        CurrentBattleState.ActiveEnemies = 0;
        ActiveEnemies.Empty();

        UE_LOG(LogTemp, Warning, TEXT("Combat ended"));
    }
}

void ACombat_BattleManager::UpdateCombatIntensity(float DeltaTime)
{
    if (CurrentBattleState.bInCombat)
    {
        // Gradually increase intensity during combat
        CurrentBattleState.CombatIntensity = FMath::Min(
            CurrentBattleState.CombatIntensity + (DeltaTime * 0.1f), 
            MaxCombatIntensity
        );
    }
    else
    {
        // Decay intensity when not in combat
        CurrentBattleState.CombatIntensity = FMath::Max(
            CurrentBattleState.CombatIntensity - (DeltaTime * IntensityDecayRate), 
            0.0f
        );
    }
}

void ACombat_BattleManager::RegisterEnemy(AActor* Enemy)
{
    if (Enemy && IsValidCombatant(Enemy) && !ActiveEnemies.Contains(Enemy))
    {
        ActiveEnemies.Add(Enemy);
        CurrentBattleState.ActiveEnemies = ActiveEnemies.Num();
        CurrentBattleState.LastCombatTime = GetWorld()->GetTimeSeconds();

        UE_LOG(LogTemp, Log, TEXT("Enemy registered: %s. Active enemies: %d"), 
               *Enemy->GetName(), CurrentBattleState.ActiveEnemies);
    }
}

void ACombat_BattleManager::UnregisterEnemy(AActor* Enemy)
{
    if (Enemy && ActiveEnemies.Contains(Enemy))
    {
        ActiveEnemies.Remove(Enemy);
        CurrentBattleState.ActiveEnemies = ActiveEnemies.Num();

        UE_LOG(LogTemp, Log, TEXT("Enemy unregistered: %s. Active enemies: %d"), 
               *Enemy->GetName(), CurrentBattleState.ActiveEnemies);

        if (CurrentBattleState.ActiveEnemies == 0)
        {
            CurrentBattleState.LastCombatTime = GetWorld()->GetTimeSeconds();
        }
    }
}

void ACombat_BattleManager::CoordinateEnemyAttacks()
{
    // Implement pack coordination logic
    for (AActor* Enemy : ActiveEnemies)
    {
        if (IsValid(Enemy))
        {
            // Basic pack behavior - enemies take turns attacking
            // More sophisticated AI would coordinate flanking, etc.
            UE_LOG(LogTemp, Log, TEXT("Coordinating attack for: %s"), *Enemy->GetName());
        }
    }
}

void ACombat_BattleManager::AssignTargetPriorities()
{
    if (!PlayerCharacter)
    {
        FindPlayerCharacter();
    }

    // For now, all enemies target the player
    // Future: could implement threat system, multiple targets, etc.
    for (AActor* Enemy : ActiveEnemies)
    {
        if (IsValid(Enemy))
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("Assigning player target to: %s"), *Enemy->GetName());
        }
    }
}

void ACombat_BattleManager::OnCombatDetectionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (IsValidCombatant(OtherActor))
    {
        // Check if it's a dinosaur/enemy
        FString ActorName = OtherActor->GetName().ToLower();
        if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
            ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("dino")))
        {
            if (PlayerCharacter)
            {
                StartCombat(OtherActor, PlayerCharacter);
            }
        }
    }
}

void ACombat_BattleManager::OnCombatDetectionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ActiveEnemies.Contains(OtherActor))
    {
        UnregisterEnemy(OtherActor);
    }
}

void ACombat_BattleManager::FindPlayerCharacter()
{
    if (!PlayerCharacter)
    {
        PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
        if (PlayerCharacter)
        {
            UE_LOG(LogTemp, Log, TEXT("Player character found: %s"), *PlayerCharacter->GetName());
        }
    }
}

void ACombat_BattleManager::UpdateEnemyStates(float DeltaTime)
{
    // Remove invalid enemies
    for (int32 i = ActiveEnemies.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveEnemies[i]))
        {
            ActiveEnemies.RemoveAt(i);
        }
    }

    CurrentBattleState.ActiveEnemies = ActiveEnemies.Num();

    // Coordinate attacks periodically
    static float LastCoordinationTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastCoordinationTime > 2.0f)
    {
        CoordinateEnemyAttacks();
        AssignTargetPriorities();
        LastCoordinationTime = CurrentTime;
    }
}

bool ACombat_BattleManager::IsValidCombatant(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return false;
    }

    // Check if it's a character/pawn
    if (Actor->IsA<ACharacter>() || Actor->IsA<APawn>())
    {
        return true;
    }

    // Check by name for dinosaur actors
    FString ActorName = Actor->GetName().ToLower();
    return ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
           ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("dino")) ||
           ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi"));
}