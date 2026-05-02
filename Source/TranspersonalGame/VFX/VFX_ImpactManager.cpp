#include "VFX_ImpactManager.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

UVFX_ImpactManager::UVFX_ImpactManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for performance

    // Initialize default values
    MaxScreenShakeIntensity = 2.0f;
    MaxScreenShakeDuration = 3.0f;
    DamageFlashDuration = 0.5f;
    DamageFlashColor = FLinearColor::Red;

    // Initialize arrays
    ActiveParticleComponents.Empty();
    EffectTimers.Empty();

    // Set default Niagara systems to null (will be set in Blueprint or code)
    DustParticleSystem = nullptr;
    BloodParticleSystem = nullptr;
    DebrisParticleSystem = nullptr;
}

void UVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Component initialized"));

    // Try to load default Niagara systems if not set
    if (!DustParticleSystem)
    {
        DustParticleSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultDust"));
    }

    if (!BloodParticleSystem)
    {
        BloodParticleSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultBlood"));
    }

    if (!DebrisParticleSystem)
    {
        DebrisParticleSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultDebris"));
    }
}

void UVFX_ImpactManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Cleanup expired effects periodically
    CleanupExpiredEffects();
}

void UVFX_ImpactManager::TriggerImpact(const FVFX_ImpactData& ImpactData)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: No valid world for impact effect"));
        return;
    }

    UNiagaraSystem* SystemToUse = nullptr;
    
    switch (ImpactData.ImpactType)
    {
        case EVFX_ImpactType::DinosaurFootstep:
        case EVFX_ImpactType::GroundImpact:
        case EVFX_ImpactType::DustExplosion:
            SystemToUse = DustParticleSystem;
            break;
            
        case EVFX_ImpactType::BloodSplatter:
        case EVFX_ImpactType::PlayerDamage:
            SystemToUse = BloodParticleSystem;
            break;
            
        default:
            SystemToUse = DebrisParticleSystem;
            break;
    }

    if (SystemToUse)
    {
        UNiagaraComponent* ParticleComponent = CreateParticleEffect(SystemToUse, ImpactData.Location, ImpactData.Scale);
        
        if (ParticleComponent)
        {
            // Set effect parameters
            ParticleComponent->SetFloatParameter(TEXT("Intensity"), ImpactData.Intensity);
            ParticleComponent->SetVectorParameter(TEXT("Scale"), ImpactData.Scale);
            
            // Schedule removal after duration
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(
                TimerHandle,
                [this, ParticleComponent]()
                {
                    RemoveParticleEffect(ParticleComponent);
                },
                ImpactData.Duration,
                false
            );
            
            EffectTimers.Add(TimerHandle);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Triggered impact effect at location %s"), *ImpactData.Location.ToString());
}

void UVFX_ImpactManager::TriggerDinosaurFootstep(FVector Location, float DinosaurSize)
{
    FVFX_ImpactData FootstepData;
    FootstepData.ImpactType = EVFX_ImpactType::DinosaurFootstep;
    FootstepData.Location = Location;
    FootstepData.Scale = FVector(DinosaurSize, DinosaurSize, 1.0f);
    FootstepData.Duration = 3.0f;
    FootstepData.Intensity = FMath::Clamp(DinosaurSize, 0.5f, 3.0f);

    TriggerImpact(FootstepData);

    // Also trigger screen shake for large dinosaurs
    if (DinosaurSize > 1.5f)
    {
        float ShakeIntensity = (DinosaurSize - 1.0f) * 0.5f;
        TriggerScreenShake(ShakeIntensity, 1.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Dinosaur footstep triggered, size: %f"), DinosaurSize);
}

void UVFX_ImpactManager::TriggerPlayerDamage(float DamageAmount)
{
    // Trigger blood splatter effect
    FVFX_ImpactData DamageData;
    DamageData.ImpactType = EVFX_ImpactType::PlayerDamage;
    DamageData.Location = GetOwner()->GetActorLocation();
    DamageData.Scale = FVector(1.0f, 1.0f, 1.0f);
    DamageData.Duration = DamageFlashDuration;
    DamageData.Intensity = FMath::Clamp(DamageAmount / 100.0f, 0.1f, 2.0f);

    TriggerImpact(DamageData);

    // Trigger screen shake based on damage
    float ShakeIntensity = FMath::Clamp(DamageAmount / 50.0f, 0.2f, MaxScreenShakeIntensity);
    TriggerScreenShake(ShakeIntensity, DamageFlashDuration * 2.0f);

    UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Player damage effect triggered, damage: %f"), DamageAmount);
}

void UVFX_ImpactManager::TriggerScreenShake(float Intensity, float Duration)
{
    if (!GetWorld())
    {
        return;
    }

    // Get player controller
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: No player controller found for screen shake"));
        return;
    }

    // Clamp values to safe ranges
    float ClampedIntensity = FMath::Clamp(Intensity, 0.1f, MaxScreenShakeIntensity);
    float ClampedDuration = FMath::Clamp(Duration, 0.1f, MaxScreenShakeDuration);

    // Apply screen shake using camera shake
    // Note: In a full implementation, you would create a UCameraShakeBase subclass
    // For now, we'll use a simple camera manager approach
    if (PlayerController->PlayerCameraManager)
    {
        // Simple camera shake simulation
        FVector ShakeOffset = FVector(
            FMath::RandRange(-ClampedIntensity, ClampedIntensity),
            FMath::RandRange(-ClampedIntensity, ClampedIntensity),
            FMath::RandRange(-ClampedIntensity * 0.5f, ClampedIntensity * 0.5f)
        );

        // Apply shake (this is a simplified version)
        PlayerController->PlayerCameraManager->StartCameraShake(nullptr, ClampedIntensity);
    }

    UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Screen shake triggered, intensity: %f, duration: %f"), ClampedIntensity, ClampedDuration);
}

void UVFX_ImpactManager::CleanupExpiredEffects()
{
    // Remove null or invalid components from active list
    ActiveParticleComponents.RemoveAll([](UNiagaraComponent* Component)
    {
        return !IsValid(Component) || !Component->IsActive();
    });

    // Clear completed timers
    EffectTimers.RemoveAll([this](const FTimerHandle& Handle)
    {
        return !GetWorld() || !GetWorld()->GetTimerManager().IsTimerActive(Handle);
    });
}

UNiagaraComponent* UVFX_ImpactManager::CreateParticleEffect(UNiagaraSystem* System, const FVector& Location, const FVector& Scale)
{
    if (!System || !GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: Invalid system or world for particle effect"));
        return nullptr;
    }

    // Spawn Niagara component at location
    UNiagaraComponent* ParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        System,
        Location,
        FRotator::ZeroRotator,
        Scale,
        true, // Auto destroy
        EPSCPoolMethod::None,
        true  // Auto activate
    );

    if (ParticleComponent)
    {
        ActiveParticleComponents.Add(ParticleComponent);
        UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Created particle effect at %s"), *Location.ToString());
    }

    return ParticleComponent;
}

void UVFX_ImpactManager::RemoveParticleEffect(UNiagaraComponent* Component)
{
    if (IsValid(Component))
    {
        Component->Deactivate();
        Component->DestroyComponent();
        ActiveParticleComponents.Remove(Component);
        UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Removed particle effect component"));
    }
}

void UVFX_ImpactManager::OnEffectExpired()
{
    // Timer callback for effect cleanup
    CleanupExpiredEffects();
}