#include "VFX_NiagaraSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"

AVFX_NiagaraSystemManager::AVFX_NiagaraSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz for performance

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create Niagara components for different effect layers
    PrimaryEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PrimaryEffectComponent"));
    PrimaryEffectComponent->SetupAttachment(RootComponent);
    PrimaryEffectComponent->SetAutoActivate(false);

    SecondaryEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SecondaryEffectComponent"));
    SecondaryEffectComponent->SetupAttachment(RootComponent);
    SecondaryEffectComponent->SetAutoActivate(false);

    AmbientEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AmbientEffectComponent"));
    AmbientEffectComponent->SetupAttachment(RootComponent);
    AmbientEffectComponent->SetAutoActivate(false);

    // Initialize default values
    GlobalVFXIntensity = 1.0f;
    CurrentEffectType = EVFX_EffectType::Fire_Campfire;
    bAutoActivateOnBeginPlay = true;
    EffectCullDistance = 5000.0f;
    CullCheckInterval = 1.0f;
    LastCullCheckTime = 0.0f;
    bIsWithinCullDistance = true;

    // Add components to tracking array
    ActiveComponents.Add(PrimaryEffectComponent);
    ActiveComponents.Add(SecondaryEffectComponent);
    ActiveComponents.Add(AmbientEffectComponent);
}

void AVFX_NiagaraSystemManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeEffectDatabase();

    if (bAutoActivateOnBeginPlay)
    {
        PlayEffect(CurrentEffectType, GetActorLocation());
    }

    UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraSystemManager: Initialized with %d effect types"), EffectDatabase.Num());
}

void AVFX_NiagaraSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateEffectCulling();
}

void AVFX_NiagaraSystemManager::InitializeEffectDatabase()
{
    // Initialize default effect data for each type
    FVFX_EffectData CampfireData;
    CampfireData.Duration = 0.0f; // Infinite duration
    CampfireData.Scale = 1.0f;
    CampfireData.bLooping = true;
    CampfireData.TintColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f); // Orange fire color
    EffectDatabase.Add(EVFX_EffectType::Fire_Campfire, CampfireData);

    FVFX_EffectData DustImpactData;
    DustImpactData.Duration = 3.0f;
    DustImpactData.Scale = 1.5f;
    DustImpactData.bLooping = false;
    DustImpactData.TintColor = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f); // Brown dust color
    EffectDatabase.Add(EVFX_EffectType::Impact_Dust, DustImpactData);

    FVFX_EffectData BloodImpactData;
    BloodImpactData.Duration = 2.0f;
    BloodImpactData.Scale = 1.0f;
    BloodImpactData.bLooping = false;
    BloodImpactData.TintColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f); // Dark red blood
    EffectDatabase.Add(EVFX_EffectType::Impact_Blood, BloodImpactData);

    FVFX_EffectData RainData;
    RainData.Duration = 0.0f; // Infinite duration
    RainData.Scale = 2.0f;
    RainData.bLooping = true;
    RainData.TintColor = FLinearColor(0.8f, 0.9f, 1.0f, 0.7f); // Light blue rain
    EffectDatabase.Add(EVFX_EffectType::Weather_Rain, RainData);

    FVFX_EffectData FootstepData;
    FootstepData.Duration = 1.5f;
    FootstepData.Scale = 2.0f;
    FootstepData.bLooping = false;
    FootstepData.TintColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f); // Earth tone
    EffectDatabase.Add(EVFX_EffectType::Dino_Footstep, FootstepData);

    FVFX_EffectData BreathData;
    BreathData.Duration = 2.0f;
    BreathData.Scale = 1.0f;
    BreathData.bLooping = false;
    BreathData.TintColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.5f); // White vapor
    EffectDatabase.Add(EVFX_EffectType::Dino_Breath, BreathData);

    FVFX_EffectData FogData;
    FogData.Duration = 0.0f; // Infinite duration
    FogData.Scale = 3.0f;
    FogData.bLooping = true;
    FogData.TintColor = FLinearColor(0.9f, 0.9f, 0.9f, 0.3f); // Light gray fog
    EffectDatabase.Add(EVFX_EffectType::Weather_Fog, FogData);

    FVFX_EffectData SplashData;
    SplashData.Duration = 1.0f;
    SplashData.Scale = 1.2f;
    SplashData.bLooping = false;
    SplashData.TintColor = FLinearColor(0.2f, 0.6f, 1.0f, 0.8f); // Blue water
    EffectDatabase.Add(EVFX_EffectType::Water_Splash, SplashData);

    FVFX_EffectData SparksData;
    SparksData.Duration = 2.0f;
    SparksData.Scale = 0.8f;
    SparksData.bLooping = false;
    SparksData.TintColor = FLinearColor(1.0f, 0.8f, 0.2f, 1.0f); // Yellow-orange sparks
    EffectDatabase.Add(EVFX_EffectType::Crafting_Sparks, SparksData);
}

void AVFX_NiagaraSystemManager::PlayEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation)
{
    if (!EffectDatabase.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraSystemManager: Effect type not found in database"));
        return;
    }

    // Stop existing effect of the same type
    StopEffect(EffectType);

    // Get available component
    UNiagaraComponent* Component = GetAvailableComponent();
    if (!Component)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_NiagaraSystemManager: No available components for effect"));
        return;
    }

    // Configure and activate effect
    const FVFX_EffectData& EffectData = EffectDatabase[EffectType];
    ConfigureNiagaraComponent(Component, EffectData);
    
    Component->SetWorldLocation(Location);
    Component->SetWorldRotation(Rotation);
    Component->Activate();

    // Track active effect
    ActiveEffects.Add(EffectType, Component);

    UE_LOG(LogTemp, Log, TEXT("VFX_NiagaraSystemManager: Playing effect %d at location %s"), 
           (int32)EffectType, *Location.ToString());
}

void AVFX_NiagaraSystemManager::StopEffect(EVFX_EffectType EffectType)
{
    if (UNiagaraComponent** ComponentPtr = ActiveEffects.Find(EffectType))
    {
        if (UNiagaraComponent* Component = *ComponentPtr)
        {
            Component->Deactivate();
        }
        ActiveEffects.Remove(EffectType);
    }
}

void AVFX_NiagaraSystemManager::StopAllEffects()
{
    for (UNiagaraComponent* Component : ActiveComponents)
    {
        if (Component)
        {
            Component->Deactivate();
        }
    }
    ActiveEffects.Empty();
}

void AVFX_NiagaraSystemManager::SetEffectIntensity(EVFX_EffectType EffectType, float Intensity)
{
    if (UNiagaraComponent** ComponentPtr = ActiveEffects.Find(EffectType))
    {
        if (UNiagaraComponent* Component = *ComponentPtr)
        {
            Component->SetFloatParameter(TEXT("Intensity"), Intensity);
        }
    }
}

void AVFX_NiagaraSystemManager::SetGlobalVFXIntensity(float Intensity)
{
    GlobalVFXIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
    
    for (UNiagaraComponent* Component : ActiveComponents)
    {
        if (Component && Component->IsActive())
        {
            Component->SetFloatParameter(TEXT("GlobalIntensity"), GlobalVFXIntensity);
        }
    }
}

void AVFX_NiagaraSystemManager::PlayDinosaurFootstepEffect(FVector Location, float DinosaurSize)
{
    FVFX_EffectData FootstepData = EffectDatabase[EVFX_EffectType::Dino_Footstep];
    FootstepData.Scale *= DinosaurSize;
    
    PlayEffect(EVFX_EffectType::Dino_Footstep, Location);
    SetEffectIntensity(EVFX_EffectType::Dino_Footstep, DinosaurSize);
}

void AVFX_NiagaraSystemManager::PlayDinosaurBreathEffect(FVector Location, FRotator Direction)
{
    PlayEffect(EVFX_EffectType::Dino_Breath, Location, Direction);
}

void AVFX_NiagaraSystemManager::PlayCampfireEffect(FVector Location)
{
    PlayEffect(EVFX_EffectType::Fire_Campfire, Location);
}

void AVFX_NiagaraSystemManager::PlayWeatherEffect(EVFX_EffectType WeatherType, float Intensity)
{
    if (WeatherType == EVFX_EffectType::Weather_Rain || WeatherType == EVFX_EffectType::Weather_Fog)
    {
        PlayEffect(WeatherType, GetActorLocation());
        SetEffectIntensity(WeatherType, Intensity);
    }
}

void AVFX_NiagaraSystemManager::PlayImpactEffect(FVector Location, EVFX_EffectType ImpactType, float ImpactForce)
{
    if (ImpactType == EVFX_EffectType::Impact_Dust || ImpactType == EVFX_EffectType::Impact_Blood)
    {
        PlayEffect(ImpactType, Location);
        SetEffectIntensity(ImpactType, ImpactForce);
    }
}

bool AVFX_NiagaraSystemManager::IsEffectActive(EVFX_EffectType EffectType) const
{
    return ActiveEffects.Contains(EffectType);
}

float AVFX_NiagaraSystemManager::GetDistanceToPlayer() const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PlayerController = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PlayerController->GetPawn())
            {
                return FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
            }
        }
    }
    return 0.0f;
}

void AVFX_NiagaraSystemManager::UpdateEffectCulling()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastCullCheckTime < CullCheckInterval)
    {
        return;
    }

    LastCullCheckTime = CurrentTime;
    float DistanceToPlayer = GetDistanceToPlayer();
    bool bShouldBeActive = DistanceToPlayer <= EffectCullDistance;

    if (bIsWithinCullDistance != bShouldBeActive)
    {
        bIsWithinCullDistance = bShouldBeActive;
        
        for (UNiagaraComponent* Component : ActiveComponents)
        {
            if (Component)
            {
                if (bIsWithinCullDistance)
                {
                    Component->SetVisibility(true);
                }
                else
                {
                    Component->SetVisibility(false);
                }
            }
        }
    }
}

UNiagaraComponent* AVFX_NiagaraSystemManager::GetAvailableComponent()
{
    for (UNiagaraComponent* Component : ActiveComponents)
    {
        if (Component && !Component->IsActive())
        {
            return Component;
        }
    }
    
    // If no inactive component found, return the primary component
    return PrimaryEffectComponent;
}

void AVFX_NiagaraSystemManager::ConfigureNiagaraComponent(UNiagaraComponent* Component, const FVFX_EffectData& EffectData)
{
    if (!Component)
    {
        return;
    }

    // Set basic parameters
    Component->SetFloatParameter(TEXT("Scale"), EffectData.Scale);
    Component->SetFloatParameter(TEXT("Duration"), EffectData.Duration);
    Component->SetFloatParameter(TEXT("GlobalIntensity"), GlobalVFXIntensity);
    Component->SetColorParameter(TEXT("TintColor"), EffectData.TintColor);
    
    // Configure looping
    if (EffectData.bLooping)
    {
        Component->SetBoolParameter(TEXT("Looping"), true);
    }
    else
    {
        Component->SetBoolParameter(TEXT("Looping"), false);
        
        // Auto-deactivate after duration if not looping
        if (EffectData.Duration > 0.0f)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [Component]()
            {
                if (Component)
                {
                    Component->Deactivate();
                }
            }, EffectData.Duration, false);
        }
    }
}