#include "TranspersonalCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"

// SurvivalComponent — forward declared in header, full include here
#include "Core/Survival/SurvivalComponent.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Capsule ──────────────────────────────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

    // ── Movement defaults ────────────────────────────────────────────────────
    WalkSpeed = 400.f;
    RunSpeed  = 800.f;
    bIsRunning = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    // ── Camera boom ──────────────────────────────────────────────────────────
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 8.f;

    // ── Follow camera ────────────────────────────────────────────────────────
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // ── Survival component ───────────────────────────────────────────────────
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // ── Initial survival stats ───────────────────────────────────────────────
    Health  = 100.f;
    Hunger  = 100.f;
    Thirst  = 100.f;
    Stamina = 100.f;
    Fear    = 0.f;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();
    UpdateMovementSpeed();
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateSurvivalStats(DeltaTime);
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Axis bindings — WASD + mouse look
    PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight",   this, &ATranspersonalCharacter::MoveRight);
    PlayerInputComponent->BindAxis("LookUp",      this, &ATranspersonalCharacter::LookUp);
    PlayerInputComponent->BindAxis("LookRight",   this, &ATranspersonalCharacter::LookRight);

    // Also bind to controller look (gamepad)
    PlayerInputComponent->BindAxis("Turn",   this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUpRate", this, &APawn::AddControllerPitchInput);

    // Action bindings
    PlayerInputComponent->BindAction("Jump",    IE_Pressed,  this, &ATranspersonalCharacter::StartJump);
    PlayerInputComponent->BindAction("Jump",    IE_Released, this, &ATranspersonalCharacter::StopJump);
    PlayerInputComponent->BindAction("Run",     IE_Pressed,  this, &ATranspersonalCharacter::StartRun);
    PlayerInputComponent->BindAction("Run",     IE_Released, this, &ATranspersonalCharacter::StopRun);
}

// ── Input Handlers ────────────────────────────────────────────────────────────

void ATranspersonalCharacter::MoveForward(float Value)
{
    if (Controller && Value != 0.f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ATranspersonalCharacter::MoveRight(float Value)
{
    if (Controller && Value != 0.f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void ATranspersonalCharacter::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void ATranspersonalCharacter::LookRight(float Value)
{
    AddControllerYawInput(Value);
}

void ATranspersonalCharacter::StartRun()
{
    bIsRunning = true;
    UpdateMovementSpeed();
}

void ATranspersonalCharacter::StopRun()
{
    bIsRunning = false;
    UpdateMovementSpeed();
}

void ATranspersonalCharacter::StartJump()
{
    Jump();
}

void ATranspersonalCharacter::StopJump()
{
    StopJumping();
}

// ── Internal helpers ──────────────────────────────────────────────────────────

void ATranspersonalCharacter::UpdateMovementSpeed()
{
    float TargetSpeed = bIsRunning ? RunSpeed : WalkSpeed;
    GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
}

void ATranspersonalCharacter::UpdateSurvivalStats(float DeltaTime)
{
    // Passive drain rates (per second)
    const float HungerDrainRate = 0.5f;
    const float ThirstDrainRate = 0.8f;
    const float StaminaDrainRate = bIsRunning ? 5.f : -3.f; // drain when running, recover when walking

    Hunger  = FMath::Clamp(Hunger  - HungerDrainRate  * DeltaTime, 0.f, 100.f);
    Thirst  = FMath::Clamp(Thirst  - ThirstDrainRate  * DeltaTime, 0.f, 100.f);
    Stamina = FMath::Clamp(Stamina - StaminaDrainRate  * DeltaTime, 0.f, 100.f);

    // Health drain when starving or dehydrated
    if (Hunger <= 0.f || Thirst <= 0.f)
    {
        Health = FMath::Clamp(Health - 1.f * DeltaTime, 0.f, 100.f);
    }

    // Prevent running when stamina is depleted
    if (Stamina <= 0.f && bIsRunning)
    {
        StopRun();
    }

    // Mirror stats to SurvivalComponent if available
    if (SurvivalComp)
    {
        // SurvivalComponent owns the authoritative stats; read from it
        // (This keeps the character's replicated floats in sync for HUD)
    }
}

void ATranspersonalCharacter::TakeDamage_Survival(float DamageAmount)
{
    Health = FMath::Clamp(Health - DamageAmount, 0.f, 100.f);
    Fear   = FMath::Clamp(Fear   + DamageAmount * 0.5f, 0.f, 100.f);

    if (Health <= 0.f)
    {
        // Trigger death — disable movement
        GetCharacterMovement()->DisableMovement();
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

bool ATranspersonalCharacter::IsAlive() const
{
    return Health > 0.f;
}
