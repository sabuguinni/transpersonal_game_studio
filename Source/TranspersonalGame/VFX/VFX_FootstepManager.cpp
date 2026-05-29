#include "VFX_FootstepManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/SceneComponent.h"

UVFX_FootstepManager::UVFX_FootstepManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms for performance

    // Initialize T-Rex footstep data
    TRexFootstep.EffectScale = 3.0f;
    TRexFootstep.DustIntensity = 2.5f;
    TRexFootstep.DustColor = FColor(101, 67, 33); // Dark brown
    TRexFootstep.EffectDuration = 3.0f;

    // Initialize Raptor footstep data
    RaptorFootstep.EffectScale = 0.8f;
    RaptorFootstep.DustIntensity = 1.2f;
    RaptorFootstep.DustColor = FColor(139, 69, 19); // Medium brown
    RaptorFootstep.EffectDuration = 1.5f;

    // Initialize Brachiosaurus footstep data
    BrachiosaurusFootstep.EffectScale = 4.0f;
    BrachiosaurusFootstep.DustIntensity = 3.0f;
    BrachiosaurusFootstep.DustColor = FColor(160, 82, 45); // Saddle brown
    BrachiosaurusFootstep.EffectDuration = 4.0f;

    // Initialize generic footstep data
    GenericFootstep.EffectScale = 1.0f;
    GenericFootstep.DustIntensity = 1.0f;
    GenericFootstep.DustColor = FColor(139, 69, 19); // Brown
    GenericFootstep.EffectDuration = 2.0f;
}

void UVFX_FootstepManager::BeginPlay()
{
    Super::BeginPlay();

    // Start cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CleanupTimerHandle,
            this,
            &UVFX_FootstepManager::CleanupFinishedEffects,
            1.0f, // Every second
            true  // Loop
        );
    }
}

void UVFX_FootstepManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Remove null components from active effects array
    ActiveFootstepEffects.RemoveAll([](UNiagaraComponent* Component) {
        return !IsValid(Component);
    });
}

void UVFX_FootstepManager::TriggerFootstepVFX(FVector Location, EDinosaurSpecies DinosaurType, float ImpactForce)
{
    FVFX_FootstepData FootstepData = GetFootstepDataForSpecies(DinosaurType);
    SpawnFootstepEffect(FootstepData, Location, ImpactForce);
}

void UVFX_FootstepManager::TriggerTRexFootstep(FVector Location, float ImpactForce)
{
    SpawnFootstepEffect(TRexFootstep, Location, ImpactForce);
}

void UVFX_FootstepManager::TriggerRaptorFootstep(FVector Location, float ImpactForce)
{
    SpawnFootstepEffect(RaptorFootstep, Location, ImpactForce);
}

void UVFX_FootstepManager::TriggerBrachiosaurusFootstep(FVector Location, float ImpactForce)
{
    SpawnFootstepEffect(BrachiosaurusFootstep, Location, ImpactForce);
}

void UVFX_FootstepManager::TriggerEnvironmentalDust(FVector Location, FVector Direction, float Intensity)
{
    if (UWorld* World = GetWorld())
    {
        // Create environmental dust effect
        UNiagaraComponent* DustEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            GenericFootstep.DustEffect.LoadSynchronous(),
            Location,
            FRotator::ZeroRotator
        );

        if (DustEffect)
        {
            // Set dust parameters
            DustEffect->SetFloatParameter(TEXT("DustIntensity"), Intensity);
            DustEffect->SetVectorParameter(TEXT("WindDirection"), Direction);
            DustEffect->SetFloatParameter(TEXT("EffectScale"), 0.5f * Intensity);

            ActiveFootstepEffects.Add(DustEffect);
        }
    }
}

void UVFX_FootstepManager::TriggerFootstepWithScreenShake(FVector Location, EDinosaurSpecies DinosaurType, float ImpactForce)
{
    // Trigger footstep VFX
    TriggerFootstepVFX(Location, DinosaurType, ImpactForce);

    // Find and trigger screen shake system
    if (AActor* Owner = GetOwner())
    {
        if (UActorComponent* ScreenShakeComp = Owner->GetComponentByClass(UActorComponent::StaticClass()))
        {
            // Call screen shake function if available
            // This integrates with the Audio Agent's screen shake system
        }
    }
}

void UVFX_FootstepManager::SpawnFootstepEffect(const FVFX_FootstepData& FootstepData, FVector Location, float ImpactForce)
{
    if (UWorld* World = GetWorld())
    {
        // Load the Niagara system
        UNiagaraSystem* NiagaraSystem = FootstepData.DustEffect.LoadSynchronous();
        if (!NiagaraSystem)
        {
            // Fallback to default system path
            NiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/Dinosaur/NS_Dino_FootstepDust"));
        }

        if (NiagaraSystem)
        {
            // Spawn the footstep effect
            UNiagaraComponent* FootstepEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                World,
                NiagaraSystem,
                Location,
                FRotator::ZeroRotator
            );

            if (FootstepEffect)
            {
                // Configure effect parameters
                float FinalScale = FootstepData.EffectScale * ImpactForce;
                float FinalIntensity = FootstepData.DustIntensity * ImpactForce;

                FootstepEffect->SetFloatParameter(TEXT("EffectScale"), FinalScale);
                FootstepEffect->SetFloatParameter(TEXT("DustIntensity"), FinalIntensity);
                FootstepEffect->SetColorParameter(TEXT("DustColor"), FootstepData.DustColor);
                FootstepEffect->SetFloatParameter(TEXT("LifeTime"), FootstepData.EffectDuration);

                // Add to active effects for cleanup
                ActiveFootstepEffects.Add(FootstepEffect);

                // Auto-destroy after duration
                if (UWorld* EffectWorld = FootstepEffect->GetWorld())
                {
                    FTimerHandle DestroyTimer;
                    EffectWorld->GetTimerManager().SetTimer(
                        DestroyTimer,
                        [FootstepEffect]()
                        {
                            if (IsValid(FootstepEffect))
                            {
                                FootstepEffect->DestroyComponent();
                            }
                        },
                        FootstepData.EffectDuration + 1.0f, // Extra second for cleanup
                        false
                    );
                }
            }
        }
    }
}

FVFX_FootstepData UVFX_FootstepManager::GetFootstepDataForSpecies(EDinosaurSpecies Species)
{
    switch (Species)
    {
        case EDinosaurSpecies::TRex:
            return TRexFootstep;
        case EDinosaurSpecies::Raptor:
            return RaptorFootstep;
        case EDinosaurSpecies::Brachiosaurus:
            return BrachiosaurusFootstep;
        default:
            return GenericFootstep;
    }
}

void UVFX_FootstepManager::CleanupFinishedEffects()
{
    // Remove finished or invalid effects
    ActiveFootstepEffects.RemoveAll([](UNiagaraComponent* Component) {
        if (!IsValid(Component))
        {
            return true;
        }
        
        // Check if effect is still active
        if (Component->IsActive())
        {
            return false;
        }
        
        // Destroy inactive components
        Component->DestroyComponent();
        return true;
    });
}