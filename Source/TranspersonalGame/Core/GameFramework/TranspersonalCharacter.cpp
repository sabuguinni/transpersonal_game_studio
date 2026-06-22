// TranspersonalCharacter.cpp
// Core Systems Programmer — Agent #03
// Prehistoric survival character: WASD movement, sprint, jump, survival stats tick.

#include "TranspersonalCharacter.h"
#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Survival Component ──────────────────────────────────────────────────
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // ── Camera Boom ─────────────────────────────────────────────────────────
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.f;
    CameraBoom->bUsePawnControlRotation = true;

    // ── Follow Camera ────────────────────────────────────────────────────────
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // ── Movement defaults ────────────────────────────────────────────────────
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate              = FRotator(0.f, 540.f, 0.f);
    GetCharacterMovement()->JumpZVelocity             = 600.f;
    GetCharacterMovement()->AirControl                = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed              = WalkSpeed;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Drain stamina while running
    if (bIsRunning && SurvivalComp)
    {
        const float CurrentStamina = SurvivalComp->GetStamina();
        if (CurrentStamina > 0.f)
        {
            SurvivalComp->ModifyStamina(-StaminaRunDrainRate * DeltaTime);
        }
        else
        {
            // Force stop run when stamina depleted
            StopRun();
        }
    }
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Axes
    PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight",   this, &ATranspersonalCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn",        this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp",      this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("TurnRate",    this, &ATranspersonalCharacter::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUpRate",  this, &ATranspersonalCharacter::LookUpAtRate);

    // Actions
    PlayerInputComponent->BindAction("Jump",    IE_Pressed,  this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump",    IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction("Sprint",  IE_Pressed,  this, &ATranspersonalCharacter::StartRun);
    PlayerInputComponent->BindAction("Sprint",  IE_Released, this, &ATranspersonalCharacter::StopRun);
}

// ── Movement ─────────────────────────────────────────────────────────────────

void ATranspersonalCharacter::MoveForward(float Value)
{
    if (Controller && Value != 0.f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
        const FVector  Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ATranspersonalCharacter::MoveRight(float Value)
{
    if (Controller && Value != 0.f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
        const FVector  Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void ATranspersonalCharacter::TurnAtRate(float Rate)
{
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ATranspersonalCharacter::LookUpAtRate(float Rate)
{
    AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ATranspersonalCharacter::StartRun()
{
    if (SurvivalComp && SurvivalComp->GetStamina() > 5.f)
    {
        bIsRunning = true;
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
    }
}

void ATranspersonalCharacter::StopRun()
{
    bIsRunning = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}
