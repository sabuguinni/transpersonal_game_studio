#include "Dir_PrehistoricCharacter.h"
#include "Engine/Engine.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ADir_PrehistoricCharacter::ADir_PrehistoricCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

    // Set movement speeds
    GetCharacterMovement()->JumpZVelocity = JumpVelocity;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

    // Create a camera boom (pulls in towards the player if there is a collision)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    // Initialize survival stats
    Health = MaxHealth;
    Stamina = MaxStamina;
    Hunger = 100.0f;
    Thirst = 100.0f;
}

void ADir_PrehistoricCharacter::BeginPlay()
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

    // Log character initialization
    UE_LOG(LogTemp, Warning, TEXT("Dir_PrehistoricCharacter initialized: %s"), *GetName());
}

void ADir_PrehistoricCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update survival stats
    UpdateSurvivalStats(DeltaTime);

    // Update movement state
    FVector Velocity = GetVelocity();
    Velocity.Z = 0.0f; // Ignore vertical movement for "is moving" check
    bIsMoving = Velocity.SizeSquared() > 1.0f;

    // Handle stamina consumption while sprinting
    if (bIsSprinting && bIsMoving)
    {
        ConsumeStamina(20.0f * DeltaTime); // Consume stamina while sprinting
        
        // Stop sprinting if out of stamina
        if (Stamina <= 0.0f)
        {
            StopSprinting();
        }
    }
    else if (!bIsSprinting && Stamina < MaxStamina)
    {
        // Regenerate stamina when not sprinting
        Stamina = FMath::Min(Stamina + 15.0f * DeltaTime, MaxStamina);
    }
}

void ADir_PrehistoricCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Jumping
        if (JumpAction)
        {
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }

        // Moving
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADir_PrehistoricCharacter::Move);
        }

        // Looking
        if (LookAction)
        {
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADir_PrehistoricCharacter::Look);
        }

        // Sprinting
        if (SprintAction)
        {
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ADir_PrehistoricCharacter::StartSprinting);
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ADir_PrehistoricCharacter::StopSprinting);
        }
    }
}

void ADir_PrehistoricCharacter::Move(const FInputActionValue& Value)
{
    // Input is a Vector2D
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // Find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // Get forward vector
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

        // Get right vector
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // Add movement
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ADir_PrehistoricCharacter::Look(const FInputActionValue& Value)
{
    // Input is a Vector2D
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // Add yaw and pitch input to controller
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ADir_PrehistoricCharacter::StartSprinting()
{
    if (HasEnoughStamina(10.0f) && bIsMoving) // Require minimum stamina and movement to sprint
    {
        bIsSprinting = true;
        GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    }
}

void ADir_PrehistoricCharacter::StopSprinting()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ADir_PrehistoricCharacter::UpdateSurvivalStats(float DeltaTime)
{
    // Gradual hunger and thirst decrease
    Hunger = FMath::Max(Hunger - 2.0f * DeltaTime, 0.0f);
    Thirst = FMath::Max(Thirst - 3.0f * DeltaTime, 0.0f);

    // Health consequences of low hunger/thirst
    if (Hunger <= 0.0f || Thirst <= 0.0f)
    {
        TakeDamage(5.0f * DeltaTime); // Lose health when starving/dehydrated
    }
    else if (Hunger > 80.0f && Thirst > 80.0f && Health < MaxHealth)
    {
        RestoreHealth(2.0f * DeltaTime); // Slowly heal when well-fed and hydrated
    }
}

void ADir_PrehistoricCharacter::TakeDamage(float DamageAmount)
{
    Health = FMath::Max(Health - DamageAmount, 0.0f);
    
    if (Health <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Character died!"));
        // TODO: Implement death logic
    }
}

void ADir_PrehistoricCharacter::RestoreHealth(float HealAmount)
{
    Health = FMath::Min(Health + HealAmount, MaxHealth);
}

void ADir_PrehistoricCharacter::ConsumeStamina(float StaminaCost)
{
    Stamina = FMath::Max(Stamina - StaminaCost, 0.0f);
}

bool ADir_PrehistoricCharacter::HasEnoughStamina(float RequiredStamina) const
{
    return Stamina >= RequiredStamina;
}