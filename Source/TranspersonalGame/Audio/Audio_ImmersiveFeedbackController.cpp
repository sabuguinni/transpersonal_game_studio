#include "Audio_ImmersiveFeedbackController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

AAudio_ImmersiveFeedbackController::AAudio_ImmersiveFeedbackController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create audio component for screen shake sounds
    ScreenShakeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ScreenShakeAudio"));
    ScreenShakeAudioComponent->SetupAttachment(RootComponent);
    ScreenShakeAudioComponent->bAutoActivate = false;
    
    // Create damage flash audio component
    DamageFlashAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DamageFlashAudio"));
    DamageFlashAudioComponent->SetupAttachment(RootComponent);
    DamageFlashAudioComponent->bAutoActivate = false;
    
    // Create footstep particle component
    FootstepParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FootstepParticles"));
    FootstepParticleComponent->SetupAttachment(RootComponent);
    FootstepParticleComponent->bAutoActivate = false;
    
    // Initialize default values
    TRexProximityDistance = 2000.0f;
    ScreenShakeIntensity = 1.0f;
    DamageFlashDuration = 0.5f;
    DamageFlashIntensity = 0.8f;
    FootstepParticleScale = 1.0f;
    bScreenShakeEnabled = true;
    bDamageFlashEnabled = true;
    bFootstepParticlesEnabled = true;
    
    // Initialize timers
    LastTRexShakeTime = 0.0f;
    TRexShakeCooldown = 1.0f;
    DamageFlashStartTime = 0.0f;
    bDamageFlashActive = false;
}

void AAudio_ImmersiveFeedbackController::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache player controller reference
    CachedPlayerController = GetWorld()->GetFirstPlayerController();
    
    // Cache player character reference
    if (CachedPlayerController)
    {
        CachedPlayerCharacter = CachedPlayerController->GetCharacter();
    }
    
    // Load material parameter collection for damage flash
    DamageFlashMPC = LoadObject<UMaterialParameterCollection>(nullptr, 
        TEXT("/Game/TranspersonalGame/Materials/MPC_DamageFlash"));
    
    if (!DamageFlashMPC)
    {
        UE_LOG(LogTemp, Warning, TEXT("DamageFlash MPC not found - creating runtime parameters"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("ImmersiveFeedbackController initialized successfully"));
}

void AAudio_ImmersiveFeedbackController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!CachedPlayerCharacter || !CachedPlayerController)
    {
        return;
    }
    
    // Update T-Rex proximity screen shake
    UpdateTRexProximityShake(DeltaTime);
    
    // Update damage flash effect
    UpdateDamageFlashEffect(DeltaTime);
    
    // Update footstep particles
    UpdateFootstepParticles(DeltaTime);
}

void AAudio_ImmersiveFeedbackController::UpdateTRexProximityShake(float DeltaTime)
{
    if (!bScreenShakeEnabled)
    {
        return;
    }
    
    // Find nearest T-Rex actor
    TArray<AActor*> TRexActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("TRex"), TRexActors);
    
    float NearestDistance = TRexProximityDistance + 1.0f;
    AActor* NearestTRex = nullptr;
    
    FVector PlayerLocation = CachedPlayerCharacter->GetActorLocation();
    
    for (AActor* TRexActor : TRexActors)
    {
        if (TRexActor)
        {
            float Distance = FVector::Dist(PlayerLocation, TRexActor->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestTRex = TRexActor;
            }
        }
    }
    
    // Trigger screen shake if T-Rex is within range
    if (NearestTRex && NearestDistance <= TRexProximityDistance)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastTRexShakeTime >= TRexShakeCooldown)
        {
            TriggerTRexScreenShake(NearestDistance);
            LastTRexShakeTime = CurrentTime;
        }
    }
}

void AAudio_ImmersiveFeedbackController::TriggerTRexScreenShake(float Distance)
{
    if (!CachedPlayerController || !bScreenShakeEnabled)
    {
        return;
    }
    
    // Calculate shake intensity based on distance (closer = stronger)
    float DistanceRatio = FMath::Clamp(1.0f - (Distance / TRexProximityDistance), 0.0f, 1.0f);
    float ShakeStrength = ScreenShakeIntensity * DistanceRatio;
    
    // Create camera shake parameters
    if (TRexScreenShakeClass)
    {
        CachedPlayerController->ClientStartCameraShake(TRexScreenShakeClass, ShakeStrength);
    }
    
    // Play screen shake audio
    if (ScreenShakeSound && ScreenShakeAudioComponent)
    {
        ScreenShakeAudioComponent->SetSound(ScreenShakeSound);
        ScreenShakeAudioComponent->SetVolumeMultiplier(ShakeStrength);
        ScreenShakeAudioComponent->Play();
    }
    
    UE_LOG(LogTemp, Log, TEXT("T-Rex screen shake triggered - Distance: %.1f, Strength: %.2f"), 
           Distance, ShakeStrength);
}

void AAudio_ImmersiveFeedbackController::TriggerDamageFlash(float Damage)
{
    if (!bDamageFlashEnabled || bDamageFlashActive)
    {
        return;
    }
    
    // Start damage flash effect
    bDamageFlashActive = true;
    DamageFlashStartTime = GetWorld()->GetTimeSeconds();
    
    // Calculate flash intensity based on damage
    float FlashStrength = FMath::Clamp(Damage / 100.0f, 0.1f, 1.0f) * DamageFlashIntensity;
    CurrentDamageFlashIntensity = FlashStrength;
    
    // Apply damage flash to material parameter collection
    if (DamageFlashMPC)
    {
        UMaterialParameterCollectionInstance* MPCInstance = 
            GetWorld()->GetParameterCollectionInstance(DamageFlashMPC);
        if (MPCInstance)
        {
            MPCInstance->SetScalarParameterValue(FName("DamageFlashIntensity"), FlashStrength);
            MPCInstance->SetScalarParameterValue(FName("DamageFlashActive"), 1.0f);
        }
    }
    
    // Play damage flash sound
    if (DamageFlashSound && DamageFlashAudioComponent)
    {
        DamageFlashAudioComponent->SetSound(DamageFlashSound);
        DamageFlashAudioComponent->SetVolumeMultiplier(FlashStrength);
        DamageFlashAudioComponent->Play();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Damage flash triggered - Damage: %.1f, Intensity: %.2f"), 
           Damage, FlashStrength);
}

void AAudio_ImmersiveFeedbackController::UpdateDamageFlashEffect(float DeltaTime)
{
    if (!bDamageFlashActive)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float ElapsedTime = CurrentTime - DamageFlashStartTime;
    
    if (ElapsedTime >= DamageFlashDuration)
    {
        // End damage flash effect
        bDamageFlashActive = false;
        
        if (DamageFlashMPC)
        {
            UMaterialParameterCollectionInstance* MPCInstance = 
                GetWorld()->GetParameterCollectionInstance(DamageFlashMPC);
            if (MPCInstance)
            {
                MPCInstance->SetScalarParameterValue(FName("DamageFlashActive"), 0.0f);
                MPCInstance->SetScalarParameterValue(FName("DamageFlashIntensity"), 0.0f);
            }
        }
    }
    else
    {
        // Update flash intensity with fade-out curve
        float FadeRatio = 1.0f - (ElapsedTime / DamageFlashDuration);
        float CurrentIntensity = CurrentDamageFlashIntensity * FadeRatio;
        
        if (DamageFlashMPC)
        {
            UMaterialParameterCollectionInstance* MPCInstance = 
                GetWorld()->GetParameterCollectionInstance(DamageFlashMPC);
            if (MPCInstance)
            {
                MPCInstance->SetScalarParameterValue(FName("DamageFlashIntensity"), CurrentIntensity);
            }
        }
    }
}

void AAudio_ImmersiveFeedbackController::TriggerFootstepParticles(const FVector& Location, bool bIsLargeCreature)
{
    if (!bFootstepParticlesEnabled)
    {
        return;
    }
    
    // Determine particle system to use
    UParticleSystem* ParticleToSpawn = bIsLargeCreature ? LargeFootstepParticles : PlayerFootstepParticles;
    
    if (ParticleToSpawn)
    {
        // Spawn footstep particles at location
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ParticleToSpawn,
            Location,
            FRotator::ZeroRotator,
            FVector(FootstepParticleScale),
            true,
            EPSCPoolMethod::AutoRelease
        );
    }
    
    // Play footstep sound
    USoundBase* FootstepSound = bIsLargeCreature ? LargeFootstepSound : PlayerFootstepSound;
    if (FootstepSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            FootstepSound,
            Location,
            bIsLargeCreature ? 2.0f : 1.0f
        );
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Footstep particles triggered at location: %s (Large: %s)"), 
           *Location.ToString(), bIsLargeCreature ? TEXT("Yes") : TEXT("No"));
}

void AAudio_ImmersiveFeedbackController::UpdateFootstepParticles(float DeltaTime)
{
    if (!CachedPlayerCharacter)
    {
        return;
    }
    
    // Check if player is moving and on ground
    FVector PlayerVelocity = CachedPlayerCharacter->GetVelocity();
    float PlayerSpeed = PlayerVelocity.Size2D();
    
    if (PlayerSpeed > 50.0f) // Minimum speed threshold
    {
        // Check if enough time has passed since last footstep
        float CurrentTime = GetWorld()->GetTimeSeconds();
        float FootstepInterval = FMath::Lerp(0.8f, 0.3f, PlayerSpeed / 600.0f); // Faster steps when running
        
        if (CurrentTime - LastPlayerFootstepTime >= FootstepInterval)
        {
            FVector FootstepLocation = CachedPlayerCharacter->GetActorLocation();
            FootstepLocation.Z -= 90.0f; // Offset to ground level
            
            TriggerFootstepParticles(FootstepLocation, false);
            LastPlayerFootstepTime = CurrentTime;
        }
    }
}

void AAudio_ImmersiveFeedbackController::SetScreenShakeEnabled(bool bEnabled)
{
    bScreenShakeEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Screen shake %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void AAudio_ImmersiveFeedbackController::SetDamageFlashEnabled(bool bEnabled)
{
    bDamageFlashEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Damage flash %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void AAudio_ImmersiveFeedbackController::SetFootstepParticlesEnabled(bool bEnabled)
{
    bFootstepParticlesEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Footstep particles %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}