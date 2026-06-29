#include "DinosaurCombatAI.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UDinosaurCombatAI::UDinosaurCombatAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for AI — performance-conscious

    // Default T-Rex attack profile
    PrimaryAttack.BaseDamage = 40.0f;
    PrimaryAttack.AttackRange = 250.0f;
    PrimaryAttack.Cooldown = 3.0f;
    PrimaryAttack.KnockbackForce = 800.0f;
    PrimaryAttack.bCanInterrupt = false;

    AIState.DetectionRadius = 2000.0f;
    AIState.AggroRadius = 1000.0f;
    AIState.MaxHealth = 100.0f;
    AIState.CurrentHealth = 100.0f;
}

void UDinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();

    // Configure species-specific stats
    switch (Species)
    {
    case ECombat_DinoSpecies::Velociraptor:
        PrimaryAttack.BaseDamage = 15.0f;
        PrimaryAttack.AttackRange = 150.0f;
        PrimaryAttack.Cooldown = 1.0f;
        PrimaryAttack.KnockbackForce = 300.0f;
        AIState.DetectionRadius = 1800.0f;
        AIState.MaxHealth = 60.0f;
        AIState.CurrentHealth = 60.0f;
        AIState.bIsPackMember = true;
        break;

    case ECombat_DinoSpecies::TRex:
        PrimaryAttack.BaseDamage = 60.0f;
        PrimaryAttack.AttackRange = 300.0f;
        PrimaryAttack.Cooldown = 4.0f;
        PrimaryAttack.KnockbackForce = 1200.0f;
        AIState.DetectionRadius = 3000.0f;
        AIState.MaxHealth = 500.0f;
        AIState.CurrentHealth = 500.0f;
        break;

    case ECombat_DinoSpecies::Triceratops:
        PrimaryAttack.BaseDamage = 35.0f;
        PrimaryAttack.AttackRange = 220.0f;
        PrimaryAttack.Cooldown = 2.5f;
        PrimaryAttack.KnockbackForce = 900.0f;
        AIState.DetectionRadius = 1200.0f;
        AIState.MaxHealth = 300.0f;
        AIState.CurrentHealth = 300.0f;
        break;

    case ECombat_DinoSpecies::Pterodactyl:
        PrimaryAttack.BaseDamage = 20.0f;
        PrimaryAttack.AttackRange = 180.0f;
        PrimaryAttack.Cooldown = 1.5f;
        PrimaryAttack.KnockbackForce = 400.0f;
        AIState.DetectionRadius = 2500.0f;
        AIState.MaxHealth = 80.0f;
        AIState.CurrentHealth = 80.0f;
        break;

    default:
        break;
    }
}

void UDinosaurCombatAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastDetectionCheck += DeltaTime;
    if (TimeSinceLastDetectionCheck >= DetectionCheckInterval)
    {
        TimeSinceLastDetectionCheck = 0.0f;
        PerformDetectionSweep();
    }

    UpdateThreatLevel();

    if (AIState.bIsPackMember)
    {
        HandlePackBehavior();
    }
}

void UDinosaurCombatAI::PerformDetectionSweep()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = Owner->GetWorld();
    if (!World) return;

    // Find the player character
    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!PlayerChar) return;

    float DistToPlayer = FVector::Dist(Owner->GetActorLocation(), PlayerChar->GetActorLocation());

    if (DistToPlayer <= AIState.AggroRadius)
    {
        AIState.bPlayerDetected = true;
        AIState.LastKnownPlayerLocation = PlayerChar->GetActorLocation();
        CurrentTarget = PlayerChar;
    }
    else if (DistToPlayer <= AIState.DetectionRadius)
    {
        // Line of sight check for detection radius
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Owner);

        bool bHit = World->LineTraceSingleByChannel(
            HitResult,
            Owner->GetActorLocation() + FVector(0, 0, 100),
            PlayerChar->GetActorLocation() + FVector(0, 0, 100),
            ECC_Visibility,
            QueryParams
        );

        if (!bHit || HitResult.GetActor() == PlayerChar)
        {
            AIState.bPlayerDetected = true;
            AIState.LastKnownPlayerLocation = PlayerChar->GetActorLocation();
            CurrentTarget = PlayerChar;
        }
    }
    else
    {
        // Player out of range — lose detection over time
        if (AIState.bPlayerDetected)
        {
            float DistToLastKnown = FVector::Dist(Owner->GetActorLocation(), AIState.LastKnownPlayerLocation);
            if (DistToLastKnown < 100.0f)
            {
                // Reached last known position, player gone
                AIState.bPlayerDetected = false;
                CurrentTarget = nullptr;
            }
        }
    }
}

void UDinosaurCombatAI::UpdateThreatLevel()
{
    if (!AIState.bPlayerDetected || !CurrentTarget)
    {
        AIState.ThreatLevel = ECombat_DinoThreatLevel::None;
        return;
    }

    // Flee if health critically low
    float HealthPercent = AIState.CurrentHealth / AIState.MaxHealth;
    if (HealthPercent <= FleeHealthThreshold)
    {
        AIState.ThreatLevel = ECombat_DinoThreatLevel::Retreating;
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float DistToTarget = FVector::Dist(Owner->GetActorLocation(), CurrentTarget->GetActorLocation());

    if (DistToTarget <= PrimaryAttack.AttackRange)
    {
        AIState.ThreatLevel = ECombat_DinoThreatLevel::Attacking;
    }
    else if (DistToTarget <= AIState.AggroRadius * 0.5f)
    {
        AIState.ThreatLevel = ECombat_DinoThreatLevel::Charging;
    }
    else
    {
        AIState.ThreatLevel = ECombat_DinoThreatLevel::Stalking;
    }
}

void UDinosaurCombatAI::DetectPlayer(AActor* Player)
{
    if (!Player) return;
    CurrentTarget = Player;
    AIState.bPlayerDetected = true;
    AIState.LastKnownPlayerLocation = Player->GetActorLocation();
}

void UDinosaurCombatAI::ExecuteAttack(AActor* Target)
{
    if (!Target || !CanAttack()) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float DistToTarget = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget > PrimaryAttack.AttackRange) return;

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        Target,
        PrimaryAttack.BaseDamage,
        nullptr,
        Owner,
        nullptr
    );

    // Apply knockback if target is a character
    ACharacter* TargetChar = Cast<ACharacter>(Target);
    if (TargetChar && TargetChar->GetCharacterMovement())
    {
        FVector KnockbackDir = (Target->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
        KnockbackDir.Z = 0.4f; // Slight upward component
        TargetChar->GetCharacterMovement()->AddImpulse(KnockbackDir * PrimaryAttack.KnockbackForce, true);
    }

    AIState.LastAttackTime = Owner->GetWorld() ? Owner->GetWorld()->GetTimeSeconds() : 0.0f;
    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: %s attacked %s for %.1f damage"),
        *Owner->GetName(), *Target->GetName(), PrimaryAttack.BaseDamage);
}

void UDinosaurCombatAI::TakeDamage_Dino(float DamageAmount, AActor* DamageSource)
{
    AIState.CurrentHealth = FMath::Max(0.0f, AIState.CurrentHealth - DamageAmount);

    // Aggro on damage source
    if (DamageSource && !AIState.bPlayerDetected)
    {
        DetectPlayer(DamageSource);
    }

    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: Took %.1f damage. Health: %.1f/%.1f"),
        DamageAmount, AIState.CurrentHealth, AIState.MaxHealth);

    // Broadcast to pack members if alpha
    if (bIsAlpha && AIState.bIsPackMember)
    {
        HandlePackBehavior();
    }
}

void UDinosaurCombatAI::SetPackCoordination(bool bEnabled, int32 PackMemberCount)
{
    AIState.bIsPackMember = bEnabled;
    AIState.PackSize = FMath::Max(1, PackMemberCount);

    // Pack members get damage bonus based on pack size
    if (bEnabled && PackMemberCount > 1)
    {
        float PackBonus = 1.0f + (PackMemberCount - 1) * 0.15f; // 15% per additional member
        PrimaryAttack.BaseDamage *= PackBonus;
        UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: Pack coordination enabled. Size: %d, Damage bonus: %.2fx"),
            PackMemberCount, PackBonus);
    }
}

void UDinosaurCombatAI::Retreat(FVector SafeLocation)
{
    AIState.ThreatLevel = ECombat_DinoThreatLevel::Retreating;
    AIState.bPlayerDetected = false;
    CurrentTarget = nullptr;
    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAI: Retreating to safe location"));
}

float UDinosaurCombatAI::GetThreatScore() const
{
    float Score = 0.0f;

    // Base score from species
    switch (Species)
    {
    case ECombat_DinoSpecies::TRex:         Score = 100.0f; break;
    case ECombat_DinoSpecies::Triceratops:  Score = 70.0f;  break;
    case ECombat_DinoSpecies::Velociraptor: Score = 50.0f;  break;
    case ECombat_DinoSpecies::Pterodactyl:  Score = 40.0f;  break;
    default:                                Score = 20.0f;  break;
    }

    // Modify by health
    float HealthPercent = AIState.MaxHealth > 0.0f ? (AIState.CurrentHealth / AIState.MaxHealth) : 0.0f;
    Score *= HealthPercent;

    // Pack multiplier
    if (AIState.bIsPackMember && AIState.PackSize > 1)
    {
        Score *= (1.0f + (AIState.PackSize - 1) * 0.3f);
    }

    // Alpha bonus
    if (bIsAlpha) Score *= 1.5f;

    return Score;
}

bool UDinosaurCombatAI::IsPlayerInAttackRange() const
{
    if (!CurrentTarget) return false;
    AActor* Owner = GetOwner();
    if (!Owner) return false;

    float Dist = FVector::Dist(Owner->GetActorLocation(), CurrentTarget->GetActorLocation());
    return Dist <= PrimaryAttack.AttackRange;
}

bool UDinosaurCombatAI::CanAttack() const
{
    AActor* Owner = GetOwner();
    if (!Owner || !Owner->GetWorld()) return false;

    float CurrentTime = Owner->GetWorld()->GetTimeSeconds();
    return (CurrentTime - AIState.LastAttackTime) >= PrimaryAttack.Cooldown;
}

void UDinosaurCombatAI::HandlePackBehavior()
{
    if (!AIState.bIsPackMember || AIState.PackSize <= 1) return;

    AActor* Owner = GetOwner();
    if (!Owner || !CurrentTarget) return;

    // Pack flanking: non-alpha members spread around target
    if (!bIsAlpha)
    {
        // Offset position relative to target to create flanking
        FVector ToTarget = (CurrentTarget->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
        FVector FlankOffset = FVector::CrossProduct(ToTarget, FVector::UpVector) * 300.0f;

        // Alternate flank direction based on actor name hash
        int32 NameHash = Owner->GetName().Len();
        if (NameHash % 2 == 0)
        {
            FlankOffset *= -1.0f;
        }

        // The actual movement is handled by the BehaviorTree/NavMesh
        // This just sets the desired flank position for the BT to use
        AIState.LastKnownPlayerLocation = CurrentTarget->GetActorLocation() + FlankOffset;
    }
}
