// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "TranspersonalCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "../StudioDirectorSubsystem.h"

DEFINE_LOG_CATEGORY(LogTranspersonalCharacter);

ATranspersonalCharacter::ATranspersonalCharacter()
{
    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
        
    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...    
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

    // Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
    // instead of recompiling to adjust them
    GetCharacterMovement()->JumpZVelocity = 700.0f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.0f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

    // Create a camera boom (pulls in towards the player if there is a collision)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character    
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    // Initialize survival stats
    Health = 100.0f;
    MaxHealth = 100.0f;
    Stamina = 100.0f;
    MaxStamina = 100.0f;
    Hunger = 100.0f;
    MaxHunger = 100.0f;
    Thirst = 100.0f;
    MaxThirst = 100.0f;
    
    // Initialize fear system
    FearLevel = 0.0f;
    MaxFearLevel = 100.0f;
    bIsHiding = false;
    bIsSneaking = false;
    
    // Initialize crafting
    CraftingLevel = 1;
    CraftingExperience = 0.0f;
    
    // Set tick enabled for survival mechanics
    PrimaryActorTick.bCanEverTick = true;
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
    
    // Register with Studio Director
    if (UStudioDirectorSubsystem* StudioDirector = GetWorld()->GetSubsystem<UStudioDirectorSubsystem>())
    {
        FSystemInfo CharacterSystemInfo;
        CharacterSystemInfo.SystemName = TEXT("PlayerCharacter");
        CharacterSystemInfo.AgentName = TEXT("Character Artist");
        CharacterSystemInfo.AgentNumber = 9;
        CharacterSystemInfo.Status = ESystemStatus::Online;
        CharacterSystemInfo.Priority = EStudioDirectorPriority::High;
        
        StudioDirector->RegisterSystem(CharacterSystemInfo);
    }
    
    UE_LOG(LogTranspersonalCharacter, Warning, TEXT("Transpersonal Character initialized: %s"), *GetName());
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update survival mechanics
    UpdateSurvivalStats(DeltaTime);
    
    // Update fear system
    UpdateFearSystem(DeltaTime);
    
    // Check for nearby threats
    CheckForThreats();
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
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
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Move);
        }

        // Looking
        if (LookAction)
        {
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Look);
        }
        
        // Sneaking
        if (SneakAction)
        {
            EnhancedInputComponent->BindAction(SneakAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::StartSneaking);
            EnhancedInputComponent->BindAction(SneakAction, ETriggerEvent::Completed, this, &ATranspersonalCharacter::StopSneaking);
        }
        
        // Running/Sprinting
        if (RunAction)
        {
            EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::StartRunning);
            EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ATranspersonalCharacter::StopRunning);
        }
        
        // Interaction
        if (InteractAction)
        {
            EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Interact);
        }
        
        // Crafting
        if (CraftAction)
        {
            EnhancedInputComponent->BindAction(CraftAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::OpenCraftingMenu);
        }
    }
}

void ATranspersonalCharacter::Move(const FInputActionValue& Value)
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
        
        // Consume stamina when moving
        if (MovementVector.Size() > 0.1f)
        {
            ConsumeStamina(GetWorld()->GetDeltaSeconds() * 10.0f);
        }
    }
}

void ATranspersonalCharacter::Look(const FInputActionValue& Value)
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

void ATranspersonalCharacter::StartSneaking()
{
    bIsSneaking = true;
    GetCharacterMovement()->MaxWalkSpeed = 150.0f; // Slower when sneaking
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Started sneaking"));
}

void ATranspersonalCharacter::StopSneaking()
{
    bIsSneaking = false;
    GetCharacterMovement()->MaxWalkSpeed = 500.0f; // Normal speed
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Stopped sneaking"));
}

void ATranspersonalCharacter::StartRunning()
{
    if (Stamina > 20.0f)
    {
        bIsRunning = true;
        GetCharacterMovement()->MaxWalkSpeed = 800.0f; // Faster when running
        UE_LOG(LogTranspersonalCharacter, Log, TEXT("Started running"));
    }
}

void ATranspersonalCharacter::StopRunning()
{
    bIsRunning = false;
    GetCharacterMovement()->MaxWalkSpeed = bIsSneaking ? 150.0f : 500.0f;
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Stopped running"));
}

void ATranspersonalCharacter::Interact()
{
    // Implement interaction logic
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Interact action triggered"));
    
    // Trace for interactable objects
    FVector Start = GetActorLocation();
    FVector End = Start + (GetActorForwardVector() * 200.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
    {
        if (AActor* HitActor = HitResult.GetActor())
        {
            UE_LOG(LogTranspersonalCharacter, Log, TEXT("Interacting with: %s"), *HitActor->GetName());
            // Add interaction logic here
        }
    }
}

void ATranspersonalCharacter::OpenCraftingMenu()
{
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Opening crafting menu"));
    // Implement crafting menu logic
}

void ATranspersonalCharacter::UpdateSurvivalStats(float DeltaTime)
{
    // Decrease hunger and thirst over time
    Hunger = FMath::Max(0.0f, Hunger - (DeltaTime * 2.0f)); // Lose 2 hunger per second
    Thirst = FMath::Max(0.0f, Thirst - (DeltaTime * 3.0f)); // Lose 3 thirst per second
    
    // Regenerate stamina when not running
    if (!bIsRunning && Stamina < MaxStamina)
    {
        Stamina = FMath::Min(MaxStamina, Stamina + (DeltaTime * 20.0f));
    }
    
    // Consume extra stamina when running
    if (bIsRunning)
    {
        ConsumeStamina(DeltaTime * 25.0f);
    }
    
    // Health effects from low stats
    if (Hunger <= 0.0f || Thirst <= 0.0f)
    {
        TakeDamage(DeltaTime * 5.0f, FDamageEvent(), nullptr, nullptr);
    }
}

void ATranspersonalCharacter::UpdateFearSystem(float DeltaTime)
{
    // Gradually reduce fear over time if safe
    if (FearLevel > 0.0f && !bNearbyThreat)
    {
        FearLevel = FMath::Max(0.0f, FearLevel - (DeltaTime * 10.0f));
    }
    
    // Apply fear effects
    if (FearLevel > 50.0f)
    {
        // High fear - shaky camera, reduced accuracy
        // This would be implemented with camera shake and input modification
    }
}

void ATranspersonalCharacter::CheckForThreats()
{
    // Simple threat detection - check for dinosaurs nearby
    TArray<AActor*> OverlappingActors;
    GetCapsuleComponent()->GetOverlappingActors(OverlappingActors);
    
    bNearbyThreat = false;
    for (AActor* Actor : OverlappingActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur\")))
        {
            bNearbyThreat = true;
            IncreaseFear(20.0f);
            break;
        }
    }
}

void ATranspersonalCharacter::ConsumeStamina(float Amount)
{
    Stamina = FMath::Max(0.0f, Stamina - Amount);
    
    // Stop running if out of stamina
    if (Stamina <= 0.0f && bIsRunning)
    {
        StopRunning();
    }
}

void ATranspersonalCharacter::IncreaseFear(float Amount)
{
    FearLevel = FMath::Min(MaxFearLevel, FearLevel + Amount);
    UE_LOG(LogTranspersonalCharacter, Warning, TEXT("Fear increased to: %f"), FearLevel);
}

void ATranspersonalCharacter::RestoreHealth(float Amount)
{
    Health = FMath::Min(MaxHealth, Health + Amount);
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Health restored to: %f"), Health);
}

void ATranspersonalCharacter::RestoreStamina(float Amount)
{
    Stamina = FMath::Min(MaxStamina, Stamina + Amount);
}

void ATranspersonalCharacter::RestoreHunger(float Amount)
{
    Hunger = FMath::Min(MaxHunger, Hunger + Amount);
}

void ATranspersonalCharacter::RestoreThirst(float Amount)
{
    Thirst = FMath::Min(MaxThirst, Thirst + Amount);
}