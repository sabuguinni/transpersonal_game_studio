#include "Core/GameFramework/TranspersonalCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ATranspersonalCharacter::ATranspersonalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set capsule size
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

    // Create camera boom
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // Create follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Initialize survival stats
    Health = 100.0f;
    MaxHealth = 100.0f;
    Stamina = 100.0f;
    MaxStamina = 100.0f;
    Hunger = 100.0f;
    MaxHunger = 100.0f;
    Thirst = 100.0f;
    MaxThirst = 100.0f;
    FearLevel = 0.0f;
    MaxFearLevel = 100.0f;
    bNearbyThreat = false;
    bIsHiding = false;
    bIsSneaking = false;
    bIsRunning = false;
    CraftingLevel = 1;
    CraftingExperience = 0.0f;
}

void ATranspersonalCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Add Input Mapping Context
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("TranspersonalCharacter: Player spawned and ready!"));
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Movement
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Move);
        }
        // Looking
        if (LookAction)
        {
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Look);
        }
        // Jumping
        if (JumpAction)
        {
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }
    }
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateSurvivalStats(DeltaTime);
    UpdateFearSystem(DeltaTime);
}

void ATranspersonalCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        
        float SpeedMultiplier = 1.0f;
        if (bIsSneaking) SpeedMultiplier = 0.4f;
        else if (bIsRunning) SpeedMultiplier = 1.6f;
        
        AddMovementInput(ForwardDirection, MovementVector.Y * SpeedMultiplier);
        AddMovementInput(RightDirection, MovementVector.X * SpeedMultiplier);
    }
}

void ATranspersonalCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ATranspersonalCharacter::StartSneaking() { bIsSneaking = true; }
void ATranspersonalCharacter::StopSneaking() { bIsSneaking = false; }
void ATranspersonalCharacter::StartRunning() { bIsRunning = true; }
void ATranspersonalCharacter::StopRunning() { bIsRunning = false; }
void ATranspersonalCharacter::Interact() { UE_LOG(LogTemp, Log, TEXT("Interact pressed")); }
void ATranspersonalCharacter::OpenCraftingMenu() { UE_LOG(LogTemp, Log, TEXT("Crafting menu opened")); }

void ATranspersonalCharacter::UpdateSurvivalStats(float DeltaTime)
{
    // Slowly decrease hunger and thirst
    Hunger = FMath::Max(0.0f, Hunger - 0.5f * DeltaTime);
    Thirst = FMath::Max(0.0f, Thirst - 0.8f * DeltaTime);
    
    // Regenerate stamina when not running
    if (!bIsRunning)
    {
        Stamina = FMath::Min(MaxStamina, Stamina + 10.0f * DeltaTime);
    }
    
    // Take damage if starving or dehydrated
    if (Hunger <= 0.0f || Thirst <= 0.0f)
    {
        Health = FMath::Max(0.0f, Health - 2.0f * DeltaTime);
    }
}

void ATranspersonalCharacter::UpdateFearSystem(float DeltaTime)
{
    if (!bNearbyThreat)
    {
        FearLevel = FMath::Max(0.0f, FearLevel - 5.0f * DeltaTime);
    }
}

void ATranspersonalCharacter::CheckForThreats()
{
    // Will be implemented with dinosaur proximity checks
}

void ATranspersonalCharacter::ConsumeStamina(float Amount)
{
    Stamina = FMath::Max(0.0f, Stamina - Amount);
}

void ATranspersonalCharacter::IncreaseFear(float Amount)
{
    FearLevel = FMath::Min(MaxFearLevel, FearLevel + Amount);
}

void ATranspersonalCharacter::RestoreHealth(float Amount) { Health = FMath::Min(MaxHealth, Health + Amount); }
void ATranspersonalCharacter::RestoreStamina(float Amount) { Stamina = FMath::Min(MaxStamina, Stamina + Amount); }
void ATranspersonalCharacter::RestoreHunger(float Amount) { Hunger = FMath::Min(MaxHunger, Hunger + Amount); }
void ATranspersonalCharacter::RestoreThirst(float Amount) { Thirst = FMath::Min(MaxThirst, Thirst + Amount); }
