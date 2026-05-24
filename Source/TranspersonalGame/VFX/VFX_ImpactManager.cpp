#include "VFX_ImpactManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/Engine.h"

AVFX_ImpactManager::AVFX_ImpactManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create Niagara particle components
    DustParticleComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DustParticleComponent"));
    DustParticleComponent->SetupAttachment(RootComponent);
    DustParticleComponent->SetAutoActivate(false);

    BloodParticleComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BloodParticleComponent"));
    BloodParticleComponent->SetupAttachment(RootComponent);
    BloodParticleComponent->SetAutoActivate(false);

    DebrisParticleComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DebrisParticleComponent"));
    DebrisParticleComponent->SetupAttachment(RootComponent);
    DebrisParticleComponent->SetAutoActivate(false);

    // Initialize default settings
    InitializeDefaultSettings();
}

void AVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: BeginPlay - Impact VFX system initialized"));
}

void AVFX_ImpactManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Clean up finished effects
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (ActiveEffects[i] && !ActiveEffects[i]->IsActive())
        {
            ActiveEffects.RemoveAt(i);
        }
    }
}

void AVFX_ImpactManager::InitializeDefaultSettings()
{
    // Initialize surface-specific settings
    FVFX_ImpactSettings DirtSettings;
    DirtSettings.DustIntensity = 1.0f;
    DirtSettings.ParticleColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f); // Brown dirt
    DirtSettings.ParticleCount = 75;
    DirtSettings.ParticleLifetime = 4.0f;
    SurfaceSettings.Add(EVFX_SurfaceType::Dirt, DirtSettings);

    FVFX_ImpactSettings StoneSettings;
    StoneSettings.DustIntensity = 0.8f;
    StoneSettings.ParticleColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f); // Gray stone
    StoneSettings.ParticleCount = 50;
    StoneSettings.ParticleLifetime = 3.0f;
    StoneSettings.ScreenShakeIntensity = 0.8f;
    SurfaceSettings.Add(EVFX_SurfaceType::Stone, StoneSettings);

    FVFX_ImpactSettings GrassSettings;
    GrassSettings.DustIntensity = 0.3f;
    GrassSettings.ParticleColor = FLinearColor(0.3f, 0.6f, 0.2f, 1.0f); // Green grass
    GrassSettings.ParticleCount = 30;
    GrassSettings.ParticleLifetime = 2.0f;
    SurfaceSettings.Add(EVFX_SurfaceType::Grass, GrassSettings);

    FVFX_ImpactSettings WaterSettings;
    WaterSettings.DustIntensity = 0.0f;
    WaterSettings.ParticleColor = FLinearColor(0.2f, 0.6f, 1.0f, 0.8f); // Blue water
    WaterSettings.ParticleCount = 100;
    WaterSettings.ParticleLifetime = 2.5f;
    SurfaceSettings.Add(EVFX_SurfaceType::Water, WaterSettings);

    FVFX_ImpactSettings SandSettings;
    SandSettings.DustIntensity = 1.2f;
    SandSettings.ParticleColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f); // Sandy yellow
    SandSettings.ParticleCount = 80;
    SandSettings.ParticleLifetime = 5.0f;
    SurfaceSettings.Add(EVFX_SurfaceType::Sand, SandSettings);

    FVFX_ImpactSettings MudSettings;
    MudSettings.DustIntensity = 0.6f;
    MudSettings.ParticleColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f); // Dark brown mud
    MudSettings.ParticleCount = 60;
    MudSettings.ParticleLifetime = 3.5f;
    SurfaceSettings.Add(EVFX_SurfaceType::Mud, MudSettings);

    // Initialize impact type settings
    FVFX_ImpactSettings FootstepSettings;
    FootstepSettings.DustIntensity = 1.5f;
    FootstepSettings.ParticleCount = 100;
    FootstepSettings.ScreenShakeIntensity = 1.0f;
    ImpactTypeSettings.Add(EVFX_ImpactType::DinosaurFootstep, FootstepSettings);

    FVFX_ImpactSettings WeaponSettings;
    WeaponSettings.DustIntensity = 0.8f;
    WeaponSettings.ParticleCount = 40;
    WeaponSettings.ScreenShakeIntensity = 0.3f;
    ImpactTypeSettings.Add(EVFX_ImpactType::WeaponHit, WeaponSettings);

    FVFX_ImpactSettings BloodSettings;
    BloodSettings.DustIntensity = 0.0f;
    BloodSettings.ParticleColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f); // Dark red blood
    BloodSettings.ParticleCount = 30;
    BloodSettings.ParticleLifetime = 8.0f;
    ImpactTypeSettings.Add(EVFX_ImpactType::BloodSplatter, BloodSettings);
}

void AVFX_ImpactManager::TriggerImpact(EVFX_ImpactType ImpactType, EVFX_SurfaceType SurfaceType, FVector Location, FVector Normal)
{
    // Get settings for this impact type and surface
    FVFX_ImpactSettings* ImpactSettings = ImpactTypeSettings.Find(ImpactType);
    FVFX_ImpactSettings* SurfaceSettings_Local = SurfaceSettings.Find(SurfaceType);

    if (!ImpactSettings || !SurfaceSettings_Local)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: Missing settings for impact type or surface type"));
        return;
    }

    // Combine settings (impact type overrides surface where applicable)
    FVFX_ImpactSettings CombinedSettings = *SurfaceSettings_Local;
    CombinedSettings.DustIntensity *= ImpactSettings->DustIntensity;
    CombinedSettings.ParticleCount = FMath::Max(ImpactSettings->ParticleCount, SurfaceSettings_Local->ParticleCount);
    CombinedSettings.ScreenShakeIntensity = ImpactSettings->ScreenShakeIntensity;

    // Apply global intensity multiplier
    CombinedSettings.DustIntensity *= GlobalIntensityMultiplier;
    CombinedSettings.ParticleCount = FMath::RoundToInt(CombinedSettings.ParticleCount * GlobalIntensityMultiplier);

    // Choose appropriate particle system
    UNiagaraSystem* SystemToUse = nullptr;
    switch (ImpactType)
    {
        case EVFX_ImpactType::DinosaurFootstep:
            SystemToUse = DustExplosionSystem;
            break;
        case EVFX_ImpactType::BloodSplatter:
            SystemToUse = BloodSplatterSystem;
            break;
        case EVFX_ImpactType::WeaponHit:
        case EVFX_ImpactType::RockCrush:
            SystemToUse = DebrisSystem;
            break;
        case EVFX_ImpactType::WaterSplash:
            SystemToUse = WaterSplashSystem;
            break;
        default:
            SystemToUse = DustExplosionSystem;
            break;
    }

    // Play the particle effect
    if (SystemToUse)
    {
        PlayParticleEffect(SystemToUse, Location, CombinedSettings);
    }

    // Apply screen shake if intensity > 0
    if (CombinedSettings.ScreenShakeIntensity > 0.0f)
    {
        ApplyScreenShake(Location, CombinedSettings.ScreenShakeIntensity);
    }

    UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Triggered %s impact on %s surface at location %s"), 
           *UEnum::GetValueAsString(ImpactType), 
           *UEnum::GetValueAsString(SurfaceType), 
           *Location.ToString());
}

void AVFX_ImpactManager::TriggerDinosaurFootstep(FVector Location, float DinosaurSize, EVFX_SurfaceType SurfaceType)
{
    // Scale effect based on dinosaur size
    float OriginalMultiplier = GlobalIntensityMultiplier;
    GlobalIntensityMultiplier *= DinosaurSize;

    TriggerImpact(EVFX_ImpactType::DinosaurFootstep, SurfaceType, Location);

    // Restore original multiplier
    GlobalIntensityMultiplier = OriginalMultiplier;
}

void AVFX_ImpactManager::TriggerWeaponImpact(FVector Location, FVector HitDirection, EVFX_SurfaceType SurfaceType)
{
    FVector Normal = -HitDirection.GetSafeNormal();
    TriggerImpact(EVFX_ImpactType::WeaponHit, SurfaceType, Location, Normal);
}

void AVFX_ImpactManager::TriggerBloodEffect(FVector Location, FVector BloodDirection, float BloodAmount)
{
    float OriginalMultiplier = GlobalIntensityMultiplier;
    GlobalIntensityMultiplier *= BloodAmount;

    TriggerImpact(EVFX_ImpactType::BloodSplatter, EVFX_SurfaceType::Stone, Location, BloodDirection.GetSafeNormal());

    GlobalIntensityMultiplier = OriginalMultiplier;
}

void AVFX_ImpactManager::TriggerEnvironmentalEffect(FVector Location, EVFX_SurfaceType SurfaceType)
{
    // Trigger a gentle environmental effect (like wind blowing dust)
    float OriginalMultiplier = GlobalIntensityMultiplier;
    GlobalIntensityMultiplier *= 0.3f; // Reduce intensity for environmental effects

    TriggerImpact(EVFX_ImpactType::DinosaurFootstep, SurfaceType, Location);

    GlobalIntensityMultiplier = OriginalMultiplier;
}

void AVFX_ImpactManager::SetImpactSettings(EVFX_ImpactType ImpactType, const FVFX_ImpactSettings& Settings)
{
    ImpactTypeSettings.Add(ImpactType, Settings);
}

void AVFX_ImpactManager::SetSurfaceSettings(EVFX_SurfaceType SurfaceType, const FVFX_ImpactSettings& Settings)
{
    SurfaceSettings.Add(SurfaceType, Settings);
}

void AVFX_ImpactManager::ClearAllActiveEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (Effect && Effect->IsActive())
        {
            Effect->DeactivateImmediate();
        }
    }
    ActiveEffects.Empty();
}

void AVFX_ImpactManager::SetEffectIntensityMultiplier(float Multiplier)
{
    GlobalIntensityMultiplier = FMath::Clamp(Multiplier, 0.0f, 10.0f);
}

void AVFX_ImpactManager::PlayParticleEffect(UNiagaraSystem* System, FVector Location, const FVFX_ImpactSettings& Settings)
{
    if (!System || !GetWorld())
    {
        return;
    }

    // Spawn Niagara effect at location
    UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), 
        System, 
        Location, 
        FRotator::ZeroRotator, 
        FVector(1.0f), 
        true, 
        true, 
        ENCPoolMethod::None, 
        true
    );

    if (SpawnedEffect)
    {
        // Set particle parameters
        SpawnedEffect->SetFloatParameter(TEXT("ParticleCount"), static_cast<float>(Settings.ParticleCount));
        SpawnedEffect->SetFloatParameter(TEXT("Lifetime"), Settings.ParticleLifetime);
        SpawnedEffect->SetFloatParameter(TEXT("Intensity"), Settings.DustIntensity);
        SpawnedEffect->SetVectorParameter(TEXT("Velocity"), Settings.ParticleVelocity);
        SpawnedEffect->SetColorParameter(TEXT("Color"), Settings.ParticleColor);

        // Add to active effects for cleanup
        ActiveEffects.Add(SpawnedEffect);
    }
}

void AVFX_ImpactManager::ApplyScreenShake(FVector Location, float Intensity)
{
    if (!GetWorld())
    {
        return;
    }

    // Apply camera shake to all player controllers
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Iterator->Get();
        if (PC && PC->PlayerCameraManager)
        {
            // Calculate distance-based intensity falloff
            FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
            float Distance = FVector::Dist(Location, CameraLocation);
            float FalloffIntensity = Intensity * FMath::Clamp(1000.0f / (Distance + 100.0f), 0.1f, 1.0f);

            // Apply screen shake (using simple camera shake for now)
            PC->PlayerCameraManager->StartCameraShake(nullptr, FalloffIntensity);
        }
    }
}