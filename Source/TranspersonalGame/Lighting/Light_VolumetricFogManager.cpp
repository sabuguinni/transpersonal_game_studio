#include "Light_VolumetricFogManager.h"
#include "Engine/World.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ULight_VolumetricFogManager::ULight_VolumetricFogManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    HeightFogActor = nullptr;
    HeightFogComponent = nullptr;
    bFogSystemInitialized = false;
}

void ULight_VolumetricFogManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeFogSystem();
    InitializeBiomeFogProfiles();
    UpdateFogSettings();
}

void ULight_VolumetricFogManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bFogSystemInitialized)
    {
        return;
    }
    
    // Auto-detect biome if enabled
    if (bAutoDetectBiome)
    {
        DetectCurrentBiome();
    }
    
    // Update fog if time or biome changed
    if (CurrentTimeOfDay != LastTimeOfDay || CurrentBiome != LastBiome)
    {
        UpdateFogSettings();
        LastTimeOfDay = CurrentTimeOfDay;
        LastBiome = CurrentBiome;
    }
}

void ULight_VolumetricFogManager::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateFogSettings();
}

void ULight_VolumetricFogManager::SetCurrentBiome(EBiomeType NewBiome)
{
    CurrentBiome = NewBiome;
    UpdateFogSettings();
}

void ULight_VolumetricFogManager::UpdateFogSettings()
{
    if (!HeightFogComponent || !bFogSystemInitialized)
    {
        return;
    }
    
    FLight_VolumetricFogSettings CurrentSettings = GetCurrentFogSettings();
    ApplyFogSettingsToComponent(CurrentSettings);
}

FLight_VolumetricFogSettings ULight_VolumetricFogManager::GetCurrentFogSettings() const
{
    FLight_BiomeFogProfile* BiomeProfile = const_cast<ULight_VolumetricFogManager*>(this)->GetBiomeFogProfile(CurrentBiome);
    if (!BiomeProfile)
    {
        return FLight_VolumetricFogSettings();
    }
    
    EDayNightPhase CurrentPhase = GetCurrentDayNightPhase();
    float TimeAlpha = GetTimeOfDayAlpha();
    
    FLight_VolumetricFogSettings ResultSettings;
    
    switch (CurrentPhase)
    {
        case EDayNightPhase::Dawn:
            ResultSettings = InterpolateFogSettings(BiomeProfile->NightSettings, BiomeProfile->DaySettings, TimeAlpha);
            break;
        case EDayNightPhase::Day:
            ResultSettings = BiomeProfile->DaySettings;
            break;
        case EDayNightPhase::Dusk:
            ResultSettings = InterpolateFogSettings(BiomeProfile->DaySettings, BiomeProfile->NightSettings, TimeAlpha);
            break;
        case EDayNightPhase::Night:
            ResultSettings = BiomeProfile->NightSettings;
            break;
        default:
            ResultSettings = BiomeProfile->DaySettings;
            break;
    }
    
    return ResultSettings;
}

void ULight_VolumetricFogManager::InitializeBiomeFogProfiles()
{
    BiomeFogProfiles.Empty();
    
    // Savana Profile
    FLight_BiomeFogProfile SavanaProfile;
    SavanaProfile.BiomeType = EBiomeType::Savana;
    SavanaProfile.DaySettings.FogDensity = 0.01f;
    SavanaProfile.DaySettings.FogInscatteringColor = FLinearColor(1.0f, 0.9f, 0.7f);
    SavanaProfile.DaySettings.VolumetricFogDistance = 8000.0f;
    SavanaProfile.NightSettings.FogDensity = 0.03f;
    SavanaProfile.NightSettings.FogInscatteringColor = FLinearColor(0.3f, 0.4f, 0.7f);
    SavanaProfile.NightSettings.VolumetricFogDistance = 5000.0f;
    BiomeFogProfiles.Add(SavanaProfile);
    
    // Pantano Profile
    FLight_BiomeFogProfile PantanoProfile;
    PantanoProfile.BiomeType = EBiomeType::Pantano;
    PantanoProfile.DaySettings.FogDensity = 0.08f;
    PantanoProfile.DaySettings.FogInscatteringColor = FLinearColor(0.7f, 0.9f, 0.8f);
    PantanoProfile.DaySettings.VolumetricFogDistance = 3000.0f;
    PantanoProfile.NightSettings.FogDensity = 0.12f;
    PantanoProfile.NightSettings.FogInscatteringColor = FLinearColor(0.2f, 0.5f, 0.4f);
    PantanoProfile.NightSettings.VolumetricFogDistance = 2000.0f;
    BiomeFogProfiles.Add(PantanoProfile);
    
    // Floresta Profile
    FLight_BiomeFogProfile FlorestaProfile;
    FlorestaProfile.BiomeType = EBiomeType::Floresta;
    FlorestaProfile.DaySettings.FogDensity = 0.04f;
    FlorestaProfile.DaySettings.FogInscatteringColor = FLinearColor(0.6f, 0.9f, 0.6f);
    FlorestaProfile.DaySettings.VolumetricFogDistance = 4000.0f;
    FlorestaProfile.NightSettings.FogDensity = 0.07f;
    FlorestaProfile.NightSettings.FogInscatteringColor = FLinearColor(0.2f, 0.4f, 0.3f);
    FlorestaProfile.NightSettings.VolumetricFogDistance = 2500.0f;
    BiomeFogProfiles.Add(FlorestaProfile);
    
    // Deserto Profile
    FLight_BiomeFogProfile DesertoProfile;
    DesertoProfile.BiomeType = EBiomeType::Deserto;
    DesertoProfile.DaySettings.FogDensity = 0.005f;
    DesertoProfile.DaySettings.FogInscatteringColor = FLinearColor(1.0f, 0.8f, 0.6f);
    DesertoProfile.DaySettings.VolumetricFogDistance = 12000.0f;
    DesertoProfile.NightSettings.FogDensity = 0.02f;
    DesertoProfile.NightSettings.FogInscatteringColor = FLinearColor(0.4f, 0.4f, 0.6f);
    DesertoProfile.NightSettings.VolumetricFogDistance = 8000.0f;
    BiomeFogProfiles.Add(DesertoProfile);
    
    // Montanha Profile
    FLight_BiomeFogProfile MontanhaProfile;
    MontanhaProfile.BiomeType = EBiomeType::Montanha;
    MontanhaProfile.DaySettings.FogDensity = 0.06f;
    MontanhaProfile.DaySettings.FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f);
    MontanhaProfile.DaySettings.VolumetricFogDistance = 6000.0f;
    MontanhaProfile.NightSettings.FogDensity = 0.09f;
    MontanhaProfile.NightSettings.FogInscatteringColor = FLinearColor(0.3f, 0.4f, 0.6f);
    MontanhaProfile.NightSettings.VolumetricFogDistance = 4000.0f;
    BiomeFogProfiles.Add(MontanhaProfile);
}

void ULight_VolumetricFogManager::DetectCurrentBiome()
{
    // Simple biome detection based on actor location
    if (AActor* Owner = GetOwner())
    {
        FVector Location = Owner->GetActorLocation();
        
        // Biome detection based on coordinates
        if (FVector::Dist2D(Location, FVector(-50000, -45000, 0)) < BiomeDetectionRadius)
        {
            CurrentBiome = EBiomeType::Pantano;
        }
        else if (FVector::Dist2D(Location, FVector(-45000, 40000, 0)) < BiomeDetectionRadius)
        {
            CurrentBiome = EBiomeType::Floresta;
        }
        else if (FVector::Dist2D(Location, FVector(55000, 0, 0)) < BiomeDetectionRadius)
        {
            CurrentBiome = EBiomeType::Deserto;
        }
        else if (FVector::Dist2D(Location, FVector(40000, 50000, 0)) < BiomeDetectionRadius)
        {
            CurrentBiome = EBiomeType::Montanha;
        }
        else
        {
            CurrentBiome = EBiomeType::Savana;
        }
    }
}

FLight_VolumetricFogSettings ULight_VolumetricFogManager::InterpolateFogSettings(const FLight_VolumetricFogSettings& SettingsA, const FLight_VolumetricFogSettings& SettingsB, float Alpha) const
{
    FLight_VolumetricFogSettings Result;
    
    Result.FogDensity = FMath::Lerp(SettingsA.FogDensity, SettingsB.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(SettingsA.FogHeightFalloff, SettingsB.FogHeightFalloff, Alpha);
    Result.FogMaxOpacity = FMath::Lerp(SettingsA.FogMaxOpacity, SettingsB.FogMaxOpacity, Alpha);
    Result.StartDistance = FMath::Lerp(SettingsA.StartDistance, SettingsB.StartDistance, Alpha);
    Result.FogCutoffDistance = FMath::Lerp(SettingsA.FogCutoffDistance, SettingsB.FogCutoffDistance, Alpha);
    Result.FogInscatteringColor = FLinearColor::LerpUsingHSV(SettingsA.FogInscatteringColor, SettingsB.FogInscatteringColor, Alpha);
    Result.FogAlbedo = FLinearColor::LerpUsingHSV(SettingsA.FogAlbedo, SettingsB.FogAlbedo, Alpha);
    Result.VolumetricFogScatteringDistribution = FMath::Lerp(SettingsA.VolumetricFogScatteringDistribution, SettingsB.VolumetricFogScatteringDistribution, Alpha);
    Result.VolumetricFogExtinctionScale = FMath::Lerp(SettingsA.VolumetricFogExtinctionScale, SettingsB.VolumetricFogExtinctionScale, Alpha);
    Result.VolumetricFogDistance = FMath::Lerp(SettingsA.VolumetricFogDistance, SettingsB.VolumetricFogDistance, Alpha);
    Result.VolumetricFogStaticLightingScatteringIntensity = FMath::Lerp(SettingsA.VolumetricFogStaticLightingScatteringIntensity, SettingsB.VolumetricFogStaticLightingScatteringIntensity, Alpha);
    Result.bOverrideLightColorsWithFogInscatteringColors = Alpha > 0.5f ? SettingsB.bOverrideLightColorsWithFogInscatteringColors : SettingsA.bOverrideLightColorsWithFogInscatteringColors;
    
    return Result;
}

void ULight_VolumetricFogManager::ApplyFogSettingsToComponent(const FLight_VolumetricFogSettings& Settings)
{
    if (!HeightFogComponent)
    {
        return;
    }
    
    HeightFogComponent->SetFogDensity(Settings.FogDensity);
    HeightFogComponent->SetFogHeightFalloff(Settings.FogHeightFalloff);
    HeightFogComponent->SetFogMaxOpacity(Settings.FogMaxOpacity);
    HeightFogComponent->SetStartDistance(Settings.StartDistance);
    HeightFogComponent->SetFogCutoffDistance(Settings.FogCutoffDistance);
    HeightFogComponent->SetFogInscatteringColor(Settings.FogInscatteringColor);
    HeightFogComponent->SetVolumetricFogAlbedo(Settings.FogAlbedo);
    HeightFogComponent->SetVolumetricFogScatteringDistribution(Settings.VolumetricFogScatteringDistribution);
    HeightFogComponent->SetVolumetricFogExtinctionScale(Settings.VolumetricFogExtinctionScale);
    HeightFogComponent->SetVolumetricFogDistance(Settings.VolumetricFogDistance);
    HeightFogComponent->SetVolumetricFogStaticLightingScatteringIntensity(Settings.VolumetricFogStaticLightingScatteringIntensity);
    HeightFogComponent->SetbOverrideLightColorsWithFogInscatteringColors(Settings.bOverrideLightColorsWithFogInscatteringColors);
}

void ULight_VolumetricFogManager::CreateHeightFogActor()
{
    if (UWorld* World = GetWorld())
    {
        HeightFogActor = World->SpawnActor<AExponentialHeightFog>();
        if (HeightFogActor)
        {
            HeightFogComponent = HeightFogActor->GetComponent();
            if (HeightFogComponent)
            {
                HeightFogComponent->SetVolumetricFog(true);
                bFogSystemInitialized = true;
            }
        }
    }
}

void ULight_VolumetricFogManager::EnableVolumetricFog(bool bEnable)
{
    if (HeightFogComponent)
    {
        HeightFogComponent->SetVolumetricFog(bEnable);
    }
}

void ULight_VolumetricFogManager::SetFogDensityMultiplier(float Multiplier)
{
    if (HeightFogComponent)
    {
        FLight_VolumetricFogSettings CurrentSettings = GetCurrentFogSettings();
        CurrentSettings.FogDensity *= Multiplier;
        ApplyFogSettingsToComponent(CurrentSettings);
    }
}

void ULight_VolumetricFogManager::InitializeFogSystem()
{
    // Find existing height fog actor or create new one
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            HeightFogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
            if (HeightFogActor)
            {
                HeightFogComponent = HeightFogActor->GetComponent();
                if (HeightFogComponent)
                {
                    HeightFogComponent->SetVolumetricFog(true);
                    bFogSystemInitialized = true;
                }
            }
        }
        else
        {
            CreateHeightFogActor();
        }
    }
}

FLight_BiomeFogProfile* ULight_VolumetricFogManager::GetBiomeFogProfile(EBiomeType BiomeType)
{
    for (FLight_BiomeFogProfile& Profile : BiomeFogProfiles)
    {
        if (Profile.BiomeType == BiomeType)
        {
            return &Profile;
        }
    }
    return nullptr;
}

float ULight_VolumetricFogManager::GetTimeOfDayAlpha() const
{
    EDayNightPhase CurrentPhase = GetCurrentDayNightPhase();
    
    switch (CurrentPhase)
    {
        case EDayNightPhase::Dawn:
            return (CurrentTimeOfDay - 5.0f) / 2.0f; // 5-7 AM
        case EDayNightPhase::Dusk:
            return (CurrentTimeOfDay - 18.0f) / 2.0f; // 6-8 PM
        default:
            return 0.0f;
    }
}

EDayNightPhase ULight_VolumetricFogManager::GetCurrentDayNightPhase() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
    {
        return EDayNightPhase::Dawn;
    }
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 18.0f)
    {
        return EDayNightPhase::Day;
    }
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 20.0f)
    {
        return EDayNightPhase::Dusk;
    }
    else
    {
        return EDayNightPhase::Night;
    }
}