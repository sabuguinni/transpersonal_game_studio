#include "VFX_NiagaraManager.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AVFX_NiagaraManager::AVFX_NiagaraManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create trigger sphere component
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(TriggerRadius);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create primary VFX component
    PrimaryVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PrimaryVFX"));
    PrimaryVFXComponent->SetupAttachment(RootComponent);
    PrimaryVFXComponent->SetAutoActivate(false);

    // Initialize default settings
    TriggerRadius = 500.0f;
    EffectCooldown = 1.0f;
    bPlayerTriggered = true;
    bDinosaurTriggered = true;
    CurrentEffectType = EVFX_EffectType::FootstepDust;
    LastTriggerTime = 0.0f;
}

void AVFX_NiagaraManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (TriggerSphere)
    {
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AVFX_NiagaraManager::OnTriggerBeginOverlap);
        TriggerSphere->SetSphereRadius(TriggerRadius);
    }

    // Initialize effect data
    InitializeEffectData();
}

void AVFX_NiagaraManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update effect systems if needed
    if (PrimaryVFXComponent && PrimaryVFXComponent->IsActive())
    {
        // Handle any per-frame VFX updates here
    }
}

void AVFX_NiagaraManager::TriggerEffect(EVFX_EffectType EffectType, FVector Location, float Intensity)
{
    if (!CanTriggerEffect())
    {
        return;
    }

    // Find effect data
    FVFX_EffectData* EffectData = nullptr;
    for (FVFX_EffectData& Effect : AvailableEffects)
    {
        if (Effect.EffectType == EffectType)
        {
            EffectData = &Effect;
            break;
        }
    }

    if (!EffectData)
    {
        return;
    }

    // Get Niagara system
    UNiagaraSystem* NiagaraSystem = GetNiagaraSystemForEffect(EffectType);
    if (!NiagaraSystem)
    {
        return;
    }

    // Spawn effect at location
    UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        NiagaraSystem,
        Location,
        FRotator::ZeroRotator,
        FVector(Intensity * EffectData->IntensityScale),
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (SpawnedEffect)
    {
        // Set effect parameters based on type
        switch (EffectType)
        {
        case EVFX_EffectType::FootstepDust:
            SpawnedEffect->SetFloatParameter(TEXT("DustIntensity"), Intensity);
            SpawnedEffect->SetFloatParameter(TEXT("ParticleCount"), 50.0f * Intensity);
            break;
        case EVFX_EffectType::BloodSplatter:
            SpawnedEffect->SetFloatParameter(TEXT("BloodAmount"), Intensity);
            SpawnedEffect->SetVectorParameter(TEXT("BloodColor"), FVector(0.8f, 0.1f, 0.1f));
            break;
        case EVFX_EffectType::CampfireFire:
            SpawnedEffect->SetFloatParameter(TEXT("FlameHeight"), 100.0f * Intensity);
            SpawnedEffect->SetFloatParameter(TEXT("EmberCount"), 30.0f * Intensity);
            break;
        case EVFX_EffectType::WaterSplash:
            SpawnedEffect->SetFloatParameter(TEXT("SplashSize"), Intensity);
            SpawnedEffect->SetFloatParameter(TEXT("DropletCount"), 40.0f * Intensity);
            break;
        case EVFX_EffectType::CraftingSparks:
            SpawnedEffect->SetFloatParameter(TEXT("SparkIntensity"), Intensity);
            SpawnedEffect->SetFloatParameter(TEXT("SparkCount"), 25.0f * Intensity);
            break;
        case EVFX_EffectType::EnvironmentalDust:
            SpawnedEffect->SetFloatParameter(TEXT("WindStrength"), Intensity);
            SpawnedEffect->SetFloatParameter(TEXT("DustDensity"), 0.5f * Intensity);
            break;
        case EVFX_EffectType::VolumetricGodRays:
            SpawnedEffect->SetFloatParameter(TEXT("RayIntensity"), Intensity);
            SpawnedEffect->SetFloatParameter(TEXT("VolumetricDensity"), 0.3f * Intensity);
            break;
        case EVFX_EffectType::WeatherRain:
            SpawnedEffect->SetFloatParameter(TEXT("RainIntensity"), Intensity);
            SpawnedEffect->SetFloatParameter(TEXT("DropCount"), 100.0f * Intensity);
            break;
        }

        // Auto-destroy if configured
        if (EffectData->bAutoDestroy && EffectData->Duration > 0.0f)
        {
            FTimerHandle DestroyTimer;
            GetWorld()->GetTimerManager().SetTimer(DestroyTimer, [SpawnedEffect]()
            {
                if (IsValid(SpawnedEffect))
                {
                    SpawnedEffect->DestroyComponent();
                }
            }, EffectData->Duration, false);
        }
    }

    // Update cooldown
    LastTriggerTime = GetWorld()->GetTimeSeconds();

    // Call Blueprint event
    OnEffectTriggered(EffectType, Location);
}

void AVFX_NiagaraManager::SetEffectType(EVFX_EffectType NewEffectType)
{
    CurrentEffectType = NewEffectType;
}

void AVFX_NiagaraManager::StopAllEffects()
{
    if (PrimaryVFXComponent)
    {
        PrimaryVFXComponent->Deactivate();
    }
}

bool AVFX_NiagaraManager::IsEffectActive() const
{
    return PrimaryVFXComponent && PrimaryVFXComponent->IsActive();
}

void AVFX_NiagaraManager::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || !CanTriggerEffect())
    {
        return;
    }

    bool bShouldTrigger = false;
    float TriggerIntensity = 1.0f;

    // Check if it's a player character
    if (bPlayerTriggered && OtherActor->IsA<ACharacter>())
    {
        bShouldTrigger = true;
        TriggerIntensity = 0.8f; // Player footsteps are lighter
    }
    // Check if it's a dinosaur (assuming dinosaur actors have "Dino" in their name)
    else if (bDinosaurTriggered && OtherActor->GetName().Contains(TEXT("Dino")))
    {
        bShouldTrigger = true;
        TriggerIntensity = 2.0f; // Dinosaur footsteps are heavier
        
        // Scale intensity based on dinosaur size (rough estimation)
        if (OtherActor->GetName().Contains(TEXT("TRex")) || OtherActor->GetName().Contains(TEXT("Brachio")))
        {
            TriggerIntensity = 3.0f; // Large dinosaurs
        }
        else if (OtherActor->GetName().Contains(TEXT("Raptor")) || OtherActor->GetName().Contains(TEXT("Veloci")))
        {
            TriggerIntensity = 1.5f; // Medium dinosaurs
        }
    }

    if (bShouldTrigger)
    {
        FVector TriggerLocation = OtherActor->GetActorLocation();
        TriggerLocation.Z = GetActorLocation().Z; // Use VFX manager's Z level
        TriggerEffect(CurrentEffectType, TriggerLocation, TriggerIntensity);
    }
}

void AVFX_NiagaraManager::InitializeEffectData()
{
    AvailableEffects.Empty();

    // Initialize default effect data
    FVFX_EffectData FootstepEffect;
    FootstepEffect.EffectType = EVFX_EffectType::FootstepDust;
    FootstepEffect.IntensityScale = 1.0f;
    FootstepEffect.Duration = 2.0f;
    FootstepEffect.bAutoDestroy = true;
    AvailableEffects.Add(FootstepEffect);

    FVFX_EffectData BloodEffect;
    BloodEffect.EffectType = EVFX_EffectType::BloodSplatter;
    BloodEffect.IntensityScale = 1.5f;
    BloodEffect.Duration = 5.0f;
    BloodEffect.bAutoDestroy = true;
    AvailableEffects.Add(BloodEffect);

    FVFX_EffectData CampfireEffect;
    CampfireEffect.EffectType = EVFX_EffectType::CampfireFire;
    CampfireEffect.IntensityScale = 1.0f;
    CampfireEffect.Duration = 0.0f; // Continuous
    CampfireEffect.bAutoDestroy = false;
    AvailableEffects.Add(CampfireEffect);

    FVFX_EffectData WaterEffect;
    WaterEffect.EffectType = EVFX_EffectType::WaterSplash;
    WaterEffect.IntensityScale = 1.2f;
    WaterEffect.Duration = 3.0f;
    WaterEffect.bAutoDestroy = true;
    AvailableEffects.Add(WaterEffect);

    FVFX_EffectData CraftingEffect;
    CraftingEffect.EffectType = EVFX_EffectType::CraftingSparks;
    CraftingEffect.IntensityScale = 0.8f;
    CraftingEffect.Duration = 1.5f;
    CraftingEffect.bAutoDestroy = true;
    AvailableEffects.Add(CraftingEffect);

    FVFX_EffectData DustEffect;
    DustEffect.EffectType = EVFX_EffectType::EnvironmentalDust;
    DustEffect.IntensityScale = 0.6f;
    DustEffect.Duration = 0.0f; // Continuous
    DustEffect.bAutoDestroy = false;
    AvailableEffects.Add(DustEffect);

    FVFX_EffectData GodRayEffect;
    GodRayEffect.EffectType = EVFX_EffectType::VolumetricGodRays;
    GodRayEffect.IntensityScale = 1.0f;
    GodRayEffect.Duration = 0.0f; // Continuous
    GodRayEffect.bAutoDestroy = false;
    AvailableEffects.Add(GodRayEffect);

    FVFX_EffectData RainEffect;
    RainEffect.EffectType = EVFX_EffectType::WeatherRain;
    RainEffect.IntensityScale = 1.0f;
    RainEffect.Duration = 0.0f; // Continuous
    RainEffect.bAutoDestroy = false;
    AvailableEffects.Add(RainEffect);
}

bool AVFX_NiagaraManager::CanTriggerEffect() const
{
    if (!GetWorld())
    {
        return false;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastTriggerTime) >= EffectCooldown;
}

UNiagaraSystem* AVFX_NiagaraManager::GetNiagaraSystemForEffect(EVFX_EffectType EffectType) const
{
    // This would normally load from content browser paths
    // For now, return nullptr and let the system handle gracefully
    // In a real implementation, you would load specific Niagara systems for each effect type
    return nullptr;
}