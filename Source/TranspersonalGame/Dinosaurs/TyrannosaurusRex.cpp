#include "Dinosaurs/TyrannosaurusRex.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ATyrannosaurusRex::ATyrannosaurusRex()
{
    // ── Species identity ──────────────────────────────────────────────────
    Species = EEng_DinoSpecies::TyrannosaurusRex;

    // ── Apex predator stats ───────────────────────────────────────────────
    DinoStats.MaxHealth        = 2000.0f;
    DinoStats.CurrentHealth    = 2000.0f;
    DinoStats.AttackDamage     = 120.0f;
    DinoStats.DetectionRadius  = 3000.0f;
    DinoStats.AttackRange      = 400.0f;
    DinoStats.WalkSpeed        = 250.0f;
    DinoStats.RunSpeed         = 900.0f;
    DinoStats.HungerDecayRate  = 0.3f;   // Slower metabolism — large body
    DinoStats.bIsCarnivore     = true;
    DinoStats.bIsPackHunter    = false;  // Solitary apex predator

    // ── Movement ──────────────────────────────────────────────────────────
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = DinoStats.WalkSpeed;
        GetCharacterMovement()->MaxAcceleration = 800.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 600.0f;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 120.0f, 0.0f);
        GetCharacterMovement()->bOrientRotationToMovement = true;
    }

    // ── Mesh scale — T-Rex is large ───────────────────────────────────────
    GetCapsuleComponent()->SetCapsuleSize(120.0f, 250.0f);
}

void ATyrannosaurusRex::BeginPlay()
{
    Super::BeginPlay();
    RoarTimer = FMath::RandRange(0.0f, RoarCooldown * 0.5f); // Stagger initial roar
}

void ATyrannosaurusRex::PerformRoar()
{
    if (!IsAlive()) return;

    RoarTimer = 0.0f;

    // Apply fear to player if within roar radius
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    if (DistToPlayer <= RoarRadius)
    {
        // Blueprint handles the actual fear application + audio/VFX
        OnRoar();
    }
}

void ATyrannosaurusRex::StartCharge(AActor* Target)
{
    if (!IsAlive() || !Target) return;

    bIsCharging = true;

    // Boost movement speed during charge
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = DinoStats.RunSpeed * ChargeSpeedMultiplier;
    }

    OnChargeStart(Target);

    // Auto-reset charge after 3 seconds
    FTimerHandle ChargeTimer;
    GetWorldTimerManager().SetTimer(ChargeTimer, [this]()
    {
        bIsCharging = false;
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->MaxWalkSpeed = DinoStats.RunSpeed;
        }
    }, 3.0f, false);
}
