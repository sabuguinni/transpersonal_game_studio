#include "VFX_NiagaraEffectManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UVFX_NiagaraEffectManager::UVFX_NiagaraEffectManager()
{
    // Constructor implementation
}

void UVFX_NiagaraEffectManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraEffectManager: Initializing VFX system"));
    
    InitializeEffectRegistry();
    InitializeBiomeEffects();
    
    // Setup cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CleanupTimerHandle,
            this,
            &UVFX_NiagaraEffectManager::CleanupExpiredEffects,
            CleanupInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraEffectManager: Initialization complete"));
}

void UVFX_NiagaraEffectManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraEffectManager: Deinitializing VFX system"));
    
    // Clear cleanup timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CleanupTimerHandle);
    }
    
    // Cleanup active effects
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    ActiveEffects.Empty();
    EffectCooldowns.Empty();
    
    Super::Deinitialize();
}

void UVFX_NiagaraEffectManager::InitializeEffectRegistry()
{
    // Campfire VFX
    FVFX_EffectSettings CampfireSettings;
    CampfireSettings.Scale = FVector(1.0f);
    CampfireSettings.Duration = 0.0f; // Persistent
    CampfireSettings.bAutoDestroy = false;
    CampfireSettings.SpawnCooldown = 2.0f;
    EffectRegistry.Add(TEXT("Campfire"), CampfireSettings);
    
    // Dinosaur Footstep VFX
    FVFX_EffectSettings FootstepSettings;
    FootstepSettings.Scale = FVector(1.0f);
    FootstepSettings.Duration = 3.0f;
    FootstepSettings.bAutoDestroy = true;
    FootstepSettings.SpawnCooldown = 0.5f;
    EffectRegistry.Add(TEXT("DinosaurFootstep"), FootstepSettings);
    
    // Blood Impact VFX
    FVFX_EffectSettings BloodSettings;
    BloodSettings.Scale = FVector(0.8f);
    BloodSettings.Duration = 8.0f;
    BloodSettings.bAutoDestroy = true;
    BloodSettings.SpawnCooldown = 0.2f;
    EffectRegistry.Add(TEXT("BloodImpact"), BloodSettings);
    
    // Rain VFX
    FVFX_EffectSettings RainSettings;
    RainSettings.Scale = FVector(5.0f, 5.0f, 1.0f);
    RainSettings.Duration = 0.0f; // Persistent
    RainSettings.bAutoDestroy = false;
    RainSettings.SpawnCooldown = 5.0f;
    EffectRegistry.Add(TEXT("Rain"), RainSettings);
    
    // Dust Cloud VFX
    FVFX_EffectSettings DustSettings;
    DustSettings.Scale = FVector(1.5f);
    DustSettings.Duration = 4.0f;
    DustSettings.bAutoDestroy = true;
    DustSettings.SpawnCooldown = 1.0f;
    EffectRegistry.Add(TEXT("DustCloud"), DustSettings);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraEffectManager: Effect registry initialized with %d effects"), EffectRegistry.Num());
}

void UVFX_NiagaraEffectManager::InitializeBiomeEffects()
{
    // Forest Biome
    FVFX_BiomeEffects ForestEffects;
    ForestEffects.AmbientParticles.Scale = FVector(1.0f);
    ForestEffects.AmbientParticles.Duration = 0.0f;
    ForestEffects.WeatherEffect.Scale = FVector(2.0f);
    ForestEffects.EnvironmentalDust.Scale = FVector(0.5f);
    ForestEffects.EffectIntensity = 0.8f;
    BiomeEffects.Add(EBiomeType::Forest, ForestEffects);
    
    // Swamp Biome
    FVFX_BiomeEffects SwampEffects;
    SwampEffects.AmbientParticles.Scale = FVector(1.2f);
    SwampEffects.AmbientParticles.Duration = 0.0f;
    SwampEffects.WeatherEffect.Scale = FVector(1.5f);
    SwampEffects.EnvironmentalDust.Scale = FVector(0.3f);
    SwampEffects.EffectIntensity = 1.0f;
    BiomeEffects.Add(EBiomeType::Swamp, SwampEffects);
    
    // Savanna Biome
    FVFX_BiomeEffects SavannaEffects;
    SavannaEffects.AmbientParticles.Scale = FVector(0.8f);
    SavannaEffects.AmbientParticles.Duration = 0.0f;
    SavannaEffects.WeatherEffect.Scale = FVector(3.0f);
    SavannaEffects.EnvironmentalDust.Scale = FVector(1.5f);
    SavannaEffects.EffectIntensity = 1.2f;
    BiomeEffects.Add(EBiomeType::Savanna, SavannaEffects);
    
    // Desert Biome
    FVFX_BiomeEffects DesertEffects;
    DesertEffects.AmbientParticles.Scale = FVector(0.6f);
    DesertEffects.AmbientParticles.Duration = 0.0f;
    DesertEffects.WeatherEffect.Scale = FVector(4.0f);
    DesertEffects.EnvironmentalDust.Scale = FVector(2.0f);
    DesertEffects.EffectIntensity = 1.5f;
    BiomeEffects.Add(EBiomeType::Desert, DesertEffects);
    
    // Mountain Biome
    FVFX_BiomeEffects MountainEffects;
    MountainEffects.AmbientParticles.Scale = FVector(0.7f);
    MountainEffects.AmbientParticles.Duration = 0.0f;
    MountainEffects.WeatherEffect.Scale = FVector(2.5f);
    MountainEffects.EnvironmentalDust.Scale = FVector(1.0f);
    MountainEffects.EffectIntensity = 0.9f;
    BiomeEffects.Add(EBiomeType::Mountain, MountainEffects);
    
    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraEffectManager: Biome effects initialized for %d biomes"), BiomeEffects.Num());
}

UNiagaraComponent* UVFX_NiagaraEffectManager::SpawnVFXAtLocation(const FString& EffectName, const FVector& Location, const FRotator& Rotation)
{
    if (!CanSpawnEffect(EffectName))
    {
        return nullptr;
    }
    
    FVFX_EffectSettings* Settings = EffectRegistry.Find(EffectName);
    if (!Settings)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraEffectManager: Effect '%s' not found in registry"), *EffectName);
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Create Niagara component
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        Settings->NiagaraSystem.LoadSynchronous(),
        Location,
        Rotation,
        Settings->Scale,
        Settings->bAutoDestroy
    );
    
    if (NiagaraComp)
    {
        ActiveEffects.Add(NiagaraComp);
        RegisterEffectCooldown(EffectName);
        
        // Set duration if specified
        if (Settings->Duration > 0.0f && Settings->bAutoDestroy)
        {
            FTimerHandle DestroyTimer;
            World->GetTimerManager().SetTimer(
                DestroyTimer,
                [NiagaraComp, this]()
                {
                    if (IsValid(NiagaraComp))
                    {
                        ActiveEffects.Remove(NiagaraComp);
                        NiagaraComp->DestroyComponent();
                    }
                },
                Settings->Duration,
                false
            );
        }
        
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraEffectManager: Spawned effect '%s' at location %s"), *EffectName, *Location.ToString());
    }
    
    return NiagaraComp;
}

UNiagaraComponent* UVFX_NiagaraEffectManager::SpawnVFXAttached(const FString& EffectName, USceneComponent* AttachComponent, const FVector& RelativeLocation)
{
    if (!AttachComponent || !CanSpawnEffect(EffectName))
    {
        return nullptr;
    }
    
    FVFX_EffectSettings* Settings = EffectRegistry.Find(EffectName);
    if (!Settings)
    {
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Create attached Niagara component
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
        Settings->NiagaraSystem.LoadSynchronous(),
        AttachComponent,
        NAME_None,
        RelativeLocation,
        FRotator::ZeroRotator,
        Settings->Scale,
        EAttachLocation::KeepRelativeOffset,
        Settings->bAutoDestroy
    );
    
    if (NiagaraComp)
    {
        ActiveEffects.Add(NiagaraComp);
        RegisterEffectCooldown(EffectName);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraEffectManager: Spawned attached effect '%s'"), *EffectName);
    }
    
    return NiagaraComp;
}

void UVFX_NiagaraEffectManager::SpawnCampfireVFX(const FVector& Location)
{
    UNiagaraComponent* FireEffect = SpawnVFXAtLocation(TEXT("Campfire"), Location);
    if (FireEffect)
    {
        // Additional campfire-specific setup
        FireEffect->SetFloatParameter(TEXT("FireIntensity"), 1.0f);
        FireEffect->SetVectorParameter(TEXT("WindDirection"), FVector(1.0f, 0.0f, 0.0f));
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraEffectManager: Campfire VFX spawned with custom parameters"));
    }
}

void UVFX_NiagaraEffectManager::SpawnDinosaurFootstepVFX(const FVector& Location, float DinosaurSize)
{
    UNiagaraComponent* FootstepEffect = SpawnVFXAtLocation(TEXT("DinosaurFootstep"), Location);
    if (FootstepEffect)
    {
        // Scale effect based on dinosaur size
        FVector ScaleMultiplier = FVector(DinosaurSize);
        FootstepEffect->SetVectorParameter(TEXT("EffectScale"), ScaleMultiplier);
        FootstepEffect->SetFloatParameter(TEXT("DustAmount"), DinosaurSize * 0.8f);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraEffectManager: Dinosaur footstep VFX spawned with size multiplier %.2f"), DinosaurSize);
    }
}

void UVFX_NiagaraEffectManager::SpawnBloodImpactVFX(const FVector& Location, const FVector& ImpactDirection)
{
    UNiagaraComponent* BloodEffect = SpawnVFXAtLocation(TEXT("BloodImpact"), Location);
    if (BloodEffect)
    {
        // Set impact direction for realistic blood splatter
        FVector NormalizedDirection = ImpactDirection.GetSafeNormal();
        BloodEffect->SetVectorParameter(TEXT("ImpactDirection"), NormalizedDirection);
        BloodEffect->SetFloatParameter(TEXT("ImpactForce"), ImpactDirection.Size() * 0.01f);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraEffectManager: Blood impact VFX spawned with direction %s"), *NormalizedDirection.ToString());
    }
}

void UVFX_NiagaraEffectManager::SpawnWeatherVFX(EBiomeType BiomeType, const FVector& Location, float Intensity)
{
    FVFX_BiomeEffects* BiomeSettings = BiomeEffects.Find(BiomeType);
    if (!BiomeSettings)
    {
        return;
    }
    
    // Spawn weather effect based on biome
    UNiagaraComponent* WeatherEffect = nullptr;
    
    switch (BiomeType)
    {
        case EBiomeType::Forest:
        case EBiomeType::Swamp:
            WeatherEffect = SpawnVFXAtLocation(TEXT("Rain"), Location);
            break;
        case EBiomeType::Desert:
        case EBiomeType::Savanna:
            WeatherEffect = SpawnVFXAtLocation(TEXT("DustCloud"), Location);
            break;
        case EBiomeType::Mountain:
            WeatherEffect = SpawnVFXAtLocation(TEXT("Rain"), Location);
            break;
    }
    
    if (WeatherEffect)
    {
        float FinalIntensity = Intensity * BiomeSettings->EffectIntensity;
        WeatherEffect->SetFloatParameter(TEXT("Intensity"), FinalIntensity);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraEffectManager: Weather VFX spawned for biome %d with intensity %.2f"), (int32)BiomeType, FinalIntensity);
    }
}

void UVFX_NiagaraEffectManager::SetBiomeVFXIntensity(EBiomeType BiomeType, float Intensity)
{
    FVFX_BiomeEffects* BiomeSettings = BiomeEffects.Find(BiomeType);
    if (BiomeSettings)
    {
        BiomeSettings->EffectIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraEffectManager: Set biome %d VFX intensity to %.2f"), (int32)BiomeType, Intensity);
    }
}

void UVFX_NiagaraEffectManager::CleanupExpiredEffects()
{
    int32 CleanedCount = 0;
    
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!IsValid(Effect) || !Effect->IsActive())
        {
            ActiveEffects.RemoveAt(i);
            CleanedCount++;
        }
    }
    
    // Clean up expired cooldowns
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    TArray<FString> ExpiredCooldowns;
    
    for (const auto& Cooldown : EffectCooldowns)
    {
        if (CurrentTime > Cooldown.Value)
        {
            ExpiredCooldowns.Add(Cooldown.Key);
        }
    }
    
    for (const FString& ExpiredEffect : ExpiredCooldowns)
    {
        EffectCooldowns.Remove(ExpiredEffect);
    }
    
    if (CleanedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraEffectManager: Cleaned up %d expired effects, %d active effects remaining"), CleanedCount, ActiveEffects.Num());
    }
}

bool UVFX_NiagaraEffectManager::CanSpawnEffect(const FString& EffectName)
{
    float* CooldownTime = EffectCooldowns.Find(EffectName);
    if (!CooldownTime)
    {
        return true;
    }
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    return CurrentTime > *CooldownTime;
}

void UVFX_NiagaraEffectManager::RegisterEffectCooldown(const FString& EffectName)
{
    FVFX_EffectSettings* Settings = EffectRegistry.Find(EffectName);
    if (Settings && Settings->SpawnCooldown > 0.0f)
    {
        float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        EffectCooldowns.Add(EffectName, CurrentTime + Settings->SpawnCooldown);
    }
}

UNiagaraSystem* UVFX_NiagaraEffectManager::LoadNiagaraSystem(const FString& AssetPath)
{
    return LoadObject<UNiagaraSystem>(nullptr, *AssetPath);
}