// TranspersonalCharacter.cpp
// Core Systems Programmer — Agent #03
// Prehistoric survival character with full SurvivalComponent integration

#include "TranspersonalCharacter.h"
#include "Core/Survival/SurvivalComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

    // Movement defaults — prehistoric human
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
    GetCharacterMovement()->JumpZVelocity = 420.f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = 300.f;   // Walk speed
    GetCharacterMovement()->MaxWalkSpeedCrouched = 150.f;
    GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

    // Camera boom
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->SocketOffset = FVector(0.f, 0.f, 60.f);

    // Follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Survival component — core gameplay system
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // Character state defaults
    bIsSprinting = false;
    bIsCrouching = false;
    SprintSpeedMultiplier = 1.8f;
    BaseWalkSpeed = 300.f;
    StaminaDrainRate = 10.f;
    StaminaRecoveryRate = 5.f;

    // Don't rotate character with controller — camera handles it
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Start survival tick timer — every 5 seconds update survival stats
    if (SurvivalComp)
    {
        GetWorldTimerManager().SetTimer(
            SurvivalTickHandle,
            this,
            &ATranspersonalCharacter::UpdateSurvivalStats,
            5.0f,
            true
        );
    }
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Sprint stamina drain
    if (bIsSprinting && SurvivalComp)
    {
        float CurrentStamina = SurvivalComp->GetStamina();
        if (CurrentStamina <= 0.f)
        {
            StopSprinting();
        }
        else
        {
            SurvivalComp->ModifyStamina(-StaminaDrainRate * DeltaTime);
        }
    }
    else if (!bIsSprinting && SurvivalComp)
    {
        // Recover stamina when not sprinting
        float CurrentStamina = SurvivalComp->GetStamina();
        float MaxStamina = SurvivalComp->GetMaxStamina();
        if (CurrentStamina < MaxStamina)
        {
            SurvivalComp->ModifyStamina(StaminaRecoveryRate * DeltaTime);
        }
    }
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Movement
    PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ATranspersonalCharacter::MoveRight);

    // Camera
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

    // Actions
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ATranspersonalCharacter::StartSprinting);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATranspersonalCharacter::StopSprinting);
    PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ATranspersonalCharacter::ToggleCrouch);
    PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ATranspersonalCharacter::Interact);
}

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

void ATranspersonalCharacter::StartSprinting()
{
    if (SurvivalComp && SurvivalComp->GetStamina() > 10.f)
    {
        bIsSprinting = true;
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * SprintSpeedMultiplier;
    }
}

void ATranspersonalCharacter::StopSprinting()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
}

void ATranspersonalCharacter::ToggleCrouch()
{
    if (bIsCrouching)
    {
        UnCrouch();
        bIsCrouching = false;
    }
    else
    {
        Crouch();
        bIsCrouching = true;
    }
}

void ATranspersonalCharacter::Interact()
{
    // Raycast for interactable objects in the world
    FVector Start = FollowCamera->GetComponentLocation();
    FVector End = Start + FollowCamera->GetForwardVector() * 300.f;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
    if (bHit && HitResult.GetActor())
    {
        // Broadcast interaction event — picked up by quest/inventory systems
        OnInteract.Broadcast(HitResult.GetActor());
    }
}

void ATranspersonalCharacter::UpdateSurvivalStats()
{
    if (!SurvivalComp) return;

    // Passive hunger/thirst drain — prehistoric survival
    SurvivalComp->ModifyHunger(-2.0f);   // Lose 2 hunger per 5 seconds
    SurvivalComp->ModifyThirst(-3.0f);   // Lose 3 thirst per 5 seconds

    // Temperature effect on health
    float Temp = SurvivalComp->GetBodyTemperature();
    if (Temp < 35.f || Temp > 40.f)
    {
        // Hypothermia or hyperthermia — damage health
        SurvivalComp->ModifyHealth(-1.0f);
    }

    // Starvation damage
    if (SurvivalComp->GetHunger() <= 0.f)
    {
        SurvivalComp->ModifyHealth(-2.0f);
    }

    // Dehydration damage
    if (SurvivalComp->GetThirst() <= 0.f)
    {
        SurvivalComp->ModifyHealth(-3.0f);
    }
}

float ATranspersonalCharacter::GetHealthPercent() const
{
    if (SurvivalComp)
    {
        return SurvivalComp->GetHealth() / SurvivalComp->GetMaxHealth();
    }
    return 1.f;
}

float ATranspersonalCharacter::GetHungerPercent() const
{
    if (SurvivalComp)
    {
        return SurvivalComp->GetHunger() / SurvivalComp->GetMaxHunger();
    }
    return 1.f;
}

float ATranspersonalCharacter::GetThirstPercent() const
{
    if (SurvivalComp)
    {
        return SurvivalComp->GetThirst() / SurvivalComp->GetMaxThirst();
    }
    return 1.f;
}

float ATranspersonalCharacter::GetStaminaPercent() const
{
    if (SurvivalComp)
    {
        return SurvivalComp->GetStamina() / SurvivalComp->GetMaxStamina();
    }
    return 1.f;
}

bool ATranspersonalCharacter::IsAlive() const
{
    if (SurvivalComp)
    {
        return SurvivalComp->GetHealth() > 0.f;
    }
    return true;
}
