#include "TranspersonalPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "PhysicsCore/PhysicsSystemManager.h"
#include "PhysicsCore/RagdollSystem.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

ATranspersonalPlayerController::ATranspersonalPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Set default interaction range
    InteractionRange = 300.0f;
    bPhysicsDebugEnabled = false;
    bCollisionDebugEnabled = false;
}

void ATranspersonalPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get physics system manager
    PhysicsManager = UPhysicsSystemManager::Get(this);
    
    // Setup Enhanced Input
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
    
    // Start interaction detection timer
    GetWorldTimerManager().SetTimer(
        InteractionUpdateTimer,
        this,
        &ATranspersonalPlayerController::UpdateInteractionDetection,
        0.1f, // Update 10 times per second
        true
    );
    
    UE_LOG(LogTemp, Log, TEXT("TranspersonalPlayerController: Initialized for survival gameplay"));
}

void ATranspersonalPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        // Movement
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATranspersonalPlayerController::Move);
        }
        
        // Looking
        if (LookAction)
        {
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATranspersonalPlayerController::Look);
        }
        
        // Jump
        if (JumpAction)
        {
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATranspersonalPlayerController::Jump);
        }
        
        // Interact
        if (InteractAction)
        {
            EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ATranspersonalPlayerController::Interact);
        }
        
        // Crouch
        if (CrouchAction)
        {
            EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ATranspersonalPlayerController::StartCrouch);
            EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ATranspersonalPlayerController::StopCrouch);
        }
        
        // Run
        if (RunAction)
        {
            EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &ATranspersonalPlayerController::StartRun);
            EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ATranspersonalPlayerController::StopRun);
        }
        
        // Physics Debug
        if (PhysicsDebugAction)
        {
            EnhancedInputComponent->BindAction(PhysicsDebugAction, ETriggerEvent::Started, this, &ATranspersonalPlayerController::TogglePhysicsDebug);
        }
    }
}

void ATranspersonalPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update physics debug visualization if enabled
    if (bPhysicsDebugEnabled && PhysicsManager)
    {
        int32 ActiveBodies;
        float SimTime;
        int32 CollisionPairs;
        PhysicsManager->GetPhysicsMetrics(ActiveBodies, SimTime, CollisionPairs);
        
        // Display physics info on screen
        if (GEngine)
        {
            FString DebugText = FString::Printf(
                TEXT("Physics Debug - Bodies: %d, SimTime: %.2fms, Collisions: %d"),
                ActiveBodies, SimTime * 1000.0f, CollisionPairs
            );
            GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, DebugText);
        }
    }
}

void ATranspersonalPlayerController::Move(const FInputActionValue& Value)
{
    if (APawn* ControlledPawn = GetPawn())
    {
        const FVector2D MovementVector = Value.Get<FVector2D>();
        
        // Get forward and right vectors
        const FVector Forward = ControlledPawn->GetActorForwardVector();
        const FVector Right = ControlledPawn->GetActorRightVector();
        
        // Add movement input
        ControlledPawn->AddMovementInput(Forward, MovementVector.Y);
        ControlledPawn->AddMovementInput(Right, MovementVector.X);
    }
}

void ATranspersonalPlayerController::Look(const FInputActionValue& Value)
{
    if (APawn* ControlledPawn = GetPawn())
    {
        const FVector2D LookAxisVector = Value.Get<FVector2D>();
        
        // Add yaw and pitch input
        ControlledPawn->AddControllerYawInput(LookAxisVector.X);
        ControlledPawn->AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ATranspersonalPlayerController::Jump()
{
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        Character->Jump();
    }
}

void ATranspersonalPlayerController::Interact()
{
    InteractWithEnvironment();
}

void ATranspersonalPlayerController::StartCrouch()
{
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        Character->Crouch();
    }
}

void ATranspersonalPlayerController::StopCrouch()
{
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        Character->UnCrouch();
    }
}

void ATranspersonalPlayerController::StartRun()
{
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            MovementComp->MaxWalkSpeed = 600.0f; // Running speed
        }
    }
}

void ATranspersonalPlayerController::StopRun()
{
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            MovementComp->MaxWalkSpeed = 300.0f; // Walking speed
        }
    }
}

void ATranspersonalPlayerController::TogglePhysicsDebug()
{
    bPhysicsDebugEnabled = !bPhysicsDebugEnabled;
    ShowPhysicsDebugInfo(bPhysicsDebugEnabled);
    
    UE_LOG(LogTemp, Log, TEXT("Physics Debug: %s"), bPhysicsDebugEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void ATranspersonalPlayerController::ApplyPhysicsImpulse(AActor* Target, const FVector& Impulse, const FVector& Location)
{
    if (!Target)
    {
        return;
    }
    
    // Try to find a primitive component to apply impulse to
    if (UPrimitiveComponent* PrimComp = Target->FindComponentByClass<UPrimitiveComponent>())
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            PrimComp->AddImpulseAtLocation(Impulse, Location);
            UE_LOG(LogTemp, Log, TEXT("Applied physics impulse to %s"), *Target->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Target %s is not simulating physics"), *Target->GetName());
        }
    }
}

void ATranspersonalPlayerController::ActivateRagdoll(AActor* Target, const FVector& ImpulseLocation, float ImpulseStrength)
{
    if (!Target)
    {
        return;
    }
    
    if (URagdollSystemComponent* RagdollComp = Target->FindComponentByClass<URagdollSystemComponent>())
    {
        RagdollComp->ActivateRagdoll(ImpulseLocation, ImpulseStrength);
        UE_LOG(LogTemp, Log, TEXT("Activated ragdoll for %s"), *Target->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Target %s has no RagdollSystemComponent"), *Target->GetName());
    }
}

void ATranspersonalPlayerController::DeactivateRagdoll(AActor* Target)
{
    if (!Target)
    {
        return;
    }
    
    if (URagdollSystemComponent* RagdollComp = Target->FindComponentByClass<URagdollSystemComponent>())
    {
        RagdollComp->DeactivateRagdoll();
        UE_LOG(LogTemp, Log, TEXT("Deactivated ragdoll for %s"), *Target->GetName());
    }
}

void ATranspersonalPlayerController::InteractWithEnvironment()
{
    AActor* InteractableActor = GetInteractableActor();
    if (!InteractableActor)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Interacting with %s"), *InteractableActor->GetName());
    
    // Determine interaction type based on actor
    FString ActorName = InteractableActor->GetName();
    
    if (ActorName.Contains(TEXT("Tree")) || ActorName.Contains(TEXT("Wood")))
    {
        // Gathering wood
        UE_LOG(LogTemp, Log, TEXT("Gathering wood from %s"), *ActorName);
    }
    else if (ActorName.Contains(TEXT("Rock")) || ActorName.Contains(TEXT("Stone")))
    {
        // Gathering stone
        UE_LOG(LogTemp, Log, TEXT("Gathering stone from %s"), *ActorName);
    }
    else if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("Creature")))
    {
        // Creature interaction (could be hostile or peaceful)
        UE_LOG(LogTemp, Log, TEXT("Interacting with creature %s"), *ActorName);
        
        // Example: Apply physics impulse to scare away small creatures
        FVector PlayerLocation = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
        FVector CreatureLocation = InteractableActor->GetActorLocation();
        FVector ImpulseDirection = (CreatureLocation - PlayerLocation).GetSafeNormal();
        
        ApplyPhysicsImpulse(InteractableActor, ImpulseDirection * 500.0f, CreatureLocation);
    }
    else
    {
        // Generic physics interaction
        FVector HitLocation = InteractableActor->GetActorLocation();
        HandlePhysicsInteraction(InteractableActor, HitLocation);
    }
}

AActor* ATranspersonalPlayerController::GetInteractableActor() const
{
    return TraceForInteractable();
}

void ATranspersonalPlayerController::StartCrafting()
{
    UE_LOG(LogTemp, Log, TEXT("Starting crafting system"));
    // TODO: Implement crafting system integration
}

void ATranspersonalPlayerController::StartBuilding()
{
    UE_LOG(LogTemp, Log, TEXT("Starting building system"));
    // TODO: Implement building system integration
}

void ATranspersonalPlayerController::StartHunting()
{
    UE_LOG(LogTemp, Log, TEXT("Starting hunting mode"));
    // TODO: Implement hunting mechanics
}

void ATranspersonalPlayerController::ShowPhysicsDebugInfo(bool bShow)
{
    bPhysicsDebugEnabled = bShow;
    
    if (PhysicsManager)
    {
        // Enable physics debug visualization in the physics manager
        // This would typically involve enabling Chaos Visual Debugger
        UE_LOG(LogTemp, Log, TEXT("Physics debug visualization: %s"), bShow ? TEXT("Enabled") : TEXT("Disabled"));
    }
}

void ATranspersonalPlayerController::ShowCollisionDebug(bool bShow)
{
    bCollisionDebugEnabled = bShow;
    
    // Enable collision debug drawing
    if (UWorld* World = GetWorld())
    {
        // This would typically involve setting console variables for collision debug
        UE_LOG(LogTemp, Log, TEXT("Collision debug visualization: %s"), bShow ? TEXT("Enabled") : TEXT("Disabled"));
    }
}

void ATranspersonalPlayerController::UpdateInteractionDetection()
{
    CurrentInteractable = TraceForInteractable();
    
    // Visual feedback for interactable objects
    if (CurrentInteractable && bCollisionDebugEnabled)
    {
        DrawDebugSphere(
            GetWorld(),
            CurrentInteractable->GetActorLocation(),
            50.0f,
            12,
            FColor::Green,
            false,
            0.15f
        );
    }
}

AActor* ATranspersonalPlayerController::TraceForInteractable() const
{
    if (!GetPawn())
    {
        return nullptr;
    }
    
    FVector Start = GetPawn()->GetActorLocation();
    FVector Forward = GetPawn()->GetActorForwardVector();
    FVector End = Start + (Forward * InteractionRange);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetPawn());
    
    if (GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_WorldDynamic,
        QueryParams
    ))
    {
        return HitResult.GetActor();
    }
    
    return nullptr;
}

void ATranspersonalPlayerController::HandlePhysicsInteraction(AActor* HitActor, const FVector& HitLocation)
{
    if (!HitActor)
    {
        return;
    }
    
    // Apply a small physics impulse to demonstrate interaction
    FVector PlayerLocation = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
    FVector ImpulseDirection = (HitLocation - PlayerLocation).GetSafeNormal();
    FVector Impulse = ImpulseDirection * 300.0f;
    
    ApplyPhysicsImpulse(HitActor, Impulse, HitLocation);
    
    UE_LOG(LogTemp, Log, TEXT("Applied physics interaction to %s"), *HitActor->GetName());
}