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

    // Initialize survival stats
    Health = 100.0f;
    MaxHealth = 100.0f;
    Hunger = 0.0f;
    MaxHunger = 100.0f;
    Thirst = 0.0f;
    MaxThirst = 100.0f;
    Stamina = 100.0f;
    MaxStamina = 100.0f;
    Fear = 0.0f;
    MaxFear = 100.0f;
    Temperature = 20.0f; // Normal body temperature
    
    // Initialize survival timers
    HungerDecayRate = 1.0f; // Hunger increases by 1 per minute
    ThirstDecayRate = 2.0f; // Thirst increases faster than hunger
    StaminaRegenRate = 25.0f; // Stamina regenerates quickly when not running
    FearDecayRate = 5.0f; // Fear decreases over time when safe
    
    // Initialize movement states
    bIsRunning = false;
    bIsCrouching = false;
    bIsClimbing = false;
    bIsSwimming = false;
    bIsHiding = false;
    
    // Initialize interaction
    InteractionRange = 200.0f;
    
    // Initialize inventory
    InventorySlots = 20;
    CurrentWeight = 0.0f;
    MaxWeight = 50.0f;
    
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
    
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("TranspersonalCharacter spawned with survival stats initialized"));
}

void ATranspersonalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update survival stats
    UpdateSurvivalStats(DeltaTime);
    
    // Update movement state
    UpdateMovementState();
    
    // Check for interactions
    CheckForInteractions();
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
        
        // Running
        if (RunAction)
        {
            EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &ATranspersonalCharacter::StartRunning);
            EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ATranspersonalCharacter::StopRunning);
        }
        
        // Crouching
        if (CrouchAction)
        {
            EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::ToggleCrouch);
        }
        
        // Interaction
        if (InteractAction)
        {
            EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ATranspersonalCharacter::Interact);
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
            float StaminaCost = bIsRunning ? 10.0f : 2.0f;
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

void ATranspersonalCharacter::StartRunning()
{
    if (Stamina > 10.0f && !bIsCrouching)
    {
        bIsRunning = true;
        GetCharacterMovement()->MaxWalkSpeed = 800.0f;
        UE_LOG(LogTranspersonalCharacter, Log, TEXT("Started running"));
    }
}

void ATranspersonalCharacter::StopRunning()
{
    bIsRunning = false;
    GetCharacterMovement()->MaxWalkSpeed = 500.0f;
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Stopped running"));
}

void ATranspersonalCharacter::ToggleCrouch()
{
    if (bIsCrouching)
    {
        UnCrouch();
        bIsCrouching = false;
        bIsRunning = false; // Can't run while crouching
        UE_LOG(LogTranspersonalCharacter, Log, TEXT("Stopped crouching"));
    }
    else
    {
        Crouch();
        bIsCrouching = true;
        bIsRunning = false; // Can't run while crouching
        UE_LOG(LogTranspersonalCharacter, Log, TEXT("Started crouching"));
    }
}

void ATranspersonalCharacter::Interact()
{
    // Find nearby interactable objects
    TArray<AActor*> NearbyActors;
    FVector PlayerLocation = GetActorLocation();
    
    // Get all actors in interaction range
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor != this)
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            if (Distance <= InteractionRange)
            {
                NearbyActors.Add(Actor);
            }
        }
    }
    
    if (NearbyActors.Num() > 0)
    {
        // Interact with the closest actor
        AActor* ClosestActor = NearbyActors[0];
        float ClosestDistance = FVector::Dist(PlayerLocation, ClosestActor->GetActorLocation());
        
        for (AActor* Actor : NearbyActors)
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                ClosestActor = Actor;
                ClosestDistance = Distance;
            }
        }
        
        UE_LOG(LogTranspersonalCharacter, Log, TEXT("Interacting with: %s"), *ClosestActor->GetName());
        
        // TODO: Implement specific interaction logic based on actor type
        OnInteract.Broadcast(ClosestActor);
    }
    else
    {
        UE_LOG(LogTranspersonalCharacter, Log, TEXT("No interactable objects in range"));
    }
}

void ATranspersonalCharacter::UpdateSurvivalStats(float DeltaTime)
{
    // Update hunger
    Hunger = FMath::Clamp(Hunger + (HungerDecayRate * DeltaTime / 60.0f), 0.0f, MaxHunger);
    
    // Update thirst
    Thirst = FMath::Clamp(Thirst + (ThirstDecayRate * DeltaTime / 60.0f), 0.0f, MaxThirst);
    
    // Update stamina
    if (!bIsRunning && Stamina < MaxStamina)
    {
        Stamina = FMath::Clamp(Stamina + (StaminaRegenRate * DeltaTime), 0.0f, MaxStamina);
    }
    
    // Update fear (decreases over time when safe)
    if (Fear > 0.0f)
    {
        Fear = FMath::Clamp(Fear - (FearDecayRate * DeltaTime), 0.0f, MaxFear);
    }
    
    // Health effects from hunger/thirst
    if (Hunger >= 90.0f || Thirst >= 90.0f)
    {
        // Lose health when critically hungry or thirsty
        TakeDamage(5.0f * DeltaTime, FDamageEvent(), nullptr, nullptr);
    }
    
    // Stop running if out of stamina
    if (bIsRunning && Stamina <= 0.0f)
    {
        StopRunning();
    }
}

void ATranspersonalCharacter::UpdateMovementState()
{
    // Update swimming state based on water volume
    // TODO: Implement water volume detection
    
    // Update climbing state based on climbable surfaces
    // TODO: Implement climbing detection
    
    // Update hiding state based on cover
    // TODO: Implement cover detection
}

void ATranspersonalCharacter::CheckForInteractions()
{
    // This is called every tick to update interaction prompts
    // TODO: Implement UI interaction prompts
}

void ATranspersonalCharacter::ConsumeStamina(float Amount)
{
    Stamina = FMath::Clamp(Stamina - Amount, 0.0f, MaxStamina);
}

void ATranspersonalCharacter::RestoreStamina(float Amount)
{
    Stamina = FMath::Clamp(Stamina + Amount, 0.0f, MaxStamina);
}

void ATranspersonalCharacter::AddFear(float Amount)
{
    Fear = FMath::Clamp(Fear + Amount, 0.0f, MaxFear);
    
    if (Fear >= 75.0f)
    {
        // High fear affects movement
        GetCharacterMovement()->MaxWalkSpeed = bIsRunning ? 600.0f : 300.0f;
    }
    else
    {
        // Normal movement speed
        GetCharacterMovement()->MaxWalkSpeed = bIsRunning ? 800.0f : 500.0f;
    }
}

void ATranspersonalCharacter::ReduceFear(float Amount)
{
    Fear = FMath::Clamp(Fear - Amount, 0.0f, MaxFear);
    
    // Restore normal movement speed when fear is low
    if (Fear < 25.0f)
    {
        GetCharacterMovement()->MaxWalkSpeed = bIsRunning ? 800.0f : 500.0f;
    }
}

void ATranspersonalCharacter::Eat(float NutritionValue)
{
    Hunger = FMath::Clamp(Hunger - NutritionValue, 0.0f, MaxHunger);
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Ate food, hunger reduced to: %f"), Hunger);
}

void ATranspersonalCharacter::Drink(float HydrationValue)
{
    Thirst = FMath::Clamp(Thirst - HydrationValue, 0.0f, MaxThirst);
    UE_LOG(LogTranspersonalCharacter, Log, TEXT("Drank water, thirst reduced to: %f"), Thirst);
}

float ATranspersonalCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    
    Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);
    
    // Add fear when taking damage
    AddFear(ActualDamage * 2.0f);
    
    UE_LOG(LogTranspersonalCharacter, Warning, TEXT("Took %f damage, health: %f, fear: %f"), ActualDamage, Health, Fear);
    
    if (Health <= 0.0f)
    {
        // Handle death
        UE_LOG(LogTranspersonalCharacter, Error, TEXT("Character died!"));
        OnDeath.Broadcast();
    }
    
    return ActualDamage;
}