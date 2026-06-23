// TranspersonalCharacter.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Prehistoric survival game — full implementation of player character

#include "Core/GameFramework/TranspersonalCharacter.h"
#include "Core/Survival/SurvivalComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Survival Component ────────────────────────────────────────────────────
    SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));

    // ── Camera boom ───────────────────────────────────────────────────────────
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // ── Follow camera ─────────────────────────────────────────────────────────
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // ── Movement defaults ─────────────────────────────────────────────────────
    WalkSpeed   = 300.0f;
    RunSpeed    = 600.0f;
    CrouchSpeed = 150.0f;
    JumpZVelocity = 420.0f;

    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->JumpZVelocity = JumpZVelocity;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    // ── Survival stat defaults ────────────────────────────────────────────────
    Health  = 100.0f;
    Hunger  = 100.0f;
    Thirst  = 100.0f;
    Stamina = 100.0f;
    Fear    = 0.0f;

    bIsRunning   = false;
    bIsCrouching = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────
void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Sync initial stats from SurvivalComp if it has defaults
    if (SurvivalComp)
    {
        SyncSurvivalStats();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick
// ─────────────────────────────────────────────────────────────────────────────
void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TickSurvivalStats(DeltaTime);
}

// ─────────────────────────────────────────────────────────────────────────────
// Input setup
// ─────────────────────────────────────────────────────────────────────────────
void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &ATranspersonalCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight",   this, &ATranspersonalCharacter::MoveRight);
    PlayerInputComponent->BindAxis("LookUp",      this, &ATranspersonalCharacter::LookUp);
    PlayerInputComponent->BindAxis("LookRight",   this, &ATranspersonalCharacter::LookRight);

    PlayerInputComponent->BindAction("Jump",   IE_Pressed,  this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump",   IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction("Run",    IE_Pressed,  this, &ATranspersonalCharacter::StartRun);
    PlayerInputComponent->BindAction("Run",    IE_Released, this, &ATranspersonalCharacter::StopRun);
    PlayerInputComponent->BindAction("Crouch", IE_Pressed,  this, &ATranspersonalCharacter::StartCrouch);
    PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ATranspersonalCharacter::StopCrouch);
}

// ─────────────────────────────────────────────────────────────────────────────
// Input handlers
// ─────────────────────────────────────────────────────────────────────────────
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
    if (Stamina > 5.0f)
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

void ATranspersonalCharacter::StartCrouch()
{
    bIsCrouching = true;
    Crouch();
    GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
}

void ATranspersonalCharacter::StopCrouch()
{
    bIsCrouching = false;
    UnCrouch();
    GetCharacterMovement()->MaxWalkSpeed = bIsRunning ? RunSpeed : WalkSpeed;
}

// ─────────────────────────────────────────────────────────────────────────────
// Survival actions
// ─────────────────────────────────────────────────────────────────────────────
void ATranspersonalCharacter::ApplyDamage(float Amount)
{
    Health = FMath::Clamp(Health - Amount, 0.0f, 100.0f);

    if (SurvivalComp)
    {
        // Mirror to component so other systems can query it
        SurvivalComp->SetEditorProperty("Health", Health);
    }

    if (Health <= 0.0f)
    {
        // Trigger death — handled by GameMode
        DisableInput(Cast<APlayerController>(GetController()));
    }
}

void ATranspersonalCharacter::Eat(float NutritionValue)
{
    Hunger = FMath::Clamp(Hunger + NutritionValue, 0.0f, 100.0f);
}

void ATranspersonalCharacter::Drink(float HydrationValue)
{
    Thirst = FMath::Clamp(Thirst + HydrationValue, 0.0f, 100.0f);
}

bool ATranspersonalCharacter::IsAlive() const
{
    return Health > 0.0f;
}

void ATranspersonalCharacter::SyncSurvivalStats()
{
    // Pull initial values from SurvivalComponent CDO defaults if available
    // SurvivalComponent owns authoritative values; character mirrors them for Blueprint access
    if (!SurvivalComp) return;

    // No direct property getters yet — component will broadcast via delegate
    // This stub is intentional: wiring happens when SurvivalComponent exposes delegates
}

// ─────────────────────────────────────────────────────────────────────────────
// Private: TickSurvivalStats
// ─────────────────────────────────────────────────────────────────────────────
void ATranspersonalCharacter::TickSurvivalStats(float DeltaTime)
{
    if (!IsAlive()) return;

    // Hunger drain
    Hunger = FMath::Clamp(Hunger - HungerDrainRate * DeltaTime, 0.0f, 100.0f);

    // Thirst drain (faster than hunger — dehydration is more dangerous)
    Thirst = FMath::Clamp(Thirst - ThirstDrainRate * DeltaTime, 0.0f, 100.0f);

    // Stamina: drain while running, regen while idle/walking
    if (bIsRunning && GetVelocity().SizeSquared() > 100.0f)
    {
        Stamina = FMath::Clamp(Stamina - StaminaDrainRun * DeltaTime, 0.0f, 100.0f);

        // Auto-stop run if stamina exhausted
        if (Stamina <= 0.0f)
        {
            StopRun();
        }
    }
    else
    {
        Stamina = FMath::Clamp(Stamina + StaminaRegenIdle * DeltaTime, 0.0f, 100.0f);
    }

    // Starvation / dehydration damage
    if (Hunger <= 0.0f)
    {
        ApplyDamage(1.0f * DeltaTime);  // 1 HP/s starvation
    }
    if (Thirst <= 0.0f)
    {
        ApplyDamage(2.0f * DeltaTime);  // 2 HP/s dehydration (more lethal)
    }

    // Fear decay (passive — combat system drives fear up)
    Fear = FMath::Clamp(Fear - 2.0f * DeltaTime, 0.0f, 100.0f);
}
