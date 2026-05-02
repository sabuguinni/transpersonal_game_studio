#include "VFX_FootstepParticleManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UVFX_FootstepParticleManager::UVFX_FootstepParticleManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check every 100ms for cleanup
    
    GlobalIntensityMultiplier = 1.0f;
    MaxFootstepDistance = 2000.0f;
    bEnableDistanceAttenuation = true;
}

void UVFX_FootstepParticleManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Try to load default Niagara systems
    if (!DustParticleSystem.IsValid())
    {
        // Try to find a basic dust system in the engine
        DustParticleSystem = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultDust")));
    }
    
    if (!MudSplashSystem.IsValid())
    {
        MudSplashSystem = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultSplash")));
    }
    
    if (!RockDebrisSystem.IsValid())
    {
        RockDebrisSystem = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Engine/VFX/Niagara/Systems/NS_DefaultDebris")));
    }
}

void UVFX_FootstepParticleManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up finished particle effects
    CleanupFinishedEffects();
}

void UVFX_FootstepParticleManager::TriggerFootstepEffect(const FVector& Location, const FVFX_FootstepConfig& Config)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Calculate distance attenuation
    float DistanceAttenuation = bEnableDistanceAttenuation ? CalculateDistanceAttenuation(Location) : 1.0f;
    if (DistanceAttenuation <= 0.0f)
    {
        return; // Too far away
    }
    
    // Get appropriate particle system for surface
    UNiagaraSystem* SystemToUse = GetSystemForSurface(Config.SurfaceType);
    if (!SystemToUse)
    {
        // Fallback to dust system
        SystemToUse = DustParticleSystem.LoadSynchronous();
    }
    
    if (SystemToUse)
    {
        // Create modified config with distance attenuation
        FVFX_FootstepConfig ModifiedConfig = Config;
        ModifiedConfig.ParticleIntensity *= DistanceAttenuation * GlobalIntensityMultiplier;
        
        // Adjust intensity based on footstep type
        switch (Config.FootstepType)
        {
            case EVFX_FootstepType::Light:
                ModifiedConfig.ParticleIntensity *= 0.3f;
                ModifiedConfig.DustRadius *= 0.5f;
                break;
            case EVFX_FootstepType::Medium:
                ModifiedConfig.ParticleIntensity *= 0.6f;
                ModifiedConfig.DustRadius *= 0.8f;
                break;
            case EVFX_FootstepType::Heavy:
                ModifiedConfig.ParticleIntensity *= 1.0f;
                ModifiedConfig.DustRadius *= 1.2f;
                break;
            case EVFX_FootstepType::Massive:
                ModifiedConfig.ParticleIntensity *= 2.0f;
                ModifiedConfig.DustRadius *= 2.0f;
                break;
        }
        
        SpawnParticleEffect(Location, SystemToUse, ModifiedConfig);
    }
}

void UVFX_FootstepParticleManager::TriggerDinosaurFootstep(const FVector& Location, EVFX_FootstepType StepType, EVFX_SurfaceType Surface)
{
    FVFX_FootstepConfig Config;
    Config.FootstepType = StepType;
    Config.SurfaceType = Surface;
    
    // Set surface-specific properties
    switch (Surface)
    {
        case EVFX_SurfaceType::Dirt:
            Config.DustColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);
            Config.DustRadius = 60.0f;
            break;
        case EVFX_SurfaceType::Mud:
            Config.DustColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
            Config.DustRadius = 80.0f;
            Config.ParticleIntensity = 1.5f;
            break;
        case EVFX_SurfaceType::Rock:
            Config.DustColor = FLinearColor(0.7f, 0.7f, 0.6f, 1.0f);
            Config.DustRadius = 40.0f;
            Config.ParticleIntensity = 0.8f;
            break;
        case EVFX_SurfaceType::Sand:
            Config.DustColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);
            Config.DustRadius = 100.0f;
            Config.ParticleIntensity = 1.2f;
            break;
        case EVFX_SurfaceType::Grass:
            Config.DustColor = FLinearColor(0.4f, 0.6f, 0.2f, 1.0f);
            Config.DustRadius = 30.0f;
            Config.ParticleIntensity = 0.5f;
            break;
    }
    
    TriggerFootstepEffect(Location, Config);
}

void UVFX_FootstepParticleManager::SetFootstepIntensityMultiplier(float Multiplier)
{
    GlobalIntensityMultiplier = FMath::Clamp(Multiplier, 0.0f, 5.0f);
}

void UVFX_FootstepParticleManager::SpawnParticleEffect(const FVector& Location, UNiagaraSystem* System, const FVFX_FootstepConfig& Config)
{
    if (!System || !GetWorld())
    {
        return;
    }
    
    // Spawn Niagara component
    UNiagaraComponent* ParticleComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        FRotator::ZeroRotator,
        FVector(1.0f),
        true, // Auto destroy
        true, // Auto activate
        ENCPoolMethod::None,
        true  // Precompile effects
    );
    
    if (ParticleComponent)
    {
        // Set particle parameters
        ParticleComponent->SetFloatParameter(TEXT("Intensity"), Config.ParticleIntensity);
        ParticleComponent->SetFloatParameter(TEXT("Radius"), Config.DustRadius);
        ParticleComponent->SetColorParameter(TEXT("Color"), Config.DustColor);
        
        // Add to active components for tracking
        ActiveParticleComponents.Add(ParticleComponent);
    }
}

float UVFX_FootstepParticleManager::CalculateDistanceAttenuation(const FVector& EffectLocation) const
{
    if (!GetWorld())
    {
        return 1.0f;
    }
    
    // Get player location
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return 1.0f;
    }
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, EffectLocation);
    
    if (Distance >= MaxFootstepDistance)
    {
        return 0.0f; // Too far
    }
    
    // Linear attenuation
    return 1.0f - (Distance / MaxFootstepDistance);
}

UNiagaraSystem* UVFX_FootstepParticleManager::GetSystemForSurface(EVFX_SurfaceType SurfaceType) const
{
    switch (SurfaceType)
    {
        case EVFX_SurfaceType::Mud:
            return MudSplashSystem.LoadSynchronous();
        case EVFX_SurfaceType::Rock:
            return RockDebrisSystem.LoadSynchronous();
        case EVFX_SurfaceType::Dirt:
        case EVFX_SurfaceType::Sand:
        case EVFX_SurfaceType::Grass:
        default:
            return DustParticleSystem.LoadSynchronous();
    }
}

void UVFX_FootstepParticleManager::CleanupFinishedEffects()
{
    // Remove null or inactive components
    ActiveParticleComponents.RemoveAll([](UNiagaraComponent* Component)
    {
        return !IsValid(Component) || !Component->IsActive();
    });
}