#include "Light_CinematicLightingManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SpotLight.h"
#include "Engine/PointLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

ALight_CinematicLightingManager::ALight_CinematicLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;
    TimeOfDayProgress = 0.0f;
    bEnableDynamicTimeOfDay = false;
    DayDurationMinutes = 20.0f;
    
    bEnableVolumetricFog = true;
    BaseFogDensity = 0.02f;
    FogInscatteringColor = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
    VolumetricFogScatteringDistribution = 0.6f;

    MainDirectionalLight = nullptr;
    MainFogActor = nullptr;
    TimeAccumulator = 0.0f;
    bLightingSystemInitialized = false;
}

void ALight_CinematicLightingManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindAndCacheLightReferences();
    InitializeDefaultLightingPresets();
    ValidateLightingSetup();
    
    bLightingSystemInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("CinematicLightingManager: System initialized successfully"));
}

void ALight_CinematicLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bLightingSystemInitialized)
    {
        return;
    }

    if (bEnableDynamicTimeOfDay)
    {
        UpdateTimeOfDay(DeltaTime);
    }

    UpdateVolumetricFog();
    UpdateCinematicZones();
}

void ALight_CinematicLightingManager::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    FLight_LightingPreset Preset = GetPresetForTimeOfDay(NewTimeOfDay);
    ApplyLightingPreset(Preset);
    
    UE_LOG(LogTemp, Warning, TEXT("CinematicLightingManager: Time of day set to %d"), (int32)NewTimeOfDay);
}

void ALight_CinematicLightingManager::SetTimeOfDayProgress(float Progress)
{
    TimeOfDayProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
    
    // Determine current and next time periods
    int32 CurrentIndex = (int32)(Progress * 7.0f);
    int32 NextIndex = (CurrentIndex + 1) % 7;
    float Alpha = FMath::Fmod(Progress * 7.0f, 1.0f);
    
    ELight_TimeOfDay CurrentPeriod = (ELight_TimeOfDay)CurrentIndex;
    ELight_TimeOfDay NextPeriod = (ELight_TimeOfDay)NextIndex;
    
    FLight_LightingPreset CurrentPreset = GetPresetForTimeOfDay(CurrentPeriod);
    FLight_LightingPreset NextPreset = GetPresetForTimeOfDay(NextPeriod);
    
    InterpolateLightingPresets(CurrentPreset, NextPreset, Alpha);
}

void ALight_CinematicLightingManager::ApplyLightingPreset(const FLight_LightingPreset& Preset)
{
    if (MainDirectionalLight && MainDirectionalLight->GetLightComponent())
    {
        UDirectionalLightComponent* DirLightComp = MainDirectionalLight->GetLightComponent();
        DirLightComp->SetLightColor(Preset.DirectionalLightColor);
        DirLightComp->SetIntensity(Preset.DirectionalLightIntensity);
        DirLightComp->SetCastVolumetricShadow(true);
        DirLightComp->SetVolumetricScatteringIntensity(Preset.VolumetricScattering);
        
        MainDirectionalLight->SetActorRotation(Preset.DirectionalLightRotation);
    }

    if (MainFogActor && MainFogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = MainFogActor->GetComponent();
        FogComp->SetFogDensity(Preset.FogDensity);
        FogComp->SetFogInscatteringColor(Preset.FogColor);
        FogComp->SetVolumetricFog(bEnableVolumetricFog);
        FogComp->SetVolumetricFogScatteringDistribution(VolumetricFogScatteringDistribution);
    }
}

void ALight_CinematicLightingManager::CreateCinematicZone(FVector Center, float Radius, ELight_LightingZone ZoneType)
{
    FLight_CinematicZone NewZone;
    NewZone.ZoneCenter = Center;
    NewZone.ZoneRadius = Radius;
    NewZone.ZoneType = ZoneType;
    NewZone.bIsActive = true;

    // Create appropriate lights based on zone type
    switch (ZoneType)
    {
        case ELight_LightingZone::Dramatic:
            CreateDramaticRimLighting(Center, FLinearColor(1.0f, 0.8f, 0.6f, 1.0f), 2500.0f);
            break;
        case ELight_LightingZone::Mysterious:
            CreateAtmosphericPointLight(Center + FVector(0, 0, 150), FLinearColor(0.6f, 0.7f, 1.0f, 1.0f), 1500.0f, 800.0f);
            break;
        case ELight_LightingZone::Warm:
            CreateAtmosphericPointLight(Center + FVector(0, 0, 100), FLinearColor(1.0f, 0.7f, 0.4f, 1.0f), 2000.0f, 1000.0f);
            break;
        case ELight_LightingZone::Cool:
            CreateVolumetricSpotlight(Center + FVector(300, 300, 200), FRotator(-45, -135, 0), FLinearColor(0.5f, 0.8f, 1.0f, 1.0f), 1800.0f);
            break;
        default:
            CreateAtmosphericPointLight(Center + FVector(0, 0, 120), FLinearColor(0.9f, 0.8f, 0.7f, 1.0f), 1200.0f, 600.0f);
            break;
    }

    CinematicZones.Add(NewZone);
    
    UE_LOG(LogTemp, Warning, TEXT("CinematicLightingManager: Created cinematic zone at %s"), *Center.ToString());
}

void ALight_CinematicLightingManager::UpdateCinematicZone(int32 ZoneIndex, bool bActivate)
{
    if (CinematicZones.IsValidIndex(ZoneIndex))
    {
        CinematicZones[ZoneIndex].bIsActive = bActivate;
        
        for (AActor* Light : CinematicZones[ZoneIndex].ZoneLights)
        {
            if (Light)
            {
                Light->SetActorHiddenInGame(!bActivate);
            }
        }
    }
}

void ALight_CinematicLightingManager::SetVolumetricFogProperties(float Density, FLinearColor InscatteringColor, float ScatteringDistribution)
{
    BaseFogDensity = Density;
    FogInscatteringColor = InscatteringColor;
    VolumetricFogScatteringDistribution = ScatteringDistribution;
    
    UpdateVolumetricFog();
}

void ALight_CinematicLightingManager::CreateDramaticRimLighting(FVector TargetLocation, FLinearColor LightColor, float Intensity)
{
    if (UWorld* World = GetWorld())
    {
        FVector SpawnLocation = TargetLocation + FVector(300, 300, 200);
        FRotator SpawnRotation = FRotator(-30, -135, 0);
        
        ASpotLight* RimLight = World->SpawnActor<ASpotLight>(ASpotLight::StaticClass(), SpawnLocation, SpawnRotation);
        if (RimLight && RimLight->GetLightComponent())
        {
            USpotLightComponent* SpotComp = RimLight->GetLightComponent();
            SpotComp->SetLightColor(LightColor);
            SpotComp->SetIntensity(Intensity);
            SpotComp->SetInnerConeAngle(25.0f);
            SpotComp->SetOuterConeAngle(45.0f);
            SpotComp->SetCastVolumetricShadow(true);
            SpotComp->SetAttenuationRadius(1200.0f);
            
            RimLight->SetActorLabel(TEXT("DramaticRimLight"));
            CinematicSpotLights.Add(RimLight);
        }
    }
}

void ALight_CinematicLightingManager::CreateAtmosphericPointLight(FVector Location, FLinearColor Color, float Intensity, float Radius)
{
    if (UWorld* World = GetWorld())
    {
        APointLight* AtmoLight = World->SpawnActor<APointLight>(APointLight::StaticClass(), Location, FRotator::ZeroRotator);
        if (AtmoLight && AtmoLight->GetLightComponent())
        {
            UPointLightComponent* PointComp = AtmoLight->GetLightComponent();
            PointComp->SetLightColor(Color);
            PointComp->SetIntensity(Intensity);
            PointComp->SetAttenuationRadius(Radius);
            PointComp->SetCastVolumetricShadow(true);
            PointComp->SetSourceRadius(50.0f);
            
            AtmoLight->SetActorLabel(TEXT("AtmosphericPointLight"));
            CinematicPointLights.Add(AtmoLight);
        }
    }
}

void ALight_CinematicLightingManager::CreateVolumetricSpotlight(FVector Location, FRotator Rotation, FLinearColor Color, float Intensity)
{
    if (UWorld* World = GetWorld())
    {
        ASpotLight* VolumetricSpot = World->SpawnActor<ASpotLight>(ASpotLight::StaticClass(), Location, Rotation);
        if (VolumetricSpot && VolumetricSpot->GetLightComponent())
        {
            USpotLightComponent* SpotComp = VolumetricSpot->GetLightComponent();
            SpotComp->SetLightColor(Color);
            SpotComp->SetIntensity(Intensity);
            SpotComp->SetInnerConeAngle(20.0f);
            SpotComp->SetOuterConeAngle(40.0f);
            SpotComp->SetCastVolumetricShadow(true);
            SpotComp->SetAttenuationRadius(1000.0f);
            SpotComp->SetVolumetricScatteringIntensity(2.0f);
            
            VolumetricSpot->SetActorLabel(TEXT("VolumetricSpotlight"));
            CinematicSpotLights.Add(VolumetricSpot);
        }
    }
}

void ALight_CinematicLightingManager::FindAndCacheLightReferences()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            MainDirectionalLight = Cast<ADirectionalLight>(FoundActors[0]);
        }

        FoundActors.Empty();
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            MainFogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
        }
    }
}

void ALight_CinematicLightingManager::ValidateLightingSetup()
{
    bool bIsValid = true;
    
    if (!MainDirectionalLight)
    {
        UE_LOG(LogTemp, Error, TEXT("CinematicLightingManager: No DirectionalLight found in scene"));
        bIsValid = false;
    }
    
    if (!MainFogActor)
    {
        UE_LOG(LogTemp, Error, TEXT("CinematicLightingManager: No ExponentialHeightFog found in scene"));
        bIsValid = false;
    }
    
    if (bIsValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("CinematicLightingManager: Lighting setup validation passed"));
    }
}

void ALight_CinematicLightingManager::InitializeDefaultLightingPresets()
{
    TimeOfDayPresets.Empty();
    
    // Dawn
    FLight_LightingPreset Dawn;
    Dawn.PresetName = TEXT("Dawn");
    Dawn.DirectionalLightColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    Dawn.DirectionalLightIntensity = 2.5f;
    Dawn.DirectionalLightRotation = FRotator(-15.0f, 75.0f, 0.0f);
    Dawn.FogColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
    Dawn.FogDensity = 0.035f;
    Dawn.VolumetricScattering = 1.5f;
    TimeOfDayPresets.Add(Dawn);
    
    // Golden Hour
    FLight_LightingPreset GoldenHour;
    GoldenHour.PresetName = TEXT("Golden Hour");
    GoldenHour.DirectionalLightColor = FLinearColor(1.0f, 0.85f, 0.7f, 1.0f);
    GoldenHour.DirectionalLightIntensity = 4.5f;
    GoldenHour.DirectionalLightRotation = FRotator(-25.0f, 45.0f, 0.0f);
    GoldenHour.FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
    GoldenHour.FogDensity = 0.025f;
    GoldenHour.VolumetricScattering = 1.2f;
    TimeOfDayPresets.Add(GoldenHour);
    
    // Night
    FLight_LightingPreset Night;
    Night.PresetName = TEXT("Night");
    Night.DirectionalLightColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    Night.DirectionalLightIntensity = 0.8f;
    Night.DirectionalLightRotation = FRotator(-80.0f, 180.0f, 0.0f);
    Night.FogColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
    Night.FogDensity = 0.015f;
    Night.VolumetricScattering = 0.8f;
    TimeOfDayPresets.Add(Night);
}

void ALight_CinematicLightingManager::CreateTestCinematicZones()
{
    CreateCinematicZone(FVector(1000, 1000, 0), 800.0f, ELight_LightingZone::Dramatic);
    CreateCinematicZone(FVector(-1000, 1000, 0), 600.0f, ELight_LightingZone::Mysterious);
    CreateCinematicZone(FVector(0, -1500, 0), 1000.0f, ELight_LightingZone::Warm);
    
    UE_LOG(LogTemp, Warning, TEXT("CinematicLightingManager: Test cinematic zones created"));
}

void ALight_CinematicLightingManager::UpdateTimeOfDay(float DeltaTime)
{
    TimeAccumulator += DeltaTime;
    
    float DayDurationSeconds = DayDurationMinutes * 60.0f;
    TimeOfDayProgress = FMath::Fmod(TimeAccumulator / DayDurationSeconds, 1.0f);
    
    SetTimeOfDayProgress(TimeOfDayProgress);
}

void ALight_CinematicLightingManager::InterpolateLightingPresets(const FLight_LightingPreset& PresetA, const FLight_LightingPreset& PresetB, float Alpha)
{
    FLight_LightingPreset InterpolatedPreset;
    
    InterpolatedPreset.DirectionalLightColor = FMath::Lerp(PresetA.DirectionalLightColor, PresetB.DirectionalLightColor, Alpha);
    InterpolatedPreset.DirectionalLightIntensity = FMath::Lerp(PresetA.DirectionalLightIntensity, PresetB.DirectionalLightIntensity, Alpha);
    InterpolatedPreset.DirectionalLightRotation = FMath::Lerp(PresetA.DirectionalLightRotation, PresetB.DirectionalLightRotation, Alpha);
    InterpolatedPreset.FogColor = FMath::Lerp(PresetA.FogColor, PresetB.FogColor, Alpha);
    InterpolatedPreset.FogDensity = FMath::Lerp(PresetA.FogDensity, PresetB.FogDensity, Alpha);
    InterpolatedPreset.VolumetricScattering = FMath::Lerp(PresetA.VolumetricScattering, PresetB.VolumetricScattering, Alpha);
    
    ApplyLightingPreset(InterpolatedPreset);
}

void ALight_CinematicLightingManager::UpdateVolumetricFog()
{
    if (MainFogActor && MainFogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = MainFogActor->GetComponent();
        FogComp->SetVolumetricFog(bEnableVolumetricFog);
        FogComp->SetVolumetricFogScatteringDistribution(VolumetricFogScatteringDistribution);
        FogComp->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.85f, 0.7f, 1.0f));
    }
}

void ALight_CinematicLightingManager::UpdateCinematicZones()
{
    for (FLight_CinematicZone& Zone : CinematicZones)
    {
        if (!Zone.bIsActive)
        {
            continue;
        }
        
        // Update zone lights based on current lighting conditions
        for (AActor* Light : Zone.ZoneLights)
        {
            if (Light)
            {
                // Adjust light intensity based on time of day
                float IntensityMultiplier = 1.0f;
                if (CurrentTimeOfDay == ELight_TimeOfDay::Night)
                {
                    IntensityMultiplier = 1.5f;
                }
                else if (CurrentTimeOfDay == ELight_TimeOfDay::Midday)
                {
                    IntensityMultiplier = 0.7f;
                }
                
                if (APointLight* PointLight = Cast<APointLight>(Light))
                {
                    if (PointLight->GetLightComponent())
                    {
                        float BaseIntensity = 1800.0f;
                        PointLight->GetLightComponent()->SetIntensity(BaseIntensity * IntensityMultiplier);
                    }
                }
                else if (ASpotLight* SpotLight = Cast<ASpotLight>(Light))
                {
                    if (SpotLight->GetLightComponent())
                    {
                        float BaseIntensity = 2500.0f;
                        SpotLight->GetLightComponent()->SetIntensity(BaseIntensity * IntensityMultiplier);
                    }
                }
            }
        }
    }
}

FLight_LightingPreset ALight_CinematicLightingManager::GetPresetForTimeOfDay(ELight_TimeOfDay TimeOfDay)
{
    for (const FLight_LightingPreset& Preset : TimeOfDayPresets)
    {
        if (Preset.PresetName.Contains(TEXT("Dawn")) && TimeOfDay == ELight_TimeOfDay::Dawn)
            return Preset;
        if (Preset.PresetName.Contains(TEXT("Golden")) && TimeOfDay == ELight_TimeOfDay::GoldenHour)
            return Preset;
        if (Preset.PresetName.Contains(TEXT("Night")) && TimeOfDay == ELight_TimeOfDay::Night)
            return Preset;
    }
    
    // Return default golden hour preset if not found
    return TimeOfDayPresets.Num() > 1 ? TimeOfDayPresets[1] : FLight_LightingPreset();
}