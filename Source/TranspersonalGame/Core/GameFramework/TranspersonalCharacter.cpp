// TranspersonalCharacter.cpp — Agent #3 Core Systems Programmer
// Prehistoric survival game — human primitive vs dinosaur world
// CYCLE: PROD_CYCLE_AUTO_20260623_010
#include "TranspersonalCharacter.h"
#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Survival Component ───────────────────────────────────────────────────
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // ── Camera Boom ──────────────────────────────────────────────────────────
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // ── Follow Camera ────────────────────────────────────────────────────────
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // ── Movement defaults ────────────────────────────────────────────────────
    WalkSpeed  = 300.0f;
    SprintSpeed = 600.0f;
    CrouchSpeed = 150.0f;
    bIsSprinting = false;

    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->JumpZVelocity = 420.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    bUseControllerRotationYaw = false;

    // ── Survival defaults ────────────────────────────────────────────────────
    Health   = 100.0f;
    Hunger   = 100.0f;
    Thirst   = 100.0f;
    Stamina  = 100.0f;
    Fear     = 0.0f;

    VolcanicHeatDamagePerSecond = 5.0f;
    DesertHeatDamagePerSecond   = 1.5f;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();
    SyncSurvivalStats();
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Drain stamina while sprinting
    if (bIsSprinting && SurvivalComp)
    {
        SurvivalComp->DrainStamina(15.0f * DeltaTime);
        if (SurvivalComp->GetStamina() <= 0.0f)
        {
            StopSprint();
        }
    }

    SyncSurvivalStats();
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight",   this, &ATranspersonalCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn",        this, &ATranspersonalCharacter::Turn);
    PlayerInputComponent->BindAxis("LookUp",      this, &ATranspersonalCharacter::LookUp);

    PlayerInputComponent->BindAction("Sprint", IE_Pressed,  this, &ATranspersonalCharacter::StartSprint);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATranspersonalCharacter::StopSprint);
    PlayerInputComponent->BindAction("Crouch", IE_Pressed,  this, &ATranspersonalCharacter::StartCrouch);
    PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ATranspersonalCharacter::StopCrouch);
    PlayerInputComponent->BindAction("Jump",   IE_Pressed,  this, &ATranspersonalCharacter::Jump);
}

// ── Movement ──────────────────────────────────────────────────────────────────

void ATranspersonalCharacter::MoveForward(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ATranspersonalCharacter::MoveRight(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void ATranspersonalCharacter::Turn(float Value)
{
    AddControllerYawInput(Value);
}

void ATranspersonalCharacter::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void ATranspersonalCharacter::StartSprint()
{
    if (SurvivalComp && SurvivalComp->GetStamina() > 10.0f)
    {
        bIsSprinting = true;
        GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    }
}

void ATranspersonalCharacter::StopSprint()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ATranspersonalCharacter::StartCrouch()
{
    Crouch();
    GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
}

void ATranspersonalCharacter::StopCrouch()
{
    UnCrouch();
    GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
}

void ATranspersonalCharacter::Jump()
{
    if (SurvivalComp && SurvivalComp->GetStamina() > 5.0f)
    {
        Super::Jump();
        SurvivalComp->DrainStamina(5.0f);
    }
}

// ── Survival ──────────────────────────────────────────────────────────────────

void ATranspersonalCharacter::ApplyBiomeDamage(float DeltaTime, bool bInVolcanic, bool bInDesert)
{
    if (!SurvivalComp) return;

    if (bInVolcanic)
    {
        SurvivalComp->ApplyDamage(VolcanicHeatDamagePerSecond * DeltaTime);
        SurvivalComp->DrainThirst(2.0f * DeltaTime); // dehydration from heat
    }
    else if (bInDesert)
    {
        SurvivalComp->ApplyDamage(DesertHeatDamagePerSecond * DeltaTime);
        SurvivalComp->DrainThirst(1.0f * DeltaTime);
    }
}

void ATranspersonalCharacter::SyncSurvivalStats()
{
    if (!SurvivalComp) return;

    Health  = SurvivalComp->GetHealth();
    Hunger  = SurvivalComp->GetHunger();
    Thirst  = SurvivalComp->GetThirst();
    Stamina = SurvivalComp->GetStamina();
    Fear    = SurvivalComp->GetFear();

    // Trigger death if health depleted
    if (Health <= 0.0f)
    {
        OnCharacterDeath();
    }
}

void ATranspersonalCharacter::OnCharacterDeath_Implementation()
{
    // Disable movement
    GetCharacterMovement()->DisableMovement();
    // Disable collision
    SetActorEnableCollision(false);
    // Broadcast death (Blueprint can override for ragdoll/respawn logic)
    unreal::log("ATranspersonalCharacter: Character died.");
}
