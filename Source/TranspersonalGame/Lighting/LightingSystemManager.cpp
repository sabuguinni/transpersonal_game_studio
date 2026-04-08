#include "LightingSystemManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/VolumetricCloud.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ULightingSystemManager::ULightingSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10x por segundo
    
    InitializeDefaultMoods();
}

void ULightingSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindLightingComponents();
    ForceUpdateLighting();
}

void ULightingSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoAdvanceTime)
    {
        UpdateTimeOfDay(DeltaTime);
    }
    
    UpdateWeatherSystem(DeltaTime);
    UpdateLightingMood();
}

void ULightingSystemManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Fmod(NewTime, 24.0f);
    if (CurrentTimeOfDay < 0.0f)
    {
        CurrentTimeOfDay += 24.0f;
    }
    
    ForceUpdateLighting();
}

void ULightingSystemManager::SetWeatherState(EWeatherState NewWeather)
{
    if (CurrentWeather != NewWeather)
    {
        CurrentWeather = NewWeather;
        bMoodTransitioning = true;
        MoodTransitionAlpha = 0.0f;
    }
}

void ULightingSystemManager::SetThreatLevel(EThreatLevel NewThreat)
{
    if (CurrentThreatLevel != NewThreat)
    {
        CurrentThreatLevel = NewThreat;
        bMoodTransitioning = true;
        MoodTransitionAlpha = 0.0f;
    }
}

ETimeOfDay ULightingSystemManager::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
        return ETimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
        return ETimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 14.0f)
        return ETimeOfDay::Midday;
    else if (CurrentTimeOfDay >= 14.0f && CurrentTimeOfDay < 17.0f)
        return ETimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 17.0f && CurrentTimeOfDay < 19.0f)
        return ETimeOfDay::Dusk;
    else
        return ETimeOfDay::Night;
}

FLightingMood ULightingSystemManager::GetCurrentMood() const
{
    return CurrentMood;
}

void ULightingSystemManager::ForceUpdateLighting()
{
    UpdateSunPosition();
    UpdateLightingMood();
}

void ULightingSystemManager::UpdateTimeOfDay(float DeltaTime)
{
    CurrentTimeOfDay += (DeltaTime * TimeScale) / 3600.0f; // Converter segundos para horas
    
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }
    
    UpdateSunPosition();
}

void ULightingSystemManager::UpdateWeatherSystem(float DeltaTime)
{
    WeatherTimer += DeltaTime;
    
    // Mudança automática de clima (opcional)
    if (WeatherTimer >= WeatherChangeInterval)
    {
        WeatherTimer = 0.0f;
        
        // Lógica simples de mudança de clima
        int32 RandomChange = FMath::RandRange(0, 2);
        if (RandomChange == 0) // 33% chance de mudança
        {
            int32 NewWeatherIndex = FMath::RandRange(0, 5);
            SetWeatherState(static_cast<EWeatherState>(NewWeatherIndex));
        }
    }
}

void ULightingSystemManager::UpdateLightingMood()
{
    // Determinar mood base pelo tempo do dia
    ETimeOfDay CurrentTimeEnum = GetCurrentTimeOfDayEnum();
    FLightingMood BaseMood = TimeOfDayMoods.Contains(CurrentTimeEnum) ? 
        TimeOfDayMoods[CurrentTimeEnum] : FLightingMood();
    
    // Aplicar modificações por ameaça
    FLightingMood ThreatModifier = ThreatMoods.Contains(CurrentThreatLevel) ?
        ThreatMoods[CurrentThreatLevel] : FLightingMood();
    
    // Calcular mood alvo
    TargetMood = BlendMoods(BaseMood, ThreatModifier, 0.5f);
    
    // Transição suave
    if (bMoodTransitioning)
    {
        MoodTransitionAlpha += GetWorld()->GetDeltaSeconds() * ThreatTransitionSpeed;
        
        if (MoodTransitionAlpha >= 1.0f)
        {
            MoodTransitionAlpha = 1.0f;
            bMoodTransitioning = false;
        }
        
        CurrentMood = BlendMoods(CurrentMood, TargetMood, MoodTransitionAlpha);
    }
    else
    {
        CurrentMood = TargetMood;
    }
    
    // Aplicar mood atual
    ApplyLightingMood(CurrentMood);
}

void ULightingSystemManager::ApplyLightingMood(const FLightingMood& Mood)
{
    UpdateSkyLight();
    UpdateAtmosphere();
    UpdateFog();
    UpdateClouds();
}

void ULightingSystemManager::UpdateSunPosition()
{
    if (!SunLight) return;
    
    // Converter hora do dia para ângulo do sol
    float SunAngle = (CurrentTimeOfDay - 6.0f) * 15.0f; // 6h = 0°, 18h = 180°
    
    // Posição do sol baseada na hora
    FRotator SunRotation;
    SunRotation.Pitch = SunAngle - 90.0f; // -90 para que meio-dia seja vertical
    SunRotation.Yaw = 0.0f; // Sol nasce no leste, se põe no oeste
    SunRotation.Roll = 0.0f;
    
    SunLight->SetActorRotation(SunRotation);
    
    // Ajustar intensidade baseada na altura do sol
    float SunHeight = FMath::Sin(FMath::DegreesToRadians(SunAngle));
    float SunIntensity = FMath::Max(0.0f, SunHeight) * CurrentMood.SunIntensity;
    
    if (UDirectionalLightComponent* SunComponent = SunLight->GetLightComponent())
    {
        SunComponent->SetIntensity(SunIntensity);
        SunComponent->SetLightColor(CurrentMood.SunColor);
    }
}

void ULightingSystemManager::UpdateSkyLight()
{
    if (!SkyLightActor) return;
    
    if (USkyLightComponent* SkyComponent = SkyLightActor->GetLightComponent())
    {
        SkyComponent->SetLightColor(CurrentMood.SkyColor);
        SkyComponent->RecaptureSky();
    }
}

void ULightingSystemManager::UpdateAtmosphere()
{
    if (!SkyAtmosphereActor) return;
    
    if (USkyAtmosphereComponent* AtmosphereComponent = SkyAtmosphereActor->GetComponent())
    {
        // Ajustar propriedades atmosféricas baseadas no mood
        // Implementação específica dependeria das propriedades expostas
    }
}

void ULightingSystemManager::UpdateFog()
{
    if (!FogActor) return;
    
    if (UExponentialHeightFogComponent* FogComponent = FogActor->GetComponent())
    {
        FogComponent->SetFogDensity(CurrentMood.FogDensity);
        FogComponent->SetFogInscatteringColor(CurrentMood.FogColor);
    }
}

void ULightingSystemManager::UpdateClouds()
{
    if (!CloudActor) return;
    
    if (UVolumetricCloudComponent* CloudComponent = CloudActor->GetVolumetricCloudComponent())
    {
        // Ajustar propriedades das nuvens baseadas no clima
        switch (CurrentWeather)
        {
            case EWeatherState::Clear:
                // Poucas nuvens
                break;
            case EWeatherState::PartlyCloudy:
                // Nuvens moderadas
                break;
            case EWeatherState::Overcast:
                // Céu coberto
                break;
            case EWeatherState::LightRain:
            case EWeatherState::HeavyRain:
            case EWeatherState::Storm:
                // Nuvens de tempestade
                break;
        }
    }
}

FLightingMood ULightingSystemManager::BlendMoods(const FLightingMood& MoodA, const FLightingMood& MoodB, float Alpha)
{
    FLightingMood BlendedMood;
    
    BlendedMood.SunColor = FLinearColor::LerpUsingHSV(MoodA.SunColor, MoodB.SunColor, Alpha);
    BlendedMood.SunIntensity = FMath::Lerp(MoodA.SunIntensity, MoodB.SunIntensity, Alpha);
    BlendedMood.SkyColor = FLinearColor::LerpUsingHSV(MoodA.SkyColor, MoodB.SkyColor, Alpha);
    BlendedMood.FogDensity = FMath::Lerp(MoodA.FogDensity, MoodB.FogDensity, Alpha);
    BlendedMood.FogColor = FLinearColor::LerpUsingHSV(MoodA.FogColor, MoodB.FogColor, Alpha);
    BlendedMood.ShadowIntensity = FMath::Lerp(MoodA.ShadowIntensity, MoodB.ShadowIntensity, Alpha);
    BlendedMood.ContrastBoost = FMath::Lerp(MoodA.ContrastBoost, MoodB.ContrastBoost, Alpha);
    
    return BlendedMood;
}

void ULightingSystemManager::InitializeDefaultMoods()
{
    // ===== MOODS POR TEMPO DO DIA =====
    
    // Dawn (05:00-07:00) - Luz dourada suave, esperança
    FLightingMood DawnMood;
    DawnMood.SunColor = FLinearColor(1.0f, 0.8f, 0.6f); // Dourado suave
    DawnMood.SunIntensity = 5.0f;
    DawnMood.SkyColor = FLinearColor(0.8f, 0.6f, 0.9f); // Rosa-lilás
    DawnMood.FogDensity = 0.04f;
    DawnMood.FogColor = FLinearColor(0.9f, 0.8f, 0.7f);
    DawnMood.ShadowIntensity = 0.6f;
    DawnMood.ContrastBoost = 0.8f;
    TimeOfDayMoods.Add(ETimeOfDay::Dawn, DawnMood);
    
    // Morning (07:00-11:00) - Luz clara, energia
    FLightingMood MorningMood;
    MorningMood.SunColor = FLinearColor(1.0f, 0.95f, 0.8f); // Branco quente
    MorningMood.SunIntensity = 8.0f;
    MorningMood.SkyColor = FLinearColor(0.4f, 0.7f, 1.0f); // Azul claro
    MorningMood.FogDensity = 0.02f;
    MorningMood.FogColor = FLinearColor(0.8f, 0.9f, 1.0f);
    MorningMood.ShadowIntensity = 0.7f;
    MorningMood.ContrastBoost = 1.0f;
    TimeOfDayMoods.Add(ETimeOfDay::Morning, MorningMood);
    
    // Midday (11:00-14:00) - Luz intensa, calor
    FLightingMood MiddayMood;
    MiddayMood.SunColor = FLinearColor(1.0f, 1.0f, 0.9f); // Branco puro
    MiddayMood.SunIntensity = 12.0f;
    MiddayMood.SkyColor = FLinearColor(0.2f, 0.5f, 1.0f); // Azul profundo
    MiddayMood.FogDensity = 0.015f;
    MiddayMood.FogColor = FLinearColor(0.7f, 0.8f, 1.0f);
    MiddayMood.ShadowIntensity = 0.9f;
    MiddayMood.ContrastBoost = 1.2f;
    TimeOfDayMoods.Add(ETimeOfDay::Midday, MiddayMood);
    
    // Afternoon (14:00-17:00) - Luz quente, relaxamento
    FLightingMood AfternoonMood;
    AfternoonMood.SunColor = FLinearColor(1.0f, 0.9f, 0.7f); // Dourado
    AfternoonMood.SunIntensity = 9.0f;
    AfternoonMood.SkyColor = FLinearColor(0.3f, 0.6f, 0.9f); // Azul médio
    AfternoonMood.FogDensity = 0.025f;
    AfternoonMood.FogColor = FLinearColor(0.8f, 0.8f, 0.9f);
    AfternoonMood.ShadowIntensity = 0.8f;
    AfternoonMood.ContrastBoost = 1.1f;
    TimeOfDayMoods.Add(ETimeOfDay::Afternoon, AfternoonMood);
    
    // Dusk (17:00-19:00) - Luz dramática, tensão
    FLightingMood DuskMood;
    DuskMood.SunColor = FLinearColor(1.0f, 0.6f, 0.3f); // Laranja intenso
    DuskMood.SunIntensity = 6.0f;
    DuskMood.SkyColor = FLinearColor(0.7f, 0.4f, 0.6f); // Rosa-roxo
    DuskMood.FogDensity = 0.035f;
    DuskMood.FogColor = FLinearColor(0.9f, 0.7f, 0.6f);
    DuskMood.ShadowIntensity = 0.9f;
    DuskMood.ContrastBoost = 1.3f;
    TimeOfDayMoods.Add(ETimeOfDay::Dusk, DuskMood);
    
    // Night (19:00-05:00) - Luz lunar, mistério
    FLightingMood NightMood;
    NightMood.SunColor = FLinearColor(0.3f, 0.4f, 0.6f); // Azul lunar
    NightMood.SunIntensity = 2.0f;
    NightMood.SkyColor = FLinearColor(0.1f, 0.2f, 0.4f); // Azul escuro
    NightMood.FogDensity = 0.05f;
    NightMood.FogColor = FLinearColor(0.4f, 0.5f, 0.7f);
    NightMood.ShadowIntensity = 1.0f;
    NightMood.ContrastBoost = 1.5f;
    TimeOfDayMoods.Add(ETimeOfDay::Night, NightMood);
    
    // ===== MOODS POR NÍVEL DE AMEAÇA =====
    
    // Safe - Iluminação normal, acolhedora
    FLightingMood SafeMood;
    SafeMood.SunColor = FLinearColor(1.0f, 1.0f, 1.0f);
    SafeMood.SunIntensity = 1.0f; // Multiplicador
    SafeMood.SkyColor = FLinearColor(1.0f, 1.0f, 1.0f);
    SafeMood.FogDensity = 1.0f; // Multiplicador
    SafeMood.FogColor = FLinearColor(1.0f, 1.0f, 1.0f);
    SafeMood.ShadowIntensity = 1.0f;
    SafeMood.ContrastBoost = 1.0f;
    ThreatMoods.Add(EThreatLevel::Safe, SafeMood);
    
    // Cautious - Ligeiramente mais frio, sombras mais profundas
    FLightingMood CautiousMood;
    CautiousMood.SunColor = FLinearColor(0.95f, 0.95f, 1.0f);
    CautiousMood.SunIntensity = 0.9f;
    CautiousMood.SkyColor = FLinearColor(0.9f, 0.9f, 1.0f);
    CautiousMood.FogDensity = 1.2f;
    CautiousMood.FogColor = FLinearColor(0.9f, 0.9f, 1.0f);
    CautiousMood.ShadowIntensity = 1.1f;
    CautiousMood.ContrastBoost = 1.1f;
    ThreatMoods.Add(EThreatLevel::Cautious, CautiousMood);
    
    // Dangerous - Tom mais frio, contraste alto
    FLightingMood DangerousMood;
    DangerousMood.SunColor = FLinearColor(0.9f, 0.9f, 1.0f);
    DangerousMood.SunIntensity = 0.8f;
    DangerousMood.SkyColor = FLinearColor(0.8f, 0.8f, 1.0f);
    DangerousMood.FogDensity = 1.5f;
    DangerousMood.FogColor = FLinearColor(0.8f, 0.8f, 1.0f);
    DangerousMood.ShadowIntensity = 1.3f;
    DangerousMood.ContrastBoost = 1.3f;
    ThreatMoods.Add(EThreatLevel::Dangerous, DangerousMood);
    
    // Hunted - Tom muito frio, sombras intensas, névoa densa
    FLightingMood HuntedMood;
    HuntedMood.SunColor = FLinearColor(0.8f, 0.8f, 1.0f);
    HuntedMood.SunIntensity = 0.6f;
    HuntedMood.SkyColor = FLinearColor(0.7f, 0.7f, 1.0f);
    HuntedMood.FogDensity = 2.0f;
    HuntedMood.FogColor = FLinearColor(0.7f, 0.7f, 1.0f);
    HuntedMood.ShadowIntensity = 1.5f;
    HuntedMood.ContrastBoost = 1.6f;
    ThreatMoods.Add(EThreatLevel::Hunted, HuntedMood);
}

void ULightingSystemManager::FindLightingComponents()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Encontrar componentes de iluminação na cena
    SunLight = Cast<ADirectionalLight>(UGameplayStatics::GetActorOfClass(World, ADirectionalLight::StaticClass()));
    SkyLightActor = Cast<ASkyLight>(UGameplayStatics::GetActorOfClass(World, ASkyLight::StaticClass()));
    SkyAtmosphereActor = Cast<ASkyAtmosphere>(UGameplayStatics::GetActorOfClass(World, ASkyAtmosphere::StaticClass()));
    CloudActor = Cast<AVolumetricCloud>(UGameplayStatics::GetActorOfClass(World, AVolumetricCloud::StaticClass()));
    FogActor = Cast<AExponentialHeightFog>(UGameplayStatics::GetActorOfClass(World, AExponentialHeightFog::StaticClass()));
}