// DinosaurRaptor.cpp — Velociraptor implementation
// Agent #4 Performance Optimizer — Cycle 012
// Pack-hunting raptor with coordinated attack AI

#include "DinosaurRaptor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ARaptorCharacter::ARaptorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Raptor base stats — smaller, faster than T-Rex
    Health = 180.0f;
    MaxHealth = 180.0f;
    AttackDamage = ClawSlashDamage;  // 25.0f
    AttackRange = 200.0f;
    DetectionRadius = 2800.0f;
    MovementSpeed = 680.0f;
    bIsAggressive = true;
    DinosaurSpecies = TEXT("Velociraptor mongoliensis");

    // Fast movement
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
        GetCharacterMovement()->JumpZVelocity = JumpStrength;
        GetCharacterMovement()->AirControl = 0.4f;
        GetCharacterMovement()->bCanWalkOffLedges = true;
        GetCharacterMovement()->MaxAcceleration = 3200.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 2400.0f;
    }

    // Capsule — smaller than T-Rex
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCapsuleHalfHeight(80.0f);
        GetCapsuleComponent()->SetCapsuleRadius(35.0f);
    }
}

void ARaptorCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Scan for pack members on spawn
    ScanForPackMembers();

    // Apply LOD settings
    ApplyLODSettings();

    // Randomly assign pack leader role
    if (PackMembers.Num() == 0)
    {
        bIsPackLeader = true;
    }
}

void ARaptorCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update leap cooldown
    if (LeapCooldownRemaining > 0.0f)
    {
        LeapCooldownRemaining -= DeltaTime;
    }

    // Periodic pack scan (every 3 seconds via inherited timer logic)
    // Pack coordination handled by AI Controller
}

void ARaptorCharacter::ScanForPackMembers()
{
    PackMembers.Empty();

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARaptorCharacter::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;

        ARaptorCharacter* OtherRaptor = Cast<ARaptorCharacter>(Actor);
        if (!OtherRaptor) continue;

        float Dist = FVector::Dist(GetActorLocation(), OtherRaptor->GetActorLocation());
        if (Dist <= PackCoordinationRadius && PackMembers.Num() < MaxPackSize - 1)
        {
            PackMembers.Add(OtherRaptor);
        }
    }

    CurrentPackCount = PackMembers.Num() + 1; // +1 for self
}

void ARaptorCharacter::CoordinatePackAttack(AActor* Target)
{
    if (!Target) return;

    // Refresh pack list
    ScanForPackMembers();

    if (PackMembers.Num() == 0) return;

    // Assign flanking positions to each pack member
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        ARaptorCharacter* Member = PackMembers[i];
        if (!Member || !IsValid(Member)) continue;

        // Each member gets a different flank angle
        float AngleOffset = (360.0f / (PackMembers.Num() + 1)) * (i + 1);
        FVector FlankPos = CalculateFlankPosition(Target);

        // Rotate flank position around target
        FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
        FRotator Rot(0.0f, AngleOffset, 0.0f);
        FVector RotatedOffset = Rot.RotateVector(ToTarget.GetSafeNormal() * StalkingDistance);
        FVector AssignedPos = Target->GetActorLocation() + RotatedOffset;

        // Signal member to move to flank position (via blackboard in AI controller)
        // This is a simplified signal — full BT integration in Agent #12
        Member->SetActorLocation(AssignedPos, true);
    }
}

bool ARaptorCharacter::TryLeapAttack(AActor* Target)
{
    if (!Target) return false;
    if (!bCanLeapAtTarget) return false;
    if (LeapCooldownRemaining > 0.0f) return false;

    float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (Dist > LeapAttackRange) return false;

    // Launch toward target
    FVector Direction = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector LaunchVelocity = Direction * 1200.0f + FVector(0.0f, 0.0f, 400.0f);
    LaunchCharacter(LaunchVelocity, true, true);

    bIsLeaping = true;
    LeapCooldownRemaining = LeapCooldown;

    // Apply leap damage
    if (UHealthComponent* HealthComp = Target->FindComponentByClass<UHealthComponent>())
    {
        // Damage applied on landing — handled by overlap event
    }

    return true;
}

void ARaptorCharacter::PerformClawSlash()
{
    // Sphere trace for nearby targets
    TArray<FHitResult> HitResults;
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * AttackRange;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    GetWorld()->SweepMultiByChannel(
        HitResults,
        Start,
        End,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(80.0f),
        Params
    );

    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor || HitActor == this) continue;

        // Apply claw damage
        UGameplayStatics::ApplyDamage(
            HitActor,
            ClawSlashDamage,
            GetController(),
            this,
            UDamageType::StaticClass()
        );
    }
}

FVector ARaptorCharacter::CalculateFlankPosition(AActor* Target)
{
    if (!Target) return GetActorLocation();

    FVector TargetLoc = Target->GetActorLocation();
    FVector TargetForward = Target->GetActorForwardVector();

    // Calculate perpendicular flank position
    FVector Right = FVector::CrossProduct(TargetForward, FVector::UpVector).GetSafeNormal();
    FVector FlankOffset = Right * StalkingDistance;

    // Alternate left/right based on actor ID
    if (GetUniqueID() % 2 == 0)
    {
        FlankOffset = -FlankOffset;
    }

    return TargetLoc + FlankOffset;
}

void ARaptorCharacter::ApplyLODSettings()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp) return;

    // Set LOD distances for performance
    // LOD0 = full detail within 1500 units
    // LOD1 = medium detail 1500-3500
    // LOD2 = low detail 3500-7000
    // Beyond 7000 = culled

    MeshComp->SetCullDistance(LOD2_Distance);
    MeshComp->bEnablePerPolyCollision = false;

    // Disable expensive features at distance
    MeshComp->bCastDynamicShadow = true;
    MeshComp->bAffectDynamicIndirectLighting = true;
    MeshComp->bReceivesDecals = true;
}
