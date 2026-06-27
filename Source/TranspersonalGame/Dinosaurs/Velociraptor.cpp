#include "Velociraptor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

AVelociraptor::AVelociraptor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Species stats — fast, fragile pack hunter
    // HP set via DinosaurBase properties
    // Speed: 800 cm/s (~29 km/h sprint — realistic dromaeosaurid)
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 800.0f;
        GetCharacterMovement()->MaxAcceleration = 2048.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 1024.0f;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f); // Agile turning
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->JumpZVelocity = 600.0f; // Can leap
        GetCharacterMovement()->AirControl = 0.3f;
    }

    // Pack leader by default — overridden when joining a pack
    bIsPackLeader = true;
    PackLeader = nullptr;
}

void AVelociraptor::BeginPlay()
{
    Super::BeginPlay();

    // Scan for nearby pack members every 5 seconds
    GetWorldTimerManager().SetTimer(
        PackScanTimer,
        this,
        &AVelociraptor::ScanForPackMembers,
        5.0f,
        true,
        1.0f
    );
}

void AVelociraptor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Decrement leap cooldown
    if (LeapCooldownRemaining > 0.0f)
    {
        LeapCooldownRemaining -= DeltaTime;
    }
}

void AVelociraptor::SetPackLeader(AVelociraptor* Leader)
{
    if (!Leader || Leader == this)
    {
        return;
    }

    PackLeader = Leader;
    bIsPackLeader = false;

    // Register with leader's pack
    if (!Leader->PackMembers.Contains(this))
    {
        Leader->PackMembers.Add(this);
    }
}

void AVelociraptor::ScanForPackMembers()
{
    if (!bIsPackLeader)
    {
        return; // Only leader manages pack
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find nearby Velociraptors within pack radius
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(PackDetectionRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    World->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        Sphere,
        Params
    );

    PackMembers.Empty();
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AVelociraptor* OtherRaptor = Cast<AVelociraptor>(Overlap.GetActor());
        if (OtherRaptor && OtherRaptor != this)
        {
            PackMembers.AddUnique(OtherRaptor);
            OtherRaptor->SetPackLeader(this);
        }
    }

    if (PackMembers.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Velociraptor pack: %s leading %d members"),
            *GetName(), PackMembers.Num());
    }
}

void AVelociraptor::CallPackToHunt(AActor* Target)
{
    if (!Target || !bIsPackLeader)
    {
        return;
    }

    // Signal all pack members to hunt the same target
    for (AVelociraptor* Member : PackMembers)
    {
        if (Member && IsValid(Member))
        {
            Member->ExecuteFlankingManeuver(Target);
        }
    }

    // Leader attacks from front
    PerformLeapAttack(Target);
}

void AVelociraptor::ExecuteFlankingManeuver(AActor* Target)
{
    if (!Target)
    {
        return;
    }

    // Calculate flanking position offset from target
    // Each pack member takes a different angle based on their index
    int32 MemberIndex = 0;
    if (PackLeader)
    {
        MemberIndex = PackLeader->PackMembers.IndexOfByKey(this);
    }

    float AngleOffset = FlankingAngle * MemberIndex;
    FVector TargetLocation = Target->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - GetActorLocation()).GetSafeNormal();

    // Rotate direction by flank angle
    FRotator FlankRotation(0.0f, AngleOffset, 0.0f);
    FVector FlankDirection = FlankRotation.RotateVector(DirectionToTarget);
    FVector FlankPosition = TargetLocation - (FlankDirection * 300.0f); // 3m from target

    // Move to flanking position then attack
    CoordinateFlankPosition(Target);
}

void AVelociraptor::CoordinateFlankPosition(AActor* Target)
{
    if (!Target)
    {
        return;
    }

    // Once in range, perform leap attack
    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget <= LeapAttackRange)
    {
        PerformLeapAttack(Target);
    }
}

void AVelociraptor::PerformLeapAttack(AActor* Target)
{
    if (!Target || LeapCooldownRemaining > 0.0f)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistToTarget > LeapAttackRange * 1.5f)
    {
        return; // Out of leap range
    }

    // Apply leap impulse toward target
    FVector LeapDirection = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector LeapVelocity = LeapDirection * 900.0f + FVector(0.0f, 0.0f, 400.0f);

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->Launch(LeapVelocity);
    }

    // Apply damage
    UGameplayStatics::ApplyDamage(
        Target,
        LeapAttackDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    // Set cooldown
    LeapCooldownRemaining = LeapAttackCooldown;

    UE_LOG(LogTemp, Log, TEXT("Velociraptor %s performed leap attack on %s for %.1f damage"),
        *GetName(), *Target->GetName(), LeapAttackDamage);
}
