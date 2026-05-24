#include "Core_MovementSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "../GameFramework/TranspersonalCharacter.h"

DEFINE_LOG_CATEGORY(LogCore_MovementSystem);

UCore_MovementSystem::UCore_MovementSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS
    
    // Initialize default values
    BaseWalkSpeed = 200.0f;
    BaseRunSpeed = 400.0f;
    SneakSpeed = 100.0f;
    
    RunStaminaDrain = 15.0f;
    SneakStaminaDrain = 5.0f;
    MinStaminaToRun = 20.0f;
    StaminaRecoveryRate = 10.0f;
    
    FearSpeedPenalty = 0.3f;
    HighFearThreshold = 70.0f;
    TremblingIntensity = 2.0f;
    
    MudSpeedModifier = 0.6f;
    RockSpeedModifier = 0.8f;
    VegetationSpeedModifier = 0.7f;
    
    CurrentSpeedModifier = 1.0f;
    CurrentTerrain = TEXT("Normal");
}

void UCore_MovementSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get references to owner and movement component
    OwnerCharacter = Cast<ATranspersonalCharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        if (MovementComponent)
        {
            MovementComponent->MaxWalkSpeed = BaseWalkSpeed;
            UE_LOG(LogCore_MovementSystem, Log, TEXT("Movement System initialized for %s"), *OwnerCharacter->GetName());
        }
        else
        {
            UE_LOG(LogCore_MovementSystem, Error, TEXT("Failed to get CharacterMovementComponent"));
        }
    }
    else
    {
        UE_LOG(LogCore_MovementSystem, Error, TEXT("Owner is not a TranspersonalCharacter"));
    }
    
    LastPosition = GetOwner()->GetActorLocation();
}

void UCore_MovementSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    // Update stamina based on current movement state
    UpdateStamina(DeltaTime);
    
    // Check terrain periodically
    LastTerrainCheck += DeltaTime;
    if (LastTerrainCheck >= TerrainCheckInterval)
    {
        DetectTerrainType();
        LastTerrainCheck = 0.0f;
    }
    
    // Apply fear-based effects
    if (OwnerCharacter->FearLevel > HighFearThreshold)
    {
        ApplyFearPenalty(OwnerCharacter->FearLevel);
    }
    
    // Update movement speed based on current conditions
    UpdateMovementSpeed();
    
    // Apply movement effects like trembling
    ApplyMovementEffects(DeltaTime);
    
    // Check if we can still run
    CheckRunningConditions();
    
    LastPosition = GetOwner()->GetActorLocation();
}

void UCore_MovementSystem::StartRunning()
{
    if (!HasEnoughStaminaToRun() || bIsSneaking)
    {
        UE_LOG(LogCore_MovementSystem, Warning, TEXT("Cannot start running - insufficient stamina or sneaking"));
        return;
    }
    
    bIsRunning = true;
    bCanRun = true;
    
    UE_LOG(LogCore_MovementSystem, Log, TEXT("Started running"));
}

void UCore_MovementSystem::StopRunning()
{
    bIsRunning = false;
    UE_LOG(LogCore_MovementSystem, Log, TEXT("Stopped running"));
}

void UCore_MovementSystem::StartSneaking()
{
    if (bIsRunning)
    {
        StopRunning();
    }
    
    bIsSneaking = true;
    UE_LOG(LogCore_MovementSystem, Log, TEXT("Started sneaking"));
}

void UCore_MovementSystem::StopSneaking()
{
    bIsSneaking = false;
    UE_LOG(LogCore_MovementSystem, Log, TEXT("Stopped sneaking"));
}

void UCore_MovementSystem::ProcessMovementInput(const FInputActionValue& Value)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }
    
    FVector2D MovementVector = Value.Get<FVector2D>();
    
    if (GetOwner()->GetController())
    {
        // Get forward and right vectors
        const FRotator Rotation = GetOwner()->GetController()->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        
        // Add movement input
        OwnerCharacter->AddMovementInput(ForwardDirection, MovementVector.Y);
        OwnerCharacter->AddMovementInput(RightDirection, MovementVector.X);
        
        // Consume stamina if moving and running/sneaking
        if (!MovementVector.IsZero())
        {
            if (bIsRunning)
            {
                ConsumeMovementStamina(RunStaminaDrain * GetWorld()->GetDeltaSeconds());
            }
            else if (bIsSneaking)
            {
                ConsumeMovementStamina(SneakStaminaDrain * GetWorld()->GetDeltaSeconds());
            }
        }
    }
}

void UCore_MovementSystem::DetectTerrainType()
{
    FHitResult HitResult = PerformGroundTrace();
    
    if (HitResult.bBlockingHit)
    {
        // Simple terrain detection based on hit surface
        FString NewTerrain = TEXT("Normal");
        
        // Check surface material or actor name for terrain type
        if (HitResult.GetActor())
        {
            FString ActorName = HitResult.GetActor()->GetName();
            
            if (ActorName.Contains(TEXT("Mud")) || ActorName.Contains(TEXT("Swamp")))
            {
                NewTerrain = TEXT("Mud");
            }
            else if (ActorName.Contains(TEXT("Rock")) || ActorName.Contains(TEXT("Stone")))
            {
                NewTerrain = TEXT("Rock");
            }
            else if (ActorName.Contains(TEXT("Grass")) || ActorName.Contains(TEXT("Vegetation")))
            {
                NewTerrain = TEXT("Vegetation");
            }
        }
        
        if (NewTerrain != CurrentTerrain)
        {
            CurrentTerrain = NewTerrain;
            ApplyTerrainModifier(CurrentTerrain);
            UE_LOG(LogCore_MovementSystem, Log, TEXT("Terrain changed to: %s"), *CurrentTerrain);
        }
    }
}

void UCore_MovementSystem::ApplyTerrainModifier(const FString& TerrainType)
{
    if (TerrainType == TEXT("Mud"))
    {
        CurrentSpeedModifier = MudSpeedModifier;
    }
    else if (TerrainType == TEXT("Rock"))
    {
        CurrentSpeedModifier = RockSpeedModifier;
    }
    else if (TerrainType == TEXT("Vegetation"))
    {
        CurrentSpeedModifier = VegetationSpeedModifier;
    }
    else
    {
        CurrentSpeedModifier = 1.0f;
    }
}

void UCore_MovementSystem::ApplyFearPenalty(float FearLevel)
{
    float FearPenalty = FMath::Clamp((FearLevel - HighFearThreshold) / (100.0f - HighFearThreshold), 0.0f, 1.0f);
    CurrentSpeedModifier *= (1.0f - (FearPenalty * FearSpeedPenalty));
}

void UCore_MovementSystem::ApplyTrembling(float Intensity)
{
    if (!OwnerCharacter || Intensity <= 0.0f)
    {
        return;
    }
    
    TremblingTimer += GetWorld()->GetDeltaSeconds();
    
    // Apply subtle camera shake effect through character rotation
    float TremblingAmount = FMath::Sin(TremblingTimer * 10.0f) * Intensity;
    FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
    CurrentRotation.Roll += TremblingAmount * 0.5f;
    OwnerCharacter->SetActorRotation(CurrentRotation);
}

void UCore_MovementSystem::ConsumeMovementStamina(float Amount)
{
    if (OwnerCharacter)
    {
        OwnerCharacter->ConsumeStamina(Amount);
    }
}

bool UCore_MovementSystem::HasEnoughStaminaToRun() const
{
    return OwnerCharacter && OwnerCharacter->Stamina >= MinStaminaToRun;
}

float UCore_MovementSystem::GetCurrentSpeed() const
{
    if (!MovementComponent)
    {
        return 0.0f;
    }
    
    return MovementComponent->MaxWalkSpeed;
}

void UCore_MovementSystem::UpdateMovementSpeed()
{
    if (!MovementComponent)
    {
        return;
    }
    
    float TargetSpeed = BaseWalkSpeed;
    
    if (bIsRunning && bCanRun)
    {
        TargetSpeed = BaseRunSpeed;
    }
    else if (bIsSneaking)
    {
        TargetSpeed = SneakSpeed;
    }
    
    // Apply terrain and fear modifiers
    TargetSpeed *= CurrentSpeedModifier;
    
    MovementComponent->MaxWalkSpeed = TargetSpeed;
}

void UCore_MovementSystem::UpdateStamina(float DeltaTime)
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Recover stamina when not running/sneaking
    if (!bIsRunning && !bIsSneaking)
    {
        OwnerCharacter->RestoreStamina(StaminaRecoveryRate * DeltaTime);
    }
}

void UCore_MovementSystem::CheckRunningConditions()
{
    if (bIsRunning && !HasEnoughStaminaToRun())
    {
        StopRunning();
        bCanRun = false;
        UE_LOG(LogCore_MovementSystem, Warning, TEXT("Stopped running due to low stamina"));
    }
    else if (!bCanRun && OwnerCharacter && OwnerCharacter->Stamina > MinStaminaToRun * 1.5f)
    {
        bCanRun = true;
    }
}

void UCore_MovementSystem::ApplyMovementEffects(float DeltaTime)
{
    if (OwnerCharacter && OwnerCharacter->FearLevel > HighFearThreshold)
    {
        ApplyTrembling(TremblingIntensity);
    }
}

FHitResult UCore_MovementSystem::PerformGroundTrace() const
{
    FHitResult HitResult;
    
    if (!OwnerCharacter)
    {
        return HitResult;
    }
    
    FVector Start = OwnerCharacter->GetActorLocation();
    FVector End = Start - FVector(0, 0, 200.0f);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_WorldStatic,
        QueryParams
    );
    
    return HitResult;
}