#include "VFX_ParticleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

UVFX_ParticleManager::UVFX_ParticleManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default particle settings
    ConfigureParticleSettings();
}

void UVFX_ParticleManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeParticleSystems();
    
    // Start cleanup timer
    GetWorld()->GetTimerManager().SetTimer(
        CleanupTimer,
        this,
        &UVFX_ParticleManager::CleanupExpiredParticles,
        5.0f,
        true
    );
}

void UVFX_ParticleManager::InitializeParticleSystems()
{
    // Load particle systems from content browser
    // These would be created as Niagara assets in the editor
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Initializing particle systems"));
    
    // Initialize particle settings for each type
    for (auto& Setting : ParticleSettings)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Configured settings for particle type %d"), 
               (int32)Setting.Key);
    }
}

void UVFX_ParticleManager::ConfigureParticleSettings()
{
    // Footstep dust settings
    FVFX_ParticleSettings FootstepSettings;
    FootstepSettings.Intensity = 1.5f;
    FootstepSettings.Duration = 3.0f;
    FootstepSettings.Scale = FVector(1.0f, 1.0f, 0.8f);
    FootstepSettings.TintColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f); // Brown dust
    ParticleSettings.Add(EVFX_ParticleType::FootstepDust, FootstepSettings);

    // Blood splatter settings
    FVFX_ParticleSettings BloodSettings;
    BloodSettings.Intensity = 2.0f;
    BloodSettings.Duration = 5.0f;
    BloodSettings.Scale = FVector(0.8f, 0.8f, 1.2f);
    BloodSettings.TintColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f); // Dark red
    ParticleSettings.Add(EVFX_ParticleType::BloodSplatter, BloodSettings);

    // Ambient pollen settings
    FVFX_ParticleSettings PollenSettings;
    PollenSettings.Intensity = 0.5f;
    PollenSettings.Duration = -1.0f; // Continuous
    PollenSettings.Scale = FVector(1.0f, 1.0f, 1.0f);
    PollenSettings.TintColor = FLinearColor(1.0f, 0.9f, 0.6f, 0.3f); // Golden pollen
    PollenSettings.bAutoDestroy = false;
    ParticleSettings.Add(EVFX_ParticleType::AmbientPollen, PollenSettings);

    // Campfire sparks settings
    FVFX_ParticleSettings SparksSettings;
    SparksSettings.Intensity = 1.8f;
    SparksSettings.Duration = 4.0f;
    SparksSettings.Scale = FVector(1.2f, 1.2f, 1.5f);
    SparksSettings.TintColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f); // Orange fire
    ParticleSettings.Add(EVFX_ParticleType::CampfireSparks, SparksSettings);
}

void UVFX_ParticleManager::SpawnParticleEffect(EVFX_ParticleType ParticleType, FVector Location, FRotator Rotation)
{
    if (!ParticleSystems.Contains(ParticleType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: No particle system found for type %d"), (int32)ParticleType);
        return;
    }

    UNiagaraSystem* System = ParticleSystems[ParticleType].LoadSynchronous();
    if (!System)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Failed to load particle system for type %d"), (int32)ParticleType);
        return;
    }

    UNiagaraComponent* ParticleComponent = CreateParticleComponent(System, Location, Rotation);
    if (ParticleComponent)
    {
        // Apply settings
        if (ParticleSettings.Contains(ParticleType))
        {
            const FVFX_ParticleSettings& Settings = ParticleSettings[ParticleType];
            
            // Set component parameters
            ParticleComponent->SetFloatParameter(TEXT("Intensity"), Settings.Intensity);
            ParticleComponent->SetVectorParameter(TEXT("Scale"), Settings.Scale);
            ParticleComponent->SetColorParameter(TEXT("TintColor"), Settings.TintColor);
            
            // Auto-destroy after duration
            if (Settings.bAutoDestroy && Settings.Duration > 0.0f)
            {
                FTimerHandle DestroyTimer;
                GetWorld()->GetTimerManager().SetTimer(
                    DestroyTimer,
                    [ParticleComponent]()
                    {
                        if (IsValid(ParticleComponent))
                        {
                            ParticleComponent->DestroyComponent();
                        }
                    },
                    Settings.Duration,
                    false
                );
            }
        }

        ActiveParticles.Add(ParticleComponent);
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned particle effect type %d at location %s"), 
               (int32)ParticleType, *Location.ToString());
    }
}

void UVFX_ParticleManager::SpawnFootstepDust(FVector FootLocation, float DinosaurSize)
{
    // Adjust particle settings based on dinosaur size
    FVFX_ParticleSettings Settings = ParticleSettings[EVFX_ParticleType::FootstepDust];
    Settings.Intensity *= DinosaurSize;
    Settings.Scale *= DinosaurSize;
    
    // Temporarily override settings
    ParticleSettings[EVFX_ParticleType::FootstepDust] = Settings;
    
    SpawnParticleEffect(EVFX_ParticleType::FootstepDust, FootLocation);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned footstep dust for dinosaur size %.2f"), DinosaurSize);
}

void UVFX_ParticleManager::SpawnBloodSplatter(FVector ImpactLocation, EDamageType DamageType, float DamageAmount)
{
    // Adjust blood intensity based on damage
    FVFX_ParticleSettings Settings = ParticleSettings[EVFX_ParticleType::BloodSplatter];
    
    switch (DamageType)
    {
        case EDamageType::Light:
            Settings.Intensity *= 0.5f;
            Settings.Scale *= 0.7f;
            break;
        case EDamageType::Medium:
            Settings.Intensity *= 1.0f;
            break;
        case EDamageType::Heavy:
            Settings.Intensity *= 1.5f;
            Settings.Scale *= 1.3f;
            break;
        case EDamageType::Critical:
            Settings.Intensity *= 2.0f;
            Settings.Scale *= 1.8f;
            Settings.Duration *= 1.5f;
            break;
    }
    
    // Scale by damage amount
    Settings.Intensity *= FMath::Clamp(DamageAmount / 100.0f, 0.1f, 3.0f);
    
    ParticleSettings[EVFX_ParticleType::BloodSplatter] = Settings;
    SpawnParticleEffect(EVFX_ParticleType::BloodSplatter, ImpactLocation);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned blood splatter for damage %.2f"), DamageAmount);
}

void UVFX_ParticleManager::SpawnAmbientParticles(FVector CenterLocation, float Radius)
{
    // Create multiple ambient particle spawns in a radius
    int32 NumSpawns = FMath::RandRange(3, 8);
    
    for (int32 i = 0; i < NumSpawns; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(0.0f, Radius * 0.5f)
        );
        
        FVector SpawnLocation = CenterLocation + RandomOffset;
        SpawnParticleEffect(EVFX_ParticleType::AmbientPollen, SpawnLocation);
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned %d ambient particle systems"), NumSpawns);
}

void UVFX_ParticleManager::UpdateDayNightParticles(float TimeOfDay)
{
    // Adjust ambient particle intensity based on time of day
    // More particles during dawn/dusk, fewer at night
    float NewIntensity = 1.0f;
    
    if (TimeOfDay < 0.25f || TimeOfDay > 0.75f) // Night
    {
        NewIntensity = 0.3f;
    }
    else if (TimeOfDay < 0.4f || TimeOfDay > 0.6f) // Dawn/Dusk
    {
        NewIntensity = 1.5f;
    }
    else // Day
    {
        NewIntensity = 1.0f;
    }
    
    AmbientParticleIntensity = NewIntensity;
    
    // Update existing ambient particles
    for (UNiagaraComponent* Particle : ActiveParticles)
    {
        if (IsValid(Particle))
        {
            Particle->SetFloatParameter(TEXT("Intensity"), AmbientParticleIntensity);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Updated day/night particles intensity to %.2f"), NewIntensity);
}

void UVFX_ParticleManager::CleanupExpiredParticles()
{
    int32 CleanedCount = 0;
    
    for (int32 i = ActiveParticles.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveParticles[i]) || !ActiveParticles[i]->IsActive())
        {
            ActiveParticles.RemoveAt(i);
            CleanedCount++;
        }
    }
    
    if (CleanedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Cleaned up %d expired particles"), CleanedCount);
    }
}

UNiagaraComponent* UVFX_ParticleManager::CreateParticleComponent(UNiagaraSystem* System, FVector Location, FRotator Rotation)
{
    if (!System || !GetWorld())
    {
        return nullptr;
    }

    UNiagaraComponent* Component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        Rotation,
        FVector(1.0f),
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    return Component;
}

void UVFX_ParticleManager::TestAllParticleTypes()
{
    if (!GetWorld())
    {
        return;
    }

    FVector TestLocation = GetOwner()->GetActorLocation();
    
    // Test each particle type
    SpawnParticleEffect(EVFX_ParticleType::FootstepDust, TestLocation + FVector(100, 0, 0));
    SpawnParticleEffect(EVFX_ParticleType::BloodSplatter, TestLocation + FVector(200, 0, 0));
    SpawnParticleEffect(EVFX_ParticleType::AmbientPollen, TestLocation + FVector(300, 0, 0));
    SpawnParticleEffect(EVFX_ParticleType::CampfireSparks, TestLocation + FVector(400, 0, 0));
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Testing all particle types at location %s"), *TestLocation.ToString());
}