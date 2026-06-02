#include "VFX_PrehistoricEffectsManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

AVFX_PrehistoricEffectsManager::AVFX_PrehistoricEffectsManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    GlobalIntensityMultiplier = 1.0f;
    bEnableWeatherEffects = true;
    bEnableCombatEffects = true;
    EffectCullingDistance = 5000.0f;

    InitializeEffectLibrary();
}

void AVFX_PrehistoricEffectsManager::BeginPlay()
{
    Super::BeginPlay();

    // Start cleanup timer for expired effects
    GetWorldTimerManager().SetTimer(EffectCleanupTimer, this, 
        &AVFX_PrehistoricEffectsManager::CleanupExpiredEffects, 2.0f, true);

    // Start LOD update timer
    GetWorldTimerManager().SetTimer(LODUpdateTimer, this, 
        &AVFX_PrehistoricEffectsManager::UpdateEffectLOD, 1.0f, true);

    UE_LOG(LogTemp, Log, TEXT("VFX_PrehistoricEffectsManager initialized with %d effect types"), 
        AvailableEffects.Num());
}

void AVFX_PrehistoricEffectsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update active effects based on distance to player
    if (ActiveEffectComponents.Num() > 0)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            for (int32 i = ActiveEffectComponents.Num() - 1; i >= 0; i--)
            {
                if (ActiveEffectComponents[i] && IsValid(ActiveEffectComponents[i]))
                {
                    float Distance = FVector::Dist(ActiveEffectComponents[i]->GetComponentLocation(), PlayerLocation);
                    
                    // Cull distant effects for performance
                    if (Distance > EffectCullingDistance)
                    {
                        ActiveEffectComponents[i]->SetVisibility(false);
                    }
                    else
                    {
                        ActiveEffectComponents[i]->SetVisibility(true);
                        
                        // Adjust intensity based on distance
                        float DistanceRatio = FMath::Clamp(1.0f - (Distance / EffectCullingDistance), 0.1f, 1.0f);
                        ActiveEffectComponents[i]->SetFloatParameter(FName("IntensityMultiplier"), 
                            GlobalIntensityMultiplier * DistanceRatio);
                    }
                }
                else
                {
                    ActiveEffectComponents.RemoveAt(i);
                }
            }
        }
    }
}

UNiagaraComponent* AVFX_PrehistoricEffectsManager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, float CustomDuration)
{
    // Find effect data for the requested type
    FVFX_EffectData* FoundEffect = AvailableEffects.FindByPredicate([EffectType](const FVFX_EffectData& Effect)
    {
        return Effect.EffectType == EffectType;
    });

    if (!FoundEffect)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect type not found: %d"), (int32)EffectType);
        return nullptr;
    }

    UNiagaraComponent* EffectComponent = CreateEffectComponent(*FoundEffect, Location, Rotation);
    
    if (EffectComponent)
    {
        ActiveEffectComponents.Add(EffectComponent);
        
        // Set custom duration if provided
        float EffectDuration = (CustomDuration > 0.0f) ? CustomDuration : FoundEffect->Duration;
        
        if (!FoundEffect->bLooping && EffectDuration > 0.0f)
        {
            // Auto-destroy non-looping effects after duration
            FTimerHandle DestroyTimer;
            GetWorldTimerManager().SetTimer(DestroyTimer, [EffectComponent]()
            {
                if (EffectComponent && IsValid(EffectComponent))
                {
                    EffectComponent->DestroyComponent();
                }
            }, EffectDuration, false);
        }
    }

    return EffectComponent;
}

void AVFX_PrehistoricEffectsManager::SpawnCampfireEffect(FVector Location)
{
    UNiagaraComponent* FireEffect = SpawnEffect(EVFX_EffectType::Fire_Campfire, Location);
    if (FireEffect)
    {
        FireEffect->SetFloatParameter(FName("FireIntensity"), 1.2f);
        FireEffect->SetVectorParameter(FName("WindDirection"), FVector(0.1f, 0.0f, 1.0f));
        UE_LOG(LogTemp, Log, TEXT("Campfire effect spawned at location: %s"), *Location.ToString());
    }
}

void AVFX_PrehistoricEffectsManager::SpawnFootstepDustEffect(FVector Location, float ImpactStrength)
{
    UNiagaraComponent* DustEffect = SpawnEffect(EVFX_EffectType::Dust_FootstepImpact, Location);
    if (DustEffect)
    {
        float ClampedStrength = FMath::Clamp(ImpactStrength, 0.1f, 3.0f);
        DustEffect->SetFloatParameter(FName("ImpactStrength"), ClampedStrength);
        DustEffect->SetFloatParameter(FName("DustAmount"), ClampedStrength * 0.8f);
        UE_LOG(LogTemp, Log, TEXT("Footstep dust effect spawned with strength: %f"), ClampedStrength);
    }
}

void AVFX_PrehistoricEffectsManager::SpawnBloodSplashEffect(FVector Location, FVector Direction)
{
    if (!bEnableCombatEffects)
    {
        return;
    }

    UNiagaraComponent* BloodEffect = SpawnEffect(EVFX_EffectType::Combat_BloodSplash, Location);
    if (BloodEffect)
    {
        FVector NormalizedDirection = Direction.GetSafeNormal();
        BloodEffect->SetVectorParameter(FName("SplashDirection"), NormalizedDirection);
        BloodEffect->SetFloatParameter(FName("SplashVelocity"), 500.0f);
        UE_LOG(LogTemp, Log, TEXT("Blood splash effect spawned"));
    }
}

void AVFX_PrehistoricEffectsManager::SpawnWaterSplashEffect(FVector Location, float SplashSize)
{
    UNiagaraComponent* WaterEffect = SpawnEffect(EVFX_EffectType::Water_Splash, Location);
    if (WaterEffect)
    {
        float ClampedSize = FMath::Clamp(SplashSize, 0.1f, 5.0f);
        WaterEffect->SetFloatParameter(FName("SplashSize"), ClampedSize);
        WaterEffect->SetFloatParameter(FName("WaterHeight"), 0.0f);
        UE_LOG(LogTemp, Log, TEXT("Water splash effect spawned with size: %f"), ClampedSize);
    }
}

void AVFX_PrehistoricEffectsManager::StartWeatherEffect(EVFX_EffectType WeatherType, float Intensity)
{
    if (!bEnableWeatherEffects)
    {
        return;
    }

    // Stop any existing weather effect of the same type
    StopWeatherEffect(WeatherType);

    // Spawn weather effect at manager location with large scale
    UNiagaraComponent* WeatherEffect = SpawnEffect(WeatherType, GetActorLocation(), FRotator::ZeroRotator, -1.0f);
    if (WeatherEffect)
    {
        WeatherEffect->SetFloatParameter(FName("WeatherIntensity"), FMath::Clamp(Intensity, 0.0f, 2.0f));
        WeatherEffect->SetVectorParameter(FName("WeatherScale"), FVector(100.0f, 100.0f, 50.0f));
        UE_LOG(LogTemp, Log, TEXT("Weather effect started: %d with intensity: %f"), (int32)WeatherType, Intensity);
    }
}

void AVFX_PrehistoricEffectsManager::StopWeatherEffect(EVFX_EffectType WeatherType)
{
    for (int32 i = ActiveEffectComponents.Num() - 1; i >= 0; i--)
    {
        if (ActiveEffectComponents[i] && IsValid(ActiveEffectComponents[i]))
        {
            // Check if this component represents the weather type we want to stop
            // This would require storing effect type metadata on components
            // For now, we'll use a simple approach
            ActiveEffectComponents[i]->DestroyComponent();
            ActiveEffectComponents.RemoveAt(i);
        }
    }
}

void AVFX_PrehistoricEffectsManager::SetGlobalIntensity(float NewIntensity)
{
    GlobalIntensityMultiplier = FMath::Clamp(NewIntensity, 0.0f, 3.0f);
    
    // Update all active effects
    for (UNiagaraComponent* Component : ActiveEffectComponents)
    {
        if (Component && IsValid(Component))
        {
            Component->SetFloatParameter(FName("GlobalIntensity"), GlobalIntensityMultiplier);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Global VFX intensity set to: %f"), GlobalIntensityMultiplier);
}

void AVFX_PrehistoricEffectsManager::CleanupExpiredEffects()
{
    for (int32 i = ActiveEffectComponents.Num() - 1; i >= 0; i--)
    {
        if (!ActiveEffectComponents[i] || !IsValid(ActiveEffectComponents[i]) || 
            !ActiveEffectComponents[i]->IsActive())
        {
            ActiveEffectComponents.RemoveAt(i);
        }
    }
}

void AVFX_PrehistoricEffectsManager::InitializeEffectLibrary()
{
    AvailableEffects.Empty();

    // Initialize campfire effect
    FVFX_EffectData CampfireData;
    CampfireData.EffectType = EVFX_EffectType::Fire_Campfire;
    CampfireData.Duration = -1.0f; // Infinite
    CampfireData.bLooping = true;
    CampfireData.Scale = FVector(1.0f);
    AvailableEffects.Add(CampfireData);

    // Initialize footstep dust effect
    FVFX_EffectData FootstepData;
    FootstepData.EffectType = EVFX_EffectType::Dust_FootstepImpact;
    FootstepData.Duration = 2.0f;
    FootstepData.bLooping = false;
    FootstepData.Scale = FVector(1.0f);
    AvailableEffects.Add(FootstepData);

    // Initialize rain effect
    FVFX_EffectData RainData;
    RainData.EffectType = EVFX_EffectType::Weather_Rain;
    RainData.Duration = -1.0f; // Infinite
    RainData.bLooping = true;
    RainData.Scale = FVector(10.0f, 10.0f, 5.0f);
    AvailableEffects.Add(RainData);

    // Initialize blood splash effect
    FVFX_EffectData BloodData;
    BloodData.EffectType = EVFX_EffectType::Combat_BloodSplash;
    BloodData.Duration = 1.5f;
    BloodData.bLooping = false;
    BloodData.Scale = FVector(1.0f);
    AvailableEffects.Add(BloodData);

    // Initialize water splash effect
    FVFX_EffectData WaterData;
    WaterData.EffectType = EVFX_EffectType::Water_Splash;
    WaterData.Duration = 3.0f;
    WaterData.bLooping = false;
    WaterData.Scale = FVector(1.0f);
    AvailableEffects.Add(WaterData);

    UE_LOG(LogTemp, Log, TEXT("VFX Effect library initialized with %d effects"), AvailableEffects.Num());
}

UNiagaraComponent* AVFX_PrehistoricEffectsManager::CreateEffectComponent(const FVFX_EffectData& EffectData, FVector Location, FRotator Rotation)
{
    if (!EffectData.NiagaraSystem.IsValid())
    {
        // For now, create a basic Niagara component without a specific system
        // In a real implementation, this would load the appropriate Niagara asset
        UE_LOG(LogTemp, Warning, TEXT("No Niagara system assigned for effect type: %d"), (int32)EffectData.EffectType);
    }

    UNiagaraComponent* EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        EffectData.NiagaraSystem.Get(),
        Location,
        Rotation,
        EffectData.Scale,
        true,
        true,
        ENCPoolMethod::None,
        true
    );

    if (EffectComponent)
    {
        EffectComponent->SetFloatParameter(FName("IntensityMultiplier"), 
            EffectData.IntensityMultiplier * GlobalIntensityMultiplier);
        
        UE_LOG(LogTemp, Log, TEXT("VFX Effect component created at location: %s"), *Location.ToString());
    }

    return EffectComponent;
}

void AVFX_PrehistoricEffectsManager::UpdateEffectLOD()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    for (UNiagaraComponent* Component : ActiveEffectComponents)
    {
        if (Component && IsValid(Component))
        {
            float Distance = FVector::Dist(Component->GetComponentLocation(), PlayerLocation);
            
            // Set LOD based on distance
            if (Distance < 1000.0f)
            {
                Component->SetFloatParameter(FName("LODLevel"), 0.0f); // High quality
            }
            else if (Distance < 3000.0f)
            {
                Component->SetFloatParameter(FName("LODLevel"), 1.0f); // Medium quality
            }
            else
            {
                Component->SetFloatParameter(FName("LODLevel"), 2.0f); // Low quality
            }
        }
    }
}