#include "VelociraptorDinosaur.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

AVelociraptorDinosaur::AVelociraptorDinosaur()
{
    // --- Core Stats (pack hunter profile) ---
    MaxHealth          = 800.0f;
    CurrentHealth      = 800.0f;
    AttackDamage       = 120.0f;
    MoveSpeed          = 1200.0f;
    DetectionRadius    = 2500.0f;
    TerritoryRadius    = 3000.0f;
    Mass               = 80.0f;

    // --- Pack configuration ---
    bIsPackHunter           = true;
    MaxPackSize             = 6;
    PackCoordinationRadius  = 1500.0f;
    PackCallRadius          = 4000.0f;
    bIsPackAlpha            = false;

    // --- Combat tuning ---
    LeapDamage      = 180.0f;
    LeapRange       = 600.0f;
    LeapCooldown    = 4.0f;
    PackCallCooldown = 12.0f;

    // --- Timing ---
    LastLeapTime     = -999.0f;
    LastPackCallTime = -999.0f;
    CurrentFlankAngle = 0.0f;

    // --- Movement: fast, agile ---
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed        = MoveSpeed;
        GetCharacterMovement()->MaxAcceleration     = 4096.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
        GetCharacterMovement()->JumpZVelocity       = 700.0f;
        GetCharacterMovement()->AirControl          = 0.4f;
        GetCharacterMovement()->GravityScale        = 1.2f;
    }

    // Capsule: smaller than T-Rex
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCapsuleHalfHeight(60.0f);
        GetCapsuleComponent()->SetCapsuleRadius(30.0f);
    }
}

void AVelociraptorDinosaur::BeginPlay()
{
    Super::BeginPlay();

    // Raptors update behavior more frequently than T-Rex (pack coordination needs faster ticks)
    GetWorldTimerManager().ClearTimer(BehaviorTimerHandle);
    GetWorldTimerManager().SetTimer(
        BehaviorTimerHandle,
        this,
        &AVelociraptorDinosaur::UpdateBehavior,
        0.25f,   // 4 Hz — faster than base 2 Hz for responsive pack AI
        true
    );
}

void AVelociraptorDinosaur::UpdateBehavior()
{
    // Call base behavior (state machine: Idle → Alert → Hunting → Fleeing)
    Super::UpdateBehavior();

    if (!CurrentTarget)
        return;

    const float Now = GetWorld()->GetTimeSeconds();
    const float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    // --- Pack Alpha: call pack when target first spotted ---
    if (bIsPackAlpha && (Now - LastPackCallTime) > PackCallCooldown)
    {
        CallPack();
    }

    // --- Flanking: circle the target while hunting ---
    if (CurrentState == EDinosaurState::Hunting && DistToTarget < PackCoordinationRadius)
    {
        FlankTarget();
    }

    // --- Leap attack: when close enough ---
    if (DistToTarget < LeapRange && (Now - LastLeapTime) > LeapCooldown)
    {
        PerformLeapAttack();
    }
}

void AVelociraptorDinosaur::PerformLeapAttack()
{
    if (!CurrentTarget || !GetWorld())
        return;

    const float Now = GetWorld()->GetTimeSeconds();
    if ((Now - LastLeapTime) < LeapCooldown)
        return;

    LastLeapTime = Now;

    // Launch raptor toward target
    const FVector ToTarget = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    const FVector LaunchVelocity = ToTarget * 1400.0f + FVector(0.0f, 0.0f, 400.0f);
    LaunchCharacter(LaunchVelocity, true, true);

    // Apply damage after brief delay (landing impact)
    FTimerHandle LeapDamageTimer;
    GetWorldTimerManager().SetTimer(LeapDamageTimer, [this]()
    {
        ExecuteLeap(CurrentTarget);
    }, 0.35f, false);

    UE_LOG(LogTemp, Log, TEXT("AVelociraptorDinosaur [%s]: LeapAttack launched toward %s"),
        *GetActorLabel(), CurrentTarget ? *CurrentTarget->GetActorLabel() : TEXT("NULL"));
}

void AVelociraptorDinosaur::ExecuteLeap(AActor* Target)
{
    if (!Target || !GetWorld())
        return;

    const float DistAtLanding = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistAtLanding < LeapRange * 1.5f)
    {
        // Deal leap damage
        UGameplayStatics::ApplyDamage(
            Target,
            LeapDamage,
            GetController(),
            this,
            UDamageType::StaticClass()
        );

        UE_LOG(LogTemp, Log, TEXT("AVelociraptorDinosaur [%s]: LeapAttack HIT %s for %.0f DMG"),
            *GetActorLabel(), *Target->GetActorLabel(), LeapDamage);
    }
}

void AVelociraptorDinosaur::FlankTarget()
{
    if (!CurrentTarget || !GetWorld())
        return;

    // Advance flank angle — each raptor in the pack gets a different offset
    // Alpha at 0°, Beta at 120°, Gamma at 240° etc.
    CurrentFlankAngle += 2.0f; // degrees per tick
    if (CurrentFlankAngle >= 360.0f)
        CurrentFlankAngle -= 360.0f;

    const float AngleRad = FMath::DegreesToRadians(CurrentFlankAngle);
    const float FlankRadius = 350.0f; // orbit radius around target

    const FVector TargetLoc = CurrentTarget->GetActorLocation();
    const FVector FlankPos = TargetLoc + FVector(
        FMath::Cos(AngleRad) * FlankRadius,
        FMath::Sin(AngleRad) * FlankRadius,
        0.0f
    );

    // Move toward flank position via AI movement (if controller available)
    if (GetController())
    {
        // Use MoveToLocation via AIController if available
        // Falls back gracefully if no AIController is set
        FVector Direction = (FlankPos - GetActorLocation()).GetSafeNormal();
        AddMovementInput(Direction, 0.8f);
    }
}

void AVelociraptorDinosaur::CallPack()
{
    if (!GetWorld())
        return;

    const float Now = GetWorld()->GetTimeSeconds();
    if ((Now - LastPackCallTime) < PackCallCooldown)
        return;

    LastPackCallTime = Now;

    TArray<AVelociraptorDinosaur*> PackMembers = FindPackMembers();

    for (AVelociraptorDinosaur* Member : PackMembers)
    {
        if (Member && Member != this && CurrentTarget)
        {
            // Share target with pack members
            Member->CurrentTarget = CurrentTarget;
            Member->CurrentState  = EDinosaurState::Hunting;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AVelociraptorDinosaur [%s]: PackCall — alerted %d pack members"),
        *GetActorLabel(), PackMembers.Num());
}

TArray<AVelociraptorDinosaur*> AVelociraptorDinosaur::FindPackMembers() const
{
    TArray<AVelociraptorDinosaur*> Members;
    if (!GetWorld())
        return Members;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVelociraptorDinosaur::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        AVelociraptorDinosaur* Raptor = Cast<AVelociraptorDinosaur>(Actor);
        if (Raptor && Raptor != this)
        {
            const float Dist = FVector::Dist(GetActorLocation(), Raptor->GetActorLocation());
            if (Dist <= PackCallRadius && Members.Num() < MaxPackSize)
            {
                Members.Add(Raptor);
            }
        }
    }

    return Members;
}
