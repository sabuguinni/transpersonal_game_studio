// TranspersonalCharacter.cpp
// Core Systems Programmer — Agent #03
// Prehistoric survival character: WASD movement, sprint, jump, survival stats integrated

#include "TranspersonalCharacter.h"
#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule size for a primitive human
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // Don't rotate character with camera — let movement component handle it
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Character movement settings — prehistoric human
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->bOrientRotationToMovement = true;
    MoveComp->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    MoveComp->JumpZVelocity = 420.0f;
    MoveComp->AirControl = 0.35f;
    MoveComp->MaxWalkSpeed = WalkSpeed;
    MoveComp->MinAnalogWalkSpeed = 20.0f;
    MoveComp->BrakingDecelerationWalking = 2000.0f;
    MoveComp->GravityScale = 1.0f;

    // Camera boom (spring arm)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 80.0f); // Slightly above character

    // Follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Survival component — core survival stats
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // Movement speeds
    WalkSpeed = 300.0f;
    SprintSpeed = 600.0f;
    CrouchSpeed = 150.0f;
    bIsSprinting = false;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Bind Enhanced Input
    if (APlayerController* PC = Cast<APlayerController>(Controller))
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

    // Sync sprint state with SurvivalComponent
    if (SurvivalComp)
    {
        // If survival component says we're exhausted, stop sprinting
        if (SurvivalComp->IsExhausted() && bIsSprinting)
        {
            StopSprint();
        }

        // Fear affects field of view — tunnel vision when terrified
        float FearPct = SurvivalComp->GetFearPercent();
        if (APlayerController* PC = Cast<APlayerController>(Controller))
        {
            float TargetFOV = FMath::Lerp(90.0f, 75.0f, FearPct); // Narrow FOV when afraid
            FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaTime, 2.0f));
        }
    }
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Movement
        if (MoveAction)
        {
            EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Move);
        }
        // Look
        if (LookAction)
        {
            EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Look);
        }
        // Jump
        if (JumpAction)
        {
            EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
            EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }
        // Sprint
        if (SprintAction)
        {
            EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::StartSprint);
            EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATranspersonalCharacter::StopSprint);
        }
        // Crouch
        if (CrouchAction)
        {
            EIC->BindAction(CrouchAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::ToggleCrouch);
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

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ATranspersonalCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ATranspersonalCharacter::StartSprint()
{
    if (SurvivalComp && SurvivalComp->IsExhausted()) return;

    bIsSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;

    if (SurvivalComp)
    {
        SurvivalComp->SetSprinting(true);
    }
}

void ATranspersonalCharacter::StopSprint()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    if (SurvivalComp)
    {
        SurvivalComp->SetSprinting(false);
    }
}

void ATranspersonalCharacter::ToggleCrouch()
{
    if (bIsCrouched)
    {
        UnCrouch();
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
    else
    {
        Crouch();
        GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
    }
}

// Survival convenience wrappers
void ATranspersonalCharacter::EatFood(float NutritionValue)
{
    if (SurvivalComp)
    {
        SurvivalComp->Eat(NutritionValue);
    }
}

void ATranspersonalCharacter::DrinkWater(float HydrationValue)
{
    if (SurvivalComp)
    {
        SurvivalComp->Drink(HydrationValue);
    }
}

void ATranspersonalCharacter::TakeSurvivalDamage(float Amount)
{
    if (SurvivalComp)
    {
        SurvivalComp->ApplyDamage(Amount);
    }
}

void ATranspersonalCharacter::AddFear(float FearAmount)
{
    if (SurvivalComp)
    {
        SurvivalComp->AddFear(FearAmount);
    }
}

bool ATranspersonalCharacter::IsAlive() const
{
    return SurvivalComp ? SurvivalComp->IsAlive() : false;
}

float ATranspersonalCharacter::GetHealthPercent() const
{
    return SurvivalComp ? SurvivalComp->GetHealthPercent() : 0.0f;
}

float ATranspersonalCharacter::GetStaminaPercent() const
{
    return SurvivalComp ? SurvivalComp->GetStaminaPercent() : 0.0f;
}
