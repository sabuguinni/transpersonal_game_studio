#include "Eng_CharacterMovementArchitecture.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"

AEng_CharacterMovementArchitecture::AEng_CharacterMovementArchitecture()
{
    PrimaryActorTick.bCanEverTick = true;

    // CAPSULE COMPONENT SETUP
    GetCapsuleComponent()->SetCapsuleSize(42.0f, 96.0f);

    // CHARACTER MOVEMENT SETUP
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // MOVEMENT COMPONENT CONFIGURATION
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.2f;

    // CAMERA BOOM SETUP
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // FOLLOW CAMERA SETUP
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // INITIALIZE DEFAULT VALUES
    InitializeDefaultValues();
}

void AEng_CharacterMovementArchitecture::InitializeDefaultValues()
{
    // MOVEMENT PARAMETERS
    BaseTurnRate = 45.0f;
    BaseLookUpRate = 45.0f;
    WalkSpeed = 600.0f;
    RunSpeed = 1200.0f;
    JumpVelocity = 600.0f;

    // SURVIVAL STATS
    MaxHealth = 100.0f;
    Health = MaxHealth;
    MaxStamina = 100.0f;
    Stamina = MaxStamina;
    Hunger = 0.0f;
    Thirst = 0.0f;

    // INTERNAL STATE
    bIsRunning = false;
    StaminaRegenRate = 10.0f;
    HungerDecayRate = 1.0f;
    ThirstDecayRate = 1.5f;
}

void AEng_CharacterMovementArchitecture::BeginPlay()
{
    Super::BeginPlay();
    
    ValidateMovementComponent();
    
    // SET INITIAL MOVEMENT SPEED
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
}

void AEng_CharacterMovementArchitecture::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // STAMINA REGENERATION
    if (!bIsRunning && Stamina < MaxStamina)
    {
        RestoreStamina(StaminaRegenRate * DeltaTime);
    }

    // SURVIVAL STATS DECAY
    UpdateHunger(HungerDecayRate * DeltaTime);
    UpdateThirst(ThirstDecayRate * DeltaTime);

    // HEALTH LOSS FROM HUNGER/THIRST
    if (Hunger >= 100.0f || Thirst >= 100.0f)
    {
        TakeDamage(5.0f * DeltaTime);
    }
}

void AEng_CharacterMovementArchitecture::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    check(PlayerInputComponent);

    // MOVEMENT BINDINGS
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AEng_CharacterMovementArchitecture::StartJump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &AEng_CharacterMovementArchitecture::StopJump);

    PlayerInputComponent->BindAction("Run", IE_Pressed, this, &AEng_CharacterMovementArchitecture::StartRun);
    PlayerInputComponent->BindAction("Run", IE_Released, this, &AEng_CharacterMovementArchitecture::StopRun);

    PlayerInputComponent->BindAxis("MoveForward", this, &AEng_CharacterMovementArchitecture::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AEng_CharacterMovementArchitecture::MoveRight);

    // CAMERA BINDINGS
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("TurnRate", this, &AEng_CharacterMovementArchitecture::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("LookUpRate", this, &AEng_CharacterMovementArchitecture::LookUpAtRate);
}

void AEng_CharacterMovementArchitecture::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void AEng_CharacterMovementArchitecture::MoveRight(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void AEng_CharacterMovementArchitecture::TurnAtRate(float Rate)
{
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AEng_CharacterMovementArchitecture::LookUpAtRate(float Rate)
{
    AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AEng_CharacterMovementArchitecture::StartJump()
{
    if (Stamina >= 20.0f)
    {
        Jump();
        ConsumeStamina(20.0f);
    }
}

void AEng_CharacterMovementArchitecture::StopJump()
{
    StopJumping();
}

void AEng_CharacterMovementArchitecture::StartRun()
{
    if (Stamina > 0.0f)
    {
        bIsRunning = true;
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
        }
    }
}

void AEng_CharacterMovementArchitecture::StopRun()
{
    bIsRunning = false;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
}

void AEng_CharacterMovementArchitecture::TakeDamage(float DamageAmount)
{
    Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
    
    if (Health <= 0.0f)
    {
        // CHARACTER DEATH LOGIC
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Character Died!"));
        }
    }
}

void AEng_CharacterMovementArchitecture::RestoreHealth(float HealAmount)
{
    Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);
}

void AEng_CharacterMovementArchitecture::ConsumeStamina(float StaminaCost)
{
    Stamina = FMath::Clamp(Stamina - StaminaCost, 0.0f, MaxStamina);
    
    // STOP RUNNING IF OUT OF STAMINA
    if (Stamina <= 0.0f && bIsRunning)
    {
        StopRun();
    }
}

void AEng_CharacterMovementArchitecture::RestoreStamina(float StaminaAmount)
{
    Stamina = FMath::Clamp(Stamina + StaminaAmount, 0.0f, MaxStamina);
}

void AEng_CharacterMovementArchitecture::UpdateHunger(float HungerChange)
{
    Hunger = FMath::Clamp(Hunger + HungerChange, 0.0f, 100.0f);
}

void AEng_CharacterMovementArchitecture::UpdateThirst(float ThirstChange)
{
    Thirst = FMath::Clamp(Thirst + ThirstChange, 0.0f, 100.0f);
}

void AEng_CharacterMovementArchitecture::ValidateMovementComponent()
{
    if (!GetCharacterMovement())
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERROR: No CharacterMovementComponent found!"));
        }
        return;
    }

    if (!CameraBoom)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERROR: No CameraBoom component found!"));
        }
        return;
    }

    if (!FollowCamera)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERROR: No FollowCamera component found!"));
        }
        return;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Character Movement Architecture Validated Successfully"));
    }
}