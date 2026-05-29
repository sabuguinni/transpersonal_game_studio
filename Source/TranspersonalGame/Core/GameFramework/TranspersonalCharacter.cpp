// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "TranspersonalCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Core/Physics/Core_RagdollSystem.h"
#include "Core/Physics/Core_PhysicsSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogTranspersonalCharacter);

ATranspersonalCharacter::ATranspersonalCharacter()
{
    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
        
    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...    
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

    // Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
    // instead of recompiling to adjust them
    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

    // Create a camera boom (pulls in towards the player if there is a collision)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character    
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    // Create Physics Components
    RagdollComponent = CreateDefaultSubobject<UCore_RagdollSystem>(TEXT("RagdollComponent"));
    PhysicsManager = CreateDefaultSubobject<UCore_PhysicsSystemManager>(TEXT("PhysicsManager"));

    // Initialize Survival Stats
    MaxHealth = 100.0f;
    Health = MaxHealth;
    MaxStamina = 100.0f;
    Stamina = MaxStamina;
    MaxHunger = 100.0f;
    Hunger = MaxHunger;
    MaxThirst = 100.0f;
    Thirst = MaxThirst;

    // Initialize Fear System
    MaxFearLevel = 100.0f;
    FearLevel = 0.0f;
    bNearbyThreat = false;

    // Initialize Movement States
    bIsHiding = false;
    bIsSneaking = false;
    bIsRunning = false;

    // Initialize Physics States
    bIsRagdolled = false;
    bPhysicsEnabled = true;

    // Initialize Crafting System
    CraftingLevel = 1;
    CraftingExperience = 0.0f;
    KnownRecipes.Add(TEXT("Stone Tool"));
    KnownRecipes.Add(TEXT("Simple Shelter"));

    // Set this character to call Tick() every frame
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

    // Initialize Physics Components
    if (RagdollComponent)
    {
        RagdollComponent->Initialize(GetMesh());
        UE_LOG(LogTranspersonalCharacter, Log, TEXT("Ragdoll component initialized"));
    }

    if (PhysicsManager)
    {
        PhysicsManager->InitializePhysicsSystem();
        UE_LOG(LogTranspersonalCharacter, Log, TEXT("Physics manager initialized"));
    }

    UE_LOG(LogTranspersonalCharacter, Log, TEXT("TranspersonalCharacter BeginPlay completed"));
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update survival mechanics
    UpdateSurvivalStats(DeltaTime);
    
    // Update fear system
    UpdateFearSystem(DeltaTime);

    // Update physics state
    UpdatePhysicsState(DeltaTime);
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
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

        // Running
        if (RunAction)
        {
            EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::StartRunning);
            EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ATranspersonalCharacter::StopRunning);
        }

        // Interacting
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

    if (Controller != nullptr && !bIsRagdolled)
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
            float StaminaCost = bIsRunning ? 20.0f : 5.0f;
            ConsumeStamina(StaminaCost * GetWorld()->GetDeltaSeconds());
        }
    }
}

void ATranspersonalCharacter::Look(const FInputActionValue& Value)
{
    // Input is a Vector2D
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr && !bIsRagdolled)
    {
        // Add yaw and pitch input to controller
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ATranspersonalCharacter::StartSneaking()
{
    if (!bIsRagdolled && Stamina > 10.0f)
    {
        bIsSneaking = true;
        bIsRunning = false;
        GetCharacterMovement()->MaxWalkSpeed = 150.0f;
        UE_LOG(LogTranspersonalCharacter, Log, TEXT("Started sneaking"));
    }
}

void ATranspersonalCharacter::StopSneaking()
{
    bIsSneaking = false;
    GetCharacterMovement()->MaxWalkSpeed = bIsRunning ? 800.0f : 500.0f;
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Stopped sneaking"));
}

void ATranspersonalCharacter::StartRunning()
{
    if (!bIsRagdolled && Stamina > 20.0f && !bIsSneaking)
    {
        bIsRunning = true;
        GetCharacterMovement()->MaxWalkSpeed = 800.0f;
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
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Interact pressed"));
    // TODO: Implement interaction system
}

void ATranspersonalCharacter::OpenCraftingMenu()
{
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Crafting menu opened"));
    // TODO: Implement crafting UI
}

void ATranspersonalCharacter::UpdateSurvivalStats(float DeltaTime)
{
    // Decrease hunger and thirst over time
    Hunger = FMath::Max(0.0f, Hunger - (2.0f * DeltaTime));
    Thirst = FMath::Max(0.0f, Thirst - (3.0f * DeltaTime));

    // Regenerate stamina when not moving or running
    if (GetVelocity().Size() < 10.0f && !bIsRunning)
    {
        RestoreStamina(25.0f * DeltaTime);
    }

    // Health decreases if hunger or thirst are too low
    if (Hunger <= 10.0f || Thirst <= 10.0f)
    {
        Health = FMath::Max(0.0f, Health - (5.0f * DeltaTime));
    }

    // Death check
    if (Health <= 0.0f)
    {
        UE_LOG(LogTranspersonalCharacter, Warning, TEXT("Character died from survival stats"));
        // TODO: Implement death system
    }
}

void ATranspersonalCharacter::UpdateFearSystem(float DeltaTime)
{
    CheckForThreats();

    if (bNearbyThreat)
    {
        IncreaseFear(30.0f * DeltaTime);
    }
    else
    {
        // Gradually reduce fear when safe
        FearLevel = FMath::Max(0.0f, FearLevel - (10.0f * DeltaTime));
    }

    // High fear affects movement
    if (FearLevel > 80.0f)
    {
        GetCharacterMovement()->MaxWalkSpeed = FMath::Max(100.0f, GetCharacterMovement()->MaxWalkSpeed * 0.7f);
    }
}

void ATranspersonalCharacter::UpdatePhysicsState(float DeltaTime)
{
    if (!bPhysicsEnabled)
        return;

    // Handle collision events
    HandleCollisionEvents();

    // Process ragdoll transitions
    ProcessRagdollTransition();

    // Update physics components
    if (RagdollComponent)
    {
        RagdollComponent->UpdateRagdoll(DeltaTime);
    }

    if (PhysicsManager)
    {
        PhysicsManager->UpdatePhysicsSystem(DeltaTime);
    }
}

void ATranspersonalCharacter::HandleCollisionEvents()
{
    // Check for high-impact collisions that might trigger ragdoll
    if (GetVelocity().Size() > 1000.0f && !bIsRagdolled)
    {
        UE_LOG(LogTranspersonalCharacter, Log, TEXT("High velocity detected, considering ragdoll activation"));
        // TODO: Implement collision-based ragdoll triggers
    }
}

void ATranspersonalCharacter::ProcessRagdollTransition()
{
    if (bIsRagdolled && RagdollComponent)
    {
        // Check if we should recover from ragdoll
        if (RagdollComponent->ShouldRecoverFromRagdoll())
        {
            DisableRagdoll();
        }
    }
}

void ATranspersonalCharacter::CheckForThreats()
{
    // Simple threat detection - check for nearby dinosaurs
    bNearbyThreat = false;
    
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, APawn::class, FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (Actor != this && Actor->GetName().Contains(TEXT("Dinosaur")))
            {
                float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
                if (Distance < 2000.0f) // 20 meter threat radius
                {
                    bNearbyThreat = true;
                    break;
                }
            }
        }
    }
}

// Survival Functions Implementation
void ATranspersonalCharacter::ConsumeStamina(float Amount)
{
    Stamina = FMath::Max(0.0f, Stamina - Amount);
    
    if (Stamina <= 0.0f && bIsRunning)
    {
        StopRunning();
    }
}

void ATranspersonalCharacter::IncreaseFear(float Amount)
{
    FearLevel = FMath::Min(MaxFearLevel, FearLevel + Amount);
}

void ATranspersonalCharacter::RestoreHealth(float Amount)
{
    Health = FMath::Min(MaxHealth, Health + Amount);
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

// Physics Functions Implementation
void ATranspersonalCharacter::EnableRagdoll()
{
    if (RagdollComponent && !bIsRagdolled)
    {
        bIsRagdolled = true;
        RagdollComponent->EnableRagdoll();
        GetCharacterMovement()->SetMovementMode(MOVE_None);
        UE_LOG(LogTranspersonalCharacter, Log, TEXT("Ragdoll enabled"));
    }
}

void ATranspersonalCharacter::DisableRagdoll()
{
    if (RagdollComponent && bIsRagdolled)
    {
        bIsRagdolled = false;
        RagdollComponent->DisableRagdoll();
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        UE_LOG(LogTranspersonalCharacter, Log, TEXT("Ragdoll disabled"));
    }
}

void ATranspersonalCharacter::ApplyImpact(const FVector& ImpactForce, const FVector& ImpactLocation)
{
    if (PhysicsManager)
    {
        PhysicsManager->ApplyImpactForce(this, ImpactForce, ImpactLocation);
    }

    // High impact forces trigger ragdoll
    if (ImpactForce.Size() > 500.0f)
    {
        EnableRagdoll();
    }

    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Impact applied: Force=%s, Location=%s"), 
           *ImpactForce.ToString(), *ImpactLocation.ToString());
}

void ATranspersonalCharacter::SetPhysicsEnabled(bool bEnabled)
{
    bPhysicsEnabled = bEnabled;
    
    if (RagdollComponent)
    {
        RagdollComponent->SetEnabled(bEnabled);
    }

    if (PhysicsManager)
    {
        PhysicsManager->SetPhysicsEnabled(bEnabled);
    }

    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Physics enabled: %s"), bEnabled ? TEXT("true") : TEXT("false"));
}