// TranspersonalCharacter.cpp — Transpersonal Game Studio
// Core Systems Programmer — Agent #3
// Primitive human survivor: WASD movement, sprint, jump, survival stats

#include "TranspersonalCharacter.h"
#include "Core/Survival/SurvivalComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Capsule ---
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // --- Camera boom (spring arm) ---
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 60.0f);

    // --- Follow camera ---
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // --- Survival component ---
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // --- Movement defaults ---
    WalkSpeed   = 400.0f;
    SprintSpeed = 700.0f;
    bIsSprinting = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 420.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->GravityScale = 1.0f;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Bind survival death delegate
    if (SurvivalComp)
    {
        SurvivalComp->OnDeath.AddDynamic(this, &ATranspersonalCharacter::OnSurvivalDeath);
    }

    // Set up Enhanced Input
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Drain stamina while sprinting
    if (bIsSprinting && SurvivalComp)
    {
        SurvivalComp->DrainStamina(8.0f * DeltaTime);
        if (!SurvivalComp->HasEnoughStamina(1.0f))
        {
            StopSprint();
        }
    }
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Move);
        }
        if (LookAction)
        {
            EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Look);
        }
        if (JumpAction)
        {
            EIC->BindAction(JumpAction, ETriggerEvent::Started,   this, &ACharacter::Jump);
            EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }
        if (SprintAction)
        {
            EIC->BindAction(SprintAction, ETriggerEvent::Started,   this, &ATranspersonalCharacter::StartSprint);
            EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATranspersonalCharacter::StopSprint);
        }
    }
}

void ATranspersonalCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDir   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(ForwardDir, MovementVector.Y);
        AddMovementInput(RightDir,   MovementVector.X);
    }
}

void ATranspersonalCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookVector = Value.Get<FVector2D>();
    if (Controller)
    {
        AddControllerYawInput(LookVector.X);
        AddControllerPitchInput(LookVector.Y);
    }
}

void ATranspersonalCharacter::StartSprint()
{
    if (!SurvivalComp || !SurvivalComp->HasEnoughStamina(10.0f)) return;
    bIsSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ATranspersonalCharacter::StopSprint()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ATranspersonalCharacter::OnSurvivalDeath()
{
    // Disable input, play death
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        DisableInput(PC);
    }
    GetCharacterMovement()->DisableMovement();
    // Ragdoll
    GetMesh()->SetSimulatePhysics(true);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

float ATranspersonalCharacter::GetHealthPercent() const
{
    return SurvivalComp ? SurvivalComp->GetHealthPercent() : 1.0f;
}

float ATranspersonalCharacter::GetHungerPercent() const
{
    return SurvivalComp ? SurvivalComp->GetHungerPercent() : 1.0f;
}

float ATranspersonalCharacter::GetThirstPercent() const
{
    return SurvivalComp ? SurvivalComp->GetThirstPercent() : 1.0f;
}

float ATranspersonalCharacter::GetStaminaPercent() const
{
    return SurvivalComp ? SurvivalComp->GetStaminaPercent() : 1.0f;
}

float ATranspersonalCharacter::GetFearPercent() const
{
    return SurvivalComp ? SurvivalComp->GetFearPercent() : 0.0f;
}
