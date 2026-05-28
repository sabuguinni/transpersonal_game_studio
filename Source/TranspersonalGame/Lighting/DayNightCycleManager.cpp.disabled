#include "DayNightCycleManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;

    // Initialize default values
    CycleSpeed = 1.0f;
    CurrentTimeOfDay = 12.0f;
    bEnableAutomaticCycle = true;
    bUseSmoothTransitions = true;
    bCyclePaused = false;
    LastTimeOfDay = ELight_TimeOfDay::Midday;

    // Initialize transition state
    bIsTransitioning = false;
    TransitionStartTime = 0.0f;
    TransitionDuration = 5.0f;
    TransitionTargetTime = 12.0f;
    TransitionStartTimeValue = 12.0f;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();

    // Register with subsystem
    if (UWorld* World = GetWorld())
    {
        if (ULight_DayNightSubsystem* Subsystem = World->GetSubsystem<ULight_DayNightSubsystem>())
        {
            Subsystem->RegisterDayNightManager(this);
        }
    }

    // Find lighting actors if not assigned
    if (!SunLight || !SkyAtmosphere || !AtmosphericFog)
    {
        SetupDefaultLightingActors();
    }

    // Initialize default settings if empty
    if (TimeOfDaySettings.Num() == 0)
    {
        InitializeDefaultTimeSettings();
    }

    // Initial lighting update
    UpdateLighting();
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableAutomaticCycle && !bCyclePaused)
    {
        // Handle transitions
        if (bIsTransitioning)
        {
            float ElapsedTime = GetWorld()->GetTimeSeconds() - TransitionStartTime;
            float Alpha = FMath::Clamp(ElapsedTime / TransitionDuration, 0.0f, 1.0f);

            if (Alpha >= 1.0f)
            {
                // Transition complete
                CurrentTimeOfDay = TransitionTargetTime;
                bIsTransitioning = false;
            }
            else
            {
                // Interpolate during transition
                CurrentTimeOfDay = FMath::Lerp(TransitionStartTimeValue, TransitionTargetTime, Alpha);
            }
        }
        else
        {
            // Normal cycle progression
            CurrentTimeOfDay += DeltaTime * CycleSpeed;
            if (CurrentTimeOfDay >= 24.0f)
            {
                CurrentTimeOfDay -= 24.0f;
            }
        }

        // Update lighting
        UpdateLighting();

        // Check for time of day changes
        ELight_TimeOfDay CurrentEnum = GetCurrentTimeOfDayEnum();
        if (CurrentEnum != LastTimeOfDay)
        {
            LastTimeOfDay = CurrentEnum;
            // Broadcast time change event (could add delegate here)
        }
    }
}

void ADayNightCycleManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Fmod(FMath::Abs(NewTime), 24.0f);
    UpdateLighting();
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDayEnum() const
{
    return TimeFloatToEnum(CurrentTimeOfDay);
}

void ADayNightCycleManager::SetCycleSpeed(float NewSpeed)
{
    CycleSpeed = FMath::Max(0.0f, NewSpeed);
}

void ADayNightCycleManager::PauseCycle()
{
    bCyclePaused = true;
}

void ADayNightCycleManager::ResumeCycle()
{
    bCyclePaused = false;
}

void ADayNightCycleManager::TransitionToTimeOfDay(ELight_TimeOfDay TargetTime, float TransitionDuration)
{
    float TargetTimeFloat = 0.0f;
    switch (TargetTime)
    {
        case ELight_TimeOfDay::Dawn: TargetTimeFloat = 6.0f; break;
        case ELight_TimeOfDay::Morning: TargetTimeFloat = 9.0f; break;
        case ELight_TimeOfDay::Midday: TargetTimeFloat = 12.0f; break;
        case ELight_TimeOfDay::Afternoon: TargetTimeFloat = 15.0f; break;
        case ELight_TimeOfDay::Dusk: TargetTimeFloat = 18.0f; break;
        case ELight_TimeOfDay::Night: TargetTimeFloat = 21.0f; break;
        case ELight_TimeOfDay::Midnight: TargetTimeFloat = 0.0f; break;
    }

    bIsTransitioning = true;
    TransitionStartTime = GetWorld()->GetTimeSeconds();
    TransitionDuration = FMath::Max(0.1f, TransitionDuration);
    TransitionTargetTime = TargetTimeFloat;
    TransitionStartTimeValue = CurrentTimeOfDay;
}

void ADayNightCycleManager::SetupDefaultLightingActors()
{
    if (!SunLight)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
        for (AActor* Actor : FoundActors)
        {
            if (Actor->GetName().Contains("Sun") || Actor->GetName().Contains("Main"))
            {
                SunLight = Cast<ADirectionalLight>(Actor);
                break;
            }
        }
    }

    if (!SkyAtmosphere)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyAtmosphere::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
        }
    }

    if (!AtmosphericFog)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            AtmosphericFog = Cast<AExponentialHeightFog>(FoundActors[0]);
        }
    }
}

void ADayNightCycleManager::InitializeDefaultTimeSettings()
{
    TimeOfDaySettings.Empty();

    // Dawn (6:00)
    FLight_TimeOfDaySettings DawnSettings;
    DawnSettings.SunIntensity = 1.5f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DawnSettings.SunAngle = -10.0f;
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Dawn, DawnSettings);

    // Morning (9:00)
    FLight_TimeOfDaySettings MorningSettings;
    MorningSettings.SunIntensity = 2.5f;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    MorningSettings.SunAngle = -30.0f;
    MorningSettings.FogDensity = 0.03f;
    MorningSettings.FogColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Morning, MorningSettings);

    // Midday (12:00)
    FLight_TimeOfDaySettings MiddaySettings;
    MiddaySettings.SunIntensity = 3.5f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    MiddaySettings.SunAngle = -60.0f;
    MiddaySettings.FogDensity = 0.01f;
    MiddaySettings.FogColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Midday, MiddaySettings);

    // Afternoon (15:00)
    FLight_TimeOfDaySettings AfternoonSettings;
    AfternoonSettings.SunIntensity = 3.0f;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    AfternoonSettings.SunAngle = -45.0f;
    AfternoonSettings.FogDensity = 0.02f;
    AfternoonSettings.FogColor = FLinearColor(0.5f, 0.6f, 0.9f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Afternoon, AfternoonSettings);

    // Dusk (18:00)
    FLight_TimeOfDaySettings DuskSettings;
    DuskSettings.SunIntensity = 1.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    DuskSettings.SunAngle = -5.0f;
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogColor = FLinearColor(0.9f, 0.4f, 0.2f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Dusk, DuskSettings);

    // Night (21:00)
    FLight_TimeOfDaySettings NightSettings;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.SunAngle = 20.0f;
    NightSettings.FogDensity = 0.06f;
    NightSettings.FogColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Night, NightSettings);

    // Midnight (0:00)
    FLight_TimeOfDaySettings MidnightSettings;
    MidnightSettings.SunIntensity = 0.05f;
    MidnightSettings.SunColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    MidnightSettings.SunAngle = 45.0f;
    MidnightSettings.FogDensity = 0.08f;
    MidnightSettings.FogColor = FLinearColor(0.05f, 0.1f, 0.3f, 1.0f);
    TimeOfDaySettings.Add(ELight_TimeOfDay::Midnight, MidnightSettings);
}

void ADayNightCycleManager::UpdateLighting()
{
    if (!SunLight && !SkyAtmosphere && !AtmosphericFog)
    {
        return;
    }

    FLight_TimeOfDaySettings CurrentSettings = GetInterpolatedSettings();

    UpdateSunPosition();
    UpdateSunLighting();
    UpdateAtmosphericFog();
    UpdateSkyAtmosphere();
}

void ADayNightCycleManager::UpdateSunPosition()
{
    if (!SunLight)
        return;

    // Calculate sun angle based on time of day
    float SunAngle = (CurrentTimeOfDay - 6.0f) * 15.0f - 90.0f; // 6AM = 0°, 12PM = 90°, 6PM = 180°
    
    FRotator SunRotation = FRotator(SunAngle, 0.0f, 0.0f);
    SunLight->SetActorRotation(SunRotation);
}

void ADayNightCycleManager::UpdateSunLighting()
{
    if (!SunLight)
        return;

    UDirectionalLightComponent* LightComponent = SunLight->GetLightComponent();
    if (!LightComponent)
        return;

    FLight_TimeOfDaySettings CurrentSettings = GetInterpolatedSettings();

    LightComponent->SetIntensity(CurrentSettings.SunIntensity);
    LightComponent->SetLightColor(CurrentSettings.SunColor);
}

void ADayNightCycleManager::UpdateAtmosphericFog()
{
    if (!AtmosphericFog)
        return;

    UExponentialHeightFogComponent* FogComponent = AtmosphericFog->GetComponent();
    if (!FogComponent)
        return;

    FLight_TimeOfDaySettings CurrentSettings = GetInterpolatedSettings();

    FogComponent->SetFogDensity(CurrentSettings.FogDensity);
    FogComponent->SetFogInscatteringColor(CurrentSettings.FogColor);
}

void ADayNightCycleManager::UpdateSkyAtmosphere()
{
    if (!SkyAtmosphere)
        return;

    // Sky atmosphere updates automatically with directional light changes
    // Additional customization can be added here if needed
}

FLight_TimeOfDaySettings ADayNightCycleManager::GetInterpolatedSettings() const
{
    ELight_TimeOfDay CurrentEnum = GetCurrentTimeOfDayEnum();
    
    if (TimeOfDaySettings.Contains(CurrentEnum))
    {
        return TimeOfDaySettings[CurrentEnum];
    }

    // Fallback to midday settings
    FLight_TimeOfDaySettings DefaultSettings;
    return DefaultSettings;
}

ELight_TimeOfDay ADayNightCycleManager::TimeFloatToEnum(float TimeValue) const
{
    if (TimeValue >= 0.0f && TimeValue < 3.0f) return ELight_TimeOfDay::Midnight;
    if (TimeValue >= 3.0f && TimeValue < 7.5f) return ELight_TimeOfDay::Dawn;
    if (TimeValue >= 7.5f && TimeValue < 10.5f) return ELight_TimeOfDay::Morning;
    if (TimeValue >= 10.5f && TimeValue < 13.5f) return ELight_TimeOfDay::Midday;
    if (TimeValue >= 13.5f && TimeValue < 16.5f) return ELight_TimeOfDay::Afternoon;
    if (TimeValue >= 16.5f && TimeValue < 19.5f) return ELight_TimeOfDay::Dusk;
    if (TimeValue >= 19.5f && TimeValue < 24.0f) return ELight_TimeOfDay::Night;
    
    return ELight_TimeOfDay::Midday;
}

// Subsystem Implementation
void ULight_DayNightSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    DayNightManager = nullptr;
}

void ULight_DayNightSubsystem::Deinitialize()
{
    DayNightManager = nullptr;
    Super::Deinitialize();
}

ADayNightCycleManager* ULight_DayNightSubsystem::GetDayNightManager() const
{
    return DayNightManager;
}

void ULight_DayNightSubsystem::RegisterDayNightManager(ADayNightCycleManager* Manager)
{
    DayNightManager = Manager;
}