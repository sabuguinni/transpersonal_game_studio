#include "VFX_EffectManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AVFX_EffectManager::AVFX_EffectManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;
    
    // Initialize VFX settings
    GlobalVFXIntensity = 1.0f;
    bEnableEnvironmentalVFX = true;
    bEnableDinosaurVFX = true;
    bEnableWeatherVFX = true;
    MaxActiveEffects = 50;
    EffectCullingDistance = 10000.0f;
    
    ActiveEffects.Reserve(MaxActiveEffects);
}

void AVFX_EffectManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Effect Manager initialized with %d max effects"), MaxActiveEffects);
    
    // Set up performance management timer
    GetWorldTimerManager().SetTimer(
        FTimerHandle(),
        this,
        &AVFX_EffectManager::ManagePerformance,
        2.0f,
        true
    );
}

void AVFX_EffectManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateEffects(DeltaTime);
}

void AVFX_EffectManager::CreateDinosaurFootstepEffect(FVector Location, float DinosaurSize)
{
    if (!bEnableDinosaurVFX) return;
    
    FVFX_EffectData FootstepEffect;
    FootstepEffect.EffectName = FString::Printf(TEXT("DinoFootstep_%d"), FMath::RandRange(1000, 9999));
    FootstepEffect.Location = Location;
    FootstepEffect.Duration = 3.0f + (DinosaurSize * 0.5f);
    FootstepEffect.Intensity = FMath::Clamp(DinosaurSize, 0.5f, 3.0f) * GlobalVFXIntensity;
    FootstepEffect.bIsActive = true;
    
    AddEffect(FootstepEffect);
    
    UE_LOG(LogTemp, Log, TEXT("Created dinosaur footstep effect at %s, size: %.2f"), 
           *Location.ToString(), DinosaurSize);
}

void AVFX_EffectManager::CreateCampfireEffect(FVector Location, float FireIntensity)
{
    if (!bEnableEnvironmentalVFX) return;
    
    FVFX_EffectData CampfireEffect;
    CampfireEffect.EffectName = FString::Printf(TEXT("Campfire_%d"), FMath::RandRange(1000, 9999));
    CampfireEffect.Location = Location;
    CampfireEffect.Duration = 300.0f; // Long-lasting fire
    CampfireEffect.Intensity = FireIntensity * GlobalVFXIntensity;
    CampfireEffect.bIsActive = true;
    
    AddEffect(CampfireEffect);
    
    UE_LOG(LogTemp, Log, TEXT("Created campfire effect at %s, intensity: %.2f"), 
           *Location.ToString(), FireIntensity);
}

void AVFX_EffectManager::CreateWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Coverage)
{
    if (!bEnableWeatherVFX) return;
    
    FVFX_EffectData WeatherEffect;
    
    switch (WeatherType)
    {
        case EVFX_EffectType::WeatherRain:
            WeatherEffect.EffectName = FString::Printf(TEXT("Rain_%d"), FMath::RandRange(1000, 9999));
            WeatherEffect.Duration = 120.0f;
            break;
        case EVFX_EffectType::WeatherFog:
            WeatherEffect.EffectName = FString::Printf(TEXT("Fog_%d"), FMath::RandRange(1000, 9999));
            WeatherEffect.Duration = 180.0f;
            break;
        case EVFX_EffectType::VolcanicAsh:
            WeatherEffect.EffectName = FString::Printf(TEXT("VolcanicAsh_%d"), FMath::RandRange(1000, 9999));
            WeatherEffect.Duration = 240.0f;
            break;
        default:
            WeatherEffect.EffectName = TEXT("UnknownWeather");
            WeatherEffect.Duration = 60.0f;
            break;
    }
    
    WeatherEffect.Location = Location;
    WeatherEffect.Intensity = Coverage * GlobalVFXIntensity;
    WeatherEffect.bIsActive = true;
    
    AddEffect(WeatherEffect);
    
    UE_LOG(LogTemp, Log, TEXT("Created weather effect %s at %s, coverage: %.2f"), 
           *WeatherEffect.EffectName, *Location.ToString(), Coverage);
}

void AVFX_EffectManager::CreateBloodEffect(FVector Location, FVector ImpactDirection, float Amount)
{
    if (!bEnableDinosaurVFX) return;
    
    FVFX_EffectData BloodEffect;
    BloodEffect.EffectName = FString::Printf(TEXT("Blood_%d"), FMath::RandRange(1000, 9999));
    BloodEffect.Location = Location;
    BloodEffect.Duration = 5.0f;
    BloodEffect.Intensity = Amount * GlobalVFXIntensity;
    BloodEffect.bIsActive = true;
    
    AddEffect(BloodEffect);
    
    UE_LOG(LogTemp, Log, TEXT("Created blood effect at %s, amount: %.2f"), 
           *Location.ToString(), Amount);
}

void AVFX_EffectManager::CreateRockImpactEffect(FVector Location, float ImpactForce)
{
    if (!bEnableEnvironmentalVFX) return;
    
    FVFX_EffectData RockImpact;
    RockImpact.EffectName = FString::Printf(TEXT("RockImpact_%d"), FMath::RandRange(1000, 9999));
    RockImpact.Location = Location;
    RockImpact.Duration = 2.0f;
    RockImpact.Intensity = FMath::Clamp(ImpactForce / 1000.0f, 0.1f, 2.0f) * GlobalVFXIntensity;
    RockImpact.bIsActive = true;
    
    AddEffect(RockImpact);
    
    UE_LOG(LogTemp, Log, TEXT("Created rock impact effect at %s, force: %.2f"), 
           *Location.ToString(), ImpactForce);
}

void AVFX_EffectManager::StopEffect(const FString& EffectName)
{
    FVFX_EffectData* Effect = FindEffect(EffectName);
    if (Effect)
    {
        Effect->bIsActive = false;
        UE_LOG(LogTemp, Log, TEXT("Stopped VFX effect: %s"), *EffectName);
    }
}

void AVFX_EffectManager::StopAllEffects()
{
    for (FVFX_EffectData& Effect : ActiveEffects)
    {
        Effect.bIsActive = false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Stopped all VFX effects (%d total)"), ActiveEffects.Num());
}

void AVFX_EffectManager::SetGlobalVFXIntensity(float NewIntensity)
{
    GlobalVFXIntensity = FMath::Clamp(NewIntensity, 0.0f, 2.0f);
    UE_LOG(LogTemp, Log, TEXT("Set global VFX intensity to %.2f"), GlobalVFXIntensity);
}

int32 AVFX_EffectManager::GetActiveEffectCount() const
{
    int32 ActiveCount = 0;
    for (const FVFX_EffectData& Effect : ActiveEffects)
    {
        if (Effect.bIsActive)
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

void AVFX_EffectManager::CleanupExpiredEffects()
{
    ActiveEffects.RemoveAll([](const FVFX_EffectData& Effect)
    {
        return !Effect.bIsActive || Effect.Duration <= 0.0f;
    });
}

bool AVFX_EffectManager::IsEffectActive(const FString& EffectName) const
{
    for (const FVFX_EffectData& Effect : ActiveEffects)
    {
        if (Effect.EffectName == EffectName && Effect.bIsActive)
        {
            return true;
        }
    }
    return false;
}

void AVFX_EffectManager::UpdateEffects(float DeltaTime)
{
    for (FVFX_EffectData& Effect : ActiveEffects)
    {
        if (Effect.bIsActive)
        {
            Effect.Duration -= DeltaTime;
            if (Effect.Duration <= 0.0f)
            {
                Effect.bIsActive = false;
            }
        }
    }
}

void AVFX_EffectManager::CullDistantEffects()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (FVFX_EffectData& Effect : ActiveEffects)
    {
        float Distance = FVector::Dist(PlayerLocation, Effect.Location);
        if (Distance > EffectCullingDistance)
        {
            Effect.bIsActive = false;
        }
    }
}

void AVFX_EffectManager::ManagePerformance()
{
    CullDistantEffects();
    CleanupExpiredEffects();
    
    int32 ActiveCount = GetActiveEffectCount();
    if (ActiveCount > MaxActiveEffects)
    {
        // Disable oldest effects
        int32 ToRemove = ActiveCount - MaxActiveEffects;
        for (int32 i = 0; i < ActiveEffects.Num() && ToRemove > 0; i++)
        {
            if (ActiveEffects[i].bIsActive)
            {
                ActiveEffects[i].bIsActive = false;
                ToRemove--;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("VFX performance management: disabled %d effects"), 
               ActiveCount - MaxActiveEffects);
    }
}

FVFX_EffectData* AVFX_EffectManager::FindEffect(const FString& EffectName)
{
    for (FVFX_EffectData& Effect : ActiveEffects)
    {
        if (Effect.EffectName == EffectName)
        {
            return &Effect;
        }
    }
    return nullptr;
}

void AVFX_EffectManager::RemoveEffect(const FString& EffectName)
{
    ActiveEffects.RemoveAll([&EffectName](const FVFX_EffectData& Effect)
    {
        return Effect.EffectName == EffectName;
    });
}

void AVFX_EffectManager::AddEffect(const FVFX_EffectData& NewEffect)
{
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        CleanupExpiredEffects();
    }
    
    if (ActiveEffects.Num() < MaxActiveEffects)
    {
        ActiveEffects.Add(NewEffect);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot add VFX effect %s - at maximum capacity"), 
               *NewEffect.EffectName);
    }
}