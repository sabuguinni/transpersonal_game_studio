#include "PrehistoricCharacterSetup.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/Engine.h"

APrehistoricCharacterSetup::APrehistoricCharacterSetup()
{
    PrimaryActorTick.bCanEverTick = true;

    // ─── Capsule ──────────────────────────────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // ─── Movement ─────────────────────────────────────────────────────────────
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->GravityScale = 1.0f;

    // ─── Camera Boom ──────────────────────────────────────────────────────────
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 60.0f);
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 8.0f;

    // ─── Follow Camera ────────────────────────────────────────────────────────
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    FollowCamera->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));

    // ─── Default Survival Stats ───────────────────────────────────────────────
    SurvivalStats.Health = 1.0f;
    SurvivalStats.Hunger = 1.0f;
    SurvivalStats.Thirst = 1.0f;
    SurvivalStats.Stamina = 1.0f;
    SurvivalStats.Fear = 0.0f;
    SurvivalStats.Temperature = 0.5f;
}

void APrehistoricCharacterSetup::BeginPlay()
{
    Super::BeginPlay();

    // Apply initial movement speed
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    UE_LOG(LogTemp, Log, TEXT("PrehistoricCharacterSetup: BeginPlay — Character initialized"));
    UE_LOG(LogTemp, Log, TEXT("  Health: %.2f | Hunger: %.2f | Thirst: %.2f"),
        SurvivalStats.Health, SurvivalStats.Hunger, SurvivalStats.Thirst);
}

void APrehistoricCharacterSetup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TickSurvivalStats(DeltaTime);
}

void APrehistoricCharacterSetup::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &APrehistoricCharacterSetup::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &APrehistoricCharacterSetup::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("TurnRate", this, &APrehistoricCharacterSetup::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("LookUpRate", this, &APrehistoricCharacterSetup::LookUpAtRate);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APrehistoricCharacterSetup::StartSprint);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APrehistoricCharacterSetup::StopSprint);
}

// ─── Movement ─────────────────────────────────────────────────────────────────

void APrehistoricCharacterSetup::MoveForward(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void APrehistoricCharacterSetup::MoveRight(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void APrehistoricCharacterSetup::TurnAtRate(float Rate)
{
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APrehistoricCharacterSetup::LookUpAtRate(float Rate)
{
    AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void APrehistoricCharacterSetup::StartSprint()
{
    if (SurvivalStats.Stamina > 0.1f)
    {
        bIsSprinting = true;
        GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    }
}

void APrehistoricCharacterSetup::StopSprint()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

// ─── Survival Stats ───────────────────────────────────────────────────────────

void APrehistoricCharacterSetup::TickSurvivalStats(float DeltaTime)
{
    if (!IsAlive()) return;

    // Drain hunger and thirst over time
    SurvivalStats.Hunger = FMath::Max(0.0f, SurvivalStats.Hunger - HungerDrainRate * DeltaTime);
    SurvivalStats.Thirst = FMath::Max(0.0f, SurvivalStats.Thirst - ThirstDrainRate * DeltaTime);

    // Stamina drain when sprinting, regen when not
    if (bIsSprinting && GetVelocity().SizeSquared() > 100.0f)
    {
        SurvivalStats.Stamina = FMath::Max(0.0f, SurvivalStats.Stamina - StaminaDrainRate * DeltaTime);
        if (SurvivalStats.Stamina <= 0.05f)
        {
            StopSprint();
        }
    }
    else
    {
        SurvivalStats.Stamina = FMath::Min(1.0f, SurvivalStats.Stamina + StaminaRegenRate * DeltaTime);
    }

    // Health drain from starvation/dehydration
    if (IsStarving())
    {
        SurvivalStats.Health = FMath::Max(0.0f, SurvivalStats.Health - 0.001f * DeltaTime);
    }
    if (IsDehydrated())
    {
        SurvivalStats.Health = FMath::Max(0.0f, SurvivalStats.Health - 0.002f * DeltaTime);
    }
}

void APrehistoricCharacterSetup::ApplyDamage(float Amount)
{
    SurvivalStats.Health = FMath::Max(0.0f, SurvivalStats.Health - Amount);
    SurvivalStats.Fear = FMath::Min(1.0f, SurvivalStats.Fear + Amount * 0.5f);

    if (!IsAlive())
    {
        UE_LOG(LogTemp, Warning, TEXT("PrehistoricCharacterSetup: Character has died!"));
        // Trigger death — handled by Blueprint or GameMode
    }
}

void APrehistoricCharacterSetup::Eat(float NutritionValue)
{
    SurvivalStats.Hunger = FMath::Min(1.0f, SurvivalStats.Hunger + NutritionValue);
    SurvivalStats.Fear = FMath::Max(0.0f, SurvivalStats.Fear - 0.1f);
    UE_LOG(LogTemp, Log, TEXT("PrehistoricCharacterSetup: Ate — Hunger now %.2f"), SurvivalStats.Hunger);
}

void APrehistoricCharacterSetup::Drink(float HydrationValue)
{
    SurvivalStats.Thirst = FMath::Min(1.0f, SurvivalStats.Thirst + HydrationValue);
    UE_LOG(LogTemp, Log, TEXT("PrehistoricCharacterSetup: Drank — Thirst now %.2f"), SurvivalStats.Thirst);
}

void APrehistoricCharacterSetup::ApplyAppearance(const FChar_AppearanceData& NewAppearance)
{
    AppearanceData = NewAppearance;
    UE_LOG(LogTemp, Log, TEXT("PrehistoricCharacterSetup: Appearance applied — ClothingVariant=%d, TribalMarkings=%s"),
        AppearanceData.ClothingVariant,
        AppearanceData.bHasTribalMarkings ? TEXT("true") : TEXT("false"));
}
