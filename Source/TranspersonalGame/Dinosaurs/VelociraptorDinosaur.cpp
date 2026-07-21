#include "VelociraptorDinosaur.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AVelociraptorDinosaur::AVelociraptorDinosaur()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule — lean, fast predator
    GetCapsuleComponent()->SetCapsuleRadius(35.0f);
    GetCapsuleComponent()->SetCapsuleHalfHeight(60.0f);

    // Movement — fastest dinosaur in the roster
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 900.0f;
        GetCharacterMovement()->JumpZVelocity = 700.0f;
        GetCharacterMovement()->AirControl = 0.4f;
        GetCharacterMovement()->bOrientRotationToMovement = true;
    }

    // Pack coordination defaults
    bIsPackLeader = false;
    PackID = 0;
    PackCoordinationRadius = 1200.0f;
    MaxPackSize = 6;

    // Leap attack defaults
    LeapRange = 500.0f;
    LeapDamage = 55.0f;
    LeapCooldown = 4.0f;
    LeapImpulseForce = 1200.0f;
    LeapCooldownRemaining = 0.0f;
    bIsLeaping = false;

    // Flanking
    FlankingAngle = 45.0f;
    bIsFlankingTarget = false;

    // Base stats (overrides DinosaurBase defaults)
    MaxHealth = 350.0f;
    CurrentHealth = 350.0f;
    BaseDamage = 55.0f;
    DetectionRadius = 1400.0f;
    BaseWalkSpeed = 900.0f;
}

void AVelociraptorDinosaur::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("VelociraptorDinosaur spawned — PackID:%d Leader:%s"),
        PackID, bIsPackLeader ? TEXT("YES") : TEXT("NO"));
}

void AVelociraptorDinosaur::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateLeapCooldown(DeltaTime);

    // Pack leader coordinates attack every 2 seconds
    if (bIsPackLeader)
    {
        AttemptPackCoordination();
    }
}

void AVelociraptorDinosaur::ExecuteLeapAttack(AActor* Target)
{
    if (!Target || !CanLeap()) return;

    bIsLeaping = true;
    LeapCooldownRemaining = LeapCooldown;

    // Launch toward target
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector LaunchVelocity = ToTarget * LeapImpulseForce + FVector(0.0f, 0.0f, 400.0f);
    LaunchCharacter(LaunchVelocity, true, true);

    // Apply damage on leap
    UGameplayStatics::ApplyDamage(Target, LeapDamage, GetController(), this, nullptr);

    UE_LOG(LogTemp, Log, TEXT("Raptor leap attack on %s — %.0f damage"), *Target->GetName(), LeapDamage);

#if WITH_EDITOR
    DrawDebugLine(GetWorld(), GetActorLocation(), Target->GetActorLocation(),
        FColor::Orange, false, 1.5f, 0, 3.0f);
#endif
}

void AVelociraptorDinosaur::SignalPackToAttack(AActor* Target)
{
    if (!bIsPackLeader || !Target) return;

    // Find all raptors in pack coordination radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVelociraptorDinosaur::StaticClass(), NearbyActors);

    int32 SignaledCount = 0;
    for (AActor* Actor : NearbyActors)
    {
        AVelociraptorDinosaur* Raptor = Cast<AVelociraptorDinosaur>(Actor);
        if (Raptor && Raptor != this && Raptor->PackID == PackID)
        {
            float Dist = FVector::Dist(GetActorLocation(), Raptor->GetActorLocation());
            if (Dist <= PackCoordinationRadius)
            {
                Raptor->bIsFlankingTarget = true;
                SignaledCount++;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Pack leader signaled %d raptors to attack %s"),
        SignaledCount, *Target->GetName());
}

void AVelociraptorDinosaur::ExecuteFlankingManeuver(AActor* Target)
{
    if (!Target) return;

    bIsFlankingTarget = true;

    // Calculate flanking position — offset by FlankingAngle from target's forward
    FVector TargetLoc = Target->GetActorLocation();
    FVector TargetFwd = Target->GetActorForwardVector();
    FRotator FlankRot = FRotator(0.0f, FlankingAngle, 0.0f);
    FVector FlankDir = FlankRot.RotateVector(TargetFwd);
    FVector FlankPos = TargetLoc + FlankDir * 600.0f;

    // Move toward flank position (AI controller would handle this in full impl)
    FVector ToFlank = (FlankPos - GetActorLocation()).GetSafeNormal();
    AddMovementInput(ToFlank, 1.0f);

    UE_LOG(LogTemp, Verbose, TEXT("Raptor flanking %s at angle %.0f"), *Target->GetName(), FlankingAngle);
}

bool AVelociraptorDinosaur::CanLeap() const
{
    return !bIsLeaping && LeapCooldownRemaining <= 0.0f;
}

void AVelociraptorDinosaur::UpdateLeapCooldown(float DeltaTime)
{
    if (LeapCooldownRemaining > 0.0f)
    {
        LeapCooldownRemaining -= DeltaTime;
        if (LeapCooldownRemaining <= 0.0f)
        {
            LeapCooldownRemaining = 0.0f;
            bIsLeaping = false;
        }
    }
}

void AVelociraptorDinosaur::AttemptPackCoordination()
{
    // Pack leader periodically checks if pack members need repositioning
    // Full implementation hooks into BehaviorTree via blackboard
    // Stub: log pack state every ~60 ticks
    static int32 CoordTick = 0;
    CoordTick++;
    if (CoordTick % 120 == 0)
    {
        UE_LOG(LogTemp, Verbose, TEXT("Pack leader (ID:%d) coordinating pack"), PackID);
    }
}
