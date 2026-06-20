#include "DinosaurCombatAI.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UDinosaurCombatAI::UDinosaurCombatAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for AI — performance conscious
}

void UDinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();

    // Apply species-specific default stats
    switch (Species)
    {
    case ECombat_DinoSpecies::TRex:
        Stats.DetectionRadius = 2500.0f;
        Stats.AttackRadius = 350.0f;
        Stats.AttackDamage = 80.0f;
        Stats.MoveSpeed = 500.0f;
        Stats.ChaseSpeed = 750.0f;
        Stats.Health = 500.0f;
        Stats.bIsPackHunter = false;
        Stats.PackSize = 1;
        break;

    case ECombat_DinoSpecies::Velociraptor:
        Stats.DetectionRadius = 1200.0f;
        Stats.AttackRadius = 150.0f;
        Stats.AttackDamage = 30.0f;
        Stats.MoveSpeed = 700.0f;
        Stats.ChaseSpeed = 1100.0f;
        Stats.Health = 80.0f;
        Stats.bIsPackHunter = true;
        Stats.PackSize = 3;
        break;

    case ECombat_DinoSpecies::Brachiosaurus:
        Stats.DetectionRadius = 800.0f;
        Stats.AttackRadius = 400.0f;
        Stats.AttackDamage = 60.0f; // Stomp damage
        Stats.MoveSpeed = 300.0f;
        Stats.ChaseSpeed = 400.0f;
        Stats.Health = 800.0f;
        Stats.bIsPackHunter = false;
        Stats.PackSize = 1;
        break;

    case ECombat_DinoSpecies::Triceratops:
        Stats.DetectionRadius = 1000.0f;
        Stats.AttackRadius = 250.0f;
        Stats.AttackDamage = 50.0f;
        Stats.MoveSpeed = 450.0f;
        Stats.ChaseSpeed = 650.0f;
        Stats.Health = 300.0f;
        Stats.bIsPackHunter = false;
        Stats.PackSize = 1;
        break;

    case ECombat_DinoSpecies::Pterodactyl:
        Stats.DetectionRadius = 3000.0f;
        Stats.AttackRadius = 200.0f;
        Stats.AttackDamage = 25.0f;
        Stats.MoveSpeed = 900.0f;
        Stats.ChaseSpeed = 1200.0f;
        Stats.Health = 60.0f;
        Stats.bIsPackHunter = false;
        Stats.PackSize = 1;
        break;

    default:
        break;
    }

    CurrentThreatState = ECombat_DinoThreatState::Patrol;
}

void UDinosaurCombatAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Find player if not tracked
    if (!TrackedPlayer)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0);
            if (PlayerChar)
            {
                TrackedPlayer = PlayerChar;
            }
        }
    }

    if (TrackedPlayer && GetOwner())
    {
        DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), TrackedPlayer->GetActorLocation());
        bPlayerDetected = (DistanceToPlayer <= Stats.DetectionRadius);
    }

    UpdateStateLogic(DeltaTime);
    StateTimer += DeltaTime;
}

void UDinosaurCombatAI::UpdateStateLogic(float DeltaTime)
{
    switch (CurrentThreatState)
    {
    case ECombat_DinoThreatState::Idle:
        if (bPlayerDetected)
        {
            TransitionToState(ECombat_DinoThreatState::Alert);
        }
        break;

    case ECombat_DinoThreatState::Patrol:
        if (bPlayerDetected && DistanceToPlayer < Stats.DetectionRadius * 0.7f)
        {
            TransitionToState(ECombat_DinoThreatState::Alert);
        }
        break;

    case ECombat_DinoThreatState::Alert:
        if (DistanceToPlayer < Stats.DetectionRadius * 0.5f)
        {
            // Raptors stalk before charging; T-Rex charges immediately
            if (Stats.bIsPackHunter)
            {
                TransitionToState(ECombat_DinoThreatState::Stalk);
            }
            else
            {
                TransitionToState(ECombat_DinoThreatState::Chase);
            }
        }
        else if (!bPlayerDetected)
        {
            TransitionToState(ECombat_DinoThreatState::Patrol);
        }
        break;

    case ECombat_DinoThreatState::Stalk:
        StalkTimer += DeltaTime;
        if (StalkTimer >= Stats.StalkDuration || DistanceToPlayer < Stats.AttackRadius * 3.0f)
        {
            StalkTimer = 0.0f;
            TransitionToState(ECombat_DinoThreatState::Chase);
        }
        else if (!bPlayerDetected)
        {
            StalkTimer = 0.0f;
            TransitionToState(ECombat_DinoThreatState::Patrol);
        }
        break;

    case ECombat_DinoThreatState::Chase:
        if (DistanceToPlayer <= Stats.AttackRadius)
        {
            TransitionToState(ECombat_DinoThreatState::Attack);
        }
        else if (DistanceToPlayer > Stats.DetectionRadius * 1.5f)
        {
            TransitionToState(ECombat_DinoThreatState::Patrol);
        }
        break;

    case ECombat_DinoThreatState::Attack:
        if (DistanceToPlayer > Stats.AttackRadius * 1.5f)
        {
            TransitionToState(ECombat_DinoThreatState::Chase);
        }
        break;

    case ECombat_DinoThreatState::Retreat:
        if (StateTimer > 5.0f)
        {
            TransitionToState(ECombat_DinoThreatState::Patrol);
        }
        break;

    case ECombat_DinoThreatState::Feeding:
        // Stay feeding until disturbed
        if (bPlayerDetected && DistanceToPlayer < Stats.DetectionRadius * 0.4f)
        {
            TransitionToState(ECombat_DinoThreatState::Alert);
        }
        break;

    default:
        break;
    }
}

void UDinosaurCombatAI::TransitionToState(ECombat_DinoThreatState NewState)
{
    if (CurrentThreatState != NewState)
    {
        CurrentThreatState = NewState;
        StateTimer = 0.0f;
    }
}

void UDinosaurCombatAI::SetThreatState(ECombat_DinoThreatState NewState)
{
    TransitionToState(NewState);
}

ECombat_DinoThreatState UDinosaurCombatAI::EvaluateThreat(float PlayerDistance, bool bPlayerVisible)
{
    if (!bPlayerVisible || PlayerDistance > Stats.DetectionRadius)
    {
        return ECombat_DinoThreatState::Patrol;
    }
    if (PlayerDistance <= Stats.AttackRadius)
    {
        return ECombat_DinoThreatState::Attack;
    }
    if (PlayerDistance <= Stats.DetectionRadius * 0.5f)
    {
        return Stats.bIsPackHunter ? ECombat_DinoThreatState::Stalk : ECombat_DinoThreatState::Chase;
    }
    return ECombat_DinoThreatState::Alert;
}

float UDinosaurCombatAI::GetAttackDamage() const
{
    return Stats.AttackDamage;
}

bool UDinosaurCombatAI::IsHostile() const
{
    return CurrentThreatState == ECombat_DinoThreatState::Chase
        || CurrentThreatState == ECombat_DinoThreatState::Attack
        || CurrentThreatState == ECombat_DinoThreatState::Stalk;
}

void UDinosaurCombatAI::OnPlayerSpotted(AActor* Player)
{
    if (Player)
    {
        TrackedPlayer = Player;
        bPlayerDetected = true;
        if (CurrentThreatState == ECombat_DinoThreatState::Idle || CurrentThreatState == ECombat_DinoThreatState::Patrol)
        {
            TransitionToState(ECombat_DinoThreatState::Alert);
        }
    }
}

void UDinosaurCombatAI::OnPlayerLost()
{
    bPlayerDetected = false;
    if (CurrentThreatState == ECombat_DinoThreatState::Chase || CurrentThreatState == ECombat_DinoThreatState::Stalk)
    {
        TransitionToState(ECombat_DinoThreatState::Alert);
    }
}

void UDinosaurCombatAI::TakeDamage_Combat(float DamageAmount)
{
    Stats.Health = FMath::Max(0.0f, Stats.Health - DamageAmount);

    // Low health — retreat
    if (Stats.Health < 30.0f && CurrentThreatState != ECombat_DinoThreatState::Retreat)
    {
        TransitionToState(ECombat_DinoThreatState::Retreat);
    }
}
