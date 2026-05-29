#include "TranspersonalCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Core/Physics/Core_RagdollSystem.h"
#include "Core/Physics/Core_PhysicsSystemManager.h"

DEFINE_LOG_CATEGORY(LogTranspersonalCharacter);

ATranspersonalCharacter::ATranspersonalCharacter()
{
    // Set this character to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Set size for collision capsule
    GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
    GetCapsuleComponent()->SetCapsuleRadius(34.0f);

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

    // Create physics components
    RagdollComponent = CreateDefaultSubobject<UCore_RagdollSystem>(TEXT("RagdollSystem"));
    PhysicsManager = CreateDefaultSubobject<UCore_PhysicsSystemManager>(TEXT("PhysicsManager"));

    // Initialize survival stats
    MaxHealth = 100.0f;
    Health = MaxHealth;
    MaxStamina = 100.0f;
    Stamina = MaxStamina;
    MaxHunger = 100.0f;
    Hunger = MaxHunger;
    MaxThirst = 100.0f;
    Thirst = MaxThirst;
    
    // Initialize fear system
    MaxFearLevel = 100.0f;
    FearLevel = 0.0f;
    bNearbyThreat = false;
    
    // Initialize movement states
    bIsHiding = false;
    bIsSneaking = false;
    bIsRunning = false;
    
    // Initialize physics states
    bIsRagdolled = false;
    bPhysicsEnabled = true;
    
    // Initialize crafting
    CraftingLevel = 1;
    CraftingExperience = 0.0f;
    KnownRecipes.Add(TEXT("Stone Tool"));
    KnownRecipes.Add(TEXT("Simple Shelter"));
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
    
    // Initialize physics systems
    if (PhysicsManager)
    {
        PhysicsManager->InitializePhysicsSystem();
    }
    
    if (RagdollComponent)
    {
        RagdollComponent->InitializeRagdoll();
    }
    
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("TranspersonalCharacter initialized with survival stats and physics systems"));
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
    
    // Handle collision events
    HandleCollisionEvents();
}

void ATranspersonalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
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
            float StaminaCost = bIsRunning ? 20.0f : 5.0f;
            ConsumeStamina(StaminaCost * GetWorld()->GetDeltaSeconds());
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
    GetCharacterMovement()->MaxWalkSpeed = 150.0f; // Slow movement when sneaking
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Started sneaking"));
}

void ATranspersonalCharacter::StopSneaking()
{
    bIsSneaking = false;
    GetCharacterMovement()->MaxWalkSpeed = bIsRunning ? 800.0f : 500.0f;
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Stopped sneaking"));
}

void ATranspersonalCharacter::StartRunning()
{
    if (Stamina > 10.0f && !bIsSneaking)
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
    // Basic interaction logic - can be extended
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Interaction triggered"));
}

void ATranspersonalCharacter::OpenCraftingMenu()
{
    // Basic crafting menu logic - can be extended
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Crafting menu opened"));
}

void ATranspersonalCharacter::UpdateSurvivalStats(float DeltaTime)
{
    // Decrease hunger and thirst over time
    Hunger = FMath::Max(0.0f, Hunger - (5.0f * DeltaTime));
    Thirst = FMath::Max(0.0f, Thirst - (8.0f * DeltaTime));
    
    // Regenerate stamina when not running
    if (!bIsRunning && Stamina < MaxStamina)
    {
        Stamina = FMath::Min(MaxStamina, Stamina + (25.0f * DeltaTime));
    }
    
    // Health decreases if hunger or thirst is critical
    if (Hunger <= 0.0f || Thirst <= 0.0f)
    {
        Health = FMath::Max(0.0f, Health - (10.0f * DeltaTime));
    }
    
    // Stop running if no stamina
    if (bIsRunning && Stamina <= 0.0f)
    {
        StopRunning();
    }
}

void ATranspersonalCharacter::UpdateFearSystem(float DeltaTime)
{
    CheckForThreats();
    
    if (bNearbyThreat)
    {
        // Increase fear when threats are nearby
        FearLevel = FMath::Min(MaxFearLevel, FearLevel + (30.0f * DeltaTime));
    }
    else
    {
        // Decrease fear when safe
        FearLevel = FMath::Max(0.0f, FearLevel - (10.0f * DeltaTime));
    }
    
    // High fear affects movement
    if (FearLevel > 70.0f)
    {
        GetCharacterMovement()->MaxWalkSpeed *= 0.8f; // Panic reduces coordination
    }
}

void ATranspersonalCharacter::CheckForThreats()
{
    // Basic threat detection - can be expanded
    bNearbyThreat = false;
    
    // Check for dangerous actors in range
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::class, NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (Actor != this && Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            if (Distance < 2000.0f) // 20 meter threat detection range
            {
                bNearbyThreat = true;
                break;
            }
        }
    }
}

void ATranspersonalCharacter::UpdatePhysicsState(float DeltaTime)
{
    if (PhysicsManager)
    {
        PhysicsManager->UpdatePhysicsSystem(DeltaTime);
    }
    
    if (RagdollComponent && bIsRagdolled)
    {
        RagdollComponent->UpdateRagdoll(DeltaTime);
    }
}

void ATranspersonalCharacter::HandleCollisionEvents()
{
    // Handle physics collision events
    if (PhysicsManager)
    {
        // Process any pending collision events
        PhysicsManager->ProcessCollisionEvents();
    }
}

void ATranspersonalCharacter::ProcessRagdollTransition()
{
    if (RagdollComponent)
    {
        if (bIsRagdolled && Health > 0.0f && FearLevel < 50.0f)
        {
            // Recover from ragdoll state
            DisableRagdoll();
        }
        else if (!bIsRagdolled && (Health <= 0.0f || FearLevel > 90.0f))
        {
            // Enter ragdoll state due to death or extreme fear
            EnableRagdoll();
        }
    }
}

// Survival Functions
void ATranspersonalCharacter::ConsumeStamina(float Amount)
{
    Stamina = FMath::Max(0.0f, Stamina - Amount);
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

// Physics Functions
void ATranspersonalCharacter::EnableRagdoll()
{
    if (RagdollComponent)
    {
        bIsRagdolled = true;
        RagdollComponent->EnableRagdoll();
        GetCharacterMovement()->SetMovementMode(MOVE_None);
        UE_LOG(LogTranspersonalCharacter, Log, TEXT("Ragdoll enabled"));
    }
}

void ATranspersonalCharacter::DisableRagdoll()
{
    if (RagdollComponent)
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
        PhysicsManager->ApplyImpactForce(ImpactForce, ImpactLocation);
    }
    
    // High impact forces can trigger ragdoll
    if (ImpactForce.Size() > 5000.0f)
    {
        EnableRagdoll();
        IncreaseFear(25.0f);
    }
}

void ATranspersonalCharacter::SetPhysicsEnabled(bool bEnabled)
{
    bPhysicsEnabled = bEnabled;
    
    if (PhysicsManager)
    {
        PhysicsManager->SetPhysicsEnabled(bEnabled);
    }
}