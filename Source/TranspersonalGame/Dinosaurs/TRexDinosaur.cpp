#include "TRexDinosaur.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ATRexDinosaur::ATRexDinosaur()
{
    PrimaryActorTick.bCanEverTick = true;

    // T-Rex base stats — apex predator
    Health = 1200.0f;
    MaxHealth = 1200.0f;
    AttackDamage = 150.0f;
    AttackRange = 280.0f;
    AggroRadius = 2500.0f;
    MoveSpeed = 550.0f;
    TurnRate = 60.0f;
    Mass = 8000.0f;
    bIsCarnivore = true;
    bIsPredator = true;
    bIsPackHunter = false;
    Species = ECore_DinosaurSpecies::TyrannosaurusRex;
    ThreatLevel = ECore_ThreatLevel::ApexPredator;

    // Movement — powerful but not agile
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, TurnRate, 0.0f);
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->GravityScale = 1.2f;
        GetCharacterMovement()->MaxAcceleration = 800.0f;
    }
}

void ATRexDinosaur::BeginPlay()
{
    Super::BeginPlay();

    // T-Rex starts in patrol state
    CurrentState = ECore_DinosaurState::Patrolling;
    TimeSinceLastRoar = RoarCooldown; // Ready to roar immediately
    RoarTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("ATRexDinosaur: T-Rex spawned — Health=%.0f, AggroRadius=%.0f"),
        Health, AggroRadius);
}

void ATRexDinosaur::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update roar cooldown timer
    RoarTimer += DeltaTime;
    TimeSinceLastRoar = RoarTimer;

    // Auto-roar when entering combat
    if (CurrentState == ECore_DinosaurState::Attacking && RoarTimer >= RoarCooldown)
    {
        PerformRoar();
    }
}

void ATRexDinosaur::PerformRoar()
{
    if (bIsRoaring) return;

    bIsRoaring = true;
    RoarTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("ATRexDinosaur: ROAR! Radius=%.0f"), RoarRadius);

    // Apply fear to all actors within roar radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == this) continue;

        float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Distance <= RoarRadius)
        {
            // Apply fear via interface or component — placeholder log
            UE_LOG(LogTemp, Verbose, TEXT("ATRexDinosaur: Fear applied to %s (dist=%.0f)"),
                *Actor->GetName(), Distance);
        }
    }

    // Reset roaring flag after short delay (animation would drive this)
    bIsRoaring = false;
}

void ATRexDinosaur::PerformStomp()
{
    FVector StompOrigin = GetActorLocation();

    UE_LOG(LogTemp, Log, TEXT("ATRexDinosaur: STOMP at %s, Radius=%.0f, Damage=%.0f"),
        *StompOrigin.ToString(), StompRadius, StompDamage);

    // Apply radial damage
    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(this);

    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        StompDamage,
        StompOrigin,
        StompRadius,
        UDamageType::StaticClass(),
        IgnoredActors,
        this,
        GetInstigatorController(),
        false
    );

#if WITH_EDITOR
    // Debug visualization in editor
    DrawDebugSphere(GetWorld(), StompOrigin, StompRadius, 12, FColor::Orange, false, 2.0f);
#endif
}

bool ATRexDinosaur::IsPlayerInVisionCone() const
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return false;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return false;

    FVector ToPlayer = (PlayerPawn->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector ForwardVec = GetActorForwardVector();

    float DotProduct = FVector::DotProduct(ForwardVec, ToPlayer);
    float VisionAngleCos = FMath::Cos(FMath::DegreesToRadians(60.0f)); // 120 degree cone

    float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

    return (DotProduct >= VisionAngleCos) && (Distance <= AggroRadius);
}

void ATRexDinosaur::PerformAttack()
{
    Super::PerformAttack();

    // T-Rex combo: bite + stomp
    // Stomp on every 3rd attack
    static int32 AttackCount = 0;
    AttackCount++;

    if (AttackCount % 3 == 0)
    {
        PerformStomp();
    }

    UE_LOG(LogTemp, Log, TEXT("ATRexDinosaur: Attack #%d — Damage=%.0f"), AttackCount, AttackDamage);
}

bool ATRexDinosaur::ShouldAggro(AActor* Target) const
{
    if (!Target) return false;

    // T-Rex aggroes on anything within radius — apex predator
    float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());

    // Also check vision cone for distant targets
    if (Distance > AggroRadius * 0.5f)
    {
        return IsPlayerInVisionCone();
    }

    return Distance <= AggroRadius;
}
